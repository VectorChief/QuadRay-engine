/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <string.h>

#include "engine.h"
#include "rtimag.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * engine.cpp: Implementation of the scene manager.
 *
 * Main file of the engine responsible for instantiating and managing scenes.
 * It contains definitions of Platform, SceneThread and Scene classes along with
 * a set of algorithms needed to process objects in the scene in order
 * to prepare data structures used by the rendering backend (tracer.cpp).
 *
 * Processing of objects consists of two major steps: update and render,
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
 * Some parts of the update are handled by the object hierarchy (object.cpp),
 * while engine performs building of surface's node, clip and tile lists,
 * custom per-side light/shadow and reflection/refraction surface lists.
 *
 * Both update and render support multi-threading and use array of SceneThread
 * objects to separate working datasets and therefore avoid thread locking.
 */

/******************************************************************************/
/******************************   STATE-LOGGING   *****************************/
/******************************************************************************/

#define RT_PRINT_STATE_INIT()                                               \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("************** print state init *************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n")

#define RT_PRINT_TIME(time)                                                 \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("---- update time -- %020" PR_Z "d ----", time);            \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n")

#define RT_PRINT_GLB()                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("******************* GLOBAL ******************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
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
        RT_LOGI("rot: {%+.25e, %+.25e, %+.25e}\n",
            obj->trm->rot[RT_X], obj->trm->rot[RT_Y], obj->trm->rot[RT_Z]);
        RT_LOGI("%s", mgn);
        RT_LOGI("               ");
        RT_LOGI("    ");
        RT_LOGI("pos: {%+.25e, %+.25e, %+.25e}",
            obj->trm->pos[RT_X], obj->trm->pos[RT_Y], obj->trm->pos[RT_Z]);
    }
    else
    {
        RT_LOGI("%s", mgn);
        RT_LOGI("               ");
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
        RT_LOGI("                                             ");
        RT_LOGI("    ");
        RT_LOGI("pos: {%+12.6f,%+12.6f,%+12.6f} rad: %7.2f",
            obj->pos[RT_X], obj->pos[RT_Y], obj->pos[RT_Z], obj->bvbox->rad);
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
    "PL", "CL", "SP", "CN", "PB", "HB", "PC", "HC", "HP"
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
rt_void print_obj(rt_pstr mgn, rt_ELEM *elm, rt_Object *obj)
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
            RT_LOGI("tag: AR, trm: %d, flg: %02X, data = %08X %s ",
                obj->obj_has_trm,
                elm != RT_NULL && elm->temp != RT_NULL ?
                ((rt_BOUND *)elm->temp)->flm : 0,
                ((rt_BOUND *)RT_GET_PTR(d)->temp)->obj, nodes[t]);
        }
        else
        {
            RT_LOGI("srf: %08X, ", (rt_word)obj);
            RT_LOGI("    ");
            RT_LOGI("tag: %s, trm: %d, flg: %02X, %s       ",
                tags[obj->tag], obj->obj_has_trm,
                elm != RT_NULL && elm->temp != RT_NULL ?
                ((rt_BOUND *)elm->temp)->flm : 0,
                sides[RT_MIN(i, RT_ARR_SIZE(sides) - 1)]);
            r = obj->bvbox->rad;
        }
        RT_LOGI("    ");
        RT_LOGI("pos: {%+12.6f,%+12.6f,%+12.6f} rad: %7.2f",
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
        print_obj("    ", RT_NULL, srf);                                    \
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
            print_obj(mgn, elm, obj);
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

#define RT_PRINT_STATE_DONE()                                               \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("************** print state done *************");           \
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
 * Initialize platform-specific pool of "thnum" threads (< 0 - no feedback).
 * Local stub below is used when platform threading functions are not provided
 * or during state-logging.
 */
static
rt_void* init_threads(rt_si32 thnum, rt_Platform *pfm)
{
    if (thnum > 0)
    {
        pfm->set_thnum(thnum); /* dummy feedback, pass core-count in platform */
    }

    return pfm;
}

/*
 * Terminate platform-specific pool of "thnum" threads.
 * Local stub below is used when platform threading functions are not provided
 * or during state-logging.
 */
static
rt_void term_threads(rt_void *tdata, rt_si32 thnum)
{

}

/*
 * Task platform-specific pool of "thnum" threads to update scene,
 * block until finished.
 * Local stub below is used when platform threading functions are not provided
 * or during state-logging. Simulate threading with sequential run.
 */
static
rt_void update_scene(rt_void *tdata, rt_si32 thnum, rt_si32 phase)
{
    rt_Scene *scn;

    if (thnum < 0)
    {
        scn = (rt_Scene *)tdata;
        thnum = -thnum;
    }
    else
    {
        scn = ((rt_Platform *)tdata)->get_cur_scene();
    }

    rt_si32 i;

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
rt_void render_scene(rt_void *tdata, rt_si32 thnum, rt_si32 phase)
{
    rt_Scene *scn;

    if (thnum < 0)
    {
        scn = (rt_Scene *)tdata;
        thnum = -thnum;
    }
    else
    {
        scn = ((rt_Platform *)tdata)->get_cur_scene();
    }

    rt_si32 i;

    for (i = 0; i < thnum; i++)
    {
        scn->render_slice(i, phase);
    }
}

/*
 * Instantiate platform.
 * Can only be called from single (main) thread.
 */
rt_Platform::rt_Platform(rt_FUNC_ALLOC f_alloc, rt_FUNC_FREE f_free,
                         rt_si32 thnum, /* (< 0 - no feedback) */
                         rt_FUNC_INIT f_init, rt_FUNC_TERM f_term,
                         rt_FUNC_UPDATE f_update,
                         rt_FUNC_RENDER f_render,
                         rt_FUNC_PRINT_LOG f_print_log,   /* has global scope */
                         rt_FUNC_PRINT_ERR f_print_err) : /* has global scope */

    rt_LogRedirect(f_print_log, f_print_err), /* must be 1st in platform init */
    rt_Heap(f_alloc, f_free)
{
    /* init scene list variables */
    head = tail = cur = RT_NULL;

    /* allocate root SIMD structure */
    s_inf = (rt_SIMD_INFOX *)
            alloc(sizeof(rt_SIMD_INFOX),
                            RT_SIMD_ALIGN);

    memset(s_inf, 0, sizeof(rt_SIMD_INFOX));

    /* allocate regs SIMD structure */
    rt_SIMD_REGS *s_reg = (rt_SIMD_REGS *)
            alloc(sizeof(rt_SIMD_REGS),
                            RT_SIMD_ALIGN);

    ASM_INIT(s_inf, s_reg)

    /* init thread management functions */
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

    /* init thread management variables */
    thnum = thnum != 0 ? thnum : 1;
    this->thnum = thnum < 0 ? thnum : -thnum; /* always < 0 at first init */
    this->tdata = RT_NULL;

    /* create platform-specific worker threads */
    tdata = this->f_init(thnum, this);
    thnum = this->thnum;
    this->thnum = thnum < 0 ? -thnum : thnum; /* always > 0 upon feedback */

    /* init tile dimensions */
    tile_w = RT_MAX(RT_TILE_W, 1);
    tile_h = RT_MAX(RT_TILE_H, 1);
    tile_w = ((tile_w + RT_SIMD_WIDTH - 1) / RT_SIMD_WIDTH) * RT_SIMD_WIDTH;

    /* init rendering backend,
     * default SIMD runtime target will be chosen */
    fsaa = RT_FSAA_NO;
    set_simd(0);
}

/*
 * Get platform's thread-pool size.
 */
rt_si32 rt_Platform::get_thnum()
{
    return thnum;
}

/*
 * Set platform's thread-pool size (only once after platform's construction).
 */
rt_si32 rt_Platform::set_thnum(rt_si32 thnum)
{
    if (this->thnum < 0)
    {
        this->thnum = thnum;
    }

    return this->thnum;
}

/*
 * Initialize SIMD target-selection variable from parameters.
 */
rt_si32 simd_init(rt_si32 n_simd, rt_si32 s_type, rt_si32 k_size)
{
    /* ------ k_size ------- s_type ------- n_simd ------ */
    return (k_size << 16) | (s_type << 8) | (n_simd);
}

/*
 * When ASM sections are used together with non-trivial logic written in C/C++
 * in the same function, optimizing compilers may produce inconsistent results
 * with optimization levels higher than O0 (tested both clang and g++).
 * Using separate functions for ASM and C/C++ resolves the issue
 * if the ASM function is not inlined (thus keeping it in a separate file).
 */
rt_void simd_version(rt_SIMD_INFOX *s_inf)
{
    ASM_ENTER(s_inf)
        verxx_xx()
    ASM_LEAVE(s_inf)
}

/*
 * Set current runtime SIMD target with "simd" equal to
 * SIMD native-size (1,..,16) in 0th (lowest) byte
 * SIMD type (1,2,4,8, 16,32) in 1st (higher) byte
 * SIMD size-factor (1, 2, 4) in 2nd (higher) byte
 */
rt_si32 rt_Platform::set_simd(rt_si32 simd)
{
    simd = switch0(s_inf, simd);

    simd_quads = (simd & 0xFF) * ((simd >> 16) & 0xFF);
    simd_width = (simd_quads * 128) / RT_ELEMENT;

    /* code below blocks SIMD target switch if incompatible with current AA */

    rt_si32 fsaa = this->fsaa;

    set_fsaa(fsaa);

    if (this->fsaa != fsaa)
    {
        this->fsaa = fsaa;
        simd = this->simd;

        simd = switch0(s_inf, simd);

        simd_quads = (simd & 0xFF) * ((simd >> 16) & 0xFF);
        simd_width = (simd_quads * 128) / RT_ELEMENT;
    }

    /* code above blocks SIMD target switch if incompatible with current AA */

    this->simd = simd;

    return simd;
}

/*
 * Set current antialiasing mode.
 */
rt_si32 rt_Platform::set_fsaa(rt_si32 fsaa)
{
    if (fsaa > get_fsaa_max())
    {
        fsaa = get_fsaa_max();
    }

    this->fsaa = fsaa;

    return fsaa;
}

/*
 * Get current antialiasing mode.
 */
rt_si32 rt_Platform::get_fsaa()
{
    return fsaa;
}

/*
 * Get maximmum antialiasing mode
 * for chosen SIMD target.
 */
rt_si32 rt_Platform::get_fsaa_max()
{
    if (simd_width >= 8)
    {
        return RT_FSAA_4X; /* 8X is reserved */
    }
    else
    if (simd_width >= 4)
    {
        return RT_FSAA_4X; /* 2X alternating */
    }

    return RT_FSAA_NO;
}

/*
 * Return tile width in pixels.
 */
rt_si32 rt_Platform::get_tile_w()
{
    return tile_w;
}

/*
 * Add given "scn" to platform's scene list.
 */
rt_void rt_Platform::add_scene(rt_Scene *scn)
{
    if (head == RT_NULL)
    {
        head = tail = cur = scn;
    }
    else
    {
        tail->next = scn;
        tail = cur = scn;
    }

    scn->next = RT_NULL;
}

/*
 * Delete given "scn" from platform's scene list.
 */
rt_void rt_Platform::del_scene(rt_Scene *scn)
{
    rt_Scene **ptr = &head, *prev = RT_NULL;

    while (*ptr != RT_NULL)
    {
        if (*ptr == scn)
        {
            *ptr = scn->next;
            break;
        }

        prev = *ptr;
        ptr = &prev->next;
    }
    if (tail == scn)
    {
        tail = prev;
    }
    if (cur == scn)
    {
        cur = head;
    }
}

/*
 * Return current scene in the list.
 */
rt_Scene* rt_Platform::get_cur_scene()
{
    return cur;
}

/*
 * Set and return current scene in the list.
 */
rt_Scene* rt_Platform::set_cur_scene(rt_Scene *scn)
{
    rt_Scene *cur = head;

    while (cur != RT_NULL)
    {
        if (cur == scn)
        {
            this->cur = cur;
            break;
        }

        cur = cur->next;
    }

    return cur;
}

/*
 * Select next scene in the list as current.
 */
rt_void rt_Platform::next_scene()
{
    if (cur != RT_NULL)
    {
        if (cur->next != RT_NULL)
        {
            cur = cur->next;
        }
        else
        {
            cur = head;
        }
    }
}

/*
 * Deinitialize platform.
 */
rt_Platform::~rt_Platform()
{
    while (head != RT_NULL)
    {
        delete head; /* calls del_scene(head) replacing it with next */
    }

    /* destroy platform-specific worker threads */
    this->f_term(tdata, thnum);

    ASM_DONE(s_inf)
}

/******************************************************************************/
/*********************************   THREAD   *********************************/
/******************************************************************************/

/*
 * Allocate scene thread in custom heap.
 */
rt_pntr rt_SceneThread::operator new(size_t size, rt_Heap *hp)
{
    return hp->alloc(size, RT_ALIGN);
}

rt_void rt_SceneThread::operator delete(rt_pntr ptr)
{

}

/*
 * Instantiate scene thread.
 */
rt_SceneThread::rt_SceneThread(rt_Scene *scene, rt_si32 index) :

    rt_Heap(scene->f_alloc, scene->f_free)
{
    this->scene = scene;
    this->index = index;

    /* allocate root SIMD structure */
    s_inf = (rt_SIMD_INFOX *)
            alloc(sizeof(rt_SIMD_INFOX),
                            RT_SIMD_ALIGN);

    memset(s_inf, 0, sizeof(rt_SIMD_INFOX));

    /* allocate regs SIMD structure */
    rt_SIMD_REGS *s_reg = (rt_SIMD_REGS *)
            alloc(sizeof(rt_SIMD_REGS),
                            RT_SIMD_ALIGN);

    ASM_INIT(s_inf, s_reg)

    /* init framebuffer's dimensions and pointer */
    s_inf->frm_w   = scene->x_res;
    s_inf->frm_h   = scene->y_res;
    s_inf->frm_row = scene->x_row;
    s_inf->frame   = scene->frame;

    /* init tilebuffer's dimensions and pointer */
    s_inf->tile_w  = scene->pfm->tile_w;
    s_inf->tile_h  = scene->pfm->tile_h;
    s_inf->tls_row = scene->tiles_in_row;
    s_inf->tiles   = scene->tiles;

    /* init framebuffer's color-planes for path-tracer */
    s_inf->ptr_r   = scene->ptr_r;
    s_inf->ptr_g   = scene->ptr_g;
    s_inf->ptr_b   = scene->ptr_b;
    s_inf->pt_on   = scene->pt_on;

#if   RT_PRNG == LCG16

    /* init PRNG's constants (32-bit LCG) */
    s_inf->pseed   = scene->pseed; /* ptr to buffer of PRNG's 32-bit seed */
    RT_SIMD_SET(s_inf->prngf, (rt_uelm)214013);     /* PRNG's 32-bit factor */
    RT_SIMD_SET(s_inf->prnga, (rt_uelm)2531011);    /* PRNG's 32-bit addend */
    RT_SIMD_SET(s_inf->prngm, (rt_uelm)0xFFFF);     /* PRNG's 16-bit mask */

#elif RT_PRNG == LCG24

    /* init PRNG's constants (32-bit LCG) */
    s_inf->pseed   = scene->pseed; /* ptr to buffer of PRNG's 32-bit seed */
    RT_SIMD_SET(s_inf->prngf, (rt_uelm)214013);     /* PRNG's 32-bit factor */
    RT_SIMD_SET(s_inf->prnga, (rt_uelm)2531011);    /* PRNG's 32-bit addend */
    RT_SIMD_SET(s_inf->prngm, (rt_uelm)0xFFFFFF);   /* PRNG's 24-bit mask */

#elif RT_PRNG == LCG32

    /* init PRNG's constants (32-bit LCG) */
    s_inf->pseed   = scene->pseed; /* ptr to buffer of PRNG's 32-bit seed */
    RT_SIMD_SET(s_inf->prngf, (rt_uelm)214013);     /* PRNG's 32-bit factor */
    RT_SIMD_SET(s_inf->prnga, (rt_uelm)2531011);    /* PRNG's 32-bit addend */
    RT_SIMD_SET(s_inf->prngm, (rt_uelm)0xFFFFFFFF); /* PRNG's 32-bit mask */

#elif RT_PRNG == LCG48

    /* init PRNG's constants (48-bit LCG) */
    s_inf->pseed   = scene->pseed; /* ptr to buffer of PRNG's 48-bit seed */
    RT_SIMD_SET(s_inf->prngf, (rt_uelm)LL(25214903917));   /* 48-bit factor */
    RT_SIMD_SET(s_inf->prnga, (rt_uelm)LL(11));     /* PRNG's 48-bit addend */
    RT_SIMD_SET(s_inf->prngm, (rt_uelm)LL(0x0000FFFFFFFFFFFF));   /* mask */

#endif /* RT_PRNG */

    /* init power series constants for sin, cos */
    RT_SIMD_SET(s_inf->sin_3, -0.1666666666666666666666666666666666666666666);
    RT_SIMD_SET(s_inf->sin_5, +0.0083333333333333333333333333333333333333333);
    RT_SIMD_SET(s_inf->sin_7, -0.0001984126984126984126984126984126984126984);
    RT_SIMD_SET(s_inf->sin_9, +0.0000027557319223985890652557319223985890652);
    RT_SIMD_SET(s_inf->cos_4, +0.0416666666666666666666666666666666666666666);
    RT_SIMD_SET(s_inf->cos_6, -0.0013888888888888888888888888888888888888888);
    RT_SIMD_SET(s_inf->cos_8, +0.0000248015873015873015873015873015873015873);

#if RT_DEBUG >= 1

    /* init polynomial constants for asin, acos */
    RT_SIMD_SET(s_inf->asn_1, -0.0187293);
    RT_SIMD_SET(s_inf->asn_2, +0.0742610);
    RT_SIMD_SET(s_inf->asn_3, -0.2121144);
    RT_SIMD_SET(s_inf->asn_4, +1.5707288);
    RT_SIMD_SET(s_inf->tmp_1, +RT_PI_2);

#endif /* RT_DEBUG >= 1 */

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
    txmin = (rt_si32 *)alloc(sizeof(rt_si32) * scene->tiles_in_col, RT_ALIGN);
    txmax = (rt_si32 *)alloc(sizeof(rt_si32) * scene->tiles_in_col, RT_ALIGN);
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
    rt_si32 x1, y1, x2, y2, i, n, t;

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
    rt_si32 xmin = 0;
    rt_si32 ymin = 0;
    rt_si32 xmax = scene->tiles_in_row - 1;
    rt_si32 ymax = scene->tiles_in_col - 1;

    for (i = 0; i < n; i++)
    {
        /* calculate points floor */
        x1 = (rt_si32)RT_FLOOR(n1[i][RT_X]);
        y1 = (rt_si32)RT_FLOOR(n1[i][RT_Y]);
        x2 = (rt_si32)RT_FLOOR(n2[i][RT_X]);
        y2 = (rt_si32)RT_FLOOR(n2[i][RT_Y]);

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
        x2 = (rt_si32)RT_FLOOR(px);

        if (y1 > ymin)
        {
            RT_UPDATE_TILES_BOUNDS(y1 - 1, x1, x2);
        }

        x1 = x2;

        for (t = y1; t <= y2; t++)
        {
            px = px + rt;
            x2 = (rt_si32)RT_FLOOR(px);
            RT_UPDATE_TILES_BOUNDS(t, x1, x2);
            x1 = x2;
        }

        if (y2 < ymax)
        {
            x2 = (rt_si32)RT_FLOOR(n2[i][RT_X]);
            RT_UPDATE_TILES_BOUNDS(y2 + 1, x1, x2);
        }
    }
}

/*
 * Insert new element derived from "tem" to a list "ptr"
 * for a given object "obj". If "tem" is NULL and "obj" is LIGHT,
 * insert new element derived from "obj" to a list "ptr".
 * Return outer-most new element (not always list's head)
 * or NULL if new element is removed (fully obscured by other elements).
 */
rt_ELEM* rt_SceneThread::insert(rt_Object *obj, rt_ELEM **ptr, rt_ELEM *tem)
{
    rt_ELEM *elm = RT_NULL, *nxt;

    if (tem == RT_NULL && obj != RT_NULL && RT_IS_LIGHT(obj))
    {
        rt_Light *lgt = (rt_Light *)obj;

        /* alloc new element for "lgt" */
        elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
        elm->data = (rt_cell)scene->slist; /* all srf are potential shadows */
        elm->simd = lgt->s_lgt;
        elm->temp = lgt->bvbox;
        /* insert element as list's head */
        elm->next = *ptr;
       *ptr = elm;
    }

    if (tem == RT_NULL)
    {
        return elm;
    }

    /* only node elements are allowed in surface lists */
    rt_Node *nd = (rt_Node *)((rt_BOUND *)tem->temp)->obj;

    /* alloc new element from template "tem" */
    elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
    elm->data = 0;                      /* copy only array node's type below */
    elm->simd = RT_IS_SURFACE(nd) ? nd->s_srf : (rt_pntr)RT_GET_FLG(tem->simd);
    elm->temp = tem->temp;

    /* check if building "hlist/slist" (pass across surfaces in "ssort") */
    if (tem->simd == RT_NULL)
    {
        rt_Surface *srf = (rt_Surface *)nd;

        /* prepare surface's trnode/bvnode list for searching */
        rt_ELEM *lst = srf->trn, *prv = RT_NULL;

#if RT_OPTS_VARRAY != 0
        if ((scene->opts & RT_OPTS_VARRAY) != 0)
        {
            lst = srf->top;
        }
#endif /* RT_OPTS_VARRAY */

        /* search matching existing trnode/bvnode for insertion,
         * run through the list hierarchy to find the inner-most node element,
         * element's "simd" field holds pointer to node's sub-list
         * along with node's type in the lower 4 bits (tr/bv) */
        for (nxt = RT_GET_PTR(*ptr); nxt != RT_NULL && lst != RT_NULL;)
        {
            if (nxt->temp == lst->temp)
            {
                prv = nxt;
                /* set insertion point to existing node's sub-list */
                ptr = RT_GET_ADR(nxt->simd);
                /* search next inner node in existing node's sub-list */
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
         * one into another array node's sub-list and one outside,
         * this allows for greater flexibility in trnode/bvnode
         * relationship, something not allowed in previous versions */

        /* allocate new node elements from outer-most to inner-most
         * if they are not already in the list */
        for (; lst != RT_NULL; lst = lst->next)
        {
            /* alloc new trnode/bvnode element as none has been found */
            nxt = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
            nxt->data = (rt_cell)prv; /* link up */
            nxt->simd = lst->simd; /* node's type */
            nxt->temp = lst->temp;
            /* insert element according to found position */
            nxt->next = RT_GET_PTR(*ptr);
            RT_SET_PTR(*ptr, rt_ELEM *, nxt);
            /* set insertion point to new node's sub-list */
            ptr = RT_GET_ADR(nxt->simd);
            prv = nxt;
        }

        elm->data = (rt_cell)prv; /* link up */
    }

    /* insert element as list's head */
    elm->next = RT_GET_PTR(*ptr);
    RT_SET_PTR(*ptr, rt_ELEM *, elm);
    /* prepare new element for sorting
     * in order to figure out its optimal position in the list
     * and thus reduce potential overdraw in the rendering backend,
     * as array node's bounding box and volume are final at this point
     * it is correct to pass its element through the sorting routine below
     * before other elements are added into array node's sub-list */

    /* sort node elements in the list "ptr" with the new element "elm"
     * based on the bounding box and volume order as seen from "obj",
     * sorting is always applied to a single flat list
     * whether it's a top-level list or array node's sub-list,
     * treating both surface and array nodes in that list
     * as single whole elements, thus sorting never violates
     * the boundaries of array nodes' sub-lists as they are
     * determined by the search/insert algorithm above */
#if RT_OPTS_INSERT != 0
    if ((scene->opts & RT_OPTS_INSERT) == 0
    ||  obj == RT_NULL) /* don't sort "hlist/slist" */
#endif /* RT_OPTS_INSERT */
    {
        return elm;
    }

    /* "state" helps avoiding stored-order-value re-computation
     * when the whole sub-list is being moved without interruption,
     * the term sub-list used here and below refers to a continuous portion
     * of a single flat list as opposed to the same term used above
     * to separate different layers of the list hierarchy */
    rt_cell state = 0;
    rt_ELEM *prv = RT_NULL;

    /* phase 1, push "elm" through the list "ptr" for as long as possible,
     * order value re-computation is avoided via "state" variable */
    for (nxt = elm->next; nxt != RT_NULL; )
    {
        /* compute the order value between "elm" and "nxt" elements */
        rt_cell op = 7 & bbox_sort(obj->bvbox,
                     (rt_BOUND *)elm->temp,
                     (rt_BOUND *)nxt->temp);
        switch (op)
        {
            /* move "elm" forward if the "op" is
             * either "do swap" or "neutral" */
            case 2:
            /* as the swap operation is performed below
             * the stored-order-value becomes "no swap" */
            op = 1;
            case 3:
            elm->next = nxt->next;
            if (prv != RT_NULL)
            {
                if (state != 0)
                {
                    RT_SET_FLG(prv->data, rt_cell, state);
                }
                else
                {
                    RT_SET_FLG(prv->data, rt_cell,
                         3 & bbox_sort(obj->bvbox,
                         (rt_BOUND *)prv->temp,
                         (rt_BOUND *)nxt->temp));
                }
                prv->next = nxt;
            }
            else
            {
                RT_SET_PTR(*ptr, rt_ELEM *, nxt);
            }
            /* if current "elm's" position is transitory, "state" keeps
             * previously computed order value between "prv" and "nxt",
             * thus the order value can be restored to "prv's" data field
             * without re-computation as the "elm" advances further */
            state = RT_GET_FLG(nxt->data);
            RT_SET_FLG(nxt->data, rt_cell, op);
            nxt->next = elm;
            prv = nxt;
            nxt = elm->next;
            break;

            case 4|1: /* remove "nxt" fully obscured by "elm" */
            elm->next = nxt->next;
            /* reset "state" as "nxt" is removed */
            state = 0;
            nxt = nxt->next;
            break;

            case 4|2: /* remove "elm" fully obscured by "nxt" */
            if (prv != RT_NULL)
            {
                if (state != 0)
                {
                    RT_SET_FLG(prv->data, rt_cell, state);
                }
                else
                {
                    RT_SET_FLG(prv->data, rt_cell,
                         3 & bbox_sort(obj->bvbox,
                         (rt_BOUND *)prv->temp,
                         (rt_BOUND *)nxt->temp));
                }
                prv->next = nxt;
            }
            else
            {
                RT_SET_PTR(*ptr, rt_ELEM *, nxt);
            }
            state = 0;
            /* stop sorting as "elm" is removed */
            elm = nxt = RT_NULL;
            break;

            /* stop phase 1 if the "op" is "no swap" */
            default:
            RT_SET_FLG(elm->data, rt_cell, op);
            /* reset "state" as the "elm" has found its place */
            state = 0;
            nxt = RT_NULL;
            break;
        }
    }
    /* check if "elm" was removed in phase 1 */
    if (elm == RT_NULL)
    {
        return elm;
    }

    rt_ELEM *end, *tlp, *cur, *ipt, *jpt;

    /* phase 2, find the "end" of the strict-order-chain from "elm",
     * order value "no swap" is considered strict */
    for (end = elm; RT_GET_FLG(end->data) == 1; end = end->next);

    /* phase 3, move the elements from behind "elm's" strict-order-chain
     * right in front of the "elm" as computed order value dictates,
     * order value re-computation is avoided via "state" variables */
    for (tlp = cur = end, nxt = end->next; nxt != RT_NULL; )
    {
        rt_bool gr = RT_FALSE;
        /* compute the order value between "elm" and "nxt" elements */
        rt_cell op = 7 & bbox_sort(obj->bvbox,
                     (rt_BOUND *)elm->temp,
                     (rt_BOUND *)nxt->temp);
        switch (op)
        {
            /* move "nxt" in front of the "elm"
             * if the "op" is "do swap" */
            case 2:
            /* ignore "elm's" removal as it mustn't happen here */
            case 4|2:
            /* as the swap operation is performed below
             * the stored-order-value becomes "no swap" */
            op = 1;
            /* repair "cur's" stored-order-value
             * to see if "tlp" needs to catch up with "nxt" */
            if (RT_GET_FLG(cur->data) == 0 && cur != tlp)
            {
                RT_SET_FLG(cur->data, rt_cell,
                     3 & bbox_sort(obj->bvbox,
                     (rt_BOUND *)cur->temp,
                     (rt_BOUND *)nxt->temp));
            }
            /* if "cur's" stored-order-value to "nxt"
             * is "neutral", then strict-order-chain
             * from "tlp->next" up to "nxt" is broken,
             * thus "tlp" catches up with "nxt" */
            if (RT_GET_FLG(cur->data) == 3 && cur != tlp)
            {
                /* repair "tlp's" stored-order-value
                 * before it catches up with "nxt" */
                if (RT_GET_FLG(tlp->data) == 0)
                {
                    ipt = tlp->next;
                    RT_SET_FLG(tlp->data, rt_cell,
                         3 & bbox_sort(obj->bvbox,
                         (rt_BOUND *)tlp->temp,
                         (rt_BOUND *)ipt->temp));
                }
                /* reset "state" as "tlp" moves forward, thus
                 * breaking the sub-list moving to the front of the "elm" */
                state = 0;
                /* move "tlp" to "cur" right before "nxt" */
                tlp = cur;
            }
            /* check if there is a tail from "end->next"
             * up to "tlp" to comb out thoroughly before
             * moving "nxt" (along with its strict-order-chain
             * from "tlp->next") to the front of the "elm" */
            if (tlp != end)
            {
                /* local "state" helps avoiding stored-order-value
                 * re-computation for tail's elements joining the comb */
                rt_cell state = 0;
                cur = tlp;
                /* run through the tail area from "end->next"
                 * up to "tlp" backwards, while combing out
                 * elements to move along with "nxt" */
                while (cur != end)
                {
                    rt_bool mv = RT_FALSE;
                    /* search for "cur's" previous element */
                    for (ipt = end; ipt->next != cur; ipt = ipt->next);
                    rt_ELEM *iel = ipt->next;
                    /* run through the strict-order-chain from "tlp->next"
                     * up to "nxt" (which serves as a comb for the tail)
                     * and compute new order values for each tail element */
                    for (jpt = tlp; jpt != nxt; jpt = jpt->next)
                    {
                        rt_cell op = 0;
                        rt_ELEM *jel = jpt->next;
                        /* if "tlp's" stored-order-value to the first
                         * comb element is not reset, use it as "op",
                         * "cur" serves as "tlp" */
                        if (cur->next == jel && RT_GET_FLG(cur->data) != 0)
                        {
                            op = RT_GET_FLG(cur->data);
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
                            op = 3 & bbox_sort(obj->bvbox,
                                 (rt_BOUND *)cur->temp,
                                 (rt_BOUND *)jel->temp);
                        }
                        /* repair "tlp's" stored-order-value to the first
                         * comb element, "cur" serves as "tlp" */
                        if (cur->next == jel)
                        {
                            RT_SET_FLG(cur->data, rt_cell, op);
                        }
                        else
                        /* remember "cur's" computed order value to the first
                         * comb element in the "state", if "cur != tlp" */
                        if (tlp->next == jel)
                        {
                            state = op;
                        }
                        /* check if order is strict, then stop
                         * and mark "cur" as moving with "nxt",
                         * "cur" will then be added to the comb */
                        if (op == 1)
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
                            /* move "tlp" to its prev, its stored-order-value
                             * is always repaired in the combing stage above */
                            tlp = ipt;
                        }
                        /* move "cur" from the middle of the tail
                         * to the front of the comb, "iel" serves
                         * as "cur" and "ipt" serves as "cur's" prev */
                        else
                        {
                            cur = tlp->next;
                            RT_SET_FLG(iel->data, rt_cell, state);
                            /* local "state" keeps previously computed order
                             * value between "cur" and the front of the comb,
                             * thus the order value can be restored to
                             * "cur's" data field without re-computation */
                            state = RT_GET_FLG(ipt->data);
                            RT_SET_FLG(ipt->data, rt_cell, 0);
                            ipt->next = iel->next;
                            iel->next = cur;
                            RT_SET_FLG(tlp->data, rt_cell, 0);
                            tlp->next = iel;
                        }
                    }
                    /* "cur" doesn't move (stays in the tail) */
                    else
                    {
                        /* repair "cur's" stored-order-value before it
                         * moves to its prev, "iel" serves as "cur" */
                        if (RT_GET_FLG(iel->data) == 0)
                        {
                            cur = iel->next;
                            RT_SET_FLG(iel->data, rt_cell,
                                 3 & bbox_sort(obj->bvbox,
                                 (rt_BOUND *)iel->temp,
                                 (rt_BOUND *)cur->temp));
                        }
                        /* reset local "state" as tail's sub-list
                         * (joining the comb) is being broken */
                        state = 0;
                    }
                    /* move "cur" to its prev */
                    cur = ipt;
                }
                /* repair "end's" stored-order-value (to the rest of the tail),
                 * "ipt" serves as "end" */
                if (RT_GET_FLG(ipt->data) == 0)
                {
                    cur = ipt->next;
                    RT_SET_FLG(ipt->data, rt_cell,
                         3 & bbox_sort(obj->bvbox,
                         (rt_BOUND *)ipt->temp,
                         (rt_BOUND *)cur->temp));
                }
            }
            /* reset "state" if the comb has grown with tail elements, thus
             * breaking the sub-list moving to the front of the "elm" */
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
                    RT_SET_FLG(prv->data, rt_cell, state);
                }
                else
                {
                    RT_SET_FLG(prv->data, rt_cell,
                         3 & bbox_sort(obj->bvbox,
                         (rt_BOUND *)prv->temp,
                         (rt_BOUND *)cur->temp));
                }
                prv->next = cur;
            }
            else
            {
                RT_SET_PTR(*ptr, rt_ELEM *, cur);
            }
            cur = nxt->next;
            RT_SET_FLG(tlp->data, rt_cell, 0);
            tlp->next = cur;
            /* "state" keeps previously computed order value between "prv"
             * and "tlp->next", thus the order value can be restored to
             * "prv's" data field without re-computation if the whole sub-list
             * is being moved in front of the "elm" without interruption */
            state = RT_GET_FLG(nxt->data);
            RT_SET_FLG(nxt->data, rt_cell, op);
            nxt->next = elm;
            prv = nxt;
            nxt = cur;
            /* make sure "cur" is right before "nxt" between the cases */
            cur = tlp;
            break;

            case 4|1: /* remove "nxt" fully obscured by "elm" */
            RT_SET_FLG(cur->data, rt_cell, 0);
            /* "cur" is always right before "nxt" between the cases */
            cur->next = nxt->next;
            /* reset "state" as the sub-list moving to the front
             * of the "elm" is broken if "tlp->next" is removed */
            if (cur == tlp)
            {
                state = 0;
            }
            nxt = nxt->next;
            break;

            /* move "nxt" forward if the "op" is
             * either "no swap" or "neutral" */
            default:
            /* repair "cur's" stored-order-value
             * before it catches up with "nxt" */
            if (RT_GET_FLG(cur->data) == 0 && cur != tlp)
            {
                RT_SET_FLG(cur->data, rt_cell,
                     3 & bbox_sort(obj->bvbox,
                     (rt_BOUND *)cur->temp,
                     (rt_BOUND *)nxt->temp));
            }
            /* if "nxt's" or "cur's" stored-order-value
             * is "neutral", then strict-order-chain
             * from "tlp->next" up to "nxt" is being broken
             * as "nxt" moves, thus "tlp" catches up with "nxt" */
            if (RT_GET_FLG(nxt->data) == 3
            || (RT_GET_FLG(cur->data) == 3 && cur != tlp))
            {
                /* repair "tlp's" stored-order-value
                 * before it catches up with "nxt" */
                if (RT_GET_FLG(tlp->data) == 0)
                {
                    cur = tlp->next;
                    RT_SET_FLG(tlp->data, rt_cell,
                         3 & bbox_sort(obj->bvbox,
                         (rt_BOUND *)tlp->temp,
                         (rt_BOUND *)cur->temp));
                }
                /* reset "state" as "tlp" moves forward, thus
                 * breaking the sub-list moving to the front of the "elm" */
                state = 0;
                /* move "tlp" to "nxt" before it advances */
                tlp = nxt;
            }
            /* make sure "cur" is right before "nxt" between the cases */
            cur = nxt;
            /* when "nxt" runs away from "tlp" it grows a
             * strict-order-chain from "tlp->next" up to "nxt",
             * which then serves as a comb for the tail area
             * from "end->next" up to "tlp" */
            nxt = nxt->next;
            break;
        }
    }
    /* repair "tlp's" stored-order-value
     * if there are elements left behind it */
    cur = tlp->next;
    if (RT_GET_FLG(tlp->data) == 0 && cur != RT_NULL)
    {
        RT_SET_FLG(tlp->data, rt_cell,
             3 & bbox_sort(obj->bvbox,
             (rt_BOUND *)tlp->temp,
             (rt_BOUND *)cur->temp));
    }

    /* return newly inserted element */
    return elm;
}

/*
 * Filter list "ptr" for a given object "obj" by
 * converting hierarchical sorted sub-lists back into
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
         * reset "data" field used as stored-order-value
         * in sorting to keep it clean for the backend */
        if (RT_IS_SURFACE(nd))
        {
            elm = nxt;
            nxt->data = 0;
        }
        else
        /* if the list element is array,
         * find the last leaf element of its sub-list hierarchy
         * and set it to the "data" field along with node's type,
         * previously kept in its "simd" field's lower 4 bits */
        if (RT_IS_ARRAY(nd))
        {
            rt_Array *arr = (rt_Array *)nd;
            rt_ELEM **org = RT_GET_ADR(nxt->simd), *prv = elm;
            elm = filter(obj, org);
            rt_cell k = RT_GET_FLG(*org);
            if (elm != RT_NULL)
            {
                elm->next = nxt->next;
                nxt->data = (rt_cell)elm | k; /* node's type */
                nxt->next = RT_GET_PTR(*org);
                nxt->simd = k == 0 ? arr->s_srf :
                            nxt->temp == arr->bvbox ? arr->s_bvb :
                            nxt->temp == arr->inbox ? arr->s_inb : RT_NULL;
            }
            else
            {
                if (prv != RT_NULL)
                {
                    prv->next = nxt->next;
                }
                else
                {
                    RT_SET_PTR(*ptr, rt_ELEM *, nxt->next);
                }

                elm = prv;
                continue;
            }

#if RT_OPTS_TILING != 0
            /* filter out bvnodes for camera if tiling is enabled */
            if ((scene->opts & RT_OPTS_TILING) != 0
            &&  obj != RT_NULL && RT_IS_CAMERA(obj) && k == 1)
            {
                if (prv != RT_NULL)
                {
                    prv->next = nxt->next;
                }
                else
                {
                    RT_SET_PTR(*ptr, rt_ELEM *, nxt->next);
                }
            }
#endif /* RT_OPTS_TILING */

            if (elm != RT_NULL)
            {
                nxt = elm;
            }
        }
    }

    return elm;
}

