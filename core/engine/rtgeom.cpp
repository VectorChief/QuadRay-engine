/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <string.h>

#include "rtconf.h"
#include "rtgeom.h"
#include "system.h"
#include "tracer.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtgeom.cpp: Implementation of the geometry utils library.
 *
 * Utility file for the engine responsible for vector-matrix operations
 * as well as routines to determine bounding and clipping boxes relationship.
 *
 * Utility file names are usually in the form of rt****.cpp/h,
 * while core engine parts are located in ******.cpp/h files.
 */

/******************************************************************************/
/********************************   MATRICES   ********************************/
/******************************************************************************/

/*
 * Identity 4x4 matrix.
 */
rt_mat4 iden4 =
{
    1.0f,       0.0f,       0.0f,       0.0f,
    0.0f,       1.0f,       0.0f,       0.0f,
    0.0f,       0.0f,       1.0f,       0.0f,
    0.0f,       0.0f,       0.0f,       1.0f,
};

/*
 * Check if given address ranges overlap.
 */
static
rt_bool in_range(rt_real *p1, rt_cell n1, rt_real *p2, rt_cell n2)
{
    if ((p1 >= p2 && p1 < p2 + n2) || (p2 >= p1 && p2 < p1 + n1))
    {
        return RT_TRUE;
    }

    return RT_FALSE;
}

/*
 * Multiply matrix by vector.
 */
rt_void matrix_mul_vector(rt_vec4 vp, rt_mat4 m1, rt_vec4 v1)
{
#if RT_DEBUG == 1
    if (in_range(vp, 4, m1[0], 16) || in_range(vp, 4, v1, 4))
    {
        throw rt_Exception("attempt to multiply vectors in place");
    }
#endif /* RT_DEBUG */

    rt_cell i;

    for (i = 0; i < 4; i++)
    {
        vp[i] = m1[0][i] * v1[0] + 
                m1[1][i] * v1[1] +
                m1[2][i] * v1[2] + 
                m1[3][i] * v1[3];
    }
}

/*
 * Multiply matrix by matrix.
 */
rt_void matrix_mul_matrix(rt_mat4 mp, rt_mat4 m1, rt_mat4 m2)
{
#if RT_DEBUG == 1
    if (in_range(mp[0], 16, m1[0], 16) || in_range(mp[0], 16, m2[0], 16))
    {
        throw rt_Exception("attempt to multiply matrices in place");
    }
#endif /* RT_DEBUG */

    rt_cell i;

    for (i = 0; i < 4; i++)
    {
        matrix_mul_vector(mp[i], m1, m2[i]);
    }
}

/*
 * Compute matrix from transform.
 */
rt_void matrix_from_transform(rt_mat4 mp, rt_TRANSFORM3D *t1)
{
    rt_mat4 mt;

    rt_real scl_x = t1->scl[RT_X];
    rt_real scl_y = t1->scl[RT_Y];
    rt_real scl_z = t1->scl[RT_Z];
    rt_mat4 sc =
    {
        scl_x,      0.0f,       0.0f,       0.0f,
        0.0f,       scl_y,      0.0f,       0.0f,
        0.0f,       0.0f,       scl_z,      0.0f,
        0.0f,       0.0f,       0.0f,       1.0f,
    };

    rt_real sin_x = RT_SINA(t1->rot[RT_X]);
    rt_real cos_x = RT_COSA(t1->rot[RT_X]);
    rt_mat4 rx =
    {
        1.0f,       0.0f,       0.0f,       0.0f,
        0.0f,      +cos_x,     +sin_x,      0.0f,
        0.0f,      -sin_x,     +cos_x,      0.0f,
        0.0f,       0.0f,       0.0f,       1.0f,
    };

    rt_real sin_y = RT_SINA(t1->rot[RT_Y]);
    rt_real cos_y = RT_COSA(t1->rot[RT_Y]);
    rt_mat4 ry =
    {
       +cos_y,      0.0f,      -sin_y,      0.0f,
        0.0f,       1.0f,       0.0f,       0.0f,
       +sin_y,      0.0f,      +cos_y,      0.0f,
        0.0f,       0.0f,       0.0f,       1.0f,
    };

    rt_real sin_z = RT_SINA(t1->rot[RT_Z]);
    rt_real cos_z = RT_COSA(t1->rot[RT_Z]);
    rt_mat4 rz =
    {
       +cos_z,     +sin_z,      0.0f,       0.0f,
       -sin_z,     +cos_z,      0.0f,       0.0f,
        0.0f,       0.0f,       1.0f,       0.0f,
        0.0f,       0.0f,       0.0f,       1.0f,
    };

    rt_real pos_x = t1->pos[RT_X];
    rt_real pos_y = t1->pos[RT_Y];
    rt_real pos_z = t1->pos[RT_Z];
    rt_mat4 ps =
    {
        1.0f,       0.0f,       0.0f,       0.0f,
        0.0f,       1.0f,       0.0f,       0.0f,
        0.0f,       0.0f,       1.0f,       0.0f,
        pos_x,      pos_y,      pos_z,      1.0f,
    };

    matrix_mul_matrix(mt, rx, sc);
    matrix_mul_matrix(mp, ry, mt);
    matrix_mul_matrix(mt, rz, mp);
    matrix_mul_matrix(mp, ps, mt);
}

/*
 * Compute upper-left 3x3 inverse of a 4x4 matrix.
 */
