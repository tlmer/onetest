# class/ -- usb class layer (the host-side USB core work, written once for the class)

The part of the Linux USB host stack that a controller driver does **not** own:
configuration selection, the endpoint model and class-driver binding.  It covers
the three gaps the Linux-proxy audit found for this line.

| Area | What it does | Reference |
|---|---|---|
| SET_CONFIGURATION | walks the configuration chain (config/interface/endpoint), picks `bConfigurationValue`, installs interfaces, altsettings and endpoints | `drivers/usb/core/message.c:1957` |
| endpoints | typed endpoint table; `bInterval` validated and rejected when out of range; packet-size ceilings per transfer type | `drivers/usb/core/config.c:254/318-390/181-201` |
| alt settings | one interface with several altsettings (zero-bandwidth <-> streaming) | `drivers/usb/core/config.c:498` |
| class drivers | interface matched against a driver id table, then probe / disconnect | `drivers/usb/core/driver.c:802/890` |

    cd usb/class/smoke && make && ./tb_usbcore   # SMOKE_GREEN_PASS
    ./tb_usbcore --red                           # SMOKE_RED_PASS

`lib/libusbcore.a` is the delivered library; `usbcore.h` is the interface a
customer references.
