#include <LinearAlgebra.h>

/// @private
void _mulmat(
        const float * a, 
        const float * b, 
        float * c, 
        const int arows, 
        const int acols, 
        const int bcols)
{
    for (int i=0; i<arows; ++i) {
        for (int j=0; j<bcols; ++j) {
            c[i*bcols+j] = 0;
            for (int k=0; k<acols; ++k) {
                c[i*bcols+j] += a[i*acols+k] * b[k*bcols+j];
            }
        }
    }
}

/// @private
void _mulvec(
        const float * a, 
        const float * x, 
        float * y, 
        const int m, 
        const int n)
{
    for (int i=0; i<m; ++i) {
        y[i] = 0;
        for (int j=0; j<n; ++j)
            y[i] += x[j] * a[i*n+j];
    }
}

/// @private
void _transpose(
        const float * a, float * at, const int m, const int n)
{
    for (int i=0; i<m; ++i)
        for (int j=0; j<n; ++j) {
            at[j*m+i] = a[i*n+j];
        }
}

/// @private
void _addmat(
        const float * a, const float * b, float * c, 
        const int m, const int n)
{
    for (int i=0; i<m; ++i) {
        for (int j=0; j<n; ++j) {
            c[i*n+j] = a[i*n+j] + b[i*n+j];
        }
    }
}

/// @private
void _negate(float * a, const int m, const int n)
{        
    for (int i=0; i<m; ++i) {
        for (int j=0; j<n; ++j) {
            a[i*n+j] = -a[i*n+j];
        }
    }
}

/// @private
void _addeye(float * a, const int n)
{
    for (int i=0; i<n; ++i) {
        a[i*n+i] += 1;
    }
}


/* Cholesky-decomposition matrix-inversion code, adapated from
http://jean-pierre.moreau.pagesperso-orange.fr/Cplus/_choles_cpp.txt */

/// @private
int _choldc1(float * a, float * p, const int n) 
{
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            float sum = a[i*n+j];
            for (int k = i - 1; k >= 0; k--) {
                sum -= a[i*n+k] * a[j*n+k];
            }
            if (i == j) {
                if (sum <= 0) {
                    return 1; /* error */
                }
                p[i] = sqrt(sum);
            }
            else {
                a[j*n+i] = sum / p[i];
            }
        }
    }

    return 0; // success:w
}

/// @private
int _choldcsl(const float * A, float * a, float * p, const int n) 
{
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            a[i*n+j] = A[i*n+j];
        }
    }
    if (_choldc1(a, p, n)) {
        return 1;
    }
    for (int i = 0; i < n; i++) {
        a[i*n+i] = 1 / p[i];
        for (int j = i + 1; j < n; j++) {
            float sum = 0;
            for (int k = i; k < j; k++) {
                sum -= a[j*n+k] * a[k*n+i];
            }
            a[j*n+i] = sum / p[j];
        }
    }

    return 0; // success
}

/// @private
int _cholsl(const float * A, float * a, float * p, const int n) 
{
    if (_choldcsl(A,a,p,n)) {
        return 1;
    }

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            a[i*n+j] = 0.0;
        }
    }
    for (int i = 0; i < n; i++) {
        a[i*n+i] *= a[i*n+i];
        for (int k = i + 1; k < n; k++) {
            a[i*n+i] += a[k*n+i] * a[k*n+i];
        }
        for (int j = i + 1; j < n; j++) {
            for (int k = j; k < n; k++) {
                a[i*n+j] += a[k*n+i] * a[k*n+j];
            }
        }
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            a[i*n+j] = a[j*n+i];
        }
    }

    return 0; // success
}

/// @private
void _addvec(
        const float * a, const float * b, float * c, const int n)
{
    for (int j=0; j<n; ++j) {
        c[j] = a[j] + b[j];
    }
}

/// @private
void _sub(
        const float * a, const float * b, float * c, const int n)
{
    for (int j=0; j<n; ++j) {
        c[j] = a[j] - b[j];
    }
}

/// @private
bool invert(const float * a, float * ainv, const int n)
{
    float tmp[n];

    return _cholsl(a, ainv, tmp, n) == 0;
}