rt_void matrix_inverse(rt_mat4 mp, rt_mat4 m1)
{
    memset(mp, 0, sizeof(rt_real) * 16);

    rt_real A = m1[1][1] * m1[2][2] - m1[2][1] * m1[1][2];
    rt_real B = m1[2][1] * m1[0][2] - m1[0][1] * m1[2][2];
    rt_real C = m1[0][1] * m1[1][2] - m1[1][1] * m1[0][2];

    rt_real D = m1[2][0] * m1[1][2] - m1[1][0] * m1[2][2];
    rt_real E = m1[0][0] * m1[2][2] - m1[2][0] * m1[0][2];
    rt_real F = m1[0][2] * m1[1][0] - m1[0][0] * m1[1][2];

    rt_real G = m1[1][0] * m1[2][1] - m1[2][0] * m1[1][1];
    rt_real H = m1[2][0] * m1[0][1] - m1[0][0] * m1[2][1];
    rt_real K = m1[0][0] * m1[1][1] - m1[1][0] * m1[0][1];

    rt_real q = 1.0f / (m1[0][0] * A + m1[1][0] * B + m1[2][0] * C);

    mp[0][0] = A * q;
    mp[0][1] = B * q;
    mp[0][2] = C * q;

    mp[1][0] = D * q;
    mp[1][1] = E * q;
    mp[1][2] = F * q;

    mp[2][0] = G * q;
    mp[2][1] = H * q;
    mp[2][2] = K * q;

#if RT_DEBUG == 1
    rt_cell i, j, k = 0;

    rt_mat4 tm;

    matrix_mul_matrix(tm, mp, m1);

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            if (RT_FABS(tm[i][j] - iden4[i][j]) > 0.00001)
            {
                k = 1;
                break;
            }
        }
    }

    if (k == 0)
    {
        return;
    }

    RT_LOGE("Original matrix\n");

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            RT_LOGE("%f ", m1[j][i]);
        }
        RT_LOGE("\n");
    }

    RT_LOGE("Inverted matrix\n");

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            RT_LOGE("%f ", mp[j][i]);
        }
        RT_LOGE("\n");
    }

    throw rt_Exception("inverted matrix mismatch");
#endif /* RT_DEBUG */
}

/******************************************************************************/
/********************************   GEOMETRY   ********************************/
/******************************************************************************/

/*
 * Determine if vert "p1" and face "q0-q1-q2" intersect as seen from vert "p0".
 * Parameters "qk, qi, qj" specify axis mapping indices for axis-aligned face,
 * so that local I, J axes (face's base) and local K axis (face's normal) are
 * mapped to the world X, Y, Z axes, in which case face is a quad as opposed
 * to a triangular face, when at least one of the axes indices equals 3.
 * False-positives are allowed due to computational inaccuracy.
 *
 * Based on the original idea by Tomas Möller (tompa[AT]clarus[DOT]se)
 * and Ben Trumbore (wbt[AT]graphics[DOT]cornell[DOT]edu)
 * presented in the article "Fast, Minimum Storage Ray/Triangle Intersection"
 * available at http://www.graphics.cornell.edu/pubs/1997/MT97.html
 * converted to division-less version with margins by VectorChief.
 *
 * Return values:
 *  0 - don't intersect
 *  1 - intersect o-p-q
 *  2 - intersect o-q-p
 *  3 - intersect o-p=q, to handle bbox stacking
 *  4 - intersect o=q-p, to handle bbox stacking
 */
static
rt_cell vert_to_face(rt_vec4 p0, rt_vec4 p1,
                     rt_vec4 q0, rt_vec4 q1, rt_vec4 q2,
                     rt_cell qk, rt_cell qi, rt_cell qj)
{
    rt_real d, s, t, u, v;

    /* check if face is an axis-aligned quad,
     * "qk, qi, qj" hold world axes indices
     * for respective local I, J, K axes */
    if (qk < 3 && qi < 3 && qj < 3)
    {
        /* distance from origin to vert
         * in face's normal direction */
        d = p1[qk] - p0[qk];

        /* distance from origin to face
         * in face's normal direction */
        t = q0[qk] - p0[qk];

        /* make sure inequality is multiplied
         * by a positive number, so that relations hold */
        t = d < 0.0f ? -t : +t;
        d = RT_FABS(d);

        /* calculate "u" parameter and test bounds */
        u = (p1[qi] - p0[qi]) * t;

        /* if hit outside with margin, return miss */
        if (u < (RT_MIN(q0[qi], q1[qi]) - p0[qi] - RT_CULL_THRESHOLD) * d
        ||  u > (RT_MAX(q0[qi], q1[qi]) - p0[qi] + RT_CULL_THRESHOLD) * d)
        {
            return 0;
        }

        /* calculate "v" parameter and test bounds */
        v = (p1[qj] - p0[qj]) * t;

        /* if hit outside with margin, return miss */
        if (v < (RT_MIN(q0[qj], q2[qj]) - p0[qj] - RT_CULL_THRESHOLD) * d
        ||  v > (RT_MAX(q0[qj], q2[qj]) - p0[qj] + RT_CULL_THRESHOLD) * d)
        {
            return 0;
        }
    }
    /* otherwise face is an arbitrary triangle */
    else
    {
        rt_vec4 e1, e2, pr, qr, mx, nx;

        /* 1st edge vector */
        RT_VEC3_SUB(e1, q1, q0);

        /* 2nd edge vector */
        RT_VEC3_SUB(e2, q2, q0);

        /* ray's vector */
        RT_VEC3_SUB(pr, p1, p0);

        /* ray's origin */
        RT_VEC3_SUB(qr, p0, q0);

        /* cross product of
         * ray's vector and 2nd edge */
        RT_VEC3_MUL(mx, pr, e2);

        /* calculate determinant "d" */
        d = RT_VEC3_DOT(e1, mx);

        /* make sure inequality is multiplied
         * by a positive number, so that relations hold */
        s = d < 0.0f ? -1.0f : +1.0f;
        d = RT_FABS(d);

        /* calculate "u" parameter and test bounds */
        u = RT_VEC3_DOT(qr, mx) * s;

        /* if hit outside with margin, return miss */
        if (u < (0.0f - RT_CULL_THRESHOLD) * d
        ||  u > (1.0f + RT_CULL_THRESHOLD) * d)
        {
            return 0;
        }

        /* cross product of
         * ray's origin and 1st edge */
        RT_VEC3_MUL(nx, qr, e1);

        /* calculate "v" parameter and test bounds */
        v = RT_VEC3_DOT(pr, nx) * s;

        /* if hit outside with margin, return miss */
        if (v < (0.0f - RT_CULL_THRESHOLD) * d
        ||  v > (1.0f + RT_CULL_THRESHOLD) * d - u)
        {
            return 0;
        }

        /* calculate "t",
         * analog of distance to intersection */
        t = RT_VEC3_DOT(e2, nx) * s;
    }

    /*            | 0 |           | 1 |            */
    /* -----------|-*-|-----------|-*-|----------- */
    /*      0     | 4 |     2     | 3 |     1      */
    return t >  (1.0f + RT_CULL_THRESHOLD) * d ? 1 :
           t >= (1.0f - RT_CULL_THRESHOLD) * d ? 3 :
           t >  (0.0f + RT_CULL_THRESHOLD) * d ? 2 :
           t >= (0.0f - RT_CULL_THRESHOLD) * d ? 4 : 0;
}

