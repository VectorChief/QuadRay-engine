/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_THREAD_H
#define RT_THREAD_H

#include "rtbase.h"
#include "engine.h"
#include "object.h"
#include "system.h"
#include "tracer.h"

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

/* Classes */

class rt_SceneThread;
class rt_Scene;

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
    rt_cell             index;

    /* surface's projected bbox
     * x-coord boundaries in the tilebuffer */
    rt_cell            *txmin;
    rt_cell            *txmax;
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
    rt_word             msize;

/*  methods */

    public:

    rt_SceneThread(rt_Scene *scene, rt_cell index);

    virtual
   ~rt_SceneThread();

    rt_void     tiling(rt_vec4 p1, rt_vec4 p2);
    rt_void     insert(rt_Object *obj, rt_ELEM **ptr, rt_Surface *srf);

    rt_void     stile(rt_Surface *srf);
    rt_ELEM*    ssort(rt_Object *obj);
    rt_ELEM*    lsort(rt_Object *obj);
};

#endif /* RT_THREAD_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