/*
 * Build trnode/bvnode list for a given surface "srf"
 * after all transform flags have been set in "update_fields",
 * thus trnode elements are handled properly.
 */
rt_void rt_SceneThread::snode(rt_Surface *srf)
{
    /* as temporary memory pool is released after every frame,
     * always rebuild the list even if the scene hasn't changed */

    /* reset surface's trnode/bvnode list */
    srf->top = RT_NULL;
    srf->trn = RT_NULL;

    /* build surface's trnode/bvnode list,
     * trnode hierarchy is flat as objects with
     * non-trivial transform are their own trnodes,
     * while bvnodes can have arbitrary depth
     * above and below the trnode (if present) */
    rt_ELEM *elm;
    rt_Object *par;

    /* phase 1, bvnodes (if any) below trnode (if any),
     * if the same array serves as both trnode and bvnode,
     * trnode is considered above only if bvnode is split,
     * thus bvnode is inserted first in this case */
    for (par = srf->bvnode; srf->trnode != RT_NULL && par != RT_NULL &&
         par->trnode == srf->trnode && (par->trnode != par ||
         ((rt_Array *)par)->trbox->rad != 0.0f);
         par = par->bvnode)
    {
        rt_Array *arr = (rt_Array *)par;

        elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
        elm->data = 0;
        elm->simd = (rt_pntr)1; /* node's type (bv) */
        elm->temp = arr->inbox;
        elm->next = srf->top;
        srf->top = elm;
    }

    /* phase 2, there can only be one trnode (if any),
     * even though there might be other trnodes
     * above and below in the object hierarchy
     * they themselves don't form the hierarchy
     * as any trnode is always its own trnode */
    if (srf->trnode != RT_NULL && srf->trnode != srf)
    {
        rt_Array *arr = (rt_Array *)srf->trnode;

        elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
        elm->data = 0;
        elm->simd = (rt_pntr)0; /* node's type (tr) */
        elm->temp = arr->trbox->rad != 0.0f ? arr->trbox : arr->inbox;
        elm->next = srf->top;
        srf->top = elm;

        elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
        elm->data = 0;
        elm->simd = (rt_pntr)0; /* node's type (tr) */
        elm->temp = srf->top->temp;
        elm->next = RT_NULL;
        srf->trn = elm;
    }

    /* phase 3, bvnodes (if any) above trnode (if any) */
    for (; par != RT_NULL; par = par->bvnode)
    {
        rt_Array *arr = (rt_Array *)par;

        elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
        elm->data = 0;
        elm->simd = (rt_pntr)1; /* node's type (bv) */
        elm->temp = arr->bvbox->rad != 0.0f ? arr->bvbox : arr->inbox;
        elm->next = srf->top;
        srf->top = elm;
    }
}

