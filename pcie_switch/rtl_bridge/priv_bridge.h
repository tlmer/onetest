

#pragma once
#include "pcie_priv_port.h"
#include "priv_params.h"
#include <systemc>
#include <vector>

template <class P = pcie_tlm::priv_params>
struct priv_bridge_t : sc_core::sc_module {
    sc_core::sc_out<bool> client0_addr_align_en;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client0_tlp_byte_en>> client0_tlp_byte_en;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client0_remote_req_id>> client0_remote_req_id;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client0_cpl_byte_cnt>> client0_cpl_byte_cnt;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client0_tlp_tc>> client0_tlp_tc;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client0_tlp_attr>> client0_tlp_attr;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client0_cpl_status>> client0_cpl_status;
    sc_core::sc_out<bool> client0_cpl_bcm;
    sc_core::sc_out<bool> client0_tlp_dv;
    sc_core::sc_out<bool> client0_tlp_eot;
    sc_core::sc_out<bool> client0_tlp_bad_eot;
    sc_core::sc_out<bool> client0_tlp_hv;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client0_tlp_fmt>> client0_tlp_fmt;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client0_tlp_type>> client0_tlp_type;
    sc_core::sc_out<bool> client0_tlp_td;
    sc_core::sc_out<bool> client0_tlp_ep;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client0_tlp_byte_len>> client0_tlp_byte_len;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client0_tlp_addr>> client0_tlp_addr;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client0_tlp_tid>> client0_tlp_tid;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client0_tlp_data>> client0_tlp_data;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client0_tlp_func_num>> client0_tlp_func_num;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client0_app_dev_id>> client0_app_dev_id;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client0_hdr_rsvd>> client0_hdr_rsvd;
    sc_core::sc_in<bool> xadm_client0_halt;
    sc_core::sc_out<bool> client1_addr_align_en;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client1_tlp_byte_en>> client1_tlp_byte_en;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client1_remote_req_id>> client1_remote_req_id;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client1_cpl_byte_cnt>> client1_cpl_byte_cnt;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client1_tlp_tc>> client1_tlp_tc;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client1_tlp_attr>> client1_tlp_attr;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client1_cpl_status>> client1_cpl_status;
    sc_core::sc_out<bool> client1_cpl_bcm;
    sc_core::sc_out<bool> client1_tlp_dv;
    sc_core::sc_out<bool> client1_tlp_eot;
    sc_core::sc_out<bool> client1_tlp_bad_eot;
    sc_core::sc_out<bool> client1_tlp_hv;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client1_tlp_fmt>> client1_tlp_fmt;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client1_tlp_type>> client1_tlp_type;
    sc_core::sc_out<bool> client1_tlp_td;
    sc_core::sc_out<bool> client1_tlp_ep;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client1_tlp_byte_len>> client1_tlp_byte_len;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client1_tlp_addr>> client1_tlp_addr;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client1_tlp_tid>> client1_tlp_tid;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client1_tlp_data>> client1_tlp_data;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client1_tlp_func_num>> client1_tlp_func_num;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client1_app_dev_id>> client1_app_dev_id;
    sc_core::sc_out<sc_dt::sc_uint<P::W_client1_hdr_rsvd>> client1_hdr_rsvd;
    sc_core::sc_in<bool> xadm_client1_halt;
    sc_core::sc_in<bool> radm_trgt1_dv;
    sc_core::sc_in<bool> radm_trgt1_hv;
    sc_core::sc_in<bool> radm_trgt1_eot;
    sc_core::sc_in<bool> radm_trgt1_tlp_abort;
    sc_core::sc_in<bool> radm_trgt1_dllp_abort;
    sc_core::sc_in<bool> radm_trgt1_ecrc_err;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_trgt1_fmt>> radm_trgt1_fmt;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_trgt1_type>> radm_trgt1_type;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_trgt1_tc>> radm_trgt1_tc;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_trgt1_attr>> radm_trgt1_attr;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_trgt1_reqid>> radm_trgt1_reqid;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_trgt1_tag>> radm_trgt1_tag;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_trgt1_func_num>> radm_trgt1_func_num;
    sc_core::sc_in<bool> radm_trgt1_td;
    sc_core::sc_in<bool> radm_trgt1_poisoned;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_trgt1_dw_len>> radm_trgt1_dw_len;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_trgt1_first_be>> radm_trgt1_first_be;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_trgt1_last_be>> radm_trgt1_last_be;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_trgt1_addr>> radm_trgt1_addr;
    sc_core::sc_in<bool> radm_trgt1_bcm;
    sc_core::sc_in<bool> radm_trgt1_cpl_last;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_trgt1_cmpltr_id>> radm_trgt1_cmpltr_id;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_trgt1_byte_cnt>> radm_trgt1_byte_cnt;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_trgt1_cpl_status>> radm_trgt1_cpl_status;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_trgt1_rsvd>> radm_trgt1_rsvd;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_trgt1_data>> radm_trgt1_data;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_trgt1_dwen>> radm_trgt1_dwen;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_trgt1_hdr_uppr_bytes>> radm_trgt1_hdr_uppr_bytes;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_trgt1_vc>> radm_trgt1_vc;
    sc_core::sc_out<bool> trgt1_radm_halt;
    sc_core::sc_out<sc_dt::sc_uint<P::W_trgt1_radm_pkt_halt>> trgt1_radm_pkt_halt;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_grant_tlp_type>> radm_grant_tlp_type;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_bypass_data>> radm_bypass_data;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_bypass_dwen>> radm_bypass_dwen;
    sc_core::sc_in<bool> radm_bypass_dv;
    sc_core::sc_in<bool> radm_bypass_hv;
    sc_core::sc_in<bool> radm_bypass_eot;
    sc_core::sc_in<bool> radm_bypass_tlp_abort;
    sc_core::sc_in<bool> radm_bypass_dllp_abort;
    sc_core::sc_in<bool> radm_bypass_ecrc_err;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_bypass_fmt>> radm_bypass_fmt;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_bypass_type>> radm_bypass_type;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_bypass_tc>> radm_bypass_tc;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_bypass_attr>> radm_bypass_attr;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_bypass_reqid>> radm_bypass_reqid;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_bypass_tag>> radm_bypass_tag;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_bypass_func_num>> radm_bypass_func_num;
    sc_core::sc_in<bool> radm_bypass_td;
    sc_core::sc_in<bool> radm_bypass_poisoned;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_bypass_dw_len>> radm_bypass_dw_len;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_bypass_first_be>> radm_bypass_first_be;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_bypass_last_be>> radm_bypass_last_be;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_bypass_addr>> radm_bypass_addr;
    sc_core::sc_in<bool> radm_bypass_bcm;
    sc_core::sc_in<bool> radm_bypass_cpl_last;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_bypass_cmpltr_id>> radm_bypass_cmpltr_id;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_bypass_byte_cnt>> radm_bypass_byte_cnt;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_bypass_cpl_status>> radm_bypass_cpl_status;
    sc_core::sc_in<sc_dt::sc_uint<P::W_radm_bypass_rsvd>> radm_bypass_rsvd;
    sc_core::sc_out<sc_dt::sc_uint<P::W_dbi_addr>> dbi_addr;
    sc_core::sc_out<sc_dt::sc_uint<P::W_dbi_din>> dbi_din;
    sc_core::sc_out<bool> dbi_cs;
    sc_core::sc_out<bool> dbi_cs2;
    sc_core::sc_out<sc_dt::sc_uint<P::W_dbi_wr>> dbi_wr;
    sc_core::sc_in<bool> lbc_dbi_ack;
    sc_core::sc_in<sc_dt::sc_uint<P::W_lbc_dbi_dout>> lbc_dbi_dout;
    sc_core::sc_out<bool> ven_msi_req;
    sc_core::sc_out<sc_dt::sc_uint<P::W_ven_msi_func_num>> ven_msi_func_num;
    sc_core::sc_out<sc_dt::sc_uint<P::W_ven_msi_tc>> ven_msi_tc;
    sc_core::sc_out<sc_dt::sc_uint<P::W_ven_msi_vector>> ven_msi_vector;
    sc_core::sc_in<bool> ven_msi_grant;
    sc_core::sc_in<bool> cfg_msi_en;
    sc_core::sc_in<sc_dt::sc_uint<P::W_cfg_pbus_num>> cfg_pbus_num;
    sc_core::sc_in<sc_dt::sc_uint<P::W_cfg_pbus_dev_num>> cfg_pbus_dev_num;
    sc_core::sc_in<sc_dt::sc_uint<P::W_cfg_2ndbus_num>> cfg_2ndbus_num;
    sc_core::sc_in<sc_dt::sc_uint<P::W_cfg_subbus_num>> cfg_subbus_num;
    sc_core::sc_in<sc_dt::sc_uint<P::W_cfg_mem_base>> cfg_mem_base;
    sc_core::sc_in<sc_dt::sc_uint<P::W_cfg_mem_limit>> cfg_mem_limit;
    sc_core::sc_out<sc_dt::sc_uint<P::W_device_type>> device_type;
    sc_core::sc_in<bool> upstream_port;
    sc_core::sc_in<bool> rdlh_link_up;
    sc_core::sc_in<bool> cfg_bridge_crs_en;
    sc_core::sc_out<bool> xal_xmt_cpl_ca;
    sc_core::sc_out<bool> xal_rcvd_cpl_ca;
    sc_core::sc_out<bool> xal_rcvd_cpl_ur;
    sc_core::sc_in<bool> radm_correctable_err;
    sc_core::sc_in<bool> radm_nonfatal_err;
    sc_core::sc_in<bool> radm_fatal_err;
    SC_HAS_PROCESS(priv_bridge_t);
    explicit priv_bridge_t(sc_core::sc_module_name nm) : sc_core::sc_module(nm),
        client0_addr_align_en("client0_addr_align_en"),
        client0_tlp_byte_en("client0_tlp_byte_en"),
        client0_remote_req_id("client0_remote_req_id"),
        client0_cpl_byte_cnt("client0_cpl_byte_cnt"),
        client0_tlp_tc("client0_tlp_tc"),
        client0_tlp_attr("client0_tlp_attr"),
        client0_cpl_status("client0_cpl_status"),
        client0_cpl_bcm("client0_cpl_bcm"),
        client0_tlp_dv("client0_tlp_dv"),
        client0_tlp_eot("client0_tlp_eot"),
        client0_tlp_bad_eot("client0_tlp_bad_eot"),
        client0_tlp_hv("client0_tlp_hv"),
        client0_tlp_fmt("client0_tlp_fmt"),
        client0_tlp_type("client0_tlp_type"),
        client0_tlp_td("client0_tlp_td"),
        client0_tlp_ep("client0_tlp_ep"),
        client0_tlp_byte_len("client0_tlp_byte_len"),
        client0_tlp_addr("client0_tlp_addr"),
        client0_tlp_tid("client0_tlp_tid"),
        client0_tlp_data("client0_tlp_data"),
        client0_tlp_func_num("client0_tlp_func_num"),
        client0_app_dev_id("client0_app_dev_id"),
        client0_hdr_rsvd("client0_hdr_rsvd"),
        xadm_client0_halt("xadm_client0_halt"),
        client1_addr_align_en("client1_addr_align_en"),
        client1_tlp_byte_en("client1_tlp_byte_en"),
        client1_remote_req_id("client1_remote_req_id"),
        client1_cpl_byte_cnt("client1_cpl_byte_cnt"),
        client1_tlp_tc("client1_tlp_tc"),
        client1_tlp_attr("client1_tlp_attr"),
        client1_cpl_status("client1_cpl_status"),
        client1_cpl_bcm("client1_cpl_bcm"),
        client1_tlp_dv("client1_tlp_dv"),
        client1_tlp_eot("client1_tlp_eot"),
        client1_tlp_bad_eot("client1_tlp_bad_eot"),
        client1_tlp_hv("client1_tlp_hv"),
        client1_tlp_fmt("client1_tlp_fmt"),
        client1_tlp_type("client1_tlp_type"),
        client1_tlp_td("client1_tlp_td"),
        client1_tlp_ep("client1_tlp_ep"),
        client1_tlp_byte_len("client1_tlp_byte_len"),
        client1_tlp_addr("client1_tlp_addr"),
        client1_tlp_tid("client1_tlp_tid"),
        client1_tlp_data("client1_tlp_data"),
        client1_tlp_func_num("client1_tlp_func_num"),
        client1_app_dev_id("client1_app_dev_id"),
        client1_hdr_rsvd("client1_hdr_rsvd"),
        xadm_client1_halt("xadm_client1_halt"),
        radm_trgt1_dv("radm_trgt1_dv"),
        radm_trgt1_hv("radm_trgt1_hv"),
        radm_trgt1_eot("radm_trgt1_eot"),
        radm_trgt1_tlp_abort("radm_trgt1_tlp_abort"),
        radm_trgt1_dllp_abort("radm_trgt1_dllp_abort"),
        radm_trgt1_ecrc_err("radm_trgt1_ecrc_err"),
        radm_trgt1_fmt("radm_trgt1_fmt"),
        radm_trgt1_type("radm_trgt1_type"),
        radm_trgt1_tc("radm_trgt1_tc"),
        radm_trgt1_attr("radm_trgt1_attr"),
        radm_trgt1_reqid("radm_trgt1_reqid"),
        radm_trgt1_tag("radm_trgt1_tag"),
        radm_trgt1_func_num("radm_trgt1_func_num"),
        radm_trgt1_td("radm_trgt1_td"),
        radm_trgt1_poisoned("radm_trgt1_poisoned"),
        radm_trgt1_dw_len("radm_trgt1_dw_len"),
        radm_trgt1_first_be("radm_trgt1_first_be"),
        radm_trgt1_last_be("radm_trgt1_last_be"),
        radm_trgt1_addr("radm_trgt1_addr"),
        radm_trgt1_bcm("radm_trgt1_bcm"),
        radm_trgt1_cpl_last("radm_trgt1_cpl_last"),
        radm_trgt1_cmpltr_id("radm_trgt1_cmpltr_id"),
        radm_trgt1_byte_cnt("radm_trgt1_byte_cnt"),
        radm_trgt1_cpl_status("radm_trgt1_cpl_status"),
        radm_trgt1_rsvd("radm_trgt1_rsvd"),
        radm_trgt1_data("radm_trgt1_data"),
        radm_trgt1_dwen("radm_trgt1_dwen"),
        radm_trgt1_hdr_uppr_bytes("radm_trgt1_hdr_uppr_bytes"),
        radm_trgt1_vc("radm_trgt1_vc"),
        trgt1_radm_halt("trgt1_radm_halt"),
        trgt1_radm_pkt_halt("trgt1_radm_pkt_halt"),
        radm_grant_tlp_type("radm_grant_tlp_type"),
        radm_bypass_data("radm_bypass_data"),
        radm_bypass_dwen("radm_bypass_dwen"),
        radm_bypass_dv("radm_bypass_dv"),
        radm_bypass_hv("radm_bypass_hv"),
        radm_bypass_eot("radm_bypass_eot"),
        radm_bypass_tlp_abort("radm_bypass_tlp_abort"),
        radm_bypass_dllp_abort("radm_bypass_dllp_abort"),
        radm_bypass_ecrc_err("radm_bypass_ecrc_err"),
        radm_bypass_fmt("radm_bypass_fmt"),
        radm_bypass_type("radm_bypass_type"),
        radm_bypass_tc("radm_bypass_tc"),
        radm_bypass_attr("radm_bypass_attr"),
        radm_bypass_reqid("radm_bypass_reqid"),
        radm_bypass_tag("radm_bypass_tag"),
        radm_bypass_func_num("radm_bypass_func_num"),
        radm_bypass_td("radm_bypass_td"),
        radm_bypass_poisoned("radm_bypass_poisoned"),
        radm_bypass_dw_len("radm_bypass_dw_len"),
        radm_bypass_first_be("radm_bypass_first_be"),
        radm_bypass_last_be("radm_bypass_last_be"),
        radm_bypass_addr("radm_bypass_addr"),
        radm_bypass_bcm("radm_bypass_bcm"),
        radm_bypass_cpl_last("radm_bypass_cpl_last"),
        radm_bypass_cmpltr_id("radm_bypass_cmpltr_id"),
        radm_bypass_byte_cnt("radm_bypass_byte_cnt"),
        radm_bypass_cpl_status("radm_bypass_cpl_status"),
        radm_bypass_rsvd("radm_bypass_rsvd"),
        dbi_addr("dbi_addr"),
        dbi_din("dbi_din"),
        dbi_cs("dbi_cs"),
        dbi_cs2("dbi_cs2"),
        dbi_wr("dbi_wr"),
        lbc_dbi_ack("lbc_dbi_ack"),
        lbc_dbi_dout("lbc_dbi_dout"),
        ven_msi_req("ven_msi_req"),
        ven_msi_func_num("ven_msi_func_num"),
        ven_msi_tc("ven_msi_tc"),
        ven_msi_vector("ven_msi_vector"),
        ven_msi_grant("ven_msi_grant"),
        cfg_msi_en("cfg_msi_en"),
        cfg_pbus_num("cfg_pbus_num"),
        cfg_pbus_dev_num("cfg_pbus_dev_num"),
        cfg_2ndbus_num("cfg_2ndbus_num"),
        cfg_subbus_num("cfg_subbus_num"),
        cfg_mem_base("cfg_mem_base"),
        cfg_mem_limit("cfg_mem_limit"),
        device_type("device_type"),
        upstream_port("upstream_port"),
        rdlh_link_up("rdlh_link_up"),
        cfg_bridge_crs_en("cfg_bridge_crs_en"),
        xal_xmt_cpl_ca("xal_xmt_cpl_ca"),
        xal_rcvd_cpl_ca("xal_rcvd_cpl_ca"),
        xal_rcvd_cpl_ur("xal_rcvd_cpl_ur"),
        radm_correctable_err("radm_correctable_err"),
        radm_nonfatal_err("radm_nonfatal_err"),
        radm_fatal_err("radm_fatal_err") {}

