/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <string.h>

#include "thread.h"
#include "rtgeom.h"

/******************************************************************************/
/******************************   STATE-LOGGING   *****************************/
/******************************************************************************/

#define RT_PRINT_LGT(elm, lgt)                                              \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lgt --------------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lgt("    ", elm, lgt);                                        \
        RT_LOGI("\n")

#define RT_PRINT_LGT_INNER(elm, lgt)                                        \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lgt - inner ------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lgt("    ", elm, lgt);                                        \
        RT_LOGI("\n")

#define RT_PRINT_LGT_OUTER(elm, lgt)                                        \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lgt - outer ------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lgt("    ", elm, lgt);                                        \
        RT_LOGI("\n")

#define RT_PRINT_CLP_LST(lst)                                               \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- clp --------------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define RT_PRINT_LST(lst)                                                   \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lst --------------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define RT_PRINT_LST_INNER(lst)                                             \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lst - inner ------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define RT_PRINT_LST_OUTER(lst)                                             \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lst - outer ------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define RT_PRINT_SHW(lst)                                                   \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- shw --------------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define RT_PRINT_SHW_INNER(lst)                                             \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- shw - inner ------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define RT_PRINT_SHW_OUTER(lst)                                             \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- shw - outer ------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

/******************************************************************************/
/*********************************   THREAD   *********************************/
/******************************************************************************/

/*
 * Instantiate scene thread.
 */
rt_SceneThread::rt_SceneThread(rt_Scene *scene, rt_cell index) :

    rt_Heap(scene->f_alloc, scene->f_free)
{
    this->scene = scene;
    this->index = index;

    /* allocate root SIMD structure */
    s_inf = (rt_SIMD_INFOX *)
            alloc(sizeof(rt_SIMD_INFOX),
                            RT_SIMD_ALIGN);

    memset(s_inf, 0, sizeof(rt_SIMD_INFOX));

    RT_SIMD_SET(s_inf->gpc01, +1.0);
    RT_SIMD_SET(s_inf->gpc02, -0.5);
    RT_SIMD_SET(s_inf->gpc03, +3.0);
    RT_SIMD_SET(s_inf->gpc04, 0x7FFFFFFF);
    RT_SIMD_SET(s_inf->gpc05, 0x3F800000);

    /* init framebuffer's dimensions and pointer */
    s_inf->frm_w   = scene->x_res;
    s_inf->frm_h   = scene->y_res;
    s_inf->frm_row = scene->x_row;
    s_inf->frame   = scene->frame;

    /* init tilebuffer's dimensions and pointer */
    s_inf->tile_w  = scene->tile_w;
    s_inf->tile_h  = scene->tile_h;
    s_inf->tls_row = scene->tiles_in_row;
    s_inf->tiles   = scene->tiles;

    /* allocate cam SIMD structure */
    s_cam = (rt_SIMD_CAMERA *)
            alloc(sizeof(rt_SIMD_CAMERA),
                            RT_SIMD_ALIGN);

    memset(s_cam, 0, sizeof(rt_SIMD_CAMERA));

    /* allocate ctx SIMD structure */
    s_ctx = (rt_SIMD_CONTEXT *)
            alloc(sizeof(rt_SIMD_CONTEXT) + /* +1 context step for shadows */
                            RT_STACK_STEP * (1 + scene->depth),
                            RT_SIMD_ALIGN);

    memset(s_ctx, 0, sizeof(rt_SIMD_CONTEXT));

    /* init memory pool in the heap for temporary per-frame allocs */
    mpool = RT_NULL;
    msize = 0;

    /* allocate misc arrays for tiling */
    txmin = (rt_cell *)alloc(sizeof(rt_cell) * scene->tiles_in_col, RT_ALIGN);
    txmax = (rt_cell *)alloc(sizeof(rt_cell) * scene->tiles_in_col, RT_ALIGN);
    verts = (rt_VERT *)alloc(sizeof(rt_VERT) * 
                             (2 * RT_VERTS_LIMIT + RT_EDGES_LIMIT), RT_ALIGN);
}

