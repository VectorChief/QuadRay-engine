/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_ENGINE_H
#define RT_ENGINE_H

#include "rtarch.h"
#include "rtbase.h"
#include "object.h"
#include "tracer.h"

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

/* Full screen anti-aliasing modes */

#define RT_FSAA_NO                  0
#define RT_FSAA_4X                  1

/* Classes */

class rt_Scene;

/******************************************************************************/
/**********************************   SCENE   *********************************/
/******************************************************************************/

class rt_Scene : public rt_Registry
{
/*  fields */

    public:

    rt_SCENE           *scn;
    rt_OBJECT           rootobj;

    rt_word             x_res;
    rt_word             y_res;
    rt_cell             x_row;
    rt_word            *frame;

    rt_SIMD_INFOX      *s_inf;
    rt_SIMD_CAMERA     *s_cam;
    rt_SIMD_CONTEXT    *s_ctx;

    rt_ELEM            *slist;
    rt_ELEM            *llist;

    rt_real             aspect;
    rt_real             factor;

    rt_word             depth;
    rt_cell             fsaa;

    rt_vec3             pos;
    rt_vec3             dir;
    rt_vec3             hor;
    rt_vec3             ver;
    rt_vec3             nrm;
    rt_vec3             amb;

    rt_Object          *root;
    rt_Camera          *cam;

/*  methods */

    public:

    rt_Scene(rt_SCENE *scn, /* frame must be SIMD-aligned */
             rt_word x_res, rt_word y_res, rt_cell x_row, rt_word *frame,
             rt_FUNC_ALLOC f_alloc, rt_FUNC_FREE f_free);

   ~rt_Scene();

    rt_void     insert(rt_Object *obj, rt_ELEM **ptr, rt_Surface *srf);
    rt_ELEM*    ssort(rt_Object *obj);
    rt_ELEM*    lsort(rt_Object *obj);

    rt_void     update(rt_long time, rt_cell action);
    rt_void     render(rt_long time);
    rt_void     render_fps(rt_word x, rt_word y,
                           rt_cell d, rt_word z, rt_word num);

    rt_word*    get_frame();
    rt_void     set_fsaa(rt_cell fsaa);
};

#endif /* RT_ENGINE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
