/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <string.h>

#include "engine.h"
#include "rtgeom.h"
#include "system.h"

/******************************************************************************/
/**********************************   SCENE   *********************************/
/******************************************************************************/

rt_Scene::rt_Scene(rt_SCENE *scn, /* frame must be SIMD-aligned */
                   rt_word x_res, rt_word y_res, rt_cell x_row, rt_word *frame,
                   rt_FUNC_ALLOC f_alloc, rt_FUNC_FREE f_free) : 

    rt_Registry(f_alloc, f_free)
{
    this->scn   = scn;

    this->x_res = x_res;
    this->y_res = y_res;
    this->x_row = x_row;

    if (frame == RT_NULL)
    {
        frame = (rt_word *)
                alloc(x_res * y_res * sizeof(rt_word),
                                RT_SIMD_ALIGN);
    }
    else
    if (((rt_word)frame & (RT_SIMD_ALIGN - 1)) != 0)
    {
        throw rt_Exception("frame pointer is not SIMD-aligned in Scene");
    }

    this->frame = frame;

    factor = 1.0f / (rt_real)x_res;
    aspect = (rt_real)y_res * factor;

    s_inf = (rt_SIMD_INFOX *)
            alloc(sizeof(rt_SIMD_INFOX),
                            RT_SIMD_ALIGN);

    memset(s_inf, 0, sizeof(rt_SIMD_INFOX));

    RT_SIMD_SET(s_inf->gpc01, +1.0);
    RT_SIMD_SET(s_inf->gpc02, -0.5);
    RT_SIMD_SET(s_inf->gpc03, +3.0);

    s_inf->frm_w   = x_res;
    s_inf->frm_h   = y_res;
    s_inf->frm_row = x_row;
    s_inf->frame   = frame;

    render0(s_inf);

    s_cam = (rt_SIMD_CAMERA *)
            alloc(sizeof(rt_SIMD_CAMERA),
                            RT_SIMD_ALIGN);

    s_ctx = (rt_SIMD_CONTEXT *)
            alloc(sizeof(rt_SIMD_CONTEXT),
                            RT_SIMD_ALIGN);

    memset(&rootobj, 0, sizeof(rt_OBJECT));

    rootobj.trm.scl[RT_I] = 1.0f;
    rootobj.trm.scl[RT_J] = 1.0f;
    rootobj.trm.scl[RT_K] = 1.0f;
    rootobj.obj = scn->root;

    root = new rt_Array(this, RT_NULL, &rootobj);
    cam  = cam_head;

    /* setup surface list */
    slist = ssort(cam);
}

/*
 * Insert element into a list for a given object.
 */
rt_void rt_Scene::insert(rt_Object *obj, rt_ELEM **ptr, rt_Surface *srf)
{
    rt_ELEM *elm = RT_NULL;

    if (srf != RT_NULL)
    {
        elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_ALIGN);
        elm->data = 0;
        elm->simd = srf->s_srf;
        elm->temp = srf;
        elm->next = *ptr;
       *ptr = elm;
    }
}

/*
 * Build surface lists for a given object.
 */
rt_ELEM* rt_Scene::ssort(rt_Object *obj)
{
    rt_Surface *srf = RT_NULL;
    rt_ELEM *lst = RT_NULL;
    rt_ELEM **ptr = &lst;

    for (srf = srf_head; srf != RT_NULL; srf = srf->next)
    {
        insert(obj, ptr, srf);
    }

    return lst;
}

/*
 * Update backend data structures and render the frame.
 */