#define RT_UPDATE_TILES_BOUNDS(cy, x1, x2)                                  \
do                                                                          \
{                                                                           \
    if (x1 < x2)                                                            \
    {                                                                       \
        if (txmin[cy] > x1)                                                 \
        {                                                                   \
            txmin[cy] = RT_MAX(x1, xmin);                                   \
        }                                                                   \
                                                                            \
        if (txmax[cy] < x2)                                                 \
        {                                                                   \
            txmax[cy] = RT_MIN(x2, xmax);                                   \
        }                                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        if (txmin[cy] > x2)                                                 \
        {                                                                   \
            txmin[cy] = RT_MAX(x2, xmin);                                   \
        }                                                                   \
                                                                            \
        if (txmax[cy] < x1)                                                 \
        {                                                                   \
            txmax[cy] = RT_MIN(x1, xmax);                                   \
        }                                                                   \
    }                                                                       \
}                                                                           \
while (0) /* "do {...} while (0)" to enforce semicolon ";" at the end */

/*
 * Update surface's projected bbox boundaries in tilebuffer
 * by processing one bbox edge at a time.
 * The tilebuffer is reset for every surface from outside of this function.
 */
rt_void rt_SceneThread::tiling(rt_vec4 p1, rt_vec4 p2)
{
    rt_real *pt, n1[3][2], n2[3][2];
    rt_real dx, dy, xx, yy, rt, px;
    rt_cell x1, y1, x2, y2, i, n, t;

    /* swap points vertically if needed */
    if (p1[RT_Y] > p2[RT_Y])
    {
        pt = p1;
        p1 = p2;
        p2 = pt;
    }

    /* initialize delta variables */
    dx = p2[RT_X] - p1[RT_X];
    dy = p2[RT_Y] - p1[RT_Y];

    /* prepare new lines with margins */
    if ((RT_FABS(dx) <= RT_LINE_THRESHOLD)
    &&  (RT_FABS(dy) <= RT_LINE_THRESHOLD))
    {
        rt = 0.0f;
        xx = dx < 0.0f ? -1.0f : 1.0f;
        yy = 1.0f;
    }
    else
    if ((RT_FABS(dx) <= RT_LINE_THRESHOLD)
    ||  (RT_FABS(dy) <= RT_LINE_THRESHOLD))
    {
        rt = 0.0f;
        xx = dx;
        yy = dy;
    }
    else
    {
        rt = dx / dy;
        xx = dx;
        yy = dy;
    }

#if RT_OPTS_TILING_EXT1 != 0
    if ((scene->opts & RT_OPTS_TILING_EXT1) != 0)
    {
        px  = RT_TILE_THRESHOLD / RT_SQRT(xx * xx + yy * yy);
        xx *= px;
        yy *= px;

        n1[0][RT_X] = p1[RT_X] - xx;
        n1[0][RT_Y] = p1[RT_Y] - yy;
        n2[0][RT_X] = p2[RT_X] + xx;
        n2[0][RT_Y] = p2[RT_Y] + yy;

        n1[1][RT_X] = n1[0][RT_X] - yy;
        n1[1][RT_Y] = n1[0][RT_Y] + xx;
        n2[1][RT_X] = n2[0][RT_X] - yy;
        n2[1][RT_Y] = n2[0][RT_Y] + xx;

        n1[2][RT_X] = n1[0][RT_X] + yy;
        n1[2][RT_Y] = n1[0][RT_Y] - xx;
        n2[2][RT_X] = n2[0][RT_X] + yy;
        n2[2][RT_Y] = n2[0][RT_Y] - xx;

        n = 3;
    }
    else
#endif /* RT_OPTS_TILING_EXT1 */
    {
        n1[0][RT_X] = p1[RT_X];
        n1[0][RT_Y] = p1[RT_Y];
        n2[0][RT_X] = p2[RT_X];
        n2[0][RT_Y] = p2[RT_Y];

        n = 1;
    }

    /* set inclusive bounds */
    rt_cell xmin = 0;
    rt_cell ymin = 0;
    rt_cell xmax = scene->tiles_in_row - 1;
    rt_cell ymax = scene->tiles_in_col - 1;

    for (i = 0; i < n; i++)
    {
        /* calculate points floor */
        x1 = RT_FLOOR(n1[i][RT_X]);
        y1 = RT_FLOOR(n1[i][RT_Y]);
        x2 = RT_FLOOR(n2[i][RT_X]);
        y2 = RT_FLOOR(n2[i][RT_Y]);

        /* reject y-outer lines */
        if (y1 > ymax || y2 < ymin)
        {
            continue;
        }

        /* process nearly vertical, nearly horizontal or x-outer line */
        if ((x1 == x2  || y1 == y2 || rt == 0.0f)
        ||  (x1 < xmin && x2 < xmin)
        ||  (x1 > xmax && x2 > xmax))
        {
            if (y1 < ymin)
            {
                y1 = ymin;
            }

            if (y2 > ymax)
            {
                y2 = ymax;
            }

            for (t = y1; t <= y2; t++)
            {
                RT_UPDATE_TILES_BOUNDS(t, x1, x2);
            }

            continue;
        }

        /* process regular line */
        y1 = y1 < ymin ? ymin : y1 + 1;
        y2 = y2 > ymax ? ymax : y2 - 1;

        px = n1[i][RT_X] + (y1 - n1[i][RT_Y]) * rt;
        x2 = RT_FLOOR(px);

        if (y1 > ymin)
        {
            RT_UPDATE_TILES_BOUNDS(y1 - 1, x1, x2);
        }

        x1 = x2;

        for (t = y1; t <= y2; t++)
        {
            px = px + rt;
            x2 = RT_FLOOR(px);
            RT_UPDATE_TILES_BOUNDS(t, x1, x2);
            x1 = x2;
        }

        if (y2 < ymax)
        {
            x2 = RT_FLOOR(n2[i][RT_X]);
            RT_UPDATE_TILES_BOUNDS(y2 + 1, x1, x2);
        }
    }
}

