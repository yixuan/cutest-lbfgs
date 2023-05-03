// Copyright (C) 2023 Yixuan Qiu <yixuan.qiu@cos.name>
// Under MIT license

#ifndef CUTEST_INTERFACE_H
#define CUTEST_INTERFACE_H

#include <iostream>
#include <stdexcept>
#include <Eigen/Core>
#include "json.hpp"

extern "C" {

// CUTEst types and functions
#include <cutest.h>

// Fortran L-BFGS function
void lbfgs_(
    const int* n, const int* m,
    double* x, const double* f, const double* g,
    const int* diagco, const double* diag,
    const int* iprint, const double* eps, const double* xtol,
    double* w, int* iflag
);

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

// Statistics
struct CUTEstStat
{
    std::string prob;        // Problem name
    int         flag;        // 0-normal, 1-solver error, 2-problem error
    std::string msg;         // Error message
    int         nvar;        // Number of variables
    int         niter;       // Number of iterations
    int         nfun;        // Number of function evluations
    double      objval;      // Final objective function value
    double      proj_grad;   // Final (projected) gradient
    double      setup_time;  // Time for setup
    double      solve_time;  // Time for solving
};

// Interface
void unconstr_lbfgs_stat(CUTEstStat& stat, bool verbose = false);
void unconstr_lbfgspp_stat(CUTEstStat& stat, bool verbose = false);
void boxconstr_lbfgsb_stat(CUTEstStat& stat, bool verbose = false);
void boxconstr_lbfgspp_stat(CUTEstStat& stat, bool verbose = false);

// Helper functions
void print_stat(const CUTEstStat& stat);

// Convert CUTEstStat object to JSON
nlohmann::json stat_to_json(const CUTEstStat& stat);


#endif  // CUTEST_INTERFACE_H
