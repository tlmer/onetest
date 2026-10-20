

#include <stdio.h>
#include <string.h>

#include "npx_hal.h"

static int g_pass, g_fail;

#define CHECK(c, ...) do { int _ok = (c); if (_ok) g_pass++; else g_fail++; \
    printf("  [%s] ", _ok ? "PASS" : "FAIL"); printf(__VA_ARGS__); \
    printf("\n"); } while (0)

#ifdef NPX_HAL_RED
#define EXP_ARCSYNC_ROLE "rproc"     
#define EXP_UNKNOWN_RC   NPX_HAL_OK  
#else
#define EXP_ARCSYNC_ROLE "ctrl"
#define EXP_UNKNOWN_RC   NPX_HAL_ENODEV
#endif

static void check_key(const char *compat, const char *role, const char *driver)
{
    const struct npx_hal_entry *e = npx_hal_lookup(compat);
    const char *r = NULL, *d = NULL;
    int rc;

    CHECK(e != NULL, "lookup %s", compat);
    if (!e)
        return;
    CHECK(strncmp(e->kern_src, "K5.15dl:", 8) == 0,
          "%s kern_src = %s", compat, e->kern_src);
    rc = npx_hal_bind(compat, &r, &d);
    CHECK(rc == NPX_HAL_OK && r && strcmp(r, role) == 0 &&
          d && strcmp(d, driver) == 0,
          "%s bind role=%s driver=%s (rc=%d)", compat, r ? r : "-", d ? d : "-", rc);
}

int main(void)
{
    const char *r = NULL, *d = NULL;
    int i, rc;

    setvbuf(stdout, NULL, _IONBF, 0);
    printf("=== NPX HAL ===\n");

    CHECK(npx_hal_count() == 4, "H-1 table rows = 4 (got %d)", npx_hal_count());
    for (i = 0; i < npx_hal_count(); i++) {
        const struct npx_hal_entry *e = npx_hal_row(i);
        CHECK(e && strcmp(e->vendor, "Synopsys") == 0,
              "H-2 row %d is an IP vendor (vendor=%s)", i, e ? e->vendor : "-");
    }

    check_key("snps,npx-rproc", "rproc", "snps_accel_rproc");
    check_key("snps,vpx-rproc", "rproc", "snps_accel_rproc");
    check_key("snps,accel-app", "app", "snps_accel");
    check_key("snps,arcsync", EXP_ARCSYNC_ROLE, "snps_arcsync");

    CHECK(npx_hal_lookup("snps,accel") == NULL,
          "H-3 bus container 'snps,accel' is not an IP model");
    rc = npx_hal_bind("snps,does-not-exist", &r, &d);
    CHECK(rc == EXP_UNKNOWN_RC, "H-4 unknown compatible => rc=%d (want %d)",
          rc, EXP_UNKNOWN_RC);

    printf("\nNPX_HAL PASS=%d FAIL=%d\n", g_pass, g_fail);
    printf("%s\n", g_fail ? "NPX_HAL FAIL" : "NPX_HAL PASS");
    return g_fail ? 1 : 0;
}