/*
 * Insert new element derived from "srf" to a list "ptr"
 * for a given object "obj". If "srf" is NULL and "obj" is LIGHT,
 * insert new element derived from "obj" to a list "ptr".
 */
rt_void rt_SceneThread::insert(rt_Object *obj, rt_ELEM **ptr, rt_Surface *srf)
{
    rt_ELEM *elm = RT_NULL;

    if (srf != RT_NULL)
    {
        /* alloc new element for srf */
        elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_ALIGN);
        elm->data = 0;
        elm->simd = srf->s_srf;
        elm->temp = srf;

        rt_ELEM *lst[2], *nxt;
        rt_Array *arr[2];

        lst[0] = RT_NULL;
        arr[0] = srf->trnode != RT_NULL && srf->trnode != srf ?
                 (rt_Array *)srf->trnode : RT_NULL;

        lst[1] = RT_NULL;
        arr[1] = srf->bvnode != RT_NULL && RT_IS_SURFACE(obj) ?
                 (rt_Array *)srf->bvnode : RT_NULL;

        rt_cell i, k = 0, n = (arr[0] != RT_NULL) + (arr[1] != RT_NULL);

        if (n != 0)
        {
            /* search matching existing trnode/bvnode for insertion */
            for (nxt = *ptr; nxt != RT_NULL; nxt = nxt->next)
            {
                for (i = 0; i < 2; i++)
                {
                    if (arr[i] == nxt->temp && (nxt->data & 0x3) == i)
                    {
#if RT_DEBUG == 1
                        if (arr[i] == RT_NULL
                        ||  lst[i] != RT_NULL)
                        {
                            throw rt_Exception("inconsistency in surface list");
                        }
#endif /* RT_DEBUG */
                        lst[i] = nxt;
                        ptr = &nxt->next;
                        k++;
                        if (k == n)
                        {
                            break;
                        }
                    }
                }
            }
        }

        k = -1;

        if (n == 2)
        {
            /* determine trnode/bvnode order on the branch */
            if (arr[0] == arr[1])
            {
                k = 0;
            }
            else
            {
                for (i = 0; i < 2; i++)
                {
                    rt_Array *par = RT_NULL;
                    for (par = (rt_Array *)arr[i]->parent; par != RT_NULL;
                         par = (rt_Array *)par->parent)
                    {
                        if (par == arr[1 - i])
                        {
                            k = i;
                            break;
                        }
                    }
                    if (k == i)
                    {
                        break;
                    }
                }
            }
        }
        else
        if (arr[0] != RT_NULL)
        {
            k = 0;
        }
        else
        if (arr[1] != RT_NULL)
        {
            k = 1;
        }

        if (n != 0 && k == -1)
        {
            throw rt_Exception("trnode and bvnode are not on the same branch");
        }
        if (n == 2 && lst[k] != RT_NULL && lst[1 - k] == RT_NULL)
        {
            throw rt_Exception("inconsistency between trnode and bvnode");
        }

        /* insert element according to found position */
        elm->next = *ptr;
       *ptr = elm;

        for (i = 0; i < n; i++, k = 1 - k)
        {
            if (arr[k] != RT_NULL && lst[k] == RT_NULL)
            {
                /* alloc new trnode/bvnode element as none has been found */
                nxt = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_ALIGN);
                nxt->data = (rt_cell)elm | k; /* last element plus flag */
                nxt->simd = arr[k]->s_srf;
                nxt->temp = arr[k];
                /* insert element according to found position */
                nxt->next = *ptr;
               *ptr = nxt;
            }
        }
    }
    else
    if (RT_IS_LIGHT(obj))
    {
        rt_Light *lgt = (rt_Light *)obj;

        /* alloc new element for lgt */
        elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_ALIGN);
        elm->data = (rt_cell)scene->slist; /* all srf are potential shadows */
        elm->simd = lgt->s_lgt;
        elm->temp = lgt;
        /* insert element as list head */
        elm->next = *ptr;
       *ptr = elm;
    }
}

