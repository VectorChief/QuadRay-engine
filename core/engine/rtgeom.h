/******************************************************************************/
/* Copyright (c) 2013-2017 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTGEOM_H
#define RT_RTGEOM_H

#include "rtbase.h"
#include "format.h"

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
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

/* Structures */

struct rt_VERT;
struct rt_EDGE;
struct rt_FACE;

struct rt_BOUND;
struct rt_SHAPE;

/******************************************************************************/
/*********************************   VECTORS   ********************************/
/******************************************************************************/

/* ---------------------------------   SET   -------------------------------- */

/*
 * Set vec3 to another vec3.
 */
#define RT_VEC3_SET(vr, va)                                                 \
do                                                                          \
{                                                                           \
    vr[0] = va[0];                                                          \
    vr[1] = va[1];                                                          \
    vr[2] = va[2];                                                          \
}                                                                           \
while (0) /* "do {...} while (0)" to enforce semicolon ";" at the end */

/*
 * Set vec3 to 1 scalar value.
 */
#define RT_VEC3_SET_VAL1(vr, sa)                                            \
do                                                                          \
{                                                                           \
    vr[0] = (sa);                                                           \
    vr[1] = (sa);                                                           \
    vr[2] = (sa);                                                           \
}                                                                           \
while (0) /* "do {...} while (0)" to enforce semicolon ";" at the end */

/* ---------------------------------   MIN   -------------------------------- */

/*
 * Compute the minimum of two vec3.
 */
#define RT_VEC3_MIN(vr, va, vb)                                             \
do                                                                          \
{                                                                           \
    vr[0] = RT_MIN(va[0], vb[0]);                                           \
    vr[1] = RT_MIN(va[1], vb[1]);                                           \
    vr[2] = RT_MIN(va[2], vb[2]);                                           \
}                                                                           \
while (0) /* "do {...} while (0)" to enforce semicolon ";" at the end */

/* ---------------------------------   MAX   -------------------------------- */

/*
 * Compute the maximum of two vec3.
 */
#define RT_VEC3_MAX(vr, va, vb)                                             \
do                                                                          \
{                                                                           \
    vr[0] = RT_MAX(va[0], vb[0]);                                           \
    vr[1] = RT_MAX(va[1], vb[1]);                                           \
    vr[2] = RT_MAX(va[2], vb[2]);                                           \
}                                                                           \
while (0) /* "do {...} while (0)" to enforce semicolon ";" at the end */

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
 * Compute the product of two vec3 (cross).
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
    vr[0] = va[0] * (sa);                                                   \
    vr[1] = va[1] * (sa);                                                   \
    vr[2] = va[2] * (sa);                                                   \
}                                                                           \
while (0) /* "do {...} while (0)" to enforce semicolon ";" at the end */

/*
 * Compute and add the product of vec3 and 1 scalar value.
 */
#define RT_VEC3_MAD_VAL1(vr, va, sa)                                        \
do                                                                          \
{                                                                           \
    vr[0] += va[0] * (sa);                                                  \
    vr[1] += va[1] * (sa);                                                  \
    vr[2] += va[2] * (sa);                                                  \
}                                                                           \
while (0) /* "do {...} while (0)" to enforce semicolon ";" at the end */

/* ---------------------------------   DOT   -------------------------------- */

/*
 * Compute the product of two vec3 (dot).
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
rt_void matrix_from_transform(rt_mat4 mp, rt_TRANSFORM3D *t1, rt_bool as);

/*
 * Compute upper-left 3x3 inverse of a 4x4 matrix.
 */
rt_void matrix_inverse(rt_mat4 mp, rt_mat4 m1);

/******************************************************************************/
/********************************   GEOMETRY   ********************************/
/******************************************************************************/

/*
 * Vert structure represents vertex in world space.
 */
struct rt_VERT
{
    rt_vec4 pos;
};

/*
 * Edge structure represents line segment in world space.
 */
