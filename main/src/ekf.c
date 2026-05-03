#include <ekf.h>
#include <LinearAlgebra.h>

#define NUM_STATES 13 //roll(gyro accum), pitch(gyro accum), yaw(gyro accum), vel_N(accel accum), vel_E(accel accum), p_N(vel_N accum), p_E(vel_E accum), bias_a_x, bias_a_y, bias_a_z, bias_g_x, bias_g_y, bias_g_z
#define NUM_OBSERVATIONS 7 //obs roll (accel), obs pitch (accel), obs yaw(mag heading), obs v_N(GPS), obs v_E(GPS), obs p_N(GPS), obs p_E(GPS)

#define ROLL_IDX 			0
#define PITCH_IDX 			1
#define YAW_IDX 			2
#define VEL_N_IDX 			3
#define VEL_E_IDX 			4
#define POS_N_IDX 			5
#define POS_E_IDX 			6
#define BIAS_A_X_IDX 		7
#define BIAS_A_Y_IDX 		8
#define BIAS_A_Z_IDX 		9
#define BIAS_G_X_IDX		10
#define BIAS_G_Y_IDX 		11
#define BIAS_G_Z_IDX 		12

#define OBS_ROLL_IDX 		0
#define OBS_PITCH_IDX 		1
#define OBS_YAW_IDX 		2
#define OBS_VEL_N_IDX		3
#define OBS_VEL_E_IDX		4
#define OBS_POS_N_IDX		5
#define OBS_POS_E_IDX		6

float x[NUM_STATES] = {0}; //State vector 

float P[NUM_STATES * NUM_STATES] = { //Prediction error covariance
    0.1, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   // r
    0,   0.1, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   // p
    0,   0,   10., 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   // y (unknown at startup → large)
    0,   0,   0,   1.0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   // vN
    0,   0,   0,   0,   1.0, 0,   0,   0,   0,   0,   0,   0,   0,   // vE
    0,   0,   0,   0,   0,   10., 0,   0,   0,   0,   0,   0,   0,   // pN (unknown at startup → large)
    0,   0,   0,   0,   0,   0,   10., 0,   0,   0,   0,   0,   0,   // pE (unknown at startup → large)
    0,   0,   0,   0,   0,   0,   0,   0.1, 0,   0,   0,   0,   0,   // bax
    0,   0,   0,   0,   0,   0,   0,   0,   0.1, 0,   0,   0,   0,   // bay
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0.1, 0,   0,   0,   // baz
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0.01,0,   0,   // bgx
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0.01,0,   // bgy
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0.01 // bgz
};

float fx[NUM_STATES] = {0};

float F[NUM_STATES * NUM_STATES] = {0}; //Jacobian of state-transition function [N x N]

// float Q[NUM_STATES * NUM_STATES] = { //template all states process noise matrix [M x M]
// // 	r         p         y         vN         vE         pN         pE         bax    bay    baz    bgx    bgy    bgz
//     7.62e-9f,  0,        0,        0,         0,         0,         0,         0,     0,     0,     0,     0,     0,     // roll (From MPU-6000: gyro density 8.73e-5 rad/s/√Hz → density² = 7.62e-9)
//     0,         7.62e-9f, 0,        0,         0,         0,         0,         0,     0,     0,     0,     0,     0,     // pitch
//     0,         0,        7.62e-9f, 0,         0,         0,         0,         0,     0,     0,     0,     0,     0,     // yaw
//     0,         0,        0,        1.54e-5f,  0,         0,         0,         0,     0,     0,     0,     0,     0,     // vel_N (From MPU-6000: accel density 3.92e-3 m/s²/√Hz → density² = 1.54e-5)
//     0,         0,        0,        0,         1.54e-5f,  0,         0,         0,     0,     0,     0,     0,     0,     // vel_E
//     0,         0,        0,        0,         0,         1.54e-5f,  0,         0,     0,     0,     0,     0,     0,     // p_N
//     0,         0,        0,        0,         0,         0,         1.54e-5f,  0,     0,     0,     0,     0,     0,     // p_E
//     0,         0,        0,        0,         0,         0,         0,         1e-6f, 0,     0,     0,     0,     0,     // bias_ax
//     0,         0,        0,        0,         0,         0,         0,         0,     1e-6f, 0,     0,     0,     0,     // bias_ay
//     0,         0,        0,        0,         0,         0,         0,         0,     0,     1e-6f, 0,     0,     0,     // bias_az
//     0,         0,        0,        0,         0,         0,         0,         0,     0,     0,     1e-7f, 0,     0,     // bias_gx
//     0,         0,        0,        0,         0,         0,         0,         0,     0,     0,     0,     1e-7f, 0,     // bias_gy
//     0,         0,        0,        0,         0,         0,         0,         0,     0,     0,     0,     0,     1e-7f  // bias_gz
// }; 

