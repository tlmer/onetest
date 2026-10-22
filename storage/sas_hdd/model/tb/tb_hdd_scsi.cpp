

#include <systemc.h>
#include "sas_hdd_tlm.h"
#include "sas_hdd_regs.h"
#include <cstdio>
#include <cstring>
#include <vector>

struct TbHdd : sc_core::sc_module {
    sc_core::sc_in<bool> clk;
    sc_core::sc_out<bool> rst_n;

    sc_core::sc_signal<sc_uint<32>> rx_data;  sc_core::sc_signal<bool> rx_valid, rx_sof, rx_eof;
    sc_core::sc_signal<sc_uint<4>>  rx_type;  sc_core::sc_signal<bool> rx_ready;
    sc_core::sc_signal<sc_uint<32>> tx_data;  sc_core::sc_signal<bool> tx_valid, tx_sof, tx_eof;
    sc_core::sc_signal<sc_uint<4>>  tx_type;  sc_core::sc_signal<bool> tx_ready;
    sc_core::sc_signal<sc_uint<32>> o_rxf, o_txf, o_good, o_chk, o_rd, o_wr, o_dbg;
    sc_core::sc_signal<sc_uint<8>>  o_op, o_st;

    SasHddTlm *hdd = nullptr;
    int pass_cnt = 0, fail_cnt = 0;

    
    struct CapFrame { std::vector<uint32_t> dw; uint8_t ft; };
    std::vector<CapFrame> cap;
    std::vector<uint32_t> cur;
    bool cap_busy = false;

    SC_HAS_PROCESS(TbHdd);
    TbHdd(sc_core::sc_module_name nm) : sc_module(nm), clk("clk"), rst_n("rst_n") {
        SC_THREAD(run);
    }
    void chk(bool ok, const char *name) {
        printf("   %s %s\n", ok ? "✓" : "✗", name);
        if (ok) pass_cnt++; else fail_cnt++;
    }

    
    void sample_tx() {
        if (tx_valid.read() && tx_ready.read()) {
            if (tx_sof.read()) { cur.clear(); cap_busy = true; }
            cur.push_back(tx_data.read().to_uint());
            if (tx_eof.read()) {
                CapFrame f; f.dw = cur; f.ft = (uint8_t)tx_type.read().to_uint();
                cap.push_back(f);
                cap_busy = false;
                cur.clear();
            }
        }
    }
    void cycle() { wait(clk.posedge_event()); wait(sc_core::SC_ZERO_TIME); sample_tx(); }
    void cycles(int n) { for (int i = 0; i < n; i++) cycle(); }

    
    void send_frame(const uint32_t *dw, int n, uint8_t ft) {
        for (int i = 0; i < n; i++) {
            rx_data.write(sc_uint<32>(dw[i]));
            rx_type.write(sc_uint<4>(ft));
            rx_valid.write(true);
            rx_sof.write(i == 0);
            rx_eof.write(i == n - 1);
            cycle();                       
        }
        rx_valid.write(false); rx_sof.write(false); rx_eof.write(false);
    }
    
    void cmd_frame(uint32_t dw[14], const uint8_t cdb[16], const uint8_t lun[8], uint16_t tag) {
        for (int i = 0; i < 14; i++) dw[i] = 0;
        dw[0] = SAS_FRAME_B0_COMMAND;
        dw[4] = ((uint32_t)(tag & 0xFF) << 8) | ((uint32_t)(tag >> 8) & 0xFF);
        dw[6] = ((uint32_t)lun[0] << 24) | ((uint32_t)lun[1] << 16) | ((uint32_t)lun[2] << 8) | lun[3];
        dw[7] = ((uint32_t)lun[4] << 24) | ((uint32_t)lun[5] << 16) | ((uint32_t)lun[6] << 8) | lun[7];
        for (int i = 0; i < 4; i++)
            dw[9 + i] = ((uint32_t)cdb[i * 4] << 0) | ((uint32_t)cdb[i * 4 + 1] << 8)
                      | ((uint32_t)cdb[i * 4 + 2] << 16) | ((uint32_t)cdb[i * 4 + 3] << 24);
    }
    uint8_t payload_byte(const CapFrame &f, int k) const {
        uint32_t d = f.dw[SAS_DATA_HDR_DW + k / 4];
        return (uint8_t)((d >> (8 * (k % 4))) & 0xFF);      
    }
    int run_cmd(const uint8_t cdb[16], const uint8_t lun[8], uint16_t tag, int expect_frames) {
        uint32_t dw[14];
        cmd_frame(dw, cdb, lun, tag);
        int base = (int)cap.size();
        send_frame(dw, 14, SAS_FT_COMMAND);
        int cap_cycles = 4000;
        while ((int)cap.size() - base < expect_frames && cap_cycles-- > 0) cycle();
        cycles(4);                            
        return base;                          
    }

