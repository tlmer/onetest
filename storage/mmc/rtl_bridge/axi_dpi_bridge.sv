// ============================================================================
// axi_dpi_bridge.sv -- onetest Linux-proxy model <-> AXI4 NoC bridge (SV side)
//
//   **FAMILY-WIDE FILE** -- one bridge for every onetest line (pcie/usb/
//   storage/network/smmu/nvme).  The customer-facing interface (module name,
//   ports, parameters, counters) is IDENTICAL across lines; only the DPI
//   symbol prefix differs (pcie_/usb_/mmc_/sata_/ufs_/eth_/smmu_/nvme_) so
//   that several models may coexist in one simulation without symbol clashes.
//
//   [*] Interface contract (customer checklist #2): AxID width = 4 |
//       outstanding = 8 | data width = 128 bit (16 B/beat, narrow writes
//       select lanes via WSTRB)
//   => this bridge = an 8-slot in-flight table (in-order within one ID;
//      different IDs may complete out of order).
//
//   Ports
//     m_*     AXI4 **master**  -> the NoC (model issues register/config/memory)
//     s_*     AXI4 **slave**   <- the NoC (DUT posted writes: DMA write-back, MSI)
//     irq_out **active-high level** interrupt OUT (model -> the SoC's interrupt
//             controller; stays high while pending, falls when the host
//             services it).  Used by pcie (MSI/MSI-X level). Lines whose role
//             is the *host* (usb/storage/network/smmu) tie it to 1'b0.
//     irq_in  **active-high level** interrupt IN (the DUT -> the model).
//             Sampled every clk; on a value change the C side is told
//             (`c_*_irq_set`), which latches it and runs the line's ISR.
//             pcie ties it to 1'b0.
//
//   DPI  exports (C side calls in) : <p>_axi_submit
//        imports (SV calls out)    : c_<p>_wdata / c_<p>_rdata / c_<p>_done
//                                    c_<p>_slv_wbeat / c_<p>_slv_rbeat
//                                    c_<p>_slv_bresp
//                                    c_<p>_irq_level / c_<p>_irq_set
// ============================================================================
`timescale 1ns/1ps

module axi_dpi_bridge #(
    parameter integer AW        = 64,       // 64-bit address (host/PCIe-style)
    parameter integer IDW       = 4,        // [*] AxID width = 4 (contract)
    parameter integer DW        = 128,      // [*] data width = 128 bit (contract)
    parameter integer OSLOT     = 8,        // [*] outstanding = 8 (contract)
    parameter integer MAX_BEATS = 256       // 4 KB / 16 B
)(
    input  wire             clk,
    input  wire             rst_n,

    // -- AXI4 master port (model -> NoC) --
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

    // -- AXI4 slave port (NoC -> model: DUT posted writes / reads) --
    input  wire [IDW-1:0]   s_awid,
    input  wire [AW-1:0]    s_awaddr,
    input  wire [7:0]       s_awlen,
    input  wire [2:0]       s_awsize,
    input  wire [1:0]       s_awburst,
    input  wire             s_awvalid,
    output reg              s_awready,

    input  wire [DW-1:0]    s_wdata,
    input  wire [DW/8-1:0]  s_wstrb,
    input  wire             s_wlast,
    input  wire             s_wvalid,
    output reg              s_wready,

    output reg  [IDW-1:0]   s_bid,
    output reg  [1:0]       s_bresp,
    output reg              s_bvalid,
    input  wire             s_bready,

    input  wire [IDW-1:0]   s_arid,
    input  wire [AW-1:0]    s_araddr,
    input  wire [7:0]       s_arlen,
    input  wire [2:0]       s_arsize,
    input  wire [1:0]       s_arburst,
    input  wire             s_arvalid,
    output reg              s_arready,

    output reg  [IDW-1:0]   s_rid,
    output reg  [DW-1:0]    s_rdata,
    output reg  [1:0]       s_rresp,
    output reg              s_rlast,
    output reg              s_rvalid,
    input  wire             s_rready,

    // -- interrupts: ACTIVE HIGH, level (out = model -> SoC; in = DUT -> model) --
    output reg              irq_out,
    input  wire             irq_in,

    // -- observation surface (criteria) --
    output reg  [31:0]      n_done,
    output reg  [31:0]      n_busy_retry,
    output reg  [31:0]      n_rerr,
    output reg  [31:0]      n_slv_recv,
    output reg  [31:0]      n_slv_miss
);
    localparam integer SLW = 3;             // OSLOT=8 => 3-bit slot index

    // -- 8-slot in-flight table (model -> NoC) --
    reg              s_valid [0:OSLOT-1];
    reg              s_isw   [0:OSLOT-1];
    reg  [AW-1:0]    s_addr  [0:OSLOT-1];
    reg  [2:0]       s_size  [0:OSLOT-1];
    reg  [8:0]       s_beats [0:OSLOT-1];
    reg  [IDW-1:0]   s_id    [0:OSLOT-1];
    reg  [63:0]      s_h     [0:OSLOT-1];
    reg  [8:0]       s_beat  [0:OSLOT-1];
    reg              s_issued[0:OSLOT-1];

    // -- C-side interface (DPI) --
    import "DPI-C" function void c_mmc_wdata(input longint unsigned h, input int beat,
                                              output bit [DW-1:0] data, output bit [DW/8-1:0] strb);
    import "DPI-C" function void c_mmc_rdata(input longint unsigned h, input int beat,
                                              input bit [DW-1:0] data);
    import "DPI-C" function void c_mmc_done (input longint unsigned h, input int resp);

    // slave-side: one call per BEAT => the C side routes each beat (axs_recv)
    import "DPI-C" function int  c_mmc_slv_wbeat(input longint unsigned addr, input int size,
                                                  input int beat, input bit is_last, input int id,
                                                  input bit [DW-1:0] data, input bit [DW/8-1:0] strb);
    import "DPI-C" function int  c_mmc_slv_rbeat(input longint unsigned addr, input int size,
                                                  input int beat, input bit is_last, input int id,
                                                  output bit [DW-1:0] data);
    import "DPI-C" function void c_mmc_slv_bresp(input longint unsigned addr, input int resp);
    import "DPI-C" function int  c_mmc_irq_level();
    import "DPI-C" function void c_mmc_irq_set(input int level);

    // -- DPI exports: C-side submit (find a free slot; full => return 1 => C retries) --
    export "DPI-C" function mmc_axi_submit;

    integer k;
    function int find_free_slot(output integer idx);
        for (k = 0; k < OSLOT; k++)
            if (!s_valid[k]) begin idx = k; return 0; end
        idx = -1;
        return 1;
    endfunction

    function int mmc_axi_submit(input longint unsigned addr, input int size, input int beats,
                                 input int id, input int is_write, input longint unsigned handle);
        integer idx;
        if (find_free_slot(idx)) begin n_busy_retry = n_busy_retry + 32'd1; return 1; end
        s_valid[idx] = 1'b1; s_isw[idx] = is_write[0];
        s_addr[idx] = addr[AW-1:0]; s_size[idx] = size[2:0]; s_beats[idx] = beats[8:0];
        s_id[idx] = id[IDW-1:0]; s_h[idx] = handle; s_beat[idx] = 9'd0; s_issued[idx] = 1'b0;
        return 0;
    endfunction

    // -- master dispatch --
    localparam I_IDLE = 2'd0, I_AW = 2'd1, I_W = 2'd2, I_AR = 2'd3;
    reg [1:0]  ist;
    reg [SLW-1:0] cur;
    reg [DW-1:0] tmp;
    reg [DW/8-1:0] tmps;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            ist <= I_IDLE; m_awvalid <= 1'b0; m_wvalid <= 1'b0;
            m_arvalid <= 1'b0; m_wlast <= 1'b0; m_wstrb <= {DW/8{1'b1}};
            for (k = 0; k < OSLOT; k++) begin s_valid[k] <= 1'b0; s_beat[k] <= 9'd0; s_issued[k] <= 1'b0; end
        end else begin
            case (ist)
                I_IDLE: begin
                    integer sel; sel = -1;
                    for (k = OSLOT-1; k >= 0; k--)
                        if (s_valid[k] && !s_issued[k]) sel = k;
                    if (sel >= 0) begin
                        cur <= sel[SLW-1:0];
                        s_issued[sel] <= 1'b1;
                        s_beat[sel] <= 9'd0;
                        if (s_isw[sel]) begin
                            m_awid <= s_id[sel]; m_awaddr <= s_addr[sel];
                            m_awlen <= s_beats[sel][7:0] - 8'd1; m_awsize <= s_size[sel];
                            m_awburst <= 2'b01; m_awvalid <= 1'b1;
                            ist <= I_AW;
                        end else begin
                            m_arid <= s_id[sel]; m_araddr <= s_addr[sel];
                            m_arlen <= s_beats[sel][7:0] - 8'd1; m_arsize <= s_size[sel];
                            m_arburst <= 2'b01; m_arvalid <= 1'b1;
                            ist <= I_AR;
                        end
                    end
                end
                I_AW: if (m_awready) begin
                    m_awvalid <= 1'b0;
                    c_mmc_wdata(s_h[cur], 9'd0, tmp, tmps);     // preload first beat
                    m_wdata <= tmp; m_wstrb <= tmps;
                    m_wlast <= (s_beats[cur] == 9'd1);
                    m_wvalid <= 1'b1;
                    ist <= I_W;
                end
                I_W: if (m_wvalid && m_wready) begin
                    if (m_wlast) begin
                        m_wvalid <= 1'b0;
                        ist <= I_IDLE;
                    end else begin
                        s_beat[cur] <= s_beat[cur] + 9'd1;
                        c_mmc_wdata(s_h[cur], s_beat[cur] + 9'd1, tmp, tmps);
                        m_wdata <= tmp; m_wstrb <= tmps;
                        m_wlast <= ((s_beat[cur] + 9'd1) == s_beats[cur] - 9'd1);
                    end
                end
                I_AR: if (m_arready) begin m_arvalid <= 1'b0; ist <= I_IDLE; end
                default: ist <= I_IDLE;
            endcase
        end
    end

    // -- master completion: B/R claim the slot back by ID --
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            m_bready <= 1'b1; m_rready <= 1'b1;
            n_done <= 32'd0; n_busy_retry <= 32'd0; n_rerr <= 32'd0;
            n_slv_recv <= 32'd0; n_slv_miss <= 32'd0;
        end else begin
            if (m_bvalid && m_bready) begin
                integer hit; hit = -1;
                for (k = OSLOT-1; k >= 0; k--)
                    if (s_valid[k] && s_isw[k] && (s_id[k] == m_bid)) hit = k;
                if (hit >= 0) begin
                    c_mmc_done(s_h[hit], m_bresp);
                    s_valid[hit] <= 1'b0;
                    n_done <= n_done + 32'd1;
                end else n_rerr <= n_rerr + 32'd1;
            end
            if (m_rvalid && m_rready) begin
                integer hit; hit = -1;
                for (k = OSLOT-1; k >= 0; k--)
                    if (s_valid[k] && !s_isw[k] && (s_id[k] == m_rid) && (s_beat[k] < s_beats[k])) hit = k;
                if (hit >= 0) begin
                    c_mmc_rdata(s_h[hit], s_beat[hit], m_rdata);
                    s_beat[hit] <= s_beat[hit] + 9'd1;
                    if (m_rlast || (s_beat[hit] == s_beats[hit] - 9'd1)) begin
                        c_mmc_done(s_h[hit], m_rresp);
                        s_valid[hit] <= 1'b0;
                        n_done <= n_done + 32'd1;
                    end
                end else n_rerr <= n_rerr + 32'd1;
            end
        end
    end

    // -- slave engine: DUT posted writes / reads => per-beat route callback --
    //    The C side routes ONE beat per call (the model's `axs_recv` unit),
    //    so this side hands over (base, size, beat_index) and the C side
    //    computes `base + beat*2^size` -- no guesswork on either side.
    reg [AW-1:0] sw_addr;
    reg [2:0]    sw_size;
    reg [8:0]    sw_beat;
    reg [IDW-1:0] sw_id;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            s_awready <= 1'b1; s_wready <= 1'b1;
            s_bvalid <= 1'b0; s_rvalid <= 1'b0;
            s_bresp <= 2'b00; s_rresp <= 2'b00; s_rlast <= 1'b0;
            sw_addr <= {AW{1'b0}}; sw_size <= 3'd0; sw_beat <= 9'd0; sw_id <= {IDW{1'b0}};
        end else begin
            // write address accepted => latch the burst descriptor
            if (s_awvalid && s_awready) begin
                sw_addr <= s_awaddr; sw_size <= s_awsize; sw_id <= s_awid; sw_beat <= 9'd0;
            end
            // write data beat => route it; last beat => B with the routed resp
            if (s_wvalid && s_wready) begin
                int r;
                r = c_mmc_slv_wbeat(sw_addr, sw_size, sw_beat, s_wlast, sw_id, s_wdata, s_wstrb);
                if (r != 0) n_slv_miss <= n_slv_miss + 32'd1;
                else        n_slv_recv <= n_slv_recv + 32'd1;
                if (s_wlast) begin
                    s_bid <= sw_id; s_bresp <= r[1:0]; s_bvalid <= 1'b1;
                end else begin
                    sw_beat <= sw_beat + 9'd1;
                end
            end
            if (s_bvalid && s_bready) s_bvalid <= 1'b0;
            // read: single-beat service (prototype scope -- .veo states the limit)
            if (s_arvalid && s_arready) begin
                s_rid <= s_arid;
                c_mmc_slv_rbeat(s_araddr, s_arsize, 9'd0, (s_arlen == 8'd0), s_arid, s_rdata);
                s_rresp <= 2'b00;
                s_rlast <= (s_arlen == 8'd0);
                s_rvalid <= 1'b1;
            end
            if (s_rvalid && s_rready) s_rvalid <= 1'b0;
        end
    end

    // -- interrupt OUT line: ACTIVE HIGH level, sampled from the model --
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) irq_out <= 1'b0;
        else        irq_out <= (c_mmc_irq_level() != 0);
    end

    // -- interrupt IN line: DUT -> model.  VALUE-CHANGE driven (family rule 7):
    //    the C side is told only when the level actually changes, and once at
    //    reset (level 0).  The C side latches it (never a bare level) and
    //    runs the line's ISR -- see <p>_axi_dpi_link.c.
    reg irq_in_q;
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            irq_in_q <= 1'b0;
            c_mmc_irq_set(0);
        end else begin
            irq_in_q <= irq_in;
            if (irq_in !== irq_in_q) c_mmc_irq_set(irq_in ? 1 : 0);
        end
    end
endmodule
