#include "interface.h"
#include "json.hpp"

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

int main()
{
    using json = nlohmann::json;
    std::string prob;
    int nvar, niter, nfun;
    double objval, proj_grad, setup_time, solve_time;

    lbfgsb_stat(prob, nvar, niter, nfun, objval, proj_grad, setup_time, solve_time, false);
    json lbfgsb = {
        {"problem", trim_space(prob)},
        {"nvar", nvar},
        {"niter", niter},
        {"nfun", nfun},
        {"objval", objval},
        {"proj_grad", proj_grad},
        {"setup_time", setup_time},
        {"solve_time", solve_time}
    };

    /*
    std::cout << "#####################################################" << std::endl;
    std::cout << "Solve                 = L-BFGS-B" << std::endl;
    std::cout << "# variables           = " << nvar << std::endl;
    std::cout << "# iterations          = " << niter << std::endl;
    std::cout << "# function calls      = " << nfun << std::endl;
    std::cout << "Final f               = " << objval << std::endl;
    std::cout << "Final ||proj_grad||   = " << proj_grad << std::endl;
    std::cout << "Setup time            = " << setup_time << " s" << std::endl;
    std::cout << "Solve time            = " << solve_time << " s" << std::endl;
    */

    lbfgspp_stat(prob, nvar, niter, nfun, objval, proj_grad, setup_time, solve_time, false);
    json lbfgspp = {
        {"problem", trim_space(prob)},
        {"nvar", nvar},
        {"niter", niter},
        {"nfun", nfun},
        {"objval", objval},
        {"proj_grad", proj_grad},
        {"setup_time", setup_time},
        {"solve_time", solve_time}
    };

    /*
    std::cout << "#####################################################" << std::endl;
    std::cout << "Solve                 = LBFGS++" << std::endl;
    std::cout << "# variables           = " << nvar << std::endl;
    std::cout << "# iterations          = " << niter << std::endl;
    std::cout << "# function calls      = " << nfun << std::endl;
    std::cout << "Final f               = " << objval << std::endl;
    std::cout << "Final ||proj_grad||   = " << proj_grad << std::endl;
    std::cout << "Setup time            = " << setup_time << " s" << std::endl;
    std::cout << "Solve time            = " << solve_time << " s" << std::endl;
    std::cout << "#####################################################" << std::endl;
    */

    json stats = {
        {"L-BFGS-B", lbfgsb},
        {"LBFGS++", lbfgspp}
    };
    std::cout << stats.dump(2) << std::endl;

    return 0;
}