/*
 * Determine if edge "p1-p2" and edge "q1-q2" intersect as seen from vert "p0".
 * Parameters "pk, qk" specify axis mapping indices for axis-aligned edges,
 * so that local K axis (edge's direction) is mapped to the world X, Y, Z axes.
 * False-positives are allowed due to computational inaccuracy.
 *
 * In order to figure out "u" and "v" intersection parameters along
 * the 1st and the 2nd edges respectively the same ray/triangle intersection
 * algorithm is used as follows: for "u" intersection the ray is "p1-p2",
 * while the face is "q1-p0-q2", for "v" intersection the ray is "q1-q2",
 * while the face is "p1-p0-p2", so that the common terms are reused.
 *
 * Return values:
 *  0 - don't intersect
 *  1 - intersect o-p-q
 *  2 - intersect o-q-p
 *  3 - intersect o-p=q, to handle bbox stacking
 *  4 - intersect o=q-p, to handle bbox stacking
 */
static
rt_cell edge_to_edge(rt_vec4 p0,
                     rt_vec4 p1, rt_vec4 p2, rt_cell pk,
                     rt_vec4 q1, rt_vec4 q2, rt_cell qk)
{
    rt_real d, s, t, u, v;

    /* check if both edges are axis-aligned,
     * "pk, qk" hold world axes indices
     * for respective local K axes */
    if (pk < 3 && qk < 3)
    {
        /* vert_to_face handles
         * this case for bbox_shad */
        if (pk == qk)
        {
            return 0;
        }

        /* determine axis
         * orthogonal to both edges */
        rt_cell mp[3][3] =
        {
            {0, 2, 1},
            {2, 1, 0},
            {1, 0, 2},
        };
        rt_cell kk = mp[pk][qk];

        /* distance from origin to 1st edge
         * in common orthogonal direction */
        d = p1[kk] - p0[kk];

        /* distance from origin to 2nd edge
         * in common orthogonal direction */
        t = q1[kk] - p0[kk];

        /* make sure inequality is multiplied
         * by a positive number, so that relations hold */
        d = t < 0.0f ? -d : +d;
        t = RT_FABS(t);

        /* calculate "u" parameter and test bounds */
        u = (q1[pk] - p0[pk]) * d;

        /* if hit outside with margin, return miss */
        if (u < (RT_MIN(p1[pk], p2[pk]) - p0[pk] - RT_CULL_THRESHOLD) * t
        ||  u > (RT_MAX(p1[pk], p2[pk]) - p0[pk] + RT_CULL_THRESHOLD) * t)
        {
            return 0;
        }

        /* make sure inequality is multiplied
         * by a positive number, so that relations hold */
        t = d < 0.0f ? -t : +t;
        d = RT_FABS(d);

        /* calculate "v" parameter and test bounds */
        v = (p1[qk] - p0[qk]) * t;

        /* if hit outside with margin, return miss */
        if (v < (RT_MIN(q1[qk], q2[qk]) - p0[qk] - RT_CULL_THRESHOLD) * d
        ||  v > (RT_MAX(q1[qk], q2[qk]) - p0[qk] + RT_CULL_THRESHOLD) * d)
        {
            return 0;
        }
    }
    /* otherwise edges are arbitrary */
    else
    {
        rt_vec4 ep, eq, pr, qr, mx, nx;

        /* 1st edge vector */
        RT_VEC3_SUB(ep, p2, p1);

        /* 2nd edge vector */
        RT_VEC3_SUB(eq, q2, q1);

        /* 1st edge origin */
        RT_VEC3_SUB(pr, p1, p0);

        /* 2nd edge origin */
        RT_VEC3_SUB(qr, q1, p0);

        /* cross product of
         * 2nd and 1st edge vectors */
        RT_VEC3_MUL(mx, eq, ep);

        /* cross product of
         * 2nd and 1st edge origins */
        RT_VEC3_MUL(nx, qr, pr);

        /* distance from origin to 2nd edge
         * in common orthogonal direction */
        t = RT_VEC3_DOT(qr, mx);

        /* make sure inequality is multiplied
         * by a positive number, so that relations hold */
        s = t < 0.0f ? -1.0f : +1.0f;
        t = RT_FABS(t);

        /* calculate "u" parameter and test bounds */
        u = RT_VEC3_DOT(eq, nx) * s;

        /* if hit outside with margin, return miss */
        if (u < (0.0f - RT_CULL_THRESHOLD) * t
        ||  u > (1.0f + RT_CULL_THRESHOLD) * t)
        {
            return 0;
        }

        /* apply to "t" the sign of "t" */
        t *= s;

        /* distance from origin to 1st edge
         * in common orthogonal direction */
        d = RT_VEC3_DOT(pr, mx);

        /* make sure inequality is multiplied
         * by a positive number, so that relations hold */
        s = d < 0.0f ? -1.0f : +1.0f;
        d = RT_FABS(d);

        /* calculate "v" parameter and test bounds */
        v = RT_VEC3_DOT(ep, nx) * s;

        /* if hit outside with margin, return miss */
        if (v < (0.0f - RT_CULL_THRESHOLD) * d
        ||  v > (1.0f + RT_CULL_THRESHOLD) * d)
        {
            return 0;
        }

        /* apply to "t" the sign of "d" */
        t *= s;
    }

    /*            | 0 |           | 1 |            */
    /* -----------|-*-|-----------|-*-|----------- */
    /*      0     | 4 |     2     | 3 |     1      */
    return t >  (1.0f + RT_CULL_THRESHOLD) * d ? 1 :
           t >= (1.0f - RT_CULL_THRESHOLD) * d ? 3 :
           t >  (0.0f + RT_CULL_THRESHOLD) * d ? 2 :
           t >= (0.0f - RT_CULL_THRESHOLD) * d ? 4 : 0;
}

