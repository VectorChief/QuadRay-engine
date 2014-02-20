/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTGEOM_H
#define RT_RTGEOM_H

#include "rtbase.h"
#include "format.h"
#include "object.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtgeom.h: Interface for the geometry utils library.
 *
 * More detailed description of this subsystem is given in rtgeom.cpp.
 * Recommended naming scheme for C++ types and definitions is given in rtbase.h.
 */

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
 * Compute upper-left 3x3 inverse of a 4x4 matrix.
 */
rt_void matrix_inverse(rt_mat4 mp, rt_mat4 m1);

/******************************************************************************/
/*********************************   VECTORS   ********************************/
/******************************************************************************/

/**********************************   VEC2   **********************************/

/* ---------------------------------   ADD   -------------------------------- */

/*
 * Compute the sum of two vec2.
 */
#define RT_VEC2_ADD(vr, va, vb)                                             \
do                                                                          \
{                                                                           \
    vr[0] = va[0] + vb[0];                                                  \
    vr[1] = va[1] + vb[1];                                                  \
}                                                                           \
while (0) /* "do {...} while (0)" to enforce semicolon ";" at the end */

/* ---------------------------------   SUB   -------------------------------- */

/*
 * Compute the difference of two vec2.
 */
#define RT_VEC2_SUB(vr, va, vb)                                             \
do                                                                          \
{                                                                           \
    vr[0] = va[0] - vb[0];                                                  \
    vr[1] = va[1] - vb[1];                                                  \
}                                                                           \
while (0) /* "do {...} while (0)" to enforce semicolon ";" at the end */

/* ---------------------------------   MUL   -------------------------------- */

/*
 * Compute the cross-product of two vec2 (complex numbers).
 */
#define RT_VEC2_MUL(vr, va, vb)                                             \
do                                                                          \
{                                                                           \
    vr[0] = va[0] * vb[0] - vb[1] * va[1];                                  \
    vr[1] = va[0] * vb[1] + vb[0] * va[1];                                  \
}                                                                           \
while (0) /* "do {...} while (0)" to enforce semicolon ";" at the end */

/*
 * Compute the product of vec2 and 1 scalar value.
 */
#define RT_VEC2_MUL_VAL1(vr, va, sa)                                        \
do                                                                          \
{                                                                           \
    vr[0] = va[0] * sa;                                                     \
    vr[1] = va[1] * sa;                                                     \
}                                                                           \
while (0) /* "do {...} while (0)" to enforce semicolon ";" at the end */

/* ---------------------------------   DOT   -------------------------------- */

/*
 * Compute the dot-product of two vec2.
 */
#define RT_VEC2_DOT(va, vb)                                                 \
(                                                                           \
    va[0] * vb[0] +                                                         \
    va[1] * vb[1]                                                           \
)

/* ---------------------------------   LEN   -------------------------------- */

/*
 * Compute the length of vec2.
 */
#define RT_VEC2_LEN(va)                                                     \
(                                                                           \
    RT_SQRT(RT_VEC2_DOT(va, va))                                            \
)

/**********************************   VEC3   **********************************/

/* ---------------------------------   ADD   -------------------------------- */

/*
 * Compute the sum of two vec3.
 */
#define RT_VEC3_ADD(vr, va, vb)                                             \
do                                                                          \
{                                                                           \
    vr[0] = va[0] + vb[0];                                                  \
    vr[1] = va[1] + vb[1];                                                  \
    vr[2] = va[2] + vb[2];                                                  \
}                                                                           \
while (0) /* "do {...} while (0)" to enforce semicolon ";" at the end */

/* ---------------------------------   SUB   -------------------------------- */

/*
 * Compute the difference of two vec3.
 */
#define RT_VEC3_SUB(vr, va, vb)                                             \
do                                                                          \
{                                                                           \
    vr[0] = va[0] - vb[0];                                                  \
    vr[1] = va[1] - vb[1];                                                  \
    vr[2] = va[2] - vb[2];                                                  \
}                                                                           \
while (0) /* "do {...} while (0)" to enforce semicolon ";" at the end */

/* ---------------------------------   MUL   -------------------------------- */

/*
 * Compute the cross-product of two vec3.
 */
#define RT_VEC3_MUL(vr, va, vb)                                             \
do                                                                          \
{                                                                           \
    vr[0] = va[1] * vb[2] - vb[1] * va[2];                                  \
    vr[1] = va[2] * vb[0] - vb[2] * va[0];                                  \
    vr[2] = va[0] * vb[1] - vb[0] * va[1];                                  \
}                                                                           \
while (0) /* "do {...} while (0)" to enforce semicolon ";" at the end */

/*
 * Compute the product of vec3 and 1 scalar value.
 */
#define RT_VEC3_MUL_VAL1(vr, va, sa)                                        \
do                                                                          \
{                                                                           \
    vr[0] = va[0] * sa;                                                     \
    vr[1] = va[1] * sa;                                                     \
    vr[2] = va[2] * sa;                                                     \
}                                                                           \
while (0) /* "do {...} while (0)" to enforce semicolon ";" at the end */

/* ---------------------------------   DOT   -------------------------------- */

/*
 * Compute the dot-product of two vec3.
 */
#define RT_VEC3_DOT(va, vb)                                                 \
(                                                                           \
    va[0] * vb[0] +                                                         \
    va[1] * vb[1] +                                                         \
    va[2] * vb[2]                                                           \
)

/* ---------------------------------   LEN   -------------------------------- */

/*
 * Compute the length of vec3.
 */
#define RT_VEC3_LEN(va)                                                     \
(                                                                           \
    RT_SQRT(RT_VEC3_DOT(va, va))                                            \
)

/******************************************************************************/
/********************************   GEOMETRY   ********************************/
/******************************************************************************/

/*
 * Determine if "shw" bbox casts shadow on "srf" bbox from "lgt" pos.
 *
 * Return values:
 *  0 - no
 *  1 - yes
 */
rt_cell bbox_shad(rt_Light *lgt, rt_Surface *shw, rt_Surface *srf);

/*
 * Determine if "nd1" and "nd2" bounds are in correct order as seen from "obj".
 *
 * Return values:
 *  1 - neutral
 *  2 - unsortable
 *  3 - don't swap
 *  4 - do swap, never stored in rt_ELEM's "data" field in the engine
 */
rt_cell bbox_sort(rt_Object *obj, rt_Node *nd1, rt_Node *nd2);

/*
 * Determine which side of clipped "srf" is seen from "pos".
 *
 * Return values:
 *  1 - inner
 *  2 - outer
 *  3 - both, also if on the surface with margin
 */
rt_cell cbox_side(rt_real *pos, rt_Surface *srf);

/*
 * Determine which side of clipped "srf" is seen from "ref" bbox.
 *
 * Return values:
 *  0 - none, if both surfaces are the same plane
 *  1 - inner
 *  2 - outer
 *  3 - both
 */
rt_cell bbox_side(rt_Surface *srf, rt_Surface *ref);

#endif /* RT_RTGEOM_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
