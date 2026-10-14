

#ifndef AXI4_MASTER_SC_H
#define AXI4_MASTER_SC_H

#include <systemc.h>

#ifdef AXM_WITH_TLM
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#endif

#include <cstring>
#include <cstdio>
#include <cstdint>

#ifndef AXM_RDY_HOLD
#define AXM_RDY_HOLD 2
#endif

extern "C" unsigned int axm_bus_rd32(unsigned long long addr);
extern "C" void         axm_bus_wr32(unsigned long long addr, unsigned int data);

extern "C" void emmc_drv_irq_attach_clock(sc_core::sc_in<bool> *clk);

class axi4_master_sc;                 
extern axi4_master_sc *g_axm;         

extern volatile int b_axm_busy;

extern "C" void axm_set_map(unsigned long long reg_base,
                            unsigned long long mem_base);

#define AXM_DEF_REG_BASE  0x00000000ULL
#define AXM_DEF_MEM_BASE  0x00000000ULL

class axi4_master_sc : public sc_core::sc_module {
public:
#ifdef AXM_WITH_TLM
    
    tlm_utils::simple_target_socket<axi4_master_sc, 32> tgt_socket;
#endif

    
    sc_in<bool>       clk;
    sc_in<bool>       resetn;

    

    sc_in<bool>       irq;

    
    sc_out<bool>       awvalid;
    sc_in<bool>        awready;
    sc_out<sc_bv<32>>  awaddr;
    sc_out<sc_bv<8>>   awlen;

    
    sc_out<bool>       wvalid;
    sc_in<bool>        wready;
    sc_out<sc_bv<32>>  wdata;
    sc_out<sc_bv<4>>   wstrb;
    sc_out<bool>       wlast;

    
    sc_in<bool>        bvalid;
    sc_out<bool>       bready;
    sc_in<sc_bv<2>>    bresp;

    
    sc_out<bool>       arvalid;
    sc_in<bool>        arready;
    sc_out<sc_bv<32>>  araddr;
    sc_out<sc_bv<8>>   arlen;

    
    sc_in<bool>        rvalid;
    sc_out<bool>       rready;
    sc_in<sc_bv<32>>   rdata;
    sc_in<sc_bv<2>>    rresp;
    sc_in<bool>        rlast;

    SC_CTOR(axi4_master_sc)
#ifdef AXM_WITH_TLM
        : tgt_socket("tgt_socket")
#endif
    {
#ifdef AXM_WITH_TLM
        tgt_socket.register_b_transport(this, &axi4_master_sc::b_transport);
#endif
        SC_METHOD(drive_auto);
        sensitive << clk.pos();
        dont_initialize();

        

    }

    

    void end_of_elaboration()
    {
        g_axm = this;
        emmc_drv_irq_attach_clock(&clk);
    }

private:
    
    bool        m_active;
    bool        m_is_write;
    sc_bv<32>   m_addr;
    sc_bv<32>   m_wdata;
    sc_bv<4>    m_wstrb;
    uint32_t   *m_rdata_out;     
    uint32_t    m_rdata;         

    
    bool m_aw_done, m_w_done, m_ar_done;
    bool m_issue;                

    
    bool m_r_asserted, m_r_taken;
    int  m_r_hold;
    bool m_b_asserted, m_b_taken;
    int  m_b_hold;

    void idle()
    {
        m_active  = false;
        m_issue   = false;
        m_aw_done = m_w_done = m_ar_done = false;
        m_r_asserted = m_r_taken = false;  m_r_hold = 0;
        m_b_asserted = m_b_taken = false;  m_b_hold = 0;
    }

public:
    
    unsigned int bus_read(uint64_t addr)
    {
        uint32_t v = 0;
        (void)move(addr, &v, 4, false);
        return v;
    }
    void bus_write(uint64_t addr, uint32_t data)
    {
        (void)move(addr, &data, 4, true);
    }

private:
    

    bool move(uint64_t addr, void *buf, unsigned len, bool is_write)
    {
        uint8_t *p = reinterpret_cast<uint8_t *>(buf);
        unsigned done = 0;
        bool ok = true;

        

        while (b_axm_busy)
            wait(clk.posedge_event());
        b_axm_busy = 1;

        
        struct BusGuard {
            volatile int *f;
            ~BusGuard() { *f = 0; }
        } guard = { &b_axm_busy };

        if (!resetn.read()) {
            int g = 0;
            while (!resetn.read() && g++ < 1000)
                wait(clk.posedge_event());
            if (!resetn.read())
                return false;
        }

        while (done < len) {
            uint32_t chunk = len - done;
            if (chunk > 4u) chunk = 4u;

            m_is_write = is_write;
            m_addr     = sc_bv<32>((uint32_t)((addr + done) & 0xFFFFFFFFu));
            m_wstrb    = sc_bv<4>((chunk >= 4u) ? 0xFu : ((1u << chunk) - 1u));

            if (is_write) {
                

                uint32_t wv = 0;
                memcpy(&wv, p + done, chunk);
                m_wdata     = sc_bv<32>(wv);
                m_rdata_out = nullptr;
            } else {
                m_rdata     = 0;
                m_rdata_out = &m_rdata;
            }

            m_aw_done = m_w_done = m_ar_done = false;
            

            m_r_asserted = m_r_taken = false;  m_r_hold = 0;
            m_b_asserted = m_b_taken = false;  m_b_hold = 0;
            m_active  = true;
            m_issue   = true;

            int guard_n = 0;
            while (m_active && guard_n++ < 10000)
                wait(clk.posedge_event());

            if (m_active) { idle(); ok = false; break; }
            if (!is_write && m_rdata_out)
                memcpy(p + done, &m_rdata, chunk);

            done += chunk;
        }
        return ok;
    }

