/******************************************************************************/
/* Copyright (c) 2013-2018 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_SCN_DEMO02_H
#define RT_SCN_DEMO02_H

#include "format.h"

#include "all_mat.h"
#include "all_obj.h"

namespace scn_demo02
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

rt_LIGHT lt_light02 =
{
    RT_LGT(PLAIN),

    RT_COL(0xFFFFFFFF),

    {/* amb     src */
        0.01,   0.7
    },
    {/* rng     cnt     lnr     qdr */
        0.0,    0.7,    0.5,    0.1
    },
};

rt_OBJECT ob_light01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        3.0,        0.0    },
        },
        RT_OBJ_SPHERE(&sp_bulb01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        3.0,        0.0    },
        },
        RT_OBJ_LIGHT(&lt_light02)
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

rt_OBJECT ob_trinity01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_ARRAY(&ob_light01),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,      120.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_ARRAY(&ob_light01),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,      240.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_ARRAY(&ob_light01),
    },
};

/******************************************************************************/
/*********************************   CAMERA   *********************************/
/******************************************************************************/

rt_OBJECT ob_camera02[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  -95.0,        0.0,      -90.0    },
/* pos */   {  -14.0,        0.0,        0.0    },
        },
        RT_OBJ_CAMERA(&cm_camera01)
    },
};

rt_OBJECT ob_camera01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  -95.0,        0.0,      +90.0    },
/* pos */   {  +14.0,        0.0,        0.0    },
        },
        RT_OBJ_CAMERA(&cm_camera01)
    },
};

/******************************************************************************/
/**********************************   TREE   **********************************/
/******************************************************************************/

/*
 * As the rendering backend doesn't attempt to sort overlapping surfaces
 * to produce the correct result, the cubes are placed slightly above the floor
 * to avoid undefined behaviour resulting from an arbitrary surface order,
 * when optimizations are turned off. When the cubes are strictly on the floor,
 * the bounding box sorting optimization handles the case properly,
 * while unoptimized code doesn't, resulting in potential visual artifacts.
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
/* rot */   {    0.0,        0.0,      -19.0    },
/* pos */   {   -5.7,       -6.5,        1.0001 },
        },
        RT_OBJ_ARRAY_REL(&ob_aliencube01, &rl_aliencube01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   -3.7,        5.5,        0.0001 },
        },
        RT_OBJ_ARRAY_REL(&ob_frametable01, &rl_bound)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   -6.0,        0.0,        4.5    },
        },
        RT_OBJ_ARRAY(&ob_trinity01),
        &an_light01,
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        5.0    },
        },
        RT_OBJ_ARRAY(&ob_camera02),
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

} /* namespace scn_demo02 */

#endif /* RT_SCN_DEMO02_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
