#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
using std::min;
using std::max;

// CUDA thread/block dimension constants (adjust for your workload)
#ifndef BLOCK_DIM_X
#define BLOCK_DIM_X 256
#endif
#ifndef GRID_DIM_X
#define GRID_DIM_X 1
#endif

// --- from lanczos.cu ---
#include <cassert>
#include <iostream>
#include <unistd.h>
#include <cmath>
#include <utility>


#define THREADS_PER_BLOCK 256

using std::cout;
using std::endl;
using std::vector;

/**
 * @brief   Cuda kernel function for vector dot product.
 *
 * @param   N   The vector size.
 * @param   x   The first input vector.
 * @param   y   The second input vector.
 * @param   z   The temp sum per block.
 */
template <typename T>

/**
 * @brief   Cuda kernel function for vector multiply in place.
 *
 * @param   N   The vector size.
 * @param   x   The input vector.
 * @param   k   The value to multiply.
 */
template <typename T>

/**
 * @brief   Cuda kernel function for vector saxpy in place(y += a * x).
 *
 * @param   N   The vector size.
 * @param   y   The output vector.
 * @param   x   The input vector.
 * @param   a   The value to multiply.
 */
template <typename T>

/**
 * @brief   Cuda kernel function for warp sparse matrix multiplication.
 *
 * @param   group_size  The number of threads used to calculate one row.
 * @param   rows        The row number of the matrix.
 * @param   begin_row   The row to begin from in this kernel launch.
 * @param   row_ptr     Row pointers in the CSR matrix.
 * @param   col_ind     Column indexes in the CSR matrix.
 * @param   values      Data values in the CSR matrix.
 * @param   x           The input vector x to multiply.
 * @param   y           The output vector y.
 */
template <typename T>

template <typename T>

/**
 * @brief   Caller function for naive Lanczos algorithm in CUDA.
 *
 * @param   m       The matrix to do operations on.
 * @param   v       The initial vector with norm 1.
 * @param   steps   The iteration times for lanczos algorithm.
 *
 * @return  The tridiagonal matrix result of lanczos algorithm.
 */
template <typename T>
symm_tridiag_matrix<T> gpu_lanczos(const csr_matrix<T> &m,
    const vector<T> &v, const int steps) {
    symm_tridiag_matrix<T> result(steps + 1);

    int rows = m.row_size();
    int cols = m.col_size();
    int nonzeros = m.nonzeros();
    const int blocks = (rows + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
    assert(rows == cols);
    assert(cols == v.size());

    // Malloc device space
    int *row_ptr, *col_ind;
    T *values, *x, *x_prev, *y, *scratch;

    // Transfer data from host to device

        cudaMemcpyHostToDevice);

        cudaMemcpyHostToDevice);

        cudaMemcpyHostToDevice);

    const int row_nonzeros = nonzeros / rows;
    int group_size = row_nonzeros > 16 ? 32 : 16;
    group_size = row_nonzeros > 8 ? group_size : 8;
    group_size = row_nonzeros > 4 ? group_size : 4;
    group_size = row_nonzeros > 2 ? group_size : 2;
    const int groups_per_block = THREADS_PER_BLOCK / group_size;
    const int multiply_blocks = (rows + groups_per_block - 1) / groups_per_block;
    // Run kernel and the values of alpha and beta are saved in the 'result' array
    double start_time = cycle_timer::current_seconds();

    double end_time = cycle_timer::current_seconds();
    cout << "GPU Lanczos iterations: " << steps << endl;
    cout << "GPU Lanczos time: " << end_time - start_time << " sec" << endl;

    // Release device space

    result.resize(steps);
    return result;
}

/**
 * @brief   Lanczos algorithm for eigendecomposition in CUDA.
 * 
 * @param   matrix  CSR matrix to decompose
 * @param   k       number of largest eigenvalues to compute
 * @param   steps   maximum steps for the iteration
 * @tparam  T       matrix element data type
 * @return  list of eigenvalues
 */
