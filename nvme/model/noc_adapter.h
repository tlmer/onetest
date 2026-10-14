// ============================================================================
// WHAT THIS IS / WHAT THIS IS NOT  (read this first)
//   This file is the ADAPTER LAYER ON OUR SIDE. It is NOT a NoC model.
//
//   Roles:
//     - RtlLink          : the interface the CUSTOMER (or the DPI/RTL side) implements.
//                          The customer's NoC stays the customer's; we only define the
//                          contract our model calls into.
//     - NocBridge        : an Axi4Slave adapter that turns our task model's transactions
//                          into RtlLink calls (so the task model and all criteria stay
//                          unchanged when the transport changes).
//     - LoopbackLink     : (formerly StubLink) a LOCAL test double used only for
//                          self-test on this machine. It is NOT a NoC and is never
//                          shipped as one.
//
//   Boundary contract (who provides what):
//     our side   : task model + memory model + this adapter (+ DPI link on the C++ side)
//     their side : the NoC itself + its AXI4 slave ports (customer provides)
// ============================================================================

// ============================================================================
// noc_adapter.h -- AXI4 <-> RTL NoC bridge (skeleton; channel form A: the NoC is RTL) [*] settled 2026-10-11
//   Topology:
//     task model (Axi4Master) -> TLM-side bus -> [NocBridge : Axi4Slave] (this file) -> RtlLink (real bridge)
//                                                                                     -> RTL NoC (silicon)
//   Contract: this file has the **Axi4Slave shape** => the task model and the criteria change
//   **not a single line** (attaching a bridge = rewiring, not swapping parts).
//   [X] three things we do NOT do (open account): bandwidth/QoS not modeled; out-of-order completion not
//      modeled (the in-order-within-one-ID assumption is pinned in §5);
//      pin-level timing is the responsibility of the RtlLink implementer.
// ============================================================================
#pragma once
#include "axi4.h"

// -- (1) RTL link abstraction: pick one of three concrete bridges (implement this interface) -------------
//   TODO (pick one):
//     (a) DPI / hwfifo -- the simulator in use in this family (xsim / VCS); the RTL side hooks an AXI
//         transaction port / UVM agent
//     (b) Verilator rtl-bridges -- libsystemctlm-soc; zero commercial tools (Verilator is local, [OK])
//     (c) libremote-port -- the RTL side connects a remote-port component; the Xilinx/DEC ecosystem standard
//   Requirement: push_* MUST **block until B/R returns** (the bridge implementation advances simulation
//   time); returns AXI_OKAY / SLVERR / DECERR
class RtlLink {
public:
    virtual Axi4Resp push_write(const Axi4Burst& b) = 0;
    virtual Axi4Resp push_read (const Axi4Burst& b, std::vector<uint8_t>& rd) = 0;
    virtual ~RtlLink() {}
};

// -- (2) the bridge: from the TLM side this looks like a slave (absorbs all task-model transactions) -----
class NocBridge : public Axi4Slave {
public:
    RtlLink& link;
    uint64_t n_w = 0, n_r = 0, n_err = 0;   // ledger (used by the criteria once the bridge is attached)
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

// -- (3) LoopbackLink: local loopback (replaces the "chip side" with a local model) ---------------------
//   Use: bring-up / bridge-wiring self-test (hook the chip-side model onto the chip bus and run the
//   same criteria set)
//   [X] NOT an RTL link -- the real link = one of the three TODOs above
class LoopbackLink : public RtlLink {
public:
    Axi4Bus& chip;                     // chip-side bus (reference slave / memory attach to it)
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
