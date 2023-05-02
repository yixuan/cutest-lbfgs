#include "interface.h"
#include "json.hpp"

using json = nlohmann::json;

// Constructor
CUTEstProblem::CUTEstProblem(integer n_) : n(n_) {}

// Compute objective function value and gradient
doublereal CUTEstProblem::operator()(const Vector& x, Vector& grad)
{
    integer status;  // Exit flag from CUTEst tools
    logical comp_grad = 1;  // Compute gradient
    doublereal fx;
    CUTEST_uofg(&status, &n, x.data(), &fx, grad.data(), &comp_grad);
    if(status)
    {
        throw std::runtime_error("** CUTEst error");
    }
    return fx;
}

// Trim trailing whitespace
inline std::string trim_space(const std::string& name)
{
    std::size_t trim = name.find_last_not_of(' ');
    return name.substr(0, trim + 1);
}

void print_stat(const CUTEstStat& stat)
{
    std::cout << "Problem               = " << stat.prob << std::endl;
    std::cout << "Flag                  = " << stat.flag << std::endl;
    std::cout << "# variables           = " << stat.nvar << std::endl;
    std::cout << "# iterations          = " << stat.niter << std::endl;
    std::cout << "# function calls      = " << stat.nfun << std::endl;
    std::cout << "Final f               = " << stat.objval << std::endl;
    std::cout << "Final ||proj_grad||   = " << stat.proj_grad << std::endl;
    std::cout << "Setup time            = " << stat.setup_time << " s" << std::endl;
    std::cout << "Solve time            = " << stat.solve_time << " s" << std::endl;
}

// Convert CUTEstStat object to JSON
json stat_to_json(const CUTEstStat& stat)
{
    json data = {
        {"problem", trim_space(stat.prob)},
        {"flag", stat.flag},
        {"msg", stat.msg},
        {"nvar", stat.nvar},
        {"niter", stat.niter},
        {"nfun", stat.nfun},
        {"objval", stat.objval},
        {"proj_grad", stat.proj_grad},
        {"setup_time", stat.setup_time},
        {"solve_time", stat.solve_time}
    };
    return data;
}