    void push(const pcie_tlm::BitVec& b) {
        client0_addr_align_en.write(b.bit(P::O_client0_addr_align_en));
        client0_tlp_byte_en.write(sc_dt::sc_uint<P::W_client0_tlp_byte_en>(b.get(P::O_client0_tlp_byte_en + P::W_client0_tlp_byte_en - 1, P::O_client0_tlp_byte_en)));
        client0_remote_req_id.write(sc_dt::sc_uint<P::W_client0_remote_req_id>(b.get(P::O_client0_remote_req_id + P::W_client0_remote_req_id - 1, P::O_client0_remote_req_id)));
        client0_cpl_byte_cnt.write(sc_dt::sc_uint<P::W_client0_cpl_byte_cnt>(b.get(P::O_client0_cpl_byte_cnt + P::W_client0_cpl_byte_cnt - 1, P::O_client0_cpl_byte_cnt)));
        client0_tlp_tc.write(sc_dt::sc_uint<P::W_client0_tlp_tc>(b.get(P::O_client0_tlp_tc + P::W_client0_tlp_tc - 1, P::O_client0_tlp_tc)));
        client0_tlp_attr.write(sc_dt::sc_uint<P::W_client0_tlp_attr>(b.get(P::O_client0_tlp_attr + P::W_client0_tlp_attr - 1, P::O_client0_tlp_attr)));
        client0_cpl_status.write(sc_dt::sc_uint<P::W_client0_cpl_status>(b.get(P::O_client0_cpl_status + P::W_client0_cpl_status - 1, P::O_client0_cpl_status)));
        client0_cpl_bcm.write(b.bit(P::O_client0_cpl_bcm));
        client0_tlp_dv.write(b.bit(P::O_client0_tlp_dv));
        client0_tlp_eot.write(b.bit(P::O_client0_tlp_eot));
        client0_tlp_bad_eot.write(b.bit(P::O_client0_tlp_bad_eot));
        client0_tlp_hv.write(b.bit(P::O_client0_tlp_hv));
        client0_tlp_fmt.write(sc_dt::sc_uint<P::W_client0_tlp_fmt>(b.get(P::O_client0_tlp_fmt + P::W_client0_tlp_fmt - 1, P::O_client0_tlp_fmt)));
        client0_tlp_type.write(sc_dt::sc_uint<P::W_client0_tlp_type>(b.get(P::O_client0_tlp_type + P::W_client0_tlp_type - 1, P::O_client0_tlp_type)));
        client0_tlp_td.write(b.bit(P::O_client0_tlp_td));
        client0_tlp_ep.write(b.bit(P::O_client0_tlp_ep));
        client0_tlp_byte_len.write(sc_dt::sc_uint<P::W_client0_tlp_byte_len>(b.get(P::O_client0_tlp_byte_len + P::W_client0_tlp_byte_len - 1, P::O_client0_tlp_byte_len)));
        client0_tlp_addr.write(sc_dt::sc_uint<P::W_client0_tlp_addr>(b.get(P::O_client0_tlp_addr + P::W_client0_tlp_addr - 1, P::O_client0_tlp_addr)));
        client0_tlp_tid.write(sc_dt::sc_uint<P::W_client0_tlp_tid>(b.get(P::O_client0_tlp_tid + P::W_client0_tlp_tid - 1, P::O_client0_tlp_tid)));
        client0_tlp_data.write(sc_dt::sc_uint<P::W_client0_tlp_data>(b.get(P::O_client0_tlp_data + P::W_client0_tlp_data - 1, P::O_client0_tlp_data)));
        client0_tlp_func_num.write(sc_dt::sc_uint<P::W_client0_tlp_func_num>(b.get(P::O_client0_tlp_func_num + P::W_client0_tlp_func_num - 1, P::O_client0_tlp_func_num)));
        client0_app_dev_id.write(sc_dt::sc_uint<P::W_client0_app_dev_id>(b.get(P::O_client0_app_dev_id + P::W_client0_app_dev_id - 1, P::O_client0_app_dev_id)));
        client0_hdr_rsvd.write(sc_dt::sc_uint<P::W_client0_hdr_rsvd>(b.get(P::O_client0_hdr_rsvd + P::W_client0_hdr_rsvd - 1, P::O_client0_hdr_rsvd)));
        client1_addr_align_en.write(b.bit(P::O_client1_addr_align_en));
        client1_tlp_byte_en.write(sc_dt::sc_uint<P::W_client1_tlp_byte_en>(b.get(P::O_client1_tlp_byte_en + P::W_client1_tlp_byte_en - 1, P::O_client1_tlp_byte_en)));
        client1_remote_req_id.write(sc_dt::sc_uint<P::W_client1_remote_req_id>(b.get(P::O_client1_remote_req_id + P::W_client1_remote_req_id - 1, P::O_client1_remote_req_id)));
        client1_cpl_byte_cnt.write(sc_dt::sc_uint<P::W_client1_cpl_byte_cnt>(b.get(P::O_client1_cpl_byte_cnt + P::W_client1_cpl_byte_cnt - 1, P::O_client1_cpl_byte_cnt)));
        client1_tlp_tc.write(sc_dt::sc_uint<P::W_client1_tlp_tc>(b.get(P::O_client1_tlp_tc + P::W_client1_tlp_tc - 1, P::O_client1_tlp_tc)));
        client1_tlp_attr.write(sc_dt::sc_uint<P::W_client1_tlp_attr>(b.get(P::O_client1_tlp_attr + P::W_client1_tlp_attr - 1, P::O_client1_tlp_attr)));
        client1_cpl_status.write(sc_dt::sc_uint<P::W_client1_cpl_status>(b.get(P::O_client1_cpl_status + P::W_client1_cpl_status - 1, P::O_client1_cpl_status)));
        client1_cpl_bcm.write(b.bit(P::O_client1_cpl_bcm));
        client1_tlp_dv.write(b.bit(P::O_client1_tlp_dv));
        client1_tlp_eot.write(b.bit(P::O_client1_tlp_eot));
        client1_tlp_bad_eot.write(b.bit(P::O_client1_tlp_bad_eot));
        client1_tlp_hv.write(b.bit(P::O_client1_tlp_hv));
        client1_tlp_fmt.write(sc_dt::sc_uint<P::W_client1_tlp_fmt>(b.get(P::O_client1_tlp_fmt + P::W_client1_tlp_fmt - 1, P::O_client1_tlp_fmt)));
        client1_tlp_type.write(sc_dt::sc_uint<P::W_client1_tlp_type>(b.get(P::O_client1_tlp_type + P::W_client1_tlp_type - 1, P::O_client1_tlp_type)));
        client1_tlp_td.write(b.bit(P::O_client1_tlp_td));
        client1_tlp_ep.write(b.bit(P::O_client1_tlp_ep));
        client1_tlp_byte_len.write(sc_dt::sc_uint<P::W_client1_tlp_byte_len>(b.get(P::O_client1_tlp_byte_len + P::W_client1_tlp_byte_len - 1, P::O_client1_tlp_byte_len)));
        client1_tlp_addr.write(sc_dt::sc_uint<P::W_client1_tlp_addr>(b.get(P::O_client1_tlp_addr + P::W_client1_tlp_addr - 1, P::O_client1_tlp_addr)));
        client1_tlp_tid.write(sc_dt::sc_uint<P::W_client1_tlp_tid>(b.get(P::O_client1_tlp_tid + P::W_client1_tlp_tid - 1, P::O_client1_tlp_tid)));
        client1_tlp_data.write(sc_dt::sc_uint<P::W_client1_tlp_data>(b.get(P::O_client1_tlp_data + P::W_client1_tlp_data - 1, P::O_client1_tlp_data)));
        client1_tlp_func_num.write(sc_dt::sc_uint<P::W_client1_tlp_func_num>(b.get(P::O_client1_tlp_func_num + P::W_client1_tlp_func_num - 1, P::O_client1_tlp_func_num)));
        client1_app_dev_id.write(sc_dt::sc_uint<P::W_client1_app_dev_id>(b.get(P::O_client1_app_dev_id + P::W_client1_app_dev_id - 1, P::O_client1_app_dev_id)));
        client1_hdr_rsvd.write(sc_dt::sc_uint<P::W_client1_hdr_rsvd>(b.get(P::O_client1_hdr_rsvd + P::W_client1_hdr_rsvd - 1, P::O_client1_hdr_rsvd)));
        trgt1_radm_halt.write(b.bit(P::O_trgt1_radm_halt));
        trgt1_radm_pkt_halt.write(sc_dt::sc_uint<P::W_trgt1_radm_pkt_halt>(b.get(P::O_trgt1_radm_pkt_halt + P::W_trgt1_radm_pkt_halt - 1, P::O_trgt1_radm_pkt_halt)));
        dbi_addr.write(sc_dt::sc_uint<P::W_dbi_addr>(b.get(P::O_dbi_addr + P::W_dbi_addr - 1, P::O_dbi_addr)));
        dbi_din.write(sc_dt::sc_uint<P::W_dbi_din>(b.get(P::O_dbi_din + P::W_dbi_din - 1, P::O_dbi_din)));
        dbi_cs.write(b.bit(P::O_dbi_cs));
        dbi_cs2.write(b.bit(P::O_dbi_cs2));
        dbi_wr.write(sc_dt::sc_uint<P::W_dbi_wr>(b.get(P::O_dbi_wr + P::W_dbi_wr - 1, P::O_dbi_wr)));
        ven_msi_req.write(b.bit(P::O_ven_msi_req));
        ven_msi_func_num.write(sc_dt::sc_uint<P::W_ven_msi_func_num>(b.get(P::O_ven_msi_func_num + P::W_ven_msi_func_num - 1, P::O_ven_msi_func_num)));
        ven_msi_tc.write(sc_dt::sc_uint<P::W_ven_msi_tc>(b.get(P::O_ven_msi_tc + P::W_ven_msi_tc - 1, P::O_ven_msi_tc)));
        ven_msi_vector.write(sc_dt::sc_uint<P::W_ven_msi_vector>(b.get(P::O_ven_msi_vector + P::W_ven_msi_vector - 1, P::O_ven_msi_vector)));
        device_type.write(sc_dt::sc_uint<P::W_device_type>(b.get(P::O_device_type + P::W_device_type - 1, P::O_device_type)));
        xal_xmt_cpl_ca.write(b.bit(P::O_xal_xmt_cpl_ca));
        xal_rcvd_cpl_ca.write(b.bit(P::O_xal_rcvd_cpl_ca));
        xal_rcvd_cpl_ur.write(b.bit(P::O_xal_rcvd_cpl_ur));
    }

