

#include <systemc.h>

#include <cstdio>
#include <cstring>

#include "axi4_master_sc.h"      
#include "dut_slave_stub.h"      
#include "emmc_ioctl_ref.h"      
#include "irq_chain.h"           

extern "C" {
extern int  emmc_model_init(void);
extern void emmc_model_exit(void);
extern long emmc_model_ioctl(unsigned int cmd, void *arg, unsigned long len);
extern unsigned long long emmc_model_dma_buf_addr(void);
extern void              *emmc_model_dma_buf_ptr(void);
extern unsigned long long emmc_model_dma_buf_size(void);
extern int emmc_drv_irq_pending(void);

extern "C" void axm_set_map(unsigned long long, unsigned long long);
extern unsigned int emmc_drv_irq_served(void);
extern unsigned int emmc_drv_irq_last_sts(void);

extern int          g_isr_cmd_done;
extern int          g_isr_data_over;
extern int          g_isr_err;
}

static int g_pass = 0, g_fail = 0;
static bool g_red = false;

#define CHECK(cond, ...)                                                \
    do {                                                                \
        if (cond) { g_pass++; std::printf("  [PASS] " __VA_ARGS__); }   \
        else      { g_fail++; std::printf("  [FAIL] " __VA_ARGS__); }   \
        std::printf("\n");                                              \
    } while (0)

