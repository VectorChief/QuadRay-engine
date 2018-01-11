/******************************************************************************/
/* Copyright (c) 2013-2018 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_SCN_DEMO01_H
#define RT_SCN_DEMO01_H

#include "format.h"

#include "all_mat.h"
#include "all_obj.h"

namespace scn_demo01
{

/******************************************************************************/
/**********************************   BASE   **********************************/
/******************************************************************************/

rt_PLANE pl_floor01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -10.0,      -10.0,      -RT_INF  },
/* max */   {  +10.0,      +10.0,      +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    2.0,        2.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_tile01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray01,
        },
    },
};

rt_PLANE pl_pane01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -4.0,       -0.3,      -RT_INF  },
/* max */   {   +4.0,       +0.3,      +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_glass01_orange01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_glass01_orange01,
        },
    },
};

rt_CYLINDER cl_tube01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,      0.0    },
/* max */   {  +RT_INF,    +RT_INF,     +3.0    },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_metal02_pink01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray01,
        },
    },
/* rad */   0.5,
};

rt_SPHERE sp_ball03 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,      0.0    },
/* max */   {  +RT_INF,    +RT_INF,    +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_metal02_pink01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray01,
        },
    },
/* rad */   0.5,
};

rt_OBJECT ob_column01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_CYLINDER(&cl_tube01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        3.0    },
        },
        RT_OBJ_SPHERE(&sp_ball03)
    },
};

rt_OBJECT ob_column02[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_CYLINDER(&cl_tube01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        3.0    },
        },
        RT_OBJ_SPHERE(&sp_ball03)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        0.2,        0.2    },
/* rot */   {    0.0,       20.0,        0.0    },
/* pos */   {    0.0,        0.0,        1.5    },
        },
        RT_OBJ_ARRAY(&ob_cube01)
    },
};

rt_RELATION rl_column02[] =
{
    {  -1,  RT_REL_BOUND_ARRAY,  -1   },
    {  -1,  RT_REL_MINUS_OUTER,   2   },
    {   0,  RT_REL_MINUS_ACCUM,  -1   },
    {   2,  RT_REL_MINUS_OUTER,   0   },
    {  -1,  RT_REL_BOUND_ARRAY,   2   },
};

rt_OBJECT ob_base01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_PLANE(&pl_floor01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,     -135.0    },
/* pos */   {   -4.5,       -4.5,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_column02, &rl_column02)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,      -45.0    },
/* pos */   {   +4.5,       -4.5,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_column02, &rl_column02)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   +4.5,       +4.5,        0.0    },
        },
        RT_OBJ_ARRAY(&ob_column01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   -4.5,       +4.5,        0.0    },
        },
        RT_OBJ_ARRAY(&ob_column01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   { -125.0,        0.0,        0.0    },
/* pos */   {    0.0,       +4.5,        0.7    },
        },
        RT_OBJ_PLANE(&pl_pane01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   { -125.0,        0.0,        0.0    },
/* pos */   {    0.0,       +4.5,        1.7    },
        },
        RT_OBJ_PLANE(&pl_pane01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   { -125.0,        0.0,        0.0    },
/* pos */   {    0.0,       +4.5,        2.7    },
        },
        RT_OBJ_PLANE(&pl_pane01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {   90.0,        0.0,        0.0    },
/* pos */   {    0.0,       -4.5,        2.7    },
        },
        RT_OBJ_PLANE(&pl_pane01)
    },
};

/******************************************************************************/
/*******************************   FIGURES   **********************************/
/******************************************************************************/

rt_SPHERE sp_ball01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,    -RT_INF  },
/* max */   {  +RT_INF,    +RT_INF,     +1.3    },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_metal01_cyan01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray01,
        },
    },
/* rad */   1.5,
};

rt_PARABOLOID pb_para01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,    -RT_INF  },
/* max */   {  +RT_INF,    +RT_INF,     +5.0    },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray01,
        },
    },
/* par */   0.3,
};

rt_CYLINDER cl_tube02 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,      0.0     },
/* max */   {  +RT_INF,    +RT_INF,     +1.5     },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray01,
        },
    },
/* rad */   3.0,
};

rt_SPHERE sp_ball02 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,    -RT_INF  },
/* max */   {  +RT_INF,    +RT_INF,    +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_air_to_glass01_blue02,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_glass01_to_air_blue02,
        },
    },
/* rad */   0.5,
};

rt_CONE cn_cone01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,     -1.0    },
/* max */   {  +RT_INF,    +RT_INF,     +0.0    },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain02_red01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray01,
        },
    },
/* rat */   1.0,
};

rt_OBJECT ob_ball01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_SPHERE(&sp_ball01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   +1.0,        0.0,        1.0    },
        },
        RT_OBJ_SPHERE_MAT(&sp_ball02, &mt_plain01_gray01, &mt_plain01_gray01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,       90.0,        0.0    },
