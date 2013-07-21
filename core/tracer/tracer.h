/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_TRACER_H
#define RT_TRACER_H

#include "rtarch.h"
#include "rtbase.h"

/******************************************************************************/
/**********************************   MISC   **********************************/
/******************************************************************************/

struct rt_SIMD_INFO_EXT : public rt_SIMD_INFO
{
    /* external parameters */

    rt_word frm_w;
#define inf_FRM_W           DP(0x100)

    rt_word frm_h;
#define inf_FRM_H           DP(0x104)

    rt_cell frm_row;
#define inf_FRM_ROW         DP(0x108)

    rt_pntr frame;
#define inf_FRAME           DP(0x10C)

    /* internal variables */

    rt_word frm_x;
#define inf_FRM_X           DP(0x110)

    rt_word frm_y;
#define inf_FRM_Y           DP(0x114)

    rt_pntr frm;
#define inf_FRM             DP(0x118)

    rt_word pad10;
#define inf_PAD10           DP(0x11C)

};

/******************************************************************************/
/***************************   GLOBAL ENTRY POINTS   **************************/
/******************************************************************************/

rt_cell render0(rt_SIMD_INFO_EXT *info);

#endif /* RT_TRACER_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