/*
 * Build tile list for a given "srf" based
 * on the area its projected bbox occupies in the tilebuffer.
 */
rt_void rt_SceneThread::stile(rt_Surface *srf)
{
    srf->s_srf->msc_p[0] = RT_NULL;

#if RT_OPTS_TILING != 0
    if ((scene->opts & RT_OPTS_TILING) == 0)
#endif /* RT_OPTS_TILING */
    {
        return;
    }

    rt_ELEM *elm = RT_NULL;
    rt_cell i, j;
    rt_cell k;

    rt_vec4 vec;
    rt_real dot;
    rt_cell ndx[2];
    rt_real tag[2], zed[2];

    /* verts_num may grow, use srf->verts_num if original is needed */
    rt_cell verts_num = srf->verts_num;
    rt_VERT *vrt = srf->verts;

    /* project bbox onto the tilebuffer */
    if (verts_num != 0)
    {
        for (i = 0; i < scene->tiles_in_col; i++)
        {
            txmin[i] = scene->tiles_in_row;
            txmax[i] = -1;
        }

        /* process bbox vertices */
        memset(verts, 0, sizeof(rt_VERT) * (2 * verts_num + srf->edges_num));

        for (k = 0; k < srf->verts_num; k++)
        {
            vec[RT_X] = vrt[k].pos[RT_X] - scene->org[RT_X];
            vec[RT_Y] = vrt[k].pos[RT_Y] - scene->org[RT_Y];
            vec[RT_Z] = vrt[k].pos[RT_Z] - scene->org[RT_Z];

            dot = RT_VECTOR_DOT(vec, scene->nrm);

            verts[k].pos[RT_Z] = dot;
            verts[k].pos[RT_W] = -1.0f; /* tag: behind screen plane */

            /* process vertices in front of or near screen plane,
             * the rest are processed with edges */
            if (dot >= 0.0f || RT_FABS(dot) <= RT_CLIP_THRESHOLD)
            {
                vec[RT_X] = vrt[k].pos[RT_X] - scene->pos[RT_X];
                vec[RT_Y] = vrt[k].pos[RT_Y] - scene->pos[RT_Y];
                vec[RT_Z] = vrt[k].pos[RT_Z] - scene->pos[RT_Z];

                dot = RT_VECTOR_DOT(vec, scene->nrm) / scene->cam->pov;

                vec[RT_X] /= dot; /* dot >= (pov - RT_CLIP_THRESHOLD) */
                vec[RT_Y] /= dot; /* pov >= (2  *  RT_CLIP_THRESHOLD) */
                vec[RT_Z] /= dot; /* thus: (dot >= RT_CLIP_THRESHOLD) */

                vec[RT_X] -= scene->dir[RT_X];
                vec[RT_Y] -= scene->dir[RT_Y];
                vec[RT_Z] -= scene->dir[RT_Z];

                verts[k].pos[RT_X] = RT_VECTOR_DOT(vec, scene->htl);
                verts[k].pos[RT_Y] = RT_VECTOR_DOT(vec, scene->vtl);

                verts[k].pos[RT_W] = +1.0f; /* tag: in front of screen plane */

                /* slightly behind (near) screen plane,
                 * generate new vertex */
                if (verts[k].pos[RT_Z] < 0.0f)
                {
                    verts[verts_num].pos[RT_X] = verts[k].pos[RT_X];
                    verts[verts_num].pos[RT_Y] = verts[k].pos[RT_Y];
                    verts_num++;

                    verts[k].pos[RT_W] = 0.0f; /* tag: near screen plane */
                }
            }
        }

        /* process bbox edges */
        for (k = 0; k < srf->edges_num; k++)
        {
            for (i = 0; i < 2; i++)
            {
                ndx[i] = srf->edges[k].index[i];
                zed[i] = verts[ndx[i]].pos[RT_Z];
                tag[i] = verts[ndx[i]].pos[RT_W];
            }

            /* skip edge if both vertices are eihter
             * behind or near screen plane */
            if (tag[0] <= 0.0f && tag[1] <= 0.0f)
            {
                continue;
            }

            for (i = 0; i < 2; i++)
            {
                /* skip vertex if in front of
                 * or near screen plane */
                if (tag[i] >= 0.0f)
                {
                    continue;
                }

                /* process edge with one "in front of"
                 * and one "behind" vertices */
                j = 1 - i;

                /* clip edge at screen plane crossing,
                 * generate new vertex */
                vec[RT_X] = vrt[ndx[i]].pos[RT_X] - vrt[ndx[j]].pos[RT_X];
                vec[RT_Y] = vrt[ndx[i]].pos[RT_Y] - vrt[ndx[j]].pos[RT_Y];
                vec[RT_Z] = vrt[ndx[i]].pos[RT_Z] - vrt[ndx[j]].pos[RT_Z];

                dot = zed[j] / (zed[j] - zed[i]); /* () >= RT_CLIP_THRESHOLD */

                vec[RT_X] *= dot;
                vec[RT_Y] *= dot;
                vec[RT_Z] *= dot;

                vec[RT_X] += vrt[ndx[j]].pos[RT_X] - scene->org[RT_X];
                vec[RT_Y] += vrt[ndx[j]].pos[RT_Y] - scene->org[RT_Y];
                vec[RT_Z] += vrt[ndx[j]].pos[RT_Z] - scene->org[RT_Z];

                verts[verts_num].pos[RT_X] = RT_VECTOR_DOT(vec, scene->htl);
                verts[verts_num].pos[RT_Y] = RT_VECTOR_DOT(vec, scene->vtl);

                ndx[i] = verts_num;
                verts_num++;
            }

            /* tile current edge */
            tiling(verts[ndx[0]].pos, verts[ndx[1]].pos); 
        }

        /* tile all newly generated vertex pairs */
        for (i = srf->verts_num; i < verts_num - 1; i++)
        {
            for (j = i + 1; j < verts_num; j++)
            {
                tiling(verts[i].pos, verts[j].pos); 
            }
        }
    }
    else
    {
        /* mark all tiles in the entire tilbuffer */
        for (i = 0; i < scene->tiles_in_col; i++)
        {
            txmin[i] = 0;
            txmax[i] = scene->tiles_in_row - 1;
        }
    }

    rt_ELEM **ptr = (rt_ELEM **)&srf->s_srf->msc_p[0];

    /* fill marked tiles with surface data */
    for (i = 0; i < scene->tiles_in_col; i++)
    {
        for (j = txmin[i]; j <= txmax[i]; j++)
        {
            elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_ALIGN);
            elm->data = i << 16 | j;
            elm->simd = srf->s_srf;
            elm->temp = srf;
           *ptr = elm;
            ptr = &elm->next;
        }
    }

   *ptr = RT_NULL;
}

