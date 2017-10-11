/******************************************************************************/
/* Copyright (c) 2013-2017 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_ENGINE_H
#define RT_ENGINE_H

#include "rtconf.h"
#include "rtgeom.h"
#include "object.h"
#include "system.h"
#include "tracer.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * engine.h: Interface for the scene manager.
 *
 * More detailed description of this subsystem is given in engine.cpp.
 * Recommended naming scheme for C++ types and definitions is given in rtbase.h.
 */

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

/*
 * Fullscreen anti-aliasing modes.
 */
#define RT_FSAA_NO                  0
#define RT_FSAA_4X                  1

/* Classes */

class rt_Platform;
class rt_SceneThread;
class rt_Scene;

/******************************************************************************/
/*****************************   MULTI-THREADING   ****************************/
/******************************************************************************/

typedef rt_pntr (*rt_FUNC_INIT)(rt_si32 thnum, rt_Scene *scn);
typedef rt_void (*rt_FUNC_TERM)(rt_pntr tdata, rt_si32 thnum);
typedef rt_void (*rt_FUNC_UPDATE)(rt_pntr tdata, rt_si32 thnum, rt_si32 phase);
typedef rt_void (*rt_FUNC_RENDER)(rt_pntr tdata, rt_si32 thnum, rt_si32 phase);

/*
 * Platform abstraction container.
 */
class rt_Platform : private rt_LogRedirect
{
/*  fields */

    private:

    /* memory management functions */
    rt_FUNC_ALLOC      f_alloc;
    rt_FUNC_FREE       f_free;

    /* threads management functions */
    rt_FUNC_INIT        f_init;
    rt_FUNC_TERM        f_term;
    rt_FUNC_UPDATE      f_update;
    rt_FUNC_RENDER      f_render;

    /* platform-specific thread-pool */
    rt_si32             thnum;
    rt_pntr             tdata;

    /* scene-list for given platform */
    rt_Scene           *head;
    rt_Scene           *tail;
    rt_Scene           *cur;

/*  methods */

    public:

    rt_Platform(rt_FUNC_ALLOC f_alloc, rt_FUNC_FREE f_free, rt_si32 thnum = 0,
                rt_FUNC_INIT f_init = RT_NULL, rt_FUNC_TERM f_term = RT_NULL,
                rt_FUNC_UPDATE f_update = RT_NULL,
                rt_FUNC_RENDER f_render = RT_NULL,
                rt_FUNC_PRINT_LOG f_print_log = RT_NULL,
                rt_FUNC_PRINT_ERR f_print_err = RT_NULL);

    virtual
   ~rt_Platform();

    rt_void     add_scene(rt_Scene *scn);
    rt_Scene*   get_cur_scene();
    rt_Scene*   set_cur_scene(rt_Scene *scn);
    rt_void     next_scene();

    friend      class rt_Scene;
};

/******************************************************************************/
/*********************************   THREAD   *********************************/
/******************************************************************************/

/*
 * SceneThread contains set of structures used by the scene manager per thread.
 */
class rt_SceneThread : public rt_Heap
{
/*  fields */

    private:

    /* scene pointer and thread index */
    rt_Scene           *scene;
    rt_si32             index;

    /* x-coord boundaries for surface's
     * projected bbox in the tilebuffer */
    rt_si32            *txmin;
    rt_si32            *txmax;
    /* temporary bbox verts buffer */
    rt_VERT            *verts;

    public:

    /* backend specific structures */
    rt_SIMD_INFOX      *s_inf;
    rt_SIMD_CAMERA     *s_cam;
    rt_SIMD_CONTEXT    *s_ctx;

    /* memory pool in the heap
     * for temporary per-frame allocs */
    rt_pntr             mpool;
    rt_ui32             msize;

/*  methods */

    private:

    rt_void     tiling(rt_vec2 p1, rt_vec2 p2);

    rt_ELEM*    insert(rt_Object *obj, rt_ELEM **ptr, rt_ELEM *tem);

    public:

    rt_ELEM*    filter(rt_Object *obj, rt_ELEM **ptr);

    rt_pntr operator new(size_t size, rt_Heap *hp);
    rt_void operator delete(rt_pntr ptr);

    rt_SceneThread(rt_Scene *scene, rt_si32 index);

    virtual
   ~rt_SceneThread();

    rt_void     snode(rt_Surface *srf);
    rt_void     sclip(rt_Surface *srf);
    rt_void     stile(rt_Surface *srf);

    rt_ELEM*    ssort(rt_Object *obj);
    rt_ELEM*    lsort(rt_Object *obj);
};

/******************************************************************************/
/**********************************   SCENE   *********************************/
/******************************************************************************/

/*
 * Scene manager (or instance of the engine).
 */
