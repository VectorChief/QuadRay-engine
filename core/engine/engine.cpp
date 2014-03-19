/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <string.h>

#include "engine.h"
#include "rtgeom.h"
#include "rtimag.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * engine.cpp: Implementation of the scene manager.
 *
 * Main file of the engine responsible for instantiating and managing the scene.
 * It contains the definition of SceneThread and Scene classes along with
 * the set of algorithms needed to process objects in the scene in order
 * to prepare data structures used by the rendering backend (tracer.cpp).
 *
 * Processing of objects consists of two major parts: update and render,
 * of which only update is handled by the engine, while render is delegated
 * to the rendering backend once all data structures have been prepared.
 *
 * Update in turn consists of five phases:
 * 0.5 phase (sequential) - hierarchical update of arrays' transform matrices
 * 1st phase (multi-threaded) - update surfaces' transform matrices, data fields
 * 2nd phase (multi-threaded) - update surfaces' clip lists, bounds, tile lists
 * 2.5 phase (sequential) - hierarchical update of arrays' bounds from surfaces
 * 3rd phase (multi-threaded) - build updated cross-surface lists
 *
 * Some parts of the update are handled by the objects hierarchy (object.cpp),
 * while engine performs building of surface's node, clip and tile lists,
 * custom per-side light/shadow and reflection/refraction surface lists.
 *
 * Both update and render support multi-threading and use array of SceneThread
 * objects to separate working data sets and therefore avoid thread locking.
 */

/******************************************************************************/
/******************************   STATE-LOGGING   *****************************/
/******************************************************************************/

#define RT_PRINT_STATE_BEG()                                                \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("************** print state beg **************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n")

#define RT_PRINT_TIME(time)                                                 \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("---------- update time -- %08u ----------", time);         \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n")

/*
 * Print camera properties.
 */
static
rt_void print_cam(rt_pstr mgn, rt_ELEM *elm, rt_Object *obj)
{
    RT_LOGI("%s", mgn);
    RT_LOGI("cam: %08X, ", (rt_word)obj);
    if (obj != RT_NULL)
    {
        RT_LOGI("    ");
        RT_LOGI("rot: {%f, %f, %f}",
            obj->trm->rot[RT_X], obj->trm->rot[RT_Y], obj->trm->rot[RT_Z]);
        RT_LOGI("    ");
        RT_LOGI("pos,rad: {%f, %f, %f}, %f",
            obj->pos[RT_X], obj->pos[RT_Y], obj->pos[RT_Z], obj->box->rad);
    }
    else
    {
        RT_LOGI("    ");
        RT_LOGI("empty object");
    }
    RT_LOGI("\n");
}

#define RT_PRINT_CAM(cam)                                                   \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("******************* CAMERA ******************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_cam("    ", RT_NULL, cam);                                    \
        RT_LOGI("\n")

/*
 * Print light properties.
 */
static
rt_void print_lgt(rt_pstr mgn, rt_ELEM *elm, rt_Object *obj)
{
    RT_LOGI("%s", mgn);
    RT_LOGI("lgt: %08X, ", (rt_word)obj);
    if (obj != RT_NULL)
    {
        RT_LOGI("    ");
        RT_LOGI("                                    ");
        RT_LOGI("    ");
        RT_LOGI("pos,rad: {%f, %f, %f}, %f",
            obj->pos[RT_X], obj->pos[RT_Y], obj->pos[RT_Z], obj->box->rad);
    }
    else
    {
        RT_LOGI("    ");
        RT_LOGI("empty object");
    }
    RT_LOGI("\n");
}

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

static
rt_pstr tags[RT_TAG_SURFACE_MAX] =
{
    "PL", "CL", "SP", "CN", "PB", "HB"
};

static
rt_pstr nodes[] =
{
    "tr",
    "bv",
    "xx",
    "xx",
};

static
rt_pstr sides[] =
{
    "out of range",
    "data = inner",
    "data = 0    ",
    "data = outer",
    "out of range",
};

static
rt_pstr markers[] =
{
    "out of range",
    "accum marker: enter",
    "empty object",
    "accum marker: leave",
    "out of range",
};

/*
 * Print surface/array properties.
 */
static
rt_void print_srf(rt_pstr mgn, rt_ELEM *elm, rt_Object *obj)
{
    RT_LOGI("%s", mgn);

    rt_cell d = elm != RT_NULL ? elm->data : 0;
    rt_cell i = RT_MAX(0, d + 2), t = RT_GET_FLG(d);
    rt_real r = 0.0f;

    if (elm != RT_NULL && elm->temp != RT_NULL)
    {
        r = ((rt_BOUND *)elm->temp)->rad;
    }
    if (obj != RT_NULL)
    {
        if (RT_IS_ARRAY(obj))
        {
            RT_LOGI("arr: %08X, ", (rt_word)obj);
            RT_LOGI("    ");
            RT_LOGI("tag: AR, trm: %d, data = %08X %s ",
                obj->obj_has_trm,
                ((rt_BOUND *)RT_GET_PTR(d)->temp)->obj, nodes[t]);
        }
        else
        {
            RT_LOGI("srf: %08X, ", (rt_word)obj);
            RT_LOGI("    ");
            RT_LOGI("tag: %s, trm: %d, %s       ",
                tags[obj->tag], obj->obj_has_trm,
                sides[RT_MIN(i, RT_ARR_SIZE(sides) - 1)]);
        }
        RT_LOGI("    ");
        RT_LOGI("pos,rad: {%f, %f, %f}, %f",
            obj->pos[RT_X], obj->pos[RT_Y], obj->pos[RT_Z], r);
    }
    else
    {
        RT_LOGI("obj: %08X, ", (rt_word)obj);
        RT_LOGI("    ");
        RT_LOGI("%s", markers[RT_MIN(i, RT_ARR_SIZE(markers) - 1)]);
    }
    RT_LOGI("\n");
}

#define RT_PRINT_SRF(srf)                                                   \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("****************** SURFACE ******************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_srf("    ", RT_NULL, srf);                                    \
        RT_LOGI("\n")

/*
 * Print list of objects.
 */
static
rt_void print_lst(rt_pstr mgn, rt_ELEM *elm)
{
    for (; elm != RT_NULL; elm = elm->next)
    {
        rt_Object *obj = elm->temp == RT_NULL ? RT_NULL :
                         (rt_Object *)((rt_BOUND *)elm->temp)->obj;

        if (obj != RT_NULL && RT_IS_LIGHT(obj))
        {
            print_lgt(mgn, elm, obj);
        }
        else
        {
            print_srf(mgn, elm, obj);
        }
    }
}

#define RT_PRINT_CLP(lst)                                                   \
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

#define RT_PRINT_LGT_LST(lst)                                               \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lgt --------------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define RT_PRINT_SRF_LST(lst)                                               \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- srf --------------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define RT_PRINT_TLS_LST(lst, i, j)                                         \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("********* screen tiles[%2d][%2d] list: ********", i, j);   \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define RT_PRINT_STATE_END()                                                \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("************** print state end **************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
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
    /* estimates are done in Scene once all counters have been initialized */
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
        if (txmax[cy] < x1)                                                 \
        {                                                                   \
            txmax[cy] = RT_MIN(x1, xmax);                                   \
        }                                                                   \
    }                                                                       \
}                                                                           \
while (0) /* "do {...} while (0)" to enforce semicolon ";" at the end */

/*
 * Update surface's projected bbox boundaries in the tilebuffer
 * by processing one bbox edge at a time, bbox faces are not used.
 * The tilebuffer is reset for every surface from outside of this function.
 */
