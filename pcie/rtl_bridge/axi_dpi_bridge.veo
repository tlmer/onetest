// ============================================================================
// axi_dpi_bridge.veo -- instantiation template (Xilinx .veo style)
//
//   module : axi_dpi_bridge          <-- THIS is the file a customer instantiates
//                                      to put the PCIe host model on their NoC
//   role   : AXI4 <-> DPI bridge between the PCIe root-complex model (C side,
//            model/axi4_tlm.h contract) and the RTL NoC.
//            m_* = AXI4 master (model issues register/config/memory traffic)
//            s_* = AXI4 slave  (the DUT's posted writes: DMA write-back, MSI)
//            irq = ACTIVE-HIGH LEVEL interrupt out (to the SoC interrupt ctrl)
//   params : AW=64 | IDW=4 (AxID width, contract) | DW=128 (contract) |
//            OSLOT=8 (outstanding, contract) | MAX_BEATS=256 (4 KB / 16 B)
//   clock  : single clock clk; reset rst_n (active low)
//   DPI    : export pcie_axi_submit (C side submits; full => returns 1 => retry)
//            imports c_pcie_wdata / c_pcie_rdata / c_pcie_done / c_pcie_slv_wbeat /
//                    c_pcie_slv_rbeat / c_pcie_slv_bresp / c_pcie_irq_level
//            => provided by the model side (pcie/rtl_bridge/pcie_axi_dpi_link.c);
//               for a stand-alone SV-only run link dpi_stubs.c instead.
//
//   Wiring
//     - m_*  -> the NoC's slave port (model = a bus master on the NoC)
//     - s_*  -> the NoC's master port (the DUT reaches host memory / MSI this way)
//     - irq  -> the SoC's interrupt controller input (level, active high)
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
    // interrupt (active high level) + observation
    .irq         (irq_pcie                   ),
    .n_done      (                           ), .n_busy_retry(               ), .n_rerr( ),
    .n_slv_recv  (                           ), .n_slv_miss  (               )
  );
