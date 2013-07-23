/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_ENGINE_H
#define RT_ENGINE_H

#include "rtarch.h"
#include "rtbase.h"
#include "rtconf.h"
#include "system.h"
#include "tracer.h"

/* Classes */

class rt_Scene;

/******************************************************************************/
/*********************************   SCENE   **********************************/
/******************************************************************************/

class rt_Scene : public rt_Heap
{
/*  fields */

    public:

    rt_word             x_res;
    rt_word             y_res;
    rt_cell             x_row;
    rt_word            *frame;

    rt_SIMD_INFOX      *s_inf;

/*  methods */

    public:

    rt_Scene(rt_word x_res, rt_word y_res, rt_cell x_row, rt_word *frame,
             rt_FUNC_ALLOC f_alloc, rt_FUNC_FREE f_free);

   ~rt_Scene();

    rt_void     render(rt_long time);
    rt_void     render_number(rt_word x, rt_word y,
                              rt_cell d, rt_word z, rt_word num);
};

#endif /* RT_ENGINE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