float Q[NUM_STATES] = { //template all states process noise matrix [N x N]
    7.62e-9f,  	// roll (From MPU-6000: gyro density 8.73e-5 rad/s/√Hz → density² = 7.62e-9)
    7.62e-9f,	// pitch
    7.62e-9f, 	// yaw
    1.54e-5f,	// vel_N (From MPU-6000: accel density 3.92e-3 m/s²/√Hz → density² = 1.54e-5)
    1.54e-5f,	// vel_E
    1.54e-5f,	// p_N
    1.54e-5f,	// p_E
    1e-6f,		// bias_ax
    1e-6f,		// bias_ay
    1e-6f,		// bias_az
    1e-7f,		// bias_gx
    1e-7f,		// bias_gy
    1e-7f  		// bias_gz
}; 

float z[NUM_OBSERVATIONS] = {0};

float hx[NUM_STATES] = {0};

// float H[NUM_OBSERVATIONS * NUM_STATES] = {//template all states sensor-function Jacobian matrix [M x N]
// 	   r  p  y  vN vE pN pE  bax bay baz bgx bgy bgz
//     1, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0, // obs_roll
//     0, 1, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0, // obs_pitch
//     0, 0, 1, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0, // obs_yaw
//     0, 0, 0, 1, 0, 0, 0,  0,  0,  0,  0,  0,  0, // obs_v_N
//     0, 0, 0, 0, 1, 0, 0,  0,  0,  0,  0,  0,  0, // obs_v_E
//     0, 0, 0, 0, 0, 1, 0,  0,  0,  0,  0,  0,  0, // obs_p_N
//     0, 0, 0, 0, 0, 0, 1,  0,  0,  0,  0,  0,  0  // obs_p_E
// }; 

// float R[NUM_OBSERVATIONS * NUM_OBSERVATIONS] = { //template all statesmeasurement-noise matrix [M x M]
// //  r_acc  p_acc  y_mag  vN_gps vE_gps pN_gps pE_gps
//     0.002f, 0,      0,     0,     0,     0,     0,     // obs_roll (from Accel)
//     0,      0.002f, 0,     0,     0,     0,     0,     // obs_pitch (from Accel)
//     0,      0,      0.001f, 0,     0,     0,     0,     // obs_yaw (from HMC5883L)
//     0,      0,      0,     0.01f, 0,     0,     0,     // obs_v_N (from LC76G Doppler)
//     0,      0,      0,     0,     0.01f, 0,     0,     // obs_v_E (from LC76G Doppler)
//     0,      0,      0,     0,     0,     6.25f, 0,     // obs_p_N (from LC76G Pos)
//     0,      0,      0,     0,     0,     0,     6.25f  // obs_p_E (from LC76G Pos)
// }; 

float R[NUM_OBSERVATIONS] = { //template all states measurement-noise matrix [M x M]
    0.001f,	// obs_roll (from Accel)
    0.001f,	// obs_pitch (from Accel)
    0.001f,	// obs_yaw (from HMC5883L)
    0.01f,	// obs_v_N (from LC76G Doppler)
    0.01f,	// obs_v_E (from LC76G Doppler)
    6.25f,	// obs_p_N (from LC76G Pos)
    6.25f  	// obs_p_E (from LC76G Pos)
}; 

float Lat_origin = 0, Lon_origin = 0;
ekf_t KalmanFilter = {0};