rt_void rt_Scene::render(rt_long time)
{
    /* update the whole objects hierarchy */

    root->update(time, iden4, 0);

    /* update backend-related parts */

    rt_Surface *srf = RT_NULL;

    for (srf = srf_head; srf != RT_NULL; srf = srf->next)
    {
        update0(srf->s_srf);
    }

    /* prepare for rendering */

    rt_real h, v;

    pos[RT_X] = cam->pos[RT_X];
    pos[RT_Y] = cam->pos[RT_Y];
    pos[RT_Z] = cam->pos[RT_Z];

    hor[RT_X] = cam->hor[RT_X];
    hor[RT_Y] = cam->hor[RT_Y];
    hor[RT_Z] = cam->hor[RT_Z];

    ver[RT_X] = cam->ver[RT_X];
    ver[RT_Y] = cam->ver[RT_Y];
    ver[RT_Z] = cam->ver[RT_Z];

    nrm[RT_X] = cam->nrm[RT_X];
    nrm[RT_Y] = cam->nrm[RT_Y];
    nrm[RT_Z] = cam->nrm[RT_Z];

            h = (1.0f);
            v = (aspect);

    dir[RT_X] = (nrm[RT_X] * cam->pov - (hor[RT_X] * h + ver[RT_X] * v) * 0.5f);
    dir[RT_Y] = (nrm[RT_Y] * cam->pov - (hor[RT_Y] * h + ver[RT_Y] * v) * 0.5f);
    dir[RT_Z] = (nrm[RT_Z] * cam->pov - (hor[RT_Z] * h + ver[RT_Z] * v) * 0.5f);

    org[RT_X] = (pos[RT_X] + dir[RT_X]);
    org[RT_Y] = (pos[RT_Y] + dir[RT_Y]);
    org[RT_Z] = (pos[RT_Z] + dir[RT_Z]);

    hor[RT_X] *= factor;
    hor[RT_Y] *= factor;
    hor[RT_Z] *= factor;

    ver[RT_X] *= factor;
    ver[RT_Y] *= factor;
    ver[RT_Z] *= factor;

    dir[RT_X] += (hor[RT_X] + ver[RT_X]) * 0.5f;
    dir[RT_Y] += (hor[RT_Y] + ver[RT_Y]) * 0.5f;
    dir[RT_Z] += (hor[RT_Z] + ver[RT_Z]) * 0.5f;

    rt_real fdh[4], fdv[4];
    rt_real fhr, fvr;

    fdh[0] = 0.0f;
    fdh[1] = 1.0f;
    fdh[2] = 2.0f;
    fdh[3] = 3.0f;

    fdv[0] = 0.0f;
    fdv[1] = 0.0f;
    fdv[2] = 0.0f;
    fdv[3] = 0.0f;

    fhr = 4.0f;
    fvr = 1.0f;

/*  rt_SIMD_CAMERA */

    RT_SIMD_SET(s_cam->t_max, RT_INF);

    s_cam->dir_x[0] = dir[RT_X] + fdh[0] * hor[RT_X] + fdv[0] * ver[RT_X];
    s_cam->dir_x[1] = dir[RT_X] + fdh[1] * hor[RT_X] + fdv[1] * ver[RT_X];
    s_cam->dir_x[2] = dir[RT_X] + fdh[2] * hor[RT_X] + fdv[2] * ver[RT_X];
    s_cam->dir_x[3] = dir[RT_X] + fdh[3] * hor[RT_X] + fdv[3] * ver[RT_X];

    s_cam->dir_y[0] = dir[RT_Y] + fdh[0] * hor[RT_Y] + fdv[0] * ver[RT_Y];
    s_cam->dir_y[1] = dir[RT_Y] + fdh[1] * hor[RT_Y] + fdv[1] * ver[RT_Y];
    s_cam->dir_y[2] = dir[RT_Y] + fdh[2] * hor[RT_Y] + fdv[2] * ver[RT_Y];
    s_cam->dir_y[3] = dir[RT_Y] + fdh[3] * hor[RT_Y] + fdv[3] * ver[RT_Y];

    s_cam->dir_z[0] = dir[RT_Z] + fdh[0] * hor[RT_Z] + fdv[0] * ver[RT_Z];
    s_cam->dir_z[1] = dir[RT_Z] + fdh[1] * hor[RT_Z] + fdv[1] * ver[RT_Z];
    s_cam->dir_z[2] = dir[RT_Z] + fdh[2] * hor[RT_Z] + fdv[2] * ver[RT_Z];
    s_cam->dir_z[3] = dir[RT_Z] + fdh[3] * hor[RT_Z] + fdv[3] * ver[RT_Z];

    RT_SIMD_SET(s_cam->hor_x, hor[RT_X] * fhr);
    RT_SIMD_SET(s_cam->hor_y, hor[RT_Y] * fhr);
    RT_SIMD_SET(s_cam->hor_z, hor[RT_Z] * fhr);

    RT_SIMD_SET(s_cam->ver_x, ver[RT_X] * fvr);
    RT_SIMD_SET(s_cam->ver_y, ver[RT_Y] * fvr);
    RT_SIMD_SET(s_cam->ver_z, ver[RT_Z] * fvr);

/*  rt_SIMD_CONTEXT */

    RT_SIMD_SET(s_ctx->t_min, cam->pov);

    RT_SIMD_SET(s_ctx->org_x, pos[RT_X]);
    RT_SIMD_SET(s_ctx->org_y, pos[RT_Y]);
    RT_SIMD_SET(s_ctx->org_z, pos[RT_Z]);

   /* render based on surface-list */

    s_inf->ctx = s_ctx;
    s_inf->cam = s_cam;
    s_inf->lst = slist;

    render0(s_inf);
}

/*
 * Update current camera with given action.
 */
rt_void rt_Scene::update(rt_long time, rt_cell action)
{
    cam->update(time, action);
}

/*
 * Return pointer to the framebuffer.
 */
rt_word* rt_Scene::get_frame()
{
    return frame;
}

rt_Scene::~rt_Scene()
{

}

/******************************************************************************/
/******************************   FPS RENDERING   *****************************/
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
 * Render given number on the screen at given coords.
 * Parameters d and z specify direction and zoom respectively.
 */
rt_void rt_Scene::render_fps(rt_word x, rt_word y,
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
    rt_word *src = RT_NULL, *dst = RT_NULL;

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
