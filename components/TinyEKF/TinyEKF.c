#include <LinearAlgebra.h>
#include <TinyEKF.h>

/**
 * Initializes the EKF
 * @param ekf pointer to an ekf_t structure
 * @param pdiag a vector of length EKF_N containing the initial values for the
 * covariance matrix diagonal
 */
static void ekf_initialize(ekf_t * ekf, const float *pdiag)
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
  */
void ekf_predict(
        ekf_t *ekf)
{        
    // \hat{x}_k = f(\hat{x}_{k-1}, u_k)
    memcpy(ekf->x, (ekf->fx), (ekf->EKF_N)*sizeof(float));

    
    // P_k = F_{k-1} P_{k-1} F^T_{k-1} + Q_{k-1}
    float FP[(ekf->EKF_N)*(ekf->EKF_N)];
    memset(FP, 0, sizeof(FP));
    _mulmat(ekf->F, ekf->P,  FP, ekf->EKF_N, ekf->EKF_N, ekf->EKF_N);

    float Ft[(ekf->EKF_N)*(ekf->EKF_N)];
    memset(Ft, 0, sizeof(Ft));
    _transpose(ekf->F, Ft, ekf->EKF_N, ekf->EKF_N);

    float FPFt[(ekf->EKF_N)*(ekf->EKF_N)];
    memset(FPFt, 0, sizeof(FPFt));
    _mulmat(FP, Ft, FPFt, ekf->EKF_N, ekf->EKF_N, ekf->EKF_N);

    _addmat(FPFt, ekf->Q, ekf->P, ekf->EKF_N, ekf->EKF_N);
}

/// @private
static void ekf_update_step3(ekf_t * ekf, float *GH)
{
    _negate(GH, ekf->EKF_N, ekf->EKF_N);
    _addeye(GH, ekf->EKF_N);
    float GHP[(ekf->EKF_N)*(ekf->EKF_N)];
    _mulmat(GH, ekf->P, GHP, ekf->EKF_N, ekf->EKF_N, ekf->EKF_N);
    memcpy(ekf->P, GHP, (ekf->EKF_N)*(ekf->EKF_N)*sizeof(float));
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
bool ekf_update(
        ekf_t *ekf)
{        
    // G_k = P_k H^T_k (H_k P_k H^T_k + R)^{-1}
    float G[(ekf->EKF_N)*(ekf->EKF_M)];
    memset(G, 0, sizeof(G));
    float Ht[(ekf->EKF_N)*(ekf->EKF_M)];
    memset(Ht, 0, sizeof(Ht));
    _transpose(ekf->H, Ht, ekf->EKF_M, ekf->EKF_N);

    float PHt[(ekf->EKF_N)*(ekf->EKF_M)];
    memset(PHt, 0, sizeof(PHt));
    _mulmat(ekf->P, Ht, PHt, ekf->EKF_N, ekf->EKF_N, ekf->EKF_M);

    float HP[(ekf->EKF_M)*(ekf->EKF_N)];
    memset(HP,0, sizeof(HP));
    _mulmat(ekf->H, ekf->P, HP, ekf->EKF_M, ekf->EKF_N, ekf->EKF_N);

    float HpHt[(ekf->EKF_M)*(ekf->EKF_M)];
    memset(HpHt, 0, sizeof(HpHt));
    _mulmat(HP, Ht, HpHt, ekf->EKF_M, ekf->EKF_N, ekf->EKF_M);

    float HpHtR[(ekf->EKF_M)*(ekf->EKF_M)];
    memset(HpHtR, 0, sizeof(HpHtR));
    _addmat(HpHt, ekf->R, HpHtR, ekf->EKF_M, ekf->EKF_M);

    float HPHtRinv[(ekf->EKF_M)*(ekf->EKF_M)];
    memset(HPHtRinv, 0, sizeof(HPHtRinv));
    if (!invert(HpHtR, HPHtRinv, ekf->EKF_M)) {
        return false;
    }
    _mulmat(PHt, HPHtRinv, G, ekf->EKF_N, ekf->EKF_M, ekf->EKF_M);


    // \hat{x}_k = \hat{x_k} + G_k(z_k - h(\hat{x}_k))
    float z_hx[ekf->EKF_M];
    memset(z_hx, 0 , sizeof(z_hx));
    _sub(ekf->z, ekf->hx, z_hx, ekf->EKF_M);

    float Gz_hx[(ekf->EKF_M)*(ekf->EKF_N)];
    memset(Gz_hx, 0, sizeof(Gz_hx));
    _mulvec(G, z_hx, Gz_hx, ekf->EKF_N, ekf->EKF_M);
    _addvec(ekf->x, Gz_hx, ekf->x, ekf->EKF_N);


    // P_k = (I - G_k H_k) P_k
    float GH[(ekf->EKF_N)*(ekf->EKF_N)];
    memset(GH, 0, sizeof(GH));
    _mulmat(G, ekf->H, GH, ekf->EKF_N, ekf->EKF_M, ekf->EKF_N);
    ekf_update_step3(ekf, GH);

    // success
    return true;
}