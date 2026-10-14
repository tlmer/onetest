CC ?= gcc
CFLAGS ?= -O2 -w -std=gnu99 -I..
# the class layer ships as a library; the bench links it
tb_usbcore: tb_usbcore.c ../lib/libusbcore.a
	$(CC) $(CFLAGS) -o $@ tb_usbcore.c ../lib/libusbcore.a
clean:
	rm -f tb_usbcore
.PHONY: clean