/*
 * Build custom clippers list from "srf" relations template
 * after all transform flags have been set in "update_fields",
 * thus trnode elements are handled properly.
 */
rt_void rt_SceneThread::sclip(rt_Surface *srf)
{
    /* as temporary memory pool is released after every frame,
     * always rebuild the list even if the scene hasn't changed */

    /* init surface's relations template */
    rt_ELEM *lst = srf->rel;

    /* init and reset custom clippers list */
    rt_ELEM **ptr = RT_GET_ADR(srf->s_srf->msc_p[2]);
   *ptr = RT_NULL;

    /* build custom clippers list from given template "lst",
     * as given template "lst" is inverted in surface's "add_relation"
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

            /* alloc new element for "srf" */
            elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
            elm->data = rel;
            elm->simd = srf->s_srf;
            elm->temp = srf->bvbox;

            if (srf->trnode != RT_NULL && srf->trnode != srf)
            {
                rt_si32 acc = 0;
                rt_ELEM *nxt;

                rt_Array *arr = (rt_Array *)srf->trnode;
                rt_BOUND *trb = arr->trbox; /* not used for clippers */

                /* search matching existing trnode for insertion
                 * either within current accum segment
                 * or outside of any accum segment */
                for (nxt = *ptr; nxt != RT_NULL; nxt = nxt->next)
                {
                    /* "acc == 0" either accum-enter-marker
                     * hasn't been inserted yet (current accum segment)
                     * or outside of any accum segment */
                    if (acc == 0
                    &&  nxt->temp == trb)
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
                    nxt->data = (rt_cell)elm; /* trnode's last element */
                    nxt->simd = arr->s_srf;
                    nxt->temp = trb;
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
     * always rebuild the list even if the scene hasn't changed */

    srf->tls = RT_NULL;

#if RT_OPTS_TILING != 0
    if ((scene->opts & RT_OPTS_TILING) == 0)
#endif /* RT_OPTS_TILING */
    {
        return;
    }

    rt_ELEM *elm;
    rt_si32 i, j;
    rt_si32 k;

    rt_vec4 vec;
    rt_real dot;
    rt_si32 ndx[2];
    rt_real tag[2], zed[2];

    /* "verts_num" may grow, use "srf->verts_num" if original is needed */
    rt_si32 verts_num = srf->bvbox->verts_num;
    rt_VERT *vrt = srf->bvbox->verts;

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
                         (2 * verts_num + srf->bvbox->edges_num));

        for (k = 0; k < srf->bvbox->verts_num; k++)
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
        for (k = 0; k < srf->bvbox->edges_num; k++)
        {
            for (i = 0; i < 2; i++)
            {
                ndx[i] = srf->bvbox->edges[k].index[i];
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
        for (i = srf->bvbox->verts_num; i < verts_num - 1; i++)
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

    rt_ELEM **ptr = RT_GET_ADR(srf->tls);

    /* fill marked tiles with surface data */
    for (i = 0; i < scene->tiles_in_col; i++)
    {
        for (j = txmin[i]; j <= txmax[i]; j++)
        {
            /* alloc new element for each tile of "srf" */
            elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
            elm->data = i << 16 | j;
            elm->simd = srf->s_srf;
            elm->temp = srf->bvbox;
            /* insert element as list's tail */
           *ptr = elm;
            ptr = &elm->next;
        }
    }

   *ptr = RT_NULL;
}

/*
 * Build surface list for a given object "obj".
 * Surface objects have separate surface lists for each side.
 */
rt_ELEM* rt_SceneThread::ssort(rt_Object *obj)
{
    /* as temporary memory pool is released after every frame,
     * always rebuild the list even if the scene hasn't changed */

    rt_Surface *srf = RT_NULL;
    rt_ELEM **pto = RT_NULL;
    rt_ELEM **pti = RT_NULL;

    if (obj != RT_NULL && RT_IS_SURFACE(obj))
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

    rt_ELEM *lst = RT_NULL;
    rt_ELEM **ptr = &lst;

    if (obj == RT_NULL)
    {
        rt_Surface *ref;

        /* linear traversal across surfaces */
        for (ref = scene->srf_head; ref != RT_NULL; ref = ref->next)
        {
            rt_ELEM tem;

            tem.data = 0;
            tem.simd = RT_NULL; /* signal "insert" to build "hlist/slist" */
            tem.temp = ref->bvbox;
            tem.next = RT_NULL;

            insert(obj, ptr, &tem);
        }
    }
    else
    {
        rt_si32 c = 0, r = 0;
        rt_ELEM *elm, *cur = RT_NULL, *prv = RT_NULL;
        rt_ELEM *cuo, *cui, *pro = RT_NULL, *pri = RT_NULL;
        rt_BOUND *abx = RT_NULL;

        /* hierarchical traversal across nodes */
        for (elm = scene->hlist; elm != RT_NULL;)
        {
            rt_BOUND *box = (rt_BOUND *)elm->temp;

#if RT_OPTS_REMOVE != 0
            /* disable array's contents removal by its bbox
             * when building for surface on the same branch */
            if ((scene->opts & RT_OPTS_REMOVE) != 0
            &&  abx != RT_NULL && RT_IS_SURFACE(obj))
            {
                rt_ELEM *top = ((rt_Surface *)obj)->top;

                for (; top != RT_NULL; top = top->next)
                {
                    if (abx == top->temp)
                    {
                        abx = RT_NULL;
                        break;
                    }
                }
            }

            r = 0;

            /* enable array's contents removal by its bbox */
            if ((scene->opts & RT_OPTS_REMOVE) != 0
            &&  abx != RT_NULL && abx != box)
            {
                r = bbox_sort(obj->bvbox, box, abx);
            }
#endif /* RT_OPTS_REMOVE */

#if RT_OPTS_2SIDED != 0
            if ((scene->opts & RT_OPTS_2SIDED) != 0
            &&  srf != RT_NULL)
            {
                /* only call "bbox_side" if all arrays above in the hierarchy
                 * are seen from both sides of the surface, don't call
                 * "bbox_side" again if two array elements have the same bbox */
                if (cur == RT_NULL && (prv == RT_NULL || prv->temp != box))
                {
                    c = bbox_side(box, srf->shape);
                }

                /* insert nodes according to
                 * side value computed above */
                cuo = RT_NULL;
                if (c & 2 && r != 6)
                {
                    cuo = insert(obj, pto, elm);
                    if (cuo != RT_NULL)
                    {
                        RT_SET_PTR(cuo->data, rt_cell, pro);
                    }
                }
                cui = RT_NULL;
                if (c & 1 && r != 6)
                {
                    cui = insert(obj, pti, elm);
                    if (cui != RT_NULL)
                    {
                        RT_SET_PTR(cui->data, rt_cell, pri);
                    }
                }

                /* if array's bbox is only seen from one side of the surface
                 * so are all of array's contents, thus skip "bbox_side" call */
                if (RT_IS_ARRAY(box) && (cuo != RT_NULL || cui != RT_NULL))
                {
                    /* set array for skipping "bbox_side" call above */
                    if (cur == RT_NULL && c < 3)
                    {
                        cur = elm;
                    }
                    if (box->fln > 1) /* insert handles "box->fln == 1" case */
                    {
                        abx = box;
                    }
                    else /* if sub-array isn't removed, "abx" isn't effective */
                    {
                        abx = RT_NULL;
                    }

                    if (cuo != RT_NULL)
                    {
                        pro = cuo;
                        pto = RT_GET_ADR(cuo->simd);
                    }
                    if (cui != RT_NULL)
                    {
                        pri = cui;
                        pti = RT_GET_ADR(cui->simd);
                    }

                    prv = elm;
                    elm = RT_GET_PTR(elm->simd);
                }
                else
                {
                    /* if anything except bbox's faces isn't removed,
                     * "abx" isn't effective */
                    if (abx != RT_NULL && !RT_IS_PLANE(box) && r != 6)
                    {
                        abx = RT_NULL;
                    }

                    while (elm != RT_NULL && elm->next == RT_NULL)
                    {
                        if (cur == RT_NULL || c & 2)
                        {
                            pro = pro != RT_NULL ? RT_GET_PTR(pro->data) :
                                  RT_NULL;
                            pto = pro != RT_NULL ? RT_GET_ADR(pro->simd) :
                                  RT_GET_ADR(srf->s_srf->lst_p[1]);
                        }
                        if (cur == RT_NULL || c & 1)
                        {
                            pri = pri != RT_NULL ? RT_GET_PTR(pri->data) :
                                  RT_NULL;
                            pti = pri != RT_NULL ? RT_GET_ADR(pri->simd) :
                                  RT_GET_ADR(srf->s_srf->lst_p[3]);
                        }

                        elm = RT_GET_PTR(elm->data);

                        if (elm == cur)
                        {
                            cur = RT_NULL;
                        }

                        abx = RT_NULL;
                    }

                    if (elm != RT_NULL)
                    {
                        elm = elm->next;
                    }

                    prv = RT_NULL;
                }
            }
            else
#endif /* RT_OPTS_2SIDED */
            {
                cur = RT_NULL;
                if (r != 6)
                {
                    cur = insert(obj, ptr, elm);
                }
                if (cur != RT_NULL)
                {
                    RT_SET_PTR(cur->data, rt_cell, prv);
                }

                if (RT_IS_ARRAY(box) && cur != RT_NULL)
                {
                    if (box->fln > 1) /* insert handles "box->fln == 1" case */
                    {
                        abx = box;
                    }
                    else /* if sub-array isn't removed, "abx" isn't effective */
                    {
                        abx = RT_NULL;
                    }

                    prv = cur;
                    ptr = RT_GET_ADR(cur->simd);
                    elm = RT_GET_PTR(elm->simd);
                }
                else
                {
                    /* if anything except bbox's faces isn't removed,
                     * "abx" isn't effective */
                    if (abx != RT_NULL && !RT_IS_PLANE(box) && r != 6)
                    {
                        abx = RT_NULL;
                    }

                    while (elm != RT_NULL && elm->next == RT_NULL)
                    {
                        prv = prv != RT_NULL ? RT_GET_PTR(prv->data) : RT_NULL;
                        ptr = prv != RT_NULL ? RT_GET_ADR(prv->simd) : &lst;
                        elm = RT_GET_PTR(elm->data);
                        abx = RT_NULL;
                    }

                    if (elm != RT_NULL)
                    {
                        elm = elm->next;
                    }
                }
            }
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
        if (*ptr != RT_NULL && obj != RT_NULL) /* don't filter "hlist/slist" */
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
 * Build light/shadow list for a given object "obj".
 * Surface objects have separate light/shadow lists for each side.
 */
rt_ELEM* rt_SceneThread::lsort(rt_Object *obj)
{
    /* as temporary memory pool is released after every frame,
     * always rebuild the list even if the scene hasn't changed */

    rt_Surface *srf = RT_NULL;
    rt_ELEM **pto = RT_NULL;
    rt_ELEM **pti = RT_NULL;

    if (obj != RT_NULL && RT_IS_SURFACE(obj))
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

    rt_ELEM *lst = RT_NULL;
    rt_ELEM **ptr = &lst;
    rt_Light *lgt;

    /* linear traversal across light sources */
    for (lgt = scene->lgt_head; lgt != RT_NULL; lgt = lgt->next)
    {
        rt_ELEM **pso = RT_NULL;
        rt_ELEM **psi = RT_NULL;
        rt_ELEM **psr = RT_NULL;

#if RT_OPTS_2SIDED != 0
        if ((scene->opts & RT_OPTS_2SIDED) != 0 && srf != RT_NULL)
        {
            rt_si32 c = bbox_side(lgt->bvbox, srf->shape);

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

        rt_si32 c = 0, s = 0;
        rt_ELEM *elm, *cur = RT_NULL, *prv = RT_NULL;
        rt_ELEM *cuo, *cui, *pro = RT_NULL, *pri = RT_NULL;

        /* hierarchical traversal across nodes */
        for (elm = scene->hlist; elm != RT_NULL;)
        {
            rt_BOUND *box = (rt_BOUND *)elm->temp;

            /* only call "bbox_shad" if all arrays above in the hierarchy
             * cast shadow on the surface, don't call
             * "bbox_shad" again if two array elements have the same bbox */
            if (prv == RT_NULL || prv->temp != box)
            {
                s = bbox_shad(lgt->bvbox, box, srf->bvbox);
            }

#if RT_OPTS_2SIDED != 0
            if ((scene->opts & RT_OPTS_2SIDED) != 0)
            {
                /* only call "bbox_side" if all arrays above in the hierarchy
                 * are seen from both sides of the surface, don't call
                 * "bbox_side" again if two array elements have the same bbox */
                if (cur == RT_NULL && (prv == RT_NULL || prv->temp != box) && s)
                {
                    c = bbox_side(box, srf->shape);
                }

                /* insert nodes according to
                 * side value computed above */
                cuo = RT_NULL;
                if (c & 2 && pso != RT_NULL && s)
                {
                    cuo = insert(obj, pso, elm);
                    if (cuo != RT_NULL)
                    {
                        RT_SET_PTR(cuo->data, rt_cell, pro);
                    }
                }
                cui = RT_NULL;
                if (c & 1 && psi != RT_NULL && s)
                {
                    cui = insert(obj, psi, elm);
                    if (cui != RT_NULL)
                    {
                        RT_SET_PTR(cui->data, rt_cell, pri);
                    }
                }

                /* if array's bbox is only seen from one side of the surface
                 * so are all of array's contents, thus skip "bbox_side" call */
                if (RT_IS_ARRAY(box) && c != 0 && s
                && (cuo != RT_NULL || cui != RT_NULL))
                {
                    /* set array for skipping "bbox_side" call above */
                    if (cur == RT_NULL && c < 3)
                    {
                        cur = elm;
                    }

                    if (cuo != RT_NULL)
                    {
                        pro = cuo;
                        pso = RT_GET_ADR(cuo->simd);
                    }
                    if (cui != RT_NULL)
                    {
                        pri = cui;
                        psi = RT_GET_ADR(cui->simd);
                    }

                    prv = elm;
                    elm = RT_GET_PTR(elm->simd);
                }
                else
                {
                    while (elm != RT_NULL && elm->next == RT_NULL)
                    {
                        if ((cur == RT_NULL || c & 2) && pso != RT_NULL)
                        {
                            pro = pro != RT_NULL ? RT_GET_PTR(pro->data) :
                                  RT_NULL;
                            pso = pro != RT_NULL ? RT_GET_ADR(pro->simd) :
                                  RT_GET_ADR((*pto)->data);
                        }
                        if ((cur == RT_NULL || c & 1) && psi != RT_NULL)
                        {
                            pri = pri != RT_NULL ? RT_GET_PTR(pri->data) :
                                  RT_NULL;
                            psi = pri != RT_NULL ? RT_GET_ADR(pri->simd) :
                                  RT_GET_ADR((*pti)->data);
                        }

                        elm = RT_GET_PTR(elm->data);

                        if (elm == cur)
                        {
                            cur = RT_NULL;
                        }
                    }

                    if (elm != RT_NULL)
                    {
                        elm = elm->next;
                    }

                    prv = RT_NULL;
                }
            }
            else
#endif /* RT_OPTS_2SIDED */
            {
                if (s)
                {
                    cur = insert(obj, psr, elm);
                    if (cur != RT_NULL)
                    {
                        RT_SET_PTR(cur->data, rt_cell, prv);
                    }
                }

                if (RT_IS_ARRAY(box) && cur != RT_NULL && s)
                {
                    prv = cur;
                    psr = RT_GET_ADR(cur->simd);
                    elm = RT_GET_PTR(elm->simd);
                }
                else
                {
                    while (elm != RT_NULL && elm->next == RT_NULL)
                    {
                        prv = prv != RT_NULL ? RT_GET_PTR(prv->data) :
                              RT_NULL;
                        psr = prv != RT_NULL ? RT_GET_ADR(prv->simd) :
                              RT_GET_ADR((*ptr)->data);
                        elm = RT_GET_PTR(elm->data);
                    }

                    if (elm != RT_NULL)
                    {
                        elm = elm->next;
                    }
                }
            }
        }

#if RT_OPTS_INSERT != 0 || RT_OPTS_TARRAY != 0 || RT_OPTS_VARRAY != 0
        if ((scene->opts & RT_OPTS_INSERT) != 0
        ||  (scene->opts & RT_OPTS_TARRAY) != 0
        ||  (scene->opts & RT_OPTS_VARRAY) != 0)
        {
            if (pso != RT_NULL && *pso != RT_NULL)
            {
                filter(RT_NULL, pso);
            }
            if (psi != RT_NULL && *psi != RT_NULL)
            {
                filter(RT_NULL, psi);
            }
            if (psr != RT_NULL && *psr != RT_NULL)
            {
                filter(RT_NULL, psr);
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
 * Deinitialize scene thread.
 */
rt_SceneThread::~rt_SceneThread()
{
    ASM_DONE(s_inf)
}

/******************************************************************************/
/**********************************   SCENE   *********************************/
/******************************************************************************/

/*
 * Allocate scene in custom heap.
 * Heap "hp" must be the same object as platform "pfm" in constructor.
 */
rt_pntr rt_Scene::operator new(size_t size, rt_Heap *hp)
{
    return hp->obj_alloc(size, RT_ALIGN);
}

/*
 * Delete scene from custom heap.
 */
rt_void rt_Scene::operator delete(rt_pntr ptr)
{
    ((rt_Scene *)ptr)->pfm->obj_free(ptr);
}

/*
 * Instantiate scene.
 * Can only be called from single (main) thread.
 * Platform "pfm" must be the same object as heap "hp" in custom "new".
 */
rt_Scene::rt_Scene(rt_SCENE *scn, /* "frame" must be SIMD-aligned or NULL */
                   rt_si32 x_res, rt_si32 y_res, rt_si32 x_row, rt_ui32 *frame,
                   rt_Platform *pfm) :

    rt_Registry(pfm->f_alloc, pfm->f_free),
    rt_List<rt_Scene>(RT_NULL)
{
    this->pfm = pfm;

    pfm->add_scene(this);

    thnum = pfm->thnum;
    tdata = pfm->tdata;

    thr_num = thnum; /* for registry of object hierarchy */

    f_update = pfm->f_update;
    f_render = pfm->f_render;

    this->scn = scn;

    /* check for locked scene data, not thread safe! (it's ok, not a bug) */
    if (scn->lock != RT_NULL)
    {
        throw rt_Exception("scene data is locked by another instance");
    }

    /* "x_row", frame's stride (in 32-bit pixels, not bytes!),
     * can be greater than "x_res", in which case the frame
     * occupies only a portion (rectangle) of the framebuffer,
     * or negative, in which case frame starts at the last line
     * and consecutive lines are located backwards in memory,
     * "x_row" must contain the whole number of SIMD widths */
    if (x_res == 0 || y_res == 0 || RT_ABS32(x_row) < x_res)
    {
        throw rt_Exception("frambuffer's dimensions are not valid");
    }

#if (RT_POINTER - RT_ADDRESS) != 0

    /* always reallocate frame in custom heap for 64-bit
     * if 32-bit address range is enabled in makefiles */
    frame = RT_NULL;

#endif /* (RT_POINTER - RT_ADDRESS) */

    if (((rt_word)frame & (RT_SIMD_ALIGN - 1)) != 0 || frame == RT_NULL
    || (RT_ABS32(x_row) & (RT_SIMD_WIDTH - 1)) != 0)
    {
        rt_si32 y_sgn = RT_SIGN(x_row);

        x_row = RT_ABS32(x_row);
        x_row = ((x_row + RT_SIMD_WIDTH - 1) / RT_SIMD_WIDTH) * RT_SIMD_WIDTH;

        frame = (rt_ui32 *)
                alloc(x_row * y_res * sizeof(rt_ui32), RT_SIMD_ALIGN);

        x_row *= y_sgn;

        if (x_row < 0)
        {
            frame += RT_ABS32(x_row) * (y_res - 1);
        }
    }

    memset(frame, 0, x_row * y_res * sizeof(rt_ui32));

    /* init framebuffer's dimensions and pointer */
    this->x_res = x_res;
    this->y_res = y_res;
    this->x_row = x_row;
    this->frame = frame;

    /* init tilebuffer's dimensions and pointer */
    tiles_in_row = (x_res + pfm->tile_w - 1) / pfm->tile_w;
    tiles_in_col = (y_res + pfm->tile_h - 1) / pfm->tile_h;

    tiles = (rt_ELEM **)
            alloc(tiles_in_row * tiles_in_col * sizeof(rt_ELEM *), RT_ALIGN);

    memset(tiles, 0, tiles_in_row * tiles_in_col * sizeof(rt_ELEM *));

    /* init pixel-width, aspect-ratio, ray-depth */
    factor = 1.0f / (rt_real)x_res;
    aspect = (rt_real)y_res * factor;
    depth = RT_MAX(RT_STACK_DEPTH, 0);
    opts &= ~scn->opts;

    pseed = RT_NULL;
    ptr_r = RT_NULL;
    ptr_g = RT_NULL;
    ptr_b = RT_NULL;

    if ((opts & RT_OPTS_PT) == 0 || (opts & RT_OPTS_BUFFERS) == 0)
    {
        /* alloc framebuffer's color-planes for path-tracer */
        ptr_r = (rt_real *)
                alloc(4 * x_row * y_res * sizeof(rt_real), RT_SIMD_ALIGN);
        ptr_g = (rt_real *)
                alloc(4 * x_row * y_res * sizeof(rt_real), RT_SIMD_ALIGN);
        ptr_b = (rt_real *)
                alloc(4 * x_row * y_res * sizeof(rt_real), RT_SIMD_ALIGN);

                /* ptr_* is initialized in reset_color() */
    }
    if ((opts & RT_OPTS_BUFFERS) == 0)
    {
        reset_color();
    }
    if ((opts & RT_OPTS_PT) == 0)
    {
        /* alloc framebuffer's seed-plane for path-tracer */
        pseed = (rt_elem *)
                alloc(4 * x_row * y_res * sizeof(rt_elem), RT_SIMD_ALIGN);

                /* pseed is initialized in reset_pseed() */
    }

    pts_c = 0.0f;
    pt_on = RT_FALSE;

    fsaa = pfm->fsaa;

    /* instantiate object hierarchy */
    memset(&rootobj, 0, sizeof(rt_OBJECT));

    rootobj.trm.scl[RT_I] = 1.0f;
    rootobj.trm.scl[RT_J] = 1.0f;
    rootobj.trm.scl[RT_K] = 1.0f;
    rootobj.obj = scn->root;

    if (!RT_IS_ARRAY(&scn->root))
    {
        throw rt_Exception("scene's root is not an array");
    }

    root = new(this) rt_Array(this, RT_NULL, &rootobj); /* also init "*_num" */

    if (cam_head == RT_NULL)
    {
        throw rt_Exception("scene doesn't contain camera");
    }

    cam = cam_head;
    cam_idx = 0;

    /* lock scene data, when scene's constructor can no longer fail */
    scn->lock = this;

    /* create scene threads array */
    tharr = (rt_SceneThread **)
            alloc(sizeof(rt_SceneThread *) * thnum, RT_ALIGN);

    rt_si32 i;

    for (i = 0; i < thnum; i++)
    {
        tharr[i] = new(this) rt_SceneThread(this, i);

        /* estimate per-frame allocs to reduce system calls per thread */
        tharr[i]->msize =  /* upper bound per surface for tiling */
            (tiles_in_row * tiles_in_col + /* plus array nodes list */
             (arr_num + 2) +     /* plus reflections/refractions */
             (srf_num + arr_num * 2 + /* plus lights and shadows */
             (srf_num + arr_num * 2 + 1) * lgt_num) * 2) * /* for both sides */
            sizeof(rt_ELEM) * (srf_num + thnum - 1) / thnum; /* per thread */
    }

    pending = 0;

    /* init memory pool in the heap for temporary per-frame allocs */
    mpool = RT_NULL; /* rough estimate for surface relations/templates */
    msize = ((srf_num + 1) * (srf_num + 1) * 2 + /* plus two surface lists */
             (srf_num + arr_num * 1) * 2 + /* plus lights and shadows list */
             (srf_num + arr_num * 2 + 1) * lgt_num + /* plus array nodes */
             tiles_in_row * tiles_in_col * arr_num) *  /* for tiling */
            sizeof(rt_ELEM);                        /* for main thread */

    /* in the estimates above ("arr_num" * x) depends on whether both
     * trnode/bvnode are allowed in the list or just one of them,
     * if the estimates are not accurate the engine should still work,
     * though not as efficient due to unnecessary allocations per frame
     * or unused extra memory reservation resulting in larger footprint */
}

/*
 * Update current camera with given "action" for a given "time".
 */
rt_void rt_Scene::update(rt_time time, rt_si32 action)
{
    cam->update_action(time, action);
}

/*
 * Update backend data structures and render frame for a given "time".
 */
rt_void rt_Scene::render(rt_time time)
{
    rt_si32 i;

#if RT_OPTS_UPDATE_EXT0 != 0
    if ((opts & RT_OPTS_UPDATE_EXT0) == 0 || rootobj.time == -1)
    { /* -->---->-- skip update1 -->---->-- */
#endif /* RT_OPTS_UPDATE_EXT0 */

    if (pending)
    {
        pending = 0;

        /* release memory for temporary per-frame allocs */
        for (i = 0; i < thnum; i++)
        {
            tharr[i]->release(tharr[i]->mpool);
        }

        release(mpool);
    }

    /* reserve memory for temporary per-frame allocs */
    mpool = reserve(msize, RT_QUAD_ALIGN);

    for (i = 0; i < thnum; i++)
    {
        tharr[i]->mpool = tharr[i]->reserve(tharr[i]->msize, RT_QUAD_ALIGN);
    }

    /* print state init */
    if (g_print)
    {
        RT_PRINT_STATE_INIT();
        RT_PRINT_TIME(time);
    }

    /* phase 0.5, hierarchical update of arrays' transform matrices */
    root->update_object(time, 0, RT_NULL, iden4);

    if (pt_on && (root->scn_changed || pfm->fsaa != fsaa))
    {
        reset_color();
    }

    /* update current antialiasing mode per scene */
    fsaa = pfm->fsaa;

    /* 1st phase of multi-threaded update */
#if RT_OPTS_THREAD != 0
    if ((opts & RT_OPTS_THREAD) != 0 && this == pfm->get_cur_scene() && !g_print
#if RT_OPTS_UPDATE_EXT1 != 0
    &&  (opts & RT_OPTS_UPDATE_EXT1) == 0
#endif /* RT_OPTS_UPDATE_EXT1 */
       )
    {
        this->f_update(tdata, thnum, 1);
    }
    else
#endif /* RT_OPTS_THREAD */
    {
        update_scene(this, -thnum, 1);
    }

    /* update ray positioning and steppers */
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

    /* update tile positioning and steppers */
    RT_VEC3_ADD(org, pos, dir);

    h = 1.0f / (factor * pfm->tile_w); /* x_res / tile_w */
    v = 1.0f / (factor * pfm->tile_h); /* x_res / tile_h */

    RT_VEC3_MUL_VAL1(htl, hor, h);
    RT_VEC3_MUL_VAL1(vtl, ver, v);

    /* 2nd phase of multi-threaded update */
#if RT_OPTS_THREAD != 0
    if ((opts & RT_OPTS_THREAD) != 0 && this == pfm->get_cur_scene() && !g_print
#if RT_OPTS_UPDATE_EXT2 != 0
    &&  (opts & RT_OPTS_UPDATE_EXT2) == 0
#endif /* RT_OPTS_UPDATE_EXT2 */
       )
    {
        this->f_update(tdata, thnum, 2);
    }
    else
#endif /* RT_OPTS_THREAD */
    {
        update_scene(this, -thnum, 2);
    }

    /* phase 2.5, hierarchical update of arrays' bounds from surfaces */
    root->update_bounds();

    rt_Surface *srf;

    /* update surfaces' node lists */
    for (srf = srf_head; srf != RT_NULL; srf = srf->next)
    {
        /* rebuild surface's node list (per-surface)
         * based on transform flags and arrays' bounds */
        tharr[0]->snode(srf);
    }

    /* rebuild global hierarchical list */
    hlist = tharr[0]->ssort(RT_NULL);

    /* rebuild global surface/node list */
    slist = tharr[0]->ssort(RT_NULL);
    tharr[0]->filter(RT_NULL, &slist);

    /* rebuild global light/shadow list,
     * "slist" is needed inside */
    llist = tharr[0]->lsort(RT_NULL);

    /* rebuild camera's surface/node list,
     * "slist" is needed inside */
    clist = tharr[0]->ssort(cam);

    if (g_print)
    {
        RT_PRINT_GLB();
        RT_PRINT_SRF_LST(slist);
        RT_PRINT_LGT_LST(llist);
        RT_PRINT_CAM(cam);
        RT_PRINT_SRF_LST(clist);
    }

    /* 3rd phase of multi-threaded update */
#if RT_OPTS_THREAD != 0
    if ((opts & RT_OPTS_THREAD) != 0 && this == pfm->get_cur_scene() && !g_print
#if RT_OPTS_UPDATE_EXT3 != 0
    &&  (opts & RT_OPTS_UPDATE_EXT3) == 0
#endif /* RT_OPTS_UPDATE_EXT3 */
       )
    {
        this->f_update(tdata, thnum, 3);
    }
    else
#endif /* RT_OPTS_THREAD */
    {
        update_scene(this, -thnum, 3);
    }

    /* screen tiling */
    rt_si32 tline, j;

#if RT_OPTS_TILING != 0
    if ((opts & RT_OPTS_TILING) != 0)
    {
        memset(tiles, 0, sizeof(rt_ELEM *) * tiles_in_row * tiles_in_col);

        rt_ELEM *elm, *nxt, *ctail = RT_NULL, **ptr = &ctail;

        /* build exact copy of reversed "clist" (should be cheap),
         * trnode elements become tailing rather than heading,
         * elements grouping for cached transform is retained */
        for (nxt = clist; nxt != RT_NULL; nxt = nxt->next)
        {
            /* alloc new element as "nxt's" copy */
            elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_QUAD_ALIGN);
            elm->data = nxt->data;
            elm->simd = nxt->simd;
            elm->temp = nxt->temp;
            /* insert element as list's head */
            elm->next = *ptr;
           *ptr = elm;
        }

        /* traverse reversed "clist" to keep original "clist's" order
         * and optimize trnode handling for each tile */
        for (elm = ctail; elm != RT_NULL; elm = elm->next)
        {
            rt_Node *nd = (rt_Node *)((rt_BOUND *)elm->temp)->obj;

            /* skip trnode elements from reversed "clist"
             * as they are handled separately for each tile */
            if (RT_IS_ARRAY(nd))
            {
                continue;
            }

            rt_Surface *srf = (rt_Surface *)nd;

            rt_ELEM *tls = srf->tls, *trn;

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
                     * grouping for cached transform is retained from "clist" */
                    trn = tiles[tline + j];

                    rt_Array *arr = (rt_Array *)srf->trnode;
                    rt_BOUND *trb = (rt_BOUND *)srf->trn->temp;

                    if (trn != RT_NULL && trn->temp == trb)
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
                        trn->data = (rt_cell)tls; /* trnode's last element */
                        trn->simd = arr->s_srf;
                        trn->temp = trb;
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
            rt_si32 i = 0, j = 0;

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
                tiles[tline + j] = clist;
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
    amb[RT_A] = cam->cam->lum[0];

    rt_Light *lgt = RT_NULL;

    for (lgt = lgt_head; lgt != RT_NULL; lgt = lgt->next)
    {
        RT_VEC3_MAD_VAL1(amb, lgt->lgt->col.hdr, lgt->lgt->lum[0]);
        amb[RT_A] += lgt->lgt->lum[0];
    }

#if RT_OPTS_UPDATE_EXT0 != 0
    } /* --<----<-- skip update1 --<----<-- */
#endif /* RT_OPTS_UPDATE_EXT0 */


#if RT_OPTS_RENDER_EXT0 != 0
    if ((opts & RT_OPTS_RENDER_EXT0) == 0)
    { /* -->---->-- skip render0 -->---->-- */
#endif /* RT_OPTS_RENDER_EXT0 */

#if 0 /* SIMD-buffers don't normally require reset between frames */
    reset_color();
#endif /* enable for SIMD-buffers as a debug option if needed */

    /* multi-threaded render */
#if RT_OPTS_THREAD != 0
    if ((opts & RT_OPTS_THREAD) != 0 && this == pfm->get_cur_scene()
#if RT_OPTS_RENDER_EXT1 != 0
    &&  (opts & RT_OPTS_RENDER_EXT1) == 0
#endif /* RT_OPTS_RENDER_EXT1 */
       )
    {
        this->f_render(tdata, thnum, 1);
    }
    else
#endif /* RT_OPTS_THREAD */
    {
        render_scene(this, -thnum, 1);
    }

    pts_c = tharr[0]->s_inf->pts_c[0];

#if RT_OPTS_RENDER_EXT0 != 0
    } /* --<----<-- skip render0 --<----<-- */
#endif /* RT_OPTS_RENDER_EXT0 */


#if RT_OPTS_UPDATE_EXT0 != 0
    if ((opts & RT_OPTS_UPDATE_EXT0) == 0)
    { /* -->---->-- skip update2 -->---->-- */
#endif /* RT_OPTS_UPDATE_EXT0 */

    /* print state done */
    if (g_print)
    {
        RT_PRINT_STATE_DONE();
        g_print = RT_FALSE;
    }

    /* release memory for temporary per-frame allocs */
    for (i = 0; i < thnum; i++)
    {
        tharr[i]->release(tharr[i]->mpool);
    }

    release(mpool);

#if RT_OPTS_UPDATE_EXT0 != 0
    } /* --<----<-- skip update2 --<----<-- */
    else
    {
        pending = 1;
    }
#endif /* RT_OPTS_UPDATE_EXT0 */
}

/*
 * Update portion of the scene with given "index"
 * as part of the multi-threaded update.
 */
rt_void rt_Scene::update_slice(rt_si32 index, rt_si32 phase)
{
    rt_si32 i;

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
            pfm->update0(srf->s_srf);

#if 0 /* SIMD-buffers don't normally require reset between frames */
            memset(srf->s_srf->msc_p[0], 255, RT_BUFFER_POOL*thnum);
#endif /* enable for SIMD-buffers as a debug option if needed */
        }
    }
}

/*
 * Render portion of the frame with given "index"
 * as part of the multi-threaded render.
 */
rt_void rt_Scene::render_slice(rt_si32 index, rt_si32 phase)
{
    /* adjust ray steppers according to antialiasing mode */
    rt_real fha[RT_SIMD_WIDTH], fhi[RT_SIMD_WIDTH], fhu; /* h - hor */
    rt_real fva[RT_SIMD_WIDTH], fvi[RT_SIMD_WIDTH], fvu; /* v - ver */
    rt_si32 i, n;

    if (pfm->fsaa == RT_FSAA_NO)
    {
        for (i = 0; i < pfm->simd_width; i++)
        {
            fha[i] = 0.0f;
            fva[i] = 0.0f;

            fhi[i] = (rt_real)i;
            fvi[i] = (rt_real)index;
        }

        fhu = (rt_real)(pfm->simd_width);
        fvu = (rt_real)thnum;
    }
    else
    if (pfm->fsaa == RT_FSAA_2X) /* alternating */
    {
        rt_real as = 0.25f;
#if RT_FSAA_REGULAR
        rt_real ar = 0.00f;
#else /* RT_FSAA_REGULAR */
        rt_real ar = 0.08f;
#endif /* RT_FSAA_REGULAR */

        for (i = 0; i < pfm->simd_width / 4; i++)
        {
            fha[i*4+0] = (-ar+as);
            fha[i*4+1] = (+ar-as);
            fha[i*4+2] = (+ar-as);
            fha[i*4+3] = (-ar+as);

            fva[i*4+0] = (+ar+as);
            fva[i*4+1] = (-ar-as);
            fva[i*4+2] = (+ar+as);
            fva[i*4+3] = (-ar-as);

            fhi[i*4+0] = (rt_real)(i*2+0);
            fhi[i*4+1] = (rt_real)(i*2+0);
            fhi[i*4+2] = (rt_real)(i*2+1);
            fhi[i*4+3] = (rt_real)(i*2+1);

            fvi[i*4+0] = (rt_real)index;
            fvi[i*4+1] = (rt_real)index;
            fvi[i*4+2] = (rt_real)index;
            fvi[i*4+3] = (rt_real)index;
        }

        fhu = (rt_real)(pfm->simd_width / 2);
        fvu = (rt_real)thnum;
    }
    else
    if (pfm->fsaa == RT_FSAA_4X)
    {
        rt_real as = 0.25f;
#if RT_FSAA_REGULAR
        rt_real ar = 0.00f;
#else /* RT_FSAA_REGULAR */
        rt_real ar = 0.08f;
#endif /* RT_FSAA_REGULAR */

        for (i = 0; i < pfm->simd_width / 4; i++)
        {
            fha[i*4+0] = (-ar-as);
            fha[i*4+1] = (-ar+as);
            fha[i*4+2] = (+ar-as);
            fha[i*4+3] = (+ar+as);

            fva[i*4+0] = (+ar-as);
            fva[i*4+1] = (-ar-as);
            fva[i*4+2] = (+ar+as);
            fva[i*4+3] = (-ar+as);

            fhi[i*4+0] = (rt_real)i;
            fhi[i*4+1] = (rt_real)i;
            fhi[i*4+2] = (rt_real)i;
            fhi[i*4+3] = (rt_real)i;

            fvi[i*4+0] = (rt_real)index;
            fvi[i*4+1] = (rt_real)index;
            fvi[i*4+2] = (rt_real)index;
            fvi[i*4+3] = (rt_real)index;
        }

        fhu = (rt_real)(pfm->simd_width / 4);
        fvu = (rt_real)thnum;
    }
    else
    if (pfm->fsaa == RT_FSAA_8X) /* 8x reserved */
    {
        ;
    }

/*  rt_SIMD_CAMERA */

    rt_SIMD_CAMERA *s_cam = tharr[index]->s_cam;

    RT_SIMD_SET(s_cam->t_max, RT_INF);

    RT_SIMD_SET(s_cam->dir_x, dir[RT_X]);
    RT_SIMD_SET(s_cam->dir_y, dir[RT_Y]);
    RT_SIMD_SET(s_cam->dir_z, dir[RT_Z]);

    RT_SIMD_SET(s_cam->hor_x, hor[RT_X]);
    RT_SIMD_SET(s_cam->hor_y, hor[RT_Y]);
    RT_SIMD_SET(s_cam->hor_z, hor[RT_Z]);

    RT_SIMD_SET(s_cam->ver_x, ver[RT_X]);
    RT_SIMD_SET(s_cam->ver_y, ver[RT_Y]);
    RT_SIMD_SET(s_cam->ver_z, ver[RT_Z]);

    RT_SIMD_SET(s_cam->hor_u, fhu);
    RT_SIMD_SET(s_cam->ver_u, fvu);

    RT_SIMD_SET(s_cam->clamp, (rt_real)255);
    RT_SIMD_SET(s_cam->cmask, (rt_elem)255);

    RT_SIMD_SET(s_cam->col_r, amb[RT_R]);
    RT_SIMD_SET(s_cam->col_g, amb[RT_G]);
    RT_SIMD_SET(s_cam->col_b, amb[RT_B]);
    RT_SIMD_SET(s_cam->l_amb, amb[RT_A]);

    RT_SIMD_SET(s_cam->x_row, (rt_real)(x_row << pfm->fsaa));
    RT_SIMD_SET(s_cam->idx_h, pfm->simd_width);

/*  rt_SIMD_CONTEXT */

    rt_SIMD_CONTEXT *s_ctx = tharr[index]->s_ctx;

    s_ctx->param[1] = -((opts & RT_OPTS_GAMMA) == 0) & RT_PROP_GAMMA;
    RT_SIMD_SET(s_ctx->t_min, cam->pov);
    RT_SIMD_SET(s_ctx->wmask, -1);

    RT_SIMD_SET(s_ctx->org_x, pos[RT_X]);
    RT_SIMD_SET(s_ctx->org_y, pos[RT_Y]);
    RT_SIMD_SET(s_ctx->org_z, pos[RT_Z]);

/*  rt_SIMD_INFOX */

    rt_SIMD_INFOX *s_inf = tharr[index]->s_inf;

    s_inf->ctx = s_ctx;
    s_inf->cam = s_cam;
    s_inf->lst = clist;

    s_inf->thndx = index;
    s_inf->thnum = thnum;
    s_inf->depth = depth;
    s_inf->fsaa  = pfm->fsaa;

    s_inf->pt_on = pt_on;

    RT_SIMD_SET(s_inf->pts_c, pts_c);

    for (n = RT_MAX(1, pt_on); n > 0; n--)
    {
        /* use of integer indices for primary rays update
         * makes related fp-math independent from SIMD width */
        for (i = 0; i < pfm->simd_width; i++)
        {
            s_cam->index[i] = i;
            s_inf->hor_c[i] = fhi[i];

            s_inf->hor_i[i] = fhi[i];
            s_inf->ver_i[i] = fvi[i];

            s_cam->hor_a[i] = fha[i];
            s_cam->ver_a[i] = fva[i];
        }

        s_inf->depth = depth;
        RT_SIMD_SET(s_ctx->wmask, -1);

        /* render frame based on tilebuffer */
        pfm->render0(s_inf);
    }
}

/*
 * Return framebuffer's stride in pixels.
 */
rt_si32 rt_Scene::get_x_row()
{
    return x_row;
}

/*
 * Print current state during next render-call.
 * Has global scope and effect on any instance.
 * The flag is reset upon rendering completion.
 */
rt_void rt_Scene::print_state()
{
    g_print = RT_TRUE;
}

/*
 * Generate next random number using XX-bit LCG method.
 */
static
rt_ui64 randomXX(rt_ui64 seed)
{
#if RT_PRNG != LCG48

    /* use 48-bit seeding to decorrelate lesser LCG */
    return (seed * LL(25214903917) + 11) & LL(0x0000FFFFFFFFFFFF);

#else /* RT_PRNG == LCG48 */

    /* use 32-bit seeding to decorrelate 48-bit LCG */
    return (seed * 214013 + 2531011) & 0xFFFFFFFF;

#endif /* RT_PRNG == LCG48 */
}

/*
 * Reset current state of framebuffer's seed-plane for path-tracer.
 */
rt_void rt_Scene::reset_pseed()
{
    if ((opts & RT_OPTS_PT) != 0)
    {
        return;
    }

    rt_si32 k, n = 4 * x_row * y_res;
    rt_ui64 seed = 1;

    for (k = 0; k < n; k++)
    {
        seed = randomXX(seed);
        pseed[k] = (rt_elem)seed;
    }
}

/*
 * Reset current state of framebuffer's color-planes for path-tracer.
 */
rt_void rt_Scene::reset_color()
{
    if ((opts & RT_OPTS_PT) != 0)
    {
        return;
    }

    pts_c = 0.0f;

    memset(ptr_r, 0, 4 * x_row * y_res * sizeof(rt_real));
    memset(ptr_g, 0, 4 * x_row * y_res * sizeof(rt_real));
    memset(ptr_b, 0, 4 * x_row * y_res * sizeof(rt_real));
}

/*
 * Get runtime optimization flags.
 */
rt_si32 rt_Scene::get_opts()
{
    return opts;
}

/*
 * Set runtime optimization flags,
 * except those turned off in the original scene definition.
 */
rt_si32 rt_Scene::set_opts(rt_si32 opts)
{
    this->opts = opts & ~scn->opts;

    /* trigger update of the whole hierarchy,
     * safe to reset time as "rootobj" never has an animator,
     * "rootobj's" time is restored within the update */
    rootobj.time = -1;

    return opts;
}

/*
 * Get path-tracer mode: 0 - off, n - on (number of frames between updates).
 */
rt_si32 rt_Scene::get_pton()
{
    return this->pt_on;
}

/*
 * Set path-tracer mode: 0 - off, n - on (number of frames between updates).
 */
rt_si32 rt_Scene::set_pton(rt_si32 pton)
{
    if ((opts & RT_OPTS_PT) == 0) /* if path-tracer is not optimized out */
    {
        rt_si32 pt_on = this->pt_on;

        this->pt_on = pton;

        if (this->pt_on && !pt_on)
        {
            reset_pseed();
            reset_color();
        }
    }

    return this->pt_on;
}

/*
 * Return current camera index.
 */
rt_si32 rt_Scene::get_cam_idx()
{
    return cam_idx;
}

/*
 * Select next camera in the list, return its index.
 */
rt_si32 rt_Scene::next_cam()
{
    if (cam->next != RT_NULL)
    {
        cam = cam->next;
        cam_idx++;
    }
    else
    {
        cam = cam_head;
        cam_idx = 0;
    }

    return cam_idx;
}

/*
 * Return pointer to the framebuffer.
 */
rt_ui32* rt_Scene::get_frame()
{
    return frame;
}

/*
 * Save current frame to an image.
 */
rt_void rt_Scene::save_frame(rt_si32 index)
{
    rt_char name[20];

    if (index < 1000)
    {
        strncpy(name, "scrXXX.bmp", 20);
    }
    else
    {
        strncpy(name, "scrXXX-Y.bmp", 20);
    }

    /* prepare filename string */
    name[5] = '0' + (index % 10);
    index /= 10;
    name[4] = '0' + (index % 10);
    index /= 10;
    name[3] = '0' + (index % 10);

    if (index >= 10)
    {
        index /= 10;
        index -= 1;
        name[7] = '0' + (index % 10);
    }

    /* prepare frame's image */
    rt_TEX tex;
    tex.ptex = get_frame();
    tex.tex_num = +x_row; /* <- temp fix for frame's stride */
    tex.x_dim = +x_res;
    tex.y_dim = -y_res;

    /* save frame's image */
    save_image(this, name, &tex);
}

/*
 * Return pointer to the platform container.
 */
rt_Platform* rt_Scene::get_platform()
{
    return pfm;
}

/*
 * Deinitialize scene.
 */
rt_Scene::~rt_Scene()
{
    rt_si32 i;

    pfm->del_scene(this);

    /* destroy scene threads array */
    for (i = 0; i < thnum; i++)
    {
        delete tharr[i];
    }

    /* destroy object hierarchy */
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
/*****************************   MISC RENDERING   *****************************/
/******************************************************************************/

#define II  0xFF000000
#define OO  0xFFFFFFFF

#define dW  5
#define dH  7

rt_ui32 digits[10][dH][dW] = 
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
rt_void rt_Scene::render_num(rt_si32 x, rt_si32 y,
                             rt_si32 d, rt_si32 z, rt_ui32 num)
{
    rt_si32 arr[16], i, c, k;

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

    rt_si32 xd, yd, xz, yz;
    rt_ui32 *src, *dst;

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
                        if (dst >= frame && dst < frame + y_res * x_row)
                        {
                           *dst++ = *src;
                        }
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

#define RT_PLOT_FRAGS   1   /* 1 enables (on -z) plotting of FSAA samples */
#define RT_PLOT_FUNCS   1   /* 1 enables (on -z) plotting of Fresnel funcs */
#define RT_PLOT_TRIGS   1   /* 1 enables (on -z) plotting of sin/cos funcs */

/*
 * Plot fragments into their respective framebuffers then save.
 * Scene's framebuffer is first cleared then overwritten.
 */
rt_void rt_Scene::plot_frags()
{
    if (RT_ELEMENT != 32)
    {
        /* plotting of funcs and trigs is ignored, use fp32 target */
        RT_LOGI("(no funcs, use fp32) ");
    }

#if RT_PLOT_FRAGS

    rt_real fdh[8], fdv[8];

    rt_si32 r = RT_MIN(x_res, y_res) - 1, i;
    rt_si32 x = (x_res - r) / 2, y = (y_res - r) / 2;

    memset(frame, 0, x_row * y_res * sizeof(rt_ui32));

    for (i = 0; i <= r; i++)
    {
        frame[(y+0)*x_row+(x+i)] = 0x000000FF;
        frame[(y+i)*x_row+(x+0)] = 0x000000FF;
        frame[(y+i)*x_row+(x+r)] = 0x000000FF;
        frame[(y+r)*x_row+(x+i)] = 0x000000FF;
    }
    /* plot samples for 2X alternating slope pattern */
    {
        rt_real as = 0.25f;
#if RT_FSAA_REGULAR
        rt_real ar = 0.00f;
#else /* RT_FSAA_REGULAR */
        rt_real ar = 0.08f;
#endif /* RT_FSAA_REGULAR */

        fdh[0] = ((-ar+as) + 0.5f) * r + 0.5f;
        fdh[1] = ((+ar-as) + 0.5f) * r + 0.5f;
        fdh[2] = ((+ar-as) + 0.5f) * r + 0.5f;
        fdh[3] = ((-ar+as) + 0.5f) * r + 0.5f;

        fdv[0] = ((+ar+as) + 0.5f) * r + 0.5f;
        fdv[1] = ((-ar-as) + 0.5f) * r + 0.5f;
        fdv[2] = ((+ar+as) + 0.5f) * r + 0.5f;
        fdv[3] = ((-ar-as) + 0.5f) * r + 0.5f;

        frame[(y + rt_si32(fdv[0]))*x_row+(x + rt_si32(fdh[0]))] = 0x00FF0000;
        frame[(y + rt_si32(fdv[1]))*x_row+(x + rt_si32(fdh[1]))] = 0x00FF0000;
        frame[(y + rt_si32(fdv[2]))*x_row+(x + rt_si32(fdh[2]))] = 0x0000FF00;
        frame[(y + rt_si32(fdv[3]))*x_row+(x + rt_si32(fdh[3]))] = 0x0000FF00;
    }

    save_frame(820);

    memset(frame, 0, x_row * y_res * sizeof(rt_ui32));

    for (i = 0; i <= r; i++)
    {
        frame[(y+0)*x_row+(x+i)] = 0x000000FF;
        frame[(y+i)*x_row+(x+0)] = 0x000000FF;
        frame[(y+i)*x_row+(x+r)] = 0x000000FF;
        frame[(y+r)*x_row+(x+i)] = 0x000000FF;
    }
    /* plot samples for 4X rotated grid pattern */
    {
        rt_real as = 0.25f;
#if RT_FSAA_REGULAR
        rt_real ar = 0.00f;
#else /* RT_FSAA_REGULAR */
        rt_real ar = 0.08f;
#endif /* RT_FSAA_REGULAR */

        fdh[0] = ((-ar-as) + 0.5f) * r + 0.5f;
        fdh[1] = ((-ar+as) + 0.5f) * r + 0.5f;
        fdh[2] = ((+ar-as) + 0.5f) * r + 0.5f;
        fdh[3] = ((+ar+as) + 0.5f) * r + 0.5f;

        fdv[0] = ((+ar-as) + 0.5f) * r + 0.5f;
        fdv[1] = ((-ar-as) + 0.5f) * r + 0.5f;
        fdv[2] = ((+ar+as) + 0.5f) * r + 0.5f;
        fdv[3] = ((-ar+as) + 0.5f) * r + 0.5f;

        frame[(y + rt_si32(fdv[0]))*x_row+(x + rt_si32(fdh[0]))] = 0x00FF0000;
        frame[(y + rt_si32(fdv[1]))*x_row+(x + rt_si32(fdh[1]))] = 0x00FF0000;
        frame[(y + rt_si32(fdv[2]))*x_row+(x + rt_si32(fdh[2]))] = 0x00FF0000;
        frame[(y + rt_si32(fdv[3]))*x_row+(x + rt_si32(fdh[3]))] = 0x00FF0000;
    }

    save_frame(840);

#endif /* RT_PLOT_FRAGS */
}

/*
 * Swap (v4) for available 128-bit target before enabling plot, use 32-bit fp.
 */
#if RT_PLOT_FUNCS

namespace rt_simd_128v4
{
rt_void plot_fresnel(rt_SIMD_INFOP *s_inf);
}

namespace rt_simd_128v4
{
rt_void plot_schlick(rt_SIMD_INFOP *s_inf);
}

namespace rt_simd_128v4
{
rt_void plot_fresnel_metal_fast(rt_SIMD_INFOP *s_inf);
}

namespace rt_simd_128v4
{
rt_void plot_fresnel_metal_slow(rt_SIMD_INFOP *s_inf);
}

#endif /* RT_PLOT_FUNCS */

/*
 * Plot functions into their respective framebuffers then save.
 * Scene's framebuffer is first cleared then overwritten.
 */
rt_void rt_Scene::plot_funcs()
{
    if (RT_ELEMENT != 32)
    {
        /* plotting of funcs and trigs is ignored, use fp32 target */
        return;
    }

#if RT_PLOT_FUNCS

    /* reserve memory for temporary buffer in the heap */
    rt_pntr s_ptr = reserve(4000, RT_SIMD_ALIGN);

    /* allocate root SIMD structure */
    rt_SIMD_INFOP *s_inf = (rt_SIMD_INFOP *)
            alloc(sizeof(rt_SIMD_INFOP),
                            RT_SIMD_ALIGN);

    memset(s_inf, 0, sizeof(rt_SIMD_INFOP));

    /* allocate regs SIMD structure */
    rt_SIMD_REGS *s_reg = (rt_SIMD_REGS *)
            alloc(sizeof(rt_SIMD_REGS),
                            RT_SIMD_ALIGN);

    ASM_INIT(s_inf, s_reg)

    rt_si32 i, h = y_res - 1;
    rt_fp32 s = RT_PI_2 / x_res;

    RT_SIMD_SET(s_inf->c_rfr, (1.0/1.5));
    RT_SIMD_SET(s_inf->rfr_2, (1.0/1.5)*(1.0/1.5));

    memset(frame, 0, x_row * y_res * sizeof(rt_ui32));

    for (i = 0; i < x_res / 4; i++)
    {
        s_inf->i_cos[0*4+0] = -RT_COS(s*(i*4+0));
        s_inf->i_cos[0*4+1] = -RT_COS(s*(i*4+1));
        s_inf->i_cos[0*4+2] = -RT_COS(s*(i*4+2));
        s_inf->i_cos[0*4+3] = -RT_COS(s*(i*4+3));

        rt_simd_128v4::plot_fresnel(s_inf);

        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+0])*h)*x_row+i*4+0] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+1])*h)*x_row+i*4+1] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+2])*h)*x_row+i*4+2] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+3])*h)*x_row+i*4+3] = 0x000000FF;

