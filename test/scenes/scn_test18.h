/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_SCN_TEST18_H
#define RT_SCN_TEST18_H

#include "format.h"

#include "all_mat.h"
#include "all_obj.h"

namespace scn_test18
{

rt_MATERIAL mt_plain01_grayPT =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFFE1E1E1),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

rt_MATERIAL mt_plain01_pinkPT =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFFE18787),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

rt_MATERIAL mt_plain01_bluePT =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFF8787E1),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

/******************************************************************************/
/**********************************   BASE   **********************************/
/******************************************************************************/

rt_PLANE pl_wall01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -49.0,      -40.8,      -RT_INF  },
/* max */   {  +49.0,      +40.8,      +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_grayPT,
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
/* min */   {  -49.0,      -85.0,      -RT_INF  },
/* max */   {  +49.0,      +85.0,      +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_grayPT,
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

rt_PLANE pl_wall_L =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -85.0,      -40.8,      -RT_INF  },
/* max */   {  +85.0,      +40.8,      +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_pinkPT,
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

rt_PLANE pl_wall_R =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -85.0,      -40.8,      -RT_INF  },
/* max */   {  +85.0,      +40.8,      +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_bluePT,
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
/**********************************   BALLS   *********************************/
/******************************************************************************/

rt_SPHERE sp_mirror_ball01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,    -RT_INF  },
/* max */   {  +RT_INF,    +RT_INF,    +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_metal03_nickel01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_metal03_nickel01,
        },
    },
/* rad */   16.5,
};

rt_SPHERE sp_glass_ball01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,    -RT_INF  },
/* max */   {  +RT_INF,    +RT_INF,    +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_air_to_glass03,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_glass03_to_air,
        },
    },
/* rad */   16.5,
};

/******************************************************************************/
/*********************************   LIGHTS   *********************************/
/******************************************************************************/

rt_LIGHT lt_light02 =
{
    RT_LGT(PLAIN),

    RT_COL(0xFFFFFFFF),

    {/* amb     src */
        0.0,    0.12
    },
    {/* rng     cnt     lnr     qdr */
        0.0,    0.7,    0.5,    0.1
    },
};

rt_SPHERE sp_bulb02 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,    -RT_INF  },
/* max */   {  +RT_INF,   -600.0+.27,  +RT_INF  },
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
/* rad */   600.0,
};

rt_OBJECT ob_light01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_SPHERE(&sp_bulb02)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,     -600.0+.14,    0.0    },
        },
        RT_OBJ_LIGHT(&lt_light02)
    },
};

/******************************************************************************/
/*********************************   CAMERA   *********************************/
/******************************************************************************/

rt_CAMERA cm_camera02 =
{
    RT_CAM(PLAIN),

    RT_COL(0xFFFFFFFF),

    {/* amb */
        0.05
    },
    {/* pov */
        1.4605
    },
    {/* dpi     dpj     dpk */
        0.5,    0.5,    0.5
    },
    {/* dri     drj     drk */
        1.5,    1.5,    1.5
    },
};

rt_OBJECT ob_camera01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   { -182.44,       0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_CAMERA(&cm_camera02)
    },
};

/******************************************************************************/
/**********************************   TREE   **********************************/
/******************************************************************************/

/*
 * The scene data was taken from smallpt project (spheres -> planes).
 * Camera is facing downward as the scene is "under the floor".
 * Needs to be redesigned for proper scale and navigation.
 *
 * Include this scene in RooT demo and run it with:
 * ./RooT.x64f64 -x 1024 -y 768 -q -i -k 1 -h -a -f 500
 * Consult with comments in RT_FEAT_PT_RANDOM_SAMPLE section
 * in core/tracer/tracer.cpp to better match smallpt results.
 * Path-tracing mode (-q) is still work in progress.
 */

rt_OBJECT ob_tree[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   50.0,       40.8,        0.0    },
        },
        RT_OBJ_PLANE(&pl_wall01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  -90.0,        0.0,        0.0    },
/* pos */   {   50.0,        0.0,       85.0    },
        },
        RT_OBJ_PLANE(&pl_wall02)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  +90.0,        0.0,        0.0    },
/* pos */   {   50.0,       81.6,       85.0    },
        },
        RT_OBJ_PLANE(&pl_wall02)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,      +90.0,        0.0    },
/* pos */   {    1.0,       40.8,       85.0    },
        },
        RT_OBJ_PLANE(&pl_wall_L)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,      -90.0,        0.0    },
/* pos */   {   99.0,       40.8,       85.0    },
        },
        RT_OBJ_PLANE(&pl_wall_R)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   50.0,      681.6-.27,   81.6    },
        },
        RT_OBJ_ARRAY(&ob_light01),
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   27.0,       16.5,       47.0    },
        },
        RT_OBJ_SPHERE(&sp_mirror_ball01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   73.0,       16.5,       78.0    },
        },
        RT_OBJ_SPHERE(&sp_glass_ball01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   50.0,       52.0,      295.6    },
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
    RT_OPTS_PT | RT_OPTS_GAMMA | RT_OPTS_FRESNEL
    /* turning off GAMMA|FRESNEL opts in turn *
     * enables respective GAMMA|FRESNEL props */
};

} /* namespace scn_test18 */

#endif /* RT_SCN_TEST18_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
