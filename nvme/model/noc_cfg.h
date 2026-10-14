

#pragma once
#include "axi4.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

struct NocAddrWin { std::string name; uint64_t base = 0, size = 0; };
struct NocMasterAttr {
    std::string name;
    int      id = 0;
    uint8_t  qos = 0, prot = 0, cache = 0;
    uint32_t user = 0;
};

struct NocCfg {
    
    std::vector<NocAddrWin>    addr{
        {"dut_regs", 0x000000ull, 0x4000ull},
        {"mem",      0x100000ull, 0x200000ull},
    };
    uint64_t task_dev = 0x0, task_asq = 0x100000, task_acq = 0x101000,
             task_iosq = 0x102000, task_iocq = 0x103000,
             task_prpl = 0x104000, task_data = 0x110000;
    std::vector<NocMasterAttr> master{{"task", 1, 0, 0, 0xF, 0}, {"dut", 2, 0, 0, 0x0, 0}};
    int outstanding = 4;      
    int idw = 4;              

    
    static NocCfg load(const char* path) {
        NocCfg c;
        std::ifstream f(path);
        if (!f) return c;
        std::string line, sec;
        while (std::getline(f, line)) {
            size_t h = line.find('#');
            if (h != std::string::npos) line = line.substr(0, h);
            trim(line);
            if (line.empty()) continue;
            if (line[0] == '[') { sec = line.substr(1, line.find(']') - 1); trim(sec); continue; }
            size_t eq = line.find('=');
            if (eq == std::string::npos) continue;
            std::string k = line.substr(0, eq), v = line.substr(eq + 1);
            trim(k); trim(v);
            if (sec == "addr") {
                std::istringstream is(v);
                uint64_t base = 0, sz = 0;
                is >> std::hex >> base >> sz;
                set_win(c.addr, k, base, sz);
            } else if (sec == "task_addr") {
                uint64_t x = strtoull(v.c_str(), nullptr, 0);
                if (k == "dev") c.task_dev = x;
                else if (k == "asq") c.task_asq = x;
                else if (k == "acq") c.task_acq = x;
                else if (k == "iosq") c.task_iosq = x;
                else if (k == "iocq") c.task_iocq = x;
                else if (k == "prpl") c.task_prpl = x;
                else if (k == "data") c.task_data = x;
            } else if (sec == "master") {
                NocMasterAttr a; a.name = k;
                std::istringstream is(v);
                std::string tok;
                while (is >> tok) {
                    size_t e2 = tok.find('=');
                    if (e2 == std::string::npos) continue;
                    std::string kk = tok.substr(0, e2), vv = tok.substr(e2 + 1);
                    unsigned long x = strtoul(vv.c_str(), nullptr, 0);
                    if (kk == "id") a.id = (int)x;
                    else if (kk == "qos") a.qos = (uint8_t)x;
                    else if (kk == "prot") a.prot = (uint8_t)x;
                    else if (kk == "cache") a.cache = (uint8_t)x;
                    else if (kk == "user") a.user = (uint32_t)x;
                }
                set_master(c.master, a);
            } else if (sec == "axi") {
                if (k == "outstanding") c.outstanding = atoi(v.c_str());
                else if (k == "idw") c.idw = atoi(v.c_str());
            }
        }
        return c;
    }

    
    template <typename Map>
    void apply_windows(Axi4Bus& bus, const Map& by_name) const {
        for (const auto& w : addr) {
            auto it = by_name.find(w.name);
            if (it != by_name.end())
                bus.attach(w.base, w.size, it->second, w.name.c_str());
        }
    }
    
    template <typename Map>
    void apply_masters(const Map& by_name) const {
        for (const auto& m : master) {
            auto it = by_name.find(m.name);
            if (it != by_name.end()) {
                it->second->m_id = (uint8_t)m.id;
                it->second->m_qos = m.qos;
                it->second->m_prot = m.prot;
                it->second->m_cache = m.cache;
                it->second->m_user = m.user;
            }
        }
    }
    void dump() const {
        printf("[noc-cfg] addr:"); for (auto& w : addr) printf(" %s=0x%llx+0x%llx", w.name.c_str(), (unsigned long long)w.base, (unsigned long long)w.size);
        printf("\n[noc-cfg] task: dev=0x%llx asq=0x%llx acq=0x%llx iosq=0x%llx iocq=0x%llx prpl=0x%llx data=0x%llx\n",
               (unsigned long long)task_dev, (unsigned long long)task_asq, (unsigned long long)task_acq,
               (unsigned long long)task_iosq, (unsigned long long)task_iocq,
               (unsigned long long)task_prpl, (unsigned long long)task_data);
        printf("[noc-cfg] master:"); for (auto& m : master) printf(" %s{id=%d qos=%d prot=%d cache=%d}", m.name.c_str(), m.id, m.qos, m.prot, m.cache);
        printf(" | axi: idw=%d outstanding=%d\n", idw, outstanding);
    }

private:
    static void trim(std::string& s) {
        size_t a = s.find_first_not_of(" \t\r\n");
        size_t b = s.find_last_not_of(" \t\r\n");
        if (a == std::string::npos) { s.clear(); return; }
        s = s.substr(a, b - a + 1);
    }
    static void set_win(std::vector<NocAddrWin>& v, const std::string& name, uint64_t base, uint64_t size) {
        for (auto& w : v) if (w.name == name) { w.base = base; w.size = size; return; }
        v.push_back({name, base, size});
    }
    static void set_master(std::vector<NocMasterAttr>& v, const NocMasterAttr& a) {
        for (auto& m : v) if (m.name == a.name) { m = a; return; }
        v.push_back(a);
    }
};
