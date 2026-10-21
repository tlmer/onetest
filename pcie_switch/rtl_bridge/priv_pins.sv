

module priv_dut (
    input  wire clk,
    input  wire rst_n,
    input  wire client0_addr_align_en,
    input  wire [7:0] client0_tlp_byte_en,
    input  wire [15:0] client0_remote_req_id,
    input  wire [11:0] client0_cpl_byte_cnt,
    input  wire [2:0] client0_tlp_tc,
    input  wire [1:0] client0_tlp_attr,
    input  wire [2:0] client0_cpl_status,
    input  wire client0_cpl_bcm,
    input  wire client0_tlp_dv,
    input  wire client0_tlp_eot,
    input  wire client0_tlp_bad_eot,
    input  wire client0_tlp_hv,
    input  wire [1:0] client0_tlp_fmt,
    input  wire [4:0] client0_tlp_type,
    input  wire client0_tlp_td,
    input  wire client0_tlp_ep,
    input  wire [12:0] client0_tlp_byte_len,
    input  wire [63:0] client0_tlp_addr,
    input  wire [7:0] client0_tlp_tid,
    input  wire [63:0] client0_tlp_data,
    input  wire [2:0] client0_tlp_func_num,
    input  wire [15:0] client0_app_dev_id,
    input  wire [14:0] client0_hdr_rsvd,
    input  wire client1_addr_align_en,
    input  wire [7:0] client1_tlp_byte_en,
    input  wire [15:0] client1_remote_req_id,
    input  wire [11:0] client1_cpl_byte_cnt,
    input  wire [2:0] client1_tlp_tc,
    input  wire [1:0] client1_tlp_attr,
    input  wire [2:0] client1_cpl_status,
    input  wire client1_cpl_bcm,
    input  wire client1_tlp_dv,
    input  wire client1_tlp_eot,
    input  wire client1_tlp_bad_eot,
    input  wire client1_tlp_hv,
    input  wire [1:0] client1_tlp_fmt,
    input  wire [4:0] client1_tlp_type,
    input  wire client1_tlp_td,
    input  wire client1_tlp_ep,
    input  wire [12:0] client1_tlp_byte_len,
    input  wire [63:0] client1_tlp_addr,
    input  wire [7:0] client1_tlp_tid,
    input  wire [63:0] client1_tlp_data,
    input  wire [2:0] client1_tlp_func_num,
    input  wire [15:0] client1_app_dev_id,
    input  wire [14:0] client1_hdr_rsvd,
    input  wire trgt1_radm_halt,
    input  wire [2:0] trgt1_radm_pkt_halt,
    input  wire [31:0] dbi_addr,
    input  wire [31:0] dbi_din,
    input  wire dbi_cs,
    input  wire dbi_cs2,
    input  wire [3:0] dbi_wr,
    input  wire ven_msi_req,
    input  wire [2:0] ven_msi_func_num,
    input  wire [2:0] ven_msi_tc,
    input  wire [4:0] ven_msi_vector,
    input  wire [3:0] device_type,
    input  wire xal_xmt_cpl_ca,
    input  wire xal_rcvd_cpl_ca,
    input  wire xal_rcvd_cpl_ur,
    output wire xadm_client0_halt,
    output wire xadm_client1_halt,
    output wire radm_trgt1_dv,
    output wire radm_trgt1_hv,
    output wire radm_trgt1_eot,
    output wire radm_trgt1_tlp_abort,
    output wire radm_trgt1_dllp_abort,
    output wire radm_trgt1_ecrc_err,
    output wire [1:0] radm_trgt1_fmt,
    output wire [4:0] radm_trgt1_type,
    output wire [2:0] radm_trgt1_tc,
    output wire [1:0] radm_trgt1_attr,
    output wire [15:0] radm_trgt1_reqid,
    output wire [7:0] radm_trgt1_tag,
    output wire [2:0] radm_trgt1_func_num,
    output wire radm_trgt1_td,
    output wire radm_trgt1_poisoned,
    output wire [9:0] radm_trgt1_dw_len,
    output wire [3:0] radm_trgt1_first_be,
    output wire [3:0] radm_trgt1_last_be,
    output wire [63:0] radm_trgt1_addr,
    output wire radm_trgt1_bcm,
    output wire radm_trgt1_cpl_last,
    output wire [15:0] radm_trgt1_cmpltr_id,
    output wire [11:0] radm_trgt1_byte_cnt,
    output wire [2:0] radm_trgt1_cpl_status,
    output wire [14:0] radm_trgt1_rsvd,
    output wire [63:0] radm_trgt1_data,
    output wire [1:0] radm_trgt1_dwen,
    output wire [63:0] radm_trgt1_hdr_uppr_bytes,
    output wire [2:0] radm_trgt1_vc,
    output wire [2:0] radm_grant_tlp_type,
    output wire [63:0] radm_bypass_data,
    output wire [1:0] radm_bypass_dwen,
    output wire radm_bypass_dv,
    output wire radm_bypass_hv,
    output wire radm_bypass_eot,
    output wire radm_bypass_tlp_abort,
    output wire radm_bypass_dllp_abort,
    output wire radm_bypass_ecrc_err,
    output wire [1:0] radm_bypass_fmt,
    output wire [4:0] radm_bypass_type,
    output wire [2:0] radm_bypass_tc,
    output wire [1:0] radm_bypass_attr,
    output wire [15:0] radm_bypass_reqid,
    output wire [7:0] radm_bypass_tag,
    output wire [2:0] radm_bypass_func_num,
    output wire radm_bypass_td,
    output wire radm_bypass_poisoned,
    output wire [9:0] radm_bypass_dw_len,
    output wire [3:0] radm_bypass_first_be,
    output wire [3:0] radm_bypass_last_be,
    output wire [63:0] radm_bypass_addr,
    output wire radm_bypass_bcm,
    output wire radm_bypass_cpl_last,
    output wire [15:0] radm_bypass_cmpltr_id,
    output wire [11:0] radm_bypass_byte_cnt,
    output wire [2:0] radm_bypass_cpl_status,
    output wire [14:0] radm_bypass_rsvd,
    output wire lbc_dbi_ack,
    output wire [31:0] lbc_dbi_dout,
    output wire ven_msi_grant,
    output wire cfg_msi_en,
    output wire [7:0] cfg_pbus_num,
    output wire [4:0] cfg_pbus_dev_num,
    output wire [7:0] cfg_2ndbus_num,
    output wire [7:0] cfg_subbus_num,
    output wire [15:0] cfg_mem_base,
    output wire [15:0] cfg_mem_limit,
    output wire upstream_port,
    output wire rdlh_link_up,
    output wire cfg_bridge_crs_en,
    output wire radm_correctable_err,
    output wire radm_nonfatal_err,
    output wire radm_fatal_err
);

    
    localparam integer CDWB = 64;   
    localparam integer TDWB = 64;   
    localparam integer ADWB = 64;   
    localparam integer TGW  = 8;   
    localparam integer RQW  = 16;   
    localparam integer FNW  = 3;   
    localparam integer BEW  = 8;   
    localparam integer BLW  = 13;   
    localparam integer DWW  = 10;   
    localparam integer BCW  = 12;   
    localparam integer RSW  = 15;   

    localparam [4:0] T_CFG0 = 5'h04, T_CFG1 = 5'h05, T_CPL = 5'ha, T_MEM = 5'h00;
    localparam [15:0] EP_VID = 16'h10EE, EP_DID = 16'h9024, EP_BDF = 16'h0100;
    localparam [15:0] DN_VID = 16'h10EE, DN_DID = 16'h9002;   
    localparam [31:0] EP_BAR0_MASK = 32'hFFFF_0000;     
    localparam integer EP_MEM_B = 65536;                
    localparam integer TX_MAXB  = 64;                   

    reg [3:0]  dbg_errs;
    reg [15:0] dbg_frames, dbg_beats;
    reg [15:0] cfg_ops, memwrs, memrds, dbi_acks, dbi_aborts, tx_drops, dbi_elbi;
    reg [RSW-1:0] dbg_snap;
    reg [7:0] ep_mem [0:EP_MEM_B-1];
    integer mi;
    initial for (mi = 0; mi < EP_MEM_B; mi = mi + 1) ep_mem[mi] = 8'hA5;   

    
    reg [31:0] ep_bar0; reg ep_bar0_sz;
    reg [15:0] ep_cmd, ep_msi_ctrl, ep_msi_data;
    reg [31:0] ep_msi_addr;

    
    reg [31:0] cdm_04, cdm_18, cdm_1c, cdm_20, cdm_24, cdm_28, cdm_2c;

    
    reg rx_active, disp_pend; reg [3:0] rx_beat; reg [511:0] rx_pay;
    reg [4:0] h_type; reg [1:0] h_fmt; reg [ADWB-1:0] h_addr;
    reg [TGW-1:0] h_tag; reg [FNW-1:0] h_fn; reg [RQW-1:0] h_reqid;
    reg [BLW-1:0] h_blen; reg [BEW-1:0] h_be;

    
    reg [1:0] tx_st; reg [3:0] tx_k;
    reg [4:0] t_type; reg [1:0] t_fmt; reg [ADWB-1:0] t_addr;
    reg [TGW-1:0] t_tag; reg [FNW-1:0] t_fn; reg [RQW-1:0] t_reqid;
    reg [DWW-1:0] t_dwl; reg [BCW-1:0] t_bcnt; reg [2:0] t_status;
    reg [511:0] t_pay; reg [7:0] t_nbytes; reg [3:0] t_nbeats;

    
    reg [1:0] dbi_st; reg dbi_n, dbi_cs_r; reg [31:0] dbi_ra, dbi_rd, dbi_dout_r; reg [3:0] dbi_rw;

    
    function [31:0] be_merge; input [31:0] oldv; input [31:0] val; input [3:0] be;
        reg [31:0] r; integer b;
        begin
            r = oldv;
            for (b = 0; b < 4; b = b + 1)
                if (be[b]) r[b*8 +: 8] = val[b*8 +: 8];
            be_merge = r;
        end
    endfunction

    
    function [31:0] cdm_read; input [7:0] a;
        begin
            case (a[7:2])
              6'h00: cdm_read = {DN_DID, DN_VID};          
              6'h01: cdm_read = {16'h0000, cdm_04[15:0]};  
              6'h02: cdm_read = 32'h0604_0001;             
              6'h03: cdm_read = 32'h0001_0000;             
              6'h06: cdm_read = cdm_18;                    
              6'h07: cdm_read = cdm_1c;                    
              6'h08: cdm_read = cdm_20;                    
              6'h09: cdm_read = cdm_24;                    
              6'h0a: cdm_read = cdm_28;                    
              6'h0b: cdm_read = cdm_2c;                    
              6'h0d: cdm_read = 32'h0000_0040;             
              6'h0f: cdm_read = 32'h0000_0100;             
              
              6'h20: cdm_read = {16'h0, dbg_frames};       
              6'h21: cdm_read = {16'h0, dbg_beats};        
              6'h22: cdm_read = {16'h0, cfg_ops};          
              6'h23: cdm_read = {16'h0, memwrs};           
              6'h24: cdm_read = {16'h0, memrds};           
              6'h25: cdm_read = {16'h0, dbi_acks};         
              6'h26: cdm_read = {16'h0, dbi_aborts};       
              6'h27: cdm_read = {16'h0, tx_drops};         
              6'h28: cdm_read = {28'h0, dbg_errs};         
              6'h29: cdm_read = {16'h0, dbi_elbi};         
              default: cdm_read = 32'h0;
            endcase
        end
    endfunction

    
    function [31:0] ep_read; input [7:0] a;
        begin
            case (a[7:2])
              6'h00: ep_read = {EP_DID, EP_VID};                        
              6'h01: ep_read = {16'h0000, ep_cmd};                      
              6'h02: ep_read = 32'h0108_0201;                           
              6'h03: ep_read = 32'h0000_0000;                           
              6'h04: ep_read = ep_bar0_sz ? EP_BAR0_MASK                
                                          : (ep_bar0 & EP_BAR0_MASK);
              6'h0d: ep_read = 32'h0000_0040;                           
              6'h0f: ep_read = 32'h0000_0100;                           
              6'h10: ep_read = 32'h0001_1050;                           
              6'h11: ep_read = 32'h0000_0001;                           
              6'h12: ep_read = 32'h0000_0000;                           
              6'h13: ep_read = 32'h0001_0001;                           
              6'h14: ep_read = {8'h60, 8'h05, ep_msi_ctrl};             
              6'h15: ep_read = ep_msi_addr;                             
              6'h16: ep_read = 32'h0;                                   
              6'h17: ep_read = {16'h0, ep_msi_data};                    
              default: ep_read = 32'h0;                                 
            endcase
        end
    endfunction

    
    task ep_write; input [7:0] a; input [31:0] din; input [3:0] be;
        reg [31:0] m;
        begin
            case (a[7:2])
              6'h01: begin m = be_merge({16'h0, ep_cmd}, din, be); ep_cmd <= m[15:0]; end
              6'h04: begin
                        m = be_merge(ep_bar0_sz ? EP_BAR0_MASK : (ep_bar0 & EP_BAR0_MASK), din, be);
                        if (m == 32'hFFFF_FFFF) ep_bar0_sz <= 1'b1;
                        else begin ep_bar0_sz <= 1'b0; ep_bar0 <= m & EP_BAR0_MASK; end
                     end
              6'h14: begin m = be_merge({16'h0, ep_msi_ctrl}, din, be); ep_msi_ctrl <= m[15:0]; end
              6'h15: begin m = be_merge(ep_msi_addr, din, be); ep_msi_addr <= m; end
              6'h17: begin m = be_merge({16'h0, ep_msi_data}, din, be); ep_msi_data <= m[15:0]; end
              default: ;
            endcase
        end
    endtask

    
    task cdm_commit; input [31:0] addr; input [31:0] din; input [3:0] be;
        begin
            case (addr[7:2])
              6'h01: cdm_04 <= be_merge(cdm_04, din, be);
              6'h06: cdm_18 <= be_merge(cdm_18, din, be);
              6'h07: cdm_1c <= be_merge(cdm_1c, din, be);
              6'h08: cdm_20 <= be_merge(cdm_20, din, be);
              6'h09: cdm_24 <= be_merge(cdm_24, din, be);
              6'h0a: cdm_28 <= be_merge(cdm_28, din, be);
              6'h0b: cdm_2c <= be_merge(cdm_2c, din, be);
              default: ;
            endcase
        end
    endtask

    
    function in_bar0; input [ADWB-1:0] a;
        begin
            in_bar0 = (a >= {32'h0, ep_bar0}) && (a < ({32'h0, ep_bar0} + 64'd65536));
        end
    endfunction

    function [3:0] dw_en; input integer dw; input integer dw_len; input [7:0] be;
        begin
            dw_en = (dw == 0) ? be[3:0] : ((dw + 1 == dw_len) ? be[7:4] : 4'hF);
        end
    endfunction

    
    task resp_begin;
        input [2:0]   st;
        input integer nb;
        input integer dwl;
        input integer bc;
        begin
            if (tx_st != 2'd0) tx_drops <= tx_drops + 16'd1;   
            else begin
                t_type   <= T_CPL;
                t_fmt    <= (nb != 0) ? 2'h2 : 2'h0;
                t_addr   <= {ADWB{1'b0}};
                t_tag    <= h_tag;
                t_fn     <= h_fn;
                t_reqid  <= h_reqid;
                t_dwl    <= dwl[DWW-1:0];
                t_bcnt   <= bc[BCW-1:0];
                t_status <= st;
                t_nbytes <= nb[7:0];
                t_nbeats <= (nb == 0) ? 4'd1 : ((nb + (TDWB/8) - 1) / (TDWB/8));
                tx_st    <= 2'd1; tx_k <= 4'd0;
            end
        end
    endtask

    
    task dispatch_frame;
        integer i, nb;
        reg [7:0] e8;
        reg [31:0] d32;
        reg [63:0] base;
        begin
            if (h_type == T_CFG0 || h_type == T_CFG1) begin
                cfg_ops <= cfg_ops + 16'd1;
                if (h_addr[15:11] != 5'h0 || h_addr[10:8] != 3'h0) begin
                    t_pay <= 512'h0;                                  
                    resp_begin(3'd1, 0, 0, 0);
                end else if (h_fmt[1]) begin                          
                    ep_write(h_addr[7:0], rx_pay[31:0], 4'hF);
                    t_pay <= 512'h0;
                    resp_begin(3'd0, 0, 0, 0);                        
                end else begin                                        
                    d32 = ep_read(h_addr[7:0]);
                    t_pay <= {480'h0, d32};
                    resp_begin(3'd0, 4, 1, 4);
                end
            end else if (h_type == T_MEM) begin
                nb = h_blen;                                          
                if (nb > TX_MAXB) nb = TX_MAXB;
                if (h_fmt[1]) begin                                   
                    memwrs <= memwrs + 16'd1;
                    if (in_bar0(h_addr)) begin
                        base = {32'h0, ep_bar0};
                        for (i = 0; i < TX_MAXB; i = i + 1) begin
                            if (i < nb) begin
                                e8 = {4'h0, dw_en(i/4, (nb+3)/4, h_be)};
                                if (e8[i%4]) ep_mem[(h_addr - base) + i] <= rx_pay[i*8 +: 8];
                            end
                        end
                    end
                end else begin                                        
                    memrds <= memrds + 16'd1;
                    if (nb == 0 || !ep_cmd[1] || !in_bar0(h_addr) || !in_bar0(h_addr + nb - 1)) begin
                        t_pay <= 512'h0;
                        resp_begin(3'd1, 0, 0, 0);                    
                    end else begin
                        base = {32'h0, ep_bar0};
                        t_pay <= 512'h0;
                        for (i = 0; i < TX_MAXB; i = i + 1)
                            if (i < nb) t_pay[i*8 +: 8] <= ep_mem[(h_addr - base) + i];
                        resp_begin(3'd0, nb, nb/4, nb);
                    end
                end
            end else begin
                t_pay <= 512'h0;                                      
                resp_begin(3'd1, 0, 0, 0);
            end
        end
    endtask

    
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            rx_active <= 1'b0; disp_pend <= 1'b0; rx_beat <= 4'd0; rx_pay <= 512'h0;
            h_type <= 5'h0; h_fmt <= 2'h0; h_addr <= {ADWB{1'b0}}; h_tag <= {TGW{1'b0}};
            h_fn <= {FNW{1'b0}}; h_reqid <= {RQW{1'b0}}; h_blen <= {BLW{1'b0}}; h_be <= {BEW{1'b0}};
            tx_st <= 2'd0; tx_k <= 4'd0; t_type <= 5'h0; t_fmt <= 2'h0; t_addr <= {ADWB{1'b0}};
            t_tag <= {TGW{1'b0}}; t_fn <= {FNW{1'b0}}; t_reqid <= {RQW{1'b0}};
            t_dwl <= {DWW{1'b0}}; t_bcnt <= {BCW{1'b0}}; t_status <= 3'h0;
            t_pay <= 512'h0; t_nbytes <= 8'd0; t_nbeats <= 4'd1;
            dbi_st <= 2'd0; dbi_n <= 1'b0; dbi_cs_r <= 1'b0;
            dbi_ra <= 32'h0; dbi_rd <= 32'h0; dbi_dout_r <= 32'h0; dbi_rw <= 4'h0;
            dbg_errs <= 4'd0; dbg_frames <= 16'd0; dbg_beats <= 16'd0;
            cfg_ops <= 16'd0; memwrs <= 16'd0; memrds <= 16'd0;
            dbi_acks <= 16'd0; dbi_aborts <= 16'd0; tx_drops <= 16'd0; dbi_elbi <= 16'd0;
            dbg_snap <= {RSW{1'b0}};
            ep_bar0 <= 32'h0; ep_bar0_sz <= 1'b0; ep_cmd <= 16'h0;
            ep_msi_ctrl <= 16'h0; ep_msi_addr <= 32'h0; ep_msi_data <= 16'h0;
            cdm_04 <= 32'h0; cdm_18 <= 32'h0; cdm_1c <= 32'h0;
            cdm_20 <= 32'h0; cdm_24 <= 32'h0; cdm_28 <= 32'h0; cdm_2c <= 32'h0;
        end else begin
            
            if (tx_st == 2'd0) dbg_snap <= {dbg_errs, dbg_frames[5:0], dbg_beats[4:0]};

            
            if (disp_pend) begin
                disp_pend <= 1'b0;
                dispatch_frame;
            end
            
            if (rx_active) begin
                if (client0_tlp_dv) begin
                    dbg_beats <= dbg_beats + 16'd1;
                    if (rx_beat < 4'd8) rx_pay[rx_beat*CDWB +: CDWB] <= client0_tlp_data;
                    rx_beat <= rx_beat + 4'd1;
                end
                if (client0_tlp_hv) dbg_errs <= dbg_errs + 4'd1;              
                if (client0_tlp_eot) begin
                    rx_active <= 1'b0; disp_pend <= 1'b1;
                    dbg_frames <= dbg_frames + 16'd1;
                end
            end else if (client0_tlp_hv) begin
                rx_active <= 1'b1; rx_beat <= 4'd0; rx_pay <= 512'h0;
                h_type <= client0_tlp_type; h_fmt <= client0_tlp_fmt; h_addr <= client0_tlp_addr;
                h_tag <= client0_tlp_tid; h_fn <= client0_tlp_func_num; h_reqid <= client0_app_dev_id;
                h_blen <= client0_tlp_byte_len; h_be <= client0_tlp_byte_en;
                if (client0_tlp_dv) begin
                    dbg_beats <= dbg_beats + 16'd1;
                    rx_pay[0 +: CDWB] <= client0_tlp_data;
                    rx_beat <= 4'd1;
                end
                if (client0_tlp_eot) begin
                    rx_active <= 1'b0; disp_pend <= 1'b1;
                    dbg_frames <= dbg_frames + 16'd1;
                end
            end else if (client0_tlp_dv) begin
                dbg_errs <= dbg_errs + 4'd1;                                 
            end

            
            dbi_cs_r <= dbi_cs;
            case (dbi_st)
              2'd0: if (dbi_cs && !dbi_cs_r) begin              
                        dbi_ra <= dbi_addr; dbi_rd <= dbi_din; dbi_rw <= dbi_wr;
                        dbi_dout_r <= cdm_read(dbi_addr[7:0]);   
                        dbi_st <= 2'd1; dbi_n <= 1'b0;
                        if (dbi_addr[0]) dbi_elbi <= dbi_elbi + 16'd1;       
                    end
              2'd1: begin
                        if (!dbi_n) begin
                            dbi_n <= 1'b1;
                            if (dbi_cs) begin                                
                                if (dbi_rw != 4'h0) cdm_commit(dbi_ra, dbi_rd, dbi_rw);
                                dbi_acks <= dbi_acks + 16'd1;
                            end else dbi_aborts <= dbi_aborts + 16'd1;       
                        end else dbi_st <= 2'd0;
                    end
            endcase

            
            if (tx_st == 2'd1) begin
                if (tx_k + 4'd1 >= t_nbeats) tx_st <= 2'd0;
                else tx_k <= tx_k + 4'd1;
            end
        end
    end

    
    wire tx_on = (tx_st == 2'd1);
    assign radm_trgt1_hv  = tx_on && (tx_k == 4'd0);
    assign radm_trgt1_dv  = tx_on && (t_nbytes != 8'd0);
    assign radm_trgt1_eot = tx_on && (tx_k == t_nbeats - 4'd1);
    assign radm_trgt1_data  = tx_on ? t_pay[tx_k*TDWB +: TDWB] : {TDWB{1'b0}};
    assign radm_trgt1_fmt   = t_fmt;
    assign radm_trgt1_type  = t_type;
    assign radm_trgt1_tc    = 3'd0;
    assign radm_trgt1_attr  = 2'd0;
    assign radm_trgt1_reqid = t_reqid;
    assign radm_trgt1_tag   = t_tag;
    assign radm_trgt1_func_num = t_fn[2:0];
    assign radm_trgt1_td    = 1'b0;
    assign radm_trgt1_poisoned = 1'b0;
    assign radm_trgt1_dw_len = t_dwl;
    assign radm_trgt1_first_be = 4'h0;
    assign radm_trgt1_last_be  = 4'h0;
    assign radm_trgt1_addr  = t_addr;
    assign radm_trgt1_bcm   = 1'b0;
    assign radm_trgt1_cpl_last = 1'b0;
    assign radm_trgt1_cmpltr_id = EP_BDF;
    assign radm_trgt1_byte_cnt  = t_bcnt;
    assign radm_trgt1_cpl_status = t_status;
    assign radm_trgt1_dwen  = 2'd0;
    assign radm_trgt1_hdr_uppr_bytes = 64'h0;
    assign radm_trgt1_vc    = 3'd0;
    assign radm_trgt1_tlp_abort = 1'b0;
    assign radm_trgt1_dllp_abort = 1'b0;
    assign radm_trgt1_ecrc_err = 1'b0;
    assign radm_grant_tlp_type = tx_on ? 3'b100 : 3'b000;   

    
    assign radm_bypass_data = 64'h0;      assign radm_bypass_dwen = 2'd0;
    assign radm_bypass_dv = 1'b0;         assign radm_bypass_hv = 1'b0;
    assign radm_bypass_eot = 1'b0;        assign radm_bypass_tlp_abort = 1'b0;
    assign radm_bypass_dllp_abort = 1'b0; assign radm_bypass_ecrc_err = 1'b0;
    assign radm_bypass_fmt = 2'd0;        assign radm_bypass_type = 5'd0;
    assign radm_bypass_tc = 3'd0;         assign radm_bypass_attr = 2'd0;
    assign radm_bypass_reqid = 16'd0;     assign radm_bypass_tag = 8'd0;
    assign radm_bypass_func_num = 3'd0;   assign radm_bypass_td = 1'b0;
    assign radm_bypass_poisoned = 1'b0;   assign radm_bypass_dw_len = 10'd0;
    assign radm_bypass_first_be = 4'd0;   assign radm_bypass_last_be = 4'd0;
    assign radm_bypass_addr = 64'd0;      assign radm_bypass_bcm = 1'b0;
    assign radm_bypass_cpl_last = 1'b0;   assign radm_bypass_cmpltr_id = 16'd0;
    assign radm_bypass_byte_cnt = 12'd0;  assign radm_bypass_cpl_status = 3'd0;
    assign radm_bypass_rsvd = 15'd0;

    
    assign lbc_dbi_ack  = (dbi_st == 2'd1);                       
    
    assign lbc_dbi_dout = ((dbi_st == 2'd1) && !dbi_cs) ? 32'h0 : dbi_dout_r;
    assign xadm_client0_halt = 1'b0;
    assign xadm_client1_halt = 1'b0;
    assign radm_trgt1_rsvd = dbg_snap;                            
    assign cfg_pbus_num     = cdm_18[7:0];
    assign cfg_2ndbus_num   = cdm_18[15:8];
    assign cfg_subbus_num   = cdm_18[23:16];
    assign cfg_pbus_dev_num = 5'd0;
    assign cfg_mem_base     = cdm_20[15:0];
    assign cfg_mem_limit    = cdm_20[31:16];
    assign upstream_port    = 1'b0;                               
    assign device_type      = 4'd0;
    assign rdlh_link_up     = 1'b1;                               
    assign ven_msi_grant    = 1'b0;                               
    assign cfg_msi_en       = 1'b0;
    assign radm_correctable_err = 1'b0;
    assign radm_nonfatal_err = 1'b0;
    assign radm_fatal_err = 1'b0;
endmodule
