/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_OBJ_ALIENCUBE_H
#define RT_OBJ_ALIENCUBE_H

#include "format.h"

#include "all_mat.h"

/******************************************************************************/
/********************************   ALIENCUBE   *******************************/
/******************************************************************************/

rt_PLANE pl_aliencube01side01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -1.0,       -1.0,      -RT_INF  },
/* max */   {   +1.0,       +1.0,      +RT_INF  },
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

rt_SPHERE sp_aliencube01ball01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,    -RT_INF  },
/* max */   {  +RT_INF,    +RT_INF,    +RT_INF  },
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
/* rad */   1.0,
};

rt_SPHERE sp_aliencube01ball02 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {   -1.0,       -1.0,       -1.0    },
/* max */   {   +1.0,       +1.0,       +1.0    },
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
/* rad */   1.5,
};

rt_CONE cn_aliencube01cone01 =
{
    {      /*   RT_I,       RT_J,       RT_K    */
/* min */   {  -RT_INF,    -RT_INF,     -1.0    },
/* max */   {  +RT_INF,    +RT_INF,     +1.0    },
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
/* rat */   0.8,
};

rt_OBJECT ob_aliencube01base01[] =
{
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  180.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,       -1.0    },
        },
            RT_OBJ_PLANE(&pl_aliencube01side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,       +1.0    },
        },
            RT_OBJ_PLANE(&pl_aliencube01side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  +90.0,        0.0,      -90.0    },
/* pos */   {   -1.0,        0.0,        0.0    },
        },
            RT_OBJ_PLANE(&pl_aliencube01side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  -90.0,        0.0,      -90.0    },
/* pos */   {   +1.0,        0.0,        0.0    },
        },
            RT_OBJ_PLANE(&pl_aliencube01side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  +90.0,        0.0,        0.0    },
/* pos */   {    0.0,       -1.0,        0.0    },
        },
            RT_OBJ_PLANE(&pl_aliencube01side01)
    },
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {  -90.0,        0.0,        0.0    },
/* pos */   {    0.0,       +1.0,        0.0    },
        },
            RT_OBJ_PLANE(&pl_aliencube01side01)
    },
};

rt_OBJECT ob_aliencube01[] =
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
/* scl */   {    1.0,        1.0,        1.0    },
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

rt_RELATION rl_aliencube01[] = 
{
    {  -1,  RT_REL_BOUND_ARRAY,  -1   },
    {   0,  RT_REL_MINUS_OUTER,   1   },
    {   0,  RT_REL_INDEX_ARRAY,  -1   },
    {   0,  RT_REL_MINUS_INNER,   2   },
    {   0,  RT_REL_INDEX_ARRAY,  -1   },
    {   1,  RT_REL_MINUS_INNER,   2   },
    {   0,  RT_REL_INDEX_ARRAY,  -1   },
    {   2,  RT_REL_MINUS_INNER,   3   },
    {   0,  RT_REL_INDEX_ARRAY,  -1   },
    {   3,  RT_REL_MINUS_INNER,   3   },
    {   0,  RT_REL_INDEX_ARRAY,  -1   },
    {   4,  RT_REL_MINUS_INNER,   4   },
    {   0,  RT_REL_INDEX_ARRAY,  -1   },
    {   5,  RT_REL_MINUS_INNER,   4   },
};

#endif /* RT_OBJ_ALIENCUBE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
