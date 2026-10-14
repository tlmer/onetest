

#include "../include/pci_ioctl_ref.h"
#include "../include/axi4_tlm.h"
#include "../include/pcie_regs.h"
#include "../include/host_ops.h"
#include "../include/pcie_core.h"

#include <stdio.h>
#include <string.h>

extern int  pcie_model_init(void);
extern void pcie_model_exit(void);
extern void pcie_model_set_backend(unsigned b);
extern long pcie_model_ioctl(unsigned int cmd, void *arg, unsigned long len);
extern void pcie_model_set_msi_vectors(unsigned n);   

static void hexdump(const char *tag, const unsigned char *p, unsigned n)
{
    unsigned i;
    printf("      %s:", tag);
    for (i = 0; i < n; i++) printf(" %02x", p[i]);
    printf("\n");
}

int main(void)
{
    struct pcie_info info;
    struct pcie_enum_req en;
    struct pcie_cfg_io c;
    struct pcie_run_state st;
    long rc;

    printf("== PCIe Root Port proxy model — example ==\n\n");

    
    pcie_model_set_backend(1);
    if (pcie_model_init() != 0) { printf("init failed\n"); return 1; }

    rc = pcie_model_ioctl(PCIE_IOC_GET_INFO, &info, sizeof(info));
    if (rc != 0) { printf("GET_INFO rc=%ld\n", rc); return 1; }
    printf("[1] controller: version=0x%08x link_up=%u\n", info.version, info.link_up);
    printf("    AXI4 interface: ID %u bits, data %u bits, %u outstanding\n",
           info.id_bits, info.data_bits, info.outstanding);
    printf("    windows: DBI 0x%llx  CFG 0x%llx  MEM 0x%llx\n",
           (unsigned long long)info.dbi_base, (unsigned long long)info.cfg_base,
           (unsigned long long)info.mem_base);

    
    memset(&en, 0, sizeof(en));
    en.max = 16u;
    rc = pcie_model_ioctl(PCIE_IOC_ENUM, &en, sizeof(en));
    if (rc != 0) { printf("ENUM rc=%ld (count=%u)\n", rc, en.count); return 1; }
    printf("\n[2] enumeration found %u device(s):\n", en.count);
    {   unsigned i;
        for (i = 0; i < en.count && i < en.max; i++)
            printf("    %02x:%02x.%u  %04x:%04x  class 0x%06x  bar0 0x%llx\n",
                   en.dev[i].bus, (en.dev[i].devfn >> 3) & 0x1f, en.dev[i].devfn & 7,
                   en.dev[i].vendor, en.dev[i].device, en.dev[i].class_code,
                   (unsigned long long)en.dev[i].bar[0]); }

    
    if (en.count > 1u) {
        memset(&c, 0, sizeof(c));
        c.bus = en.dev[1].bus; c.devfn = en.dev[1].devfn; c.where = 0x00u; c.size = 4u;
        rc = pcie_model_ioctl(PCIE_IOC_CFG_RD, &c, sizeof(c));
        printf("\n[3] CFG_RD %02x:%02x.%u w0 = 0x%08x (vendor:device)\n",
               c.bus, (c.devfn >> 3) & 0x1f, c.devfn & 7, c.data);
    }

    

    {   u64 da = 0;
        unsigned char *p = (unsigned char *)pcie_model_dma_alloc("example-buf", 256u, &da);
        struct axi4_tlm_beat b;
        unsigned char payload[16];
        unsigned i;
        if (!p) { printf("dma_alloc failed\n"); return 1; }
        for (i = 0; i < 16u; i++) payload[i] = (unsigned char)(0x10u + i);
        memset(&b, 0, sizeof(b));
        b.write = 1; b.burst = 1; b.size = 4u; b.len = 0; b.addr = da;
        memcpy(b.data, payload, 16u);
        memset(b.strb, 1, 16u);
        rc = axs_recv(pcie_model_axs(), &b);
        printf("\n[4a] DUT DMA write to 0x%llx -> owner hits=%u resp=%u\n",
               (unsigned long long)da, pcie_model_dma_owner_hits("example-buf"), b.resp);
        hexdump("buffer", p, 16u);

        
        pcie_model_set_msi_vectors(8u);
        pcie_dbi_wr32(PCIE_MSI_ADDR_LO, 0xFEE00000u);
        pcie_dbi_wr32(PCIE_MSI_ADDR_HI, 0u);
        pcie_dbi_wr32(PCIE_MSI_INTR0_ENABLE, 0xffffffffu);
        pcie_dbi_wr32(PCIE_MSI_INTR0_MASK, 0u);
        pcie_dbi_wr32(PCIE_MSI_INTR0_STATUS, 0xffffffffu);
        axi4_tlm_beat_from32(&b, 0xFEE00000ull + 3u, 1, 0x00000003u, 0xfu);   
        (void)axs_recv(pcie_model_axs(), &b);
        pcie_irq_line_poll();
        printf("[4b] MSI vector 3 posted -> irq line = %d (active high)\n", pcie_irq_line_level());
        
        pcie_dbi_wr32(PCIE_MSI_INTR0_STATUS, 1u << 3);
        printf("     after ack (write-1-clear) -> irq line = %d\n", pcie_irq_line_level());
    }

    rc = pcie_model_ioctl(PCIE_IOC_GET_STATE, &st, sizeof(st));
    printf("\n[5] state: link=%u msi_recv=%u cfg_cycles=%u route_miss=%u delivered=%u\n",
           st.link_up, st.msi_recv, st.cfg_cycles, st.route_miss, st.delivered);

    pcie_model_exit();
    printf("\n== example done ==\n");
    return 0;
}
