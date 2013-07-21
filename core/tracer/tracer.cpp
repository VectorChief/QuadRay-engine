/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include "tracer.h"

/******************************************************************************/
/*********************************   RENDER   *********************************/
/******************************************************************************/

/* This function is one of the backend's
 * global entry points (hence 0) used by the engine.
 * It renders the whole frame based on the data structures
 * prepared by the engine.
 */
rt_cell render0(rt_SIMD_INFO_EXT *info)
{

/******************************************************************************/
/*********************************   ENTER   **********************************/
/******************************************************************************/

    ASM_ENTER(info)

        movxx_mi(Mebp, inf_FRM_Y, IB(0))

    LBL(YY_cyc)

        movxx_ld(Reax, Mebp, inf_FRM_Y)
        mulxn_ld(Reax, Mebp, inf_FRM_ROW)
        shlxx_ri(Reax, IB(2))
        addxx_ld(Reax, Mebp, inf_FRAME)
        movxx_st(Reax, Mebp, inf_FRM)

        movxx_mi(Mebp, inf_FRM_X, IB(0))

    LBL(XX_cyc)

        movxx_ld(Reax, Mebp, inf_FRM_X)
        movxx_rr(Redx, Reax)
        shlxx_ri(Reax, IB(2))
        addxx_ld(Reax, Mebp, inf_FRM)

        /* fill with blue gradient for a test */
        shrxx_ri(Redx, IB(2))
        movxx_st(Redx, Oeax, PLAIN)
        addxx_ri(Reax, IB(4))
        movxx_st(Redx, Oeax, PLAIN)
        addxx_ri(Reax, IB(4))
        movxx_st(Redx, Oeax, PLAIN)
        addxx_ri(Reax, IB(4))
        movxx_st(Redx, Oeax, PLAIN)

    LBL(XX_end)

        addxx_mi(Mebp, inf_FRM_X, IB(4))

        movxx_ld(Reax, Mebp, inf_FRM_X)
        cmpxx_rm(Reax, Mebp, inf_FRM_W)
        jltxx_lb(XX_cyc)

    LBL(YY_end)

        addxx_mi(Mebp, inf_FRM_Y, IB(1))

        movxx_ld(Reax, Mebp, inf_FRM_Y)
        cmpxx_rm(Reax, Mebp, inf_FRM_H)
        jltxx_lb(YY_cyc)

    ASM_LEAVE(info)

/******************************************************************************/
/*********************************   LEAVE   **********************************/
/******************************************************************************/

    return 0;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
