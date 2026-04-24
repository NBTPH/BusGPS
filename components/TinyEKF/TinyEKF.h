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
#include <math.h>

/**
  * Floating-point precision defaults to single but can be made double via
    <tt><b>#define _float_t double</b></tt> before <tt>#include <tinyekf.h></tt>
  */
#ifndef _float_t
#define _float_t float
#endif

typedef struct{
    int EKF_M; //number of states
    int EKF_N; //number of observations

    _float_t *x; //State vector [M x 1]
    _float_t *P; //Prediction error covariance [M x M]

    _float_t *fx; //predicted values [M x 1]
    _float_t *F; //Jacobian of state-transition function [M x M]
    _float_t *Q; //process noise matrix [M x M]

    _float_t *z; //observations [N x 1]
    _float_t *hx; //predicted values from prediction step [M x 1]
    _float_t *H; //sensor-function Jacobian matrix [N x M]
    _float_t *R; //measurement-noise matrix [N x N]
}ekf_t;

#endif