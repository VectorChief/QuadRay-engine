/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <string.h>

#include "engine.h"
#include "rtgeom.h"
#include "system.h"
#include "tracer.h"

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
rt_void print_cam(rt_pstr mgn, rt_ELEM *elm, rt_Object *obj)
{
    RT_LOGI("%s", mgn);
    RT_LOGI("cam: %08X, ", (rt_word)obj);
    RT_LOGI("CAM: %08X, ", (rt_word)RT_NULL);
    RT_LOGI("elm: %08X, ", (rt_word)elm);
    if (obj != RT_NULL)
    {
        RT_LOGI("    ");
        RT_LOGI("rot: {%f, %f, %f}",
            obj->trm->rot[RT_X], obj->trm->rot[RT_Y], obj->trm->rot[RT_Z]);
        RT_LOGI("    ");
        RT_LOGI("pos: {%f, %f, %f}",
            obj->pos[RT_X], obj->pos[RT_Y], obj->pos[RT_Z]);
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
rt_void print_lgt(rt_pstr mgn, rt_ELEM *elm, rt_Object *obj)
{
    rt_SIMD_LIGHT *s_lgt = elm != RT_NULL ? (rt_SIMD_LIGHT *)elm->simd :
                           obj != RT_NULL ?   ((rt_Light *)obj)->s_lgt :
                                                                RT_NULL;
    RT_LOGI("%s", mgn);
    RT_LOGI("lgt: %08X, ", (rt_word)obj);
    RT_LOGI("LGT: %08X, ", (rt_word)s_lgt);
    RT_LOGI("elm: %08X, ", (rt_word)elm);
    if (s_lgt != RT_NULL)
    {
        RT_LOGI("    ");
        RT_LOGI("                                    ");
        RT_LOGI("    ");
        RT_LOGI("pos: {%f, %f, %f}",
            s_lgt->pos_x[0], s_lgt->pos_y[0], s_lgt->pos_z[0]);
    }
    else
    {
        RT_LOGI("    ");
        RT_LOGI("empty object");
    }
    RT_LOGI("\n");
}

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
rt_void print_srf(rt_pstr mgn, rt_ELEM *elm, rt_Object *obj)
{
    rt_SIMD_SURFACE *s_srf = elm != RT_NULL ? (rt_SIMD_SURFACE *)elm->simd :
                             obj != RT_NULL ?      ((rt_Node *)obj)->s_srf :
                                                                    RT_NULL;
    RT_LOGI("%s", mgn);
    RT_LOGI("srf: %08X, ", (rt_word)obj);
    RT_LOGI("SRF: %08X, ", (rt_word)s_srf);
    RT_LOGI("elm: %08X, ", (rt_word)elm);

    rt_cell d = elm != RT_NULL ? elm->data : 0;
    rt_cell i = RT_MAX(0, d + 2), n = d & 0x3;

    if (s_srf != RT_NULL && obj != RT_NULL)
    {
        if (RT_IS_ARRAY(obj))
        {
            RT_LOGI("    ");
            RT_LOGI("tag: AR, trm: %d, data = %08X %s ",
                s_srf->a_map[3], d & 0xFFFFFFFC, nodes[n]);
        }
        else
        {
            RT_LOGI("    ");
            RT_LOGI("tag: %s, trm: %d, %s       ",
                tags[obj->tag], s_srf->a_map[3],
                sides[RT_MIN(i, RT_ARR_SIZE(sides) - 1)]);
        }
        RT_LOGI("    ");
        RT_LOGI("pos: {%f, %f, %f}",
            obj->pos[RT_X], obj->pos[RT_Y], obj->pos[RT_Z]);
    }
    else
    {
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
rt_void print_lst(rt_pstr mgn, rt_ELEM *elm)
{
    for (; elm != RT_NULL; elm = elm->next)
    {
        rt_Object *obj = (rt_Object *)elm->temp;

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
/*****************************   MULTI-THREADING   ****************************/
/******************************************************************************/

/*
 * Initialize platform-specific handle for pool of "thnum" threads.
 * Local stub for when platform threading functions are not provided.
 */
static
rt_void* init_threads(rt_cell thnum, rt_Scene *scn)
{
    return scn;
}

/*
 * Terminate platform-specific handle for pool of "thnum" threads.
 * Local stub for when platform threading functions are not provided.
 */
static
rt_void term_threads(rt_void *tdata, rt_cell thnum)
{

}

/*
 * Tell platform-specific pool of "thnum" threads to update scene,
 * block until finished. Simulate threading with sequential run.
 * Local stub for when platform threading functions are not provided.
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
 * Tell platform-specific pool of "thnum" threads to render scene,
 * block until finished. Simulate threading with sequential run.
 * Local stub for when platform threading functions are not provided.
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
rt_Scene::rt_Scene(rt_SCENE *scn, /* frame must be SIMD-aligned or NULL */
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

    /* init framebuffer's dimensions and pointer */
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
    if ((rt_word)frame & (RT_SIMD_ALIGN - 1) != 0)
    {
        throw rt_Exception("frame pointer is not simd-aligned in scene");
    }

    this->frame = frame;

    /* init tilebuffer's dimensions and pointer */
    tile_w = RT_TILE_W;
    tile_h = RT_TILE_H;

    tiles_in_row = (x_res + tile_w - 1) / tile_w;
    tiles_in_col = (y_res + tile_h - 1) / tile_h;

    tiles = (rt_ELEM **)
            alloc(sizeof(rt_ELEM *) * (tiles_in_row * tiles_in_col), RT_ALIGN);

    /* init aspect ratio, rays depth and fsaa */
    factor = 1.0f / (rt_real)x_res;
    aspect = (rt_real)y_res * factor;

    depth = RT_STACK_DEPTH;
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

    root = new rt_Array(this, RT_NULL, &rootobj); /* init srf_num */

    if (cam_head == RT_NULL)
    {
        throw rt_Exception("scene doesn't contain camera");
    }

    cam = cam_head;

    /* lock scene data, when scene constructor can no longer fail */
    scn->lock = this;

    /* create scene threads array */
    thnum = RT_THREADS_NUM;

    tharr = (rt_SceneThread **)
            alloc(sizeof(rt_SceneThread *) * thnum, RT_ALIGN);

    rt_cell i;

    for (i = 0; i < thnum; i++)
    {
        tharr[i] = new rt_SceneThread(this, i);

        /* estimate per frame allocs to reduce system calls per thread */
        tharr[i]->msize =  /* upper bound per surface for tiling */
            (tiles_in_row * tiles_in_col + /* plus reflections/refractions */
            2 * (srf_num + arr_num * 2) + /* plus lights and shadows */
            2 * (lgt_num * (1 + srf_num + arr_num * 2))) * /* per side */
            sizeof(rt_ELEM) * (srf_num + thnum - 1) / thnum; /* per thread */
    }

    /* init memory pool in the heap for temporary per-frame allocs */
    mpool = RT_NULL; /* rough estimate for surface relations/templates */
    msize = ((srf_num + 1) * (srf_num + 1) * 2 + /* plus two surface lists */
            2 * (srf_num + arr_num) + /* plus one lights and shadows list */
            lgt_num * (1 + srf_num + arr_num * 2) + /* plus array nodes */
            tiles_in_row * tiles_in_col * arr_num) *  /* for tiling */
            sizeof(rt_ELEM);                        /* for main thread */

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
    cam->update(time, action);
}

/*
 * Update backend data structures and render frame for a given "time".
 */
rt_void rt_Scene::render(rt_long time)
{
    rt_cell i;

    /* reserve memory for temporary per-frame allocs */
    mpool = reserve(msize, RT_ALIGN);

    for (i = 0; i < thnum; i++)
    {
        tharr[i]->mpool = tharr[i]->reserve(tharr[i]->msize, RT_ALIGN);
    }

    /* print state beg */
    if (g_print)
    {
        RT_PRINT_STATE_BEG();

        RT_PRINT_TIME(time);
    }

    /* update the whole objects hierarchy */
    root->update(time, iden4, RT_UPDATE_FLAG_OBJ);

    /* update rays positioning and steppers */
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

    /* aim rays at camera's top-left corner */
    dir[RT_X] = (nrm[RT_X] * cam->pov - (hor[RT_X] * h + ver[RT_X] * v) * 0.5f);
    dir[RT_Y] = (nrm[RT_Y] * cam->pov - (hor[RT_Y] * h + ver[RT_Y] * v) * 0.5f);
    dir[RT_Z] = (nrm[RT_Z] * cam->pov - (hor[RT_Z] * h + ver[RT_Z] * v) * 0.5f);

    /* update tiles positioning and steppers */
    org[RT_X] = (pos[RT_X] + dir[RT_X]);
    org[RT_Y] = (pos[RT_Y] + dir[RT_Y]);
    org[RT_Z] = (pos[RT_Z] + dir[RT_Z]);

            h = (1.0f / (factor * tile_w)); /* x_res / tile_w */
            v = (1.0f / (factor * tile_h)); /* x_res / tile_h */

    htl[RT_X] = (hor[RT_X] * h);
    htl[RT_Y] = (hor[RT_Y] * h);
    htl[RT_Z] = (hor[RT_Z] * h);

    vtl[RT_X] = (ver[RT_X] * v);
    vtl[RT_Y] = (ver[RT_Y] * v);
    vtl[RT_Z] = (ver[RT_Z] * v);

    /* multi-threaded update */
#if RT_OPTS_THREAD != 0
    if ((opts & RT_OPTS_THREAD) != 0 && g_print == RT_FALSE)
    {
        this->f_update(tdata, thnum, 1);
    }
    else
#endif /* RT_OPTS_THREAD */
    {
        if (g_print)
        {
            RT_PRINT_CAM(cam);
        }

        update_scene(this, thnum, 1);
    }

    root->update_bounds();

    /* rebuild global surface list */
    slist = tharr[0]->ssort(cam);

    /* rebuild global light/shadow list,
     * slist is needed inside */
    llist = tharr[0]->lsort(cam);

#if RT_OPTS_THREAD != 0
    if ((opts & RT_OPTS_THREAD) != 0 && g_print == RT_FALSE)
    {
        this->f_update(tdata, thnum, 2);
    }
    else
#endif /* RT_OPTS_THREAD */
    {
        if (g_print)
        {
            RT_PRINT_LGT_LST(llist);

            RT_PRINT_SRF_LST(slist);
        }

        update_scene(this, thnum, 2);
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
            elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_ALIGN);
            elm->data = nxt->data;
            elm->simd = nxt->simd;
            elm->temp = nxt->temp;
            /* insert element as list head */
            elm->next = *ptr;
           *ptr = elm;

        }

        /* traverse reversed slist to keep original slist order
         * and optimize trnode handling for each tile */
        for (elm = stail; elm != RT_NULL; elm = elm->next)
        {
            rt_Object *obj = (rt_Object *)elm->temp;

            /* skip trnode elements from reversed slist
             * as they are handled separately for each tile */
            if (RT_IS_ARRAY(obj))
            {
                continue;
            }

            rt_Surface *srf = (rt_Surface *)elm->temp;

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
                     * as elements grouping for cached transform is retained
                     * from slist, only tile list head needs to be checked */
                    trn = tiles[tline + j];

                    if (trn != RT_NULL && trn->temp == srf->trnode)
                    {
                        /* insert element under existing trnode */
                        tls->next = trn->next;
                        trn->next = tls;
                    }
                    else
                    {
                        /* insert element as list head */
                        tls->next = tiles[tline + j];
                        tiles[tline + j] = tls;

                        rt_Array *arr = (rt_Array *)srf->trnode;

                        /* alloc new trnode element as none has been found */
                        trn = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_ALIGN);
                        trn->data = (rt_cell)tls; /* trnode's last element */
                        trn->simd = arr->s_srf;
                        trn->temp = arr;
                        /* insert element as list head */
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

                    /* insert element as list head */
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
    hor[RT_X] *= factor;
    hor[RT_Y] *= factor;
    hor[RT_Z] *= factor;

    ver[RT_X] *= factor;
    ver[RT_Y] *= factor;
    ver[RT_Z] *= factor;

    dir[RT_X] += (hor[RT_X] + ver[RT_X]) * 0.5f;
    dir[RT_Y] += (hor[RT_Y] + ver[RT_Y]) * 0.5f;
    dir[RT_Z] += (hor[RT_Z] + ver[RT_Z]) * 0.5f;

    /* accumulate ambient from camera and all light sources */
    amb[RT_R] = cam->cam->col.hdr[RT_R] * cam->cam->lum[0];
    amb[RT_G] = cam->cam->col.hdr[RT_G] * cam->cam->lum[0];
    amb[RT_B] = cam->cam->col.hdr[RT_B] * cam->cam->lum[0];

    rt_Light *lgt = RT_NULL;

    for (lgt = lgt_head; lgt != RT_NULL; lgt = lgt->next)
    {
        amb[RT_R] += lgt->lgt->col.hdr[RT_R] * lgt->lgt->lum[0];
        amb[RT_G] += lgt->lgt->col.hdr[RT_G] * lgt->lgt->lum[0];
        amb[RT_B] += lgt->lgt->col.hdr[RT_B] * lgt->lgt->lum[0];
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

    rt_Surface *srf = RT_NULL;

    if (phase == 1)
    {
        for (srf = srf_head, i = 0; srf != RT_NULL; srf = srf->next, i++)
        {
            if ((i % thnum) != index)
            {
                continue;
            }

            srf->update(0, RT_NULL, RT_UPDATE_FLAG_SRF);

            /* rebuild per-surface tile list */
            tharr[index]->stile(srf);
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

            if (g_print)
            {
                RT_PRINT_SRF(srf);
            }

            /* rebuild per-surface surface lists */
            tharr[index]->ssort(srf);

            /* rebuild per-surface light/shadow lists */
            tharr[index]->lsort(srf);

            /* update per-surface backend-related parts */
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

        fdv[0] = (+ar-as) + index;
        fdv[1] = (-ar-as) + index;
        fdv[2] = (+ar+as) + index;
        fdv[3] = (-ar+as) + index;

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
 * Print current state.
 */
rt_void rt_Scene::print_state()
{
    g_print = RT_TRUE;
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