template <typename T>
vector<T> gpu_lanczos_eigen(const csr_matrix<T> &matrix, int k, int steps) {
    int cols = matrix.col_size();
    assert(cols > 0);
    vector<T> v(cols, 0);
    v[0] = 1;
    symm_tridiag_matrix<T> tridiag = gpu_lanczos(matrix, v, steps);
    return lanczos_no_spurious(tridiag, k);
}

template vector<float> gpu_lanczos_eigen(const csr_matrix<float> &matrix, int k, int steps);
template vector<double> gpu_lanczos_eigen(const csr_matrix<double> &matrix, int k, int steps);


// --- from main.cu ---
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <getopt.h>


using std::cout;
using std::cerr;
using std::endl;

static string graph_file;
static int node_count = 0;
static int eigen_count = 0;
static bool double_precision = false;



template <typename T>



// --- from cycle_timer.h ---
#ifndef _SYRAH_CYCLE_TIMER_H_
#define _SYRAH_CYCLE_TIMER_H_

#if defined(__APPLE__)
#if defined(__x86_64__)
    #include <sys/sysctl.h>
#else
    #include <mach/mach.h>
    #include <mach/mach_time.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#elif _WIN32
    #include <windows.h>
    #include <time.h>
#else
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <sys/time.h>
#endif

/**
 * @brief CPU cycle timer.
 * 
 * This uses the cycle counter of the processor. Different
 * processors in the system will have different values for this. If
 * your process moves across processors, then the delta time you
 * measure will likely be incorrect. This is mostly for fine
 * grained measurements where the process is likely to be on the
 * same processor. For more global things you should use the
 * Time interface.
 * 
 * Also note that if your processors' speeds change (i.e. processors
 * scaling) or if you are in a heterogenous environment, you will
 * likely get spurious results.
 */
class cycle_timer {
public:
    typedef unsigned long long sys_clock;

    /**
     * @brief Gets current CPU time.
     * @details Time zero is at some arbitrary point in the past.
     * @return the current CPU time, in terms of clock ticks
     */
    static sys_clock current_ticks() {
#if defined(__APPLE__) && !defined(__x86_64__)
        return mach_absolute_time();
#elif defined(_WIN32)
        LARGE_INTEGER qwTime;
        QueryPerformanceCounter(&qwTime);
        return qwTime.QuadPart;
#elif defined(__x86_64__)
        unsigned int a, d;
        asm volatile("rdtsc" : "=a" (a), "=d" (d));
        return static_cast<unsigned long long>(a) |
            (static_cast<unsigned long long>(d) << 32);
#elif defined(__ARM_NEON__) && 0 // mrc requires superuser
        unsigned int val;
        asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(val));
        return val;
#else
        timespec spec;
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &spec);
        return cycle_timer::sys_clock(static_cast<float>(spec.tv_sec) * 1e9 + static_cast<float>(spec.tv_nsec));
