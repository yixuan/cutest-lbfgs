#include "interface.h"
#include <LBFGS.h>
using namespace LBFGSpp;

void unconstr_lbfgs_stat(CUTEstStat& stat, bool verbose)
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
        stat.msg = "Problem contains general constrints.";
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

    // Algorithm parameters
    const integer param_m = 6;
    const integer param_maxit = 10000;
    const doublereal param_eps = 1e-5;
    // Machine precision
    const doublereal param_xtol = std::numeric_limits<doublereal>::epsilon();
    // Do not provide H0
    const integer diagco = 0;
    // Reuse the memory of lb
    Vector& diag = lb;

    // Objective function
    CUTEstProblem fun(CUTEst_nvar);
    doublereal fx;
    Vector grad(CUTEst_nvar);

    // Printing options
    integer iprint[2];
    iprint[0] = -1;  // Do not print
    iprint[1] = 0;   // 0-3, larger value for more output

    // Working space and flags
    Vector work(CUTEst_nvar * (2 * param_m + 1) + 2 * param_m);
    integer iflag = 0;

    // Optimization process
    integer i;
    for (i = 0; i < param_maxit; i++)
    {
        // Compute objective function value and gradient
        fx = fun(x, grad);
        // Call L-BFGS routine
        lbfgs_(&CUTEst_nvar, &param_m, x.data(), &fx, grad.data(),
            &diagco, diag.data(), iprint, &param_eps, &param_xtol,
            work.data(), &iflag);
        // If iflag = 1, then continue iteration
        if (iflag == 1)
        {
            continue;
        } // If iflag = 0, then the solver finishes
        else if (iflag == 0) {
            break;
        } // If iflag < 0, then some error occurs
        else
        {
            // Errors
            stat.prob = std::string(prob_name);
            stat.nvar = CUTEst_nvar;
            stat.flag = 1;
            stat.msg = std::string("L-BFGS solver failed with code ") +
                std::to_string(iflag);
            CUTEST_uterminate(&status);
            return;
        }
    }

    doublereal calls[4], time[2];
    CUTEST_ureport(&status, calls, time);

    if(verbose)
        std::cout << "x = " << x.transpose().head(5) << " ... " << x.transpose().tail(5) << std::endl << std::endl;

    stat.prob = std::string(prob_name);
    stat.nvar = CUTEst_nvar;
    stat.niter = i + 1;
    stat.nfun = calls[0];
    stat.objval = fx;
    stat.proj_grad = grad.norm();
    stat.setup_time = time[0];
    stat.solve_time = time[1];

    CUTEST_uterminate(&status);
}
