/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
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
#endif /* RT_DEBUG */

/*
 * Multiply matrix by vector.
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
 * Multiply matrix by matrix.
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
 * Compute inverse matrix.
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
            if (RT_FABS(tm[i][j] - iden_m[i][j]) > 0.00001)
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

    throw rt_Exception("Inverted matrix mismatch detected.");
#endif /* RT_DEBUG */
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
