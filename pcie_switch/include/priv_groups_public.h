

#pragma once

namespace pcie_tlm {

struct priv_group_widths {
    static constexpr int W_client0_in = 242;
    static constexpr int W_client0_out = 1;
    static constexpr int W_client1_in = 242;
    static constexpr int W_client1_out = 1;
    static constexpr int W_dbi_in = 70;
    static constexpr int W_dbi_out = 33;
    static constexpr int W_err_in = 3;
    static constexpr int W_err_out = 3;
    static constexpr int W_misc_in = 4;
    static constexpr int W_misc_out = 2;
    static constexpr int W_msi_in = 12;
    static constexpr int W_msi_out = 2;
    static constexpr int W_radm_out = 313;
    static constexpr int W_sideband_out = 62;
    static constexpr int W_trgt1_in = 4;
};

} 
