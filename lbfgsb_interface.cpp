#include <iostream>
#include <Eigen/Core>

extern "C" {

#include <cutest.h>

// Fortran L-BFGS-B function
void setulb_(
    const int* n, const int* m,
    double* x, const double* l, const double* u, const int* nbd,
    double* f, double* g,
    const double* factr, const double* pgtol,
    double* wa, int* iwa,
    int* itask, const int* iprint,
    int* icsave, int* lsave, int* isave, double* dsave
);

}

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
            std::cout << "itask = " << itask << std::endl;
            throw std::runtime_error("abnormal exit");
        }
    }

    doublereal calls[4], time[2];
    CUTEST_ureport(&status, calls, time);

    std::cout << "x = " << x.transpose().head(5) << " ... " << x.transpose().tail(5) << std::endl << std::endl;

    std::cout << "# variables           = " << CUTEst_nvar << std::endl;
    std::cout << "# iterations          = " << i << std::endl;
    std::cout << "# function calls      = " << calls[0] << std::endl;
    std::cout << "Final f               = " << fx << std::endl;
    std::cout << "Final ||proj_grad||   = " << dsave[12] << std::endl;
    std::cout << "Setup time            = " << time[0] << " s" << std::endl;
    std::cout << "Solve time            = " << time[1] << " s" << std::endl;

    CUTEST_uterminate(&status);

    return 0;
}
