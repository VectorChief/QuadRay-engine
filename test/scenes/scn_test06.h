/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_SCN_TEST06_H
#define RT_SCN_TEST06_H

#include "format.h"

#include "all_mat.h"
#include "all_obj.h"

namespace scn_test06
{

/******************************************************************************/
/**********************************   BASE   **********************************/
/******************************************************************************/

rt_SPHERE sp_frame01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,     -0.0,      -RT_INF  },
/* max */   {  +RT_INF,    +RT_INF,     +0.0    },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray02,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_metal01_pink01,
        },
    },
/* rad */   3.5,
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

/* mat */   &mt_plain01_red01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_gray02,
        },
    },
/* rad */   0.5,
};

/******************************************************************************/
/*********************************   CAMERA   *********************************/
/******************************************************************************/

rt_OBJECT ob_camera01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   { -160.0,        0.0,       90.0    },
/* pos */   {    2.1,        3.2,        0.0    },
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
/* pos */   {    0.0,        0.0,        3.5    },
        },
        RT_OBJ_SPHERE(&sp_frame01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    3.5,        0.5,        0.5    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,       +2.8,        1.1    },
        },
        RT_OBJ_SPHERE(&sp_ball01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        3.3    },
        },
        RT_OBJ_ARRAY(&ob_light01),
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
    RT_OPTS_PT
    /* turning off GAMMA|FRESNEL opts in turn *
     * enables respective GAMMA|FRESNEL props */
};

} /* namespace scn_test06 */

#endif /* RT_SCN_TEST06_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