/*
 * Determine if there are holes in "srf" not related to "ref"
 * or inside custom clippers accum segments.
 * Holes are either minmax clippers or custom clippers
 * potentially allowing to see "srf" inner side from outside.
 *
 * Return values:
 *  0 - no
 *  1 - yes, minmax only
 *  2 - yes, custom only
 *  3 - yes, both
 */
static
rt_cell surf_hole(rt_SHAPE *srf, rt_SHAPE *ref)
{
    rt_cell c = 0;

    if (srf->tag == RT_TAG_PLANE)
    {
        return c;
    }

    /* check minmax clippers */
    if (srf->cmin[RT_X] != -RT_INF || srf->cmax[RT_X] != +RT_INF
    ||  srf->cmin[RT_Y] != -RT_INF || srf->cmax[RT_Y] != +RT_INF
    ||  srf->cmin[RT_Z] != -RT_INF || srf->cmax[RT_Z] != +RT_INF)
    {
        c |= 1;
    }

    /* init custom clippers list */
    rt_ELEM *elm = (rt_ELEM *)*srf->ptr;

    rt_cell skip = 0;

    /* run through custom clippers list */
    for (; elm != RT_NULL; elm = elm->next)
    {
        rt_BOUND *obj = (rt_BOUND *)elm->temp;

        /* skip accum markers */
        if (obj == RT_NULL)
        {
            skip = 1 - skip;
            continue;
        }

        /* skip trnode elements */
        if (RT_IS_ARRAY(obj))
        {
            continue;
        }

        /* if there is clipper other than "ref"
           or inside accum segment, stop */
        if (obj != ref || skip == 1)
        {
            c |= 2;
            break;
        }
    }

    return c;
}

/*
 * Determine whether surface "clp" outside of any accum segment
 * clips surface "srf" and which "clp" side "srf" is clipped by.
 *
 * Return values:
 *  0 - no, might be inside accum segment
 *  1 - yes, inner
 *  2 - yes, outer
 */
static
rt_cell surf_clip(rt_SHAPE *srf, rt_SHAPE *clp)
{
    rt_cell c = 0;

    /* init custom clippers list */
    rt_ELEM *elm = (rt_ELEM *)*srf->ptr;

    rt_cell skip = 0;

    /* run through custom clippers list */
    for (; elm != RT_NULL; elm = elm->next)
    {
        rt_BOUND *obj = (rt_BOUND *)elm->temp;

        /* skip accum markers */
        if (obj == RT_NULL)
        {
            skip = 1 - skip;
            continue;
        }

        /* skip trnode elements */
        if (RT_IS_ARRAY(obj))
        {
            continue;
        }

        /* if there is clipper "clp"
           outside of accum segment, stop */
        if (obj == clp && skip == 0)
        {
            c = elm->data;
            break;
        }
    }

    /* convert inner/outer
     * from (-1, +1) to (1, 2) notation */
    return c == 0 ? 0 : 1 + ((1 + c) >> 1);
}

/*
 * Determine whether non-clipped "srf" is convex or concave.
 *
 * Return values:
 *  0 - convex
 *  1 - concave
 */
static
rt_cell surf_conc(rt_SHAPE *srf)
{
    rt_cell c = 0;

    if (srf->tag == RT_TAG_CONE
    ||  srf->tag == RT_TAG_HYPERBOLOID)
    {
        c = 1;
    }

    return c;
}

/*
 * Determine whether clipped "srf" is convex or concave.
 *
 * Return values:
 *  0 - convex
 *  1 - concave
 */
static
rt_cell cbox_conc(rt_SHAPE *srf)
{
    rt_cell c = 0;

    rt_vec4  zro = {0.0f, 0.0f, 0.0f, 0.0f};
    rt_real *pps = srf->trnode == srf ? zro : srf->pos;

    if ((srf->tag == RT_TAG_CONE
    ||   srf->tag == RT_TAG_HYPERBOLOID)
    &&  (srf->sci[RT_W] <= 0.0f
    &&   srf->bmin[srf->map[RT_K]] < pps[srf->map[RT_K]]
    &&   srf->bmax[srf->map[RT_K]] > pps[srf->map[RT_K]]
    ||   srf->sci[RT_W] > 0.0f))
    {
        c = 1;
    }

    return c;
}

/*
 * Transform "pos" into "srf" trnode space using "loc"
 * as temporary storage for return value.
 *
 * Return values:
 *  new pos
 */
static
rt_real *surf_tran(rt_BOUND *srf, rt_vec4 pos, rt_vec4 loc)
{
    rt_vec4  dff;
    rt_real *pps = pos;

    if (srf->trnode != RT_NULL)
    {
        RT_VEC3_SUB(dff, pps, srf->trnode->pos);
        dff[RT_W] = 0.0f;

        matrix_mul_vector(loc, *srf->trnode->pinv, dff);

        pps = loc;
    }

    return pps;
}

/*
 * Determine if "pos" is outside "srf" cbox plus margin.
 *
 * Return values:
 *  0 - no
 *  1 - yes
 *  2 - yes, on the border with margin
 */
