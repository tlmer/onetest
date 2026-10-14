

#ifndef DUT_SLAVE_STUB_H
#define DUT_SLAVE_STUB_H

#include <systemc.h>
#include <cstdio>
#include <cstring>

class dut_slave_stub : public sc_core::sc_module {
public:
    
    sc_in<bool>        clk;
    sc_in<bool>        resetn;

    
    sc_in<bool>        awvalid;
    sc_out<bool>       awready;
    sc_in<sc_bv<32>>   awaddr;

    
    sc_in<bool>        wvalid;
    sc_out<bool>       wready;
    sc_in<sc_bv<32>>   wdata;
    sc_in<sc_bv<4>>    wstrb;
    sc_in<bool>        wlast;

    
    sc_out<bool>       bvalid;
    sc_in<bool>        bready;
    sc_out<sc_bv<2>>   bresp;

    
    sc_in<bool>        arvalid;
    sc_out<bool>       arready;
    sc_in<sc_bv<32>>   araddr;

    
    sc_out<bool>       rvalid;
    sc_in<bool>        rready;
    sc_out<sc_bv<32>>  rdata;
    sc_out<sc_bv<2>>   rresp;
    sc_out<bool>       rlast;

    

    sc_out<bool>       irq;

    
    static const uint32_t REG_SIZE  = 0x10000;    
    static const uint32_t MEM_SIZE  = (8u << 20); 

    SC_CTOR(dut_slave_stub)
        : clk("clk"), resetn("resetn"),
          awvalid("awvalid"), awready("awready"), awaddr("awaddr"),
          wvalid("wvalid"), wready("wready"), wdata("wdata"),
          wstrb("wstrb"), wlast("wlast"),
          bvalid("bvalid"), bready("bready"), bresp("bresp"),
          arvalid("arvalid"), arready("arready"), araddr("araddr"),
          rvalid("rvalid"), rready("rready"), rdata("rdata"),
          rresp("rresp"), rlast("rlast"), irq("irq")
    {
        SC_METHOD(step);
        sensitive << clk.pos();
        dont_initialize();

        SC_METHOD(drive);
        sensitive << clk.pos();
        dont_initialize();

        m_st = ST_IDLE;
        memset(m_mem, 0, sizeof(m_mem));
        memset(m_reg, 0, sizeof(m_reg));
        
        m_reg[0x06Cu / 4] = 0x53470000u;       
        m_reg[0x070u / 4] = (512u << 8) | 1u;  
    }

    
    uint8_t *mem_ptr() { return m_mem; }

private:
    enum st_e { ST_IDLE, ST_BV, ST_RV };
    st_e     m_st;
    uint32_t m_rdata = 0, m_bresp = 0, m_rresp = 0;

    uint32_t m_reg[REG_SIZE / 4];
    uint8_t  m_mem[MEM_SIZE];

    

    

    unsigned m_unmapped = 0;
public:
    unsigned unmapped() const { return m_unmapped; }
private:

    int bus_read(uint32_t a, uint32_t *out)
    {
        if (a < REG_SIZE) { *out = m_reg[(a & (REG_SIZE - 1)) / 4]; return 0; }
        if (a + 4u <= MEM_SIZE) {
            uint32_t v = 0;
            memcpy(&v, &m_mem[a], 4);
            *out = v;
            return 0;
        }
        m_unmapped++;
        *out = 0xFFFFFFFFu;                       
        return -1;
    }

    int bus_write(uint32_t a, uint32_t d, uint32_t strb)
    {
        if (a < REG_SIZE) {
            uint32_t off = a & (REG_SIZE - 1);

            

            if (off == 0x02Cu && (d & 0x80000000u)) {
                m_reg[0x044u / 4] |= 0x00000004u;      
                if (d & ((1u << 9) | (1u << 10)))      
                    m_reg[0x044u / 4] |= 0x00000008u;  
            }

            
            if (off == 0x044u)
                m_reg[0x044u / 4] &= ~d;

            uint32_t cur = m_reg[off / 4], nv = 0;
            for (int b = 0; b < 4; b++) {
                uint32_t mask = 0xFFu << (b * 8);
                nv |= (strb & (1u << b)) ? (d & mask) : (cur & mask);
            }
            if (off != 0x044u)                         
                m_reg[off / 4] = nv;
            return 0;
        }
        if (a + 4u <= MEM_SIZE) {
            for (int b = 0; b < 4; b++)
                if (strb & (1u << b))
                    m_mem[a + b] = (uint8_t)((d >> (b * 8)) & 0xFFu);
            return 0;
        }
        m_unmapped++;                                  
        return -1;
    }

    

    void step()
    {
        if (!resetn.read()) { m_st = ST_IDLE; return; }

        switch (m_st) {
        case ST_IDLE:
            if (awvalid.read() && wvalid.read()) {
                m_bresp = (bus_write(awaddr.read().to_uint(), wdata.read().to_uint(),
                                     wstrb.read().to_uint()) == 0) ? 0u : 2u;   
                m_st = ST_BV;
            } else if (arvalid.read()) {
                m_rresp = (bus_read(araddr.read().to_uint(), &m_rdata) == 0) ? 0u : 2u;
                m_st = ST_RV;
            }
            break;
        case ST_BV:
            if (bready.read()) m_st = ST_IDLE;
            break;
        case ST_RV:
            if (rready.read()) m_st = ST_IDLE;
            break;
        }
    }

    
    void drive()
    {
        awready.write(true);
        wready.write(true);
        arready.write(true);
        bvalid.write(m_st == ST_BV);
        bresp.write(sc_bv<2>(m_bresp));
        rvalid.write(m_st == ST_RV);
        rdata.write(sc_bv<32>(m_rdata));
        rresp.write(sc_bv<2>(m_rresp));
        rlast.write(true);

        

        irq.write((m_reg[0x044u / 4] & 0x00000004u) != 0);
    }
};

#endif 
