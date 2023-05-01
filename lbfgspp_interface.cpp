#include "interface.h"
#include <LBFGSB.h>
using namespace LBFGSpp;

void lbfgspp_stat(
    int& nvar, int& niter, int& nfun,
    double& objval, double& proj_grad,
    double& setup_time, double& solve_time,
    bool verbose)
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
        return;
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
        return;
    }
    if(verbose)
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
        return;
    }
    if(verbose)
    {
        std::cout << "x0 = " <<  x.transpose().head(10) << " ... " <<  x.transpose().tail(10) << std::endl;
        std::cout << "lb = " << lb.transpose().head(10) << " ... " << lb.transpose().tail(10) << std::endl;
        std::cout << "ub = " << ub.transpose().head(10) << " ... " << ub.transpose().tail(10) << std::endl << std::endl;
    }

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
    niter = solver.minimize(fun, x, fx, lb, ub);

    doublereal calls[4], time[2];
    CUTEST_ureport(&status, calls, time);

    if(verbose)
        std::cout << "x = " << x.transpose().head(5) << " ... " << x.transpose().tail(5) << std::endl << std::endl;

    nvar = CUTEst_nvar;
    nfun = calls[0];
    objval = fx;
    proj_grad = solver.final_grad_norm();
    setup_time = time[0];
    solve_time = time[1];

    CUTEST_uterminate(&status);
}
