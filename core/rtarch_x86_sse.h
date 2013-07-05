/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X86_SSE_H
#define RT_RTARCH_X86_SSE_H

#include "rtarch_x86.h"

/* registers    MOD,  REG,  SIB */

#define Xmm0    0x03, 0x00, EMPTY
#define Xmm1    0x03, 0x01, EMPTY
#define Xmm2    0x03, 0x02, EMPTY
#define Xmm3    0x03, 0x03, EMPTY
#define Xmm4    0x03, 0x04, EMPTY
#define Xmm5    0x03, 0x05, EMPTY
#define Xmm6    0x03, 0x06, EMPTY
#define Xmm7    0x03, 0x07, EMPTY

/******************************************************************************/
/**********************************   SSE   ***********************************/
/******************************************************************************/

/* fpu (SSE1) */

#define movps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x28) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define movps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x28) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP

#define movps_st(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x29) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP


#define addps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x58) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define addps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x58) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP


#define subps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5C) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define subps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5C) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP

#endif /* RT_RTARCH_X86_SSE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
