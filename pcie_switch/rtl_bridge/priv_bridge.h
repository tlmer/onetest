

#pragma once
#include "pcie_priv_port.h"
#include <systemc>

struct priv_bridge : sc_core::sc_module {
    sc_core::sc_out<bool> client0_addr_align_en;
    sc_core::sc_out<sc_uint<8>> client0_tlp_byte_en;
    sc_core::sc_out<sc_uint<16>> client0_remote_req_id;
    sc_core::sc_out<sc_uint<12>> client0_cpl_byte_cnt;
    sc_core::sc_out<sc_uint<3>> client0_tlp_tc;
    sc_core::sc_out<sc_uint<2>> client0_tlp_attr;
    sc_core::sc_out<sc_uint<3>> client0_cpl_status;
    sc_core::sc_out<bool> client0_cpl_bcm;
    sc_core::sc_out<bool> client0_tlp_dv;
    sc_core::sc_out<bool> client0_tlp_eot;
    sc_core::sc_out<bool> client0_tlp_bad_eot;
    sc_core::sc_out<bool> client0_tlp_hv;
    sc_core::sc_out<sc_uint<2>> client0_tlp_fmt;
    sc_core::sc_out<sc_uint<5>> client0_tlp_type;
    sc_core::sc_out<bool> client0_tlp_td;
    sc_core::sc_out<bool> client0_tlp_ep;
    sc_core::sc_out<sc_uint<13>> client0_tlp_byte_len;
    sc_core::sc_out<sc_uint<64>> client0_tlp_addr;
    sc_core::sc_out<sc_uint<8>> client0_tlp_tid;
    sc_core::sc_out<sc_uint<64>> client0_tlp_data;
    sc_core::sc_out<sc_uint<3>> client0_tlp_func_num;
    sc_core::sc_out<sc_uint<16>> client0_app_dev_id;
    sc_core::sc_out<sc_uint<15>> client0_hdr_rsvd;
    sc_core::sc_in<bool> xadm_client0_halt;
    sc_core::sc_out<bool> client1_addr_align_en;
    sc_core::sc_out<sc_uint<8>> client1_tlp_byte_en;
    sc_core::sc_out<sc_uint<16>> client1_remote_req_id;
    sc_core::sc_out<sc_uint<12>> client1_cpl_byte_cnt;
    sc_core::sc_out<sc_uint<3>> client1_tlp_tc;
    sc_core::sc_out<sc_uint<2>> client1_tlp_attr;
    sc_core::sc_out<sc_uint<3>> client1_cpl_status;
    sc_core::sc_out<bool> client1_cpl_bcm;
    sc_core::sc_out<bool> client1_tlp_dv;
    sc_core::sc_out<bool> client1_tlp_eot;
    sc_core::sc_out<bool> client1_tlp_bad_eot;
    sc_core::sc_out<bool> client1_tlp_hv;
    sc_core::sc_out<sc_uint<2>> client1_tlp_fmt;
    sc_core::sc_out<sc_uint<5>> client1_tlp_type;
    sc_core::sc_out<bool> client1_tlp_td;
    sc_core::sc_out<bool> client1_tlp_ep;
    sc_core::sc_out<sc_uint<13>> client1_tlp_byte_len;
    sc_core::sc_out<sc_uint<64>> client1_tlp_addr;
    sc_core::sc_out<sc_uint<8>> client1_tlp_tid;
    sc_core::sc_out<sc_uint<64>> client1_tlp_data;
    sc_core::sc_out<sc_uint<3>> client1_tlp_func_num;
    sc_core::sc_out<sc_uint<16>> client1_app_dev_id;
    sc_core::sc_out<sc_uint<15>> client1_hdr_rsvd;
    sc_core::sc_in<bool> xadm_client1_halt;
    sc_core::sc_in<bool> radm_trgt1_dv;
    sc_core::sc_in<bool> radm_trgt1_hv;
    sc_core::sc_in<bool> radm_trgt1_eot;
    sc_core::sc_in<bool> radm_trgt1_tlp_abort;
    sc_core::sc_in<bool> radm_trgt1_dllp_abort;
    sc_core::sc_in<bool> radm_trgt1_ecrc_err;
    sc_core::sc_in<sc_uint<2>> radm_trgt1_fmt;
    sc_core::sc_in<sc_uint<5>> radm_trgt1_type;
    sc_core::sc_in<sc_uint<3>> radm_trgt1_tc;
    sc_core::sc_in<sc_uint<2>> radm_trgt1_attr;
    sc_core::sc_in<sc_uint<16>> radm_trgt1_reqid;
    sc_core::sc_in<sc_uint<8>> radm_trgt1_tag;
    sc_core::sc_in<sc_uint<3>> radm_trgt1_func_num;
    sc_core::sc_in<bool> radm_trgt1_td;
    sc_core::sc_in<bool> radm_trgt1_poisoned;
    sc_core::sc_in<sc_uint<10>> radm_trgt1_dw_len;
    sc_core::sc_in<sc_uint<4>> radm_trgt1_first_be;
    sc_core::sc_in<sc_uint<4>> radm_trgt1_last_be;
    sc_core::sc_in<sc_uint<64>> radm_trgt1_addr;
    sc_core::sc_in<bool> radm_trgt1_bcm;
    sc_core::sc_in<bool> radm_trgt1_cpl_last;
    sc_core::sc_in<sc_uint<16>> radm_trgt1_cmpltr_id;
    sc_core::sc_in<sc_uint<12>> radm_trgt1_byte_cnt;
    sc_core::sc_in<sc_uint<3>> radm_trgt1_cpl_status;
    sc_core::sc_in<sc_uint<15>> radm_trgt1_rsvd;
    sc_core::sc_in<sc_uint<64>> radm_trgt1_data;
    sc_core::sc_in<sc_uint<2>> radm_trgt1_dwen;
    sc_core::sc_in<sc_uint<64>> radm_trgt1_hdr_uppr_bytes;
    sc_core::sc_in<sc_uint<3>> radm_trgt1_vc;
    sc_core::sc_out<bool> trgt1_radm_halt;
    sc_core::sc_out<sc_uint<3>> trgt1_radm_pkt_halt;
    sc_core::sc_in<sc_uint<3>> radm_grant_tlp_type;
    sc_core::sc_in<sc_uint<64>> radm_bypass_data;
    sc_core::sc_in<sc_uint<2>> radm_bypass_dwen;
    sc_core::sc_in<bool> radm_bypass_dv;
    sc_core::sc_in<bool> radm_bypass_hv;
    sc_core::sc_in<bool> radm_bypass_eot;
    sc_core::sc_in<bool> radm_bypass_tlp_abort;
    sc_core::sc_in<bool> radm_bypass_dllp_abort;
    sc_core::sc_in<bool> radm_bypass_ecrc_err;
    sc_core::sc_in<sc_uint<2>> radm_bypass_fmt;
    sc_core::sc_in<sc_uint<5>> radm_bypass_type;
    sc_core::sc_in<sc_uint<3>> radm_bypass_tc;
    sc_core::sc_in<sc_uint<2>> radm_bypass_attr;
    sc_core::sc_in<sc_uint<16>> radm_bypass_reqid;
    sc_core::sc_in<sc_uint<8>> radm_bypass_tag;
    sc_core::sc_in<sc_uint<3>> radm_bypass_func_num;
    sc_core::sc_in<bool> radm_bypass_td;
    sc_core::sc_in<bool> radm_bypass_poisoned;
    sc_core::sc_in<sc_uint<10>> radm_bypass_dw_len;
    sc_core::sc_in<sc_uint<4>> radm_bypass_first_be;
    sc_core::sc_in<sc_uint<4>> radm_bypass_last_be;
    sc_core::sc_in<sc_uint<64>> radm_bypass_addr;
    sc_core::sc_in<bool> radm_bypass_bcm;
    sc_core::sc_in<bool> radm_bypass_cpl_last;
    sc_core::sc_in<sc_uint<16>> radm_bypass_cmpltr_id;
    sc_core::sc_in<sc_uint<12>> radm_bypass_byte_cnt;
    sc_core::sc_in<sc_uint<3>> radm_bypass_cpl_status;
    sc_core::sc_in<sc_uint<15>> radm_bypass_rsvd;
    sc_core::sc_out<sc_uint<32>> dbi_addr;
    sc_core::sc_out<sc_uint<32>> dbi_din;
    sc_core::sc_out<bool> dbi_cs;
    sc_core::sc_out<bool> dbi_cs2;
    sc_core::sc_out<sc_uint<4>> dbi_wr;
    sc_core::sc_in<bool> lbc_dbi_ack;
    sc_core::sc_in<sc_uint<32>> lbc_dbi_dout;
    sc_core::sc_out<bool> ven_msi_req;
    sc_core::sc_out<sc_uint<3>> ven_msi_func_num;
    sc_core::sc_out<sc_uint<3>> ven_msi_tc;
    sc_core::sc_out<sc_uint<5>> ven_msi_vector;
    sc_core::sc_in<bool> ven_msi_grant;
    sc_core::sc_in<bool> cfg_msi_en;
    sc_core::sc_in<sc_uint<8>> cfg_pbus_num;
    sc_core::sc_in<sc_uint<5>> cfg_pbus_dev_num;
    sc_core::sc_in<sc_uint<8>> cfg_2ndbus_num;
    sc_core::sc_in<sc_uint<8>> cfg_subbus_num;
    sc_core::sc_in<sc_uint<16>> cfg_mem_base;
    sc_core::sc_in<sc_uint<16>> cfg_mem_limit;
    sc_core::sc_out<sc_uint<4>> device_type;
    sc_core::sc_in<bool> upstream_port;
    sc_core::sc_in<bool> rdlh_link_up;
    sc_core::sc_in<bool> cfg_bridge_crs_en;
    sc_core::sc_out<bool> xal_xmt_cpl_ca;
    sc_core::sc_out<bool> xal_rcvd_cpl_ca;
    sc_core::sc_out<bool> xal_rcvd_cpl_ur;
    sc_core::sc_in<bool> radm_correctable_err;
    sc_core::sc_in<bool> radm_nonfatal_err;
    sc_core::sc_in<bool> radm_fatal_err;
    SC_HAS_PROCESS(priv_bridge);
    explicit priv_bridge(sc_core::sc_module_name nm) : sc_core::sc_module(nm),
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
        client0_addr_align_en.write(b.bit(0));
        client0_tlp_byte_en.write(sc_uint<8>(b.get(8, 1)));
        client0_remote_req_id.write(sc_uint<16>(b.get(24, 9)));
        client0_cpl_byte_cnt.write(sc_uint<12>(b.get(36, 25)));
        client0_tlp_tc.write(sc_uint<3>(b.get(39, 37)));
        client0_tlp_attr.write(sc_uint<2>(b.get(41, 40)));
        client0_cpl_status.write(sc_uint<3>(b.get(44, 42)));
        client0_cpl_bcm.write(b.bit(45));
        client0_tlp_dv.write(b.bit(46));
        client0_tlp_eot.write(b.bit(47));
        client0_tlp_bad_eot.write(b.bit(48));
        client0_tlp_hv.write(b.bit(49));
        client0_tlp_fmt.write(sc_uint<2>(b.get(51, 50)));
        client0_tlp_type.write(sc_uint<5>(b.get(56, 52)));
        client0_tlp_td.write(b.bit(57));
        client0_tlp_ep.write(b.bit(58));
        client0_tlp_byte_len.write(sc_uint<13>(b.get(71, 59)));
        client0_tlp_addr.write(sc_uint<64>(b.get(135, 72)));
        client0_tlp_tid.write(sc_uint<8>(b.get(143, 136)));
        client0_tlp_data.write(sc_uint<64>(b.get(207, 144)));
        client0_tlp_func_num.write(sc_uint<3>(b.get(210, 208)));
        client0_app_dev_id.write(sc_uint<16>(b.get(226, 211)));
        client0_hdr_rsvd.write(sc_uint<15>(b.get(241, 227)));
        client1_addr_align_en.write(b.bit(243));
        client1_tlp_byte_en.write(sc_uint<8>(b.get(251, 244)));
        client1_remote_req_id.write(sc_uint<16>(b.get(267, 252)));
        client1_cpl_byte_cnt.write(sc_uint<12>(b.get(279, 268)));
        client1_tlp_tc.write(sc_uint<3>(b.get(282, 280)));
        client1_tlp_attr.write(sc_uint<2>(b.get(284, 283)));
        client1_cpl_status.write(sc_uint<3>(b.get(287, 285)));
        client1_cpl_bcm.write(b.bit(288));
        client1_tlp_dv.write(b.bit(289));
        client1_tlp_eot.write(b.bit(290));
        client1_tlp_bad_eot.write(b.bit(291));
        client1_tlp_hv.write(b.bit(292));
        client1_tlp_fmt.write(sc_uint<2>(b.get(294, 293)));
        client1_tlp_type.write(sc_uint<5>(b.get(299, 295)));
        client1_tlp_td.write(b.bit(300));
        client1_tlp_ep.write(b.bit(301));
        client1_tlp_byte_len.write(sc_uint<13>(b.get(314, 302)));
        client1_tlp_addr.write(sc_uint<64>(b.get(378, 315)));
        client1_tlp_tid.write(sc_uint<8>(b.get(386, 379)));
        client1_tlp_data.write(sc_uint<64>(b.get(450, 387)));
        client1_tlp_func_num.write(sc_uint<3>(b.get(453, 451)));
        client1_app_dev_id.write(sc_uint<16>(b.get(469, 454)));
        client1_hdr_rsvd.write(sc_uint<15>(b.get(484, 470)));
        trgt1_radm_halt.write(b.bit(796));
        trgt1_radm_pkt_halt.write(sc_uint<3>(b.get(799, 797)));
        dbi_addr.write(sc_uint<32>(b.get(1077, 1046)));
        dbi_din.write(sc_uint<32>(b.get(1109, 1078)));
        dbi_cs.write(b.bit(1110));
        dbi_cs2.write(b.bit(1111));
        dbi_wr.write(sc_uint<4>(b.get(1115, 1112)));
        ven_msi_req.write(b.bit(1149));
        ven_msi_func_num.write(sc_uint<3>(b.get(1152, 1150)));
        ven_msi_tc.write(sc_uint<3>(b.get(1155, 1153)));
        ven_msi_vector.write(sc_uint<5>(b.get(1160, 1156)));
        device_type.write(sc_uint<4>(b.get(1227, 1224)));
        xal_xmt_cpl_ca.write(b.bit(1231));
        xal_rcvd_cpl_ca.write(b.bit(1232));
        xal_rcvd_cpl_ur.write(b.bit(1233));
    }

    void pull(pcie_tlm::BitVec& b) const {
        b.set_bit(242, xadm_client0_halt.read());
        b.set_bit(485, xadm_client1_halt.read());
        b.set_bit(486, radm_trgt1_dv.read());
        b.set_bit(487, radm_trgt1_hv.read());
        b.set_bit(488, radm_trgt1_eot.read());
        b.set_bit(489, radm_trgt1_tlp_abort.read());
        b.set_bit(490, radm_trgt1_dllp_abort.read());
        b.set_bit(491, radm_trgt1_ecrc_err.read());
        b.set(493, 492, radm_trgt1_fmt.read().to_uint64());
        b.set(498, 494, radm_trgt1_type.read().to_uint64());
        b.set(501, 499, radm_trgt1_tc.read().to_uint64());
        b.set(503, 502, radm_trgt1_attr.read().to_uint64());
        b.set(519, 504, radm_trgt1_reqid.read().to_uint64());
        b.set(527, 520, radm_trgt1_tag.read().to_uint64());
        b.set(530, 528, radm_trgt1_func_num.read().to_uint64());
        b.set_bit(531, radm_trgt1_td.read());
        b.set_bit(532, radm_trgt1_poisoned.read());
        b.set(542, 533, radm_trgt1_dw_len.read().to_uint64());
        b.set(546, 543, radm_trgt1_first_be.read().to_uint64());
        b.set(550, 547, radm_trgt1_last_be.read().to_uint64());
        b.set(614, 551, radm_trgt1_addr.read().to_uint64());
        b.set_bit(615, radm_trgt1_bcm.read());
        b.set_bit(616, radm_trgt1_cpl_last.read());
        b.set(632, 617, radm_trgt1_cmpltr_id.read().to_uint64());
        b.set(644, 633, radm_trgt1_byte_cnt.read().to_uint64());
        b.set(647, 645, radm_trgt1_cpl_status.read().to_uint64());
        b.set(662, 648, radm_trgt1_rsvd.read().to_uint64());
        b.set(726, 663, radm_trgt1_data.read().to_uint64());
        b.set(728, 727, radm_trgt1_dwen.read().to_uint64());
        b.set(792, 729, radm_trgt1_hdr_uppr_bytes.read().to_uint64());
        b.set(795, 793, radm_trgt1_vc.read().to_uint64());
        b.set(802, 800, radm_grant_tlp_type.read().to_uint64());
        b.set(866, 803, radm_bypass_data.read().to_uint64());
        b.set(868, 867, radm_bypass_dwen.read().to_uint64());
        b.set_bit(869, radm_bypass_dv.read());
        b.set_bit(870, radm_bypass_hv.read());
        b.set_bit(871, radm_bypass_eot.read());
        b.set_bit(872, radm_bypass_tlp_abort.read());
        b.set_bit(873, radm_bypass_dllp_abort.read());
        b.set_bit(874, radm_bypass_ecrc_err.read());
        b.set(876, 875, radm_bypass_fmt.read().to_uint64());
        b.set(881, 877, radm_bypass_type.read().to_uint64());
        b.set(884, 882, radm_bypass_tc.read().to_uint64());
        b.set(886, 885, radm_bypass_attr.read().to_uint64());
        b.set(902, 887, radm_bypass_reqid.read().to_uint64());
        b.set(910, 903, radm_bypass_tag.read().to_uint64());
        b.set(913, 911, radm_bypass_func_num.read().to_uint64());
        b.set_bit(914, radm_bypass_td.read());
        b.set_bit(915, radm_bypass_poisoned.read());
        b.set(925, 916, radm_bypass_dw_len.read().to_uint64());
        b.set(929, 926, radm_bypass_first_be.read().to_uint64());
        b.set(933, 930, radm_bypass_last_be.read().to_uint64());
        b.set(997, 934, radm_bypass_addr.read().to_uint64());
        b.set_bit(998, radm_bypass_bcm.read());
        b.set_bit(999, radm_bypass_cpl_last.read());
        b.set(1015, 1000, radm_bypass_cmpltr_id.read().to_uint64());
        b.set(1027, 1016, radm_bypass_byte_cnt.read().to_uint64());
        b.set(1030, 1028, radm_bypass_cpl_status.read().to_uint64());
        b.set(1045, 1031, radm_bypass_rsvd.read().to_uint64());
        b.set_bit(1116, lbc_dbi_ack.read());
        b.set(1148, 1117, lbc_dbi_dout.read().to_uint64());
        b.set_bit(1161, ven_msi_grant.read());
        b.set_bit(1162, cfg_msi_en.read());
        b.set(1170, 1163, cfg_pbus_num.read().to_uint64());
        b.set(1175, 1171, cfg_pbus_dev_num.read().to_uint64());
        b.set(1183, 1176, cfg_2ndbus_num.read().to_uint64());
        b.set(1191, 1184, cfg_subbus_num.read().to_uint64());
        b.set(1207, 1192, cfg_mem_base.read().to_uint64());
        b.set(1223, 1208, cfg_mem_limit.read().to_uint64());
        b.set_bit(1228, upstream_port.read());
        b.set_bit(1229, rdlh_link_up.read());
        b.set_bit(1230, cfg_bridge_crs_en.read());
        b.set_bit(1234, radm_correctable_err.read());
        b.set_bit(1235, radm_nonfatal_err.read());
        b.set_bit(1236, radm_fatal_err.read());
    }
};
