/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_SCN_TEST13_H
#define RT_SCN_TEST13_H

#include "format.h"

#include "all_mat.h"
#include "all_obj.h"

namespace scn_test13
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
/**********************************   CUBES   *********************************/
/******************************************************************************/

rt_PLANE pl_cube02side01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -1.0,       -1.0,      -RT_INF  },
/* max */   {   +1.0,       +1.0,      +RT_INF  },
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

/* mat */   &mt_plain01_gray02,
        },
    },
};

rt_OBJECT ob_cube02[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  180.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,       -1.0    },
        },
        RT_OBJ_PLANE(&pl_cube02side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,       +1.0    },
        },
        RT_OBJ_PLANE(&pl_cube02side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  +90.0,        0.0,      -90.0    },
/* pos */   {   -1.0,        0.0,        0.0    },
        },
        RT_OBJ_PLANE(&pl_cube02side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  -90.0,        0.0,      -90.0    },
/* pos */   {   +1.0,        0.0,        0.0    },
        },
        RT_OBJ_PLANE(&pl_cube02side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  +90.0,        0.0,        0.0    },
/* pos */   {    0.0,       -1.0,        0.0    },
        },
        RT_OBJ_PLANE(&pl_cube02side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  -90.0,        0.0,        0.0    },
/* pos */   {    0.0,       +1.0,        0.0    },
        },
        RT_OBJ_PLANE(&pl_cube02side01)
    },
};

rt_PLANE pl_cube03side01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -1.0,       -1.0,      -RT_INF  },
/* max */   {   +1.0,       +1.0,      +RT_INF  },
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
};

rt_OBJECT ob_cube03[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  180.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,       -1.0    },
        },
        RT_OBJ_PLANE(&pl_cube03side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,       +1.0    },
        },
        RT_OBJ_PLANE(&pl_cube03side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  +90.0,        0.0,      -90.0    },
/* pos */   {   -1.0,        0.0,        0.0    },
        },
        RT_OBJ_PLANE(&pl_cube03side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  -90.0,        0.0,      -90.0    },
/* pos */   {   +1.0,        0.0,        0.0    },
        },
        RT_OBJ_PLANE(&pl_cube03side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  +90.0,        0.0,        0.0    },
/* pos */   {    0.0,       -1.0,        0.0    },
        },
        RT_OBJ_PLANE(&pl_cube03side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  -90.0,        0.0,        0.0    },
/* pos */   {    0.0,       +1.0,        0.0    },
        },
        RT_OBJ_PLANE(&pl_cube03side01)
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
/* rot */   {  -95.0,        0.0,       90.0    },
/* pos */   {   14.0,        0.0,        0.0    },
        },
        RT_OBJ_CAMERA(&cm_camera01)
    },
};

/******************************************************************************/
/**********************************   TREE   **********************************/
/******************************************************************************/

/*
 * As the rendering backend doesn't attempt to sort overlapping surfaces
 * the cubes are placed slightly above the floor to avoid undefined behaviour.
 *
 * Bbox sorting optimization (RT_OPTS_INSERT) resolves this situation properly,
 * but doesn't scale very well for larger scenes with lots of objects.
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
/* rot */   {    0.0,        0.0,       17.0    },
/* pos */   {  -10.7,       -4.7,        1.0001 },
        },
        RT_OBJ_ARRAY_REL(&ob_cube01, &rl_bound)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,      -43.0    },
/* pos */   {  -10.7,        0.5,        1.0001 },
        },
        RT_OBJ_ARRAY_REL(&ob_cube02, &rl_bound)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,       -7.0    },
/* pos */   {  -10.7,        5.5,        1.0001 },
        },
        RT_OBJ_ARRAY_REL(&ob_cube03, &rl_bound)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,       19.0    },
/* pos */   {   -5.7,        0.5,        1.0001 },
        },
        RT_OBJ_ARRAY_REL(&ob_aliencube01, &rl_aliencube01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.1    },
/* pos */   {   -3.7,        5.5,        0.0001 },
        },
        RT_OBJ_ARRAY_REL(&ob_frametable01, &rl_bound)
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
    /* list of optimizations to be turned off *
     * refer to core/engine/format.h for defs */
    RT_OPTS_PT
    /* turning off GAMMA|FRESNEL opts in turn *
     * enables respective GAMMA|FRESNEL props */
};

} /* namespace scn_test13 */

#endif /* RT_SCN_TEST13_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
