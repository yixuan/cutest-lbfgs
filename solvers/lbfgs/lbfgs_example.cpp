#include <iostream>
#include <Eigen/Core>

using Vector = Eigen::VectorXd;

// Fortran L-BFGS function
extern "C" {

void lbfgs_(
    const int* n, const int* m,
    double* x, const double* f, const double* g,
    const int* diagco, const double* diag,
    const int* iprint, const double* eps, const double* xtol,
    double* w, int* iflag
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
    // Initial guess
    Vector x = Vector::Zero(n);
    // Space to store gradient
    Vector grad = Vector::Zero(n);
    // Maximum number of iterations
    const int maxiter = 1000;

    // Set up solver parameters
    // Number of correction vectors in L-BFGS
    const int m = 10;
    // Do not provide H0
    const int diagco = 0;
    Vector diag(n);
    // Convergence tolerance
    const double eps = 1e-6;
    // Machine precision
    const double xtol = std::numeric_limits<double>::epsilon();
    // Printing options
    int iprint[2];
    iprint[0] = 1;  // Print in every iteration
    iprint[1] = 0;  // 0-3, larger value for more output

    // Working space and flags
    Vector work(n * (2 * m + 1) + 2 * m);
    int iflag = 0;

    // Optimization process
    double objval;
    int i;
    for (i = 0; i < maxiter; i++)
    {
        // Compute objective function value and gradient
        objval = fun(x, grad);
        // Call L-BFGS routine
        lbfgs_(&n, &m, x.data(), &objval, grad.data(),
            &diagco, diag.data(), iprint, &eps, &xtol,
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
            std::cout << "L-BFGS solver failed with code " << iflag << std::endl;
            return iflag;
        }
    }

    // Print final results
    std::cout << "================================================" << std::endl;
    std::cout << "x    = " << x.transpose() << std::endl;
    std::cout << "grad = " << grad.transpose() << std::endl;
    std::cout << "# function calls = " << i + 1 << std::endl;
    std::cout << "Final f          = " << objval << std::endl;
    std::cout << "Final ||grad||   = " << grad.norm() << std::endl;

    return 0;
}
