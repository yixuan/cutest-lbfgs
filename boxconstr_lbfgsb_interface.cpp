// Copyright (C) 2023 Yixuan Qiu <yixuan.qiu@cos.name>
// Under MIT license

#include "interface.h"

void boxconstr_lbfgsb_stat(CUTEstStat& stat, bool verbose)
{
    using Vector = Eigen::Matrix<doublereal, Eigen::Dynamic, 1>;
    using IntVector = Eigen::VectorXi;

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
        CUTEST_uterminate(&status);
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
        CUTEST_uterminate(&status);
        return;
    }

    // Algorithm parameters
    const integer param_m = 6;
    const integer param_maxit = 10000;
    // const doublereal param_factr = 0.0;
    const doublereal param_factr = 1e7;
    const doublereal param_pgtol = 1e-5;

    // Bound type indicators
    IntVector nbd(CUTEst_nvar);
    const doublereal inf = std::numeric_limits<double>::infinity();
    for(int i = 0; i < CUTEst_nvar; i++)
    {
        if(lb[i] == -inf)
        {
            nbd[i] = (ub[i] == inf) ? 0 : 3;
        } else {
            nbd[i] = (ub[i] == inf) ? 1 : 2;
        }
    }

    // Objective function
    CUTEstProblem fun(CUTEst_nvar);
    doublereal fx;
    Vector grad(CUTEst_nvar);

    // Printing options
    // Do not print
    const integer iprint = -1;
    // Working space and flags
    Vector wa(2 * param_m * CUTEst_nvar + 11 * param_m * param_m + 5 * CUTEst_nvar + 8 * param_m);
    IntVector iwa(3 * CUTEst_nvar);
    integer itask;
    integer icsave;
    integer lsave[4];
    integer isave[44];
    double dsave[29];

    // Optimization process
    itask = 2;
    int i = 0;
    while (i < param_maxit)
    {
        // Call L-BFGS-B routine
        setulb_(&CUTEst_nvar, &param_m, x.data(), lb.data(), ub.data(), nbd.data(),
            &fx, grad.data(), &param_factr, &param_pgtol,
            wa.data(), iwa.data(), &itask, &iprint,
            &icsave, lsave, isave, dsave);

        // std::cout << "i = " << i << ", itask = " << itask << std::endl;
        if (itask == 4 || itask == 20 || itask == 21)
        {
            // Compute objective function value and gradient
            fx = fun(x, grad);
            // std::cout << "   x    = " <<  x.transpose() << std::endl;
            // std::cout << "   grad = " <<  grad.transpose() << std::endl;
            // std::cout << "   fx   = " <<  fx << std::endl;
        } else if (itask >= 6 && itask <= 8) {
            // Converged
            break;
        } else if (itask == 1) {
            // New x, update iteration number
            i = isave[29];
        } else {
            // Errors
            stat.prob = std::string(prob_name);
            stat.nvar = CUTEst_nvar;
            stat.flag = 1;
            stat.msg = std::string("Solver abnormal exit. itask = ") +
                std::to_string(itask);
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
    stat.niter = i;
    stat.nfun = calls[0];
    stat.objval = fx;
    stat.proj_grad = dsave[12];
    stat.setup_time = time[0];
    stat.solve_time = time[1];

    CUTEST_uterminate(&status);
}
