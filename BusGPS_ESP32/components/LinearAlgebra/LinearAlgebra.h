#ifndef _LINALG_H
#define _LINALG_H

#include <common.h>
#include <math.h>

void _mulmat(
        const float * a, 
        const float * b, 
        float * c, 
        const int arows, 
        const int acols, 
        const int bcols);

void _mulvec(
        const float * a, 
        const float * x, 
        float * y, 
        const int m, 
        const int n);

void _transpose(const float * a, float * at, const int m, const int n);

void _addmat(
        const float * a, const float * b, float * c, 
        const int m, const int n);

void _negate(float * a, const int m, const int n);

void _addeye(float * a, const int n);


/* Cholesky-decomposition matrix-inversion code, adapated from
http://jean-pierre.moreau.pagesperso-orange.fr/Cplus/_choles_cpp.txt */

int _choldc1(float * a, float * p, const int n);

int _choldcsl(const float * A, float * a, float * p, const int n);

int _cholsl(const float * A, float * a, float * p, const int n);

void _addvec(const float * a, const float * b, float * c, const int n);

void _sub(const float * a, const float * b, float * c, const int n);

bool invert(const float * a, float * ainv, const int n);

bool jacobi_eigensystem(float *input_matrix, float *eigenvalues, float *eigenvectors, const int n);

bool angle_between(const float * a, const float * b, float * c, const int n);

int print_matrix(const float * a, const int m, const int n);

int print_vector(const float * a, const int n);

#endif