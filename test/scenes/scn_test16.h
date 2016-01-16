/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_SCN_TEST16_H
#define RT_SCN_TEST16_H

#include "format.h"

#include "all_mat.h"
#include "all_obj.h"

namespace scn_test16
{

/******************************************************************************/
/**********************************   BASE   **********************************/
/******************************************************************************/

rt_PLANE pl_floor01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -15.0,      -10.0,      -RT_INF  },
/* max */   {  +15.0,      +10.0,      +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    2.0,        2.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain03_tile01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray02,
        },
    },
};

rt_PLANE pl_ceiling01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -15.0,      -10.0,      -RT_INF  },
/* max */   {  +15.0,      +10.0,      +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_white01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray02,
        },
    },
};

rt_PLANE pl_wall01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -10.0,        0.0,      -RT_INF  },
/* max */   {  +10.0,       10.0,      +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_white01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray02,
        },
    },
};

rt_PLANE pl_wall02 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -15.0,        0.0,      -RT_INF  },
/* max */   {  +15.0,       10.0,      +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_white01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray02,
        },
    },
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
/* rot */   {  180.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,       10.0    },
        },
        RT_OBJ_PLANE(&pl_ceiling01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {   90.0,        0.0,        0.0    },
/* pos */   {    0.0,       10.0,        0.0    },
        },
        RT_OBJ_PLANE(&pl_wall02)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {   90.0,        0.0,      180.0    },
/* pos */   {    0.0,      -10.0,        0.0    },
        },
        RT_OBJ_PLANE(&pl_wall02)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {   90.0,        0.0,       90.0    },
/* pos */   {  -15.0,        0.0,        0.0    },
        },
        RT_OBJ_PLANE(&pl_wall01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {   90.0,        0.0,      -90.0    },
/* pos */   {   15.0,        0.0,        0.0    },
        },
        RT_OBJ_PLANE(&pl_wall01)
    },
};

/******************************************************************************/
/*********************************   OBJECTS   ********************************/
/******************************************************************************/

rt_OBJECT ob_aliencube02[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
            RT_OBJ_ARRAY(&ob_aliencube01base01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
            RT_OBJ_SPHERE(&sp_aliencube01ball02)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
            RT_OBJ_CONE(&cn_aliencube01cone01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    0.5,        1.0,        1.0    },
/* rot */   {    0.0,       90.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
            RT_OBJ_CONE(&cn_aliencube01cone01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {   90.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
            RT_OBJ_CONE(&cn_aliencube01cone01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
            RT_OBJ_SPHERE(&sp_aliencube01ball01)
    },
};

rt_OBJECT ob_frametable02[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.5,        1.0,        1.5    },
/* rot */   {   90.0,        0.0,      +90.0    },
/* pos */   {   -2.8,        0.0,        1.5    },
        },
            RT_OBJ_ARRAY_REL(&ob_frametable01leg01, &rl_bound)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.5,        1.0,        1.5    },
/* rot */   {   90.0,        0.0,      -90.0    },
/* pos */   {   +2.8,        0.0,        1.5    },
        },
            RT_OBJ_ARRAY_REL(&ob_frametable01leg01, &rl_bound)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        3.0    },
        },
            RT_OBJ_ARRAY_REL(&ob_frametable01top01, &rl_bound)
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
        RT_OBJ_SPHERE(&sp_bulb01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_LIGHT(&lt_light01)
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
/* rot */   {  -90.0,        0.0,       90.0    },
/* pos */   {   11.0,        0.0,        0.0    },
        },
        RT_OBJ_CAMERA(&cm_camera01)
    },
};

/******************************************************************************/
/**********************************   TREE   **********************************/
/******************************************************************************/

rt_PLANE pl_cube02side01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -1.0,       -1.0,      -RT_INF  },
/* max */   {   +1.0,       +1.0,      +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    2.0,        2.0    },
/* rot */              0.0           ,
/* pos */   {   -1.0,       -1.0    },

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
};

rt_OBJECT ob_err01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        1.1    },
        },
        RT_OBJ_PLANE(&pl_cube02side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,       17.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_cube01, &rl_bound)
    },
};

/*
 * As the rendering backend doesn't attempt to sort overlapping surfaces
 * to produce the correct result, the cubes are placed slightly above the floor
 * to avoid undefined behaviour resulting from an arbitrary surface order,
 * when optimizations are disabled. When the cubes are strictly on the floor,
 * the bounding box sorting optimization handles the case properly, while
 * unoptimized code doesn't resulting in the test failure.
 */

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
/* pos */   {  -10.5,       -5.5,        1.0001 },
        },
        RT_OBJ_ARRAY_REL(&ob_err01, &rl_bound)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        0.7,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {  -10.5,       -1.5,        1.0001 },
        },
        RT_OBJ_ARRAY_REL(&ob_aliencube02, &rl_aliencube01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    0.7,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,       90.0    },
/* pos */   {  -10.5,       +4.5,        0.0001 },
        },
        RT_OBJ_ARRAY_REL(&ob_frametable02, &rl_bound)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   -6.0,        3.0,        4.5    },
        },
        RT_OBJ_ARRAY(&ob_light01),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        5.0    },
        },
        RT_OBJ_ARRAY(&ob_camera01),
    },
};

/******************************************************************************/
/**********************************   SCENE   *********************************/
/******************************************************************************/

rt_SCENE sc_root =
{
    RT_OBJ_ARRAY(&ob_tree),
};

} /* namespace scn_test16 */

#endif /* RT_SCN_TEST16_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
