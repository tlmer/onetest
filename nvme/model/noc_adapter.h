

#pragma once
#include "axi4.h"

class RtlLink {
public:
    virtual Axi4Resp push_write(const Axi4Burst& b) = 0;
    virtual Axi4Resp push_read (const Axi4Burst& b, std::vector<uint8_t>& rd) = 0;
    virtual ~RtlLink() {}
};

class NocBridge : public Axi4Slave {
public:
    RtlLink& link;
    uint64_t n_w = 0, n_r = 0, n_err = 0;   
    explicit NocBridge(RtlLink& l) : link(l) {}
    Axi4Resp write(const Axi4Burst& b) override {
        n_w++;
        Axi4Resp r = link.push_write(b);
        if (r != AXI_OKAY) n_err++;
        return r;
    }
    Axi4Resp read(const Axi4Burst& b, std::vector<uint8_t>& rd) override {
        n_r++;
        Axi4Resp r = link.push_read(b, rd);
        if (r != AXI_OKAY) n_err++;
        return r;
    }
};

class LoopbackLink : public RtlLink {
public:
    Axi4Bus& chip;                     
    explicit LoopbackLink(Axi4Bus& c) : chip(c) {}
    Axi4Resp push_write(const Axi4Burst& b) override {
        return chip.write(b.addr, b.size, b.beats, b.wdata, b.wstrb,
                          Axi4Attr{b.id, b.qos, b.prot, b.cache, b.user});
    }
    Axi4Resp push_read(const Axi4Burst& b, std::vector<uint8_t>& rd) override {
        return chip.read(b.addr, b.size, b.beats, rd,
                         Axi4Attr{b.id, b.qos, b.prot, b.cache, b.user});
    }
};