static
rt_cell surf_cbox(rt_SHAPE *srf, rt_vec4 pos)
{
    rt_cell c = 0;

    /* transform "pos" to "srf" trnode space,
     * where cbox is defined */
    rt_vec4  loc;
    rt_real *pps = surf_tran(srf, pos, loc);

    /* margin is applied to "pps"
     * as cmin/cmax might be infinite */
    if (pps[RT_X] + RT_CULL_THRESHOLD <  srf->cmin[RT_X]
    ||  pps[RT_Y] + RT_CULL_THRESHOLD <  srf->cmin[RT_Y]
    ||  pps[RT_Z] + RT_CULL_THRESHOLD <  srf->cmin[RT_Z]
    ||  pps[RT_X] - RT_CULL_THRESHOLD >  srf->cmax[RT_X]
    ||  pps[RT_Y] - RT_CULL_THRESHOLD >  srf->cmax[RT_Y]
    ||  pps[RT_Z] - RT_CULL_THRESHOLD >  srf->cmax[RT_Z])
    {
        c = 1;
    }
    else
    if (pps[RT_X] - RT_CULL_THRESHOLD <= srf->cmin[RT_X]
    ||  pps[RT_Y] - RT_CULL_THRESHOLD <= srf->cmin[RT_Y]
    ||  pps[RT_Z] - RT_CULL_THRESHOLD <= srf->cmin[RT_Z]
    ||  pps[RT_X] + RT_CULL_THRESHOLD >= srf->cmax[RT_X]
    ||  pps[RT_Y] + RT_CULL_THRESHOLD >= srf->cmax[RT_Y]
    ||  pps[RT_Z] + RT_CULL_THRESHOLD >= srf->cmax[RT_Z])
    {
        c = 2;
    }

    /*    inner   | b |   outer    */
    /* -----------|-*-|----------- */
    /*      0     | 2 |     1      */
    return c;
}

/*
 * Determine if "pos" is inside "srf" bbox minus margin.
 *
 * Return values:
 *  0 - no
 *  1 - yes
 *  2 - yes, on the border with margin
 */
static
rt_cell surf_bbox(rt_BOUND *srf, rt_vec4 pos)
{
    rt_cell c = 0;

    /* transform "pos" to "srf" trnode space,
     * where bbox is defined */
    rt_vec4  loc;
    rt_real *pps = surf_tran(srf, pos, loc);

    /* margin is applied to "pps"
     * for consistency with surf_cbox */
    if (pps[RT_X] - RT_CULL_THRESHOLD >  srf->bmin[RT_X]
    &&  pps[RT_Y] - RT_CULL_THRESHOLD >  srf->bmin[RT_Y]
    &&  pps[RT_Z] - RT_CULL_THRESHOLD >  srf->bmin[RT_Z]
    &&  pps[RT_X] + RT_CULL_THRESHOLD <  srf->bmax[RT_X]
    &&  pps[RT_Y] + RT_CULL_THRESHOLD <  srf->bmax[RT_Y]
    &&  pps[RT_Z] + RT_CULL_THRESHOLD <  srf->bmax[RT_Z])
    {
        c = 1;
    }
    else
    if (pps[RT_X] + RT_CULL_THRESHOLD >= srf->bmin[RT_X]
    &&  pps[RT_Y] + RT_CULL_THRESHOLD >= srf->bmin[RT_Y]
    &&  pps[RT_Z] + RT_CULL_THRESHOLD >= srf->bmin[RT_Z]
    &&  pps[RT_X] - RT_CULL_THRESHOLD <= srf->bmax[RT_X]
    &&  pps[RT_Y] - RT_CULL_THRESHOLD <= srf->bmax[RT_Y]
    &&  pps[RT_Z] - RT_CULL_THRESHOLD <= srf->bmax[RT_Z])
    {
        c = 2;
    }

    /*    inner   | b |   outer    */
    /* -----------|-*-|----------- */
    /*      1     | 2 |     0      */
    return c;
}

/*
 * Determine which side of non-clipped "srf" is seen from "pos".
 *
 * Return values:
 *  0 - none, on the surface with margin
 *  1 - inner
 *  2 - outer
 */
static
rt_cell surf_side(rt_SHAPE *srf, rt_vec4 pos)
{
    rt_vec4  loc;
    rt_real *pps = surf_tran(srf, pos, loc);

    if (srf->trnode != srf)
    {
        RT_VEC3_SUB(loc, pps, srf->pos);
    }

    rt_real d;

    if (srf->tag == RT_TAG_PLANE)
    {
        d = RT_VEC3_DOT(loc, srf->sck);
    }
    else
    {
        rt_real dcj = RT_VEC3_DOT(loc, srf->scj);
        rt_real dci = loc[RT_X] * loc[RT_X] * srf->sci[RT_X]
                    + loc[RT_Y] * loc[RT_Y] * srf->sci[RT_Y]
                    + loc[RT_Z] * loc[RT_Z] * srf->sci[RT_Z];
        d = dci - dcj - srf->sci[RT_W];
    }

    /*    inner   | s |   outer    */
    /* -----------|-*-|----------- */
    /*      1     | 0 |     2      */
    return d >  (0.0f + RT_CULL_THRESHOLD) ? 2 :
           d >= (0.0f - RT_CULL_THRESHOLD) ? 0 : 1;
}

/*
 * Determine which side of clipped "srf" is seen from "pos".
 *
 * Return values:
 *  1 - inner
 *  2 - outer
 *  3 - both, also if on the surface with margin
 */
static
rt_cell cbox_side(rt_SHAPE *srf, rt_vec4 pos)
{
    rt_cell k, c = 0;

    c = surf_side(srf, pos);

    /* if "pos" is on the surface with margin,
     * return both sides */
    if (c == 0)
    {
        c = 3;
        return c;
    }

    k = srf->tag == RT_TAG_PLANE ? 1 : 0;

    /* if "srf" is PLANE,
     * only one side can be seen */
    if (k == 1)
    {
        return c;
    }

    k = surf_conc(srf);

    /* if "srf" is convex and "pos" is inside,
     * only one side can be seen */
    if (k == 0 && c == 1)
    {
        return c;
    }

    k = surf_hole(srf, srf);

    /* check if "srf" has holes */
    if (k == 0)
    {
        return c;
    }
    if (k & 2)
    {
        c = 3;
        return c;
    }

    k = surf_cbox(srf, pos);

    /* check if "pos" is outside of "srf" cbox */
    if (k != 0)
    {
        c = 3;
        return c;
    }

    return c;
}

/*
 * Determine if "nd1" bbox casts shadow on "nd2" bbox as seen from "obj".
 *
 * Return values:
 *  0 - no
 *  1 - yes
 */
