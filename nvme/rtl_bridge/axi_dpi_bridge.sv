// ============================================================================
// axi_dpi_bridge.sv -- AXI4 <-> DPI bridge (SV side; pairs with the C++-side DpiLink) [created 2026-10-11]
//   Bridge choice = DPI/hwfifo (user decision); [*] **NoC contract (user decision 2026-10-11):
//   AxID width = 4 | outstanding = 4** => this bridge = a **4-slot in-flight table** (in-order within
//   one ID; different IDs may complete out of order).
//   Flow:
//     C side (DpiLink) -- rtl_axi_write/rtl_axi_read (export; writes a free slot; full => return 1 and
//     retry) --> slot table
//        -> dispatch engine: AW+W / AR ([X] does not wait for B/R => at most 4 in flight)
//        -> completion engine: B/R **claim the slot back by BID/RID** => c_axi_done (import; call back
//           into C to release the block)
//       write data: c_get_wdata (fetched per beat) | read data: c_put_rdata (filled per beat)
//   [!] DW is fixed at 512 (64B/beat), a single request is <=64 beats (4KB) -- consistent with the
//     task-model prototype scope.
// ============================================================================
`timescale 1ns/1ps

module axi_dpi_bridge #(
    parameter integer AW        = 40,
    parameter integer IDW       = 4,        // [*] AxID width = 4 (contract)
    parameter integer DW        = 512,
    parameter integer OSLOT     = 4,        // [*] outstanding = 4 (contract)
    parameter integer MAX_BEATS = 64
)(
    input  wire             clk,
    input  wire             rst_n,

    // -- AXI4 master port (to the NoC) --
    output reg  [IDW-1:0]   m_awid,
    output reg  [AW-1:0]    m_awaddr,
    output reg  [7:0]       m_awlen,
    output reg  [2:0]       m_awsize,
    output reg  [1:0]       m_awburst,
    output reg              m_awvalid,
    input  wire             m_awready,

    output reg  [DW-1:0]    m_wdata,
    output reg  [DW/8-1:0]  m_wstrb,
    output reg              m_wlast,
    output reg              m_wvalid,
    input  wire             m_wready,

    input  wire [IDW-1:0]   m_bid,
    input  wire [1:0]       m_bresp,
    input  wire             m_bvalid,
    output reg              m_bready,

    output reg  [IDW-1:0]   m_arid,
    output reg  [AW-1:0]    m_araddr,
    output reg  [7:0]       m_arlen,
    output reg  [2:0]       m_arsize,
    output reg  [1:0]       m_arburst,
    output reg              m_arvalid,
    input  wire             m_arready,

    input  wire [IDW-1:0]   m_rid,
    input  wire [DW-1:0]    m_rdata,
    input  wire [1:0]       m_rresp,
    input  wire             m_rlast,
    input  wire             m_rvalid,
    output reg              m_rready,

    // -- bench / observation surface (used by the criteria) --
    output reg  [31:0]      n_done,
    output reg  [31:0]      n_busy_retry,   // times a slot was full on a C-side submit
    output reg  [31:0]      n_rerr          // no slot to claim back / protocol anomaly
);
    localparam integer SLW = 2;             // OSLOT=4 => 2-bit slot index

    // -- 4-slot in-flight table --
    reg              s_valid [0:OSLOT-1];
    reg              s_isw   [0:OSLOT-1];
    reg  [AW-1:0]    s_addr  [0:OSLOT-1];
    reg  [2:0]       s_size  [0:OSLOT-1];
    reg  [8:0]       s_beats [0:OSLOT-1];
    reg  [IDW-1:0]   s_id    [0:OSLOT-1];
    reg  [63:0]      s_h     [0:OSLOT-1];
    reg  [8:0]       s_beat  [0:OSLOT-1];   // beats dispatched / received
    reg              s_issued[0:OSLOT-1];   // [*] dispatched (W finished / AR issued => 1; prevents re-dispatch)

    // -- C-side interface (DPI) --
    import "DPI-C" function void c_get_wdata(input longint unsigned h, input int beat,
                                             output bit [DW-1:0] data);
    import "DPI-C" function void c_put_rdata(input longint unsigned h, input int beat,
                                             input bit [DW-1:0] data);
    import "DPI-C" function void c_axi_done (input longint unsigned h, input int resp);

    // -- DPI export: submit (find a free slot; full => return 1) --
    export "DPI-C" function rtl_axi_write;
    export "DPI-C" function rtl_axi_read;

    integer k;
    function int find_free_slot(output integer idx);
        for (k = 0; k < OSLOT; k++)
            if (!s_valid[k]) begin idx = k; return 0; end
        idx = -1;
        return 1;
    endfunction

    function int rtl_axi_write(input longint unsigned addr, input int size, input int beats,
                               input int id, input longint unsigned handle);
        integer idx;
        if (find_free_slot(idx)) begin n_busy_retry = n_busy_retry + 1; return 1; end
        s_valid[idx] = 1'b1; s_isw[idx] = 1'b1;
        s_addr[idx] = addr[AW-1:0]; s_size[idx] = size[2:0]; s_beats[idx] = beats[8:0];
        s_id[idx] = id[IDW-1:0]; s_h[idx] = handle; s_beat[idx] = 9'd0; s_issued[idx] = 1'b0;
        return 0;
    endfunction

    function int rtl_axi_read(input longint unsigned addr, input int size, input int beats,
                              input int id, input longint unsigned handle);
        integer idx;
        if (find_free_slot(idx)) begin n_busy_retry = n_busy_retry + 1; return 1; end
        s_valid[idx] = 1'b1; s_isw[idx] = 1'b0;
        s_addr[idx] = addr[AW-1:0]; s_size[idx] = size[2:0]; s_beats[idx] = beats[8:0];
        s_id[idx] = id[IDW-1:0]; s_h[idx] = handle; s_beat[idx] = 9'd0; s_issued[idx] = 1'b0;
        return 0;
    endfunction

    // -- dispatch engine ([X] does not wait for B/R => at most 4 in flight) --
    localparam I_IDLE = 2'd0, I_AW = 2'd1, I_W = 2'd2, I_AR = 2'd3;
    reg [1:0]  ist;
    reg [SLW-1:0] cur;
    reg [DW-1:0] tmp;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            ist <= I_IDLE; m_awvalid <= 1'b0; m_wvalid <= 1'b0;
            m_arvalid <= 1'b0; m_wlast <= 1'b0; m_wstrb <= {DW/8{1'b1}};
            for (k = 0; k < OSLOT; k++) begin s_valid[k] <= 1'b0; s_beat[k] <= 9'd0; s_issued[k] <= 1'b0; end
        end else begin
            case (ist)
                I_IDLE: begin
                    integer sel; sel = -1;
                    for (k = OSLOT-1; k >= 0; k--)          // take the lowest slot index (not yet dispatched)
                        if (s_valid[k] && !s_issued[k]) sel = k;
                    if (sel >= 0) begin
                        cur <= sel[SLW-1:0];
                        s_issued[sel] <= 1'b1;              // [*] mark as dispatched (prevents re-dispatch)
                        s_beat[sel] <= 9'd0;
                        if (s_isw[sel]) begin
                            m_awid <= s_id[sel]; m_awaddr <= s_addr[sel];
                            m_awlen <= s_beats[sel] - 9'd1; m_awsize <= s_size[sel];
                            m_awburst <= 2'b01; m_awvalid <= 1'b1;
                            ist <= I_AW;
                        end else begin
                            m_arid <= s_id[sel]; m_araddr <= s_addr[sel];
                            m_arlen <= s_beats[sel] - 9'd1; m_arsize <= s_size[sel];
                            m_arburst <= 2'b01; m_arvalid <= 1'b1;
                            ist <= I_AR;
                        end
                    end
                end
                I_AW: if (m_awready) begin
                    m_awvalid <= 1'b0;
                    c_get_wdata(s_h[cur], 9'd0, tmp);       // preload the first beat
                    m_wdata <= tmp; m_wstrb <= {DW/8{1'b1}};
                    m_wlast <= (s_beats[cur] == 9'd1);
                    m_wvalid <= 1'b1;
                    ist <= I_W;
                end
                I_W: if (m_wvalid && m_wready) begin        // [*] two-phase: valid already asserted => consume now
                    if (m_wlast) begin
                        m_wvalid <= 1'b0;
                        ist <= I_IDLE;
                    end else begin
                        s_beat[cur] <= s_beat[cur] + 9'd1;
                        c_get_wdata(s_h[cur], s_beat[cur] + 9'd1, tmp);
                        m_wdata <= tmp;
                        m_wlast <= ((s_beat[cur] + 9'd1) == s_beats[cur] - 9'd1);
                    end
                end
                I_AR: if (m_arready) begin m_arvalid <= 1'b0; ist <= I_IDLE; end
                default: ist <= I_IDLE;
            endcase
        end
    end

    // -- completion engine: B/R claim the oldest in-flight slot by ID --
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            m_bready <= 1'b1; m_rready <= 1'b1;
            n_done <= 32'd0; n_busy_retry <= 32'd0; n_rerr <= 32'd0;
        end else begin
            // B channel (write completion)
            if (m_bvalid && m_bready) begin
                integer hit; hit = -1;
                for (k = OSLOT-1; k >= 0; k--)              // oldest with the same ID
                    if (s_valid[k] && s_isw[k] && (s_id[k] == m_bid)) hit = k;
                if (hit >= 0) begin
                    c_axi_done(s_h[hit], m_bresp);
                    s_valid[hit] <= 1'b0;
                    n_done <= n_done + 32'd1;
                end else n_rerr <= n_rerr + 32'd1;
            end
            // R channel (read data / completion)
            if (m_rvalid && m_rready) begin
                integer hit; hit = -1;
                for (k = OSLOT-1; k >= 0; k--)
                    if (s_valid[k] && !s_isw[k] && (s_id[k] == m_rid) && (s_beat[k] < s_beats[k])) hit = k;
                if (hit >= 0) begin
                    c_put_rdata(s_h[hit], s_beat[hit], m_rdata);
                    s_beat[hit] <= s_beat[hit] + 9'd1;
                    if (m_rlast || (s_beat[hit] == s_beats[hit] - 9'd1)) begin
                        c_axi_done(s_h[hit], m_rresp);
                        s_valid[hit] <= 1'b0;
                        n_done <= n_done + 32'd1;
                    end
                end else n_rerr <= n_rerr + 32'd1;
            end
        end
    end
endmodule