#if RT_DEBUG >= 2

        RT_LOGI("Fresnel_outer[%03X] = %f\n", i*4+0, s_inf->o_rfl[0*4+0]);
        RT_LOGI("Fresnel_outer[%03X] = %f\n", i*4+1, s_inf->o_rfl[0*4+1]);
        RT_LOGI("Fresnel_outer[%03X] = %f\n", i*4+2, s_inf->o_rfl[0*4+2]);
        RT_LOGI("Fresnel_outer[%03X] = %f\n", i*4+3, s_inf->o_rfl[0*4+3]);

#endif /* RT_DEBUG */
    }

    save_frame(910);

    memset(frame, 0, x_row * y_res * sizeof(rt_ui32));

    for (i = 0; i < x_res / 4; i++)
    {
        s_inf->i_cos[0*4+0] = -RT_COS(s*(i*4+0));
        s_inf->i_cos[0*4+1] = -RT_COS(s*(i*4+1));
        s_inf->i_cos[0*4+2] = -RT_COS(s*(i*4+2));
        s_inf->i_cos[0*4+3] = -RT_COS(s*(i*4+3));

        rt_simd_128v4::plot_schlick(s_inf);

        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+0])*h)*x_row+i*4+0] = 0x00FF0000;
        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+1])*h)*x_row+i*4+1] = 0x00FF0000;
        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+2])*h)*x_row+i*4+2] = 0x00FF0000;
        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+3])*h)*x_row+i*4+3] = 0x00FF0000;

