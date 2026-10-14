/* SPDX-License-Identifier: GPL-2.0 */
/*
 * irq_chain.h -- interrupt path (pin-level; wire your DUT in like this)
 *
 * ============================================================================
 * How an interrupt travels (and how it maps to real hardware)
 * ============================================================================
 *   DUT (eMMC host controller)
 *      |  irq pin (level, active high; held until the ISR clears the
 *      |           interrupt status register)
 *      v
 *   axi4_master_sc.irq     <- connect your DUT's irq here
 *      |  k_drv_irq_deliver()  (dispatch is deferred by one cycle)
 *      v
 *   your registered ISR (emmc_drv_isr_bind)
 *      |  the ISR reads the status register over AXI4, then writes it clear
 *      v
 *   DUT sees the status cleared  ->  deasserts irq  (level semantics)
 *
 * -- Three disciplines (real hardware must follow these too) ---------------
 *   1. LEVEL semantics: once raised, irq must stay high until the ISR
 *      clears the DUT's interrupt status. Do not pulse it -- the ISR may
 *      run several cycles later and the interrupt would be lost.
 *   2. DEFERRED dispatch: the ISR must not run inside an outer bus
 *      transaction. The ISR reads registers itself, which would re-enter
 *      the transaction and make SystemC report E115 (two drivers on one
 *      signal). This model defers dispatch by one delta -- on real hardware
 *      the interrupt also arrives after the transaction.
 *   3. LATCH on arrival: set a pending flag, let the ISR take it. Never
 *      test the raw level directly -- a level wired straight through leaves
 *      only a one-cycle acceptance window.
 * ============================================================================
 */
#ifndef IRQ_CHAIN_H
#define IRQ_CHAIN_H

#include <systemc.h>

/* ============================================================================
 * Per-bit dispatch
 * ============================================================================
 *   Hardware has ONE irq line but several interrupt sources (command
 *   complete, data complete, RX FIFO, TX FIFO, IDMAC, error classes).
 *   The convention (same as the real driver):
 *       1. the ISR reads the interrupt status register (RINTSTS)
 *       2. dispatch by BIT to the matching handler
 *       3. each handler does its work; the status is written clear at the end
 *
 *   Register only the bits you care about -- unbound bits are not handled.
 *
 *   Common RINTSTS bits (DW_mmc convention):
 *      bit2  CMD_DONE    command complete     <- wake the command thread
 *      bit3  DATA_OVER   transfer complete    <- wake the data thread
 *      bit4  TXDR        TX FIFO request
 *      bit5  RXDR        RX FIFO request
 *      bit6  RCRC        response CRC error  \
 *      bit7  DCRC        data CRC error       |  error class: record and
 *      bit8  RTO         response timeout     |  run the recovery flow
 *      bit9  DRTO        data read timeout   /
 *      (IDMAC status lives in a separate register, IDSTS)                  */
typedef void (*emmc_isr_fn)(unsigned int sts, void *ctx);

/* Bind a handler to a bit group. mask_bits may cover several bits; binding
   the same mask again replaces the previous handler. */
extern "C" void emmc_drv_isr_bind(unsigned int mask_bits,
                                  emmc_isr_fn fn, void *ctx);

/* Interrupt arrived (called by irq_observer; dispatch is deferred one
   cycle). */
extern "C" void k_drv_irq_deliver(void);

/* Is there an unhandled interrupt? (optional observation point) */
extern "C" int  emmc_drv_irq_pending(void);

/* -- dispatch statistics (observable by tests) -- */
extern "C" unsigned int emmc_drv_irq_served(void);   /* dispatches served */
extern "C" unsigned int emmc_drv_irq_last_sts(void); /* last RINTSTS seen */

/* ============================================================================
 * Interrupt observation point -- connect the DUT's irq pin here
 * ============================================================================
 *   dut_irq  -->  irq_obs.irq
 *   irq_obs.irq_out --> axi4_master_sc.irq
 *   (Connecting the DUT's irq straight to axi4_master_sc.irq also works;
 *    this module is an optional observation/logging stage that makes the
 *    interrupt timing visible on a waveform.) */
SC_MODULE(irq_observer) {
    sc_in<bool>  clk;
    sc_in<bool>  irq;        /* <- DUT interrupt pin (level, active high) */
    sc_out<bool> irq_out;    /* -> axi4_master_sc.irq */

    SC_CTOR(irq_observer)
    {
        SC_METHOD(sample);
        sensitive << clk.pos();
        dont_initialize();
    }

    void sample()
    {
        bool lvl = irq.read();
        irq_out.write(lvl);
        if (lvl)
            k_drv_irq_deliver();   /* latch on arrival; dispatch is deferred */
    }
};

#endif /* IRQ_CHAIN_H */
