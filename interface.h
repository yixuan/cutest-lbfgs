#include <iostream>
#include <stdexcept>
#include <Eigen/Core>

extern "C" {

// CUTEst types and functions
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
    CUTEstProblem(integer n_);

    doublereal operator()(const Vector& x, Vector& grad);
};

// Interface
void lbfgsb_stat(
    int& nvar, int& niter, int& nfun,
    double& objval, double& proj_grad,
    double& setup_time, double& solve_time,
    bool verbose = false);

void lbfgspp_stat(
    int& nvar, int& niter, int& nfun,
    double& objval, double& proj_grad,
    double& setup_time, double& solve_time,
    bool verbose = false);
