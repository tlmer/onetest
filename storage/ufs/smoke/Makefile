CC ?= gcc
CFLAGS ?= -O2 -w -std=gnu99 -I../include
LDFLAGS ?= -lpthread
tb_fsmoke: tb_fsmoke.c ../lib/libufs_model.a
	$(CC) $(CFLAGS) -o $@ tb_fsmoke.c ../lib/libufs_model.a $(LDFLAGS)
clean:
	rm -f tb_fsmoke
.PHONY: clean