#endif
    }

    /**
     * @brief Gets current CPU second.
     * @details This is slower than current_ticks(). Time zero is at some arbitrary point in the past.
     * @return the current CPU time, in terms of seconds
     */
    static double current_seconds() {
        return current_ticks() * seconds_per_tick();
    }

    /**
     * @brief Gets the conversion from seconds to ticks.
     * @details
     * @return ticks per second
     */
    static double ticks_per_second() {
        return 1.0 / seconds_per_tick();
    }

    /**
     * @brief Gets time tick units.
     * @details
     * @return tick units
     */
    static const char *tick_units() {
#if defined(__APPLE__) && !defined(__x86_64__)
        return "ns";
#elif defined(__WIN32__) || defined(__x86_64__)
        return "cycles";
#else
        return "ns"; // clock_gettime
#endif
    }

    /**
     * @brief Gets the conversion from ticks to seconds.
     * @details
     * @return seconds per tick
     */
    static double seconds_per_tick() {
        static bool initialized = false;
        static double seconds_per_tick_val;

        if (initialized)
            return seconds_per_tick_val;

#if defined(__APPLE__)
#ifdef __x86_64__
        int args[] = { CTL_HW, HW_CPU_FREQ };
        unsigned int Hz;
        size_t len = sizeof(Hz);
        if (sysctl(args, 2, &Hz, &len, NULL, 0) != 0) {
            fprintf(stderr, "failed to initialize seconds_per_tick_val\n");
            exit(-1);
        }
        seconds_per_tick_val = 1.0 / (double) Hz;
#else
        mach_timebase_info_data_t time_info;
        mach_timebase_info(&time_info);

        // scales to nanoseconds without 1e-9f
        seconds_per_tick_val = (1e-9 * static_cast<double>(time_info.numer)) /
            static_cast<double>(time_info.denom);
#endif
#elif defined(_WIN32)
        LARGE_INTEGER qwTicksPerSec;
        QueryPerformanceFrequency(&qwTicksPerSec);
        seconds_per_tick_val = 1.0 / static_cast<double>(qwTicksPerSec.QuadPart);
#else
        FILE *fp = fopen("/proc/cpuinfo", "r");
        char input[1024];
        if (!fp) {
            fprintf(stderr, "cycle_timer::seconds_per_tick failed: cannot find /proc/cpuinfo\n");
            exit(-1);
        }
        // in case we do not find it, e.g. on the N900
        seconds_per_tick_val = 1e-9;
        while (!feof(fp) && fgets(input, 1024, fp)) {
            // NOTE(boulos): because reading cpuinfo depends on dynamic
            // frequency scaling it is better to read the @ sign first
            float GHz, MHz;
            if (strstr(input, "model name")) {
                char *at_sign = strstr(input, "@");
                if (at_sign) {
                    char *after_at = at_sign + 1;
                    char *GHz_str = strstr(after_at, "GHz");
                    char *MHz_str = strstr(after_at, "MHz");
                    if (GHz_str) {
                        *GHz_str = '\0';
                        if (1 == sscanf(after_at, "%f", &GHz)) {
                            seconds_per_tick_val = 1e-9f / GHz;
                            break;
                        }
                    } else if (MHz_str) {
                        *MHz_str = '\0';
                        if (1 == sscanf(after_at, "%f", &MHz)) {
                            seconds_per_tick_val = 1e-6f / MHz;
                            break;
                        }
                    }
                }
            } else if (1 == sscanf(input, "cpu MHz : %f", &MHz)) {
                seconds_per_tick_val = 1e-6f / MHz;
                break;
            }
        }
        fclose(fp);
#endif
        initialized = true;
        return seconds_per_tick_val;
    }

    /**
     * @brief Gets the conversion from ticks to milliseconds
     * @details
     * @return milliseconds per tick
     */
    static double milliseconds_per_tick() {
        return seconds_per_tick() * 1000.0;
    }

private:
    cycle_timer();
};

#endif


// --- from eigen.h ---
#ifndef _EIGEN_H_
#define _EIGEN_H_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>


using std::cout;
using std::endl;
using std::vector;

/**
 * @brief   Lanczos algorithm for eigendecomposition.
 * 
 * @param   matrix  CSR matrix to decompose
 * @param   k       number of largest eigenvalues to compute
 * @param   steps   maximum steps for the iteration
 * @tparam  T       matrix element data type
 * @return  list of eigenvalues
 */
template <typename T>
vector<T> lanczos_eigen(const csr_matrix<T> &matrix, int k, int steps) {
    int n = matrix.row_size();
    assert(n > 0 && n == matrix.col_size());
    assert(steps > 2 * k);

    symm_tridiag_matrix<T> tridiag(steps);
    vector<vector<T> > basis;

    vector<T> r(n, 0);
    r[0] = 1; // initialize a "random" vector
    T beta = l2_norm(r);
    double start_time = cycle_timer::current_seconds();
    for (int t = 0; t < steps; ++t) {
        if (t > 0) {
            tridiag.beta(t - 1) = beta;
        }
        multiply_inplace(r, 1 / beta);
        basis.push_back(r);
        r = multiply(matrix, r);
        T alpha = dot_product(basis[t], r);
        saxpy_inplace(r, -alpha, basis[t]);
        if (t > 0) {
            saxpy_inplace(r, -beta, basis[t - 1]);
        }
        tridiag.alpha(t) = alpha;
        beta = l2_norm(r);
    }
    double end_time = cycle_timer::current_seconds();
    cout << "CPU Lanczos iterations: " << steps << endl;
    cout << "CPU Lanczos time: " << end_time - start_time << " sec" << endl;
    return lanczos_no_spurious(tridiag, k);
}