void ekf_init(float init_roll, float init_pitch, float init_yaw, float init_Lat_origin, float init_Lon_origin){
	KalmanFilter.EKF_M = NUM_OBSERVATIONS;
	KalmanFilter.EKF_N = NUM_STATES;

	KalmanFilter.x = x;
	KalmanFilter.P = P;

	KalmanFilter.fx = fx;
	KalmanFilter.F = F;
	KalmanFilter.Q = NULL;

	KalmanFilter.z = z;
	KalmanFilter.hx = hx;
	KalmanFilter.H = NULL;
	KalmanFilter.R = NULL;

	x[ROLL_IDX] = init_roll;
	x[PITCH_IDX] = init_pitch;
	x[YAW_IDX] = init_yaw;

	Lat_origin = init_Lat_origin;
	Lon_origin = init_Lon_origin;
}

void ekf_estimate(MPU6050_Sensor_t IMU, float timestep){
	fx[ROLL_IDX] = x[ROLL_IDX] + timestep * ((IMU.Gyro.x * (M_PI / 180.0f)) - x[BIAS_G_X_IDX]); //new angle = last angle + dt * (angle rate - last bias) (need to convert degree/s to radian/s)
	fx[PITCH_IDX] = x[PITCH_IDX] + timestep * ((IMU.Gyro.y * (M_PI / 180.0f)) - x[BIAS_G_Y_IDX]);

    float predicted_yaw = x[YAW_IDX] + timestep * ((IMU.Gyro.z * (M_PI / 180.0f)) - x[BIAS_G_Z_IDX]); //accumulate gyro z
    predicted_yaw = fmodf(predicted_yaw, 2.0f * M_PI); //because accumulating can goes out of range, we need to wrap it back into range
    if(predicted_yaw < 0.0f) predicted_yaw += 2.0f * M_PI; //fmod keeps the negative sign, so we fix it
	fx[YAW_IDX] = predicted_yaw;

    float ax = (IMU.Accel.x * 9.80665)  - x[BIAS_A_X_IDX]; //(need to convert G force to m/s²)
    float ay = (IMU.Accel.y * 9.80665) - x[BIAS_A_Y_IDX];
    float az = (IMU.Accel.z * 9.80665) - x[BIAS_A_Z_IDX];

    //use last state here instead of current measurement because the acceleration happened over the last timestep while we were at the old angle.
    float cr = cosf(x[ROLL_IDX]);
    float sr = sinf(x[ROLL_IDX]);
    float cp = cosf(x[PITCH_IDX]);
    float sp = sinf(x[PITCH_IDX]);
    float cy = cosf(x[YAW_IDX]);
    float sy = sinf(x[YAW_IDX]);

    //apply the rotation matrix to project 3D body accel to 2D earth accel (North/East)
    float accel_N = (cy * cp) * ax + (cy * sp * sr - sy * cr) * ay + (cy * sp * cr + sy * sr) * az;
    float accel_E = (sy * cp) * ax + (sy * sp * sr + cy * cr) * ay + (sy * sp * cr - cy * sr) * az;

	fx[VEL_N_IDX] = x[VEL_N_IDX] + timestep * accel_N;
    fx[VEL_E_IDX] = x[VEL_E_IDX] + timestep * accel_E;
	
	//again use the old velocity (x not fx) here to find the new position
	fx[POS_N_IDX] = x[POS_N_IDX] + timestep * x[VEL_N_IDX]; 
    fx[POS_E_IDX] = x[POS_E_IDX] + timestep * x[VEL_E_IDX];

	fx[BIAS_A_X_IDX] = x[BIAS_A_X_IDX];
    fx[BIAS_A_Y_IDX] = x[BIAS_A_Y_IDX];
    fx[BIAS_A_Z_IDX] = x[BIAS_A_Z_IDX];
    fx[BIAS_G_X_IDX] = x[BIAS_G_X_IDX];
    fx[BIAS_G_Y_IDX] = x[BIAS_G_Y_IDX];
    fx[BIAS_G_Z_IDX] = x[BIAS_G_Z_IDX];

	//calculate F(Jacobian of state-transition function, which is literally just the derivative of the equations above), column elements affect the row elements
	memset(F, 0, sizeof(F));
	_addeye(F, NUM_STATES);

	//Position wrt Velocity
	F[POS_N_IDX * NUM_STATES + VEL_N_IDX] = timestep;
    F[POS_E_IDX * NUM_STATES + VEL_E_IDX] = timestep;

	//Angle with respect to Gyro Biases (Subtracting bias means derivative is -dt)
	F[ROLL_IDX  * NUM_STATES + BIAS_G_X_IDX] = -timestep;
    F[PITCH_IDX * NUM_STATES + BIAS_G_Y_IDX] = -timestep;
    F[YAW_IDX   * NUM_STATES + BIAS_G_Z_IDX] = -timestep;

	//Velocity with respect to Accel Biases (Rotated by the DCM)
	F[VEL_N_IDX * NUM_STATES + BIAS_A_X_IDX] = -timestep * (cy * cp);
    F[VEL_N_IDX * NUM_STATES + BIAS_A_Y_IDX] = -timestep * (cy * sp * sr - sy * cr);
    F[VEL_N_IDX * NUM_STATES + BIAS_A_Z_IDX] = -timestep * (cy * sp * cr + sy * sr);

    F[VEL_E_IDX * NUM_STATES + BIAS_A_X_IDX] = -timestep * (sy * cp);
    F[VEL_E_IDX * NUM_STATES + BIAS_A_Y_IDX] = -timestep * (sy * sp * sr + cy * cr);
    F[VEL_E_IDX * NUM_STATES + BIAS_A_Z_IDX] = -timestep * (sy * sp * cr - cy * sr);

	//Velocity with respect to Yaw (Shortcut: Derivative of North wrt Yaw is -East, East wrt Yaw is North!)
    F[VEL_N_IDX * NUM_STATES + YAW_IDX] = -timestep * accel_E;
    F[VEL_E_IDX * NUM_STATES + YAW_IDX] =  timestep * accel_N;

    //Velocity with respect to Pitch
    float d_accel_N_dp = (-cy * sp) * ax + (cy * cp * sr) * ay + (cy * cp * cr) * az;
    float d_accel_E_dp = (-sy * sp) * ax + (sy * cp * sr) * ay + (sy * cp * cr) * az;
    F[VEL_N_IDX * NUM_STATES + PITCH_IDX] = timestep * d_accel_N_dp;
    F[VEL_E_IDX * NUM_STATES + PITCH_IDX] = timestep * d_accel_E_dp;

    //Velocity with respect to Roll
    float d_accel_N_dr = (cy * sp * cr + sy * sr) * ay + (-cy * sp * sr + sy * cr) * az;
    float d_accel_E_dr = (sy * sp * cr - cy * sr) * ay + (-sy * sp * sr - cy * cr) * az;
    F[VEL_N_IDX * NUM_STATES + ROLL_IDX] = timestep * d_accel_N_dr;
    F[VEL_E_IDX * NUM_STATES + ROLL_IDX] = timestep * d_accel_E_dr;

	//Because Q continous, we need to scale it by dt
	float Q_scaled[NUM_STATES * NUM_STATES] = {0};
	for(uint32_t i = 0; i < NUM_STATES; i++){
		Q_scaled[i * NUM_STATES + i] = Q[i] * timestep;
	}

	//Because position is integrated from velocity (which is intergrated from acceleration), its uncertainty grows at a faster rate over time.
	Q_scaled[POS_N_IDX * NUM_STATES + POS_N_IDX] = Q[POS_N_IDX] * (timestep * timestep);
	Q_scaled[POS_E_IDX * NUM_STATES + POS_E_IDX] = Q[POS_E_IDX] * (timestep * timestep);

	KalmanFilter.Q = Q_scaled;

	//Calculate predictions
	// x̂ₖ = f(x̂ₖ₋₁, uₖ) and 
	// Pₖ = Fₖ₋₁ Pₖ₋₁ Fₖ₋₁ᵀ + Qₖ₋₁
	ekf_predict(&KalmanFilter);
	KalmanFilter.Q = NULL; //restore back to NULL to avoid dangling pointer
}