rt_cell bbox_shad(rt_BOUND *obj, rt_BOUND *nd1, rt_BOUND *nd2)
{
    /* check if nodes differ and have bounds */
    if (nd1->rad == 0.0f || nd2->rad == 0.0f || nd1 == nd2)
    {
        return 1;
    }

    /* check if cones from bounding spheres don't intersect */
    rt_vec4 nd1_vec;
    RT_VEC3_SUB(nd1_vec, nd1->mid, obj->pos);
    rt_real nd1_len = RT_VEC3_LEN(nd1_vec);

    rt_vec4 nd2_vec;
    RT_VEC3_SUB(nd2_vec, nd2->mid, obj->pos);
    rt_real nd2_len = RT_VEC3_LEN(nd2_vec);

    rt_real dff_ang = RT_VEC3_DOT(nd1_vec, nd2_vec);

    dff_ang = nd1_len <= RT_CULL_THRESHOLD ? 0.0f : dff_ang / nd1_len;
    rt_real nd1_ang = nd1_len >= nd1->rad && nd1_len > RT_CULL_THRESHOLD ?
                        RT_ASIN(nd1->rad / nd1_len) : (rt_real)RT_2_PI;

    dff_ang = nd2_len <= RT_CULL_THRESHOLD ? 0.0f : dff_ang / nd2_len;
    rt_real nd2_ang = nd2_len >= nd2->rad && nd2_len > RT_CULL_THRESHOLD ?
                        RT_ASIN(nd2->rad / nd2_len) : (rt_real)RT_2_PI;

    dff_ang = RT_ACOS(dff_ang);

    if (nd1_ang + nd2_ang < dff_ang)
    {
        return 0;
    }

    /* check if bounding spheres themselves don't intersect */
    rt_vec4 dff_vec;
    RT_VEC3_SUB(dff_vec, nd1->mid, nd2->mid);
    rt_real dff_len = RT_VEC3_LEN(dff_vec);

    if (nd1->rad + nd2->rad < dff_len)
    {
        /* check if shadow bounding sphere is fully behind */
        if (nd1_len > nd2_len)
        {
            return 0;
        }
    }

    /* check if shadow bbox optimization is disabled in runtime */
#if RT_OPTS_SHADOW_EXT1 != 0
    if ((*obj->opts & RT_OPTS_SHADOW_EXT1) == 0)
#endif /* RT_OPTS_SHADOW_EXT1 */
    {
        return 1;
    }

    /* check if "obj" pos is inside "nd1" bbox */
    rt_cell k;

    k = surf_bbox(nd1, obj->pos);

    if (k != 0)
    {
        return 1;
    }

    /* check if bounding boxes cast shadow */
    rt_cell i, j;

    /* run through "nd2" faces and "nd1" verts */
    for (j = 0; j < nd2->faces_num; j++)
    {
        rt_FACE *fc = &nd2->faces[j];

        for (i = 0; i < nd1->verts_num; i++)
        {
            k = vert_to_face(obj->pos, nd1->verts[i].pos,
                             nd2->verts[fc->index[0]].pos,
                             nd2->verts[fc->index[1]].pos,
                             nd2->verts[fc->index[2]].pos,
                             fc->k, fc->i, fc->j);
            if (k == 1)
            {
                return 1;
            }
            if (fc->k < 3)
            {
                continue;
            }
            k = vert_to_face(obj->pos, nd1->verts[i].pos,
                             nd2->verts[fc->index[2]].pos,
                             nd2->verts[fc->index[3]].pos,
                             nd2->verts[fc->index[0]].pos,
                             fc->k, fc->i, fc->j);
            if (k == 1)
            {
                return 1;
            }
        }
    }

    /* run through "nd1" faces and "nd2" verts */
    for (j = 0; j < nd1->faces_num; j++)
    {
        rt_FACE *fc = &nd1->faces[j];

        for (i = 0; i < nd2->verts_num; i++)
        {
            k = vert_to_face(obj->pos, nd2->verts[i].pos,
                             nd1->verts[fc->index[0]].pos,
                             nd1->verts[fc->index[1]].pos,
                             nd1->verts[fc->index[2]].pos,
                             fc->k, fc->i, fc->j);
            if (k == 2 || k == 4)
            {
                return 1;
            }
            if (fc->k < 3)
            {
                continue;
            }
            k = vert_to_face(obj->pos, nd2->verts[i].pos,
                             nd1->verts[fc->index[2]].pos,
                             nd1->verts[fc->index[3]].pos,
                             nd1->verts[fc->index[0]].pos,
                             fc->k, fc->i, fc->j);
            if (k == 2 || k == 4)
            {
                return 1;
            }
        }
    }

    /* run through "nd2" edges and "nd1" edges */
    for (j = 0; j < nd2->edges_num; j++)
    {
        rt_EDGE *ej = &nd2->edges[j];

        for (i = 0; i < nd1->edges_num; i++)
        {
            rt_EDGE *ei = &nd1->edges[i];

            k = edge_to_edge(obj->pos,
                             nd1->verts[ei->index[0]].pos,
                             nd1->verts[ei->index[1]].pos, ei->k,
                             nd2->verts[ej->index[0]].pos,
                             nd2->verts[ej->index[1]].pos, ej->k);
            if (k == 1)
            {
                return 1;
            }
        }
    }

    return 0;
}

/*
 * Determine the order of "nd1" and "nd2" bboxes as seen from "obj".
 *
 * Return values:
 *  1 - neutral
 *  2 - unsortable
 *  3 - don't swap
 *  4 - do swap, not part of the stored order value in the engine
 */