template <typename T>
vector<T> lanczos_no_spurious(symm_tridiag_matrix<T> &tridiag, int k, const T epsilon = 1e-3) {
    assert(tridiag.size() > 0);
    double start_time = cycle_timer::current_seconds();

    vector<T> eigen = tqlrat_eigen(tridiag);
    tridiag.remove_forward(0);
    vector<T> test_eigen = tqlrat_eigen(tridiag);
    vector<T> result;

    int i = 0;
    int j = 0;
    while (j <= (int)eigen.size()) { // scan through one position beyond the end of the list
        if (j < (int)eigen.size() && std::abs(eigen[j] - eigen[i]) < epsilon) {
            j++;
            continue;
        }
        // simple eigenvalues not in test set are preserved
        // multiple eigenvalues are only preserved once
        if (j - i > 1 || approximate_find(test_eigen, eigen[i], epsilon) == test_eigen.end()) {
            result.push_back(eigen[i]);
        }
        i = j++;
    }
    std::sort(result.rbegin(), result.rend());
    result.resize(std::min((int)result.size(), k));

    double end_time = cycle_timer::current_seconds();
    cout << "spurious removal time: " << end_time - start_time << " sec" << endl;
    return result;
}

/**
 * @brief   Calculating eigenvalues for symmetric tridiagonal matrices.
 * @details Reinsch, C. H. (1973). Algorithm 464: Eigenvalues of a Real, Symmetric, Tridiagonal Matrix.
 *          Communications of the ACM, 16(11), 689.
 * 
 * @param   matrix  symmetric tridiagonal matrix to decompose
 * @param   epsilon precision threshold
 * @tparam  T       matrix element data type
 * @return  list of eigenvalues
 */
template <typename T>
vector<T> tqlrat_eigen(const symm_tridiag_matrix<T> &matrix, const T epsilon = 1e-8) {
    double start_time = cycle_timer::current_seconds();

    int n = matrix.size();
    vector<T> d(matrix.alpha_data(), matrix.alpha_data() + n);
    vector<T> e2(n, 0);
    for (int i = 0; i < n - 1; ++i) {
        e2[i] = matrix.beta(i) * matrix.beta(i);
    }
    T b(0), b2(0), f(0);
    for (int k = 0; k < n; ++k) {
        T h = epsilon * epsilon * (d[k] * d[k] + e2[k]);
        if (b2 < h) {
            b = sqrt(h);
            b2 = h;
        }
        int m = k;
        while (m < n && e2[m] > b2) {
            ++m;
        }
        if (m == n) {
            --m;
        }
        if (m > k) {
            do {
                T g = d[k];
                T p2 = sqrt(e2[k]);
                h = (d[k + 1] - g) / (2.0 * p2);
                T r2 = sqrt(h * h + 1.0);
                d[k] = h = p2 / (h < 0.0 ? h - r2 : h + r2);
                h = g - h;
                f = f + h;
                for (int i = k + 1; i < n; ++i) {
                    d[i] -= h;
                }
                h = g = std::abs(d[m] - 0.0) < epsilon ? b : d[m];
                T s2 = 0.0;
                for (int i = m - 1; i >= k; --i) {
                    p2 = g * h;
                    r2 = p2 + e2[i];
                    e2[i + 1] = s2 * r2;
                    s2 = e2[i] / r2;
                    d[i + 1] = h + s2 * (h + d[i]);
                    g = d[i] - e2[i] / g;
                    if (std::abs(g - 0.0) < epsilon) {
                        g = b;
                    }
                    h = g * p2 / r2;
                }
                e2[k] = s2 * g * h;
                d[k] = h;
            } while (e2[k] > b2);
        }
        h = d[k] + f;
        int j;
        for (j = k; j > 0; --j) {
            if (h >= d[j - 1]) {
                break;
            }
            d[j] = d[j - 1];
        }
        d[j] = h;
    }

    double end_time = cycle_timer::current_seconds();
    cout << "TQLRAT time: " << end_time - start_time << " sec" << endl;
    return d;
}

