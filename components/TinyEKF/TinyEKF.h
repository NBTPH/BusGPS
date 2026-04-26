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
#include <LinearAlgebra.h>

/**
  * Floating-point precision defaults to single but can be made double via
    <tt><b>#define float double</b></tt> before <tt>#include <tinyekf.h></tt>
  */

typedef struct{
	int EKF_M; //number of states
	int EKF_N; //number of observations

	float *x; //State vector [M x 1]
	float *P; //Prediction error covariance [M x M]

	float *fx; //predicted values [M x 1]
	float *F; //Jacobian of state-transition function [M x M]
	float *Q; //process noise matrix [M x M]

	float *z; //observations [N x 1]
	float *hx; //predicted values from prediction step [M x 1]
	float *H; //sensor-function Jacobian matrix [N x M]
	float *R; //measurement-noise matrix [N x N]
}ekf_t;

#endif