    void run() {
        printf("[TB] pre-registered criteria D1..D10 (HDD byte level)\n");
        const uint8_t lun0[8] = {0,0,0,0,0,0,0,0};
        const uint8_t lun1[8] = {0,0,0,0,0,0,0,1};
        uint8_t cdb[16];
        
        for (int i = 0; i < 4; i++) {
            uint8_t *p = hdd->lba_ptr(0x10 + i);
            for (int k = 0; k < 512; k++) p[k] = (uint8_t)((0x10 + i) * 512 + k) ^ 0x5A;
        }

        tick_boot();

        
        {
            memset(cdb, 0, 16);
            int b = run_cmd(cdb, lun0, 0x1234, 1);
            const CapFrame &f = cap[b];
            chk(cap.size() == (size_t)(b + 1) && f.ft == SAS_FT_RESPONSE && f.dw.size() == SAS_RESP_FRAME_DW,
                "D1 TUR: exactly 1 RESPONSE frame (12 dw, frame type 4)");
            chk((f.dw[0] & 0xFFu) == 0x40, "D1 RESPONSE dw0 byte0 = 0x40");
            chk(((f.dw[4] & 0xFFu) == 0x12) && (((f.dw[4] >> 8) & 0xFFu) == 0x34),
                "D1 tag echo = 0x1234 (be16: byte16=0x12/byte17=0x34)");
            chk(((f.dw[SAS_RESP_DW_STATUS] >> 24) & 0xFFu) == 0x00, "D1 byte35 STATUS = 0x00（GOOD ✓）");
        }
        
        {
            memset(cdb, 0, 16); cdb[0] = 0x12; cdb[4] = 36;
            int b = run_cmd(cdb, lun0, 0x2001, 2);
            const CapFrame &fd = cap[b], &fr = cap[b + 1];
            chk(fd.ft == SAS_FT_DATA && fd.dw.size() == SAS_DATA_HDR_DW + 9,
                "D2 INQUIRY: DATA frame = 6+9 dw (36B)");
            chk((fd.dw[0] & 0xFFu) == 0x60, "D2 DATA dw0 byte0 = 0x60");
            chk(payload_byte(fd, 4) == 32, "D2 payload byte4 = 32 (RTL INQ_STD_LEN-4)");
            chk(memcmp(&fd.dw[SAS_DATA_HDR_DW + 2], "-UME MIS", 8) == 0, "D2 vendor = -UME MIS");
            {
                uint8_t prod[12];
                for (int i = 0; i < 12; i++) prod[i] = payload_byte(fd, 16 + i);
                chk(memcmp(prod, "-SAS-DHVM652", 12) == 0, "D2 product = -SAS-DHVM652");
            }
            chk((payload_byte(fd, 0) & 0xE0) == 0x00, "D2 byte0 PQ=0（LUN=0 ✓）");
            chk(fr.ft == SAS_FT_RESPONSE, "D2 RESPONSE follows");
        }
        
        {
            memset(cdb, 0, 16); cdb[0] = 0x12; cdb[4] = 36;
            int b = run_cmd(cdb, lun1, 0x2002, 2);
            chk((payload_byte(cap[b], 0) & 0xE0) == 0x60, "D3 LUN!=0 => PQ=011b (byte0 -> 0x60; defect #)");
        }
        
        {
            memset(cdb, 0, 16); cdb[0] = 0x1A; cdb[2] = 0x19; cdb[4] = 0x40;
            int b = run_cmd(cdb, lun0, 0x2003, 2);
            const CapFrame &fd = cap[b];
            chk(fd.dw.size() == SAS_DATA_HDR_DW + 16, "D4 MODE SENSE: DATA payload 64B (16 dw; defect #)");
            chk(payload_byte(fd, 0) == 0x3F && payload_byte(fd, 4) == 0x08 && payload_byte(fd, 5) == 0x12,
                "D4 byte0=0x3F / byte4=0x08 / byte5=0x12 ✓");
        }
        
        {
            memset(cdb, 0, 16); cdb[0] = 0x25;
            int b = run_cmd(cdb, lun0, 0x2004, 2);
            const CapFrame &fd = cap[b];
            chk(fd.dw.size() == SAS_DATA_HDR_DW + 2, "D5 RCAP10: DATA payload 8B");
            chk(payload_byte(fd, 0) == 0 && payload_byte(fd, 1) == 0 && payload_byte(fd, 2) == 0 &&
                payload_byte(fd, 3) == 255, "D5 last_lba = 255（BE ✓ = blocks-1 ✓）");
            chk(payload_byte(fd, 4) == 0x00 && payload_byte(fd, 5) == 0x00 &&
                payload_byte(fd, 6) == 0x02 && payload_byte(fd, 7) == 0x00,
                "D5 block length = 0x00000200 (BE @byte4-7; RTL source)");
        }
        
        {
            memset(cdb, 0, 16); cdb[0] = 0xA0;
            int b = run_cmd(cdb, lun0, 0x2005, 2);
            const CapFrame &fd = cap[b];
            chk(fd.dw.size() == SAS_DATA_HDR_DW + 16 && payload_byte(fd, 3) == 0x08,
                "D6 REPORT LUNS: 64B, byte3 = 0x08 (length field BE; defect #)");
        }
        
        {
            memset(cdb, 0, 16); cdb[0] = 0x28; cdb[5] = 0x10; cdb[8] = 0x04;
            int b = run_cmd(cdb, lun0, 0x2006, 4);
            chk(cap.size() >= (size_t)(b + 4) && cap[b].ft == SAS_FT_DATA &&
                cap[b + 1].ft == SAS_FT_DATA && cap[b + 2].ft == SAS_FT_DATA &&
                cap[b + 3].ft == SAS_FT_RESPONSE,
                "D7 READ 2048B: 3 DATA frames + 1 RESPONSE (>1000B split)");
            
            uint32_t o0 = cap[b].dw[SAS_DATA_DW_OFFSET], o1 = cap[b + 1].dw[SAS_DATA_DW_OFFSET],
                     o2 = cap[b + 2].dw[SAS_DATA_DW_OFFSET];
            chk(o0 == 0 && o1 == 0xE8030000u && o2 == 0xD0070000u,
                "D7 DataOffset = 0/1000/2000 (be32 encoded)");
            
            bool ok = true;
            int n = 0;
            for (int f = 0; f < 3 && ok; f++) {
                int pldw = (int)cap[b + f].dw.size() - SAS_DATA_HDR_DW;
                for (int i = 0; i < pldw * 4; i++) {
                    uint8_t exp = (uint8_t)(0x10 * 512 + n + i) ^ 0x5A;
                    if (n + i >= 2048) break;
                    if (payload_byte(cap[b + f], i) != exp) { ok = false; break; }
                }
                n += pldw * 4;
            }
            chk(ok && n == 2048, "D7 payload 2048B byte-exact == store");
        }
        
        {
            memset(cdb, 0, 16); cdb[0] = 0x7F;
            int b = run_cmd(cdb, lun0, 0x2007, 1);
            const CapFrame &f = cap[b];
            chk(f.ft == SAS_FT_RESPONSE && ((f.dw[SAS_RESP_DW_STATUS] >> 24) & 0xFFu) == 0x02,
                "D8 unknown opcode => RESPONSE byte35 = 0x02 (CHECK CONDITION; defect #)");
        }
        
        {
            const uint8_t ops[3] = {0x1B, 0x35, 0x15};
            bool ok = true;
            for (int i = 0; i < 3; i++) {
                memset(cdb, 0, 16); cdb[0] = ops[i];
                int b = run_cmd(cdb, lun0, (uint16_t)(0x2100 + i), 1);
                const CapFrame &f = cap[b];
                ok &= (f.ft == SAS_FT_RESPONSE) && (((f.dw[SAS_RESP_DW_STATUS] >> 24) & 0xFFu) == 0x00);
            }
            chk(ok, "D9 START STOP/SYNC CACHE/MODE SELECT => RESPONSE GOOD, no DATA");
        }
        
        chk(hdd->o_cmds_good.read().to_uint() == 10u && hdd->o_cmds_chk.read().to_uint() == 1u,
            "D10 o_cmds_good=10 / o_cmds_chk=1 (11 commands)");
        chk(hdd->o_lba_read.read().to_uint() == 4u, "D10 o_lba_read = 4");

        printf("[TOTAL] PASS=%d FAIL=%d\n", pass_cnt, fail_cnt);
        sc_core::sc_stop();
    }
    void tick_boot() {
        cycles(4);
        while (!rst_n.read()) cycle();
        cycles(4);
    }
};

