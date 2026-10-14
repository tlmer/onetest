CC ?= gcc
CFLAGS ?= -O2 -w -std=gnu99 -I..
LDFLAGS ?= -lpthread
# class library (netdev + the IP-model seam) + the ethernet IP model library
tb_csmoke: tb_csmoke.c ../lib/libnet_class.a ../../ethernet/lib/libeth_model.a
	$(CC) $(CFLAGS) -o $@ tb_csmoke.c ../lib/libnet_class.a ../../ethernet/lib/libeth_model.a $(LDFLAGS)
clean:
	rm -f tb_csmoke
.PHONY: clean
