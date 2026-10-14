// ============================================================================
// tb_axi_dpi_bridge.sv -- SV-side self-check bench for the DPI bridge (xsim) [*] 4 slots in flight (outstanding=4)
//   Criteria: (1) 4 writes accepted + the 5th rejected (limit=4) (2) after the writes memory equals the
//             pattern byte by byte
//             (3) 4 reads accepted + read data claimed by RID, byte by byte (4) no claim anomalies
//             (5) handle echoed back
//   Final line: TB_AXI_DPI_BRIDGE PASS|FAIL
// ============================================================================
`timescale 1ns/1ps

module axi_slave_sim #(
    parameter integer AW = 40, IDW = 4, DW = 512, MB = 4096
)(
    input  wire clk, rst_n,
    input  wire [IDW-1:0] awid,  input wire [AW-1:0] awaddr, input wire [7:0] awlen,
    input  wire [2:0] awsize,    input wire [1:0] awburst,   input wire awvalid, output reg awready,
    input  wire [DW-1:0] wdata,  input wire [DW/8-1:0] wstrb, input wire wlast,  input wire wvalid, output reg wready,
    output reg  [IDW-1:0] bid,   output reg [1:0] bresp,      output reg bvalid,  input wire bready,
    input  wire [IDW-1:0] arid,  input wire [AW-1:0] araddr,  input wire [7:0] arlen,
    input  wire [2:0] arsize,    input wire [1:0] arburst,    input wire arvalid, output reg arready,
    output wire [IDW-1:0] rid,   output wire [DW-1:0] rdata,  output wire [1:0] rresp,
    output wire rlast,           output wire rvalid,          input wire rready
);
    reg [DW-1:0] mem [0:MB-1];
    reg [AW-1:0] wptr;
    reg [8:0]    wleft;
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            awready <= 1'b1; wready <= 1'b0; bvalid <= 1'b0; bresp <= 2'd0; bid <= 0; wptr <= 0; wleft <= 0;
        end else begin
            if (bvalid && bready) bvalid <= 1'b0;
            if (awvalid && awready) begin wptr <= awaddr; wleft <= awlen; awready <= 1'b0; wready <= 1'b1; end
            if (wvalid && wready) begin
                mem[wptr[AW-1:6]] <= wdata;
                wptr <= wptr + 64;
                if (wleft == 0) begin
                    wready <= 1'b0; bvalid <= 1'b1; bid <= awid; bresp <= 2'd0; awready <= 1'b1;
                end else wleft <= wleft - 1;
            end
        end
    end
    reg [AW-1:0] rptr;
    reg [8:0]    rleft;
    reg          rd_active;
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin arready <= 1'b1; rptr <= 0; rleft <= 0; rd_active <= 1'b0; end
        else begin
            if (arvalid && arready && !rd_active) begin
                rptr <= araddr; rleft <= arlen; arready <= 1'b0; rd_active <= 1'b1;
            end
            if (rd_active && rready) begin
                rptr <= rptr + 64;
                if (rleft == 0) begin rd_active <= 1'b0; arready <= 1'b1; end
                else rleft <= rleft - 1;
            end
        end
    end
    assign rid    = arid;
    assign rdata  = mem[rptr[AW-1:6]];
    assign rresp  = 2'd0;
    assign rvalid = rd_active;
    assign rlast  = (rleft == 0);
endmodule

module tb_axi_dpi_bridge;
    localparam AW = 40, IDW = 4, DW = 512;
    reg clk = 0, rst_n = 0;
    always #5 clk = ~clk;

    wire [IDW-1:0] awid; wire [AW-1:0] awaddr; wire [7:0] awlen; wire [2:0] awsize; wire [1:0] awburst;
    wire awvalid, awready;
    wire [DW-1:0] wdata; wire [DW/8-1:0] wstrb; wire wlast, wvalid, wready;
    wire [IDW-1:0] bid; wire [1:0] bresp; wire bvalid, bready;
    wire [IDW-1:0] arid; wire [AW-1:0] araddr; wire [7:0] arlen; wire [2:0] arsize; wire [1:0] arburst;
    wire arvalid, arready;
    wire [IDW-1:0] rid; wire [DW-1:0] rdata; wire [1:0] rresp; wire rlast, rvalid, rready;
    wire [31:0] o_n_done, o_busy, o_rerr;   // [*] must be declared explicitly (implicit net = 1 bit => the compare never holds => hang)

    axi_dpi_bridge #(.AW(AW), .IDW(IDW), .DW(DW)) dut (
        .clk(clk), .rst_n(rst_n),
        .m_awid(awid), .m_awaddr(awaddr), .m_awlen(awlen), .m_awsize(awsize), .m_awburst(awburst),
        .m_awvalid(awvalid), .m_awready(awready),
        .m_wdata(wdata), .m_wstrb(wstrb), .m_wlast(wlast), .m_wvalid(wvalid), .m_wready(wready),
        .m_bid(bid), .m_bresp(bresp), .m_bvalid(bvalid), .m_bready(bready),
        .m_arid(arid), .m_araddr(araddr), .m_arlen(arlen), .m_arsize(arsize), .m_arburst(arburst),
        .m_arvalid(arvalid), .m_arready(arready),
        .m_rid(rid), .m_rdata(rdata), .m_rresp(rresp), .m_rlast(rlast), .m_rvalid(rvalid), .m_rready(rready),
        .n_done(o_n_done), .n_busy_retry(o_busy), .n_rerr(o_rerr)
    );

    axi_slave_sim #(.AW(AW), .IDW(IDW), .DW(DW)) slave (
        .clk(clk), .rst_n(rst_n),
        .awid(awid), .awaddr(awaddr), .awlen(awlen), .awsize(awsize), .awburst(awburst),
        .awvalid(awvalid), .awready(awready),
        .wdata(wdata), .wstrb(wstrb), .wlast(wlast), .wvalid(wvalid), .wready(wready),
        .bid(bid), .bresp(bresp), .bvalid(bvalid), .bready(bready),
        .arid(arid), .araddr(araddr), .arlen(arlen), .arsize(arsize), .arburst(arburst),
        .arvalid(arvalid), .arready(arready),
        .rid(rid), .rdata(rdata), .rresp(rresp), .rlast(rlast), .rvalid(rvalid), .rready(rready)
    );

    integer errors = 0, rdata_err = 0;
    localparam [63:0] HBASE = 64'hA5A5_0000;

    // pattern (same formula independently as the C-side c_get_wdata)
    function automatic [DW-1:0] pat(input [63:0] h, input int beat);
        for (int i = 0; i < DW/8; i++) pat[i*8 +: 8] = (h[7:0] + beat*64 + i) & 8'hFF;
    endfunction

    task automatic chk(input bit cond, input string msg);
        if (cond) $display("  [PASS] %s", msg);
        else begin $display("  [FAIL] %s", msg); errors = errors + 1; end
    endtask

    // R-channel monitor: byte-by-byte check of read data claimed by RID
    always @(posedge clk) begin
        if (rst_n && rvalid && rready) begin
            automatic bit [DW-1:0] exp = pat(HBASE + (rid - 1), 0);
            if (rdata !== exp) rdata_err = rdata_err + 1;
        end
    end

    integer b, i, rc;
    bit [DW-1:0] exp;
    integer base;
    // watchdog: not finished after 20us => declare FAIL and exit (anti-hang; [X] "ran long enough" is not a criterion)
    initial begin
        #20000;
        $display("  [FAIL] watchdog: not finished after 20us");
        $display("TB_AXI_DPI_BRIDGE FAIL(watchdog)");
        $finish;
    end
    initial begin
        repeat (4) @(posedge clk);
        rst_n = 1;
        repeat (4) @(posedge clk);

        // -- (1) 4 writes (fill all 4 slots) --
        @(negedge clk);
        for (b = 0; b < 4; b++) begin
            rc = dut.rtl_axi_write(AW'(40'h1000 + b*64), 3'd6, 9'd1, IDW'(b+1), HBASE + b);
            chk(rc == 0, $sformatf("write #%0d accepted (slot %0d)", b, b));
        end
        // -- (2) the 5th => must be rejected ([*] outstanding=4 limit) --
        rc = dut.rtl_axi_write(AW'(40'h2000), 3'd6, 9'd1, IDW'(1), 64'hDEAD);
        chk(rc == 1, "5th write rejected ([*] outstanding=4 limit enforced)");

        wait (o_n_done == 32'd4);
        @(negedge clk);
        // -- (3) after the 4 writes: memory equals the pattern byte by byte --
        rdata_err = 0;
        for (b = 0; b < 4; b++) begin
            base = (40'h1000 + b*64) >> 6;
            exp = pat(HBASE + b, 0);
            for (i = 0; i < DW; i++)
                if (slave.mem[base][i] !== exp[i]) rdata_err = rdata_err + 1;
        end
        chk(rdata_err == 0, "4 writes: memory equals the pattern byte by byte (4x64B)");
        rdata_err = 0;

        // -- (4) 4 reads (same address, same handle => the C side can recompute independently) --
        for (b = 0; b < 4; b++) begin
            rc = dut.rtl_axi_read(AW'(40'h1000 + b*64), 3'd6, 9'd1, IDW'(b+1), HBASE + b);
            chk(rc == 0, $sformatf("read #%0d accepted", b));
        end
        wait (o_n_done == 32'd8);
        @(negedge clk);
        chk(rdata_err == 0, "4 reads: data claimed by RID, byte by byte (see the monitor)");
        chk(o_rerr == 32'd0, "no claim anomalies (n_rerr=0)");
        chk(o_busy == 32'd1, "slot-full count = 1 (only the 5th)");
        chk(dut.n_done == 32'd8, "completion count = 8 (4 writes + 4 reads)");

        if (errors == 0) $display("TB_AXI_DPI_BRIDGE PASS");
        else             $display("TB_AXI_DPI_BRIDGE FAIL(%0d)", errors);
        $finish;
    end
endmodule
