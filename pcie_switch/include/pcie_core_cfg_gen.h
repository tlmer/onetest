

#pragma once

namespace pcie_tlm {

struct CoreCfgParam { const char* name; long long value; int line; };
struct CoreCfgEntry { const char* cfg; const char* file; int n; const CoreCfgParam* p; };

static const CoreCfgParam kCfg_config_cxl_old[] = {
    {"TRGT_DATA_WD", 256LL, 23050},
    {"CX_TAG_SIZE", 8LL, 14548},
    {"FLT_Q_ADDR_WIDTH", 64LL, 5650},
    {"FLT_Q_ATTR_WIDTH", 2LL, 10574},
    {"CX_NVC", 1LL, 4726},
    {"CX_NVC_XALI_EXPANSION", 1LL, 5823},
    {"CX_NFUNC", 1LL, 4316},
    {"CX_NFUNC_WD", 3LL, 4395},
    {"CX_NHQ", 2LL, 4197},
    {"CX_NW", 8LL, 4005},
    {"CX_BUSNUM_WD", 8LL, 4700},
    {"CX_DEVNUM_WD", 5LL, 4716},
    {"CX_LBC_NW", 2LL, 8606},
    {"CX_MSI_NFUNC", 1LL, 4366},
    {"CX_RADM_NFC_TRGT_INTF", 2LL, 4265},
    {"CX_NL", 8LL, 3144},
};

static const CoreCfgParam kCfg_config_ep[] = {
    {"TRGT_DATA_WD", 256LL, 23042},
    {"CX_TAG_SIZE", 8LL, 14541},
    {"FLT_Q_ADDR_WIDTH", 32LL, 5647},
    {"FLT_Q_ATTR_WIDTH", 2LL, 10567},
    {"CX_NVC", 1LL, 4723},
    {"CX_NVC_XALI_EXPANSION", 1LL, 5820},
    {"CX_NFUNC", 1LL, 4313},
    {"CX_NFUNC_WD", 3LL, 4392},
    {"CX_NHQ", 2LL, 4194},
    {"CX_NW", 8LL, 4002},
    {"CX_BUSNUM_WD", 8LL, 4697},
    {"CX_DEVNUM_WD", 5LL, 4713},
    {"CX_LBC_NW", 1LL, 8599},
    {"CX_MSI_NFUNC", 1LL, 4363},
    {"CX_RADM_NFC_TRGT_INTF", 2LL, 4262},
    {"CX_NL", 4LL, 3141},
};

static const CoreCfgParam kCfg_config_ep_cxl[] = {
    {"TRGT_DATA_WD", 512LL, 23054},
    {"CX_TAG_SIZE", 10LL, 14552},
    {"FLT_Q_ADDR_WIDTH", 64LL, 5651},
    {"FLT_Q_ATTR_WIDTH", 2LL, 10578},
    {"CX_NVC", 1LL, 4727},
    {"CX_NVC_XALI_EXPANSION", 1LL, 5824},
    {"CX_NFUNC", 1LL, 4317},
    {"CX_NFUNC_WD", 3LL, 4396},
    {"CX_NHQ", 1LL, 4198},
    {"CX_NW", 16LL, 4006},
    {"CX_BUSNUM_WD", 8LL, 4701},
    {"CX_DEVNUM_WD", 5LL, 4717},
    {"CX_LBC_NW", 2LL, 8610},
    {"CX_MSI_NFUNC", 1LL, 4367},
    {"CX_RADM_NFC_TRGT_INTF", 1LL, 4266},
    {"CX_NL", 4LL, 3145},
};

static const CoreCfgParam kCfg_config_epx1cxl[] = {
    {"TRGT_DATA_WD", 256LL, 25098},
    {"CX_TAG_SIZE", 8LL, 15295},
    {"FLT_Q_ADDR_WIDTH", 64LL, 5876},
    {"FLT_Q_ATTR_WIDTH", 2LL, 11280},
    {"CX_NVC", 1LL, 5171},
    {"CX_NVC_XALI_EXPANSION", 1LL, 6059},
    {"CX_NFUNC", 1LL, 4753},
    {"CX_NFUNC_WD", 3LL, 4832},
    {"CX_NHQ", 2LL, 4692},
    {"CX_NW", 8LL, 4528},
    {"CX_BUSNUM_WD", 8LL, 5149},
    {"CX_DEVNUM_WD", 5LL, 5157},
    {"CX_LBC_NW", 2LL, 9037},
    {"CX_MSI_NFUNC", 1LL, 4803},
    {"CX_RADM_NFC_TRGT_INTF", 2LL, 5316},
    {"CX_NL", 4LL, 3489},
};

static const CoreCfgParam kCfg_config_root3[] = {
    {"TRGT_DATA_WD", 64LL, 23139},
    {"CX_TAG_SIZE", 8LL, 14562},
    {"FLT_Q_ADDR_WIDTH", 64LL, 5657},
    {"FLT_Q_ATTR_WIDTH", 2LL, 10589},
    {"CX_NVC", 1LL, 4733},
    {"CX_NVC_XALI_EXPANSION", 1LL, 5830},
    {"CX_NFUNC", 1LL, 4323},
    {"CX_NFUNC_WD", 3LL, 4402},
    {"CX_NHQ", 1LL, 4204},
    {"CX_NW", 2LL, 4002},
    {"CX_BUSNUM_WD", 8LL, 4707},
    {"CX_DEVNUM_WD", 5LL, 4723},
    {"CX_LBC_NW", 1LL, 8613},
    {"CX_MSI_NFUNC", 1LL, 4373},
    {"CX_RADM_NFC_TRGT_INTF", 1LL, 4272},
    {"CX_NL", 1LL, 3141},
};

static const CoreCfgParam kCfg_config_root_cxl[] = {
    {"TRGT_DATA_WD", 512LL, 23054},
    {"CX_TAG_SIZE", 10LL, 14552},
    {"FLT_Q_ADDR_WIDTH", 64LL, 5651},
    {"FLT_Q_ATTR_WIDTH", 2LL, 10578},
    {"CX_NVC", 1LL, 4727},
    {"CX_NVC_XALI_EXPANSION", 1LL, 5824},
    {"CX_NFUNC", 1LL, 4317},
    {"CX_NFUNC_WD", 3LL, 4396},
    {"CX_NHQ", 1LL, 4198},
    {"CX_NW", 16LL, 4006},
    {"CX_BUSNUM_WD", 8LL, 4701},
    {"CX_DEVNUM_WD", 5LL, 4717},
    {"CX_LBC_NW", 2LL, 8610},
    {"CX_MSI_NFUNC", 1LL, 4367},
    {"CX_RADM_NFC_TRGT_INTF", 1LL, 4266},
    {"CX_NL", 4LL, 3145},
};

static const CoreCfgParam kCfg_config_sw3[] = {
    {"TRGT_DATA_WD", 64LL, 23135},
    {"CX_TAG_SIZE", 8LL, 14559},
    {"FLT_Q_ADDR_WIDTH", 64LL, 5658},
    {"FLT_Q_ATTR_WIDTH", 2LL, 10586},
    {"CX_NVC", 1LL, 4734},
    {"CX_NVC_XALI_EXPANSION", 1LL, 5831},
    {"CX_NFUNC", 1LL, 4324},
    {"CX_NFUNC_WD", 3LL, 4403},
    {"CX_NHQ", 1LL, 4205},
    {"CX_NW", 2LL, 4003},
    {"CX_BUSNUM_WD", 8LL, 4708},
    {"CX_DEVNUM_WD", 5LL, 4724},
    {"CX_LBC_NW", 1LL, 8610},
    {"CX_MSI_NFUNC", 1LL, 4374},
    {"CX_RADM_NFC_TRGT_INTF", 1LL, 4273},
    {"CX_NL", 1LL, 3142},
};

static const CoreCfgParam kCfg_config_sw3dn[] = {
    {"TRGT_DATA_WD", 64LL, 23042},
    {"CX_TAG_SIZE", 8LL, 14541},
    {"FLT_Q_ADDR_WIDTH", 64LL, 5648},
    {"FLT_Q_ATTR_WIDTH", 2LL, 10568},
    {"CX_NVC", 1LL, 4724},
    {"CX_NVC_XALI_EXPANSION", 1LL, 5821},
    {"CX_NFUNC", 1LL, 4314},
    {"CX_NFUNC_WD", 3LL, 4393},
    {"CX_NHQ", 1LL, 4195},
    {"CX_NW", 2LL, 4003},
    {"CX_BUSNUM_WD", 8LL, 4698},
    {"CX_DEVNUM_WD", 5LL, 4714},
    {"CX_LBC_NW", 1LL, 8600},
    {"CX_MSI_NFUNC", 1LL, 4364},
    {"CX_RADM_NFC_TRGT_INTF", 1LL, 4263},
    {"CX_NL", 1LL, 3142},
};

static const CoreCfgParam kCfg_config_sw5cxl[] = {
    {"TRGT_DATA_WD", 512LL, 25085},
    {"CX_TAG_SIZE", 10LL, 15281},
    {"FLT_Q_ADDR_WIDTH", 64LL, 5875},
    {"FLT_Q_ATTR_WIDTH", 2LL, 11266},
    {"CX_NVC", 1LL, 5170},
    {"CX_NVC_XALI_EXPANSION", 1LL, 6058},
    {"CX_NFUNC", 1LL, 4752},
    {"CX_NFUNC_WD", 3LL, 4831},
    {"CX_NHQ", 1LL, 4691},
    {"CX_NW", 16LL, 4527},
    {"CX_BUSNUM_WD", 8LL, 5148},
    {"CX_DEVNUM_WD", 5LL, 5156},
    {"CX_LBC_NW", 2LL, 9027},
    {"CX_MSI_NFUNC", 1LL, 4802},
    {"CX_RADM_NFC_TRGT_INTF", 1LL, 5315},
    {"CX_NL", 16LL, 3489},
};

static const CoreCfgParam kCfg_config_sw5x16cxl[] = {
    {"TRGT_DATA_WD", 512LL, 25085},
    {"CX_TAG_SIZE", 10LL, 15281},
    {"FLT_Q_ADDR_WIDTH", 64LL, 5875},
    {"FLT_Q_ATTR_WIDTH", 2LL, 11266},
    {"CX_NVC", 1LL, 5170},
    {"CX_NVC_XALI_EXPANSION", 1LL, 6058},
    {"CX_NFUNC", 1LL, 4752},
    {"CX_NFUNC_WD", 3LL, 4831},
    {"CX_NHQ", 1LL, 4691},
    {"CX_NW", 16LL, 4527},
    {"CX_BUSNUM_WD", 8LL, 5148},
    {"CX_DEVNUM_WD", 5LL, 5156},
    {"CX_LBC_NW", 2LL, 9027},
    {"CX_MSI_NFUNC", 1LL, 4802},
    {"CX_RADM_NFC_TRGT_INTF", 1LL, 5315},
    {"CX_NL", 16LL, 3489},
};

static const CoreCfgEntry kCfgTable[] = {
    {"config_cxl.old", "config_cxl.old/src/cxl_DWC_pcie_ctl_cc_constants.svh", 16, kCfg_config_cxl_old},
    {"config_ep", "config_ep/src/DWC_pcie_ctl_cc_constants.svh", 16, kCfg_config_ep},
    {"config_ep_cxl", "config_ep_cxl/src/DWC_pcie_ctl_cc_constants.svh", 16, kCfg_config_ep_cxl},
    {"config_epx1cxl", "config_epx1cxl/src/Tiger_DWC_pcie_ctl_cc_constants.svh", 16, kCfg_config_epx1cxl},
    {"config_root3", "config_root3/src/DWC_pcie_ctl_cc_constants.svh", 16, kCfg_config_root3},
    {"config_root_cxl", "config_root_cxl/src/DWC_pcie_ctl_cc_constants.svh", 16, kCfg_config_root_cxl},
    {"config_sw3", "config_sw3/src/DWC_pcie_ctl_cc_constants.svh", 16, kCfg_config_sw3},
    {"config_sw3dn", "config_sw3dn/src/dwc_pcie_ctl_cc_constants.svh", 16, kCfg_config_sw3dn},
    {"config_sw5cxl", "config_sw5cxl/src/DWC_pcie_ctl_cc_constants.svh", 16, kCfg_config_sw5cxl},
    {"config_sw5x16cxl", "config_sw5x16cxl/src/DWC_pcie_ctl_cc_constants.svh", 16, kCfg_config_sw5x16cxl},
};
static const int kCfgTableN = 10;

} 