/**
 * @brief   QR eigendecomposition for symmetric tridiagonal matrices.
 * 
 * @param   matrix  symmetric tridiagonal matrix to decompose
 * @param   epsilon precision threshold
 * @tparam  T       matrix element data type
 * @return  list of eigenvalues
 */
template <typename T>
vector<T> qr_eigen(const symm_tridiag_matrix<T> &matrix, const T epsilon = 1e-8) {
    double start_time = cycle_timer::current_seconds();
    symm_tridiag_matrix<T> tridiag = matrix;
    int n = tridiag.size();

    tridiag.resize(n + 1);
    tridiag.alpha(n) = 0;
    tridiag.beta(n - 1) = 0;
    for (int i = 0; i < n - 1; ++i) {
        tridiag.beta(i) = tridiag.beta(i) * tridiag.beta(i);
    }
    bool converged = false;
    while (!converged) {
        T diff(0);
        T u(0);
        T ss2(0), s2(0); // previous and current value of s^2
        for (int i = 0; i < n; ++i) {
            T gamma = tridiag.alpha(i) - u;
            T p2 = T(std::abs(1 - s2)) < epsilon ? (1 - ss2) * tridiag.beta(i - 1) : gamma * gamma / (1 - s2);
            if (i > 0) {
                tridiag.beta(i - 1) = s2 * (p2 + tridiag.beta(i));
            }
            ss2 = s2;
            s2 = tridiag.beta(i) / (p2 + tridiag.beta(i));
            u = s2 * (gamma + tridiag.alpha(i + 1));
            // update alpha
            T old = tridiag.alpha(i);
            tridiag.alpha(i) = gamma + u;
            diff = std::max(diff, T(std::abs(old - tridiag.alpha(i))));
        }
        if (diff < epsilon) {
            converged = true;
        }
    }
    double end_time = cycle_timer::current_seconds();
    cout << "QR decomposition time: " << end_time - start_time << " sec" << endl;
    return vector<T>(tridiag.alpha_data(), tridiag.alpha_data() + n);
}

#endif


// --- from graph_io.h ---
#ifndef _GRAPH_IO_H
#define _GRAPH_IO_H

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


using std::ifstream;
using std::getline;
using std::string;
using std::stringstream;
using std::cerr;
using std::exit;

template <typename T>
coo_matrix<T> adjacency_matrix_from_graph(int node_count, const string &path) {
    coo_matrix<T> matrix(node_count);
    ifstream input_stream(path);
    if (!input_stream.is_open()) {
        cerr << "Error: failed to open the file: " << path << '\n';
        exit(EXIT_FAILURE);
    }
    string line;
    T w(1);
    while (getline(input_stream, line)) {
        stringstream input(line);
        int i, j;
        input >> i >> j;
        if (i >= node_count || j >= node_count) {
            continue;
        }
        matrix.add_entry(i, j, w);
    }
    return matrix;
}

template <typename T>
symm_tridiag_matrix<T> symm_tridiag_matrix_from_file(const string &path) {
    int n;
    ifstream input(path);
    if (!input.is_open()) {
        cerr << "Error: failed to open the file: " << path << '\n';
        exit(EXIT_FAILURE);
    }
    input >> n;
    symm_tridiag_matrix<T> matrix(n);
    for (int i = 0; i < n; ++i) {
        input >> matrix.alpha(i);
    }
    for (int i = 0; i < n - 1; ++i) {
        input >> matrix.beta(i);
    }
    return matrix;
}

#endif


// --- from lanczos.h ---
#ifndef _CUDA_ALGEBRA_H_
#define _CUDA_ALGEBRA_H_

#include <vector>


using std::vector;

template <typename T>
vector<T> gpu_lanczos_eigen(const csr_matrix<T> &matrix,
    int k, int steps);

#endif