    void drive_auto() { drive_impl(true); }

    

    void drive_impl(bool write_pins)
    {
        if (!resetn.read()) {
            if (write_pins) {
                awvalid.write(false); wvalid.write(false);
                arvalid.write(false); bready.write(false); rready.write(false);
                awlen.write(0); arlen.write(0); wlast.write(false);
            }
            idle();
            return;
        }

        if (!m_active) {
            if (write_pins) {
                awvalid.write(false); wvalid.write(false);
                arvalid.write(false); bready.write(false); rready.write(false);
                awlen.write(0); arlen.write(0); wlast.write(false);
            }
            return;
        }

        
        if (m_issue) {
            m_issue = false;
            if (write_pins) {
                if (m_is_write) {
                    awaddr.write(m_addr); awlen.write(0); awvalid.write(true);
                    wdata.write(m_wdata); wstrb.write(m_wstrb);
                    wlast.write(true);    wvalid.write(true);
                } else {
                    araddr.write(m_addr); arlen.write(0); arvalid.write(true);
                }
            }
            return;
        }

        if (m_is_write) {
            
            if (!m_aw_done) {
                if (write_pins) {
                    awaddr.write(m_addr); awlen.write(0); awvalid.write(true);
                }
                if (awready.read())
                    m_aw_done = true;
                if (!m_w_done) {
                    if (write_pins) {
                        wdata.write(m_wdata); wstrb.write(m_wstrb);
                        wlast.write(true);    wvalid.write(true);
                    }
                    if (wready.read())
                        m_w_done = true;
                }
                return;
            }
            if (write_pins) awvalid.write(false);

            if (!m_w_done) {
                if (write_pins) {
                    wdata.write(m_wdata); wstrb.write(m_wstrb);
                    wlast.write(true);    wvalid.write(true);
                }
                if (wready.read())
                    m_w_done = true;
                return;
            }
            if (write_pins) { wvalid.write(false); wlast.write(false); }

            

            if (!m_b_asserted) {
                if (write_pins) bready.write(true);
                m_b_asserted = true;      
                return;
            }
            if (!m_b_taken) {
                if (bvalid.read()) {
                    m_b_taken = true;
                    m_b_hold  = AXM_RDY_HOLD;
                }
                return;
            }
            if (m_b_hold > 0) { m_b_hold--; return; }
            if (write_pins) bready.write(false);
            m_active = false;
        } else {
            
            if (!m_ar_done) {
                if (write_pins) {
                    araddr.write(m_addr); arlen.write(0); arvalid.write(true);
                }
                if (arready.read())
                    m_ar_done = true;
                return;
            }
            if (write_pins) arvalid.write(false);

            

            if (!m_r_asserted) {
                if (write_pins) rready.write(true);
                m_r_asserted = true;      
                return;
            }
            if (!m_r_taken) {
                if (rvalid.read()) {
                    if (m_rdata_out)
                        *m_rdata_out = rdata.read().to_uint();
                    m_r_taken = true;
                    m_r_hold  = AXM_RDY_HOLD;
                }
                return;
            }
            if (m_r_hold > 0) { m_r_hold--; return; }
            if (write_pins) rready.write(false);
            m_active = false;
        }
    }

#ifdef AXM_WITH_TLM
    

    void b_transport(tlm::tlm_generic_payload &gp, sc_core::sc_time &delay)
    {
        if (delay != sc_core::SC_ZERO_TIME) {
            sc_core::sc_time d = delay;
            delay = sc_core::SC_ZERO_TIME;
            wait(d);
        }

        unsigned len = gp.get_data_length();
        if (len == 0)
            len = 4;

        bool ok = move(gp.get_address(), gp.get_data_ptr(), len,
                       gp.get_command() == tlm::TLM_WRITE_COMMAND);

        gp.set_response_status(ok ? tlm::TLM_OK_RESPONSE
                                  : tlm::TLM_GENERIC_ERROR_RESPONSE);
    }
#endif 
};

#endif 
