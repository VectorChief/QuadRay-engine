/******************************************************************************/
/* Copyright (c) 2013-2023 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_SCN_TEST12_H
#define RT_SCN_TEST12_H

#include "format.h"

#include "all_mat.h"
#include "all_obj.h"

namespace scn_test12
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

rt_SPHERE sp_ball01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,    -RT_INF  },
/* max */   {  +RT_INF,    +RT_INF,    +RT_INF  },
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

rt_OBJECT ob_boundcube01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_ARRAY(&ob_cube01)
    },
};

rt_RELATION rl_boundcube01[] =
{
    {  -1,  RT_REL_BOUND_ARRAY,   0   },
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
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        0.2,        0.2    },
/* rot */   {    0.0,       20.0,        0.0    },
/* pos */   {    0.0,        0.0,        1.5    },
        },
        RT_OBJ_ARRAY_REL(&ob_boundcube01, &rl_boundcube01)
    },
};

rt_RELATION rl_column01[] =
{
    {  -1,  RT_REL_BOUND_ARRAY,  -1   },
    {  -1,  RT_REL_MINUS_OUTER,   2   },
    {   0,  RT_REL_MINUS_ACCUM,  -1   },
    {   2,  RT_REL_MINUS_OUTER,   0   },
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
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        1.5    },
        },
        RT_OBJ_SPHERE(&sp_ball01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,     -135.0    },
/* pos */   {   -4.5,       -4.5,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_column01, &rl_column01)
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
/* rot */   {   90.0,        0.0,        0.0    },
/* pos */   {    0.0,       -4.5,        2.7    },
        },
        RT_OBJ_PLANE(&pl_pane01)
    },
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

/******************************************************************************/
/*********************************   LIGHTS   *********************************/
/******************************************************************************/

rt_OBJECT ob_light01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_LIGHT(&lt_light01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_SPHERE(&sp_bulb01)
    },
};

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
        RT_OBJ_ARRAY(&ob_base01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   -2.0,        0.0,        4.5    },
        },
        RT_OBJ_ARRAY(&ob_light01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   +2.0,        0.0,        4.5    },
        },
        RT_OBJ_ARRAY(&ob_light01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        5.0    },
        },
        RT_OBJ_ARRAY(&ob_camera01)
    },
};

/******************************************************************************/
/**********************************   SCENE   *********************************/
/******************************************************************************/

rt_SCENE sc_root =
{
    RT_OBJ_ARRAY(&ob_tree),
    /* list of optimizations to be turned off *
     * refer to core/engine/format.h for defs */
    
    /* turning off GAMMA|FRESNEL opts in turn *
     * enables respective GAMMA|FRESNEL props */
};

} /* namespace scn_test12 */

#endif /* RT_SCN_TEST12_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