class rt_Scene : private rt_Registry, public rt_List<rt_Scene>
{
/*  fields */

    private:

    /* platform pointer and its shared flag */
    rt_Platform        *pfm;
    rt_si32             shared;

    /* root scene object from scene data */
    rt_SCENE           *scn;
    /* dummy for root's identity transform */
    rt_OBJECT           rootobj;

    /* framebuffer's dimensions and pointer */
    rt_si32             x_res;
    rt_si32             y_res;
    rt_si32             x_row;
    rt_ui32            *frame;

    /* single tile dimensions in pixels */
    rt_si32             tile_w;
    rt_si32             tile_h;
    /* tilebuffer's dimensions and pointer */
    rt_si32             tiles_in_row;
    rt_si32             tiles_in_col;
    rt_ELEM           **tiles;

    /* aspect ratio and pixel width */
    rt_real             aspect;
    rt_real             factor;

    /* rays depth and anti-aliasing */
    rt_ui32             depth;
    rt_si32             fsaa;

    /* memory pool in the heap
     * for temporary per-frame allocs */
    rt_pntr             mpool;
    rt_ui32             msize;
    /* pending release flag */
    rt_si32             pending;

    /* threads management functions */
    rt_FUNC_UPDATE      f_update;
    rt_FUNC_RENDER      f_render;

    /* scene threads array and its
     * platform-specific handle */
    rt_si32             thnum;
    rt_SceneThread    **tharr;
    rt_pntr             tdata;

    /* width and quads parameters of the
     * currently active SIMD runtime target */
    rt_si32             simd_width;
    rt_si32             simd_quads;

    /* global hierarchical list */
    rt_ELEM            *hlist;
    /* global surface/node list */
    rt_ELEM            *slist;
    /* global light/shadow list */
    rt_ELEM            *llist;
    /* camera's surface/node list */
    rt_ELEM            *clist;

    /* rays positioning variables */
    rt_vec4             pos;
    rt_vec4             dir;
    /* rays steppers variables */
    rt_vec4             hor;
    rt_vec4             ver;
    /* screen's normal direction */
    rt_vec4             nrm;
    /* tiles positioning variables */
    rt_vec4             org;
    /* tiles steppers variables */
    rt_vec4             htl;
    rt_vec4             vtl;
    /* accumulated ambient color */
    rt_vec4             amb;

    /* root of the object hierarchy */
    rt_Array           *root;
    /* current camera */
    rt_Camera          *cam;
    rt_si32             cam_idx;

/*  methods */

    rt_void     init_scene(rt_SCENE *scn,
                           rt_si32 x_res, rt_si32 y_res, rt_si32 x_row,
                           rt_ui32 *frame);

    public:

    rt_Scene(rt_SCENE *scn, /* "frame" must be SIMD-aligned or NULL */
             rt_si32 x_res, rt_si32 y_res, rt_si32 x_row, rt_ui32 *frame,
             rt_Platform *pfm);

    rt_Scene(rt_SCENE *scn, /* "frame" must be SIMD-aligned or NULL */
             rt_si32 x_res, rt_si32 y_res, rt_si32 x_row, rt_ui32 *frame,
             rt_FUNC_ALLOC f_alloc, rt_FUNC_FREE f_free, rt_si32 thnum = 0,
             rt_FUNC_INIT f_init = RT_NULL, rt_FUNC_TERM f_term = RT_NULL,
             rt_FUNC_UPDATE f_update = RT_NULL,
             rt_FUNC_RENDER f_render = RT_NULL,
             rt_FUNC_PRINT_LOG f_print_log = RT_NULL,
             rt_FUNC_PRINT_ERR f_print_err = RT_NULL);

    virtual
   ~rt_Scene();

    rt_void     update(rt_time time, rt_si32 action);
    rt_void     render(rt_time time);

    rt_void     update_slice(rt_si32 index, rt_si32 phase);
    rt_void     render_slice(rt_si32 index, rt_si32 phase);

    rt_void     render_num(rt_si32 x, rt_si32 y,
                           rt_si32 d, rt_si32 z, rt_ui32 num);

    rt_si32     get_x_row();
    rt_si32     get_tile_w();
    rt_void     print_state();

    rt_si32     get_opts();
    rt_si32     set_opts(rt_si32 opts);
    rt_si32     set_fsaa(rt_si32 fsaa);
    rt_si32     set_simd(rt_si32 simd);

    rt_si32     get_cam_idx();
    rt_si32     next_cam();
    rt_ui32*    get_frame();
    rt_void     save_frame(rt_si32 index);

    rt_Platform*get_platform();

    friend      class rt_SceneThread;
};

/* internal SIMD format converters */
rt_si32 simd_init(rt_si32 q_simd, rt_si32 s_type, rt_si32 v_size);

#endif /* RT_ENGINE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
