/* SPDX-License-Identifier: GPL-2.0 */
/*
 * axi4_master_sc.h — AXI4 pin-level master
 *
 * ============================================================================
 * Why this exists (instead of a vendor bridge)
 * ============================================================================
 *   A vendor TLM-to-AXI bridge was evaluated first and did not work in
 *   practice: b_transport returned immediately with delay=0 and the slave
 *   never saw a single transaction -- the bridge's worker threads were stuck
 *   behind an ACE wrapper layer and an aligner proxy socket. That component
 *   drags in an ACE channel stack plus an aligner plus a proxy socket, far
 *   more surface than a smoke test needs.
 *
 *   => A minimal pin-level AXI4 master, written here:
 *        - only what this device needs: single beat, INCR, no IDs, no ACE
 *        - the boundary is still the real AXI4 pins
 *        - small enough to debug
 *
 *   On the TLM side it is still a target (b_transport entry):
 *     driver (synchronous) -> b_transport -> this -> AXI4 pins -> slave
 *
 *   Key deliberate difference from the vendor bridge: b_transport waits
 *   until the transaction completes, which is exactly what the synchronous
 *   driver model needs.
 *
 * ============================================================================
 * Usage
 * ============================================================================
 *   (a) Function calls (recommended, the SDK default):
 *         axm_bus_rd32() / axm_bus_wr32() -- see the global accessors below.
 *         Leave AXM_WITH_TLM undefined; there is no socket to bind.
 *   (b) TLM transactions:
 *         define AXM_WITH_TLM at compile time and bind tgt_socket to your
 *         initiator.
 *   Enabling AXM_WITH_TLM without binding the socket makes SystemC abort
 *   with E109 (complete binding failed).
 */
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

/* ============================================================================
 * Ready-line timing
 * ============================================================================
 * A ready line is asserted for at least one full clock cycle before the
 * response beat is sampled, and stays asserted for a few cycles after the
 * beat is taken.
 *
 * Why this is not cosmetic: against a cycle-accurate slave (or across the
 * SystemC/Verilog boundary in co-simulation, where the bridge is value-change
 * driven) a ready line that is raised and dropped inside one delta is not a
 * value change at all -- the signal never moves, the slave never sees the
 * assertion, and it stalls in its response state forever while the master
 * reads stale data. A pure SystemC slave does not show this, which is how it
 * survives until the pins reach real RTL.
 *
 * AXM_RDY_HOLD = cycles the ready line stays high after the beat is taken;
 * it absorbs a one-cycle transport latency in either direction.
 */
#ifndef AXM_RDY_HOLD
#define AXM_RDY_HOLD 2
#endif

/* ============================================================================
 * Global accessors -- for whoever drives the bus (driver / stub)
 * ============================================================================
 * The bus driver lives elsewhere (the shim or driver_api_stub.c) while the
 * AXI4 master is an SC_MODULE, so a pair of global functions connects them.
 *   - single instance (the case in this model); multiple instances would
 *     need an index-keyed lookup
 *   - only callable from an SC process context (they wait on clock edges)
 *   - C linkage: called from C files (driver_api_stub.c) and C++
 *     (irq_chain.cpp) alike
 */
extern "C" unsigned int axm_bus_rd32(unsigned long long addr);
extern "C" void         axm_bus_wr32(unsigned long long addr, unsigned int data);

/* Interrupt path (implemented in irq_chain.cpp; supply an empty stub if you
   do not use interrupts). */
extern "C" void emmc_drv_irq_attach_clock(sc_core::sc_in<bool> *clk);

class axi4_master_sc;                 /* forward declaration */
extern axi4_master_sc *g_axm;         /* global instance, assigned in
                                         end_of_elaboration */