/* pos */   {    0.0,        0.0,        4.0    },
        },
        RT_OBJ_CYLINDER(&cl_tube02)
    },
};

rt_RELATION rl_ball01[] =
{
    {  -1,  RT_REL_MINUS_OUTER,   1   },
    {  -1,  RT_REL_MINUS_INNER,   2   },
    {   0,  RT_REL_MINUS_ACCUM,  -1   },
    {   1,  RT_REL_MINUS_OUTER,   0   },
    {   2,  RT_REL_MINUS_OUTER,   0   },
    {   1,  RT_REL_MINUS_INNER,   2   },
    {   2,  RT_REL_MINUS_OUTER,   1   },
};

rt_OBJECT ob_figures01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    0.5,        1.0,        1.0    },
/* rot */   {    0.0,      -60.0,        0.0    },
/* pos */   {   -0.5,        0.0,        2.0    },
        },
        RT_OBJ_PARABOLOID(&pb_para01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        1.5    },
        },
        RT_OBJ_ARRAY_REL(&ob_ball01, &rl_ball01),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   -2.0,       -2.0,        0.5    },
        },
        RT_OBJ_SPHERE(&sp_ball02)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   +2.0,       -2.0,        0.5    },
        },
        RT_OBJ_SPHERE(&sp_ball02)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   +2.0,       +2.0,        0.5    },
        },
        RT_OBJ_SPHERE(&sp_ball02)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   -2.0,       +2.0,        0.5    },
        },
        RT_OBJ_SPHERE(&sp_ball02)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   -3.0,        0.0,        1.0    },
        },
        RT_OBJ_CONE(&cn_cone01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   +3.0,        0.0,        1.0    },
        },
        RT_OBJ_CONE(&cn_cone01)
    },
};

rt_RELATION rl_figures01[] =
{
    {   1,  RT_REL_INDEX_ARRAY,  -1   },
    {   0,  RT_REL_MINUS_INNER,   0   },
    {  -1,  RT_REL_INDEX_ARRAY,   1   },
    {   0,  RT_REL_MINUS_OUTER,   0   },
};

/******************************************************************************/
/*********************************   CAMERA   *********************************/
/******************************************************************************/

rt_OBJECT ob_camera01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  -95.0,        0.0,        0.0    },
/* pos */   {    0.0,      -20.0,        0.0    },
        },
        RT_OBJ_CAMERA(&cm_camera01)
    },
};

rt_void an_camera01(rt_time time, rt_time last_time,
                    rt_TRANSFORM3D *trm, rt_pntr pobj)
{
    rt_real t = (time - last_time) / 50.0f;

    trm->rot[RT_Z] += t;

    if (trm->rot[RT_Z] >= 360.0f)
    {
        trm->rot[RT_Z] -= 360.0f;
    }
}

/******************************************************************************/
/*********************************   LIGHTS   *********************************/
/******************************************************************************/

rt_OBJECT ob_light01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    2.0,        0.0,        0.0    },
        },
        RT_OBJ_LIGHT(&lt_light01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    2.0,        0.0,        0.0    },
        },
        RT_OBJ_SPHERE(&sp_bulb01)
    },
};

rt_void an_light01(rt_time time, rt_time last_time,
                   rt_TRANSFORM3D *trm, rt_pntr pobj)
{
    rt_real t = (time - last_time) / 50.0f;

    trm->rot[RT_Z] += t * 7;

    if (trm->rot[RT_Z] >= 360.0f)
    {
        trm->rot[RT_Z] -= 360.0f;
    }
}

/******************************************************************************/
/**********************************   TREE   **********************************/
/******************************************************************************/

rt_OBJECT ob_tree[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_ARRAY(&ob_base01),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_figures01, &rl_figures01),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        7.5,        1.5    },
        },
        RT_OBJ_SPHERE(&::sp_ball01),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,       15.0    },
/* pos */   {    0.0,       -7.5,        1.0    },
        },
        RT_OBJ_ARRAY(&ob_cube01),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        5.0    },
        },
        RT_OBJ_ARRAY(&ob_camera01),
        &an_camera01,
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        4.5    },
        },
        RT_OBJ_ARRAY(&ob_light01),
        &an_light01,
    },
};

rt_RELATION rl_tree[] =
{
    {  -1,  RT_REL_BOUND_ARRAY,   3   },
};

/******************************************************************************/
/**********************************   SCENE   *********************************/
/******************************************************************************/

rt_SCENE sc_root =
{
    RT_OBJ_ARRAY_REL(&ob_tree, &rl_tree),
};

} /* namespace scn_demo01 */

#endif /* RT_SCN_DEMO01_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
