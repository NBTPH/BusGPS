/*
 * Extended Kalman Filter for embedded processors
 *
 * Copyright (C) 2024 Simon D. Levy
 *
 * MIT License
 */
#ifndef _TinyEKF_H_
#define _TinyEKF_H_

#include <math.h>
#include <stdbool.h>
#include <string.h>

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

    _float_t *x; //State vector
    _float_t *P; //Prediction error covariance

    _float_t *fx; //predicted values
    _float_t *F; //Jacobian of state-transition function
    _float_t *Q; //process noise matrix

    _float_t *z; //observations
    _float_t *hx; //predicted values from prediction step
    _float_t *H; //sensor-function Jacobian matrix
    _float_t *R; //measurement-noise matrix 
}ekf_t;

#endif