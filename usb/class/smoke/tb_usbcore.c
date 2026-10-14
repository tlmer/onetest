

#include "../usbcore.h"
#include <stdio.h>
#include <string.h>

static int g_pass, g_fail;
#define CHECK(c, ...) do { if (c) { g_pass++; printf("  [PASS] " __VA_ARGS__); } \
                           else  { g_fail++; printf("  [FAIL] " __VA_ARGS__); } \
                           printf("\n"); } while (0)

static u32 chain_msc(u8 *b)
{
    u32 n = 0;
    b[n++]=9; b[n++]=USB_DT_CONFIG; b[n++]=32;b[n++]=0; b[n++]=1; b[n++]=1; b[n++]=0; b[n++]=0x80; b[n++]=50;
    b[n++]=9; b[n++]=USB_DT_INTERFACE; b[n++]=0; b[n++]=0; b[n++]=2; b[n++]=8; b[n++]=6; b[n++]=0x50; b[n++]=0;
    b[n++]=7; b[n++]=USB_DT_ENDPOINT; b[n++]=0x81; b[n++]=0x02; b[n++]=0x00; b[n++]=0x02; b[n++]=0;   
    b[n++]=7; b[n++]=USB_DT_ENDPOINT; b[n++]=0x02; b[n++]=0x02; b[n++]=0x00; b[n++]=0x02; b[n++]=0;   
    return n;
}

static u32 chain_hid(u8 *b, u8 interval)
{
    u32 n = 0;
    b[n++]=9; b[n++]=USB_DT_CONFIG; b[n++]=25;b[n++]=0; b[n++]=1; b[n++]=1; b[n++]=0; b[n++]=0x80; b[n++]=50;
    b[n++]=9; b[n++]=USB_DT_INTERFACE; b[n++]=0; b[n++]=0; b[n++]=1; b[n++]=3; b[n++]=1; b[n++]=2; b[n++]=0;
    b[n++]=7; b[n++]=USB_DT_ENDPOINT; b[n++]=0x81; b[n++]=0x03; b[n++]=0x08; b[n++]=0x00; b[n++]=interval;
    return n;
}

static u32 chain_cam(u8 *b, u16 isoc_mps)
{
    u32 n = 0;
    b[n++]=9; b[n++]=USB_DT_CONFIG; b[n++]=63;b[n++]=0; b[n++]=2; b[n++]=1; b[n++]=0; b[n++]=0x80; b[n++]=50;
    
    b[n++]=9; b[n++]=USB_DT_INTERFACE; b[n++]=0; b[n++]=0; b[n++]=0; b[n++]=0x0e; b[n++]=1; b[n++]=0; b[n++]=0;
    
    b[n++]=9; b[n++]=USB_DT_INTERFACE; b[n++]=0; b[n++]=1; b[n++]=2; b[n++]=0x0e; b[n++]=1; b[n++]=0; b[n++]=0;
    b[n++]=7; b[n++]=USB_DT_ENDPOINT; b[n++]=0x81; b[n++]=0x01;                    
    b[n++]=(u8)(isoc_mps & 0xff); b[n++]=(u8)(isoc_mps >> 8); b[n++]=1;
    b[n++]=7; b[n++]=USB_DT_ENDPOINT; b[n++]=0x02; b[n++]=0x01;                    
    b[n++]=(u8)(isoc_mps & 0xff); b[n++]=(u8)(isoc_mps >> 8); b[n++]=1;
    return n;
}

static int g_storage_probes, g_storage_disc, g_hid_probes;
static int probe_storage(struct usb_dev_model *d, struct usb_intf_model *i) { (void)d;(void)i; g_storage_probes++; return 0; }
static void disc_storage(struct usb_dev_model *d, struct usb_intf_model *i) { (void)d;(void)i; g_storage_disc++; }
static int probe_hid(struct usb_dev_model *d, struct usb_intf_model *i) { (void)d;(void)i; g_hid_probes++; return 0; }
static const struct usb_class_driver DRV_STORAGE = { "usb-storage", 8, 6, 0x50, probe_storage, disc_storage };
static const struct usb_class_driver DRV_HID     = { "usbhid",      3, 1, 2,    probe_hid,     0 };

