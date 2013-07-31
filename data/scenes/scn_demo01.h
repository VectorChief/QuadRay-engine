/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_SCN_DEMO01_H
#define RT_SCN_DEMO01_H

#include "format.h"

#include "all_mat.h"
#include "all_obj.h"

/******************************************************************************/
/**********************************   BASE   **********************************/
/******************************************************************************/

rt_PLANE pl_floor01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -5.0,       -5.0,      -RT_INF  },
/* max */   {   +5.0,       +5.0,      +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    2.0,        2.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_tile01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_gray01,
        },
    },
};

rt_CYLINDER  cl_tube01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,      0.0    },
/* max */   {  +RT_INF,    +RT_INF,     +4.5    },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_pink01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_gray01,
        },
    },
/* rad */   0.5,
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
/* pos */   {   -4.5,       -4.5,        0.0    },
        },
            RT_OBJ_CYLINDER(&cl_tube01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   +4.5,       -4.5,        0.0    },
        },
            RT_OBJ_CYLINDER(&cl_tube01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   +4.5,       +4.5,        0.0    },
        },
            RT_OBJ_CYLINDER(&cl_tube01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   -4.5,       +4.5,        0.0    },
        },
            RT_OBJ_CYLINDER(&cl_tube01)
    },
};

/******************************************************************************/
/*******************************   FIGURES   **********************************/
/******************************************************************************/

rt_SPHERE  sp_ball01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,    -RT_INF  },
/* max */   {  +RT_INF,    +RT_INF,    +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_cyan01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_gray01,
        },
    },
/* rad */   1.5,
};

rt_PARABOLOID  pb_para01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,    -RT_INF  },
/* max */   {  +RT_INF,    +RT_INF,    +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_pink01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_gray01,
        },
    },
/* par */   1.0,
};

rt_SPHERE  sp_ball02 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,    -RT_INF  },
/* max */   {  +RT_INF,    +RT_INF,    +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_blue02,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_gray01,
        },
    },
/* rad */   0.5,
};

rt_OBJECT  ob_figures01[] =
{
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
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        2.0    },
        },
            RT_OBJ_PARABOLOID(&pb_para01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,       90.0,        0.0    },
/* pos */   {   -1.5,       -1.5,        1.5    },
        },
            RT_OBJ_CYLINDER(&cl_tube01)
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
};

rt_RELATION  rl_figures01[] =
{
    {   0,  RT_REL_MINUS_INNER,   1   },
    {   1,  RT_REL_MINUS_OUTER,   0   },
    {   0,  RT_REL_MINUS_INNER,   2   },
    {   2,  RT_REL_MINUS_OUTER,   0   },
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

rt_void an_camera01(rt_long time, rt_long last_time,
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

rt_void an_light01(rt_long time, rt_long last_time,
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

/******************************************************************************/
/**********************************   SCENE   *********************************/
/******************************************************************************/

rt_SCENE sc_root =
{
    RT_OBJ_ARRAY(&ob_tree),
};

#endif /* RT_SCN_DEMO01_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
