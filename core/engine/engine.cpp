/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <string.h>

#include "engine.h"

/******************************************************************************/
/*********************************   SCENE   **********************************/
/******************************************************************************/

rt_Scene::rt_Scene(rt_word x_res, rt_word y_res, rt_cell x_row, rt_word *frame,
                   rt_FUNC_ALLOC f_alloc, rt_FUNC_FREE f_free) : 

    rt_Heap(f_alloc, f_free)
{
    this->x_res = x_res;
    this->y_res = y_res;
    this->x_row = x_row;
    this->frame = frame;

    info = (rt_SIMD_INFO_EXT *)alloc(sizeof(rt_SIMD_INFO_EXT), RT_SIMD_ALIGN);

    RT_SIMD_SET(info->gpc01, +1.0);
    RT_SIMD_SET(info->gpc02, -0.5);
    RT_SIMD_SET(info->gpc03, +3.0);

    info->frm_w   = x_res;
    info->frm_h   = y_res;
    info->frm_row = x_row;
    info->frame   = frame;
}

/*
 * This method renders the whole frame.
 */
rt_void rt_Scene::render(rt_long time)
{
    render0(info);
}

rt_Scene::~rt_Scene()
{

}

/******************************************************************************/
/*****************************   FPS RENDERING   ******************************/
/******************************************************************************/

#define II  0xFF000000
#define OO  0xFFFFFFFF

#define dW  5
#define dH  7

rt_word digits[10][dH][dW] = 
{
    {
        OO, OO, OO, OO, OO,
        OO, II, II, II, OO,
        OO, II, OO, II, OO,
        OO, II, OO, II, OO,
        OO, II, OO, II, OO,
        OO, II, II, II, OO,
        OO, OO, OO, OO, OO,
    },
    {
        OO, OO, OO, OO, OO,
        OO, OO, II, OO, OO,
        OO, II, II, OO, OO,
        OO, OO, II, OO, OO,
        OO, OO, II, OO, OO,
        OO, II, II, II, OO,
        OO, OO, OO, OO, OO,
    },
    {
        OO, OO, OO, OO, OO,
        OO, II, II, II, OO,
        OO, OO, OO, II, OO,
        OO, II, II, II, OO,
        OO, II, OO, OO, OO,
        OO, II, II, II, OO,
        OO, OO, OO, OO, OO,
    },
    {
        OO, OO, OO, OO, OO,
        OO, II, II, II, OO,
        OO, OO, OO, II, OO,
        OO, II, II, II, OO,
        OO, OO, OO, II, OO,
        OO, II, II, II, OO,
        OO, OO, OO, OO, OO,
    },
    {
        OO, OO, OO, OO, OO,
        OO, II, OO, II, OO,
        OO, II, OO, II, OO,
        OO, II, II, II, OO,
        OO, OO, OO, II, OO,
        OO, OO, OO, II, OO,
        OO, OO, OO, OO, OO,
    },
    {
        OO, OO, OO, OO, OO,
        OO, II, II, II, OO,
        OO, II, OO, OO, OO,
        OO, II, II, II, OO,
        OO, OO, OO, II, OO,
        OO, II, II, II, OO,
        OO, OO, OO, OO, OO,
    },
    {
        OO, OO, OO, OO, OO,
        OO, II, II, II, OO,
        OO, II, OO, OO, OO,
        OO, II, II, II, OO,
        OO, II, OO, II, OO,
        OO, II, II, II, OO,
        OO, OO, OO, OO, OO,
    },
    {
        OO, OO, OO, OO, OO,
        OO, II, II, II, OO,
        OO, OO, OO, II, OO,
        OO, OO, OO, II, OO,
        OO, OO, OO, II, OO,
        OO, OO, OO, II, OO,
        OO, OO, OO, OO, OO,
    },
    {
        OO, OO, OO, OO, OO,
        OO, II, II, II, OO,
        OO, II, OO, II, OO,
        OO, II, II, II, OO,
        OO, II, OO, II, OO,
        OO, II, II, II, OO,
        OO, OO, OO, OO, OO,
    },
    {
        OO, OO, OO, OO, OO,
        OO, II, II, II, OO,
        OO, II, OO, II, OO,
        OO, II, II, II, OO,
        OO, OO, OO, II, OO,
        OO, II, II, II, OO,
        OO, OO, OO, OO, OO,
    },
};

/*
 * This method renders given number on the screen at given coords.
 * Parameters d and z specify direction and zoom respectively.
 */
rt_void rt_Scene::render_number(rt_word x, rt_word y,
                                rt_cell d, rt_word z, rt_word num)
{
    rt_word arr[16], i, c, k;

    for (i = 0, c = 0; i < 16; i++)
    {
        k = num % 10;
        num /= 10;
        arr[i] = k;

        if (k != 0)
        {
            c = i;
        }
    }    

    c++;
    d = (d + 1) / 2;

    rt_word xd, yd, xz, yz;
    rt_word *src = NULL, *dst = NULL;

    for (i = 0; i < c; i++)
    {
        k = arr[i];
        src = &digits[k][0][0];
        dst = frame + y * x_res + x + (c * d - 1 - i) * dW * z;

        for (yd = 0; yd < dH; yd++)
        {
            for (yz = 0; yz < z; yz++)
            {
                for (xd = 0; xd < dW; xd++)
                {
                    for (xz = 0; xz < z; xz++)
                    {
                        *dst++ = *src;
                    }
                    src++;
                }
                dst += x_res - dW * z;
                src -= dW;
            }
            src += dW;
        }
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