#if RT_DEBUG >= 2

        RT_LOGI("Schlick_outer[%03X] = %f\n", i*4+0, s_inf->o_rfl[0*4+0]);
        RT_LOGI("Schlick_outer[%03X] = %f\n", i*4+1, s_inf->o_rfl[0*4+1]);
        RT_LOGI("Schlick_outer[%03X] = %f\n", i*4+2, s_inf->o_rfl[0*4+2]);
        RT_LOGI("Schlick_outer[%03X] = %f\n", i*4+3, s_inf->o_rfl[0*4+3]);

#endif /* RT_DEBUG */
    }

    save_frame(920);

    RT_SIMD_SET(s_inf->c_rfr, (1.5/1.0));
    RT_SIMD_SET(s_inf->rfr_2, (1.5/1.0)*(1.5/1.0));

    memset(frame, 0, x_row * y_res * sizeof(rt_ui32));

    for (i = 0; i < x_res / 4; i++)
    {
        s_inf->i_cos[0*4+0] = -RT_COS(s*(i*4+0));
        s_inf->i_cos[0*4+1] = -RT_COS(s*(i*4+1));
        s_inf->i_cos[0*4+2] = -RT_COS(s*(i*4+2));
        s_inf->i_cos[0*4+3] = -RT_COS(s*(i*4+3));

        rt_simd_128v4::plot_fresnel(s_inf);

        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+0])*h)*x_row+i*4+0] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+1])*h)*x_row+i*4+1] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+2])*h)*x_row+i*4+2] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+3])*h)*x_row+i*4+3] = 0x000000FF;

