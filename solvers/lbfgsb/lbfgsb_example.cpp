#include <iostream>
#include <Eigen/Core>

using Vector = Eigen::VectorXd;
using IntVector = Eigen::VectorXi;

// Fortran L-BFGS-B function
extern "C" {

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
class Rosenbrock
{
private:
    int n;
public:
    Rosenbrock(int n_) : n(n_) {}
    double operator()(const Vector& x, Vector& grad)
    {
        double fx = 0.0;
        for(int i = 0; i < n; i += 2)
        {
            double t1 = 1.0 - x[i];
            double t2 = 10 * (x[i + 1] - x[i] * x[i]);
            grad[i + 1] = 20 * t2;
            grad[i]     = -2.0 * (x[i] * grad[i + 1] + t1);
            fx += t1 * t1 + t2 * t2;
        }
        return fx;
    }
};

int main()
{
    // Set up problem size
    const int n = 10;
    // Create problem
    Rosenbrock fun(n);
    // Bounds for variables
    Vector lb = Vector::Constant(n, 2.0);
    Vector ub = Vector::Constant(n, 4.0);
    // Initial guess
    Vector x = Vector::Constant(n, 3.0);
    // Space to store gradient
    Vector grad = Vector::Zero(n);
    // Maximum number of iterations
    const int maxiter = 1000;

    // Set up solver parameters
    // Number of correction vectors in L-BFGS-B
    const int m = 10;
    // Bound type indicators
    IntVector nbd(n);
    const double inf = std::numeric_limits<double>::infinity();
    for(int i = 0; i < n; i++)
    {
        if(lb[i] == -inf)
        {
            nbd[i] = (ub[i] == inf) ? 0 : 3;
        } else {
            nbd[i] = (ub[i] == inf) ? 1 : 2;
        }
    }
    // Convergence tolerance
    const double factr = 1e7;
    const double pgtol = 1e-5;
    // Printing options
    // Do not print
    const int iprint = -1;
    // Working space and flags
    Vector wa(2 * m * n + 11 * m * m + 5 * n + 8 * m);
    IntVector iwa(3 * n);
    int itask;
    int icsave;
    int lsave[4];
    int isave[44];
    double dsave[29];

    // Optimization process
    double objval;
    itask = 2;
    int i = 0;
    while (i < maxiter)
    {
        // Call L-BFGS-B routine
        setulb_(&n, &m, x.data(), lb.data(), ub.data(), nbd.data(),
            &objval, grad.data(), &factr, &pgtol,
            wa.data(), iwa.data(), &itask, &iprint,
            &icsave, lsave, isave, dsave);

        std::cout << "i = " << i << ", itask = " << itask << std::endl;
        if (itask == 4 || itask == 20 || itask == 21)
        {
            // Compute objective function value and gradient
            objval = fun(x, grad);
            std::cout << "   x      = " <<  x.transpose() << std::endl;
            std::cout << "   grad   = " <<  grad.transpose() << std::endl;
            std::cout << "   objval = " <<  objval << std::endl;
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

    // Print final results
    std::cout << "================================================" << std::endl;
    std::cout << "x    = " << x.transpose() << std::endl;
    std::cout << "grad = " << grad.transpose() << std::endl;
    std::cout << "# iterations        = " << i + 1 << std::endl;
    std::cout << "# function calls    = " << isave[33] << std::endl;
    std::cout << "Final f             = " << objval << std::endl;
    std::cout << "Final ||proj_grad|| = " << dsave[12] << std::endl;

    return 0;
}
