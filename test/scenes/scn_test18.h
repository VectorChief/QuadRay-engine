/******************************************************************************/
/* Copyright (c) 2013-2019 VectorChief (at github, bitbucket, sourceforge)    */
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

rt_MATERIAL mt_plain01_black01 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFF000000),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    0.0,    1.0
    },
};

rt_MATERIAL mt_glass02_black01 =
{
    RT_MAT(PLAIN),

    RT_TEX(PCOLOR, 0xFF000000),

    {/* dff     spc     pow */
        1.0,    0.0,    1.0
    },
    {/* rfl     trn     rfr */
        0.0,    1.0,    1.0
    },
};

/******************************************************************************/
/**********************************   BASE   **********************************/
/******************************************************************************/

rt_SPHERE sp_back01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,    -RT_INF  },
/* max */   {  +RT_INF,    +RT_INF,    +RT_INF  },
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
/* rad */   1e5,
};

rt_SPHERE sp_front01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,    -RT_INF  },
/* max */   {  +RT_INF,    +RT_INF,    +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_glass02_black01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_black01,
        },
    },
/* rad */   1e5,
};

rt_SPHERE sp_left01 =
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

/* mat */   &mt_plain01_red01,
        },
    },
/* rad */   1e5,
};

rt_SPHERE sp_right01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,    -RT_INF  },
/* max */   {  +RT_INF,    +RT_INF,    +RT_INF  },
        {
/* OUTER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_blue01,
        },
        {
/* INNER        RT_U,       RT_V    */
/* scl */   {    1.0,        1.0    },
/* rot */              0.0           ,
/* pos */   {    0.0,        0.0    },

/* mat */   &mt_plain01_blue01,
        },
    },
/* rad */   1e5,
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
        0.1,   10.7
    },
    {/* rng     cnt     lnr     qdr */
        0.0,    0.7,    0.5,    0.1
    },
};

rt_SPHERE sp_bulb02 =
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
/* pos */   {    0.0, -600.0+.14,        0.0    },
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
/* rot */   { -180.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
        RT_OBJ_CAMERA(&cm_camera02)
    },
};

/******************************************************************************/
/**********************************   TREE   **********************************/
/******************************************************************************/

/*
 * The scene data was taken almost "as is" from smallpt project.
 * Camera is facing downward as the scene is "under the floor".
 * Need to be redesigned for proper scale and navigation.
 *
 * Light's form and position as well as camera's FOV and wall colors
 * differ slightly from smallpt, still work in progress.
 */

rt_OBJECT ob_tree[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   50.0,       40.8,        1e5    },
        },
        RT_OBJ_SPHERE(&sp_back01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   50.0,        1e5,       81.6    },
        },
        RT_OBJ_SPHERE(&sp_back01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   50.0,  -1e5+81.6,       81.6    },
        },
        RT_OBJ_SPHERE(&sp_back01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   50.0,       40.8,   -1e5+170    },
        },
        RT_OBJ_SPHERE(&sp_front01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {+1e5+ 1,       40.8,       81.6    },
        },
        RT_OBJ_SPHERE(&sp_left01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {-1e5+99,       40.8,       81.6    },
        },
        RT_OBJ_SPHERE(&sp_right01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   50.0,  681.6-.27,       81.6    },
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

rt_RELATION rl_tree[] =
{
    {   0,  RT_REL_MINUS_OUTER,   1   },
    {   0,  RT_REL_MINUS_OUTER,   2   },
    {   0,  RT_REL_MINUS_OUTER,   4   },
    {   0,  RT_REL_MINUS_OUTER,   5   },
    {   1,  RT_REL_MINUS_OUTER,   0   },
    {   1,  RT_REL_MINUS_OUTER,   3   },
    {   1,  RT_REL_MINUS_OUTER,   4   },
    {   1,  RT_REL_MINUS_OUTER,   5   },
    {   2,  RT_REL_MINUS_OUTER,   0   },
    {   2,  RT_REL_MINUS_OUTER,   3   },
    {   2,  RT_REL_MINUS_OUTER,   4   },
    {   2,  RT_REL_MINUS_OUTER,   5   },
    {   3,  RT_REL_MINUS_OUTER,   1   },
    {   3,  RT_REL_MINUS_OUTER,   2   },
    {   3,  RT_REL_MINUS_OUTER,   4   },
    {   3,  RT_REL_MINUS_OUTER,   5   },
    {   4,  RT_REL_MINUS_OUTER,   0   },
    {   4,  RT_REL_MINUS_OUTER,   1   },
    {   4,  RT_REL_MINUS_OUTER,   2   },
    {   4,  RT_REL_MINUS_OUTER,   3   },
    {   5,  RT_REL_MINUS_OUTER,   0   },
    {   5,  RT_REL_MINUS_OUTER,   1   },
    {   5,  RT_REL_MINUS_OUTER,   2   },
    {   5,  RT_REL_MINUS_OUTER,   3   },
    {   6,  RT_REL_MINUS_OUTER,   2   },
};

/******************************************************************************/
/**********************************   SCENE   *********************************/
/******************************************************************************/

rt_SCENE sc_root =
{
    RT_OBJ_ARRAY_REL(&ob_tree, &rl_tree),
    /* list of optimizations to be turned off *
     * refer to core/engine/format.h for defs */
    RT_OPTS_GAMMA | RT_OPTS_FRESNEL
    /* turning off GAMMA|FRESNEL opts in turn *
     * enables respective GAMMA|FRESNEL props */
};

} /* namespace scn_test18 */

#endif /* RT_SCN_TEST18_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
