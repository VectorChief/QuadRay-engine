/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTGEOM_H
#define RT_RTGEOM_H

#include "rtbase.h"
#include "format.h"

/******************************************************************************/
/********************************   MATRICES   ********************************/
/******************************************************************************/

/*
 * Identity matrix.
 */
extern rt_mat4 iden4;

/*
 * This algorithm multiplies matrix by vector.
 */
rt_void matrix_mul_vector(rt_vec4 vp, rt_mat4  m1, rt_vec4 v1);

/*
 * This algorithm multiplies matrix by matrix.
 */
rt_void matrix_mul_matrix(rt_mat4 mp, rt_mat4 m1, rt_mat4 m2);

/*
 * This algorithm computes matrix from transform.
 */
rt_void matrix_from_transform(rt_mat4 mp, rt_TRANSFORM3D *t1);

#endif /* RT_RTGEOM_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
