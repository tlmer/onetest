/* SPDX-License-Identifier: GPL-2.0 */
/*
 * dut_slave_stub.h -- DUT-side AXI4 slave skeleton
 *
 *   REPLACE THIS with your eMMC host controller. The shape below is what
 *   the SDK's AXI4 master expects.
 *
 *   This placeholder is a minimal AXI4 slave containing:
 *     - a 64KB register window (for the identification registers the SDK
 *       reads, and for command/status traffic)
 *     - an 8MB memory region (for DMA buffers)
 *   Its port list is identical to your DUT's, so swapping it in is a
 *   drop-in replacement.
 *
 *   The port contract is standard AXI4, 32-bit data, single beat.
 *
 * -- Handshake discipline (copy this into your DUT wrapper; real hardware
 *    must follow it too) -----------------------------------------------------
 *    1. valid, once raised, must stay asserted until accepted by ready
 *    2. the ready side asserts first (the master is always ready first, the
 *       slave then returns valid)
 *    3. both sides must not de-assert on the same beat -- that becomes a
 *       mutual-withdraw race and silently drops data
 *   This skeleton already implements the discipline.
 */
#ifndef DUT_SLAVE_STUB_H
#define DUT_SLAVE_STUB_H

#include <systemc.h>
#include <cstdio>
#include <cstring>

class dut_slave_stub : public sc_core::sc_module {
public:
    /* -- clock / reset -- */
    sc_in<bool>        clk;
    sc_in<bool>        resetn;

    /* -- write address -- */
    sc_in<bool>        awvalid;
    sc_out<bool>       awready;
    sc_in<sc_bv<32>>   awaddr;

    /* -- write data -- */
    sc_in<bool>        wvalid;
    sc_out<bool>       wready;
    sc_in<sc_bv<32>>   wdata;
    sc_in<sc_bv<4>>    wstrb;
    sc_in<bool>        wlast;

    /* -- write response -- */
    sc_out<bool>       bvalid;
    sc_in<bool>        bready;
    sc_out<sc_bv<2>>   bresp;

    /* -- read address -- */
    sc_in<bool>        arvalid;
    sc_out<bool>       arready;
    sc_in<sc_bv<32>>   araddr;

    /* -- read data -- */
    sc_out<bool>       rvalid;
    sc_in<bool>        rready;
    sc_out<sc_bv<32>>  rdata;
    sc_out<sc_bv<2>>   rresp;
    sc_out<bool>       rlast;

    /* -- interrupt pin (level, active high)
          Convention: raised and HELD until the ISR writes the interrupt
          status register clear (see RINTSTS below). Never make it a
          one-cycle pulse -- the ISR may run several cycles later and the
          interrupt would be lost. -- */
    sc_out<bool>       irq;

    /* -- address space (change to match your DUT) -- */
    static const uint32_t REG_SIZE  = 0x10000;    /* 64KB registers */
    static const uint32_t MEM_SIZE  = (8u << 20); /* 8MB memory */

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
        /* identification registers (replace with your DUT's real values) */
        m_reg[0x06Cu / 4] = 0x53470000u;       /* VERID */
        m_reg[0x070u / 4] = (512u << 8) | 1u;  /* HCON */
    }

    /* direct access for the testbench (to observe DMA results) */
    uint8_t *mem_ptr() { return m_mem; }

private:
    enum st_e { ST_IDLE, ST_BV, ST_RV };
    st_e     m_st;
    uint32_t m_rdata = 0, m_bresp = 0, m_rresp = 0;

    uint32_t m_reg[REG_SIZE / 4];
    uint8_t  m_mem[MEM_SIZE];

    /* Address dispatch: register window vs memory region.
       Observed lesson: if both regions share one read function, the
       register indexing (& (REG_SIZE-1)) truncates memory addresses and
       the data comes back wrong. */
    uint32_t bus_read(uint32_t a)
    {
        if (a < REG_SIZE)
            return m_reg[(a & (REG_SIZE - 1)) / 4];
        if (a + 4u <= MEM_SIZE) {
            uint32_t v = 0;
            memcpy(&v, &m_mem[a], 4);
            return v;
        }
        return 0;
    }

    void bus_write(uint32_t a, uint32_t d, uint32_t strb)
    {
        if (a < REG_SIZE) {
            uint32_t off = a & (REG_SIZE - 1);

            /* CMD register: raise interrupt status (real-controller
               behaviour).
                 - command without a data phase -> CMD_DONE only
                 - command with a data phase (direction bit set) ->
                   CMD_DONE and DATA_OVER
               Real hardware raises these two beats apart; this stub asserts
               them together, and the dispatcher still calls the two
               handlers separately -- that is the point of per-bit dispatch. */
            if (off == 0x02Cu && (d & 0x80000000u)) {
                m_reg[0x044u / 4] |= 0x00000004u;      /* CMD_DONE */
                if (d & ((1u << 9) | (1u << 10)))      /* XFER_READ|XFER_WRITE */
                    m_reg[0x044u / 4] |= 0x00000008u;  /* DATA_OVER */
            }

            /* interrupt status register: write-1-clear (the level drops) */
            if (off == 0x044u)
                m_reg[0x044u / 4] &= ~d;

            uint32_t cur = m_reg[off / 4], nv = 0;
            for (int b = 0; b < 4; b++) {
                uint32_t mask = 0xFFu << (b * 8);
                nv |= (strb & (1u << b)) ? (d & mask) : (cur & mask);
            }
            if (off != 0x044u)                         /* RINTSTS handled above */
                m_reg[off / 4] = nv;
            return;
        }
        if (a + 4u <= MEM_SIZE) {
            for (int b = 0; b < 4; b++)
                if (strb & (1u << b))
                    m_mem[a + b] = (uint8_t)((d >> (b * 8)) & 0xFFu);
        }
    }

    /* clock edge: sample the AXI inputs, update state
       (valid is held until accepted) */
    void step()
    {
        if (!resetn.read()) { m_st = ST_IDLE; return; }

        switch (m_st) {
        case ST_IDLE:
            if (awvalid.read() && wvalid.read()) {
                bus_write(awaddr.read().to_uint(), wdata.read().to_uint(),
                          wstrb.read().to_uint());
                m_bresp = 0; m_st = ST_BV;
            } else if (arvalid.read()) {
                m_rdata = bus_read(araddr.read().to_uint());
                m_rresp = 0; m_st = ST_RV;
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

    /* output drive -- pin writes happen only in this process */
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

        /* interrupt pin follows the status register (level, held until
           cleared) */
        irq.write((m_reg[0x044u / 4] & 0x00000004u) != 0);
    }
};

#endif /* DUT_SLAVE_STUB_H */