struct rt_EDGE
{
    rt_si32 index[2];
    rt_si32 k;
};

/*
 * Face structure represents rectangle on a plane in world space.
 */
struct rt_FACE
{
    rt_si32 index[4];
    rt_si32 k, i, j;
};

/*
 * Bound structure represents object's boundary, which includes
 * bounding box's local parameters, bounding box's global geometry,
 * bounding sphere around bounding box and transform matrices.
 */
struct rt_BOUND
{
    /* host object's pointer and tag */
    rt_pntr             obj;
    rt_si32             tag;
    /* host object's axis mapping */
    rt_si32            *map;
    rt_si32            *sgn;
    /* host object's matrices */
    rt_mat4            *pinv;
    rt_mat4            *pmtx;
    /* host object's position */
    rt_real            *pos;

    /* runtime optimization flags */
    rt_si32            *opts;

    /* host object's trnode
     * BOUND struct pointer */
    rt_BOUND           *trnode;

    /* bounding box,
     * all sides clipped or non-clipped are boundaries */
    rt_vec4             bmin;
    rt_vec4             bmax;

    /* bounding box geometry */
    rt_VERT            *verts;
    rt_si32             verts_num;
    rt_EDGE            *edges;
    rt_si32             edges_num;
    rt_FACE            *faces;
    rt_si32             faces_num;

    /* bounding volume center */
    rt_vec4             mid;
    /* bounding volume radius */
    rt_real             rad;

    /* number of flags set for bbox's fully covered (by plane) faces */
    rt_si32             fln;
    /* in minmax data format: (1 - min, 2 - max) << (axis_index * 2) */
    rt_si32             flm;
    /* in faces index format as defined in bx_faces: 1 << face_index */
    rt_si32             flf;
};

/*
 * Shape structure represents surface's shape, which includes
 * surface's geometry coefficients, custom clippers list and
 * clipping box's local parameters based on axis clippers.
 */
struct rt_SHAPE : public rt_BOUND
{
    /* clipping box,
     * non-clipped sides are at respective +/-infinity */
    rt_vec4             cmin;
    rt_vec4             cmax;

    /* surface shape coeffs */
    rt_vec4             sci;
    rt_vec4             scj;
    rt_vec4             sck;
    /* custom clippers list */
    rt_pntr            *ptr;
};

/*
 * Determine if "nd1's" bbox casts shadow on "nd2's" bbox
 * as seen from "obj's" bbox "mid" (light's "pos").
 *
 * Return values:
 *   0 - no
 *   1 - yes
 */
rt_si32 bbox_shad(rt_BOUND *obj, rt_BOUND *nd1, rt_BOUND *nd2);

/*
 * Convert bbox flags from "flm" to "flf" format.
 *
 * Return values:
 *   flags
 */
rt_si32 bbox_flag(rt_si32 *map, rt_si32 flm);

/*
 * Determine the order of "nd1's" and "nd2's" bboxes
 * as seen from "obj's" bbox "mid".
 *
 * Return values:
 *   1 - no swap
 *   2 - do swap
 *   3 - neutral
 * 4|1 - no swap, remove (nd1 fully obscures nd2)
 * 4|2 - do swap, remove (nd2 fully obscures nd1)
 * 8|1 - no swap, unsortable
 * 8|2 - do swap, unsortable
 */
rt_si32 bbox_sort(rt_BOUND *obj, rt_BOUND *nd1, rt_BOUND *nd2);

/*
 * Determine which side of clipped "srf" is seen
 * from "obj's" entire bbox ("pos" in case of light or camera).
 *
 * Return values:
 *   0 - none, if both surfaces are the same plane
 *   1 - inner
 *   2 - outer
 *   3 - both, also if on the surface with margin
 */
rt_si32 bbox_side(rt_BOUND *obj, rt_SHAPE *srf);

#endif /* RT_RTGEOM_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
