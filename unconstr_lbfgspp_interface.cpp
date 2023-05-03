#include "interface.h"
#include <LBFGS.h>
using namespace LBFGSpp;

void unconstr_lbfgspp_stat(CUTEstStat& stat, bool verbose)
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
    // Number of general constraints
    integer CUTEst_nconstr;
    CUTEST_cdimen(&status, &funit, &CUTEst_nvar, &CUTEst_nconstr);
    if(status)
    {
        stat.flag = 2;
        stat.msg = "Error getting problem dimension.";
        return;
    }
    if(CUTEst_nconstr > 0)
    {
        stat.flag = 2;
        stat.msg = "Problem contains general constraints.";
        return;
    }
    if(verbose)
        std::cout << "nvar = " << CUTEst_nvar << std::endl;

    // Reserve memory for variables and bounds,
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
        CUTEST_uterminate(&status);
        return;
    }
    // Even for unconstrained problems, lb and ub will be specified,
    // but with a "fake" infinity value of +/- 1e20
    // We need to make sure the problem is indeed unconstrained
    const doublereal near_inf = 9.0e19;
    if(lb.maxCoeff() > -near_inf || ub.minCoeff() < near_inf)
    {
        stat.flag = 2;
        stat.msg = "Problem is not unconstrained.";
        CUTEST_uterminate(&status);
        return;
    }
    if(verbose)
        std::cout << "x0 = " <<  x.transpose().head(10) << " ... " <<  x.transpose().tail(10) << std::endl;

    // Problem name
    char prob_name[16];
    std::fill(prob_name, prob_name + 16, 0);
    CUTEST_probname(&status, prob_name);
    if(status)
    {
        stat.flag = 2;
        stat.msg = "Error getting problem name.";
        CUTEST_uterminate(&status);
        return;
    }

    // Set up LBFGS++ parameters
    LBFGSParam<doublereal> param;
    param.m = 6;
    // For very large problems, restrict to 1000 iterations
    param.max_iterations = (CUTEst_nvar < 50000) ? 10000 : 1000;
    param.epsilon = 1e-5;
    param.epsilon_rel = 1e-5;
    param.past = 0;
    param.delta = 0.0;
    param.max_linesearch = 100;

    // Objective function
    CUTEstProblem fun(CUTEst_nvar);
    doublereal fx;

    // Solver
    LBFGSSolver<doublereal> solver(param);
    int niter;
    try {
        niter = solver.minimize(fun, x, fx);
    } catch (std::exception& e) {
        stat.prob = std::string(prob_name);
        stat.nvar = CUTEst_nvar;
        stat.flag = 1;
        stat.msg = e.what();
        CUTEST_uterminate(&status);
        return;
    }

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