/* ============================================================================
 * Bus serialization flag
 * ============================================================================
 * Why it is required:
 *   More than one SC process can drive the bus --
 *     - the driver flow (command issue, register reads/writes)
 *     - the interrupt dispatcher (reads the status register, writes it clear)
 *   They can issue transactions at the same time: an interrupt may arrive
 *   while a command transaction is still in flight. The master has exactly
 *   one FSM, so two processes mutating it corrupt each other's state and the
 *   transaction never completes -- observed as 26 timeouts with the bus
 *   going completely silent.
 *   Hardware solves this with bus arbitration; the equivalent here is a flag
 *   lock: whoever sets b_axm_busy first proceeds, the other waits for idle.
 *   Valid under cooperative single-threaded SystemC scheduling (the case
 *   here). Real concurrency would need a real lock.
 */
extern volatile int b_axm_busy;

/* ============================================================================
 * Address mapping -- configurable
 * ============================================================================
 * Why it must be configurable: different chips place the DUT's AXI4 slave at
 * different addresses. The same eMMC controller might sit at 0x0 on one chip
 * and at 0x8000_0000 on another; the memory window likewise.
 *   => The driver always uses device-relative addresses (registers
 *      0x000..0x1FF, memory from 0x1000000). The mapping adds the base
 *      before the address goes on the bus.
 *
 * Configure at run time:
 *     axm_set_map(reg_base, mem_base);
 * or change the two default macros below at compile time.
 *
 * If your DUT's windows are already at 0 / 0x1000000, nothing needs
 * configuring -- that is the default.
 * The mapping is a global singleton; multiple DUTs would need extending.
 */
extern "C" void axm_set_map(unsigned long long reg_base,
                            unsigned long long mem_base);

/* Defaults are the identity map (device-relative address == bus address), so
   an unconfigured model does not silently mis-map.
   The DMA buffers inside the device core live at device address 0x1000000
   (the host region); if your DUT's memory window is elsewhere, align it via
   mem_base. */
#define AXM_DEF_REG_BASE  0x00000000ULL
#define AXM_DEF_MEM_BASE  0x00000000ULL

class axi4_master_sc : public sc_core::sc_module {
public:
#ifdef AXM_WITH_TLM
    /* -- TLM side (optional, see the header comment) -- */
    tlm_utils::simple_target_socket<axi4_master_sc, 32> tgt_socket;
#endif

    /* -- AXI4 clock / reset -- */
    sc_in<bool>       clk;
    sc_in<bool>       resetn;

    /* -- Interrupt input: connect the DUT's irq pin (level, active high).
          Bind a constant-false signal if unused.
          One interrupt line in this model; multi-line needs
          sc_in<sc_bv<N>>. -- */
    sc_in<bool>       irq;

    /* -- AXI4 write address -- */
    sc_out<bool>       awvalid;
    sc_in<bool>        awready;
    sc_out<sc_bv<32>>  awaddr;
    sc_out<sc_bv<8>>   awlen;

    /* -- AXI4 write data -- */
    sc_out<bool>       wvalid;
    sc_in<bool>        wready;
    sc_out<sc_bv<32>>  wdata;
    sc_out<sc_bv<4>>   wstrb;
    sc_out<bool>       wlast;

    /* -- AXI4 write response -- */
    sc_in<bool>        bvalid;
    sc_out<bool>       bready;
    sc_in<sc_bv<2>>    bresp;

    /* -- AXI4 read address -- */
    sc_out<bool>       arvalid;
    sc_in<bool>        arready;
    sc_out<sc_bv<32>>  araddr;
    sc_out<sc_bv<8>>   arlen;

    /* -- AXI4 read data -- */
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

        /* Ports are initialized in end_of_elaboration, not in the
           constructor -- SystemC discipline: initial values may only be
           written once the ports are bound. */
    }

    /* Global accessor binding + interrupt clock handoff.
       Must happen in end_of_elaboration (ports bound), not in the
       constructor. */
    void end_of_elaboration()
    {
        g_axm = this;
        emmc_drv_irq_attach_clock(&clk);
    }

