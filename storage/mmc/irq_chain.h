

#ifndef IRQ_CHAIN_H
#define IRQ_CHAIN_H

#include <systemc.h>

typedef void (*emmc_isr_fn)(unsigned int sts, void *ctx);

extern "C" void emmc_drv_isr_bind(unsigned int mask_bits,
                                  emmc_isr_fn fn, void *ctx);

extern "C" void k_drv_irq_deliver(void);

extern "C" int  emmc_drv_irq_pending(void);

extern "C" unsigned int emmc_drv_irq_served(void);   
extern "C" unsigned int emmc_drv_irq_last_sts(void); 

SC_MODULE(irq_observer) {
    sc_in<bool>  clk;
    sc_in<bool>  irq;        
    sc_out<bool> irq_out;    

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
            k_drv_irq_deliver();   
    }
};

#endif 