// --- from linear_algebra.h ---
#ifndef _LINEAR_ALGEBRA_H_
#define _LINEAR_ALGEBRA_H_

#include <cassert>
#include <cmath>
#include <vector>


using std::sqrt;
using std::vector;

template <typename T>
T dot_product(const vector<T> &v1, const vector<T> &v2) {
    int n = v1.size();
    assert(n == v2.size());
    T s = 0;
    for (int i = 0; i < n; ++i) {
        s += v1[i] * v2[i];
    }
    return s;
}

template <typename T>
vector<T> multiply(const csr_matrix<T> &m, const vector<T> &v) {
    int rows = m.row_size();
    int cols = m.col_size();
    assert(cols == v.size());
    vector<T> product(v.size(), 0);
    for (int r = 0; r < rows; ++r) {
        int start = m.row_ptr(r);
        int end = m.row_ptr(r + 1);
        for (int i = start; i < end; ++i) {
            int c = m.col_ind(i);
            product[r] += m.values(i) * v[c];
        }
    }
    return product;
}

template <typename T>
void multiply_inplace(vector<T> &v, const T &k) {
    for (auto p = v.begin(); p != v.end(); ++p) {
        *p *= k;
    }
}

template <typename T>
void add_inplace(vector<T> &v, const T &k) {
    for (auto p = v.begin(); p != v.end(); ++p) {
        *p += k;
    }
}

template <typename T>
void saxpy_inplace(vector<T> &y, const T &a, const vector<T> &x) {
    int n = y.size();
    assert(n == x.size());
    for (int i = 0; i < n; ++i) {
        y[i] += a * x[i];
    }
}

template <typename T>
T l2_norm(const vector<T> &v) {
    return T(sqrt(dot_product(v, v)));
}

template <typename InputIterator, typename T>
InputIterator approximate_find(InputIterator first, InputIterator last, const T &val, const T &eps) {
    while (first != last) {
        if (T(std::abs(*first - val)) < eps) {
            return first;
        }
        ++first;
    }
    return last;
}

template <typename T>
typename vector<T>::const_iterator approximate_find(const vector<T> &input, const T &val, const T &eps) {
    return approximate_find(input.begin(), input.end(), val, eps);
}

#endif


// --- from matrix.h ---
#ifndef _MATRIX_H_
#define _MATRIX_H_

#include <cassert>
#include <vector>

using std::vector;

/**
 * @brief A coordinate list sparse matrix.
 * @details
 * 
 * @tparam T Element data type.
 */
template <typename T>
class coo_matrix {
public:
    coo_matrix(int m, int n)
        : row_size_(m), col_size_(n) {}
    coo_matrix(int n) : coo_matrix(n, n) {}

    int row_size() const { return row_size_; }
    int col_size() const { return col_size_; }
    int nonzeros() const { return values_.size(); }

    const T &values(int i) const { return values_[i]; }
    const int &row(int i) const { return row_[i]; }
    const int &col(int i) const { return col_[i]; }

    T &values(int i) { return values_[i]; }
    int &row(int i) { return row_[i]; }
    int &col(int i) { return col_[i]; }

    const T *values_data(int i) const { return values_.data(); }
    const int *row_data(int i) const { return row_.data(); }
    const int *col_data(int i) const { return col_.data(); }

    T *values_data(int i) { return values_.data(); }
    int *row_data(int i) { return row_.data(); }
    int *col_data(int i) { return col_.data(); }

    void add_entry(int i, int j, T &v);

private:
    int row_size_, col_size_;
    vector<T> values_;
    vector<int> row_;
    vector<int> col_;
};

template <typename T>
void coo_matrix<T>::add_entry(int i, int j, T &v) {
    row_.push_back(i);
    col_.push_back(j);
    values_.push_back(v);
}

/**
 * @brief A compressed sparse row (CSR) matrix.
 * @details
 * 
 * @tparam T Element data type.
 */
template <typename T>
class csr_matrix {
public:
    csr_matrix(int m, int n)
        : row_size_(m), col_size_(n), row_ptr_(m + 1, 0) {}
    csr_matrix(int n) : csr_matrix(n, n) {}
    csr_matrix(const coo_matrix<T> &matrix);