/*
 * Build surface lists for a given "obj".
 * Surfaces have separate surface lists for each side.
 */
rt_ELEM* rt_SceneThread::ssort(rt_Object *obj)
{
    rt_Surface *srf = RT_NULL;
    rt_ELEM **pto = RT_NULL;
    rt_ELEM **pti = RT_NULL;

    if (RT_IS_SURFACE(obj))
    {
        srf = (rt_Surface *)obj;

        if (g_print && srf->s_srf->msc_p[2] != RT_NULL)
        {
            RT_PRINT_CLP_LST((rt_ELEM *)srf->s_srf->msc_p[2]);
        }

        pto = (rt_ELEM **)&srf->s_srf->lst_p[1];
        pti = (rt_ELEM **)&srf->s_srf->lst_p[3];

#if RT_OPTS_RENDER != 0
        if ((scene->opts & RT_OPTS_RENDER) != 0
        && (((rt_word)srf->s_srf->mat_p[1] & RT_PROP_REFLECT) != 0
        ||  ((rt_word)srf->s_srf->mat_p[3] & RT_PROP_REFLECT) != 0
        ||  ((rt_word)srf->s_srf->mat_p[1] & RT_PROP_OPAQUE) == 0
        ||  ((rt_word)srf->s_srf->mat_p[3] & RT_PROP_OPAQUE) == 0))
        {
           *pto = RT_NULL;
           *pti = RT_NULL;

            /* RT_LOGI("Building slist for surface\n"); */
        }
        else
#endif /* RT_OPTS_RENDER */
        {
           *pto = scene->slist; /* all srf are potential rfl/rfr */
           *pti = scene->slist; /* all srf are potential rfl/rfr */

            return RT_NULL;
        }
    }

    rt_Surface *ref = RT_NULL;
    rt_ELEM *lst = RT_NULL;
    rt_ELEM **ptr = &lst;

    for (ref = scene->srf_head; ref != RT_NULL; ref = ref->next)
    {
#if RT_OPTS_2SIDED != 0
        if ((scene->opts & RT_OPTS_2SIDED) != 0 && srf != RT_NULL)
        {
            rt_cell c = bbox_side(srf, ref);

            if (c & 2)
            {
                insert(obj, pto, ref);
            }
            if (c & 1)
            {
                insert(obj, pti, ref);
            }
        }
        else
#endif /* RT_OPTS_2SIDED */
        {
            insert(obj, ptr, ref);
        }
    }

    if (srf == RT_NULL)
    {
        return lst;
    }

    if (g_print)
    {
#if RT_OPTS_2SIDED != 0
        if (*pto != RT_NULL)
        {
            RT_PRINT_LST_OUTER(*pto);
        }
        if (*pti != RT_NULL)
        {
            RT_PRINT_LST_INNER(*pti);
        }
#endif /* RT_OPTS_2SIDED */
        if (*ptr != RT_NULL)
        {
            RT_PRINT_LST(*ptr);
        }
    }

#if RT_OPTS_2SIDED != 0
    if ((scene->opts & RT_OPTS_2SIDED) == 0)
#endif /* RT_OPTS_2SIDED */
    {
       *pto = lst;
       *pti = lst;

        return RT_NULL;
    }

    return lst;
}

