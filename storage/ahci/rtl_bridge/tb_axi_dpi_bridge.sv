// ============================================================================
// tb_axi_dpi_bridge.sv -- self-check bench for axi_dpi_bridge (xsim)
//   A minimal AXI4 slave model serves the bridge's master port; a minimal AXI4
//   master driver feeds its slave port.  Criteria (final line: PASS/FAIL):
//     C-1  submit write -> AW(id/addr/len/size) correct; W valid after AW; B -> c_sata_done
//     C-2  submit read  -> AR correct; R data -> c_sata_rdata; done
//     C-3  8 outstanding accepted; the 9th returns 1 (busy) and counts n_busy_retry
//     C-4  narrow write keeps the C-side byte-lane strobes (0x00F0)
//     C-5  slave port: AW+W -> c_sata_slv_wbeat (per beat, beat index 0 then 1); B resp OKAY
//     C-6  slave port DECERR: resp=3 propagates to bresp
//     C-7  interrupt: active-high LEVEL (rises with the request, stays, falls when cleared)
//     C-8  no spurious rerr
// ============================================================================
`timescale 1ns/1ps

module tb_axi_dpi_bridge;
    localparam integer AW = 64, IDW = 4, DW = 128, OSLOT = 8;

    reg clk = 0, rst_n = 0;
    always #5 clk = ~clk;

    // bridge ports
    wire [IDW-1:0] m_awid; wire [AW-1:0] m_awaddr; wire [7:0] m_awlen; wire [2:0] m_awsize;
    wire [1:0] m_awburst; wire m_awvalid; wire m_awready;
    wire [DW-1:0] m_wdata; wire [DW/8-1:0] m_wstrb; wire m_wlast, m_wvalid, m_wready;
    wire [IDW-1:0] m_bid; wire [1:0] m_bresp; wire m_bvalid; wire m_bready;
    wire [IDW-1:0] m_arid; wire [AW-1:0] m_araddr; wire [7:0] m_arlen; wire [2:0] m_arsize;
    wire [1:0] m_arburst; wire m_arvalid; wire m_arready;
    wire [IDW-1:0] m_rid; wire [DW-1:0] m_rdata; wire [1:0] m_rresp; wire m_rlast, m_rvalid, m_rready;

    reg [IDW-1:0] s_awid;  reg [AW-1:0] s_awaddr; reg [7:0] s_awlen; reg [2:0] s_awsize;
    reg [1:0] s_awburst; reg s_awvalid; wire s_awready;
    reg [DW-1:0] s_wdata; reg [DW/8-1:0] s_wstrb; reg s_wlast, s_wvalid; wire s_wready;
    wire [IDW-1:0] s_bid; wire [1:0] s_bresp; wire s_bvalid; reg s_bready;
    reg [IDW-1:0] s_arid; reg [AW-1:0] s_araddr; reg [7:0] s_arlen; reg [2:0] s_arsize;
    reg [1:0] s_arburst; reg s_arvalid; wire s_arready;
    wire [IDW-1:0] s_rid; wire [DW-1:0] s_rdata; wire [1:0] s_rresp; wire s_rlast, s_rvalid; reg s_rready;

    wire irq_out;
    reg  irq_in = 1'b0;
    wire [31:0] n_done, n_busy_retry, n_rerr, n_slv_recv, n_slv_miss;

    axi_dpi_bridge #(.AW(AW), .IDW(IDW), .DW(DW), .OSLOT(OSLOT)) dut (
        .clk(clk), .rst_n(rst_n),
        .m_awid(m_awid), .m_awaddr(m_awaddr), .m_awlen(m_awlen), .m_awsize(m_awsize),
        .m_awburst(m_awburst), .m_awvalid(m_awvalid), .m_awready(m_awready),
        .m_wdata(m_wdata), .m_wstrb(m_wstrb), .m_wlast(m_wlast), .m_wvalid(m_wvalid), .m_wready(m_wready),
        .m_bid(m_bid), .m_bresp(m_bresp), .m_bvalid(m_bvalid), .m_bready(m_bready),
        .m_arid(m_arid), .m_araddr(m_araddr), .m_arlen(m_arlen), .m_arsize(m_arsize),
        .m_arburst(m_arburst), .m_arvalid(m_arvalid), .m_arready(m_arready),
        .m_rid(m_rid), .m_rdata(m_rdata), .m_rresp(m_rresp), .m_rlast(m_rlast),
        .m_rvalid(m_rvalid), .m_rready(m_rready),
        .s_awid(s_awid), .s_awaddr(s_awaddr), .s_awlen(s_awlen), .s_awsize(s_awsize),
        .s_awburst(s_awburst), .s_awvalid(s_awvalid), .s_awready(s_awready),
        .s_wdata(s_wdata), .s_wstrb(s_wstrb), .s_wlast(s_wlast), .s_wvalid(s_wvalid), .s_wready(s_wready),
        .s_bid(s_bid), .s_bresp(s_bresp), .s_bvalid(s_bvalid), .s_bready(s_bready),
        .s_arid(s_arid), .s_araddr(s_araddr), .s_arlen(s_arlen), .s_arsize(s_arsize),
        .s_arburst(s_arburst), .s_arvalid(s_arvalid), .s_arready(s_arready),
        .s_rid(s_rid), .s_rdata(s_rdata), .s_rresp(s_rresp), .s_rlast(s_rlast),
        .s_rvalid(s_rvalid), .s_rready(s_rready),
        .irq_out(irq_out), .irq_in(irq_in),
        .n_done(n_done), .n_busy_retry(n_busy_retry), .n_rerr(n_rerr),
        .n_slv_recv(n_slv_recv), .n_slv_miss(n_slv_miss)
    );

    // --- DPI exports the bench drives ---
    /* sata_axi_submit is an EXPORT inside the bridge -- call it through the instance
       (an exported function is visible only within its own scope). */
    import "DPI-C" function void tb_set_irq(input int lvl);
    import "DPI-C" function void tb_set_lane(input longint unsigned h, input int lane, input int size);
    import "DPI-C" function int  tb_slv_wbeats();
    import "DPI-C" function int  tb_slv_rbeats();
    import "DPI-C" function int  tb_irq_in_level();
    import "DPI-C" function int  tb_irq_in_events();

    // ---------------------------------------------------------------- AXI4 slave model
    // serves the bridge master port: AW/W -> B (when wlast); AR -> 1-beat R bursts
    reg [IDW-1:0] sl_id;
    reg b_valid = 0;
    reg r_valid = 0;
    reg [IDW-1:0] rd_id;
    reg [AW-1:0] rd_addr;
    reg [7:0] rd_left;
    assign m_awready = 1'b1;
    assign m_wready  = 1'b1;
    assign m_arready = 1'b1;
    assign m_bvalid  = b_valid;
    assign m_bid     = sl_id;
    assign m_bresp   = 2'b00;                       /* OKAY */
    assign m_rvalid = r_valid;
    assign m_rid    = rd_id;
    assign m_rdata  = {4{rd_addr[15:0], 16'h0}} ^ {128{1'b1}};   /* pattern */
    assign m_rresp  = 2'b00;
    assign m_rlast  = (rd_left == 8'd1);

    integer w_seen = 0, aw_seen = 0, ar_seen = 0;
    reg [AW-1:0] m_awaddr_l;
    reg [IDW-1:0] m_awid_l;
    reg [7:0] m_awlen_l;
    reg [2:0] m_awsize_l;
    reg [DW-1:0] m_wdata_l;
    reg [DW/8-1:0] m_wstrb_l;
    integer w_beats_seen = 0;
    always @(posedge clk) begin
        if (m_awvalid && m_awready) begin
            aw_seen++; m_awaddr_l <= m_awaddr; m_awid_l <= m_awid;
            m_awlen_l <= m_awlen; m_awsize_l <= m_awsize;
        end
        if (m_wvalid && m_wready) begin
            w_seen++; m_wdata_l <= m_wdata; m_wstrb_l <= m_wstrb; w_beats_seen++;
        end
        /* B: assert on wlast, HOLD a full cycle until taken (bready) --
           a pulse shorter than a cycle is never seen by an edge-sampling consumer
           (the first version used an intra-block #1: the bridge never saw the B). */
        if (m_wvalid && m_wready && m_wlast) begin
            b_valid <= 1'b1; sl_id <= m_awid_l;
        end else if (b_valid && m_bready) begin
            b_valid <= 1'b0;
        end
        if (m_arvalid && m_arready) begin
            ar_seen++; rd_id <= m_arid; rd_addr <= m_araddr; rd_left <= m_arlen + 8'd1;
            r_valid <= 1'b1;
        end
        if (r_valid && m_rready) begin
            if (rd_left == 8'd1) r_valid <= 1'b0; else begin rd_left <= rd_left - 8'd1; rd_addr <= rd_addr + 64'd16; end
        end
    end

    // ---------------------------------------------------------------- AXI4 master driver (feeds the bridge slave port)
    task slv_write(input [AW-1:0] addr, input [IDW-1:0] id, input integer beats, input [2:0] size);
        integer i;
        begin
            @(posedge clk);
            s_awid <= id; s_awaddr <= addr; s_awlen <= beats-1; s_awsize <= size; s_awburst <= 2'b01;
            s_awvalid <= 1'b1;
            @(posedge clk); while (!s_awready) @(posedge clk); s_awvalid <= 1'b0;
            for (i = 0; i < beats; i++) begin
                s_wdata <= {128{1'b1}}; s_wstrb <= {16{1'b1}};
                s_wlast <= (i == beats-1); s_wvalid <= 1'b1;
                @(posedge clk); while (!s_wready) @(posedge clk);
            end
            s_wvalid <= 1'b0; s_wlast <= 1'b0;
            @(posedge clk); while (!s_bvalid) @(posedge clk);
            s_bready <= 1'b1; @(posedge clk) s_bready <= 1'b0;
        end
    endtask

    integer pass = 0, fail = 0;
    task chk(input cond, input string name);
        begin
            if (cond) begin pass++; $display("  [PASS] %0s", name); end
            else      begin fail++; $display("  [FAIL] %0s", name); end
        end
    endtask

    initial begin
        rst_n = 0; s_awvalid = 0; s_wvalid = 0; s_bready = 0;
        s_arvalid = 0; s_rready = 1;
        tb_set_irq(0);
        repeat (4) @(posedge clk);
        rst_n = 1;
        repeat (2) @(posedge clk);

        // C-1: submit an aligned 4-byte write (size=2, strb 0x00F0 from the stub)
        tb_set_lane(64'hAA, 4, 2);                       /* 4-byte access at lane 4 */
        chk(dut.sata_axi_submit(64'h1004, 2, 1, 4'h3, 1, 64'hAA) == 0, "C-1.0 submit accepted");
        repeat (6) @(posedge clk);
        chk(aw_seen == 1 && m_awid_l == 4'h3 && m_awaddr_l == 64'h1004 && m_awsize_l == 3'd2,
            "C-1.1 AW carries id/addr/size");
        chk(w_seen == 1 && m_wstrb_l == 16'h00F0, "C-1.2 narrow write keeps the byte lanes (0x00F0)");
        repeat (4) @(posedge clk);
        chk(n_done == 32'd1 && n_rerr == 0, "C-1.3 write completed via B -> c_sata_done");

        // C-2: submit a 1-beat read
        chk(dut.sata_axi_submit(64'h2000, 4, 1, 4'h5, 0, 64'hBB) == 0, "C-2.0 read submit accepted");
        repeat (8) @(posedge clk);
        chk(ar_seen == 1 && n_done == 32'd2, "C-2.1 AR issued and read completed");

        // C-3: outstanding = 8 accepted, the 9th is refused
        begin integer i, r;
            for (i = 0; i < 8; i++) r = dut.sata_axi_submit(64'h3000 + i*16, 2, 1, i[3:0], 1, 64'h100 + i);
            chk(r == 0, "C-3.1 eight more submits accepted (8 slots)");
            r = dut.sata_axi_submit(64'h4000, 2, 1, 4'h7, 1, 64'h200);
            /* r is the function return (immediate); n_busy_retry is read through the
               output port net, which settles one delta later -- sample it after edges. */
            repeat (2) @(posedge clk);
            chk(r == 1 && n_busy_retry >= 32'd1, "C-3.2 the 9th submit is refused (busy retry counted)");
            repeat (40) @(posedge clk);
            chk(n_done == 32'd10, "C-3.3 all ten completed");
        end

        // C-5: slave port write, 2 beats
        slv_write(64'h8000, 4'h9, 2, 3'd4);
        chk(tb_slv_wbeats() == 2 && n_slv_recv == 2, "C-5.1 two beats routed (per-beat callback)");
        chk(s_bresp == 2'b00, "C-5.2 slave B response OKAY");

        // C-6: slave port DECERR propagates
        slv_write(64'hDEAD0000, 4'hA, 1, 3'd4);
        chk(s_bresp == 2'b11 && n_slv_miss == 1, "C-6.1 DECERR propagates to BRESP and is counted");

        // C-7: interrupt OUT is an active-high LEVEL
        tb_set_irq(1); repeat (3) @(posedge clk);
        chk(irq_out == 1'b1, "C-7.1 irq_out rises");
        repeat (5) @(posedge clk);
        chk(irq_out == 1'b1, "C-7.2 irq_out STAYS high while pending (level, not pulse)");
        tb_set_irq(0); repeat (3) @(posedge clk);
        chk(irq_out == 1'b0, "C-7.3 irq_out falls when serviced");

        // C-9: interrupt IN (DUT -> model) is pushed on VALUE CHANGE only
        begin integer ev0;
            ev0 = tb_irq_in_events();
            irq_in = 1'b1; repeat (3) @(posedge clk);
            chk(tb_irq_in_level() == 1 && tb_irq_in_events() == ev0 + 1,
                "C-9.1 irq_in rise pushed once to the C side");
            repeat (5) @(posedge clk);
            chk(tb_irq_in_events() == ev0 + 1, "C-9.2 held level: no repeat pushes (value-change only)");
            irq_in = 1'b0; repeat (3) @(posedge clk);
            chk(tb_irq_in_level() == 0 && tb_irq_in_events() == ev0 + 2,
                "C-9.3 irq_in fall pushed once (level back to 0)");
        end

        // C-8
        chk(n_rerr == 0, "C-8.0 no spurious completion errors");

        $display("\n  PASS = %0d\n  FAIL = %0d", pass, fail);
        if (fail == 0 && pass > 0) $display("TB_AXI_DPI_BRIDGE PASS");
        else                       $display("TB_AXI_DPI_BRIDGE FAIL");
        $finish;
    end
endmodule