rt_void rt_SceneThread::tiling(rt_vec2 p1, rt_vec2 p2)
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
 * Return outer-most new element (not always list's head).
 */
rt_ELEM* rt_SceneThread::insert(rt_Object *obj, rt_ELEM **ptr, rt_Surface *srf)
{
    rt_ELEM *elm = RT_NULL;

    if (srf == RT_NULL && RT_IS_LIGHT(obj))
    {
        rt_Light *lgt = (rt_Light *)obj;

        /* alloc new element for lgt */
        elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
        elm->data = (rt_cell)scene->slist; /* all srf are potential shadows */
        elm->simd = lgt->s_lgt;
        elm->temp = lgt->box;
        /* insert element as list's head */
        elm->next = *ptr;
       *ptr = elm;
    }

    if (srf == RT_NULL)
    {
        return elm;
    }

    /* alloc new element for srf */
    elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
    elm->data = 0;
    elm->simd = srf->s_srf;
    elm->temp = srf->box;

    /* search matching existing trnode/bvnode for insertion,
     * run through the list hierarchy to find the inner-most node,
     * node's "simd" field holds pointer to node's sublist
     * along with node's type in the lower 4 bits (trnode/bvnode) */
    rt_ELEM *nxt, *lst = srf->trn;

#if RT_OPTS_VARRAY != 0
    if ((scene->opts & RT_OPTS_VARRAY) != 0)
    {
        lst = srf->top;
    }
#endif /* RT_OPTS_VARRAY */

#if RT_OPTS_TILING != 0
    if ((scene->opts & RT_OPTS_TILING) != 0
    &&  RT_IS_CAMERA(obj))
    {
        lst = srf->trn;
    }
#endif /* RT_OPTS_TILING */

    for (nxt = RT_GET_PTR(*ptr); nxt != RT_NULL && lst != RT_NULL;)
    {
        if (nxt->temp == lst->temp)
        {
            /* set insertion point to existing node's sublist */
            ptr = RT_GET_ADR(nxt->simd);
            /* search next inner node in existing node's sublist */
            nxt = RT_GET_PTR(*ptr);
            lst = lst->next;
        }
        else
        {
            nxt = nxt->next;
        }
    }
    /* search above is desgined in such a way, that contents
     * of one array node can now be split across the boundary
     * of another array node by inserting two different node
     * elements of the same type belonging to the same array,
     * one into another array node's sublist and one outside,
     * this allows for greater flexibility in trnode/bvnode
     * relationship, something not allowed in previous versions */

    /* allocate new node elements from outer-most to inner-most
     * if they are not already in the list */
    rt_ELEM **org = RT_NULL;

    for (; lst != RT_NULL; lst = lst->next)
    {
        /* alloc new trnode/bvnode element as none has been found */
        nxt = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
        nxt->data = lst->data;
        nxt->simd = lst->simd; /* node's type */
        nxt->temp = lst->temp;
        /* insert element according to found position */
        nxt->next = RT_GET_PTR(*ptr);
        RT_SET_PTR(*ptr, rt_ELEM *, nxt);
        if (org == RT_NULL)
        {
            org = ptr;
        }
        /* set insertion point to new node's sublist */
        ptr = RT_GET_ADR(nxt->simd);
    }

    /* insert element according to found position */
    elm->next = RT_GET_PTR(*ptr);
    RT_SET_PTR(*ptr, rt_ELEM *, elm);
    /* prepare outer-most new element for sorting
     * in order to figure out its optimal position in the list
     * and thus reduce potential overdraw in the rendering backend,
     * as array's bounding volume is final at this point
     * it is correct to pass it through the sorting routine below
     * before other elements are added into its node's sublist */
    if (org != RT_NULL)
    {
        ptr = org;
        elm = RT_GET_PTR(*ptr);
    }

    /* sort nodes in the list "ptr" with the new element "elm"
     * based on the bounding volume order as seen from "obj",
     * sorting is always applied to a single flat list
     * whether it's a top-level list or array node's sublist
     * treating both surface and array nodes in that list
     * as single whole elements, thus sorting never violates
     * the boundaries of the array node sublists as they are
     * determined by the search/insert algorithm above */
#if RT_OPTS_INSERT == 1
    if ((scene->opts & RT_OPTS_INSERT) == 0)
#endif /* RT_OPTS_INSERT */
    {
        return elm;
    }

    /* "state" helps to avoid stored order value re-computation
     * when the whole sublist is being moved (one element at a time),
     * the term sublist used here and below refers to a continuous portion
     * of a single flat list as opposed to the same term used above
     * to separate different layers of the list hierarchy */
    rt_cell state = 0;
    rt_ELEM *prv = RT_NULL;

    /* phase 1, push "elm" through the list "ptr" for as long as possible,
     * order value re-computation is avoided via "state" variable */
    for (nxt = elm->next; nxt != RT_NULL; )
    {
        /* compute the order value between "elm" and "nxt" elements */
        rt_cell op = bbox_sort(obj->box,
                              (rt_BOUND *)elm->temp,
                              (rt_BOUND *)nxt->temp);
        switch (op)
        {
            /* move "elm" forward if the "op" is
             * either "do swap" or "neutral" */
            case 4:
            /* as the swap operation is performed below
             * the stored order value becomes "don't swap" */
            op = 3;
            case 1:
            elm->next = nxt->next;
            if (prv != RT_NULL)
            {
                if (state != 0)
                {
                    prv->data = state;
                }
                else
                {
                    prv->data = bbox_sort(obj->box,
                                         (rt_BOUND *)prv->temp,
                                         (rt_BOUND *)nxt->temp);
                }
                prv->next = nxt;
            }
            else
            {
                RT_SET_PTR(*ptr, rt_ELEM *, nxt);
            }
            /* if current "elm" position is transitory, "state" keeps
             * previously computed order value between "prv" and "nxt",
             * thus the order value can be restored to "prv" data field
             * without re-computation as the "elm" advances further */
            state = nxt->data;
            nxt->data = op;
            nxt->next = elm;
            prv = nxt;
            nxt = elm->next;
            break;

            /* stop phase 1 if the "op" is
             * either "don't swap" or "unsortable" */
            default:
            elm->data = op;
            /* reset "state" as the "elm" has found its place */
            state = 0;
            nxt = RT_NULL;
            break;
        }
    }

    rt_ELEM *end, *tlp, *cur, *ipt, *jpt;

    /* phase 2, find the "end" of the strict-order-chain from "elm",
     * order values "don't swap" and "unsortable" are considered strict */
    for (end = elm; end->data == 3 || end->data == 2; end = end->next);

    /* phase 3, move the elements from behind "elm" strict-order-chain
     * right in front of the "elm" as computed order value dictates,
     * order value re-computation is avoided via "state" variables */
    for (tlp = end, nxt = end->next; nxt != RT_NULL; )
    {
        rt_bool gr = RT_FALSE;
        /* compute the order value between "elm" and "nxt" elements */
        rt_cell op = bbox_sort(obj->box,
                              (rt_BOUND *)elm->temp,
                              (rt_BOUND *)nxt->temp);
        switch (op)
        {
            /* move "nxt" in front of the "elm"
             * if the "op" is "do swap" */
            case 4:
            /* as the swap operation is performed below
             * the stored order value becomes "don't swap" */
            op = 3;
            /* check if there is a tail from "end->next"
             * up to "tlp" to comb out thoroughly before
             * moving "nxt" (along with its strict-order-chain
             * from "tlp->next") to the front of the "elm" */
            if (tlp != end)
            {
                /* local "state" helps to avoid stored order value
                 * re-computation for tail's elements joining the comb */
                rt_cell state = 0;
                cur = tlp;
                /* run through the tail area from "end->next"
                 * up to "tlp" backwards, while combing out
                 * elements to move along with "nxt" */
                while (cur != end)
                {
                    rt_bool mv = RT_FALSE;
                    /* search for "cur" previous element,
                     * can be optimized out for dual-linked list,
                     * though the runtime overhead of managing dual-linked list
                     * can easily overweight the added benefit, use ptr packed
                     * into "data" field with "op" as prev if needed, the same
                     * way as "simd" stores array node's sublist with type */
                    for (ipt = end; ipt->next != cur; ipt = ipt->next);
                    rt_ELEM *iel = ipt->next;
                    /* run through the strict-order-chain from "tlp->next"
                     * up to "nxt" (which serves as a comb for the tail)
                     * and compute new order values for each tail element */
                    for (jpt = tlp; jpt != nxt; jpt = jpt->next)
                    {
                        rt_cell op = 0;
                        rt_ELEM *jel = jpt->next;
                        /* if "tlp" stored order value to the first
                         * comb element is not reset, use it as "op",
                         * "cur" serves as "tlp" */
                        if (cur->next == jel && cur->data != 0)
                        {
                            op = cur->data;
                        }
                        else
                        /* if "state" is not reset, it stores the order value
                         * from "cur" to the first comb element (last moved),
                         * use it as "op" */
                        if (tlp->next == jel && state != 0)
                        {
                            op = state;
                        }
                        /* compute new order value */
                        else
                        {
                            op = bbox_sort(obj->box,
                                          (rt_BOUND *)cur->temp,
                                          (rt_BOUND *)jel->temp);
                        }
                        /* repair "tlp" stored order value to the first
                         * comb element, "cur" serves as "tlp" */
                        if (cur->next == jel)
                        {
                            cur->data = op;
                        }
                        else
                        /* remember "cur" computed order value to the first
                         * comb element in the "state", if "cur" != "tlp" */
                        if (tlp->next == jel)
                        {
                            state = op;
                        }
                        /* check if order is strict, then stop
                         * and mark "cur" as moving with "nxt",
                         * "cur" will then be added to the comb */
                        if (op == 3 || op == 2)
                        {
                            mv = RT_TRUE;
                            break;
                        }
                    }
                    /* check if "cur" needs to move (join the comb) */
                    if (mv == RT_TRUE)
                    {
                        gr = RT_TRUE;
                        /* check if "cur" was the last tail's element,
                         * then tail gets shorten by one element "cur",
                         * which at the same time joins the comb */
                        if (cur == tlp)
                        {
                            /* move "tlp" to its prev, its stored order value
                             * is always repaired in the combing stage above */
                            tlp = ipt;
                        }
                        /* move "cur" from the middle of the tail
                         * to the front of the comb, "iel" serves
                         * as "cur" and "ipt" serves as "cur" prev */
                        else
                        {
                            cur = tlp->next;
                            iel->data = state;
                            /* local "state" keeps previously computed order
                             * value between "cur" and the front of the comb,
                             * thus the order value can be restored to
                             * "cur" data field without re-computation */
                            state = ipt->data;
                            ipt->data = 0;
                            ipt->next = iel->next;
                            iel->next = cur;
                            tlp->data = 0;
                            tlp->next = iel;
                        }
                    }
                    /* "cur" doesn't move (stays in the tail) */
                    else
                    {
                        /* repair "cur" stored order value before it
                         * moves to its prev, "iel" serves as "cur" */
                        if (iel->data == 0)
                        {
                            cur = iel->next;
                            iel->data = bbox_sort(obj->box,
                                                 (rt_BOUND *)iel->temp,
                                                 (rt_BOUND *)cur->temp);
                        }
                        /* reset local "state" as tail's sublist
                         * (joining the comb) is being broken */
                        state = 0;
                    }
                    /* move "cur" to its prev */
                    cur = ipt;
                }
                /* repair "end" stored order value (to the rest of the tail),
                 * "ipt" serves as "end" */
                if (ipt->data == 0)
                {
                    cur = ipt->next;
                    ipt->data = bbox_sort(obj->box,
                                         (rt_BOUND *)ipt->temp,
                                         (rt_BOUND *)cur->temp);
                }
            }
            /* reset "state" if the comb has grown with tail elements, thus
             * breaking the sublist moving to the front of the "elm" */
            if (gr == RT_TRUE)
            {
                state = 0;
            }
            /* move "nxt" along with its comb (if any)
             * from "tlp->next" to the front of the "elm" */
            cur = tlp->next;
            if (prv != RT_NULL)
            {
                if (state != 0)
                {
                    prv->data = state;
                }
                else
                {
                    prv->data = bbox_sort(obj->box,
                                         (rt_BOUND *)prv->temp,
                                         (rt_BOUND *)cur->temp);
                }
                prv->next = cur;
            }
            else
            {
                RT_SET_PTR(*ptr, rt_ELEM *, cur);
            }
            cur = nxt->next;
            tlp->data = 0;
            tlp->next = cur;
            /* "state" keeps previously computed order value between "nxt"
             * and "nxt->next", thus the order value can be restored to
             * "prv" data field without re-computation if the whole
             * sublist is being moved from "nxt" to the front of the "elm" */
            state = nxt->data;
            nxt->data = op;
            nxt->next = elm;
            prv = nxt;
            nxt = cur;
            break;

            /* move "nxt" forward if the "op" is
             * "don't swap", "neutral" or "unsortable" */
            default:
            /* if "nxt" stored order value (to "nxt->next")
             * is "neutral", then strict-order-chain
             * from "tlp->next" up to "nxt" is being broken
             * as "nxt" moves, thus "tlp" catches up with "nxt" */
            if (nxt->data != 3 && nxt->data != 2)
            {
                /* repair "tlp" stored order value
                 * before it catches up with "nxt" */
                if (tlp->data == 0)
                {
                    cur = tlp->next;
                    tlp->data = bbox_sort(obj->box,
                                         (rt_BOUND *)tlp->temp,
                                         (rt_BOUND *)cur->temp);
                }
                /* reset "state" as "tlp" moves forward, thus
                 * breaking the sublist moving to the front of the "elm" */
                state = 0;
                /* move "tlp" to "nxt" before it advances */
                tlp = nxt;
            }
            /* when "nxt" runs away from "tlp" it grows a
             * strict-order-chain from "tlp->next" up to "nxt",
             * which then serves as a comb for the tail area
             * from "end->next" up to "tlp" */
            nxt = nxt->next;
            break;
        }
    }
    /* repair "tlp" stored order value
     * if there are elements left behind it */
    cur = tlp->next;
    if (tlp->data == 0 && cur != RT_NULL)
    {
        tlp->data = bbox_sort(obj->box,
                             (rt_BOUND *)tlp->temp,
                             (rt_BOUND *)cur->temp);
    }

    return elm;
}

/*
 * Filter list "ptr" for a given object "obj" by
 * converting hierarchical sorted sublists back into
 * a single flat list suitable for rendering backend,
 * while cleaning "data" and "simd" fields at the same time.
 * Return last leaf element of the list hierarchy (recursive).
 */
rt_ELEM* rt_SceneThread::filter(rt_Object *obj, rt_ELEM **ptr)
{
    rt_ELEM *elm = RT_NULL, *nxt;

    if (ptr == RT_NULL)
    {
        return elm;
    }

    for (nxt = RT_GET_PTR(*ptr); nxt != RT_NULL; nxt = nxt->next)
    {
        /* only node elements are allowed in surface lists */
        rt_Node *nd = (rt_Node *)((rt_BOUND *)nxt->temp)->obj;

        /* if the list element is surface,
         * reset "data" field used as stored order value
         * in sorting to keep it clean for the backend */
        if (RT_IS_SURFACE(nd))
        {
            elm = nxt;
            nxt->data = 0;
        }
        else
        /* if the list element is array,
         * find the last leaf element of its sublist hierarchy
         * and set it to the "data" field along with node's type,
         * previously kept in its "simd" field's lower 4 bits */
        if (RT_IS_ARRAY(nd))
        {
            ptr = RT_GET_ADR(nxt->simd);
            elm = filter(obj, ptr);
            elm->next = nxt->next;
            rt_cell k = RT_GET_FLG(*ptr);
            nxt->data = (rt_cell)elm | k; /* node's type */
            nxt->next = RT_GET_PTR(*ptr);
            nxt->simd = k == 0 ? nd->s_srf : ((rt_Array *)nd)->s_box;
            nxt = elm;
        }
    }

    return elm;
}

/*
 * Build trnode/bvnode sequence for a given surface "srf"
 * after all transform flags have been set in update_fields,
 * so that trnode elements are handled properly.
 */
rt_void rt_SceneThread::snode(rt_Surface *srf)
{
    /* as temporary memory pool is released after every frame,
     * always rebuild the list even if the surface hasn't changed */

    /* reset surface's trnode/bvnode sequence */
    srf->top = RT_NULL;
    srf->trn = RT_NULL;

    /* build surface's trnode/bvnode sequence,
     * trnodes hierarchy is flat as objects with
     * non-trivial transform are their own trnodes,
     * while bvnodes can have arbitrary depth
     * above and below the trnode (if present) */
    rt_ELEM *elm;
    rt_Object *par;

    /* phase 1, bvnodes (if any) below trnode (if any),
     * if the same array serves as both trnode and bvnode,
     * bvnode is considered above, thus trnode is inserted first */
    for (par = srf->bvnode; par != RT_NULL &&
         par->trnode == srf->trnode && par->trnode != par;
         par = par->bvnode)
    {
        elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
        elm->data = 0;
        elm->simd = (rt_pntr)1; /* node's type (bv) */
        elm->temp = par->box;
        elm->next = srf->top;
        srf->top = elm;
    }

    /* phase 2, there can only be one trnode (if any),
     * even though there might be other trnodes
     * above and below in the objects hierarchy
     * they themselves don't form the hierarchy
     * as any trnode is always its own trnode */
    if (srf->trnode != RT_NULL && srf->trnode != srf)
    {
        rt_Array *arr = (rt_Array *)srf->trnode;
        rt_BOUND *aux = arr == par ? arr->inbox : arr->trbox;

        elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
        elm->data = 0;
        elm->simd = (rt_pntr)0; /* node's type (tr) */
        elm->temp = aux;
        elm->next = srf->top;
        srf->top = elm;

        elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
        elm->data = 0;
        elm->simd = (rt_pntr)0; /* node's type (tr) */
        elm->temp = aux;
        elm->next = RT_NULL;
        srf->trn = elm;
    }

    /* phase 3, bvnodes (if any) above trnode (if any) */
    for (; par != RT_NULL; par = par->bvnode)
    {
        elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
        elm->data = 0;
        elm->simd = (rt_pntr)1; /* node's type (bv) */
        elm->temp = par->box;
        elm->next = srf->top;
        srf->top = elm;
    }
}

/*
 * Build custom clippers list from "srf" relations template
 * after all transform flags have been set in update_fields,
 * so that trnode elements are handled properly.
 */
rt_void rt_SceneThread::sclip(rt_Surface *srf)
{
    /* as temporary memory pool is released after every frame,
     * always rebuild the list even if the surface hasn't changed */

    /* init surface's relations template */
    rt_ELEM *lst = srf->rel;

    /* init and reset custom clippers list */
    rt_ELEM **ptr = RT_GET_ADR(srf->s_srf->msc_p[2]);
   *ptr = RT_NULL;

    /* build custom clippers list from given template "lst",
     * as given template "lst" is inverted in surface's add_relation
     * and elements are inserted into the list's head here,
     * the original relations template from scene data is inverted twice,
     * thus accum enter/leave markers will end up in correct order */
    for (; lst != RT_NULL; lst = lst->next)
    {
        rt_ELEM *elm;
        rt_cell rel = lst->data;
        rt_Object *obj = lst->temp == RT_NULL ? RT_NULL :
                         (rt_Object *)((rt_BOUND *)lst->temp)->obj;

        if (obj == RT_NULL)
        {
            /* alloc new element for accum marker */
            elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
            elm->data = rel;
            elm->simd = RT_NULL; /* accum marker */
            elm->temp = RT_NULL;
            /* insert element as list's head */
            elm->next = *ptr;
           *ptr = elm;
        }
        else
        if (RT_IS_SURFACE(obj))
        {
            rt_Surface *srf = (rt_Surface *)obj;

            /* alloc new element for srf */
            elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
            elm->data = rel;
            elm->simd = srf->s_srf;
            elm->temp = srf->box;

            if (srf->trnode != RT_NULL && srf->trnode != srf)
            {
                rt_cell acc = 0;
                rt_ELEM *nxt;

                rt_Array *arr = (rt_Array *)srf->trnode;
                rt_BOUND *aux = arr->trbox; /* bound is not used in clippers */

                /* search matching existing trnode for insertion
                 * either within current accum segment
                 * or outside of any accum segment */
                for (nxt = *ptr; nxt != RT_NULL; nxt = nxt->next)
                {
                    /* (acc == 0) either accum-enter-marker
                     * hasn't been inserted yet (current accum segment)
                     * or outside of any accum segment */
                    if (acc == 0
                    &&  nxt->temp == aux)
                    {
                        break;
                    }

                    /* skip all non-accum-marker elements */
                    if (nxt->temp != RT_NULL)
                    {
                        continue;
                    }

                    /* didn't find trnode within current accum segment,
                     * leaving cycle, new trnode element will be inserted */
                    if (acc == 0
                    &&  nxt->data == RT_ACCUM_LEAVE)
                    {
                        nxt = RT_NULL;
                        break;
                    }

                    /* skip accum segment different from the one
                     * current element is being inserted into */
                    if (acc == 0
                    &&  nxt->data == RT_ACCUM_ENTER)
                    {
                        acc = 1;
                    }

                    /* keep track of accum segments */
                    if (acc == 1
                    &&  nxt->data == RT_ACCUM_LEAVE)
                    {
                        acc = 0;
                    }
                }

                if (nxt == RT_NULL)
                {
                    /* insert element as list's head */
                    elm->next = *ptr;
                   *ptr = elm;

                    /* alloc new trnode element as none has been found */
                    nxt = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
                    nxt->data = (rt_cell)elm; /* trnode's last elem */
                    nxt->simd = arr->s_srf;
                    nxt->temp = aux;
                    /* insert element as list's head */
                    nxt->next = *ptr;
                   *ptr = nxt;
                }
                else
                {
                    /* insert element under existing trnode */
                    elm->next = nxt->next;
                    nxt->next = elm;
                }
            }
            else
            {
                /* insert element as list's head */
                elm->next = *ptr;
               *ptr = elm;
            }
        }
    }
}

/*
 * Build tile list for a given surface "srf" based
 * on the area its projected bbox occupies in the tilebuffer.
 */
rt_void rt_SceneThread::stile(rt_Surface *srf)
{
    /* as temporary memory pool is released after every frame,
     * always rebuild the list even if the surface hasn't changed */

    srf->s_srf->msc_p[0] = RT_NULL;

#if RT_OPTS_TILING != 0
    if ((scene->opts & RT_OPTS_TILING) == 0)
#endif /* RT_OPTS_TILING */
    {
        return;
    }

    rt_ELEM *elm;
    rt_cell i, j;
    rt_cell k;

    rt_vec4 vec;
    rt_real dot;
    rt_cell ndx[2];
    rt_real tag[2], zed[2];

    /* verts_num may grow, use srf->verts_num if original is needed */
    rt_cell verts_num = srf->shp->verts_num;
    rt_VERT *vrt = srf->shp->verts;

    /* project bbox onto the tilebuffer */
    if (verts_num != 0)
    {
        for (i = 0; i < scene->tiles_in_col; i++)
        {
            txmin[i] = scene->tiles_in_row;
            txmax[i] = -1;
        }

        /* process bbox vertices */
        memset(verts, 0, sizeof(rt_VERT) *
                         (2 * verts_num + srf->shp->edges_num));

        for (k = 0; k < srf->shp->verts_num; k++)
        {
            RT_VEC3_SUB(vec, vrt[k].pos, scene->org);

            dot = RT_VEC3_DOT(vec, scene->nrm);

            verts[k].pos[RT_Z] = dot;
            verts[k].pos[RT_W] = -1.0f; /* tag: behind screen plane */

            /* process vertices in front of or near screen plane,
             * the rest are processed with edges */
            if (dot >= 0.0f || RT_FABS(dot) <= RT_CLIP_THRESHOLD)
            {
                RT_VEC3_SUB(vec, vrt[k].pos, scene->pos);

                dot = RT_VEC3_DOT(vec, scene->nrm) / scene->cam->pov;

                RT_VEC3_MUL_VAL1(vec, vec, 1.0f / dot);
                /* dot >= (pov - RT_CLIP_THRESHOLD) */
                /* pov >= (2  *  RT_CLIP_THRESHOLD) */
                /* thus: (dot >= RT_CLIP_THRESHOLD) */

                RT_VEC3_SUB(vec, vec, scene->dir);

                verts[k].pos[RT_X] = RT_VEC3_DOT(vec, scene->htl);
                verts[k].pos[RT_Y] = RT_VEC3_DOT(vec, scene->vtl);

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
        for (k = 0; k < srf->shp->edges_num; k++)
        {
            for (i = 0; i < 2; i++)
            {
                ndx[i] = srf->shp->edges[k].index[i];
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

                /* process edge with one in front of
                 * and one behind screen plane vertices */
                j = 1 - i;

                /* clip edge at screen plane crossing,
                 * generate new vertex */
                RT_VEC3_SUB(vec, vrt[ndx[i]].pos, vrt[ndx[j]].pos);

                dot = zed[j] / (zed[j] - zed[i]); /* () >= RT_CLIP_THRESHOLD */

                RT_VEC3_MUL_VAL1(vec, vec, dot);

                RT_VEC3_ADD(vec, vec, vrt[ndx[j]].pos);
                RT_VEC3_SUB(vec, vec, scene->org);

                verts[verts_num].pos[RT_X] = RT_VEC3_DOT(vec, scene->htl);
                verts[verts_num].pos[RT_Y] = RT_VEC3_DOT(vec, scene->vtl);

                ndx[i] = verts_num;
                verts_num++;
            }

            /* tile current edge */
            tiling(verts[ndx[0]].pos, verts[ndx[1]].pos); 
        }

        /* tile all newly generated vertex pairs */
        for (i = srf->shp->verts_num; i < verts_num - 1; i++)
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

    rt_ELEM **ptr = RT_GET_ADR(srf->s_srf->msc_p[0]);

    /* fill marked tiles with surface data */
    for (i = 0; i < scene->tiles_in_col; i++)
    {
        for (j = txmin[i]; j <= txmax[i]; j++)
        {
            /* alloc new element for each srf tile */
            elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
            elm->data = i << 16 | j;
            elm->simd = srf->s_srf;
            elm->temp = srf->box;
            /* insert element as list's tail */
           *ptr = elm;
            ptr = &elm->next;
        }
    }

   *ptr = RT_NULL;
}

/*
 * Build surface lists for a given object "obj".
 * Surfaces have separate surface lists for each side.
 */
rt_ELEM* rt_SceneThread::ssort(rt_Object *obj)
{
    /* as temporary memory pool is released after every frame,
     * always rebuild the list even if the object hasn't changed */

    rt_Surface *srf = RT_NULL;
    rt_ELEM **pto = RT_NULL;
    rt_ELEM **pti = RT_NULL;

    if (RT_IS_SURFACE(obj))
    {
        srf = (rt_Surface *)obj;

        if (g_print && srf->s_srf->msc_p[2] != RT_NULL)
        {
            RT_PRINT_CLP((rt_ELEM *)srf->s_srf->msc_p[2]);
        }

        pto = RT_GET_ADR(srf->s_srf->lst_p[1]);
        pti = RT_GET_ADR(srf->s_srf->lst_p[3]);

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
            rt_cell c = bbox_side(ref->box, srf->shp);

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

#if RT_OPTS_INSERT != 0 || RT_OPTS_TARRAY != 0 || RT_OPTS_VARRAY != 0
    if ((scene->opts & RT_OPTS_INSERT) != 0
    ||  (scene->opts & RT_OPTS_TARRAY) != 0
    ||  (scene->opts & RT_OPTS_VARRAY) != 0)
    {
        if (pto != RT_NULL && *pto != RT_NULL)
        {
            filter(obj, pto);
        }
        if (pti != RT_NULL && *pti != RT_NULL)
        {
            filter(obj, pti);
        }
        if (*ptr != RT_NULL)
        {
            filter(obj, ptr);
        }
    }
#endif /* RT_OPTS_INSERT, RT_OPTS_TARRAY, RT_OPTS_VARRAY */

    if (srf == RT_NULL)
    {
        return lst;
    }

    if (g_print)
    {
        if (*pto != RT_NULL)
        {
            RT_PRINT_LST_OUTER(*pto);
        }
        if (*pti != RT_NULL)
        {
            RT_PRINT_LST_INNER(*pti);
        }
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
    }

    return RT_NULL;
}

/*
 * Build light/shadow lists for a given object "obj".
 * Surfaces have separate light/shadow lists for each side.
 */
rt_ELEM* rt_SceneThread::lsort(rt_Object *obj)
{
    /* as temporary memory pool is released after every frame,
     * always rebuild the list even if the object hasn't changed */

    rt_Surface *srf = RT_NULL;
    rt_ELEM **pto = RT_NULL;
    rt_ELEM **pti = RT_NULL;

    if (RT_IS_SURFACE(obj))
    {
        srf = (rt_Surface *)obj;

        pto = RT_GET_ADR(srf->s_srf->lst_p[0]);
        pti = RT_GET_ADR(srf->s_srf->lst_p[2]);

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
        rt_ELEM **pso = RT_NULL;
        rt_ELEM **psi = RT_NULL;

#if RT_OPTS_2SIDED != 0
        if ((scene->opts & RT_OPTS_2SIDED) != 0 && srf != RT_NULL)
        {
            rt_cell c = bbox_side(lgt->box, srf->shp);

            if (c & 2)
            {
                insert(lgt, pto, RT_NULL);

                pso = RT_GET_ADR((*pto)->data);
               *pso = RT_NULL;

                if (g_print)
                {
                    RT_PRINT_LGT_OUTER(*pto, lgt);
                }
            }
            if (c & 1)
            {
                insert(lgt, pti, RT_NULL);

                psi = RT_GET_ADR((*pti)->data);
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

            psr = RT_GET_ADR((*ptr)->data);

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
            if (bbox_shad(lgt->box, shw->box, srf->box) == 0)
            {
                continue;
            }

#if RT_OPTS_2SIDED != 0
            if ((scene->opts & RT_OPTS_2SIDED) != 0)
            {
                rt_cell c = bbox_side(shw->box, srf->shp);

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

#if RT_OPTS_INSERT != 0 || RT_OPTS_TARRAY != 0 || RT_OPTS_VARRAY != 0
        if ((scene->opts & RT_OPTS_INSERT) != 0
        ||  (scene->opts & RT_OPTS_TARRAY) != 0
        ||  (scene->opts & RT_OPTS_VARRAY) != 0)
        {
            if (pso != RT_NULL && *pso != RT_NULL)
            {
                filter(lgt, pso);
            }
            if (psi != RT_NULL && *psi != RT_NULL)
            {
                filter(lgt, psi);
            }
            if (psr != RT_NULL && *psr != RT_NULL)
            {
                filter(lgt, psr);
            }
        }
#endif /* RT_OPTS_INSERT, RT_OPTS_TARRAY, RT_OPTS_VARRAY */

        if (g_print)
        {
            if (pso != RT_NULL && *pso != RT_NULL)
            {
                RT_PRINT_SHW_OUTER(*pso);
            }
            if (psi != RT_NULL && *psi != RT_NULL)
            {
                RT_PRINT_SHW_INNER(*psi);
            }
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
    }

    return RT_NULL;
}

/*
 * Destroy scene thread.
 */
rt_SceneThread::~rt_SceneThread()
{

}

/******************************************************************************/
/*****************************   MULTI-THREADING   ****************************/
/******************************************************************************/

/*
 * Initialize platform-specific pool of "thnum" threads.
 * Local stub below is used when platform threading functions are not provided
 * or during state-logging.
 */
static
rt_void* init_threads(rt_cell thnum, rt_Scene *scn)
{
    return scn;
}

/*
 * Terminate platform-specific pool of "thnum" threads.
 * Local stub below is used when platform threading functions are not provided
 * or during state-logging.
 */
static
rt_void term_threads(rt_void *tdata, rt_cell thnum)
{

}

/*
 * Task platform-specific pool of "thnum" threads to update scene,
 * block until finished.
 * Local stub below is used when platform threading functions are not provided
 * or during state-logging. Simulate threading with sequential run.
 */
static
rt_void update_scene(rt_void *tdata, rt_cell thnum, rt_cell phase)
{
    rt_Scene *scn = (rt_Scene *)tdata;

    rt_cell i;

    for (i = 0; i < thnum; i++)
    {
        scn->update_slice(i, phase);
    }
}

/*
 * Task platform-specific pool of "thnum" threads to render scene,
 * block until finished.
 * Local stub below is used when platform threading functions are not provided
 * or during state-logging. Simulate threading with sequential run.
 */
static
rt_void render_scene(rt_void *tdata, rt_cell thnum, rt_cell phase)
{
    rt_Scene *scn = (rt_Scene *)tdata;

    rt_cell i;

    for (i = 0; i < thnum; i++)
    {
        scn->render_slice(i, phase);
    }
}

/******************************************************************************/
/**********************************   SCENE   *********************************/
/******************************************************************************/

/*
 * Instantiate scene.
 * Can only be called from single (main) thread.
 */
rt_Scene::rt_Scene(rt_SCENE *scn, /* frame ptr must be SIMD-aligned or NULL */
                   rt_word x_res, rt_word y_res, rt_cell x_row, rt_word *frame,
                   rt_FUNC_ALLOC f_alloc, rt_FUNC_FREE f_free,
                   rt_FUNC_INIT f_init, rt_FUNC_TERM f_term,
                   rt_FUNC_UPDATE f_update,
                   rt_FUNC_RENDER f_render,
                   rt_FUNC_PRINT_LOG f_print_log,
                   rt_FUNC_PRINT_ERR f_print_err) : 

    rt_LogRedirect(f_print_log, f_print_err), /* must be first in scene init */
    rt_Registry(f_alloc, f_free)
{
    this->scn = scn;

    /* check for locked scene data, not thread safe! */
    if (scn->lock != RT_NULL)
    {
        throw rt_Exception("scene data is locked by another instance");
    }

    /* x_row, frame's stride (in 32-bit pixels, not bytes!),
     * can be greater than x_res, in which case the frame
     * occupies only a portion (rectangle) of the framebuffer,
     * or negative, in which case frame starts at the last line
     * and consecutive lines are located backwards in memory,
     * x_row must contain the whole number of SIMD widths */
    if (x_res == 0 || RT_ABS(x_row) < x_res
    ||  y_res == 0 || RT_ABS(x_row) & (RT_SIMD_WIDTH - 1))
    {
        throw rt_Exception("frambuffer's dimensions are not valid");
    }

    /* init framebuffer's dimensions and pointer */
    this->x_res = x_res;
    this->y_res = y_res;
    this->x_row = x_row;

    if (frame == RT_NULL)
    {
        frame = (rt_word *)
                alloc(RT_ABS(x_row) * y_res * sizeof(rt_word), RT_SIMD_ALIGN);

        if (x_row < 0)
        {
            frame += RT_ABS(x_row) * (y_res - 1);
        }
    }
    else
    if ((rt_word)frame & (RT_SIMD_ALIGN - 1) != 0)
    {
        throw rt_Exception("frame pointer is not simd-aligned in scene");
    }

    this->frame = frame;

    /* init tilebuffer's dimensions and pointer */
    tile_w = RT_MAX(RT_TILE_W, 1);
    tile_h = RT_MAX(RT_TILE_H, 1);

    tile_w = ((tile_w + RT_SIMD_WIDTH - 1) / RT_SIMD_WIDTH) * RT_SIMD_WIDTH;

    tiles_in_row = (x_res + tile_w - 1) / tile_w;
    tiles_in_col = (y_res + tile_h - 1) / tile_h;

    tiles = (rt_ELEM **)
            alloc(sizeof(rt_ELEM *) * (tiles_in_row * tiles_in_col), RT_ALIGN);

    /* init aspect ratio, rays depth and fsaa */
    factor = 1.0f / (rt_real)x_res;
    aspect = (rt_real)y_res * factor;

    depth = RT_MAX(RT_STACK_DEPTH, 0);
    fsaa  = RT_FSAA_NO;

    /* instantiate objects hierarchy */
    memset(&rootobj, 0, sizeof(rt_OBJECT));

    rootobj.trm.scl[RT_I] = 1.0f;
    rootobj.trm.scl[RT_J] = 1.0f;
    rootobj.trm.scl[RT_K] = 1.0f;
    rootobj.obj = scn->root;

    if (scn->root.tag != RT_TAG_ARRAY)
    {
        throw rt_Exception("scene's root is not an array");
    }

    root = new rt_Array(this, RT_NULL, &rootobj); /* also init *_num fields */

    if (cam_head == RT_NULL)
    {
        throw rt_Exception("scene doesn't contain camera");
    }

    cam = cam_head;

    /* lock scene data, when scene constructor can no longer fail */
    scn->lock = this;

    /* create scene threads array */
    thnum = RT_MAX(RT_THREADS_NUM, 1);

    tharr = (rt_SceneThread **)
            alloc(sizeof(rt_SceneThread *) * thnum, RT_ALIGN);

    rt_cell i;

    for (i = 0; i < thnum; i++)
    {
        tharr[i] = new rt_SceneThread(this, i);

        /* estimate per-frame allocs to reduce system calls per thread */
        tharr[i]->msize =  /* upper bound per surface for tiling */
            (tiles_in_row * tiles_in_col + /* plus array nodes list */
            (arr_num + 2) +     /* plus reflections/refractions */
            (srf_num + arr_num * 2 + /* plus lights and shadows */
            (srf_num + arr_num * 2 + 1) * lgt_num) * 2) * /* for both sides */
            sizeof(rt_ELEM) * (srf_num + thnum - 1) / thnum; /* per thread */
    }

    /* init memory pool in the heap for temporary per-frame allocs */
    mpool = RT_NULL; /* rough estimate for surface relations/templates */
    msize = ((srf_num + 1) * (srf_num + 1) * 2 + /* plus two surface lists */
            (srf_num + arr_num * 1) * 2 + /* plus lights and shadows list */
            (srf_num + arr_num * 2 + 1) * lgt_num + /* plus array nodes */
            tiles_in_row * tiles_in_col * arr_num) *  /* for tiling */
            sizeof(rt_ELEM);                        /* for main thread */

    /* in the estimates above (arr_num * x) depends on whether both
     * trnode/bvnode are allowed in the list or just one of them,
     * if the estimates are not accurate the engine would still work,
     * though not as efficient due to unnecessary allocations per frame */

    /* init threads management functions */
    if (f_init != RT_NULL && f_term != RT_NULL
    &&  f_update != RT_NULL && f_render != RT_NULL)
    {
        this->f_init = f_init;
        this->f_term = f_term;
        this->f_update = f_update;
        this->f_render = f_render;
    }
    else
    {
        this->f_init = init_threads;
        this->f_term = term_threads;
        this->f_update = update_scene;
        this->f_render = render_scene;
    }

    /* create platform-specific worker threads */
    tdata = this->f_init(thnum, this);

    /* init rendering backend */
    render0(tharr[0]->s_inf);
}

/*
 * Update current camera with given "action" for a given "time".
 */
rt_void rt_Scene::update(rt_long time, rt_cell action)
{
    cam->update_action(time, action);
}

/*
 * Update backend data structures and render frame for a given "time".
 */
rt_void rt_Scene::render(rt_long time)
{
    rt_cell i;

    /* reserve memory for temporary per-frame allocs */
    mpool = reserve(msize, RT_QUAD_ALIGN);

    for (i = 0; i < thnum; i++)
    {
        tharr[i]->mpool = tharr[i]->reserve(tharr[i]->msize, RT_QUAD_ALIGN);
    }

    /* print state beg */
    if (g_print)
    {
        RT_PRINT_STATE_BEG();
        RT_PRINT_TIME(time);
    }

    /* phase 0.5, hierarchical update of arrays' transform matrices */
    root->update_object(time, 0, RT_NULL, iden4);

    /* 1st phase of multi-threaded update */
#if RT_OPTS_THREAD != 0
    if ((opts & RT_OPTS_THREAD) != 0 && !g_print)
    {
        this->f_update(tdata, thnum, 1);
    }
    else
#endif /* RT_OPTS_THREAD */
    {
        update_scene(this, thnum, 1);
    }

    /* update rays positioning and steppers */
    rt_real h, v;

    RT_VEC3_SET(pos, cam->pos);
    RT_VEC3_SET(hor, cam->hor);
    RT_VEC3_SET(ver, cam->ver);
    RT_VEC3_SET(nrm, cam->nrm);

    h = -0.5f * 1.0f;
    v = -0.5f * aspect;

    /* aim rays at camera's top-left corner */
    RT_VEC3_MUL_VAL1(dir, nrm, cam->pov);
    RT_VEC3_MAD_VAL1(dir, hor, h);
    RT_VEC3_MAD_VAL1(dir, ver, v);

    /* update tiles positioning and steppers */
    RT_VEC3_ADD(org, pos, dir);

    h = 1.0f / (factor * tile_w); /* x_res / tile_w */
    v = 1.0f / (factor * tile_h); /* x_res / tile_h */

    RT_VEC3_MUL_VAL1(htl, hor, h);
    RT_VEC3_MUL_VAL1(vtl, ver, v);

    if (g_print)
    {
        RT_PRINT_CAM(cam);
    }

    /* 2nd phase of multi-threaded update */
#if RT_OPTS_THREAD != 0
    if ((opts & RT_OPTS_THREAD) != 0 && !g_print)
    {
        this->f_update(tdata, thnum, 2);
    }
    else
#endif /* RT_OPTS_THREAD */
    {
        update_scene(this, thnum, 2);
    }

    /* phase 2.5, hierarchical update of arrays' bounds from surfaces */
    root->update_bounds();

    /* rebuild camera's surface list */
    slist = tharr[0]->ssort(cam);

    /* rebuild camera's light/shadow list,
     * slist is needed inside */
    llist = tharr[0]->lsort(cam);

    if (g_print)
    {
        RT_PRINT_LGT_LST(llist);
        RT_PRINT_SRF_LST(slist);
    }

    /* 3rd phase of multi-threaded update */
#if RT_OPTS_THREAD != 0
    if ((opts & RT_OPTS_THREAD) != 0 && !g_print)
    {
        this->f_update(tdata, thnum, 3);
    }
    else
#endif /* RT_OPTS_THREAD */
    {
        update_scene(this, thnum, 3);
    }

    /* screen tiling */
    rt_cell tline;
    rt_cell j;

#if RT_OPTS_TILING != 0
    if ((opts & RT_OPTS_TILING) != 0)
    {
        memset(tiles, 0, sizeof(rt_ELEM *) * tiles_in_row * tiles_in_col);

        rt_ELEM *elm, *nxt, *stail = RT_NULL, **ptr = &stail;

        /* build exact copy of reversed slist (should be cheap),
         * trnode elements become tailing rather than heading,
         * elements grouping for cached transform is retained */
        for (nxt = slist; nxt != RT_NULL; nxt = nxt->next)
        {
            /* alloc new element as nxt copy */
            elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
            elm->data = nxt->data;
            elm->simd = nxt->simd;
            elm->temp = nxt->temp;
            /* insert element as list's head */
            elm->next = *ptr;
           *ptr = elm;
        }

        /* traverse reversed slist to keep original slist order
         * and optimize trnode handling for each tile */
        for (elm = stail; elm != RT_NULL; elm = elm->next)
        {
            rt_Node *nd = (rt_Node *)((rt_BOUND *)elm->temp)->obj;

            /* skip trnode elements from reversed slist
             * as they are handled separately for each tile */
            if (RT_IS_ARRAY(nd))
            {
                continue;
            }

            rt_Surface *srf = (rt_Surface *)nd;

            rt_ELEM *tls = (rt_ELEM *)srf->s_srf->msc_p[0], *trn;

            if (srf->trnode != RT_NULL && srf->trnode != srf)
            {
                for (; tls != RT_NULL; tls = nxt)
                {
                    i = (rt_word)tls->data >> 16;
                    j = (rt_word)tls->data & 0xFFFF;

                    nxt = tls->next;

                    tls->data = 0;

                    tline = i * tiles_in_row;

                    /* check matching existing trnode for insertion,
                     * only tile list's head needs to be checked as elements
                     * grouping for cached transform is retained from slist */
                    trn = tiles[tline + j];

                    rt_Array *arr = (rt_Array *)srf->trnode;
                    rt_BOUND *aux = (rt_BOUND *)srf->trn->temp;

                    if (trn != RT_NULL && trn->temp == aux)
                    {
                        /* insert element under existing trnode */
                        tls->next = trn->next;
                        trn->next = tls;
                    }
                    else
                    {
                        /* insert element as list's head */
                        tls->next = tiles[tline + j];
                        tiles[tline + j] = tls;

                        /* alloc new trnode element as none has been found */
                        trn = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
                        trn->data = (rt_cell)tls; /* trnode's last elem */
                        trn->simd = arr->s_srf;
                        trn->temp = aux;
                        /* insert element as list's head */
                        trn->next = tiles[tline + j];
                        tiles[tline + j] = trn;
                    }
                }
            }
            else
            {
                for (; tls != RT_NULL; tls = nxt)
                {
                    i = (rt_word)tls->data >> 16;
                    j = (rt_word)tls->data & 0xFFFF;

                    nxt = tls->next;

                    tls->data = 0;

                    tline = i * tiles_in_row;

                    /* insert element as list's head */
                    tls->next = tiles[tline + j];
                    tiles[tline + j] = tls;
                }
            }
        }

        if (g_print)
        {
            rt_cell i = 0, j = 0;

            tline = i * tiles_in_row;

            RT_PRINT_TLS_LST(tiles[tline + j], i, j);
        }
    }
    else
#endif /* RT_OPTS_TILING */
    {
        for (i = 0; i < tiles_in_col; i++)
        {
            tline = i * tiles_in_row;

            for (j = 0; j < tiles_in_row; j++)
            {
                tiles[tline + j] = slist;
            }
        }
    }

    /* aim rays at pixel centers */
    RT_VEC3_MUL_VAL1(hor, hor, factor);
    RT_VEC3_MUL_VAL1(ver, ver, factor);

    RT_VEC3_MAD_VAL1(dir, hor, 0.5f);
    RT_VEC3_MAD_VAL1(dir, ver, 0.5f);

    /* accumulate ambient from camera and all light sources */
    RT_VEC3_MUL_VAL1(amb, cam->cam->col.hdr, cam->cam->lum[0]);

    rt_Light *lgt = RT_NULL;

    for (lgt = lgt_head; lgt != RT_NULL; lgt = lgt->next)
    {
        RT_VEC3_MAD_VAL1(amb, lgt->lgt->col.hdr, lgt->lgt->lum[0]);
    }

    /* multi-threaded render */
#if RT_OPTS_THREAD != 0
    if ((opts & RT_OPTS_THREAD) != 0)
    {
        this->f_render(tdata, thnum, 0);
    }
    else
#endif /* RT_OPTS_THREAD */
    {
        render_scene(this, thnum, 0);
    }

    /* print state end */
    if (g_print)
    {
        RT_PRINT_STATE_END();
        g_print = RT_FALSE;
    }

    /* release memory from temporary per-frame allocs */
    for (i = 0; i < thnum; i++)
    {
        tharr[i]->release(tharr[i]->mpool);
    }

    release(mpool);
}

/*
 * Update portion of the scene with given "index"
 * as part of the multi-threaded update.
 */
rt_void rt_Scene::update_slice(rt_cell index, rt_cell phase)
{
    rt_cell i;

    rt_Array   *arr;
    rt_Camera  *cam;
    rt_Light   *lgt;
    rt_Surface *srf;

    if (phase == 1)
    {
        for (arr = arr_head, i = 0; arr != RT_NULL; arr = arr->next, i++)
        {
            if ((i % thnum) != index)
            {
                continue;
            }

            /* update array's fields from transform matrix
             * updated in sequential phase 0.5 */
            arr->update_fields();
        }

        for (cam = cam_head, i = 0; cam != RT_NULL; cam = cam->next, i++)
        {
            if ((i % thnum) != index)
            {
                continue;
            }

            /* update camera's fields and transform matrix
             * from parent array's transform matrix
             * updated in sequential phase 0.5 */
            cam->update_fields();
        }

        for (lgt = lgt_head, i = 0; lgt != RT_NULL; lgt = lgt->next, i++)
        {
            if ((i % thnum) != index)
            {
                continue;
            }

            /* update light's fields and transform matrix
             * from parent array's transform matrix
             * updated in sequential phase 0.5 */
            lgt->update_fields();
        }

        for (srf = srf_head, i = 0; srf != RT_NULL; srf = srf->next, i++)
        {
            if ((i % thnum) != index)
            {
                continue;
            }

            /* update surface's fields and transform matrix
             * from parent array's transform matrix
             * updated in sequential phase 0.5 */
            srf->update_fields();

            /* rebuild surface's node list (per-surface)
             * based on transform flags updated above */
            tharr[index]->snode(srf);
        }
    }
    else
    if (phase == 2)
    {
        for (srf = srf_head, i = 0; srf != RT_NULL; srf = srf->next, i++)
        {
            if ((i % thnum) != index)
            {
                continue;
            }

            /* rebuild surface's clip list (cross-surface)
             * based on transform flags updated in 1st phase above */
            tharr[index]->sclip(srf);

            /* update surface's bounds taking into account surfaces
             * from custom clippers list updated above */
            srf->update_bounds();

            /* rebuild surface's tile list (per-surface)
             * based on surface bounds updated above */
            tharr[index]->stile(srf);
        }
    }
    else
    if (phase == 3)
    {
        for (srf = srf_head, i = 0; srf != RT_NULL; srf = srf->next, i++)
        {
            if ((i % thnum) != index)
            {
                continue;
            }

            if (g_print)
            {
                RT_PRINT_SRF(srf);
            }

            /* rebuild surface's rfl/rfr surface lists (cross-surface)
             * based on surface bounds updated in 2nd phase above
             * and array bounds updated in sequential phase 2.5 */
            tharr[index]->ssort(srf);

            /* rebuild surface's light/shadow lists (cross-surface)
             * based on surface bounds updated in 2nd phase above
             * and array bounds updated in sequential phase 2.5 */
            tharr[index]->lsort(srf);

            /* update surface's backend-related parts */
            update0(srf->s_srf);
        }
    }
}

/*
 * Render portion of the frame with given "index"
 * as part of the multi-threaded render.
 */
rt_void rt_Scene::render_slice(rt_cell index, rt_cell phase)
{
    /* adjust ray steppers according to anti-aliasing mode */
    rt_real fdh[4], fdv[4];
    rt_real fhr, fvr;

    if (fsaa == RT_FSAA_4X)
    {
        rt_real as = 0.25f;
        rt_real ar = 0.08f;

        fdh[0] = (-ar-as);
        fdh[1] = (-ar+as);
        fdh[2] = (+ar-as);
        fdh[3] = (+ar+as);

        fdv[0] = (+ar-as) + (rt_real)index;
        fdv[1] = (-ar-as) + (rt_real)index;
        fdv[2] = (+ar+as) + (rt_real)index;
        fdv[3] = (-ar+as) + (rt_real)index;

        fhr = 1.0f;
        fvr = (rt_real)thnum;
    }
    else
    {
        fdh[0] = 0.0f;
        fdh[1] = 1.0f;
        fdh[2] = 2.0f;
        fdh[3] = 3.0f;

        fdv[0] = (rt_real)index;
        fdv[1] = (rt_real)index;
        fdv[2] = (rt_real)index;
        fdv[3] = (rt_real)index;

        fhr = 4.0f;
        fvr = (rt_real)thnum;
    }

/*  rt_SIMD_CAMERA */

    rt_SIMD_CAMERA *s_cam = tharr[index]->s_cam;

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

    RT_SIMD_SET(s_cam->clamp, 255.0f);
    RT_SIMD_SET(s_cam->cmask, 0xFF);

    RT_SIMD_SET(s_cam->col_r, amb[RT_R]);
    RT_SIMD_SET(s_cam->col_g, amb[RT_G]);
    RT_SIMD_SET(s_cam->col_b, amb[RT_B]);

/*  rt_SIMD_CONTEXT */

    rt_SIMD_CONTEXT *s_ctx = tharr[index]->s_ctx;

    RT_SIMD_SET(s_ctx->t_min, cam->pov);

    RT_SIMD_SET(s_ctx->org_x, pos[RT_X]);
    RT_SIMD_SET(s_ctx->org_y, pos[RT_Y]);
    RT_SIMD_SET(s_ctx->org_z, pos[RT_Z]);

/*  rt_SIMD_INFOX */

    rt_SIMD_INFOX *s_inf = tharr[index]->s_inf;

    s_inf->ctx = s_ctx;
    s_inf->cam = s_cam;
    s_inf->lst = slist;

    s_inf->index = index;
    s_inf->thnum = thnum;
    s_inf->depth = depth;
    s_inf->fsaa  = fsaa;

   /* render frame based on tilebuffer */
    render0(s_inf);
}

/*
 * Return pointer to the framebuffer.
 */
rt_word* rt_Scene::get_frame()
{
    return frame;
}

/*
 * Print current state.
 */
rt_void rt_Scene::print_state()
{
    g_print = RT_TRUE;
}

/*
 * Set fullscreen anti-aliasing mode.
 */
rt_void rt_Scene::set_fsaa(rt_cell fsaa)
{
    this->fsaa = fsaa;
}

/*
 * Set runtime optimization flags.
 */
rt_void rt_Scene::set_opts(rt_cell opts)
{
    this->opts = opts;

    /* trigger full hierarchy update,
     * safe to reset time as rootobj never has animator,
     * rootobj time is restored within the update */
    rootobj.time = -1;
}

/*
 * Select next camera in the list.
 */
rt_void rt_Scene::next_cam()
{
    if (cam->next != RT_NULL)
    {
        cam = cam->next;
    }
    else
    {
        cam = cam_head;
    }
}

/*
 * Save current frame.
 */
rt_void rt_Scene::save_frame(rt_cell index)
{
    rt_char name[] = "scrXXX.bmp";

    name[5] = '0' + (index % 10);
    index /= 10;
    name[4] = '0' + (index % 10);
    index /= 10;
    name[3] = '0' + (index % 10);

    rt_TEX tex;
    tex.ptex = get_frame();
    tex.x_dim = +x_res;
    tex.y_dim = -y_res;

    save_image(this, name, &tex);
}

/*
 * Destroy scene.
 */
rt_Scene::~rt_Scene()
{
    /* destroy worker threads */
    this->f_term(tdata, thnum);

    rt_cell i;

    for (i = 0; i < thnum; i++)
    {
        delete tharr[i];
    }

    /* destroy objects hierarchy */
    delete root;

    /* destroy textures */
    while (tex_head)
    {
        rt_Texture *tex = tex_head->next;
        delete tex_head;
        tex_head = tex;
    }

    /* unlock scene data */
    scn->lock = RT_NULL;
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
 * Render given number "num" on the screen at given coords "x" and "y".
 * Parameters "d" and "z" specify direction and zoom respectively.
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
    rt_word *src, *dst;

    for (i = 0; i < c; i++)
    {
        k = arr[i];
        src = &digits[k][0][0];
        dst = frame + y * x_row + x + (c * d - 1 - i) * dW * z;

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
                dst += x_row - dW * z;
                src -= dW;
            }
            src += dW;
        }
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
