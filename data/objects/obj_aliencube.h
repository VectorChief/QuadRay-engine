/******************************************************************************/
/* Copyright (c) 2013-2015 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_OBJ_ALIENCUBE_H
#define RT_OBJ_ALIENCUBE_H

#include "format.h"

#include "all_mat.h"

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

rt_SPHERE sp_aliencube01ball05 =
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

rt_SPHERE sp_aliencube01ball06 =
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

rt_OBJECT ob_aliencube01[] =
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
    {
        {  /*   RT_X,       RT_Y,       RT_Z    */
/* scl */   {    1.0,        1.0,        1.0    },
/* rot */   {    0.0,        0.0,        0.0    },
/* pos */   {    0.0,        0.0,        0.0    },
        },
            RT_OBJ_SPHERE(&sp_aliencube01ball05)
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
            RT_OBJ_SPHERE(&sp_aliencube01ball06)
    },
};

rt_RELATION rl_aliencube01[] = 
{
    {   0,  RT_REL_MINUS_OUTER,   6   },
    {   1,  RT_REL_MINUS_OUTER,   6   },
    {   2,  RT_REL_MINUS_OUTER,   6   },
    {   3,  RT_REL_MINUS_OUTER,   6   },
    {   4,  RT_REL_MINUS_OUTER,   6   },
    {   5,  RT_REL_MINUS_OUTER,   6   },
    {   0,  RT_REL_MINUS_INNER,   7   },
    {   1,  RT_REL_MINUS_INNER,   7   },
    {   2,  RT_REL_MINUS_INNER,   8   },
    {   3,  RT_REL_MINUS_INNER,   8   },
    {   4,  RT_REL_MINUS_INNER,   9   },
    {   5,  RT_REL_MINUS_INNER,   9   },
};

#endif /* RT_OBJ_ALIENCUBE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
