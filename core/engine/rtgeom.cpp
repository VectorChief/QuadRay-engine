/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <string.h>

#include "rtgeom.h"

/******************************************************************************/
/********************************   MATRICES   ********************************/
/******************************************************************************/

/*
 * Identity matrix.
 */
rt_mat4 iden4 =
{
    1.0f,       0.0f,       0.0f,       0.0f,
    0.0f,       1.0f,       0.0f,       0.0f,
    0.0f,       0.0f,       1.0f,       0.0f,
    0.0f,       0.0f,       0.0f,       1.0f,
};

#if RT_DEBUG == 1
/*
 * This algorithm checks if given address ranges overlap.
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
#endif /* RT_DEBUG */

/*
 * This algorithm multiplies matrix by vector.
 */
rt_void matrix_mul_vector(rt_vec4 vp, rt_mat4 m1, rt_vec4 v1)
{
#if RT_DEBUG == 1

    if (in_range(vp, 4, m1[0], 16) || in_range(vp, 4, v1, 4))
    {
        throw rt_Exception("Attempting to multiply vectors in place");
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
 * This algorithm multiplies matrix by matrix.
 */
rt_void matrix_mul_matrix(rt_mat4 mp, rt_mat4 m1, rt_mat4 m2)
{
#if RT_DEBUG == 1

    if (in_range(mp[0], 16, m1[0], 16) || in_range(mp[0], 16, m2[0], 16))
    {
        throw rt_Exception("Attempting to multiply matrices in place");
    }

#endif /* RT_DEBUG */

    rt_cell i;

    for (i = 0; i < 4; i++)
    {
        matrix_mul_vector(mp[i], m1, m2[i]);
    }
}

/*
 * This algorithm computes matrix from transform.
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

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
