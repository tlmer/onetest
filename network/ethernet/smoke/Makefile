CC ?= gcc
CFLAGS ?= -O2 -w -std=gnu99 -I../include
LDFLAGS ?= -lpthread
tb_esmoke: tb_esmoke.c ../lib/libeth_model.a
	$(CC) $(CFLAGS) -o $@ tb_esmoke.c ../lib/libeth_model.a $(LDFLAGS)
clean:
	rm -f tb_esmoke
.PHONY: clean
