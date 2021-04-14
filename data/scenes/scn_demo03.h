/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_SCN_DEMO03_H
#define RT_SCN_DEMO03_H

#include "format.h"

#include "all_mat.h"
#include "all_obj.h"

namespace scn_demo03
{

/******************************************************************************/
/**********************************   BASE   **********************************/
/******************************************************************************/

rt_PLANE pl_floor01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -7.0,       -7.0,      -RT_INF  },
/* max */   {   +7.0,       +7.0,      +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain03_white01,
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

/******************************************************************************/
/**********************************   FRAME   *********************************/
/******************************************************************************/

rt_CYLINDER cl_tube01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,      0.0    },
/* max */   {  +RT_INF,    +RT_INF,     +2.0    },
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

/* mat */   &mt_plain01_gray02,
        },
    },
/* rad */   0.3,
};

rt_CYLINDER cl_tube02 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,      0.0    },
/* max */   {  +RT_INF,    +RT_INF,     +5.0    },
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

/* mat */   &mt_plain01_gray02,
        },
    },
/* rad */   0.3,
};

rt_SPHERE sp_ball01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,      0.0    },
/* max */   {  +RT_INF,    +RT_INF,    +RT_INF  },
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

/* mat */   &mt_plain01_gray02,
        },
    },
/* rad */   0.3,
};

rt_OBJECT ob_cargo01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   -5.0,        0.0,        0.0    },
        },
        RT_OBJ_ARRAY(&ob_cube01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   -5.0,        0.0,        0.0    },
        },
        RT_OBJ_CYLINDER(&cl_tube01),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,       90.0,        0.0    },
/* pos */   {   -5.0,        0.0,        1.5    },
        },
        RT_OBJ_CYLINDER(&cl_tube02),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    0.3,        0.3,        0.3    },
/* rot */   {    0.0,      210.0,        0.0    },
/* pos */   {   -3.5,        0.0,        1.5    },
        },
        RT_OBJ_CYLINDER(&cl_tube02),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   -5.0,        0.0,        2.0    },
        },
        RT_OBJ_SPHERE(&sp_ball01),
    },
};

rt_RELATION rl_cargo01[] =
{
    {  -1,  RT_REL_BOUND_ARRAY,  -1   },
    {  -1,  RT_REL_UNTIE_INDEX,   2   },
};

rt_RELATION rl_frame[] =
{
    {  -1,  RT_REL_BOUND_ARRAY,  -1   },
};

rt_OBJECT ob_frame01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   +2.5,       -2.5,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_cargo01, &rl_cargo01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,       90.0    },
/* pos */   {   +2.5,       +2.5,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_cargo01, &rl_cargo01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,      180.0    },
/* pos */   {   -2.5,       +2.5,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_cargo01, &rl_cargo01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,      270.0    },
/* pos */   {   -2.5,       -2.5,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_cargo01, &rl_cargo01)
    },
};

rt_OBJECT ob_frame02[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,       90.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_cargo01, &rl_cargo01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,      -30.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_cargo01, &rl_cargo01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,     -150.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_cargo01, &rl_cargo01)
    },
};

rt_OBJECT ob_frame03[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   +2.5,       -2.5,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_cargo01, &rl_cargo01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,       90.0    },
/* pos */   {   +2.5,       +2.5,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_cargo01, &rl_cargo01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,      180.0    },
/* pos */   {   -2.5,       +2.5,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_cargo01, &rl_cargo01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,      270.0    },
/* pos */   {   -2.5,       -2.5,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_cargo01, &rl_cargo01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,       30.0,        0.0    },
/* pos */   {   -2.5,        0.0,        1.5    },
        },
        RT_OBJ_CYLINDER(&cl_tube02),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,       30.0,       90.0    },
/* pos */   {    0.0,       -2.5,        1.5    },
        },
        RT_OBJ_CYLINDER(&cl_tube02),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,       30.0,      180.0    },
/* pos */   {   +2.5,        0.0,        1.5    },
        },
        RT_OBJ_CYLINDER(&cl_tube02),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,       30.0,      270.0    },
/* pos */   {    0.0,       +2.5,        1.5    },
        },
        RT_OBJ_CYLINDER(&cl_tube02),
    },
};

rt_OBJECT ob_frame04[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.7,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,       90.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_cargo01, &rl_cargo01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.7,        1.0    },
/* rot */   {    0.0,        0.0,      -30.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_cargo01, &rl_cargo01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.7    },
/* rot */   {    0.0,        0.0,     -150.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_cargo01, &rl_cargo01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_CYLINDER(&cl_tube02),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,      180.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_SPHERE(&sp_ball01),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        5.0    },
        },
        RT_OBJ_SPHERE(&sp_ball01),
    },
};

rt_OBJECT ob_megaframe01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    0.5,        0.5,        0.5    },
/* rot */   {    0.0,        0.0,       45.0    },
/* pos */   {   -3.0,        0.0,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_frame01, &rl_frame),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    0.5,        0.5,        0.5    },
/* rot */   {    0.0,        0.0,       60.0    },
/* pos */   {   +3.0,        0.0,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_frame02, &rl_frame),
    },
};

rt_OBJECT ob_megaframe02[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    0.5,        0.5,        0.5    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   -3.0,        0.0,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_frame03, &rl_frame),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    0.5,        0.4,        0.5    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   +3.0,        0.0,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_frame04, &rl_frame),
    },
};

rt_OBJECT ob_hyperframe01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        1.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_megaframe01, &rl_frame),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        4.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_megaframe02, &rl_frame),
    },
};

/******************************************************************************/
/*********************************   CAMERA   *********************************/
/******************************************************************************/

rt_OBJECT ob_camera03[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  -95.0,        0.0,      -90.0    },
/* pos */   {   +4.0,        5.0,        0.0    },
        },
        RT_OBJ_CAMERA(&cm_camera01)
    },
};

rt_OBJECT ob_camera02[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  -95.0,        0.0,      +90.0    },
/* pos */   {   -4.0,        5.0,        0.0    },
        },
        RT_OBJ_CAMERA(&cm_camera01)
    },
};

rt_OBJECT ob_camera01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  -95.0,        0.0,        0.0    },
/* pos */   {    0.0,      -21.0,        0.0    },
        },
        RT_OBJ_CAMERA(&cm_camera01)
    },
};

rt_void an_camera01(rt_time time, rt_time last_time,
                    rt_TRANSFORM3D *trm, rt_pntr pobj)
{
    rt_real t = time / 1500.0f;

    trm->rot[RT_Z] = 15 * sin(t);
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
        RT_OBJ_PLANE(&pl_floor01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  +90.0,        0.0,        0.0    },
/* pos */   {    0.0,       +7.0,        5.0    },
        },
        RT_OBJ_PLANE(&pl_floor01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,      +90.0,        0.0    },
/* pos */   {   -7.0,        0.0,        5.0    },
        },
        RT_OBJ_PLANE(&pl_floor01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,      -90.0,        0.0    },
/* pos */   {   +7.0,        0.0,        5.0    },
        },
        RT_OBJ_PLANE(&pl_floor01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        7.0    },
        },
        RT_OBJ_ARRAY(&ob_light01),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_ARRAY_REL(&ob_hyperframe01, &rl_frame),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        5.0    },
        },
        RT_OBJ_ARRAY(&ob_camera03)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        5.0    },
        },
        RT_OBJ_ARRAY(&ob_camera02)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        5.0    },
        },
        RT_OBJ_ARRAY(&ob_camera01),
        &an_camera01
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

} /* namespace scn_demo03 */

#endif /* RT_SCN_DEMO03_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