static struct usb_dev_model g_dev;   

int main(int argc, char **argv)
{
    int red = (argc > 1 && strcmp(argv[1], "--red") == 0);
    u8 chain[128];

    printf("== usb class layer bench: SET_CONFIGURATION / endpoints / class-driver hooks (%s) ==\n",
           red ? "red" : "green");
    if (red) {
        

        (void)usb_dev_model_init(&g_dev, 3, 2);
        u32 n = chain_hid(chain, 200);   
        int before = usbcore_set_configuration(&g_dev, chain, n, 1);
        CHECK(before == USB_ERANGE, "RED-0 before injection: bInterval=200 rejected (criterion works)");
        usbcore_dbg_faults(1);
        int after = usbcore_set_configuration(&g_dev, chain, n, 1);
        CHECK(after == USB_OK,
              "RED-1 checks removed => bInterval=200 silently accepted (rc=%d) => U-4.3 has teeth", after);
        (void)usb_dev_model_init(&g_dev, 4, 1);
        u32 m = chain_cam(chain, 1024);  
        after = usbcore_set_configuration(&g_dev, chain, m, 1);
        CHECK(after == USB_OK,
              "RED-2 checks removed => isochronous mps=1024 at full speed accepted => U-5.4 has teeth");
        usbcore_dbg_faults(0);
        printf("== PASS=%d FAIL=%d ==\n", g_pass, g_fail);
        printf("%s\n", (g_fail == 0 && g_pass > 0) ? "SMOKE_RED_PASS" : "SMOKE_RED_FAIL");
        return (g_fail == 0 && g_pass > 0) ? 0 : 1;
    }

    
    (void)usb_dev_model_init(&g_dev, 3, 2);            
    u32 n = chain_msc(chain);
    int r = usbcore_set_configuration(&g_dev, chain, n, 1);
    struct usb_intf_model *intf = usbcore_interface(&g_dev, 0);
    CHECK(r == USB_OK && g_dev.configured && intf && intf->nep == 2,
          "U-1.1 SET_CONFIGURATION: interface 0 installed with 2 endpoints (rc=%d)", r);
    CHECK(intf && intf->klass == 8 && intf->subclass == 6 && intf->proto == 0x50,
          "U-1.2 interface class 08/06/50 read out of the chain byte for byte");

    
    {
        const struct usb_ep_model *in  = usbcore_ep_find(&g_dev, 0, 0x81, USB_EP_XFER_BULK);
        const struct usb_ep_model *out = usbcore_ep_find(&g_dev, 0, 0x02, USB_EP_XFER_BULK);
        const struct usb_ep_model *wrong = usbcore_ep_find(&g_dev, 0, 0x81, USB_EP_XFER_INT);
        CHECK(in && out && in->mps == 512 && out->mps == 512,
              "U-2.1 bulk endpoints found by (address, type): IN 0x81 / OUT 0x02, mps 512");
        CHECK(wrong == 0, "U-2.2 lookup is typed: 0x81 as interrupt => not found (no silent match)");
    }

    
    {
        g_storage_probes = g_storage_disc = g_hid_probes = 0;
        (void)usbcore_register_class_driver(&DRV_STORAGE);
        (void)usbcore_register_class_driver(&DRV_HID);
        int bound = usbcore_bind_interfaces(&g_dev);
        CHECK(bound == 1 && g_storage_probes == 1 && g_hid_probes == 0,
              "U-3.1 matched by interface id (K:driver.c:802): usb-storage probed (%d), usbhid not (%d)",
              g_storage_probes, g_hid_probes);
        CHECK(intf && intf->driver == &DRV_STORAGE,
              "U-3.2 the interface is bound to usb-storage (probe returned 0)");
        int un = usbcore_unbind_interfaces(&g_dev);
        CHECK(un == 1 && g_storage_disc == 1 && !g_dev.configured,
              "U-3.3 unbind calls disconnect (%d) and clears the configured state",
              g_storage_disc);
    }

    
    {
        (void)usb_dev_model_init(&g_dev, 4, 2);
        u32 m = chain_hid(chain, 10);
        int rr = usbcore_set_configuration(&g_dev, chain, m, 1);
        const struct usb_ep_model *ep = usbcore_ep_find(&g_dev, 0, 0x81, USB_EP_XFER_INT);
        CHECK(rr == USB_OK && ep && ep->type == USB_EP_XFER_INT,
              "U-4.1 interrupt endpoint accepted at bInterval=10 (K:config.c:335: 1..16)");
        CHECK(ep && usbcore_ep_interval_us(ep, 2) == 512u * 125u,
              "U-4.2 interval math: 2^(10-1) microframes = %u us (high speed)",
              ep ? usbcore_ep_interval_us(ep, 2) : 0);
        u32 m2 = chain_hid(chain, 200);
        CHECK(usbcore_set_configuration(&g_dev, chain, m2, 1) == USB_ERANGE,
              "U-4.3 bInterval=200 is out of range for high speed => explicit error (K:config.c:380)");
    }

    
    {
        (void)usb_dev_model_init(&g_dev, 5, 1);                  
        u32 m = chain_cam(chain, 1023);
        int rr = usbcore_set_configuration(&g_dev, chain, m, 1);
        struct usb_intf_model *a0 = usbcore_interface(&g_dev, 0);
        CHECK(rr == USB_OK && a0 && a0->alt == 0 && a0->nep == 0,
              "U-5.1 alt setting 0 is selected by default: zero bandwidth (no endpoints)");
        CHECK(usbcore_set_interface(&g_dev, 0, 1) == USB_OK,
              "U-5.2 SET_INTERFACE(alt=1) accepted");
        {
            struct usb_intf_model *a1 = usbcore_interface(&g_dev, 0);
            const struct usb_ep_model *iso = usbcore_ep_find(&g_dev, 0, 0x81, USB_EP_XFER_ISOC);
            CHECK(a1 && a1->alt == 1 && a1->nep == 2 && iso && iso->mps == 1023,
                  "U-5.3 alt 1 installs 2 isochronous endpoints (mps 1023 = full-speed ceiling)");
        }
        u32 m2 = chain_cam(chain, 1024);
        CHECK(usbcore_set_configuration(&g_dev, chain, m2, 1) == USB_ERANGE,
              "U-5.4 isochronous mps=1024 at full speed => rejected (K:config.c:187)");
    }

    
    {
        u8 bad[32];
        (void)usb_dev_model_init(&g_dev, 6, 2);
        memset(bad, 0, sizeof(bad));
        bad[0] = 0; bad[1] = USB_DT_CONFIG;                     
        CHECK(usbcore_set_configuration(&g_dev, bad, 32, 1) == USB_EINVAL,
              "U-6.1 bLength=0 => EINVAL (no infinite walk)");
        u32 m = chain_msc(chain);
        CHECK(usbcore_set_configuration(&g_dev, chain, 5, 1) == USB_EINVAL,
              "U-6.2 truncated chain (len < descriptor) => EINVAL");
        CHECK(usbcore_set_configuration(&g_dev, chain, m, 7) == USB_ENODEV,
              "U-6.3 bConfigurationValue=7 not in the chain => ENODEV (no silent fallback)");
        u32 h = chain_hid(chain, 10);
        memset(&g_dev, 0, sizeof(g_dev));
        (void)usb_dev_model_init(&g_dev, 6, 2);
        CHECK(usbcore_set_configuration(&g_dev, chain, h, 1) == USB_OK,
              "U-6.4 sanity: the same walk accepts a well-formed chain");
    }

    printf("== PASS=%d FAIL=%d ==\n", g_pass, g_fail);
    printf("%s\n", (g_fail == 0 && g_pass > 0) ? "SMOKE_GREEN_PASS" : "SMOKE_GREEN_FAIL");
    return (g_fail == 0 && g_pass > 0) ? 0 : 1;
}
