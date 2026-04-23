#include <TinyEKF.h>
// Linear alegbra ////////////////////////////////////////////////////////////

/// @private
static void _mulmat(
        const _float_t * a, 
        const _float_t * b, 
        _float_t * c, 
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
static void _mulvec(
        const _float_t * a, 
        const _float_t * x, 
        _float_t * y, 
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
static void _transpose(
        const _float_t * a, _float_t * at, const int m, const int n)
{
    for (int i=0; i<m; ++i)
        for (int j=0; j<n; ++j) {
            at[j*m+i] = a[i*n+j];
        }
}

/// @private
static void _addmat(
        const _float_t * a, const _float_t * b, _float_t * c, 
        const int m, const int n)
{
    for (int i=0; i<m; ++i) {
        for (int j=0; j<n; ++j) {
            c[i*n+j] = a[i*n+j] + b[i*n+j];
        }
    }
}

/// @private
static void _negate(_float_t * a, const int m, const int n)
{        
    for (int i=0; i<m; ++i) {
        for (int j=0; j<n; ++j) {
            a[i*n+j] = -a[i*n+j];
        }
    }
}

/// @private
static void _addeye(_float_t * a, const int n)
{
    for (int i=0; i<n; ++i) {
        a[i*n+i] += 1;
    }
}


/* Cholesky-decomposition matrix-inversion code, adapated from
http://jean-pierre.moreau.pagesperso-orange.fr/Cplus/_choles_cpp.txt */

/// @private
static int _choldc1(_float_t * a, _float_t * p, const int n) 
{
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            _float_t sum = a[i*n+j];
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
static int _choldcsl(const _float_t * A, _float_t * a, _float_t * p, const int n) 
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
            _float_t sum = 0;
            for (int k = i; k < j; k++) {
                sum -= a[j*n+k] * a[k*n+i];
            }
            a[j*n+i] = sum / p[j];
        }
    }

    return 0; // success
}

/// @private
static int _cholsl(const _float_t * A, _float_t * a, _float_t * p, const int n) 
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
static void _addvec(
        const _float_t * a, const _float_t * b, _float_t * c, const int n)
{
    for (int j=0; j<n; ++j) {
        c[j] = a[j] + b[j];
    }
}

/// @private
static void _sub(
        const _float_t * a, const _float_t * b, _float_t * c, const int n)
{
    for (int j=0; j<n; ++j) {
        c[j] = a[j] - b[j];
    }
}

/// @private
static bool invert(const _float_t * a, _float_t * ainv, const int n)
{
    _float_t tmp[n];

    return _cholsl(a, ainv, tmp, n) == 0;
}

/**
 * Initializes the EKF
 * @param ekf pointer to an ekf_t structure
 * @param pdiag a vector of length EKF_N containing the initial values for the
 * covariance matrix diagonal
 */
static void ekf_initialize(ekf_t * ekf, const _float_t *pdiag)
{
    for (int i=0; i<ekf->EKF_N; ++i) {

        for (int j=0; j<ekf->EKF_N; ++j) {

            ekf->P[i*(ekf->EKF_N)+j] = i==j ? pdiag[i] : 0;
        }

        ekf->x[i] = 0;
    }
}

/**
  * Runs the EKF prediction step
  * @param ekf pointer to an ekf_t structure
  * @param fx predicted values
  * @param F Jacobian of state-transition function
  * @param Q process noise matrix
  * 
  */static void ekf_predict(
        ekf_t * ekf)
{        
    // \hat{x}_k = f(\hat{x}_{k-1}, u_k)
    memcpy(ekf->x, (ekf->fx), (ekf->EKF_N)*sizeof(_float_t));

    
    // P_k = F_{k-1} P_{k-1} F^T_{k-1} + Q_{k-1}
    _float_t FP[(ekf->EKF_N)*(ekf->EKF_N)];
    memset(FP, 0, sizeof(FP));
    _mulmat(ekf->F, ekf->P,  FP, ekf->EKF_N, ekf->EKF_N, ekf->EKF_N);

    _float_t Ft[(ekf->EKF_N)*(ekf->EKF_N)];
    memset(Ft, 0, sizeof(Ft));
    _transpose(ekf->F, Ft, ekf->EKF_N, ekf->EKF_N);

    _float_t FPFt[(ekf->EKF_N)*(ekf->EKF_N)];
    memset(FPFt, 0, sizeof(FPFt));
    _mulmat(FP, Ft, FPFt, ekf->EKF_N, ekf->EKF_N, ekf->EKF_N);

    _addmat(FPFt, ekf->Q, ekf->P, ekf->EKF_N, ekf->EKF_N);
}