#if RT_DEBUG >= 2

        RT_LOGI("Fresnel_inner[%03X] = %f\n", i*4+0, s_inf->o_rfl[0*4+0]);
        RT_LOGI("Fresnel_inner[%03X] = %f\n", i*4+1, s_inf->o_rfl[0*4+1]);
        RT_LOGI("Fresnel_inner[%03X] = %f\n", i*4+2, s_inf->o_rfl[0*4+2]);
        RT_LOGI("Fresnel_inner[%03X] = %f\n", i*4+3, s_inf->o_rfl[0*4+3]);

#endif /* RT_DEBUG */
    }

    save_frame(930);

    memset(frame, 0, x_row * y_res * sizeof(rt_ui32));

    for (i = 0; i < x_res / 4; i++)
    {
        s_inf->i_cos[0*4+0] = -RT_COS(s*(i*4+0));
        s_inf->i_cos[0*4+1] = -RT_COS(s*(i*4+1));
        s_inf->i_cos[0*4+2] = -RT_COS(s*(i*4+2));
        s_inf->i_cos[0*4+3] = -RT_COS(s*(i*4+3));

        rt_simd_128v4::plot_schlick(s_inf);

        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+0])*h)*x_row+i*4+0] = 0x00FF0000;
        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+1])*h)*x_row+i*4+1] = 0x00FF0000;
        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+2])*h)*x_row+i*4+2] = 0x00FF0000;
        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+3])*h)*x_row+i*4+3] = 0x00FF0000;