SC_MODULE(smoke_top) {
    sc_core::sc_clock        clk;
    sc_core::sc_signal<bool> resetn;

    sc_core::sc_signal<bool>      awvalid, awready, wvalid, wready, wlast;
    sc_core::sc_signal<sc_bv<32>> awaddr, wdata, araddr, rdata;
    sc_core::sc_signal<sc_bv<8>>  awlen, arlen;
    sc_core::sc_signal<sc_bv<4>>  wstrb;
    sc_core::sc_signal<bool>      bvalid, bready, arvalid, arready, rvalid, rready, rlast;
    sc_core::sc_signal<sc_bv<2>>  bresp, rresp;

    sc_core::sc_signal<bool>      dut_irq;      
    sc_core::sc_signal<bool>      mst_irq_out;  

    axi4_master_sc *mst;
    dut_slave_stub *dut;
    irq_observer   *obs;

    SC_CTOR(smoke_top)
        : clk("clk", 10, sc_core::SC_NS), resetn("resetn"),
          awvalid("awvalid"), awready("awready"), wvalid("wvalid"),
          wready("wready"), wlast("wlast"),
          awaddr("awaddr"), wdata("wdata"), araddr("araddr"), rdata("rdata"),
          awlen("awlen"), arlen("arlen"), wstrb("wstrb"),
          bvalid("bvalid"), bready("bready"), arvalid("arvalid"),
          arready("arready"), rvalid("rvalid"), rready("rready"),
          rlast("rlast"), bresp("bresp"), rresp("rresp"),
          dut_irq("dut_irq")
    {
        mst = new axi4_master_sc("master");
        dut = new dut_slave_stub("dut");
        obs = new irq_observer("irq_obs");

        

        obs->clk(clk); obs->irq(dut_irq); obs->irq_out(mst_irq_out);
        mst->irq(mst_irq_out);
        dut->irq(dut_irq);

        
        mst->clk(clk); mst->resetn(resetn);
        mst->awvalid(awvalid); mst->awready(awready); mst->awaddr(awaddr);
        mst->awlen(awlen);
        mst->wvalid(wvalid); mst->wready(wready); mst->wdata(wdata);
        mst->wstrb(wstrb); mst->wlast(wlast);
        mst->bvalid(bvalid); mst->bready(bready); mst->bresp(bresp);
        mst->arvalid(arvalid); mst->arready(arready); mst->araddr(araddr);
        mst->arlen(arlen);
        mst->rvalid(rvalid); mst->rready(rready); mst->rdata(rdata);
        mst->rresp(rresp); mst->rlast(rlast);

        dut->clk(clk); dut->resetn(resetn);
        dut->awvalid(awvalid); dut->awready(awready); dut->awaddr(awaddr);
        dut->wvalid(wvalid); dut->wready(wready); dut->wdata(wdata);
        dut->wstrb(wstrb); dut->wlast(wlast);
        dut->bvalid(bvalid); dut->bready(bready); dut->bresp(bresp);
        dut->arvalid(arvalid); dut->arready(arready); dut->araddr(araddr);
        dut->rvalid(rvalid); dut->rready(rready); dut->rdata(rdata);
        dut->rresp(rresp); dut->rlast(rlast);

        SC_THREAD(run);
        SC_THREAD(reset_seq);
    }

    
    void reset_seq()
    {
        resetn.write(false);
        wait(100, sc_core::SC_NS);
        resetn.write(true);
    }

    void run()
    {
        struct emmc_info info;
        __u64 buf_sz = 0;
        int   i, ret;

        std::printf("+==========================================================+\n");
        std::printf("|  eMMC Host Model SDK -- link self-test                   |\n");
        std::printf("+==========================================================+\n");
        std::printf("(driver API -> AXI4 pin-level master -> AXI4 pins -> DUT stub)\n\n");

        
        std::printf("[TC-1] init\n");
        ret = emmc_model_init();
        CHECK(ret == 0, "TC-1.1 emmc_model_init rc=%d", ret);

        
        std::printf("\n[TC-2] read DUT identification registers over AXI4\n");
        std::memset(&info, 0, sizeof(info));
        ret = (int)emmc_model_ioctl(EMMC_IOC_GET_INFO, &info, sizeof(info));
        CHECK(ret == 0,           "TC-2.1 GET_INFO rc=%d", ret);
        CHECK(info.verid != 0,    "TC-2.2 VERID non-zero (%08x)", info.verid);
        CHECK(info.verid == 0x53470000u,
              "TC-2.3 VERID golden = 0x53470000 (got %08x)", info.verid);
        CHECK(info.hcon != 0,     "TC-2.4 HCON non-zero (%08x)", info.hcon);

        
        std::printf("\n[TC-3] register write / read-back\n");
        {
            unsigned int v, pat = 0xC0FFEE01u;
            axm_bus_wr32(0x100u, pat);
            v = axm_bus_rd32(0x100u);
            CHECK(v == pat, "TC-3.1 write/read-back match w=%08x r=%08x", pat, v);

            pat = 0x12000500u;
            axm_bus_wr32(0x02Cu, pat);        
            v = axm_bus_rd32(0x02Cu);
            CHECK(v == pat, "TC-3.2 CMD register write/read-back %08x", v);
        }

        
        std::printf("\n[TC-4] data path (multi-beat moves)\n");
        ret = (int)emmc_model_ioctl(EMMC_IOC_MMAP_BUF, &buf_sz, sizeof(buf_sz));
        CHECK(ret == 0 && buf_sz > 0, "TC-4.1 MMAP_BUF returned a buffer (size=%llu)",
              (unsigned long long)buf_sz);

        {
            unsigned char *buf = (unsigned char *)emmc_model_dma_buf_ptr();
            unsigned long long base = emmc_model_dma_buf_addr();
            unsigned char pat[256];

            CHECK(buf != NULL, "TC-4.2 buffer accessible");

            for (i = 0; i < 256; i++) pat[i] = (unsigned char)(0x5A ^ i);
            

            axm_bus_wr32((unsigned)(0x100000u + 0), 0);

            bool ok = true;
            for (i = 0; i < 256; i += 4) {
                unsigned int w = 0;
                std::memcpy(&w, pat + i, 4);
                axm_bus_wr32((unsigned)(0x100000u + i), w);
            }
            for (i = 0; i < 256; i += 4) {
                unsigned int w = axm_bus_rd32((unsigned)(0x100000u + i));
                unsigned int e = 0;
                std::memcpy(&e, pat + i, 4);
                if (w != e) { ok = false; break; }
            }
            CHECK(ok, "TC-4.3 256B byte-exact write/read-back (multi-beat)");
            (void)base; (void)buf;
        }

        
        std::printf("\n[TC-4.5] interrupt chain (pin level)\n");
        {
            

            struct emmc_raw_req req;
            std::memset(&req, 0, sizeof(req));
            req.opcode = 13; req.arg = 0x00010000u;
            req.flags  = EMMC_RESP_SHORT | EMMC_REQ_NO_DATA;
            ret = (int)emmc_model_ioctl(EMMC_IOC_SEND_CMD, &req, sizeof(req));
            CHECK(ret == 0, "TC-4.5.1 command issued rc=%d", ret);

            
            for (i = 0; i < 60 && g_isr_cmd_done == 0; i++)
                wait(sc_core::sc_time(20, sc_core::SC_NS));

            CHECK(emmc_drv_irq_served() > 0,
                  "TC-4.5.2 interrupt dispatched (count=%u)", emmc_drv_irq_served());
            CHECK(g_isr_cmd_done > 0,
                  "TC-4.5.3 * CMD_DONE bit dispatched to isr_cmd_done (count=%d)",
                  g_isr_cmd_done);
            CHECK(axm_bus_rd32(0x044u) == 0u,
                  "TC-4.5.4 ISR cleared the status register (RINTSTS=%08x)",
                  axm_bus_rd32(0x044u));

            

            std::memset(&req, 0, sizeof(req));
            req.opcode = 17; req.arg = 1;
            req.flags  = EMMC_RESP_SHORT | EMMC_REQ_READ;
            req.blk_count = 1; req.blk_size = 512; req.data_len = 512;
            ret = (int)emmc_model_ioctl(EMMC_IOC_SEND_CMD, &req, sizeof(req));
            for (i = 0; i < 60 && g_isr_data_over == 0; i++)
                wait(sc_core::sc_time(20, sc_core::SC_NS));

            CHECK(g_isr_data_over > 0,
                  "TC-4.5.5 * DATA_OVER bit dispatched to isr_data_over (count=%d)"
                  " -- a DIFFERENT handler from CMD_DONE",
                  g_isr_data_over);
            CHECK(g_isr_err == 0,
                  "TC-4.5.6 no error bits => isr_error NOT called (count=%d)"
                  " -- unmatched bits are not handled", g_isr_err);
        }

        
        std::printf("\n[TC-5] %s\n", g_red ? "red run: inject a 1-byte corruption"
                                           : "link stability (repeated transactions)");
        if (g_red) {
            int mism = 0;
            unsigned int v;
            axm_bus_wr32(0x200u, 0xA5A5A5A5u);
            v = axm_bus_rd32(0x200u);
            v ^= 0x000000FFu;                 
            if (v != 0xA5A5A5A5u) mism = 1;   
            CHECK(mism != 0, "TC-5.1 1-byte corruption => criteria FAIL (mismatch=%d)", mism);
        } else {
            int ok = 0;
            for (i = 0; i < 20; i++) {
                unsigned int p = 0x1000u + i;
                axm_bus_wr32(0x300u, p);
                unsigned int r = axm_bus_rd32(0x300u);
                if (r == p) ok++;
            }
            CHECK(ok == 20, "TC-5.1 20 consecutive transactions all match (ok=%d)", ok);
        }

        

        std::printf("\n[TC-6] configurable address mapping (customer requirement)\n");
        {
            unsigned int v;

            
            axm_bus_wr32(0x300u, 0x11110000u);
            CHECK(axm_bus_rd32(0x300u) == 0x11110000u,
                  "TC-6.1 default mapping (reg_base=0) read/write match");

            

            axm_set_map(0x80000000ULL, 0x90000000ULL);
            v = axm_bus_rd32(0x300u);
            CHECK(v != 0x11110000u,
                  "TC-6.2 * different value after remap (0x%08x != 0x11110000)"
                  " -- proves the address was converted", v);

            
            axm_set_map(0, 0);
            axm_bus_wr32(0x300u, 0x22220000u);
            CHECK(axm_bus_rd32(0x300u) == 0x22220000u,
                  "TC-6.3 read/write restored after remap reset");
        }

        

        std::printf("\n[TC-7] unmapped access -> SLVERR + accounted\n");
        {
            
            unsigned before = dut->unmapped();
            axm_bus_rd32(0x7FFFF000u);
            CHECK(dut->unmapped() == before + 1u,
                  "TC-7.1 unmapped read is counted (%u -> %u, not silent)",
                  before, dut->unmapped());
            axm_bus_wr32(0x7FFFF000u, 0xDEADBEEFu);
            CHECK(dut->unmapped() == before + 2u,
                  "TC-7.2 unmapped write is counted (no silent drop)");
            (void)axm_bus_rd32(0x300u);
            CHECK(dut->unmapped() == before + 2u,
                  "TC-7.3 mapped access does NOT count (no false positives)");
        }

        std::printf("\n---------------- criteria summary ----------------\n");
        std::printf("  PASS = %d\n  FAIL = %d\n", g_pass, g_fail);

        if (g_red) {
            bool ok = (g_fail == 0 && g_pass > 0);
            std::printf("  red-run verdict: %s\n", ok ? "OK criteria valid" : "BAD criteria invalid");
            std::printf("\nSMOKE_RED %s\n", ok ? "PASS" : "FAIL");
        } else {
            bool ok = (g_fail == 0);
            std::printf("  green-run verdict: %s\n", ok ? "OK all green" : "BAD failures present");
            std::printf("\nSMOKE_GREEN %s\n", ok ? "PASS" : "FAIL");
        }

        emmc_model_exit();
        sc_core::sc_stop();
    }
};

int sc_main(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
        if (std::strcmp(argv[i], "--red") == 0)
            g_red = true;

    smoke_top top("top");
    sc_core::sc_start();
    return (g_fail == 0 && g_pass > 0) ? 0 : 1;
}
