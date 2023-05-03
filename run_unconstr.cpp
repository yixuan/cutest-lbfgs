// Copyright (C) 2023 Yixuan Qiu <yixuan.qiu@cos.name>
// Under MIT license

#include "interface.h"

int main()
{
    using json = nlohmann::json;

    CUTEstStat stat1, stat2;

    unconstr_lbfgs_stat(stat1, false);
    json lbfgs = stat_to_json(stat1);
    lbfgs["alg"] = "L-BFGS";
    lbfgs["solver"] = "Classic";

    // std::cout << "#####################################################" << std::endl;
    // std::cout << "Solver                = L-BFGS" << std::endl;
    // print_stat(stat1);

    unconstr_lbfgspp_stat(stat2, false);
    json lbfgspp = stat_to_json(stat2);
    lbfgspp["alg"] = "L-BFGS";
    lbfgspp["solver"] = "LBFGS++";

    // std::cout << "#####################################################" << std::endl;
    // std::cout << "Solver                = LBFGS++" << std::endl;
    // print_stat(stat2);
    // std::cout << "#####################################################" << std::endl;

    std::cout << lbfgs.dump(2) << std::endl;
    std::cout << lbfgspp.dump(2) << std::endl;

    return 0;
}
