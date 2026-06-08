/*
 * Extended Kalman Filter for embedded processors
 *
 * Copyright (C) 2024 Simon D. Levy
 *
 * MIT License
 */
#ifndef _TinyEKF_H_
#define _TinyEKF_H_

#include <common.h>
#include "esp_attr.h"

/**
  * Floating-point precision defaults to single but can be made double via
    <tt><b>#define float double</b></tt> before <tt>#include <tinyekf.h></tt>
  */

typedef struct{
	int EKF_M; //number of observation
	int EKF_N; //number of states

	float *x; //State vector [N x 1]
	float *P; //Prediction error covariance [N x N]

	float *fx; //predicted values [N x 1]
	float *F; //Jacobian of state-transition function [N x N]
	float *Q; //process noise matrix [N x N]

	float *z; //observations [M x 1]
	float *hx; //predicted values from prediction step [N x 1]
	float *H; //sensor-function Jacobian matrix [M x N]
	float *R; //measurement-noise matrix [M x M]
}ekf_t;

void ekf_predict(ekf_t *ekf);
bool ekf_update(ekf_t *ekf);

#endif