rt_cell bbox_sort(rt_BOUND *obj, rt_BOUND *nd1, rt_BOUND *nd2)
{
    /* check if nodes differ and have bounds */
    if (nd1->rad == 0.0f || nd2->rad == 0.0f || nd1 == nd2)
    {
        return 2;
    }

    /* check if cones from bounding spheres don't intersect */
    rt_vec4 nd1_vec;
    RT_VEC3_SUB(nd1_vec, nd1->mid, obj->pos);
    rt_real nd1_len = RT_VEC3_LEN(nd1_vec);

    rt_vec4 nd2_vec;
    RT_VEC3_SUB(nd2_vec, nd2->mid, obj->pos);
    rt_real nd2_len = RT_VEC3_LEN(nd2_vec);

    rt_real dff_ang = RT_VEC3_DOT(nd1_vec, nd2_vec);

    dff_ang = nd1_len <= RT_CULL_THRESHOLD ? 0.0f : dff_ang / nd1_len;
    rt_real nd1_ang = nd1_len >= nd1->rad && nd1_len > RT_CULL_THRESHOLD ?
                        RT_ASIN(nd1->rad / nd1_len) : (rt_real)RT_2_PI;

    dff_ang = nd2_len <= RT_CULL_THRESHOLD ? 0.0f : dff_ang / nd2_len;
    rt_real nd2_ang = nd2_len >= nd2->rad && nd2_len > RT_CULL_THRESHOLD ?
                        RT_ASIN(nd2->rad / nd2_len) : (rt_real)RT_2_PI;

    dff_ang = RT_ACOS(dff_ang);

    if (nd1_ang + nd2_ang < dff_ang)
    {
        return 1;
    }

    /* check if bounding spheres themselves don't intersect */
    rt_vec4 dff_vec;
    RT_VEC3_SUB(dff_vec, nd1->mid, nd2->mid);
    rt_real dff_len = RT_VEC3_LEN(dff_vec);

    if (nd1->rad + nd2->rad < dff_len)
    {
        /* check the order for bounding spheres */
        if (nd1_len < nd2_len)
        {
            return 3;
        }
        else
        {
            return 4;
        }
    }

    /* check if bbox sorting optimization is disabled in runtime */
#if RT_OPTS_INSERT_EXT1 != 0
    if ((*obj->opts & RT_OPTS_INSERT_EXT1) == 0)
#endif /* RT_OPTS_INSERT_EXT1 */
    {
        return 2;
    }

    /* check the order for bounding boxes */
    rt_cell i, j, k, c = 0;

    /* run through "nd2" faces and "nd1" verts */
    for (j = 0; j < nd2->faces_num; j++)
    {
        rt_FACE *fc = &nd2->faces[j];

        for (i = 0; i < nd1->verts_num; i++)
        {
            k = vert_to_face(obj->pos, nd1->verts[i].pos,
                             nd2->verts[fc->index[0]].pos,
                             nd2->verts[fc->index[1]].pos,
                             nd2->verts[fc->index[2]].pos,
                             fc->k, fc->i, fc->j);
            if (k == 4)
            {
                k =  2;
            }
            if (k == 1 || k == 2)
            {
                if (c == 0)
                {
                    c =  k;
                }
                else
                if (c != k)
                {
                    return 2;
                }
            }
            if (fc->k < 3)
            {
                continue;
            }
            k = vert_to_face(obj->pos, nd1->verts[i].pos,
                             nd2->verts[fc->index[2]].pos,
                             nd2->verts[fc->index[3]].pos,
                             nd2->verts[fc->index[0]].pos,
                             fc->k, fc->i, fc->j);
            if (k == 4)
            {
                k =  2;
            }
            if (k == 1 || k == 2)
            {
                if (c == 0)
                {
                    c =  k;
                }
                else
                if (c != k)
                {
                    return 2;
                }
            }
        }
    }

    /* run through "nd1" faces and "nd2" verts */
    for (j = 0; j < nd1->faces_num; j++)
    {
        rt_FACE *fc = &nd1->faces[j];

        for (i = 0; i < nd2->verts_num; i++)
        {
            k = vert_to_face(obj->pos, nd2->verts[i].pos,
                             nd1->verts[fc->index[0]].pos,
                             nd1->verts[fc->index[1]].pos,
                             nd1->verts[fc->index[2]].pos,
                             fc->k, fc->i, fc->j);
            if (k == 4)
            {
                k =  2;
            }
            k ^= 3;
            if (k == 1 || k == 2)
            {
                if (c == 0)
                {
                    c =  k;
                }
                else
                if (c != k)
                {
                    return 2;
                }
            }
            if (fc->k < 3)
            {
                continue;
            }
            k = vert_to_face(obj->pos, nd2->verts[i].pos,
                             nd1->verts[fc->index[2]].pos,
                             nd1->verts[fc->index[3]].pos,
                             nd1->verts[fc->index[0]].pos,
                             fc->k, fc->i, fc->j);
            if (k == 4)
            {
                k =  2;
            }
            k ^= 3;
            if (k == 1 || k == 2)
            {
                if (c == 0)
                {
                    c =  k;
                }
                else
                if (c != k)
                {
                    return 2;
                }
            }
        }
    }

    /* run through "nd2" edges and "nd1" edges */
    for (j = 0; j < nd2->edges_num; j++)
    {
        rt_EDGE *ej = &nd2->edges[j];

        for (i = 0; i < nd1->edges_num; i++)
        {
            rt_EDGE *ei = &nd1->edges[i];

            k = edge_to_edge(obj->pos,
                             nd1->verts[ei->index[0]].pos,
                             nd1->verts[ei->index[1]].pos, ei->k,
                             nd2->verts[ej->index[0]].pos,
                             nd2->verts[ej->index[1]].pos, ej->k);
            if (k == 4)
            {
                k =  2;
            }
            if (k == 1 || k == 2)
            {
                if (c == 0)
                {
                    c =  k;
                }
                else
                if (c != k)
                {
                    return 2;
                }
            }
        }
    }

    return c == 0 ? 1 : c + 2;
}

/*
 * Determine if "nd1" and "nd2" bboxes intersect.
 *
 * Return values:
 *  0 - no
 *  1 - yes, quick - might be fully inside
 *  2 - yes, thorough - borders intersect
 */
