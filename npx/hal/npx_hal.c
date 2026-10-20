

#include <string.h>

#include "npx_hal.h"

static const struct npx_hal_entry g_table[] = {
{ "npu", "Synopsys", "ARC NPX6", "snps,npx-rproc", "snps_accel_rproc", "rproc",
  "K5.15dl:drivers/remoteproc/snps_accel/accel_rproc.c:599", 1 },
{ "npu", "Synopsys", "ARC VPX", "snps,vpx-rproc", "snps_accel_rproc", "rproc",
  "K5.15dl:drivers/remoteproc/snps_accel/accel_rproc.c:598", 1 },
{ "npu", "Synopsys", "ARC NPX app", "snps,accel-app", "snps_accel", "app",
  "K5.15dl:drivers/misc/snps_accel/snps_accel_drv.c:486", 1 },
{ "npu", "Synopsys", "ARCSync", "snps,arcsync", "snps_arcsync", "ctrl",
  "K5.15dl:drivers/misc/snps_accel/snps_arcsync.c:1015", 1 },
};

int npx_hal_count(void) { return (int)(sizeof(g_table) / sizeof(g_table[0])); }

const struct npx_hal_entry *npx_hal_row(int i)
{
    if (i < 0 || i >= npx_hal_count())
        return NULL;
    return &g_table[i];
}

const struct npx_hal_entry *npx_hal_lookup(const char *compat)
{
    int i;
    if (!compat)
        return NULL;
    for (i = 0; i < npx_hal_count(); i++)
        if (strcmp(g_table[i].compat, compat) == 0)
            return &g_table[i];
    return NULL;
}

int npx_hal_bind(const char *compat, const char **role, const char **driver)
{
    const struct npx_hal_entry *e = npx_hal_lookup(compat);
    if (!e)
        return NPX_HAL_ENODEV;
    if (!e->built)
        return NPX_HAL_ENOSYS;
    if (role)
        *role = e->role;
    if (driver)
        *driver = e->driver;
    return NPX_HAL_OK;
}
