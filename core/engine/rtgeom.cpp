/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <string.h>

#include "rtgeom.h"
#include "system.h"

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
 * False-positives are allowed due to computational inaccuracy.
 *
 * Return values:
 *  0 - don't intersect
 *  1 - intersect o-p-q
 *  2 - intersect o-q-p
 *  3 - intersect o-p=q (to handle bbox stacking)
 *  4 - intersect o=q-p (to handle bbox stacking)
 */
static
rt_cell vert_to_face(rt_vec4 p0, rt_vec4 p1,
                     rt_vec4 q0, rt_vec4 q1, rt_vec4 q2,
                     rt_cell qk, rt_cell qi, rt_cell qj)
{
    rt_real d, s, t, u, v;

    if (qk < 3 && qi < 3 && qj < 3)
    {
        /* distance to vert
         * in face's normal direction */
        d = p1[qk] - p0[qk];

        /* distance to face's origin
         * in face's normal direction */
        t = q0[qk] - p0[qk];

        /* make sure inequality is multiplied
         * by a positive number, so that relations hold */
        t = d < 0.0f ? -t : +t;
        d = RT_FABS(d);

        /* calculate u parameter and test bounds */
        u = (p1[qi] - p0[qi]) * t;

        /* if hit outside with margin,
         * return miss */
        if (u < (RT_MIN(q0[qi], q1[qi]) - p0[qi] - RT_CULL_THRESHOLD) * d
        ||  u > (RT_MAX(q0[qi], q1[qi]) - p0[qi] + RT_CULL_THRESHOLD) * d)
        {
            return 0;
        }

        /* calculate v parameter and test bounds */
        v = (p1[qj] - p0[qj]) * t;

        /* if hit outside with margin,
         * return miss */
        if (v < (RT_MIN(q1[qj], q2[qj]) - p0[qj] - RT_CULL_THRESHOLD) * d
        ||  v > (RT_MAX(q1[qj], q2[qj]) - p0[qj] + RT_CULL_THRESHOLD) * d)
        {
            return 0;
        }
    }
    else
    {
        rt_vec4 e1, e2, pr, qr, mx, nx;

        /* direction of the face's
         * 1st edge vector */
        e1[RT_X] = q1[RT_X] - q0[RT_X];
        e1[RT_Y] = q1[RT_Y] - q0[RT_Y];
        e1[RT_Z] = q1[RT_Z] - q0[RT_Z];

        /* direction of the face's
         * 2nd edge vector */
        e2[RT_X] = q2[RT_X] - q0[RT_X];
        e2[RT_Y] = q2[RT_Y] - q0[RT_Y];
        e2[RT_Z] = q2[RT_Z] - q0[RT_Z];

        /* direction of the ray vector */
        pr[RT_X] = p1[RT_X] - p0[RT_X];
        pr[RT_Y] = p1[RT_Y] - p0[RT_Y];
        pr[RT_Z] = p1[RT_Z] - p0[RT_Z];

        /* direction of the ray origin
         * from face's origin */
        qr[RT_X] = p0[RT_X] - q0[RT_X];
        qr[RT_Y] = p0[RT_Y] - q0[RT_Y];
        qr[RT_Z] = p0[RT_Z] - q0[RT_Z];

        /* cross product of the ray vector
         * and face's 2nd edge */
        RT_VECTOR_CROSS(mx, pr, e2);

        /* calculate determinant */
        d = RT_VECTOR_DOT(e1, mx);

        /* make sure inequality is multiplied
         * by a positive number, so that relations hold */
        s = d < 0.0f ? -1.0f : +1.0f;
        d = RT_FABS(d);

        /* calculate u parameter and test bounds */
        u = RT_VECTOR_DOT(qr, mx) * s;

        /* if hit outside with margin,
         * return miss */
        if (u < (0.0f - RT_CULL_THRESHOLD) * d
        ||  u > (1.0f + RT_CULL_THRESHOLD) * d)
        {
            return 0;
        }

        /* cross product of the ray origin
         * and face's 1st edge */
        RT_VECTOR_CROSS(nx, qr, e1);

        /* calculate v parameter and test bounds */
        v = RT_VECTOR_DOT(pr, nx) * s;

        /* if hit outside with margin,
         * return miss */
        if (v < (0.0f - RT_CULL_THRESHOLD) * d
        ||  v > (1.0f + RT_CULL_THRESHOLD) * d - u)
        {
            return 0;
        }

        /* calculate t,
         * analog of distance to intersection */
        t = RT_VECTOR_DOT(e2, nx) * s;
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
 * False-positives are allowed due to computational inaccuracy.
 *
 * Return values:
 *  0 - don't intersect
 *  1 - intersect o-p-q
 *  2 - intersect o-q-p
 *  3 - intersect o-p=q (to handle bbox stacking)
 *  4 - intersect o=q-p (to handle bbox stacking)
 */
static
rt_cell edge_to_edge(rt_vec4 p0,
                     rt_vec4 p1, rt_vec4 p2, rt_cell pk,
                     rt_vec4 q1, rt_vec4 q2, rt_cell qk)
{
    rt_real d, s, t, u, v;

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

        /* distance to 1st edge's origin
         * in common orthogonal direction */
        d = p1[kk] - p0[kk];

        /* distance to 2nd edge's origin
         * in common orthogonal direction */
        t = q1[kk] - p0[kk];

        /* make sure inequality is multiplied
         * by a positive number, so that relations hold */
        d = t < 0.0f ? -d : +d;
        t = RT_FABS(t);

        /* calculate u parameter and test bounds */
        u = (q1[pk] - p0[pk]) * d;

        /* if hit outside with margin,
         * return miss */
        if (u < (RT_MIN(p1[pk], p2[pk]) - p0[pk] - RT_CULL_THRESHOLD) * t
        ||  u > (RT_MAX(p1[pk], p2[pk]) - p0[pk] + RT_CULL_THRESHOLD) * t)
        {
            return 0;
        }

        /* make sure inequality is multiplied
         * by a positive number, so that relations hold */
        t = d < 0.0f ? -t : +t;
        d = RT_FABS(d);

        /* calculate v parameter and test bounds */
        v = (p1[qk] - p0[qk]) * t;

        /* if hit outside with margin,
         * return miss */
        if (v < (RT_MIN(q1[qk], q2[qk]) - p0[qk] - RT_CULL_THRESHOLD) * d
        ||  v > (RT_MAX(q1[qk], q2[qk]) - p0[qk] + RT_CULL_THRESHOLD) * d)
        {
            return 0;
        }
    }
    else
    {
        rt_vec4 ep, eq, pr, qr, mx, nx;

        /* direction of the 1st edge vector */
        ep[RT_X] = p2[RT_X] - p1[RT_X];
        ep[RT_Y] = p2[RT_Y] - p1[RT_Y];
        ep[RT_Z] = p2[RT_Z] - p1[RT_Z];

        /* direction of the 2nd edge vector */
        eq[RT_X] = q2[RT_X] - q1[RT_X];
        eq[RT_Y] = q2[RT_Y] - q1[RT_Y];
        eq[RT_Z] = q2[RT_Z] - q1[RT_Z];

        /* direction of the 1st edge origin */
        pr[RT_X] = p1[RT_X] - p0[RT_X];
        pr[RT_Y] = p1[RT_Y] - p0[RT_Y];
        pr[RT_Z] = p1[RT_Z] - p0[RT_Z];

        /* direction of the 2nd edge origin */
        qr[RT_X] = q1[RT_X] - p0[RT_X];
        qr[RT_Y] = q1[RT_Y] - p0[RT_Y];
        qr[RT_Z] = q1[RT_Z] - p0[RT_Z];

        /* cross product of two edge vectors */
        RT_VECTOR_CROSS(mx, eq, ep);

        /* cross product of two edge origins */
        RT_VECTOR_CROSS(nx, qr, pr);

        /* projection of 1st edge's origin
         * in common orthogonal direction */
        d = RT_VECTOR_DOT(pr, mx);

        /* make sure inequality is multiplied
         * by a positive number, so that relations hold */
        s = d < 0.0f ? -1.0f : +1.0f;
        d = RT_FABS(d);

        /* calculate v parameter and test bounds */
        v = RT_VECTOR_DOT(ep, nx) * s;

        /* if hit outside with margin,
         * return miss */
        if (v < (0.0f - RT_CULL_THRESHOLD) * d
        ||  v > (1.0f + RT_CULL_THRESHOLD) * d)
        {
            return 0;
        }

        v = s;

        /* projection of 2nd edge's origin
         * in common orthogonal direction */
        t = RT_VECTOR_DOT(qr, mx);

        /* make sure inequality is multiplied
         * by a positive number, so that relations hold */
        s = t < 0.0f ? -1.0f : +1.0f;
        t = RT_FABS(t);

        /* calculate u parameter and test bounds */
        u = RT_VECTOR_DOT(eq, nx) * s;

        /* if hit outside with margin,
         * return miss */
        if (u < (0.0f - RT_CULL_THRESHOLD) * t
        ||  u > (1.0f + RT_CULL_THRESHOLD) * t)
        {
            return 0;
        }

        t *= v * s;
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
rt_cell surf_hole(rt_Surface *srf, rt_Surface *ref)
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

    rt_cell skip = 0;

    rt_ELEM *elm = (rt_ELEM *)srf->s_srf->msc_p[2];

    /* run through custom clippers list */
    for (; elm != RT_NULL; elm = elm->next)
    {
        rt_Object *obj = (rt_Object *)elm->temp;

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
 * Determine which side of clipper "clp" outside of any accum segment
 * surface "srf" is clipped by.
 *
 * Return values:
 *  0 - not clipped or "clp" inside accum segment
 *  1 - clipped by "clp" inner side
 *  2 - clipped by "clp" outer side
 */
static
rt_cell surf_clip(rt_Surface *srf, rt_Surface *clp)
{
    rt_cell c = 0;

    /* init custom clippers list */
    rt_ELEM *elm = (rt_ELEM *)srf->s_srf->msc_p[2];

    rt_cell skip = 0;

    /* run through custom clippers list */
    for (; elm != RT_NULL; elm = elm->next)
    {
        rt_Object *obj = (rt_Object *)elm->temp;

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
rt_cell surf_conc(rt_Surface *srf)
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
rt_cell cbox_conc(rt_Surface *srf)
{
    rt_cell c = 0;

    rt_vec4  zro = {0.0f, 0.0f, 0.0f, 0.0f};
    rt_real *pps = srf->trnode == srf ? zro : srf->pos;

    if ((srf->tag == RT_TAG_CONE
    ||   srf->tag == RT_TAG_HYPERBOLOID)
    &&  (srf->sci[RT_W] <= 0.0f
    &&   srf->bmin[srf->mp_k] < pps[srf->mp_k]
    &&   srf->bmax[srf->mp_k] > pps[srf->mp_k]
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
rt_real *surf_tran(rt_vec4 loc, rt_vec4 pos, rt_Surface *srf)
{
    rt_vec4  dff;
    rt_real *pps = pos;

    if (srf->trnode != RT_NULL)
    {
        dff[RT_X] = pps[RT_X] - srf->trnode->pos[RT_X];
        dff[RT_Y] = pps[RT_Y] - srf->trnode->pos[RT_Y];
        dff[RT_Z] = pps[RT_Z] - srf->trnode->pos[RT_Z];
        dff[RT_W] = 0.0f;

        matrix_mul_vector(loc, srf->trnode->inv, dff);

        pps = loc;
    }

    return pps;
}

/*
 * Determine if "pos" is outside of "srf" cbox minus margin.
 *
 * Return values:
 *  0 - no
 *  1 - yes
 */
static
rt_cell surf_cbox(rt_vec4 pos, rt_Surface *srf)
{
    rt_cell c = 0;

    /* transform "pos" to "srf" trnode space,
     * where cbox is defined */
    rt_vec4  loc;
    rt_real *pps = surf_tran(loc, pos, srf);

    /* margin is applied to "pps"
     * as cmin/cmax might be infinite */
    if (pps[RT_X] - RT_CULL_THRESHOLD <= srf->cmin[RT_X]
    ||  pps[RT_X] + RT_CULL_THRESHOLD >= srf->cmax[RT_X]
    ||  pps[RT_Y] - RT_CULL_THRESHOLD <= srf->cmin[RT_Y]
    ||  pps[RT_Y] + RT_CULL_THRESHOLD >= srf->cmax[RT_Y]
    ||  pps[RT_Z] - RT_CULL_THRESHOLD <= srf->cmin[RT_Z]
    ||  pps[RT_Z] + RT_CULL_THRESHOLD >= srf->cmax[RT_Z])
    {
        c = 1;
    }

    return c;
}

/*
 * Determine if "pos" is inside of "srf" bbox plus margin.
 *
 * Return values:
 *  0 - no
 *  1 - yes
 */
static
rt_cell surf_bbox(rt_vec4 pos, rt_Surface *srf)
{
    rt_cell c = 0;

    /* transform "pos" to "srf" trnode space,
     * where bbox is defined */
    rt_vec4  loc;
    rt_real *pps = surf_tran(loc, pos, srf);

    /* margin is applied to "pps"
     * for consistency with surf_cbox */
    if (pps[RT_X] + RT_CULL_THRESHOLD >= srf->bmin[RT_X]
    &&  pps[RT_X] - RT_CULL_THRESHOLD <= srf->bmax[RT_X]
    &&  pps[RT_Y] + RT_CULL_THRESHOLD >= srf->bmin[RT_Y]
    &&  pps[RT_Y] - RT_CULL_THRESHOLD <= srf->bmax[RT_Y]
    &&  pps[RT_Z] + RT_CULL_THRESHOLD >= srf->bmin[RT_Z]
    &&  pps[RT_Z] - RT_CULL_THRESHOLD <= srf->bmax[RT_Z])
    {
        c = 1;
    }

    return c;
}

/*
 * Determine which side of non-clipped "srf" is seen from "pos".
 *
 * Return values:
 *  0 - none (on the surface with margin)
 *  1 - inner
 *  2 - outer
 */
static
rt_cell surf_side(rt_vec4 pos, rt_Surface *srf)
{
    rt_vec4  loc;
    rt_real *pps = surf_tran(loc, pos, srf);

    if (srf->trnode != srf)
    {
        loc[RT_X] = pps[RT_X] - srf->pos[RT_X];
        loc[RT_Y] = pps[RT_Y] - srf->pos[RT_Y];
        loc[RT_Z] = pps[RT_Z] - srf->pos[RT_Z];
    }

    rt_real d;

    if (srf->tag == RT_TAG_PLANE)
    {
        d = RT_VECTOR_DOT(loc, srf->sck);
    }
    else
    {
        rt_real dcj = RT_VECTOR_DOT(loc, srf->scj);
        rt_real dci = loc[RT_X] * loc[RT_X] * srf->sci[RT_X]
                    + loc[RT_Y] * loc[RT_Y] * srf->sci[RT_Y]
                    + loc[RT_Z] * loc[RT_Z] * srf->sci[RT_Z];
        d = dci - dcj - srf->sci[RT_W];
    }

    /*            | 0 |            */
    /* -----------|-*-|----------- */
    /*      1     | 0 |     2      */
    return d >  (0.0f + RT_CULL_THRESHOLD) ? 2 :
           d >= (0.0f - RT_CULL_THRESHOLD) ? 0 : 1;
}

/*
 * Determine which side of clipped "srf" is seen from "pos".
 *
 * Return values:
 *  0 - none (on the surface with margin)
 *  1 - inner
 *  2 - outer
 *  3 - both
 */
rt_cell cbox_side(rt_real *pos, rt_Surface *srf)
{
    rt_cell side = surf_side(pos, srf);

    if (srf->tag == RT_TAG_PLANE)
    {
        return side;
    }

    rt_cell conc = surf_conc(srf);

    if (conc == 0 && side == 1)
    {
        return side;
    }

    rt_cell hole = surf_hole(srf, srf);

    if (hole == 0)
    {
        return side;
    }

    if (hole & 2)
    {
        side = 3;
        return side;
    }

    rt_cell cbox = surf_cbox(pos, srf);

    if (cbox == 1)
    {
        side = 3;
        return side;
    }

    return side;
}

/*
 * Determine if "shw" bbox casts shadow on "srf" bbox from "lgt" pos.
 *
 * Return values:
 *  0 - no
 *  1 - yes
 */
rt_cell bbox_shad(rt_Light *lgt, rt_Surface *shw, rt_Surface *srf)
{
    /* check if surfaces differ and have bounds */
    if (srf->verts_num == 0 || shw->verts_num == 0 || srf == shw)
    {
        return 1;
    }

    /* check first if bounding spheres cast shadows */
    rt_vec4 shw_vec;

    shw_vec[RT_X] = shw->mid[RT_X] - lgt->pos[RT_X];
    shw_vec[RT_Y] = shw->mid[RT_Y] - lgt->pos[RT_Y];
    shw_vec[RT_Z] = shw->mid[RT_Z] - lgt->pos[RT_Z];
    shw_vec[RT_W] = 0.0f;

    rt_vec4 srf_vec;

    srf_vec[RT_X] = srf->mid[RT_X] - lgt->pos[RT_X];
    srf_vec[RT_Y] = srf->mid[RT_Y] - lgt->pos[RT_Y];
    srf_vec[RT_Z] = srf->mid[RT_Z] - lgt->pos[RT_Z];
    srf_vec[RT_W] = 0.0f;

    rt_real ang = RT_VECTOR_DOT(shw_vec, srf_vec);

    rt_real f = 0.0f, len = 0.0f;

    f = shw_vec[RT_X];
    len += f * f;
    f = shw_vec[RT_Y];
    len += f * f;
    f = shw_vec[RT_Z];
    len += f * f;

    len = RT_SQRT(len);
    ang = len <= RT_CULL_THRESHOLD ? 0.0f : ang / len;

    rt_real shw_ang = len >= shw->rad ?
                        RT_ASIN(shw->rad / len) : (rt_real)RT_2_PI;

    f = len = 0.0f;

    f = srf_vec[RT_X];
    len += f * f;
    f = srf_vec[RT_Y];
    len += f * f;
    f = srf_vec[RT_Z];
    len += f * f;

    len = RT_SQRT(len);
    ang = len <= RT_CULL_THRESHOLD ? 0.0f : ang / len;

    rt_real srf_ang = len >= srf->rad ?
                        RT_ASIN(srf->rad / len) : (rt_real)RT_2_PI;

    ang = RT_ACOS(ang);

    if (shw_ang + srf_ang < ang)
    {
        return 0;
    }

    /* check if shadow bbox optimization is disabled in runtime */
#if RT_OPTS_SHADOW_EXT1 != 0
    if ((lgt->rg->opts & RT_OPTS_SHADOW_EXT1) == 0)
#endif /* RT_OPTS_SHADOW_EXT1 */
    {
        return 1;
    }

    /* check if "lgt" pos is inside "shw" bbox */
    if (surf_bbox(lgt->pos, shw) == 1)
    {
        return 1;
    }

    /* check if bounding boxes cast shadows */
    rt_cell i, j;

    for (j = 0; j < srf->faces_num; j++)
    {
        rt_FACE *fc = &srf->faces[j];

        for (i = 0; i < shw->verts_num; i++)
        {
            if (vert_to_face(lgt->pos, shw->verts[i].pos,
                             srf->verts[fc->index[0]].pos,
                             srf->verts[fc->index[1]].pos,
                             srf->verts[fc->index[2]].pos,
                             fc->k, fc->i, fc->j) == 1)
            {
                return 1;
            }
            if (fc->k < 3)
            {
                continue;
            }
            if (vert_to_face(lgt->pos, shw->verts[i].pos,
                             srf->verts[fc->index[2]].pos,
                             srf->verts[fc->index[3]].pos,
                             srf->verts[fc->index[0]].pos,
                             fc->k, fc->i, fc->j) == 1)
            {
                return 1;
            }
        }
    }

    for (j = 0; j < shw->faces_num; j++)
    {
        rt_FACE *fc = &shw->faces[j];

        for (i = 0; i < srf->verts_num; i++)
        {
            if (vert_to_face(lgt->pos, srf->verts[i].pos,
                             shw->verts[fc->index[0]].pos,
                             shw->verts[fc->index[1]].pos,
                             shw->verts[fc->index[2]].pos,
                             fc->k, fc->i, fc->j) == 2)
            {
                return 1;
            }
            if (fc->k < 3)
            {
                continue;
            }
            if (vert_to_face(lgt->pos, srf->verts[i].pos,
                             shw->verts[fc->index[2]].pos,
                             shw->verts[fc->index[3]].pos,
                             shw->verts[fc->index[0]].pos,
                             fc->k, fc->i, fc->j) == 2)
            {
                return 1;
            }
        }
    }

    for (j = 0; j < srf->edges_num; j++)
    {
        rt_EDGE *ej = &srf->edges[j];

        for (i = 0; i < shw->edges_num; i++)
        {
            rt_EDGE *ei = &shw->edges[i];

            if (edge_to_edge(lgt->pos,
                             shw->verts[ei->index[0]].pos,
                             shw->verts[ei->index[1]].pos, ei->k,
                             srf->verts[ej->index[0]].pos,
                             srf->verts[ej->index[1]].pos, ej->k) == 1)
            {
                return 1;
            }
        }
    }

    return 0;
}

/*
 * Determine if two bboxes interpenetrate.
 *
 * Return values:
 *  0 - no
 *  1 - yes (quick - might be fully inside)
 *  2 - yes (thorough - borders intersect)
 */
static
rt_cell bbox_fuse(rt_Surface *srf, rt_Surface *ref)
{
    rt_cell i, j;

    if (srf->verts_num == 0 || ref->verts_num == 0 || srf == ref)
    {
        return 2;
    }

    /* check first if bounding spheres interpenetrate */
    rt_real f = 0.0f, len = 0.0f;

    f = srf->mid[RT_X] - ref->mid[RT_X];
    len += f * f;
    f = srf->mid[RT_Y] - ref->mid[RT_Y];
    len += f * f;
    f = srf->mid[RT_Z] - ref->mid[RT_Z];
    len += f * f;

    len = RT_SQRT(len);

    if (srf->rad + ref->rad < len)
    {
        return 0;
    }

    /* check if one bbox's mid is inside another */
    if (surf_bbox(ref->mid, srf) == 1)
    {
        return 1;
    }

    if (surf_bbox(srf->mid, ref) == 1)
    {
        return 1;
    }

    /* check if edges of one bbox intersect faces of another */
    for (j = 0; j < srf->faces_num; j++)
    {
        rt_FACE *fc = &srf->faces[j];

        for (i = 0; i < ref->edges_num; i++)
        {
            rt_EDGE *ei = &ref->edges[i];

            if (vert_to_face(ref->verts[ei->index[0]].pos,
                             ref->verts[ei->index[1]].pos,
                             srf->verts[fc->index[0]].pos,
                             srf->verts[fc->index[1]].pos,
                             srf->verts[fc->index[2]].pos,
                             fc->k, fc->i, fc->j) == 2)
            {
                return 2;
            }
            if (fc->k < 3)
            {
                continue;
            }
            if (vert_to_face(ref->verts[ei->index[0]].pos,
                             ref->verts[ei->index[1]].pos,
                             srf->verts[fc->index[2]].pos,
                             srf->verts[fc->index[3]].pos,
                             srf->verts[fc->index[0]].pos,
                             fc->k, fc->i, fc->j) == 2)
            {
                return 2;
            }
        }
    }

    for (j = 0; j < ref->faces_num; j++)
    {
        rt_FACE *fc = &ref->faces[j];

        for (i = 0; i < srf->edges_num; i++)
        {
            rt_EDGE *ei = &srf->edges[i];

            if (vert_to_face(srf->verts[ei->index[0]].pos,
                             srf->verts[ei->index[1]].pos,
                             ref->verts[fc->index[0]].pos,
                             ref->verts[fc->index[1]].pos,
                             ref->verts[fc->index[2]].pos,
                             fc->k, fc->i, fc->j) == 2)
            {
                return 2;
            }
            if (fc->k < 3)
            {
                continue;
            }
            if (vert_to_face(srf->verts[ei->index[0]].pos,
                             srf->verts[ei->index[1]].pos,
                             ref->verts[fc->index[2]].pos,
                             ref->verts[fc->index[3]].pos,
                             ref->verts[fc->index[0]].pos,
                             fc->k, fc->i, fc->j) == 2)
            {
                return 2;
            }
        }
    }

    return 0;
}

/*
 * Determine which side of clipped "srf" is seen from "ref" bbox.
 *
 * Return values:
 *  0 - none
 *  1 - inner
 *  2 - outer
 *  3 - both
 */
rt_cell bbox_side(rt_Surface *srf, rt_Surface *ref)
{
    rt_cell i, j, k, m, n, p, c = 0;

    p = srf->tag == RT_TAG_PLANE ? 1 : 0;

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

    /* check clip relationship */
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

    /* check if all "ref" verts are on the same side */
    if (p == 1)
    {
        if (ref->verts_num == 0)
        {
            c |= 3;
        }
        for (i = 0; i < ref->verts_num; i++)
        {
            c |= surf_side(ref->verts[i].pos, srf);
            if (c == 3)
            {
                break;
            }
        }
        return c;
    }

    /* check if bboxes interpenetrate */
    n = bbox_fuse(srf, ref);

    if (n != 0 && m == 1 || n == 2)
    {
        c |= 3;
        return c;
    }

    /* check if all "ref" verts are inside "srf" */
    if (n == 1 && m == 0)
    {
        c |= 1;
        for (i = 0; i < ref->verts_num; i++)
        {
            n = surf_side(ref->verts[i].pos, srf);
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

    /* check if all "ref" verts are inside "srf" cbox */
    if (k == 1)
    {
        c |= 2;
        for (i = 0; i < ref->verts_num; i++)
        {
            k = surf_cbox(ref->verts[i].pos, srf);
            if (k == 1)
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
