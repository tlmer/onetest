

#ifndef PCI_BUS_H
#define PCI_BUS_H
#include "pci_shim.h"

#define PCI_MAX_DEVICES_PER_BUS 32      
#define PCI_MAX_BUSES           64

struct pci_resource {
    resource_size_t start, end;
    unsigned long flags;
    struct pci_resource *parent;        
    struct pci_resource *child, *sibling;   
    const char *name;
    unsigned nr_assigned;               
    resource_size_t cursor;             
    unsigned add_size;                  
    

    

    resource_size_t dom_offset;
};

struct pci_bus;
struct pci_dev {
    struct pci_dev *next;               
    struct pci_bus *bus;
    struct pci_bus *subordinate;        

    unsigned int devfn;                 
    u16 vendor, device;
    u32 class_code;                      
    u8 revision;
    u8 hdr_type;                        
    u8 multifunction;                   
    u16 subsys_vendor, subsys_device;

    struct pci_resource resource[PCI_NUM_RESOURCES_IOV];   
    int rom_base_reg;                   
    u8 pin, irq;                        
    int cfg_size;                       
    int pcie_cap;                       
    u16 pcie_flags_reg;                 
    u32 devcap;                         
    int ari_enabled;                    
    int aer_cap;                        
    int pm_cap;                         
    int l1ss_cap;                       
    int current_state;                  
    unsigned pcie_mpss : 3;             
    int broken_intx_masking;            
    int transparent;                    
    u8 io_window, pref_window, pref_64_window; 
    int is_hotplug_bridge;              
    u32 dma_mask;                       
    int is_added;                       
    unsigned seen;                      

    
    int  is_physfn, is_virtfn;          
    struct pci_dev *physfn;             
    int  sriov_pos;                     
    u16  sriov_total, sriov_numvfs, sriov_vf_did;
    u16  sriov_offset, sriov_stride;    
    u32  sriov_pgsz;
    int  iov_nres, sriov_vfs_added;
};

struct pci_bus {
    int number;                         
    struct pci_bus *parent;             
    struct pci_dev *self;               
    struct pci_ops *ops;                
    void *sysdata;                      
    struct pci_host_bridge *bridge;     
    struct pci_bus *next_sibling;

    struct pci_dev *devices;            
    int nr_devices;

    struct pci_resource busn_res;       
    struct pci_resource *windows[3];    
    struct pci_resource mem_w[3];       

    u8 primary, secondary, subordinate; 
    u16 bridge_ctl;

    

    resource_size_t pci_base_cpu;

};

static inline int pci_ari_enabled(struct pci_bus *bus)
{ return bus && bus->self && bus->self->ari_enabled; }
void pci_configure_ari(struct pci_dev *dev);                    

int  pci_next_fn(struct pci_bus *bus, struct pci_dev *dev, int fn);

#define resource_type(r) ((r)->flags & IORESOURCE_TYPE_BITS)
static inline resource_size_t resource_size(const struct pci_resource *r)
{ return r->end - r->start + 1; }
static inline int resource_contains(const struct pci_resource *a,
                                    const struct pci_resource *b)
{ return a->start <= b->start && a->end >= b->end; }

struct pci_ops {
    const char *name;
    void *(*map_bus)(struct pci_bus *bus, unsigned int devfn, int where);
    int  (*read)(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 *val);
    int  (*write)(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 val);
};

struct pci_host_bridge {
    struct pci_bus *bus;
    struct pci_ops *ops;                
    struct pci_ops *child_ops;          
    void *sysdata;
    int busnr;
    int size_windows;                   
    struct pci_resource windows[4];     
    resource_size_t offsets[4];         
    int nr_windows;
};

struct pci_bus_region { resource_size_t start, end; };

struct pci_bus *pci_alloc_bus(int number, struct pci_bus *parent,
                              struct pci_dev *self, struct pci_ops *ops,
                              struct pci_host_bridge *bridge);
struct pci_bus *pci_alloc_child_bus(struct pci_bus *parent, struct pci_dev *bridge, int busnr);
struct pci_dev  *pci_get_slot(struct pci_bus *bus, unsigned int devfn);   
struct pci_dev  *pci_alloc_dev(struct pci_bus *bus);                      
void pci_bus_add_devices(struct pci_bus *bus);                            
struct pci_host_bridge *pci_find_host_bridge(struct pci_bus *bus);        
struct pci_bus *pci_find_bus_by_number(struct pci_bus *from, int number);
void pcibios_resource_to_bus(struct pci_bus *bus, struct pci_bus_region *region,
                             struct pci_resource *res);                   
void pcibios_bus_to_resource(struct pci_bus *bus, struct pci_resource *res,
                             struct pci_bus_region *region);              
struct pci_resource *pci_find_parent_resource(const struct pci_dev *dev,
                                              struct pci_resource *res);  
int pci_bus_alloc_resource(struct pci_bus *bus, struct pci_resource *res,
                           resource_size_t size, resource_size_t align,
                           resource_size_t min, unsigned long type_mask,
                           resource_size_t (*alignf)(void *, const struct pci_resource *,
                                                     resource_size_t, resource_size_t),
                           void *alignf_data);                            
resource_size_t pci_window_offset(struct pci_bus *bus, int idx);
resource_size_t pci_host_window_offset(struct pci_bus *bus, int idx);
resource_size_t host_offset(struct pci_bus *bus);
resource_size_t res_base_of(const struct pci_resource *r);

resource_size_t bump_next(struct pci_resource *w, resource_size_t size,
                          resource_size_t align, int *ok);
resource_size_t pci_bus_addr_to_cpu(struct pci_bus *bus, int idx, resource_size_t a);
resource_size_t pci_cpu_addr_to_bus(struct pci_bus *bus, int idx, resource_size_t a);

int  pci_resource_alloc(struct pci_resource *root, struct pci_resource *res,
                        resource_size_t size, resource_size_t align,
                        resource_size_t min, resource_size_t max,
                        resource_size_t (*alignf)(void *, const struct pci_resource *,
                                                  resource_size_t, resource_size_t),
                        void *alignf_data);

int  pci_resource_alloc_at(struct pci_resource *root, resource_size_t addr_base,
                        struct pci_resource *res,
                        resource_size_t size, resource_size_t align,
                        resource_size_t min, resource_size_t max,
                        resource_size_t (*alignf)(void *, const struct pci_resource *,
                                                  resource_size_t, resource_size_t),
                        void *alignf_data);
int  pci_request_resource(struct pci_resource *root, struct pci_resource *res);
void pci_release_resource_tree(struct pci_resource *res);

void pci_enum_reset(void);

struct pci_enum_stats {
    unsigned cfg_reads, cfg_writes, cfg_ur;      
    unsigned bar_probe_writes, bar_probe_reads;  
    unsigned devices_found, bridges_found, buses;
    unsigned windows_assigned, bars_assigned;
    unsigned enum_runs;
};
extern struct pci_enum_stats g_pci_stats;

#endif 
