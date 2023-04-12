/******************************************************************************/
/* Copyright (c) 2013-2023 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_ALL_OBJ_H
#define RT_ALL_OBJ_H

#include "format.h"

#include "all_mat.h"
#include "obj_aliencube.h"
#include "obj_frametable.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * all_obj.h: Common file for all objects.
 *
 * Recommended naming scheme for objects:
 *
 * - All object names start with **_ followed by object's specific name,
 *   where ** is a two-letter code corresponding to object's type
 *   defined in format.h:
 *
 *   pl for PLANE
 *   cl for CYLINDER
 *   sp for SPHERE
 *   cn for CONE
 *   pb for PARABOLOID
 *   hb for HYPERBOLOID
 *   pc for PARACYLINDER
 *   hc for HYPERCYLINDER
 *   hp for HYPERPARABOLOID
 *   cm for CAMERA
 *   lt for LIGHT
 *   ob for OBJECT array
 *   an for OBJECT's animator function
 *   sc for SCENE
 *
 * - If object is constructed from other objects it is recommended to have
 *   parent's name as prefix in all sub-objects names to avoid name collisions
 *   in global namespace (for example pl_cube01side01).
 *
 * - Complex objects can be placed in separate files obj_*.h, where * is
 *   objects's name (for example obj_cube01.h), object names in this
 *   case follow the same rules as described above (for example ob_cube01)
 */

/******************************************************************************/
/**********************************   BOXES   *********************************/
/******************************************************************************/

rt_PLANE pl_cube01side01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -1.0,       -1.0,      -RT_INF  },
/* max */   {   +1.0,       +1.0,      +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    2.0,        2.0    },
/* rot */              0.0           ,
/* pos */   {   -1.0,       -1.0    },

/* mat */   &mt_plain01_crate01,
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

rt_OBJECT ob_cube01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  180.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,       -1.0    },
        },
        RT_OBJ_PLANE(&pl_cube01side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,       +1.0    },
        },
        RT_OBJ_PLANE(&pl_cube01side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  +90.0,        0.0,      -90.0    },
/* pos */   {   -1.0,        0.0,        0.0    },
        },
        RT_OBJ_PLANE(&pl_cube01side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  -90.0,        0.0,      -90.0    },
/* pos */   {   +1.0,        0.0,        0.0    },
        },
        RT_OBJ_PLANE(&pl_cube01side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  +90.0,        0.0,        0.0    },
/* pos */   {    0.0,       -1.0,        0.0    },
        },
        RT_OBJ_PLANE(&pl_cube01side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  -90.0,        0.0,        0.0    },
/* pos */   {    0.0,       +1.0,        0.0    },
        },
        RT_OBJ_PLANE(&pl_cube01side01)
    },
};

/******************************************************************************/
/**********************************   BALLS   *********************************/
/******************************************************************************/

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

/* mat */   &mt_plain01_white01,
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

/******************************************************************************/
/*********************************   CAMERA   *********************************/
/******************************************************************************/

rt_CAMERA cm_camera01 =
{
    RT_CAM(PLAIN),

    RT_COL(0xFFFFFFFF),

    {/* amb */
        0.05
    },
    {/* pov */
        1.0
    },
    {/* dpi     dpj     dpk */
        0.5,    0.5,    0.5
    },
    {/* dri     drj     drk */
        1.5,    1.5,    1.5
    },
};

/******************************************************************************/
/**********************************   LIGHT   *********************************/
/******************************************************************************/

rt_LIGHT lt_light01 =
{
    RT_LGT(PLAIN),

    RT_COL(0xFFFFFFFF),

    {/* amb     src */
        0.01,   1.7
    },
    {/* rng     cnt     lnr     qdr */
        0.0,    0.7,    0.5,    0.1
    },
};

rt_SPHERE sp_bulb01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,    -RT_INF  },
/* max */   {  +RT_INF,    +RT_INF,    +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_light01_bulb01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_light01_bulb01,
        },
    },
/* rad */   0.05,
};

#endif /* RT_ALL_OBJ_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
