// ============================================================================
// axi_dpi_bridge.veo -- instantiation template (Xilinx .veo style)
//
//   module : axi_dpi_bridge          <-- THIS is the file a customer instantiates
//                                      to put an onetest model on their NoC
//   role   : AXI4 <-> DPI bridge between the onetest Linux-proxy model (C side)
//            and the RTL NoC.  ONE bridge for every line (pcie/usb/storage/
//            network/smmu/nvme) -- same ports, same parameters, same counters;
//            the DPI symbol prefix is the ONLY per-line difference
//            (this copy: usb_axi_submit / c_usb_*).
//            m_*     = AXI4 master (model issues register/config/memory traffic)
//            s_*     = AXI4 slave  (the DUT's posted writes: DMA write-back, MSI)
//            irq_out = ACTIVE-HIGH LEVEL interrupt OUT (model -> the SoC
//                      interrupt ctrl; pcie = MSI/MSI-X level). Host-role lines
//                      (usb/storage/network/smmu) tie it to 1'b0.
//            irq_in  = ACTIVE-HIGH LEVEL interrupt IN (the DUT -> the model),
//                      pushed to the C side on value change; pcie ties 1'b0.
//   params : AW=64 | IDW=4 (AxID width, contract) | DW=128 (contract) |
//            OSLOT=8 (outstanding, contract) | MAX_BEATS=256 (4 KB / 16 B)
//   clock  : single clock clk; reset rst_n (active low)
//   DPI    : export usb_axi_submit (C side submits; full => returns 1 => retry)
//            imports c_usb_wdata / c_usb_rdata / c_usb_done / c_usb_slv_wbeat /
//                    c_usb_slv_rbeat / c_usb_slv_bresp /
//                    c_usb_irq_level / c_usb_irq_set
//            => provided by the line's rtl_bridge/pcie_axi_dpi_link.c;
//               for a stand-alone SV-only run link dpi_stubs.c instead.
//
//   Wiring
//     - m_*    -> the NoC's slave port (model = a bus master on the NoC)
//     - s_*    -> the NoC's master port (the DUT reaches host memory / MSI this way)
//     - irq_out-> the SoC's interrupt controller input (level, active high)
//     - irq_in <- the DUT's interrupt output (level, active high)
//     - n_done / n_busy_retry / n_rerr / n_slv_recv / n_slv_miss -> observation
//   Prototype scope: slave-port READ is served as a single beat (see the R path);
//   write bursts up to MAX_BEATS beats are routed per beat.
// ============================================================================
  axi_dpi_bridge #(
    .AW        (64 ),      // 64-bit addresses
    .IDW       (4  ),      // [*] AxID width = 4
    .DW        (128),      // [*] data width = 128 bit
    .OSLOT     (8  ),      // [*] outstanding = 8
    .MAX_BEATS (256)       // 4 KB / 16 B
  ) u_pcie_bridge (
    .clk         (clk                        ),
    .rst_n       (rst_n                      ),
    // AXI4 master (model -> NoC)
    .m_awid      (                           ), .m_awaddr (                  ),
    .m_awlen     (                           ), .m_awsize (                  ),
    .m_awburst   (                           ), .m_awvalid(                  ), .m_awready( ),
    .m_wdata     (                           ), .m_wstrb  (                  ),
    .m_wlast     (                           ), .m_wvalid (                  ), .m_wready ( ),
    .m_bid       (                           ), .m_bresp  (                  ),
    .m_bvalid    (                           ), .m_bready (                  ),
    .m_arid      (                           ), .m_araddr (                  ),
    .m_arlen     (                           ), .m_arsize (                  ),
    .m_arburst   (                           ), .m_arvalid(                  ), .m_arready( ),
    .m_rid       (                           ), .m_rdata  (                  ),
    .m_rresp     (                           ), .m_rlast  (                  ),
    .m_rvalid    (                           ), .m_rready (                  ),
    // AXI4 slave (NoC -> model)
    .s_awid      (                           ), .s_awaddr (                  ),
    .s_awlen     (                           ), .s_awsize (                  ),
    .s_awburst   (                           ), .s_awvalid(                  ), .s_awready( ),
    .s_wdata     (                           ), .s_wstrb  (                  ),
    .s_wlast     (                           ), .s_wvalid (                  ), .s_wready ( ),
    .s_bid       (                           ), .s_bresp  (                  ),
    .s_bvalid    (                           ), .s_bready (                  ),
    .s_arid      (                           ), .s_araddr (                  ),
    .s_arlen     (                           ), .s_arsize (                  ),
    .s_arburst   (                           ), .s_arvalid(                  ), .s_arready( ),
    .s_rid       (                           ), .s_rdata  (                  ),
    .s_rresp     (                           ), .s_rlast  (                  ),
    .s_rvalid    (                           ), .s_rready (                  ),
    // interrupts (active high level) + observation
    //   irq_out: model -> SoC (pcie = MSI level; host-role lines tie 1'b0)
    //   irq_in : DUT -> model (host-role lines wire the DUT's line; pcie ties 1'b0)
    .irq_out     (irq_out_pcie               ),
    .irq_in      (irq_in_pcie                ),
    .n_done      (                           ), .n_busy_retry(               ), .n_rerr( ),
    .n_slv_recv  (                           ), .n_slv_miss  (               )
  );