#if RT_DEBUG >= 2

        RT_LOGI("Schlick_inner[%03X] = %f\n", i*4+0, s_inf->o_rfl[0*4+0]);
        RT_LOGI("Schlick_inner[%03X] = %f\n", i*4+1, s_inf->o_rfl[0*4+1]);
        RT_LOGI("Schlick_inner[%03X] = %f\n", i*4+2, s_inf->o_rfl[0*4+2]);
        RT_LOGI("Schlick_inner[%03X] = %f\n", i*4+3, s_inf->o_rfl[0*4+3]);

#endif /* RT_DEBUG */
    }

    save_frame(940);

    RT_SIMD_SET(s_inf->c_rcp, (0.27));
    RT_SIMD_SET(s_inf->ext_2, (2.77)*(2.77));

    memset(frame, 0, x_row * y_res * sizeof(rt_ui32));

    for (i = 0; i < x_res / 4; i++)
    {
        s_inf->i_cos[0*4+0] = -RT_COS(s*(i*4+0));
        s_inf->i_cos[0*4+1] = -RT_COS(s*(i*4+1));
        s_inf->i_cos[0*4+2] = -RT_COS(s*(i*4+2));
        s_inf->i_cos[0*4+3] = -RT_COS(s*(i*4+3));

        rt_simd_128v4::plot_fresnel_metal_fast(s_inf);

        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+0])*h)*x_row+i*4+0] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+1])*h)*x_row+i*4+1] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+2])*h)*x_row+i*4+2] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+3])*h)*x_row+i*4+3] = 0x000000FF;