/// @private
static void ekf_update_step3(ekf_t * ekf, _float_t *GH)
{
    _negate(GH, ekf->EKF_N, ekf->EKF_N);
    _addeye(GH, ekf->EKF_N);
    _float_t GHP[(ekf->EKF_N)*(ekf->EKF_N)];
    _mulmat(GH, ekf->P, GHP, ekf->EKF_N, ekf->EKF_N, ekf->EKF_N);
    memcpy(ekf->P, GHP, (ekf->EKF_N)*(ekf->EKF_N)*sizeof(_float_t));
}

/**
  * Runs the EKF update step
  * @param ekf pointer to an ekf_t structure
  * @param z observations
  * @param hx predicted values
  * @param H sensor-function Jacobian matrix
  * @param R measurement-noise matrix
  * 
  */
static bool ekf_update(
        ekf_t * ekf)
{        
    // G_k = P_k H^T_k (H_k P_k H^T_k + R)^{-1}
    _float_t G[(ekf->EKF_N)*(ekf->EKF_M)];
    memset(G, 0, sizeof(G));
    _float_t Ht[(ekf->EKF_N)*(ekf->EKF_M)];
    memset(Ht, 0, sizeof(Ht));
    _transpose(ekf->H, Ht, ekf->EKF_M, ekf->EKF_N);

    _float_t PHt[(ekf->EKF_N)*(ekf->EKF_M)];
    memset(PHt, 0, sizeof(PHt));
    _mulmat(ekf->P, Ht, PHt, ekf->EKF_N, ekf->EKF_N, ekf->EKF_M);

    _float_t HP[(ekf->EKF_M)*(ekf->EKF_N)];
    memset(HP,0, sizeof(HP));
    _mulmat(ekf->H, ekf->P, HP, ekf->EKF_M, ekf->EKF_N, ekf->EKF_N);

    _float_t HpHt[(ekf->EKF_M)*(ekf->EKF_M)];
    memset(HpHt, 0, sizeof(HpHt));
    _mulmat(HP, Ht, HpHt, ekf->EKF_M, ekf->EKF_N, ekf->EKF_M);

    _float_t HpHtR[(ekf->EKF_M)*(ekf->EKF_M)];
    memset(HpHtR, 0, sizeof(HpHtR));
    _addmat(HpHt, ekf->R, HpHtR, ekf->EKF_M, ekf->EKF_M);

    _float_t HPHtRinv[(ekf->EKF_M)*(ekf->EKF_M)];
    memset(HPHtRinv, 0, sizeof(HPHtRinv));
    if (!invert(HpHtR, HPHtRinv, ekf->EKF_M)) {
        return false;
    }
    _mulmat(PHt, HPHtRinv, G, ekf->EKF_N, ekf->EKF_M, ekf->EKF_M);


    // \hat{x}_k = \hat{x_k} + G_k(z_k - h(\hat{x}_k))
    _float_t z_hx[ekf->EKF_M];
    memset(z_hx, 0 , sizeof(z_hx));
    _sub(ekf->z, ekf->hx, z_hx, ekf->EKF_M);

    _float_t Gz_hx[(ekf->EKF_M)*(ekf->EKF_N)];
    memset(Gz_hx, 0, sizeof(Gz_hx));
    _mulvec(G, z_hx, Gz_hx, ekf->EKF_N, ekf->EKF_M);
    _addvec(ekf->x, Gz_hx, ekf->x, ekf->EKF_N);


    // P_k = (I - G_k H_k) P_k
    _float_t GH[(ekf->EKF_N)*(ekf->EKF_N)];
    memset(GH, 0, sizeof(GH));
    _mulmat(G, ekf->H, GH, ekf->EKF_N, ekf->EKF_M, ekf->EKF_N);
    ekf_update_step3(ekf, GH);

    // success
    return true;
}