void ekf_update_tilt(MPU6050_Sensor_t IMU){
	KalmanFilter.EKF_M = 2; //there are 2 observations in this case

	float roll_accel = (atan2f(-IMU.Accel.y, sqrtf((IMU.Accel.x * IMU.Accel.x) + (IMU.Accel.z * IMU.Accel.z))));
	float pitch_accel = (atan2f(IMU.Accel.x, sqrtf((IMU.Accel.y * IMU.Accel.y) + (IMU.Accel.z * IMU.Accel.z))));

	z[OBS_ROLL_IDX] = roll_accel;
	z[OBS_PITCH_IDX] = pitch_accel;

	hx[OBS_ROLL_IDX]  = x[ROLL_IDX];
    hx[OBS_PITCH_IDX] = x[PITCH_IDX];

	float H[2 * NUM_STATES] = {0};
	H[0 * NUM_STATES + ROLL_IDX]  = 1.0f; // Row 0 maps Obs Roll to Roll
    H[1 * NUM_STATES + PITCH_IDX] = 1.0f; // Row 1 maps Obs Pitch to Pitch
	KalmanFilter.H = H;

	float R_resized[2 * 2] = {0};
    R_resized[0 * 2 + 0] = R[0]; // Replace with your Roll R variance
    R_resized[1 * 2 + 1] = R[1]; // Replace with your Pitch R variance
	KalmanFilter.R = R_resized;

	//Calculate Update
	//Gₖ = Pₖ Hₖᵀ (Hₖ Pₖ Hₖᵀ + R)⁻¹
	//x̂ₖ = x̂ₖ + Gₖ(zₖ - h(x̂ₖ))
	//Pₖ = (I - Gₖ Hₖ) Pₖ
	ekf_update(&KalmanFilter);

	KalmanFilter.H = NULL; //restore back to NULL to avoid dangling pointer
	KalmanFilter.R = NULL; //restore back to NULL to avoid dangling pointer
}