    void pull(pcie_tlm::BitVec& b) const {
        b.set_bit(P::O_xadm_client0_halt, xadm_client0_halt.read());
        b.set_bit(P::O_xadm_client1_halt, xadm_client1_halt.read());
        b.set_bit(P::O_radm_trgt1_dv, radm_trgt1_dv.read());
        b.set_bit(P::O_radm_trgt1_hv, radm_trgt1_hv.read());
        b.set_bit(P::O_radm_trgt1_eot, radm_trgt1_eot.read());
        b.set_bit(P::O_radm_trgt1_tlp_abort, radm_trgt1_tlp_abort.read());
        b.set_bit(P::O_radm_trgt1_dllp_abort, radm_trgt1_dllp_abort.read());
        b.set_bit(P::O_radm_trgt1_ecrc_err, radm_trgt1_ecrc_err.read());
        b.set(P::O_radm_trgt1_fmt + P::W_radm_trgt1_fmt - 1, P::O_radm_trgt1_fmt, radm_trgt1_fmt.read().to_uint64());
        b.set(P::O_radm_trgt1_type + P::W_radm_trgt1_type - 1, P::O_radm_trgt1_type, radm_trgt1_type.read().to_uint64());
        b.set(P::O_radm_trgt1_tc + P::W_radm_trgt1_tc - 1, P::O_radm_trgt1_tc, radm_trgt1_tc.read().to_uint64());
        b.set(P::O_radm_trgt1_attr + P::W_radm_trgt1_attr - 1, P::O_radm_trgt1_attr, radm_trgt1_attr.read().to_uint64());
        b.set(P::O_radm_trgt1_reqid + P::W_radm_trgt1_reqid - 1, P::O_radm_trgt1_reqid, radm_trgt1_reqid.read().to_uint64());
        b.set(P::O_radm_trgt1_tag + P::W_radm_trgt1_tag - 1, P::O_radm_trgt1_tag, radm_trgt1_tag.read().to_uint64());
        b.set(P::O_radm_trgt1_func_num + P::W_radm_trgt1_func_num - 1, P::O_radm_trgt1_func_num, radm_trgt1_func_num.read().to_uint64());
        b.set_bit(P::O_radm_trgt1_td, radm_trgt1_td.read());
        b.set_bit(P::O_radm_trgt1_poisoned, radm_trgt1_poisoned.read());
        b.set(P::O_radm_trgt1_dw_len + P::W_radm_trgt1_dw_len - 1, P::O_radm_trgt1_dw_len, radm_trgt1_dw_len.read().to_uint64());
        b.set(P::O_radm_trgt1_first_be + P::W_radm_trgt1_first_be - 1, P::O_radm_trgt1_first_be, radm_trgt1_first_be.read().to_uint64());
        b.set(P::O_radm_trgt1_last_be + P::W_radm_trgt1_last_be - 1, P::O_radm_trgt1_last_be, radm_trgt1_last_be.read().to_uint64());
        b.set(P::O_radm_trgt1_addr + P::W_radm_trgt1_addr - 1, P::O_radm_trgt1_addr, radm_trgt1_addr.read().to_uint64());
        b.set_bit(P::O_radm_trgt1_bcm, radm_trgt1_bcm.read());
        b.set_bit(P::O_radm_trgt1_cpl_last, radm_trgt1_cpl_last.read());
        b.set(P::O_radm_trgt1_cmpltr_id + P::W_radm_trgt1_cmpltr_id - 1, P::O_radm_trgt1_cmpltr_id, radm_trgt1_cmpltr_id.read().to_uint64());
        b.set(P::O_radm_trgt1_byte_cnt + P::W_radm_trgt1_byte_cnt - 1, P::O_radm_trgt1_byte_cnt, radm_trgt1_byte_cnt.read().to_uint64());
        b.set(P::O_radm_trgt1_cpl_status + P::W_radm_trgt1_cpl_status - 1, P::O_radm_trgt1_cpl_status, radm_trgt1_cpl_status.read().to_uint64());
        b.set(P::O_radm_trgt1_rsvd + P::W_radm_trgt1_rsvd - 1, P::O_radm_trgt1_rsvd, radm_trgt1_rsvd.read().to_uint64());
        b.set(P::O_radm_trgt1_data + P::W_radm_trgt1_data - 1, P::O_radm_trgt1_data, radm_trgt1_data.read().to_uint64());
        b.set(P::O_radm_trgt1_dwen + P::W_radm_trgt1_dwen - 1, P::O_radm_trgt1_dwen, radm_trgt1_dwen.read().to_uint64());
        b.set(P::O_radm_trgt1_hdr_uppr_bytes + P::W_radm_trgt1_hdr_uppr_bytes - 1, P::O_radm_trgt1_hdr_uppr_bytes, radm_trgt1_hdr_uppr_bytes.read().to_uint64());
        b.set(P::O_radm_trgt1_vc + P::W_radm_trgt1_vc - 1, P::O_radm_trgt1_vc, radm_trgt1_vc.read().to_uint64());
        b.set(P::O_radm_grant_tlp_type + P::W_radm_grant_tlp_type - 1, P::O_radm_grant_tlp_type, radm_grant_tlp_type.read().to_uint64());
        b.set(P::O_radm_bypass_data + P::W_radm_bypass_data - 1, P::O_radm_bypass_data, radm_bypass_data.read().to_uint64());
        b.set(P::O_radm_bypass_dwen + P::W_radm_bypass_dwen - 1, P::O_radm_bypass_dwen, radm_bypass_dwen.read().to_uint64());
        b.set_bit(P::O_radm_bypass_dv, radm_bypass_dv.read());
        b.set_bit(P::O_radm_bypass_hv, radm_bypass_hv.read());
        b.set_bit(P::O_radm_bypass_eot, radm_bypass_eot.read());
        b.set_bit(P::O_radm_bypass_tlp_abort, radm_bypass_tlp_abort.read());
        b.set_bit(P::O_radm_bypass_dllp_abort, radm_bypass_dllp_abort.read());
        b.set_bit(P::O_radm_bypass_ecrc_err, radm_bypass_ecrc_err.read());
        b.set(P::O_radm_bypass_fmt + P::W_radm_bypass_fmt - 1, P::O_radm_bypass_fmt, radm_bypass_fmt.read().to_uint64());
        b.set(P::O_radm_bypass_type + P::W_radm_bypass_type - 1, P::O_radm_bypass_type, radm_bypass_type.read().to_uint64());
        b.set(P::O_radm_bypass_tc + P::W_radm_bypass_tc - 1, P::O_radm_bypass_tc, radm_bypass_tc.read().to_uint64());
        b.set(P::O_radm_bypass_attr + P::W_radm_bypass_attr - 1, P::O_radm_bypass_attr, radm_bypass_attr.read().to_uint64());
        b.set(P::O_radm_bypass_reqid + P::W_radm_bypass_reqid - 1, P::O_radm_bypass_reqid, radm_bypass_reqid.read().to_uint64());
        b.set(P::O_radm_bypass_tag + P::W_radm_bypass_tag - 1, P::O_radm_bypass_tag, radm_bypass_tag.read().to_uint64());
        b.set(P::O_radm_bypass_func_num + P::W_radm_bypass_func_num - 1, P::O_radm_bypass_func_num, radm_bypass_func_num.read().to_uint64());
        b.set_bit(P::O_radm_bypass_td, radm_bypass_td.read());
        b.set_bit(P::O_radm_bypass_poisoned, radm_bypass_poisoned.read());
        b.set(P::O_radm_bypass_dw_len + P::W_radm_bypass_dw_len - 1, P::O_radm_bypass_dw_len, radm_bypass_dw_len.read().to_uint64());
        b.set(P::O_radm_bypass_first_be + P::W_radm_bypass_first_be - 1, P::O_radm_bypass_first_be, radm_bypass_first_be.read().to_uint64());
        b.set(P::O_radm_bypass_last_be + P::W_radm_bypass_last_be - 1, P::O_radm_bypass_last_be, radm_bypass_last_be.read().to_uint64());
        b.set(P::O_radm_bypass_addr + P::W_radm_bypass_addr - 1, P::O_radm_bypass_addr, radm_bypass_addr.read().to_uint64());
        b.set_bit(P::O_radm_bypass_bcm, radm_bypass_bcm.read());
        b.set_bit(P::O_radm_bypass_cpl_last, radm_bypass_cpl_last.read());
        b.set(P::O_radm_bypass_cmpltr_id + P::W_radm_bypass_cmpltr_id - 1, P::O_radm_bypass_cmpltr_id, radm_bypass_cmpltr_id.read().to_uint64());
        b.set(P::O_radm_bypass_byte_cnt + P::W_radm_bypass_byte_cnt - 1, P::O_radm_bypass_byte_cnt, radm_bypass_byte_cnt.read().to_uint64());
        b.set(P::O_radm_bypass_cpl_status + P::W_radm_bypass_cpl_status - 1, P::O_radm_bypass_cpl_status, radm_bypass_cpl_status.read().to_uint64());
        b.set(P::O_radm_bypass_rsvd + P::W_radm_bypass_rsvd - 1, P::O_radm_bypass_rsvd, radm_bypass_rsvd.read().to_uint64());
        b.set_bit(P::O_lbc_dbi_ack, lbc_dbi_ack.read());
        b.set(P::O_lbc_dbi_dout + P::W_lbc_dbi_dout - 1, P::O_lbc_dbi_dout, lbc_dbi_dout.read().to_uint64());
        b.set_bit(P::O_ven_msi_grant, ven_msi_grant.read());
        b.set_bit(P::O_cfg_msi_en, cfg_msi_en.read());
        b.set(P::O_cfg_pbus_num + P::W_cfg_pbus_num - 1, P::O_cfg_pbus_num, cfg_pbus_num.read().to_uint64());
        b.set(P::O_cfg_pbus_dev_num + P::W_cfg_pbus_dev_num - 1, P::O_cfg_pbus_dev_num, cfg_pbus_dev_num.read().to_uint64());
        b.set(P::O_cfg_2ndbus_num + P::W_cfg_2ndbus_num - 1, P::O_cfg_2ndbus_num, cfg_2ndbus_num.read().to_uint64());
        b.set(P::O_cfg_subbus_num + P::W_cfg_subbus_num - 1, P::O_cfg_subbus_num, cfg_subbus_num.read().to_uint64());
        b.set(P::O_cfg_mem_base + P::W_cfg_mem_base - 1, P::O_cfg_mem_base, cfg_mem_base.read().to_uint64());
        b.set(P::O_cfg_mem_limit + P::W_cfg_mem_limit - 1, P::O_cfg_mem_limit, cfg_mem_limit.read().to_uint64());
        b.set_bit(P::O_upstream_port, upstream_port.read());
        b.set_bit(P::O_rdlh_link_up, rdlh_link_up.read());
        b.set_bit(P::O_cfg_bridge_crs_en, cfg_bridge_crs_en.read());
        b.set_bit(P::O_radm_correctable_err, radm_correctable_err.read());
        b.set_bit(P::O_radm_nonfatal_err, radm_nonfatal_err.read());
        b.set_bit(P::O_radm_fatal_err, radm_fatal_err.read());
    }
};

using priv_bridge = priv_bridge_t<pcie_tlm::priv_params>;
