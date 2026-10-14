// ============================================================================
// axi_dpi_bridge.veo -- AXI4 <-> DPI bridge instantiation template (Verilog/VHDL instantiation body,
//   following the Xilinx .veo style)
//
// File information
//   module : axi_dpi_bridge
//   role   : AXI4 bridge between the TLM side (C++ DpiLink) and the RTL NoC; 4 slots in flight
//            (outstanding=4)
//   params : AW=40 | IDW=4 (AxID width, contract) | DW=512 (64B/beat) | OSLOT=4 (outstanding,
//            contract) | MAX_BEATS=64 (4KB)
//   clock  : single clock clk; reset rst_n (active low, synchronous release recommended)
//   DPI    : export rtl_axi_write / rtl_axi_read (C-side submit entry;
//            import c_get_wdata / c_put_rdata / c_axi_done (data and completion callbacks)
//            => pairs with model/rtl_link_dpi.h (C++ side DpiLink); elaboration reports an error when
//               the C library is missing
//
// Wiring
//   - AXI4 master port (m_*) => connect to the chip NoC's slave port (or to where the NoC master port
//     takes arbitration input)
//   - n_done / n_busy_retry / n_rerr => optional, connect to observation / coverage
//
// Usage: copy the contents between the INST_TAG markers below into your Verilog top level, then
//        rename the instance and the signal names
// ============================================================================

//----------- Begin Cut here for INSTANTIATION Template ---// INST_TAG

  axi_dpi_bridge #(
    .AW        (40),        // address width
    .IDW       (4),         // AxID width (contract = 4)
    .DW        (512),       // data width (64B/beat)
    .OSLOT     (4),         // number of outstanding slots (contract = 4)
    .MAX_BEATS (64)         // max beats per request (4KB)
  ) instance_name (
    // clock / reset
    .clk           (clk),              // input  wire          clk
    .rst_n         (rst_n),            // input  wire          rst_n
    // AXI4 write address channel
    .m_awid        (m_awid),           // output [IDW-1:0]     m_awid
    .m_awaddr      (m_awaddr),         // output [AW-1:0]      m_awaddr
    .m_awlen       (m_awlen),          // output [7:0]         m_awlen
    .m_awsize      (m_awsize),         // output [2:0]         m_awsize
    .m_awburst     (m_awburst),        // output [1:0]         m_awburst
    .m_awvalid     (m_awvalid),        // output               m_awvalid
    .m_awready     (m_awready),        // input                m_awready
    // AXI4 write data channel
    .m_wdata       (m_wdata),          // output [DW-1:0]      m_wdata
    .m_wstrb       (m_wstrb),          // output [DW/8-1:0]    m_wstrb
    .m_wlast       (m_wlast),          // output               m_wlast
    .m_wvalid      (m_wvalid),         // output               m_wvalid
    .m_wready      (m_wready),         // input                m_wready
    // AXI4 write response channel
    .m_bid         (m_bid),            // input  [IDW-1:0]     m_bid
    .m_bresp       (m_bresp),          // input  [1:0]         m_bresp
    .m_bvalid      (m_bvalid),         // input                m_bvalid
    .m_bready      (m_bready),         // output               m_bready
    // AXI4 read address channel
    .m_arid        (m_arid),           // output [IDW-1:0]     m_arid
    .m_araddr      (m_araddr),         // output [AW-1:0]      m_araddr
    .m_arlen       (m_arlen),          // output [7:0]         m_arlen
    .m_arsize      (m_arsize),         // output [2:0]         m_arsize
    .m_arburst     (m_arburst),        // output [1:0]         m_arburst
    .m_arvalid     (m_arvalid),        // output               m_arvalid
    .m_arready     (m_arready),        // input                m_arready
    // AXI4 read data channel
    .m_rid         (m_rid),            // input  [IDW-1:0]     m_rid
    .m_rdata       (m_rdata),          // input  [DW-1:0]      m_rdata
    .m_rresp       (m_rresp),          // input  [1:0]         m_rresp
    .m_rlast       (m_rlast),          // input                m_rlast
    .m_rvalid      (m_rvalid),         // input                m_rvalid
    .m_rready      (m_rready),         // output               m_rready
    // observation (optional)
    .n_done        (n_done),           // output [31:0]        completion count
    .n_busy_retry  (n_busy_retry),     // output [31:0]        slot-full retry count
    .n_rerr        (n_rerr)            // output [31:0]        claim-anomaly count
  );

// INST_TAG_END ------ End INSTANTIATION Template ---------