void ekf_update_heading(HMC5883_Sensor_t Mag){
    KalmanFilter.EKF_M = 1; //there are 1 observations in this case

    //Grab current best guess for tilt from the EKF state
    float roll  = x[ROLL_IDX];
    float pitch = x[PITCH_IDX];

    //Project the Mag vectors onto the horizontal plane
    float Mag_x_Comp = Mag.x * cosf(pitch) + Mag.y * sinf(roll) * sinf(pitch) + Mag.z * cosf(roll) * sinf(pitch);
    float Mag_y_Comp = Mag.y * cosf(roll) - Mag.z * sinf(roll);

    //Calculate heading 
    float headingRad = atan2f(-Mag_y_Comp, Mag_x_Comp); //calculate heading, the output is the angle value in radiant of North relative to X axis (now how tf do we compensate tilting when we already have filter for tilt? do we use x (previous state?))
    float declinationAngleRad = -0.64166666666667  * (M_PI / 180.0f); //declination angle from Mag North to True North in HCMC as of April 2026
    headingRad += declinationAngleRad;
    if(headingRad < 0) headingRad += (2.0f * M_PI); //we want angle of X relative to North axis clockwise in radian

    //To prevent wrap around errors in the EKF, we feed it with the shortest path error instead
    float error = headingRad - x[YAW_IDX]; //amount from the current heading to the new heading
    error = fmodf(error, 2.0f * M_PI); //because this can goes out of range, we need to wrap it back into range
    if(error > M_PI) 
        error -= 2.0f * M_PI;
    else if (error < -M_PI)
        error += 2.0f * M_PI;
    
    z[0] = x[YAW_IDX] + error;
    hx[0] = x[YAW_IDX];

    float H[1 * NUM_STATES] = {0};
    H[YAW_IDX] = 1.0f; // Maps Obs Yaw to Yaw
    KalmanFilter.H = H;

    float R_resized[1] = {R[OBS_YAW_IDX]};
	KalmanFilter.R = R_resized;

    ekf_update(&KalmanFilter);

    //The EKF might pushed the state slightly below or above range we must wrap it back before the next predict cycle.
    x[YAW_IDX] = fmodf(x[YAW_IDX], 2.0f * M_PI);
    if (x[YAW_IDX] < 0.0f) {
        x[YAW_IDX] += 2.0f * M_PI;
    }

    KalmanFilter.H = NULL; //restore back to NULL to avoid dangling pointer
	KalmanFilter.R = NULL; //restore back to NULL to avoid dangling pointer
}

void ekf_update_position(){
    return;
}