int sc_main(int argc, char **argv) {
    (void)argc; (void)argv;
    sc_core::sc_clock clk("clk", 10, sc_core::SC_NS);
    sc_core::sc_signal<bool> rst_n;
    SasHddTlm hdd("hdd", 256);                 
    TbHdd tb("tb");
    tb.clk(clk); tb.rst_n(rst_n); tb.hdd = &hdd;
    hdd.clk(clk); hdd.rst_n(rst_n);
    hdd.rx_data(tb.rx_data); hdd.rx_valid(tb.rx_valid); hdd.rx_sof(tb.rx_sof);
    hdd.rx_eof(tb.rx_eof); hdd.rx_type(tb.rx_type); hdd.rx_ready(tb.rx_ready);
    hdd.tx_data(tb.tx_data); hdd.tx_valid(tb.tx_valid); hdd.tx_sof(tb.tx_sof);
    hdd.tx_eof(tb.tx_eof); hdd.tx_type(tb.tx_type); hdd.tx_ready(tb.tx_ready);
    hdd.o_rx_frames(tb.o_rxf); hdd.o_tx_frames(tb.o_txf);
    hdd.o_cmds_good(tb.o_good); hdd.o_cmds_chk(tb.o_chk);
    hdd.o_lba_read(tb.o_rd); hdd.o_lba_write(tb.o_wr);
    hdd.o_last_opcode(tb.o_op); hdd.o_last_status(tb.o_st);
    hdd.o_dbg_state(tb.o_dbg);
    tb.tx_ready.write(true);                   
    rst_n.write(false);
    sc_core::sc_start(200, sc_core::SC_NS);
    rst_n.write(true);
    sc_core::sc_start();
    int rc = (tb.fail_cnt == 0) ? 0 : 1;
    printf("%s\n", rc == 0 ? "TB_HDD_SCSI PASS" : "TB_HDD_SCSI FAIL");
    return rc;
}