static
rt_cell bbox_fuse(rt_BOUND *nd1, rt_BOUND *nd2)
{
    /* check if nodes differ and have bounds */
    if (nd1->rad == 0.0f || nd2->rad == 0.0f || nd1 == nd2)
    {
        return 2;
    }

    /* check if bounding spheres don't intersect */
    rt_vec4 dff_vec;
    RT_VEC3_SUB(dff_vec, nd1->mid, nd2->mid);
    rt_real dff_len = RT_VEC3_LEN(dff_vec);

    if (nd1->rad + nd2->rad < dff_len)
    {
        return 0;
    }

    /* check if one bbox's mid is inside another */
    rt_cell k;

    k = surf_bbox(nd1, nd2->mid);

    if (k != 0)
    {
        return 1;
    }

    k = surf_bbox(nd2, nd1->mid);

    if (k != 0)
    {
        return 1;
    }

    /* check if edges of one bbox intersect faces of another */
    rt_cell i, j;

    /* run through "nd2" faces and "nd1" edges */
    for (j = 0; j < nd2->faces_num; j++)
    {
        rt_FACE *fc = &nd2->faces[j];

        for (i = 0; i < nd1->edges_num; i++)
        {
            rt_EDGE *ei = &nd1->edges[i];

            k = vert_to_face(nd1->verts[ei->index[0]].pos,
                             nd1->verts[ei->index[1]].pos,
                             nd2->verts[fc->index[0]].pos,
                             nd2->verts[fc->index[1]].pos,
                             nd2->verts[fc->index[2]].pos,
                             fc->k, fc->i, fc->j);
            if (k == 2)
            {
                return 2;
            }
            if (fc->k < 3)
            {
                continue;
            }
            k = vert_to_face(nd1->verts[ei->index[0]].pos,
                             nd1->verts[ei->index[1]].pos,
                             nd2->verts[fc->index[2]].pos,
                             nd2->verts[fc->index[3]].pos,
                             nd2->verts[fc->index[0]].pos,
                             fc->k, fc->i, fc->j);
            if (k == 2)
            {
                return 2;
            }
        }
    }

    /* run through "nd1" faces and "nd2" edges */
    for (j = 0; j < nd1->faces_num; j++)
    {
        rt_FACE *fc = &nd1->faces[j];

        for (i = 0; i < nd2->edges_num; i++)
        {
            rt_EDGE *ei = &nd2->edges[i];

            k = vert_to_face(nd2->verts[ei->index[0]].pos,
                             nd2->verts[ei->index[1]].pos,
                             nd1->verts[fc->index[0]].pos,
                             nd1->verts[fc->index[1]].pos,
                             nd1->verts[fc->index[2]].pos,
                             fc->k, fc->i, fc->j);
            if (k == 2)
            {
                return 2;
            }
            if (fc->k < 3)
            {
                continue;
            }
            k = vert_to_face(nd2->verts[ei->index[0]].pos,
                             nd2->verts[ei->index[1]].pos,
                             nd1->verts[fc->index[2]].pos,
                             nd1->verts[fc->index[3]].pos,
                             nd1->verts[fc->index[0]].pos,
                             fc->k, fc->i, fc->j);
            if (k == 2)
            {
                return 2;
            }
        }
    }

    return 0;
}

/*
 * Determine which side of clipped "srf" is seen from "obj".
 *
 * Return values:
 *  0 - none, if both surfaces are the same plane
 *  1 - inner
 *  2 - outer
 *  3 - both, also if on the surface with margin
 */
rt_cell bbox_side(rt_BOUND *obj, rt_SHAPE *srf)
{
    /* check if "obj" is CAMERA or LIGHT */
    if (RT_IS_LIGHT(obj) || RT_IS_CAMERA(obj))
    {
        return cbox_side(srf, obj->pos);
    }

    rt_cell i, j, k, m, n, p, c = 0;

    p = srf->tag == RT_TAG_PLANE ? 1 : 0;

    /* check if "obj" is SURFACE */
    if (RT_IS_SURFACE(obj))
    {
        rt_SHAPE *ref = (rt_SHAPE *)obj;

        /* check if surfaces are the same */
        if (srf == ref)
        {
            if (p == 0)
            {
                m = cbox_conc(srf);
                c |= 1;
                if (m == 1)
                {
                    c |= 2;
                }
            }
            return c;
        }

        /* check "srf" and "ref" clip relationship */
        i = surf_clip(ref, srf);
        j = surf_clip(srf, ref);

        k = surf_hole(srf, ref);

        m = cbox_conc(srf);
        n = cbox_conc(ref);

        if (i == 2 && j == 2
        ||  i == 2 && j == 0)
        {
            c |= 1;
            if (m == 1 && k != 0)
            {
                c |= 2;
            }
            return c;
        }
        if (i == 2 && j == 1)
        {
            c |= 1;
            if (m == 1)
            {
                c |= 2;
            }
            return c;
        }
        if (i == 1 && j == 2)
        {
            c |= 2;
            if (n == 1 && p == 0 || k != 0)
            {
                c |= 1;
            }
            return c;
        }
        if (i == 1 && j == 1)
        {
            c |= 2;
            if (p == 0)
            {
                c |= 1;
            }
            return c;
        }
        if (i == 1 && j == 0)
        {
            c |= 2;
            if (k != 0)
            {
                c |= 1;
            }
            return c;
        }
        if (i == 0 && j == 2
        ||  i == 0 && j == 1)
        {
            c |= 3;
            return c;
        }
    }

    /* check if all "obj" verts are on the same side */
    if (p == 1)
    {
        if (obj->verts_num == 0)
        {
            c |= 3;
        }
        for (i = 0; i < obj->verts_num; i++)
        {
            c |= surf_side(srf, obj->verts[i].pos);
            if (c == 3)
            {
                break;
            }
        }
        return c;
    }

    /* check if bboxes intersect */
    n = bbox_fuse(srf, obj);

    if (n != 0 && m == 1 || n == 2)
    {
        c |= 3;
        return c;
    }

    /* check if all "obj" verts are inside "srf" */
    if (n == 1 && m == 0)
    {
        c |= 1;
        for (i = 0; i < obj->verts_num; i++)
        {
            n = surf_side(srf, obj->verts[i].pos);
            if (n == 2)
            {
                c |= 2;
                break;
            }
        }
        return c;
    }

    /* check if "srf" has holes */
    if (k == 0)
    {
        c |= 2;
        return c;
    }
    if (k & 2)
    {
        c |= 3;
        return c;
    }

    /* check if all "obj" verts are inside "srf" cbox */
    if (k == 1)
    {
        c |= 2;
        for (i = 0; i < obj->verts_num; i++)
        {
            k = surf_cbox(srf, obj->verts[i].pos);
            if (k != 0)
            {
                c |= 1;
                break;
            }
        }
    }

    return c;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
