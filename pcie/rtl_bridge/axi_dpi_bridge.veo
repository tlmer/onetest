

  axi_dpi_bridge #(
    .AW        (64 ),      
    .IDW       (4  ),      
    .DW        (128),      
    .OSLOT     (8  ),      
    .MAX_BEATS (256),      
    .IRQ_VEC_BITS (1)      
  ) u_pcie_bridge (
    .clk         (clk                        ),
    .rst_n       (rst_n                      ),
    
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
    
    
    
    .irq_out     (irq_out_pcie               ),
    .irq_vec_out (irq_vec_out_pcie           ),
    .irq_in      (irq_in_pcie                ),
    .n_done      (                           ), .n_busy_retry(               ), .n_rerr( ),
    .n_slv_recv  (                           ), .n_slv_miss  (               )
  );
