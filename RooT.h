/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include "rtimag.h"
#include "engine.h"

#include "scn_demo01.h"
using namespace scn_demo01;

#define RT_X_RES    800
#define RT_Y_RES    480
#define RT_TITLE    "QuadRay engine demo, (C) 2013 VectorChief"

rt_astr     title   = RT_TITLE;
rt_cell     x_res   = RT_X_RES;
rt_cell     y_res   = RT_Y_RES;
rt_cell     x_row   = RT_X_RES;
rt_word    *frame   = RT_NULL;

rt_cell     fsaa    = RT_FSAA_NO;

rt_Scene   *scene   = RT_NULL;

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