private:
    /* -- operands of the transaction currently in flight -- */
    bool        m_active;
    bool        m_is_write;
    sc_bv<32>   m_addr;
    sc_bv<32>   m_wdata;
    sc_bv<4>    m_wstrb;
    uint32_t   *m_rdata_out;     /* where the read result is written back */
    uint32_t    m_rdata;         /* current beat's read value */

    /* -- handshake phase flags -- */
    bool m_aw_done, m_w_done, m_ar_done;
    bool m_issue;                /* issue-pending; pin writes happen only in
                                    drive_impl() */

    /* -- ready-line sequencing (see AXM_RDY_HOLD above) -- */
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
    /* -- direct transfers, called by axm_bus_rd32/wr32 (no TLM payload) -- */
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
    /* Generic transfer kernel (up to 4KB; anything above 4 bytes is split
       into 4-byte transactions).
       NOTE: single beats, no INCR bursts -- see docs/integration.md. */
    bool move(uint64_t addr, void *buf, unsigned len, bool is_write)
    {
        uint8_t *p = reinterpret_cast<uint8_t *>(buf);
        unsigned done = 0;
        bool ok = true;

        /* Bus serialization: wait for idle (see b_axm_busy above).
           Waiting advances the clock; it does not spin. */
        while (b_axm_busy)
            wait(clk.posedge_event());
        b_axm_busy = 1;

        /* RAII guard so every return path releases the bus */
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
                /* Do not memcpy into an sc_bv object -- it is not POD and
                   the data is lost (observed: writes turned into zero).
                   Stage through a uint32_t first. */
                uint32_t wv = 0;
                memcpy(&wv, p + done, chunk);
                m_wdata     = sc_bv<32>(wv);
                m_rdata_out = nullptr;
            } else {
                m_rdata     = 0;
                m_rdata_out = &m_rdata;
            }

            m_aw_done = m_w_done = m_ar_done = false;
            /* ready-line sequencing is per transaction too: leaving these set
               from the previous beat makes the next transaction report done
               before it ever sampled a beat (observed as a one-beat lag). */
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

    /* Clock edge: advance the handshake one step. Every pin write happens
       here.
       ---------------------------------------------------------------------
       Pin writes must come only from this path. sc_signal requires a single
       driving process; if b_transport wrote pins directly, the caller's
       process would be recorded as a second driver and SystemC aborts with
       E115.
       ---------------------------------------------------------------------
       Handshake sequence (one step per clock):
         write: AW and W issued -> each waits to be accepted -> bready raised
                -> wait for bvalid -> deassert
         read:  AR issued -> waits to be accepted -> rready raised -> wait for
                rvalid -> deassert

       Deadlock note (observed): if bready only rises after bvalid and the
       slave only raises bvalid after bready, both sides wait forever. The
       ready side must assert first, as a real AXI master does. */
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

        /* Issue: push the pending transaction onto the pins. */
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
            /* 1) AW and W issued together, each waits to be accepted */
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

            /* 2) Ready side asserts first, then the response is sampled
                  (reversing the order deadlocks). Three steps -- assert /
                  sample / hold -- so the ready line is a genuine value
                  change on the pins; see AXM_RDY_HOLD. */
            if (!m_b_asserted) {
                if (write_pins) bready.write(true);
                m_b_asserted = true;      /* sample from the next cycle on */
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
            /* 1) AR issued, waits to be accepted */
            if (!m_ar_done) {
                if (write_pins) {
                    araddr.write(m_addr); arlen.write(0); arvalid.write(true);
                }
                if (arready.read())
                    m_ar_done = true;
                return;
            }
            if (write_pins) arvalid.write(false);

            /* 2) Ready side asserts first, then the beat is sampled (same
                  reason as the write path): the beat may already be waiting,
                  so asserting and dropping ready inside one cycle would make
                  the assertion invisible to the slave. */
            if (!m_r_asserted) {
                if (write_pins) rready.write(true);
                m_r_asserted = true;      /* sample from the next cycle on */
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
    /* b_transport -- synchronous, optional path. Shares the move() kernel
       with bus_read/bus_write; no duplicate implementation. */
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
#endif /* AXM_WITH_TLM */
};

/* The global accessor implementations live in axi4_master_sc.cpp -- not in
   this header, which would duplicate symbols across translation units and
   needs matching C/C++ linkage. */

#endif /* AXI4_MASTER_SC_H */