/*
 * Build light/shadow lists for a given "obj".
 * Surfaces have separate light/shadow lists for each side.
 */
rt_ELEM* rt_SceneThread::lsort(rt_Object *obj)
{
    rt_Surface *srf = RT_NULL;
    rt_ELEM **pto = RT_NULL;
    rt_ELEM **pti = RT_NULL;

    if (RT_IS_SURFACE(obj))
    {
        srf = (rt_Surface *)obj;

        pto = (rt_ELEM **)&srf->s_srf->lst_p[0];
        pti = (rt_ELEM **)&srf->s_srf->lst_p[2];

#if RT_OPTS_SHADOW != 0
        if ((scene->opts & RT_OPTS_SHADOW) != 0)
        {
           *pto = RT_NULL;
           *pti = RT_NULL;

            /* RT_LOGI("Building llist for surface\n"); */
        }
        else
#endif /* RT_OPTS_SHADOW */
        {
           *pto = scene->llist; /* all lgt are potential sources */
           *pti = scene->llist; /* all lgt are potential sources */

            return RT_NULL;
        }
    }

    rt_Light *lgt = RT_NULL;
    rt_ELEM *lst = RT_NULL;
    rt_ELEM **ptr = &lst;

    for (lgt = scene->lgt_head; lgt != RT_NULL; lgt = lgt->next)
    {
        rt_ELEM **psr = RT_NULL;
#if RT_OPTS_2SIDED != 0
        rt_ELEM **pso = RT_NULL;
        rt_ELEM **psi = RT_NULL;

        if ((scene->opts & RT_OPTS_2SIDED) != 0 && srf != RT_NULL)
        {
            rt_cell c = cbox_side(lgt->pos, srf);

            if (c & 2)
            {
                insert(lgt, pto, RT_NULL);

                pso = (rt_ELEM **)&(*pto)->data;
               *pso = RT_NULL;

                if (g_print)
                {
                    RT_PRINT_LGT_OUTER(*pto, lgt);
                }
            }
            if (c & 1)
            {
                insert(lgt, pti, RT_NULL);

                psi = (rt_ELEM **)&(*pti)->data;
               *psi = RT_NULL;

                if (g_print)
                {
                    RT_PRINT_LGT_INNER(*pto, lgt);
                }
            }
        }
        else
#endif /* RT_OPTS_2SIDED */
        {
            insert(lgt, ptr, RT_NULL);

            psr = (rt_ELEM **)&(*ptr)->data;

            if (g_print && srf != RT_NULL)
            {
                RT_PRINT_LGT(*ptr, lgt);
            }
        }

#if RT_OPTS_SHADOW != 0
        if (srf == RT_NULL)
        {
            continue;
        }

        if (psr != RT_NULL)
        {
           *psr = RT_NULL;
        }

        rt_Surface *shw = RT_NULL;

        for (shw = scene->srf_head; shw != RT_NULL; shw = shw->next)
        {
            if (bbox_shad(lgt, shw, srf) == 0)
            {
                continue;
            }

#if RT_OPTS_2SIDED != 0
            if ((scene->opts & RT_OPTS_2SIDED) != 0)
            {
                rt_cell c = bbox_side(srf, shw);

                if (c & 2 && pso != RT_NULL)
                {
                    insert(lgt, pso, shw);
                }
                if (c & 1 && psi != RT_NULL)
                {
                    insert(lgt, psi, shw);
                }
            }
            else
#endif /* RT_OPTS_2SIDED */
            {
                insert(lgt, psr, shw);
            }
        }

        if (g_print)
        {
#if RT_OPTS_2SIDED != 0
            if (pso != RT_NULL && *pso != RT_NULL)
            {
                RT_PRINT_SHW_OUTER(*pso);
            }
            if (psi != RT_NULL && *psi != RT_NULL)
            {
                RT_PRINT_SHW_INNER(*psi);
            }
#endif /* RT_OPTS_2SIDED */
            if (psr != RT_NULL && *psr != RT_NULL)
            {
                RT_PRINT_SHW(*psr);
            }
        }
#endif /* RT_OPTS_SHADOW */
    }

    if (srf == RT_NULL)
    {
        return lst;
    }

#if RT_OPTS_2SIDED != 0
    if ((scene->opts & RT_OPTS_2SIDED) == 0)
#endif /* RT_OPTS_2SIDED */
    {
       *pto = lst;
       *pti = lst;

        return RT_NULL;
    }

    return lst;
}

/*
 * Destroy scene thread.
 */
rt_SceneThread::~rt_SceneThread()
{

}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
