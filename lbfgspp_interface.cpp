#include "interface.h"
#include <LBFGSB.h>
using namespace LBFGSpp;

void lbfgspp_stat(CUTEstStat& stat, bool verbose)
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
        stat.flag = 2;
        stat.msg = "Error opening file OUTSDIF.d.";
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
        stat.flag = 2;
        stat.msg = "Error getting problem dimension.";
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
        stat.flag = 2;
        stat.msg = "Error setting up problem.";
        return;
    }
    if(verbose)
    {
        std::cout << "x0 = " <<  x.transpose().head(10) << " ... " <<  x.transpose().tail(10) << std::endl;
        std::cout << "lb = " << lb.transpose().head(10) << " ... " << lb.transpose().tail(10) << std::endl;
        std::cout << "ub = " << ub.transpose().head(10) << " ... " << ub.transpose().tail(10) << std::endl << std::endl;
    }

    // Problem name
    char prob_name[16];
    std::fill(prob_name, prob_name + 16, 0);
    CUTEST_probname(&status, prob_name);
    if(status)
    {
        stat.flag = 2;
        stat.msg = "Error getting problem name.";
        return;
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
    int niter = solver.minimize(fun, x, fx, lb, ub);

    doublereal calls[4], time[2];
    CUTEST_ureport(&status, calls, time);

    if(verbose)
        std::cout << "x = " << x.transpose().head(5) << " ... " << x.transpose().tail(5) << std::endl << std::endl;

    stat.prob = std::string(prob_name);
    stat.nvar = CUTEst_nvar;
    stat.niter = niter;
    stat.nfun = calls[0];
    stat.objval = fx;
    stat.proj_grad = solver.final_grad_norm();
    stat.setup_time = time[0];
    stat.solve_time = time[1];

    CUTEST_uterminate(&status);
}