    int row_size() const { return row_size_; }
    int col_size() const { return col_size_; }
    int nonzeros() const { return values_.size(); }

    const T &values(int i) const { return values_[i]; }
    const int &col_ind(int i) const { return col_ind_[i]; }
    const int &row_ptr(int i) const { return row_ptr_[i]; }

    const T *values_data() const { return values_.data(); }
    const int *col_ind_data() const { return col_ind_.data(); }
    const int *row_ptr_data() const { return row_ptr_.data(); }

private:
    int row_size_, col_size_;
    vector<T> values_;
    vector<int> col_ind_;
    vector<int> row_ptr_;
};

template <typename T>
csr_matrix<T>::csr_matrix(const coo_matrix<T> &matrix)
    : csr_matrix(matrix.row_size(), matrix.col_size()) {
    for (int i = 0; i < matrix.nonzeros(); ++i) {
        row_ptr_[matrix.row(i)]++;
    }
    for (int i = 1; i < row_size_ + 1; ++i) {
        row_ptr_[i] += row_ptr_[i - 1];
    }
    col_ind_.resize(matrix.nonzeros());
    values_.resize(matrix.nonzeros());
    for (int i = 0; i < matrix.nonzeros(); ++i) {
        int pos = --row_ptr_[matrix.row(i)];
        col_ind_[pos] = matrix.col(i);
        values_[pos] = matrix.values(i);
    }
}

/**
 * @brief A symmetric tridiagonal matrix.
 * @details
 * 
 * @tparam T Element data type.
 */
template <typename T>
class symm_tridiag_matrix {
public:
    symm_tridiag_matrix(int n)
        : alpha_(n), beta_(n - 1) {}

    int size() const { return alpha_.size(); }
    void resize(int n);
    void remove_forward(int i);
    void remove_backward(int i);

    const T &alpha(int i) const { return alpha_[i]; }
    const T &beta(int i) const { return beta_[i]; }

    T &alpha(int i) { return alpha_[i]; }
    T &beta(int i) { return beta_[i]; }

    const T *alpha_data() const { return alpha_.data(); }
    const T *beta_data() const { return beta_.data(); }

    T *alpha_data() { return alpha_.data(); }
    T *beta_data() { return beta_.data(); }

private:
    vector<T> alpha_; /**< main diagonal entries */
    vector<T> beta_; /**< diagonal entries below or above the main diagonal */
};

template <typename T>
void symm_tridiag_matrix<T>::resize(int n) {
    alpha_.resize(n);
    beta_.resize(n - 1);
}

template <typename T>
void symm_tridiag_matrix<T>::remove_forward(int i) {
    assert(i < size() - 1);
    alpha_.erase(alpha_.begin() + i);
    beta_.erase(beta_.begin() + i);
}

template <typename T>
void symm_tridiag_matrix<T>::remove_backward(int i) {
    assert(i > 0);
    alpha_.erase(alpha_.begin() + i);
    beta_.erase(beta_.begin() - 1 + i);
}

#endif


// --- from utils.h ---
#ifndef _UTILS_H
#define _UTILS_H

#include <algorithm>
#include <cmath>
#include <iterator>
#include <iostream>
#include <limits>
#include <random>

template <typename T>
void print_vector(const vector<T> &v) {
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>(std::cout, " "));
    std::cout << std::endl;
}

template <typename T>
vector<T> random_vector(int n) {
	std::default_random_engine generator;
	std::uniform_real_distribution<T> distribution(0.0, 1.0);
	vector<T> result(n);
	for (int i = 0; i < n; ++i) {
		result[i] = distribution(generator);
	}
	return result;
}

template <typename T>
T diff_vector(const vector<T> &a, const vector<T> &b) {
	int n = a.size();
	if (n != b.size()) {
		return std::numeric_limits<T>::max();
	}
	T diff(0);
	for (int i = 0; i < n; ++i) {
		diff = std::max(diff, T(std::abs(a[i] - b[i])));
	}
	return diff;
}

#endif