#if RT_DEBUG >= 2

        RT_LOGI("Fresnel_metal_fast[%03X] = %f\n", i*4+0, s_inf->o_rfl[0*4+0]);
        RT_LOGI("Fresnel_metal_fast[%03X] = %f\n", i*4+1, s_inf->o_rfl[0*4+1]);
        RT_LOGI("Fresnel_metal_fast[%03X] = %f\n", i*4+2, s_inf->o_rfl[0*4+2]);
        RT_LOGI("Fresnel_metal_fast[%03X] = %f\n", i*4+3, s_inf->o_rfl[0*4+3]);

#endif /* RT_DEBUG */
    }

    save_frame(950);

    memset(frame, 0, x_row * y_res * sizeof(rt_ui32));

    for (i = 0; i < x_res / 4; i++)
    {
        s_inf->i_cos[0*4+0] = -RT_COS(s*(i*4+0));
        s_inf->i_cos[0*4+1] = -RT_COS(s*(i*4+1));
        s_inf->i_cos[0*4+2] = -RT_COS(s*(i*4+2));
        s_inf->i_cos[0*4+3] = -RT_COS(s*(i*4+3));

        rt_simd_128v4::plot_fresnel_metal_slow(s_inf);

        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+0])*h)*x_row+i*4+0] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+1])*h)*x_row+i*4+1] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+2])*h)*x_row+i*4+2] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->o_rfl[0*4+3])*h)*x_row+i*4+3] = 0x000000FF;

#if RT_DEBUG >= 2

        RT_LOGI("Fresnel_metal_slow[%03X] = %f\n", i*4+0, s_inf->o_rfl[0*4+0]);
        RT_LOGI("Fresnel_metal_slow[%03X] = %f\n", i*4+1, s_inf->o_rfl[0*4+1]);
        RT_LOGI("Fresnel_metal_slow[%03X] = %f\n", i*4+2, s_inf->o_rfl[0*4+2]);
        RT_LOGI("Fresnel_metal_slow[%03X] = %f\n", i*4+3, s_inf->o_rfl[0*4+3]);

#endif /* RT_DEBUG */
    }

    save_frame(960);

    ASM_DONE(s_inf)

    release(s_ptr);

    rt_si32 r = RT_MIN(x_res, y_res) - 1;
    rt_si32 x = (x_res - r) / 2, y = (y_res - r) / 2;

    memset(frame, 0, x_row * y_res * sizeof(rt_ui32));

    for (i = 0; i <= r; i++)
    {
        frame[(y+0)*x_row+(x+i)] = 0x000000FF;
        frame[(y+i)*x_row+(x+0)] = 0x000000FF;
        frame[(y+i)*x_row+(x+r)] = 0x000000FF;
        frame[(y+r)*x_row+(x+i)] = 0x000000FF;
    }
    /* plot reference and actual Gamma color conversion */
    for (i = 0, s = r; i <= r; i++)
    {
        frame[(y+rt_si32((1.0f-RT_POW(i/s,1/2.2))*s))*x_row+x+i] = 0x00FF0000;
        frame[(y+rt_si32((1.0f-RT_POW(i/s,  2.2))*s))*x_row+x+i] = 0x00FF0000;

        frame[(y+rt_si32((1.0f-RT_POW(i/s,1/2.0))*s))*x_row+x+i] = 0x0000FF00;
        frame[(y+rt_si32((1.0f-RT_POW(i/s,  2.0))*s))*x_row+x+i] = 0x0000FF00;
    }

    save_frame(970);

#endif /* RT_PLOT_FUNCS */
}

/*
 * Swap (v4) for available 128-bit target before enabling plot, use 32-bit fp.
 */
#if RT_PLOT_TRIGS

namespace rt_simd_128v4
{
rt_void plot_sin(rt_SIMD_INFOX *s_inf);
}

namespace rt_simd_128v4
{
rt_void plot_cos(rt_SIMD_INFOX *s_inf);
}

namespace rt_simd_128v4
{
rt_void plot_asin(rt_SIMD_INFOX *s_inf);
}

namespace rt_simd_128v4
{
rt_void plot_acos(rt_SIMD_INFOX *s_inf);
}

#endif /* RT_PLOT_TRIGS */

/*
 * Plot trigonometrics into their respective framebuffers then save.
 * Scene's framebuffer is first cleared then overwritten.
 */
rt_void rt_Scene::plot_trigs()
{
    if (RT_ELEMENT != 32)
    {
        /* plotting of funcs and trigs is ignored, use fp32 target */
        return;
    }

#if RT_PLOT_TRIGS

    /* reserve memory for temporary buffer in the heap */
    rt_pntr s_ptr = reserve(4000, RT_SIMD_ALIGN);

    /* allocate root SIMD structure */
    rt_SIMD_INFOX *s_inf = (rt_SIMD_INFOX *)
            alloc(sizeof(rt_SIMD_INFOX),
                            RT_SIMD_ALIGN);

    memset(s_inf, 0, sizeof(rt_SIMD_INFOX));

    /* init power series constants for sin, cos */
    RT_SIMD_SET(s_inf->sin_3, -0.1666666666666666666666666666666666666666666);
    RT_SIMD_SET(s_inf->sin_5, +0.0083333333333333333333333333333333333333333);
    RT_SIMD_SET(s_inf->sin_7, -0.0001984126984126984126984126984126984126984);
    RT_SIMD_SET(s_inf->sin_9, +0.0000027557319223985890652557319223985890652);
    RT_SIMD_SET(s_inf->cos_4, +0.0416666666666666666666666666666666666666666);
    RT_SIMD_SET(s_inf->cos_6, -0.0013888888888888888888888888888888888888888);
    RT_SIMD_SET(s_inf->cos_8, +0.0000248015873015873015873015873015873015873);

#if RT_DEBUG >= 1

    /* init polynomial constants for asin, acos */
    RT_SIMD_SET(s_inf->asn_1, -0.0187293);
    RT_SIMD_SET(s_inf->asn_2, +0.0742610);
    RT_SIMD_SET(s_inf->asn_3, -0.2121144);
    RT_SIMD_SET(s_inf->asn_4, +1.5707288);
    RT_SIMD_SET(s_inf->tmp_1, +RT_PI_2);

#endif /* RT_DEBUG >= 1 */

    /* allocate regs SIMD structure */
    rt_SIMD_REGS *s_reg = (rt_SIMD_REGS *)
            alloc(sizeof(rt_SIMD_REGS),
                            RT_SIMD_ALIGN);

    ASM_INIT(s_inf, s_reg)

    rt_si32 i, k = (y_res - 1) / 2;
    rt_fp32 s = RT_2_PI / x_res, t = 0.53f;

    memset(frame, 0, x_row * y_res * sizeof(rt_ui32));

    for (i = 0; i < x_res / 4; i++)
    {
        s_inf->hor_i[0*4+0] = s*(i*4+0 - x_res/2);
        s_inf->hor_i[0*4+1] = s*(i*4+1 - x_res/2);
        s_inf->hor_i[0*4+2] = s*(i*4+2 - x_res/2);
        s_inf->hor_i[0*4+3] = s*(i*4+3 - x_res/2);

        rt_simd_128v4::plot_sin(s_inf);

        frame[rt_si32((1.0f-s_inf->pts_o[0*4+0]*t)*k)*x_row+i*4+0] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->pts_o[0*4+1]*t)*k)*x_row+i*4+1] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->pts_o[0*4+2]*t)*k)*x_row+i*4+2] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->pts_o[0*4+3]*t)*k)*x_row+i*4+3] = 0x000000FF;

#if RT_DEBUG >= 2

        RT_LOGI("sin[%03X] = %f\n", i*4+0, s_inf->pts_o[0*4+0]);
        RT_LOGI("sin[%03X] = %f\n", i*4+1, s_inf->pts_o[0*4+1]);
        RT_LOGI("sin[%03X] = %f\n", i*4+2, s_inf->pts_o[0*4+2]);
        RT_LOGI("sin[%03X] = %f\n", i*4+3, s_inf->pts_o[0*4+3]);

#endif /* RT_DEBUG */
    }

    save_frame(980);

    memset(frame, 0, x_row * y_res * sizeof(rt_ui32));

    for (i = 0; i < x_res / 4; i++)
    {
        s_inf->hor_i[0*4+0] = s*(i*4+0 - x_res/2);
        s_inf->hor_i[0*4+1] = s*(i*4+1 - x_res/2);
        s_inf->hor_i[0*4+2] = s*(i*4+2 - x_res/2);
        s_inf->hor_i[0*4+3] = s*(i*4+3 - x_res/2);

        rt_simd_128v4::plot_cos(s_inf);

        frame[rt_si32((1.0f-s_inf->pts_o[0*4+0]*t)*k)*x_row+i*4+0] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->pts_o[0*4+1]*t)*k)*x_row+i*4+1] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->pts_o[0*4+2]*t)*k)*x_row+i*4+2] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->pts_o[0*4+3]*t)*k)*x_row+i*4+3] = 0x000000FF;

#if RT_DEBUG >= 2

        RT_LOGI("cos[%03X] = %f\n", i*4+0, s_inf->pts_o[0*4+0]);
        RT_LOGI("cos[%03X] = %f\n", i*4+1, s_inf->pts_o[0*4+1]);
        RT_LOGI("cos[%03X] = %f\n", i*4+2, s_inf->pts_o[0*4+2]);
        RT_LOGI("cos[%03X] = %f\n", i*4+3, s_inf->pts_o[0*4+3]);

#endif /* RT_DEBUG */
    }

    save_frame(990);

#if RT_DEBUG >= 1

    /* asin/acos plotting is not up to scale yet */
    s = 2.0f / x_res;
    t = 1.0f / RT_PI;

    memset(frame, 0, x_row * y_res * sizeof(rt_ui32));

    for (i = 0; i < x_res / 4; i++)
    {
        s_inf->hor_i[0*4+0] = s*(i*4+0 - x_res/2);
        s_inf->hor_i[0*4+1] = s*(i*4+1 - x_res/2);
        s_inf->hor_i[0*4+2] = s*(i*4+2 - x_res/2);
        s_inf->hor_i[0*4+3] = s*(i*4+3 - x_res/2);

        rt_simd_128v4::plot_acos(s_inf);

        frame[rt_si32((1.0f-s_inf->pts_o[0*4+0]*t)*k)*x_row+i*4+0] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->pts_o[0*4+1]*t)*k)*x_row+i*4+1] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->pts_o[0*4+2]*t)*k)*x_row+i*4+2] = 0x000000FF;
        frame[rt_si32((1.0f-s_inf->pts_o[0*4+3]*t)*k)*x_row+i*4+3] = 0x000000FF;

#if RT_DEBUG >= 2

        RT_LOGI("acos[%03X] = %f\n", i*4+0, s_inf->pts_o[0*4+0]);
        RT_LOGI("acos[%03X] = %f\n", i*4+1, s_inf->pts_o[0*4+1]);
        RT_LOGI("acos[%03X] = %f\n", i*4+2, s_inf->pts_o[0*4+2]);
        RT_LOGI("acos[%03X] = %f\n", i*4+3, s_inf->pts_o[0*4+3]);

#endif /* RT_DEBUG */
    }

    save_frame(900);

#endif /* RT_DEBUG >= 1 */

    ASM_DONE(s_inf)

    release(s_ptr);

#endif /* RT_PLOT_TRIGS */
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
