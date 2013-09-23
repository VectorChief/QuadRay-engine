/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTGEOM_H
#define RT_RTGEOM_H

#include "rtbase.h"
#include "format.h"
#include "object.h"

/******************************************************************************/
/********************************   MATRICES   ********************************/
/******************************************************************************/

/*
 * Identity matrix.
 */
extern rt_mat4 iden4;

/*
 * Multiply matrix by vector.
 */
rt_void matrix_mul_vector(rt_vec4 vp, rt_mat4  m1, rt_vec4 v1);

/*
 * Multiply matrix by matrix.
 */
rt_void matrix_mul_matrix(rt_mat4 mp, rt_mat4 m1, rt_mat4 m2);

/*
 * Compute matrix from transform.
 */
rt_void matrix_from_transform(rt_mat4 mp, rt_TRANSFORM3D *t1);

/*
 * Compute inverse matrix.
 */
rt_void matrix_inverse(rt_mat4 mp, rt_mat4 m1);

/******************************************************************************/
/********************************   GEOMETRY   ********************************/
/******************************************************************************/

/*
 * Compute vector dot-product.
 */
#define RT_VECTOR_DOT(v1, v2)                                               \
(                                                                           \
    v1[RT_X] * v2[RT_X] +                                                   \
    v1[RT_Y] * v2[RT_Y] +                                                   \
    v1[RT_Z] * v2[RT_Z]                                                     \
)

/*
 * Compute vector cross-product.
 */
#define RT_VECTOR_CROSS(vp, v1, v2)                                         \
do                                                                          \
{                                                                           \
    vp[RT_X] = v1[RT_Y] * v2[RT_Z] - v2[RT_Y] * v1[RT_Z];                   \
    vp[RT_Y] = v1[RT_Z] * v2[RT_X] - v2[RT_Z] * v1[RT_X];                   \
    vp[RT_Z] = v1[RT_X] * v2[RT_Y] - v2[RT_X] * v1[RT_Y];                   \
}                                                                           \
while (0) /* "do {...} while (0)" to enforce semicolon ";" at the end */

/*
 * Determine if vert "p1" and face "q0-q1-q2" intersect from vert "p0".
 *
 * Return values:
 *  0 - don't intersect (or lie on the same plane)
 *  1 - intersect o-p-q ( ^ dir: 0, 3)
 *  2 - intersect o-q-p ( ^ dir: 0, 3)
 *  3 - intersect o-p=q
 */
rt_cell vert_to_face(rt_vec4 p0, rt_vec4 p1, rt_cell dir,
                     rt_vec4 q0, rt_vec4 q1, rt_vec4 q2,
                     rt_cell qk, rt_cell qi, rt_cell qj);

/*
 * Determine if edge "p1-p2" and edge "q1-q2" intersect from vert "p0".
 *
 * Return values:
 *  0 - don't intersect (or lie on the same plane)
 *  1 - intersect o-p-q
 *  2 - intersect o-q-p
 *  3 - intersect o-p=q
 */
rt_cell edge_to_edge(rt_vec4 p0,
                     rt_vec4 p1, rt_vec4 p2, rt_cell pk,
                     rt_vec4 q1, rt_vec4 q2, rt_cell qk);

/*
 * Determine if "shw" bbox casts shadow on "srf" bbox from "lgt" pos.
 *
 * Return values:
 *  0 - no
 *  1 - yes
 */
rt_cell bbox_shad(rt_Light *lgt, rt_Surface *shw, rt_Surface *srf);

#endif /* RT_RTGEOM_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