/// @brief Finds the smallest eigenvalue of a symmetric matrix using the Jacobi method.
bool jacobi_eigensystem(float *input_matrix, float *eigenvalues, float *eigenvectors, const int n){
    if(n <= 0 || input_matrix == NULL || eigenvalues == NULL || eigenvectors == NULL){
        return false;
    }
    
    float *A = malloc(sizeof(float) * n * n);
    if(A == NULL){
        return false;
    }
    memcpy(A, input_matrix, sizeof(float) * n * n);

    // Initialize eigenvectors as the identity matrix
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            eigenvectors[i * n + j] = (i == j) ? 1.0f : 0.0f;
        }
    }

    if (n == 1) {
        eigenvalues[0] = A[0];
        free(A);
        return true;
    }

    const int MAX_ITERATIONS = 100 * n * n;
    const float EPSILON = 1e-6f; 

    for (int iter = 0; iter < MAX_ITERATIONS; ++iter) {
        // Find the largest off-diagonal element in A
        int p = 0, q = 1;
        float max_val = 0.0f;
        
        for (int i = 0; i < n - 1; ++i) {
            for (int j = i + 1; j < n; ++j) {
                float val = fabsf(A[i * n + j]);
                if (val > max_val) {
                    max_val = val;
                    p = i;
                    q = j;
                }
            }
        }

        // Check for convergence
        if (max_val < EPSILON) {
            break; 
        }

        // Compute the Givens rotation parameters (c = cos, s = sin)
        float diff = A[q * n + q] - A[p * n + p];
        float c, s, t;

        if (fabsf(A[p * n + q]) < 1e-15f) {
            c = 1.0f;
            s = 0.0f;
        } else {
            float tau = diff / (2.0f * A[p * n + q]);
            if (tau >= 0.0f) {
                t = 1.0f / (tau + sqrtf(1.0f + tau * tau));
            } else {
                t = -1.0f / (-tau + sqrtf(1.0f + tau * tau));
            }
            c = 1.0f / sqrtf(1.0f + t * t);
            s = t * c;
        }

        // Update the matrix A (A = G^T * A * G)
        float App = A[p * n + p];
        float Aqq = A[q * n + q];
        float Apq = A[p * n + q];

        A[p * n + p] = c * c * App - 2.0f * s * c * Apq + s * s * Aqq;
        A[q * n + q] = s * s * App + 2.0f * s * c * Apq + c * c * Aqq;
        A[p * n + q] = 0.0f;
        A[q * n + p] = 0.0f;

        for (int i = 0; i < n; ++i) {
            if (i != p && i != q) {
                float Aip = A[i * n + p];
                float Aiq = A[i * n + q];
                
                A[i * n + p] = c * Aip - s * Aiq;
                A[p * n + i] = A[i * n + p]; // Keep it symmetric
                
                A[i * n + q] = s * Aip + c * Aiq;
                A[q * n + i] = A[i * n + q]; // Keep it symmetric
            }
        }

        // Update the eigenvectors matrix
        // By applying the rotation to the rows 'p' and 'q', we naturally 
        // accumulate the eigenvectors such that row 'i' corresponds to eigenvalue 'i'.
        for (int j = 0; j < n; ++j) {
            float E_pj = eigenvectors[p * n + j];
            float E_qj = eigenvectors[q * n + j];
            
            eigenvectors[p * n + j] = c * E_pj - s * E_qj;
            eigenvectors[q * n + j] = s * E_pj + c * E_qj;
        }
    }

    //Extract the eigenvalues from the diagonal of the converged matrix A
    for (int i = 0; i < n; ++i) {
        eigenvalues[i] = A[i * n + i];
    }
    free(A);
    return true;
}

bool angle_between(const float * a, const float * b, float * c, const int n){
    if(n <= 0 || a == NULL || b == NULL || c == NULL){
        return false;
    }

    float dot_product = 0;
    for(int i = 0; i < n; i++){
        dot_product += a[i] * b[i];
    }

    float a_mag = 0, b_mag = 0;
    for(int i = 0; i < n; i++){
        a_mag += a[i] * a[i];
        b_mag += b[i] * b[i];
    }
    a_mag = sqrtf(a_mag);
    b_mag = sqrtf(b_mag);

    *c = acosf(dot_product / (a_mag * b_mag)) * (180.0f / M_PI); //return in degree
    return true;
}

int print_matrix(const float *a, const int m, const int n){
    int result = 0;
    for(int i = 0; i < m; i++){
        result = debug_printf("[");
        for(int j = 0; j < n; j++){
            result = debug_printf("%10.6f ", a[i * n + j]);
        }
        result = debug_printf("]\n");
    }
    return result;
}

int print_vector(const float * a, const int n){
    for(int i = 0; i < n; i++){
        debug_printf("%10.6f ", a[i]);
    }
    return debug_printf("\n");
}