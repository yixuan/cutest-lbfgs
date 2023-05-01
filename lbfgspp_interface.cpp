#include <iostream>
#include <Eigen/Core>

extern "C" {
#include <cutest.h>
}

#include <LBFGSB.h>
using namespace LBFGSpp;

// Problem class
class CUTEstProblem
{
private:
    using Vector = Eigen::Matrix<doublereal, Eigen::Dynamic, 1>;
    integer n;
public:
    CUTEstProblem(integer n_) : n(n_) {}

    doublereal operator()(const Vector& x, Vector& grad)
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
};

int main()
{
    using Vector = Eigen::Matrix<doublereal, Eigen::Dynamic, 1>;

    // Open problem description file OUTSDIF.d
    const char fname[] = "OUTSDIF.d";
    // FORTRAN unit number for OUTSDIF.d
    integer funit = 42;
    // Exit flag from OPEN and CLOSE
    integer ierr = 0;
    FORTRAN_open(&funit, fname, &ierr);
    if(ierr)
    {
        std::cout << "Error opening file OUTSDIF.d.\nAborting.\n";
        return 1;
    }

    // Determine problem size
    // Exit flag from CUTEst tools
    integer status;
    // Number of variables
    integer CUTEst_nvar;
    CUTEST_udimen(&status, &funit, &CUTEst_nvar);
    if(status)
    {
        std::cout << "** CUTEst error, status = " << status << ", aborting\n";
        return 1;
    }
    std::cout << "nvar = " << CUTEst_nvar << std::endl;

    // Reserve memory for variables, bounds, and multipliers,
    // and call appropriate initialization routine for CUTEst
    Vector x(CUTEst_nvar);
    Vector lb(CUTEst_nvar);
    Vector ub(CUTEst_nvar);
    // FORTRAN unit number for error output
    integer iout = 6;
    // FORTRAN unit internal input/output
    integer io_buffer = 11;
    CUTEST_usetup(&status, &funit, &iout, &io_buffer,
                  &CUTEst_nvar, x.data(), lb.data(), ub.data());
    if(status)
    {
        std::cout << "** CUTEst error, status = " << status << ", aborting\n";
        return status;
    }
    std::cout << "x0 = " <<  x.transpose().head(10) << " ... " <<  x.transpose().tail(10) << std::endl;
    std::cout << "lb = " << lb.transpose().head(10) << " ... " << lb.transpose().tail(10) << std::endl;
    std::cout << "ub = " << ub.transpose().head(10) << " ... " << ub.transpose().tail(10) << std::endl << std::endl;

    // Set up LBFGS++ parameters
    LBFGSBParam<doublereal> param;
    param.m = 6;
    param.max_iterations = 10000;
    // param.delta = 0.0;
    param.delta = 1e7 * std::numeric_limits<doublereal>::epsilon();
    param.epsilon = 1e-5;
    param.epsilon_rel = 0.0;
    param.past = 1;
    param.max_submin = 0;
    param.max_linesearch = 100;

    // Objective function
    CUTEstProblem fun(CUTEst_nvar);
    doublereal fx;

    // Solver
    LBFGSBSolver<doublereal> solver(param);
    integer niter = solver.minimize(fun, x, fx, lb, ub);

    doublereal calls[4], time[2];
    CUTEST_ureport(&status, calls, time);

    std::cout << "x = " << x.transpose().head(5) << " ... " << x.transpose().tail(5) << std::endl << std::endl;

    std::cout << "# variables           = " << CUTEst_nvar << std::endl;
    std::cout << "# iterations          = " << niter << std::endl;
    std::cout << "# function calls      = " << calls[0] << std::endl;
    std::cout << "Final f               = " << fx << std::endl;
    std::cout << "Final ||proj_grad||   = " << solver.final_grad_norm() << std::endl;
    std::cout << "Setup time            = " << time[0] << " s" << std::endl;
    std::cout << "Solve time            = " << time[1] << " s" << std::endl;

    CUTEST_uterminate(&status);

    return 0;
}
