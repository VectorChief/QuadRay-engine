/******************************************************************************/
/* Copyright (c) 2013-2017 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_OBJ_FRAMETABLE_H
#define RT_OBJ_FRAMETABLE_H

#include "format.h"

#include "all_mat.h"

/******************************************************************************/
/*******************************   FRAMETABLE   *******************************/
/******************************************************************************/

/* --------------------------------   BOUND   ------------------------------- */

rt_RELATION rl_bound[] =
{
    {  -1,  RT_REL_BOUND_ARRAY,  -1   },
};

/* -------------------------------   CORNER   ------------------------------- */

rt_CYLINDER cl_frametable01outer01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -0.0,       -0.0,       -0.2    },
/* max */   {  +RT_INF,    +RT_INF,     +0.2    },
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
/* rad */   0.5,
};

rt_CYLINDER cl_frametable01inner01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -0.0,       -0.0,       -0.2    },
/* max */   {  +RT_INF,    +RT_INF,     +0.2    },
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
/* mat */   &mt_plain03_white01,
        },
    },
/* rad */   0.4,
};

rt_PLANE pl_frametable01side01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -0.0,       -0.0,      -RT_INF  },
/* max */   {   +0.5,       +0.5,      +RT_INF  },
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

rt_OBJECT ob_frametable01corner01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
            RT_OBJ_CYLINDER(&cl_frametable01outer01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
            RT_OBJ_CYLINDER(&cl_frametable01inner01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,       +0.2    },
        },
            RT_OBJ_PLANE(&pl_frametable01side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  180.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.5,       -0.2    },
        },
            RT_OBJ_PLANE(&pl_frametable01side01)
    },
};

rt_RELATION rl_frametable01corner01[] = 
{
    {  -1,  RT_REL_BOUND_ARRAY,  -1   },
    {   2,  RT_REL_MINUS_OUTER,   0   },
    {   2,  RT_REL_MINUS_INNER,   1   },
    {   3,  RT_REL_MINUS_OUTER,   0   },
    {   3,  RT_REL_MINUS_INNER,   1   },
};

/* -------------------------------   HORLEG   ------------------------------- */

rt_PLANE pl_frametable01side02 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -1.5,       -0.2,      -RT_INF  },
/* max */   {   +1.5,       +0.2,      +RT_INF  },
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

rt_PLANE pl_frametable01side03 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -1.5,       -0.0,      -RT_INF  },
/* max */   {   +1.5,       +0.1,      +RT_INF  },
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

rt_OBJECT ob_frametable01horleg01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {   90.0,        0.0,        0.0    },
/* pos */   {    0.0,       -0.1,        0.0    },
        },
            RT_OBJ_PLANE(&pl_frametable01side02)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  -90.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
            RT_OBJ_PLANE(&pl_frametable01side02)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  180.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,       -0.2    },
        },
            RT_OBJ_PLANE(&pl_frametable01side03)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,       -0.1,       +0.2    },
        },
            RT_OBJ_PLANE(&pl_frametable01side03)
    },
};

/* -------------------------------   VERLEG   ------------------------------- */

rt_PLANE pl_frametable01side04 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -1.0,       -0.2,      -RT_INF  },
/* max */   {   +1.0,       +0.2,      +RT_INF  },
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

rt_PLANE pl_frametable01side05 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -1.0,       -0.0,      -RT_INF  },
/* max */   {   +1.0,       +0.1,      +RT_INF  },
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

rt_OBJECT ob_frametable01verleg01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {   90.0,        0.0,        0.0    },
/* pos */   {    0.0,       -0.1,        0.0    },
        },
            RT_OBJ_PLANE(&pl_frametable01side04)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  -90.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
            RT_OBJ_PLANE(&pl_frametable01side04)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  180.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,       -0.2    },
        },
            RT_OBJ_PLANE(&pl_frametable01side05)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,       -0.1,       +0.2    },
        },
            RT_OBJ_PLANE(&pl_frametable01side05)
    },
};

/* --------------------------------   FRAME   ------------------------------- */

rt_OBJECT ob_frametable01leg01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,      180.0    },
/* pos */   {   -1.5,       -1.0,        0.0    },
        },
            RT_OBJ_ARRAY_REL(&ob_frametable01corner01, &rl_frametable01corner01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,      -90.0    },
/* pos */   {   +1.5,       -1.0,        0.0    },
        },
            RT_OBJ_ARRAY_REL(&ob_frametable01corner01, &rl_frametable01corner01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  180.0,        0.0,        0.0    },
/* pos */   {    0.0,       -1.5,        0.0    },
        },
            RT_OBJ_ARRAY_REL(&ob_frametable01horleg01, &rl_bound)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  180.0,        0.0,      -90.0    },
/* pos */   {   -2.0,        0.0,        0.0    },
        },
            RT_OBJ_ARRAY_REL(&ob_frametable01verleg01, &rl_bound)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,       90.0    },
/* pos */   {   -1.5,       +1.0,        0.0    },
        },
            RT_OBJ_ARRAY_REL(&ob_frametable01corner01, &rl_frametable01corner01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {   +1.5,       +1.0,        0.0    },
        },
            RT_OBJ_ARRAY_REL(&ob_frametable01corner01, &rl_frametable01corner01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,       +1.5,        0.0    },
        },
            RT_OBJ_ARRAY_REL(&ob_frametable01horleg01, &rl_bound)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  180.0,        0.0,      +90.0    },
/* pos */   {   +2.0,        0.0,        0.0    },
        },
            RT_OBJ_ARRAY_REL(&ob_frametable01verleg01, &rl_bound)
    },
};

/* ---------------------------------   TOP   -------------------------------- */

rt_PLANE pl_frametable01side06 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -2.5,       -1.5,      -RT_INF  },
/* max */   {   +2.5,       +1.5,      +RT_INF  },
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
/* mat */   &mt_plain01_gray02,
        },
    },
};

rt_PLANE pl_frametable01side07 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -2.5,       -0.1,      -RT_INF  },
/* max */   {   +2.5,       +0.0,      +RT_INF  },
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
/* mat */   &mt_plain01_gray02,
        },
    },
};

rt_OBJECT ob_frametable01top01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  180.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,       -0.1    },
        },
            RT_OBJ_PLANE(&pl_frametable01side06)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
            RT_OBJ_PLANE(&pl_frametable01side06)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  -90.0,        0.0,        0.0    },
/* pos */   {    0.0,       +1.5,       -0.1    },
        },
            RT_OBJ_PLANE(&pl_frametable01side07)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  +90.0,        0.0,        0.0    },
/* pos */   {    0.0,       -1.5,        0.0    },
        },
            RT_OBJ_PLANE(&pl_frametable01side07)
    },
};

/* --------------------------------   TABLE   ------------------------------- */

rt_OBJECT ob_frametable01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {   90.0,        0.0,      +90.0    },
/* pos */   {   -2.7,        0.0,        1.5    },
        },
            RT_OBJ_ARRAY_REL(&ob_frametable01leg01, &rl_bound)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {   90.0,        0.0,      -90.0    },
/* pos */   {   +2.7,        0.0,        1.5    },
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

#endif /* RT_OBJ_FRAMETABLE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
