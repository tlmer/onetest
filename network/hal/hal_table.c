/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hal_table.c -- the network HAL key table (four levels x key)
 *
 * Each row's `kern_src` is where in the KERNEL binding table the key was read
 * from (file:line) -- the keys are read out of the kernel, not invented, and
 * chip makers are deliberately absent (their controllers are never licensed
 * out, so they can never be a DUT).
 *
 * Many-to-one is normal: every `snps,dwmac*` compatible selects the one
 * DW GMAC model (that is the unit a customer licenses -- the IP core, not
 * the SoC integration).
 *
 * status: "built" = model exists under network/model/;
 *         "todo"  = IP vendor identified, model can be added.
 */
#include "hal.h"

/* model-side ops adapters (hal.c); switched by HAL_WITH_ETH */
extern const struct network_ip_ops network_ops_dwmac;    /* Synopsys DW GMAC     */
extern const struct network_ip_ops network_ops_dwxgmac;  /* Synopsys DW XLGMAC   */

#ifdef HAL_WITH_ETH
#define OPS_GMAC   (&network_ops_dwmac)
#define OPS_XLGMAC (&network_ops_dwxgmac)
#else
#define OPS_GMAC   ((const struct network_ip_ops *)0)
#define OPS_XLGMAC ((const struct network_ip_ops *)0)
#endif

#define K_NETWORK "network"
#define K_ETH     "ethernet"

static const struct network_hal_entry g_table[] = {
/* -- subclass: ethernet / vendor: Synopsys / model: DW GMAC (DWC Ether QoS) -------- */
{ { K_NETWORK, K_ETH, "Synopsys", "DWC Ether QoS (GMAC)" },
  HAL_KEY_DT, "snps,dwmac-3.40a", "K:drivers/net/ethernet/stmicro/stmmac/dwmac-generic.c:74",
  "built", OPS_GMAC },
{ { K_NETWORK, K_ETH, "Synopsys", "DWC Ether QoS (GMAC)" },   /* same IP, newer names */
  HAL_KEY_DT, "snps,dwmac-3.50a", "K:drivers/net/ethernet/stmicro/stmmac/dwmac-generic.c:75",
  "built", OPS_GMAC },
{ { K_NETWORK, K_ETH, "Synopsys", "DWC Ether QoS (GMAC)" },
  HAL_KEY_DT, "snps,dwmac-3.610", "K:drivers/net/ethernet/stmicro/stmmac/dwmac-generic.c:76",
  "built", OPS_GMAC },
{ { K_NETWORK, K_ETH, "Synopsys", "DWC Ether QoS (GMAC)" },
  HAL_KEY_DT, "snps,dwmac-3.70a", "K:drivers/net/ethernet/stmicro/stmmac/dwmac-generic.c:77",
  "built", OPS_GMAC },
{ { K_NETWORK, K_ETH, "Synopsys", "DWC Ether QoS (GMAC)" },
  HAL_KEY_DT, "snps,dwmac-3.710", "K:drivers/net/ethernet/stmicro/stmmac/dwmac-generic.c:78",
  "built", OPS_GMAC },
{ { K_NETWORK, K_ETH, "Synopsys", "DWC Ether QoS (GMAC)" },
  HAL_KEY_DT, "snps,dwmac-4.00", "K:drivers/net/ethernet/stmicro/stmmac/dwmac-generic.c:79",
  "built", OPS_GMAC },
{ { K_NETWORK, K_ETH, "Synopsys", "DWC Ether QoS (GMAC)" },
  HAL_KEY_DT, "snps,dwmac-4.10a", "K:drivers/net/ethernet/stmicro/stmmac/dwmac-generic.c:80",
  "built", OPS_GMAC },
{ { K_NETWORK, K_ETH, "Synopsys", "DWC Ether QoS (GMAC)" },
  HAL_KEY_DT, "snps,dwmac", "K:drivers/net/ethernet/stmicro/stmmac/dwmac-generic.c:81",
  "built", OPS_GMAC },

/* -- subclass: ethernet / vendor: Synopsys / model: DW XLGMAC ---------------------- */
{ { K_NETWORK, K_ETH, "Synopsys", "DWC XLGMAC" },
  HAL_KEY_DT, "snps,dwxgmac-2.10", "K:drivers/net/ethernet/stmicro/stmmac/dwmac-generic.c:82",
  "built", OPS_XLGMAC },
{ { K_NETWORK, K_ETH, "Synopsys", "DWC XLGMAC" },
  HAL_KEY_DT, "snps,dwxgmac", "K:drivers/net/ethernet/stmicro/stmmac/dwmac-generic.c:83",
  "built", OPS_XLGMAC },

/* -- subclass: ethernet / vendor: Cadence (IP supplier; model not written yet) ----- */
{ { K_NETWORK, K_ETH, "Cadence", "GEM / MACB" },
  HAL_KEY_DT, "cdns,macb", "K:drivers/net/ethernet/cadence/macb_main.c:4809",
  "todo", (const struct network_ip_ops *)0 },
{ { K_NETWORK, K_ETH, "Cadence", "GEM / MACB" },
  HAL_KEY_DT, "cdns,gem", "K:drivers/net/ethernet/cadence/macb_main.c:4812",
  "todo", (const struct network_ip_ops *)0 }
};

const struct network_hal_entry *network_hal_table(unsigned *count)
{
    if (count) *count = (unsigned)(sizeof(g_table) / sizeof(g_table[0]));
    return g_table;
}
