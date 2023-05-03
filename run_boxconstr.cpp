// Copyright (C) 2023 Yixuan Qiu <yixuan.qiu@cos.name>
// Under MIT license

#include "interface.h"

int main()
{
    using json = nlohmann::json;

    CUTEstStat stat1, stat2;

    boxconstr_lbfgsb_stat(stat1, false);
    json lbfgsb = stat_to_json(stat1);
    lbfgsb["alg"] = "L-BFGS-B";
    lbfgsb["solver"] = "Classic";

    // std::cout << "#####################################################" << std::endl;
    // std::cout << "Solver                = L-BFGS-B" << std::endl;
    // print_stat(stat1);

    boxconstr_lbfgspp_stat(stat2, false);
    json lbfgspp = stat_to_json(stat2);
    lbfgspp["alg"] = "L-BFGS-B";
    lbfgspp["solver"] = "LBFGS++";

    // std::cout << "#####################################################" << std::endl;
    // std::cout << "Solver                = LBFGS++" << std::endl;
    // print_stat(stat2);
    // std::cout << "#####################################################" << std::endl;

    std::cout << lbfgsb.dump(2) << std::endl;
    std::cout << lbfgspp.dump(2) << std::endl;

    return 0;
}
