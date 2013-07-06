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


#define mulps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x59) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define mulps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x59) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP


#define divps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5E) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define divps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5E) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP


#define sqrps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x51) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define rsqps_rr(RG, RT, RM)                                                \
        EMITB(0x0F) EMITB(0x51) MRM(MOD(RM), REG(RT), REG(RM), SIB(RM))     \
        EMITB(0x0F) EMITB(0x5E) MRM(MOD(RT), REG(RG), REG(RT), SIB(RT))


#define andps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x54) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define andps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x54) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP


#define annps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x55) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define annps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x55) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP


#define orrps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x56) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define orrps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x56) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP


#define xorps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x57) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define xorps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x57) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP


#define minps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5D) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define minps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5D) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP


#define maxps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5F) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define maxps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5F) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP


#define ceqps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))     \
        EMITB(0x00)

#define cltps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))     \
        EMITB(0x01)

#define cleps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))     \
        EMITB(0x02)

#define cneps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))     \
        EMITB(0x04)

#define cgeps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))     \
        EMITB(0x05)

#define cgtps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))     \
        EMITB(0x06)


#define movsn_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x50) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define NONE  0x00
#define FULL  0x0F

#define CHECK_MASK(lb, cc)                                                  \
        movsn_rr(Reax, Xmm7)                                                \
        cmpxx_ri(Reax, IM(cc))                                              \
        jeqxx_lb(lb)


#define mxcsr_ld(RM, DP)                                                    \
        EMITB(0x0F) EMITB(0xAE) MRM(MOD(RM),    0x02, REG(RM), SIB(RM)) DP

#define mxcsr_st(RM, DP)                                                    \
        EMITB(0x0F) EMITB(0xAE) MRM(MOD(RM),    0x03, REG(RM), SIB(RM)) DP

#define FCTRL_ENTER()                                                       \
        mxcsr_st(Mebp, inf_FCTRL)                                           \
        movxx_ld(Reax, Mebp, inf_FCTRL)                                     \
        orrxx_mi(Mebp, inf_FCTRL, IM(1 << 13))                              \
        mxcsr_ld(Mebp, inf_FCTRL)                                           \
        movxx_st(Reax, Mebp, inf_FCTRL)

#define FCTRL_LEAVE()                                                       \
        mxcsr_ld(Mebp, inf_FCTRL)

/* int (SSE2) */

#define cvtpi_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5B) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define cvtps_rr(RG, RM)                                                    \
        EMITB(0x66) EMITB(0x0F) EMITB(0x5B)                                 \
                                MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))


#define addpi_rr(RG, RM)                                                    \
        EMITB(0x66) EMITB(0x0F) EMITB(0xFE)                                 \
                                MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define addpi_ld(RG, RM, DP)                                                \
        EMITB(0x66) EMITB(0x0F) EMITB(0xFE)                                 \
                                MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP


#define shlpi_ri(RM, IB)                                                    \
        EMITB(0x66) EMITB(0x0F) EMITB(0x72)                                 \
                                MRM(MOD(RM),    0x06, REG(RM), SIB(RM)) IB

#define shlpi_ld(RG, RM, DP)                                                \
        EMITB(0x66) EMITB(0x0F) EMITB(0xF2)                                 \
                                MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP


#define shrpi_ri(RM, IB)                                                    \
        EMITB(0x66) EMITB(0x0F) EMITB(0x72)                                 \
                                MRM(MOD(RM),    0x02, REG(RM), SIB(RM)) IB

#define shrpi_ld(RG, RM, DP)                                                \
        EMITB(0x66) EMITB(0x0F) EMITB(0xD2)                                 \
                                MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP

#endif /* RT_RTARCH_X86_SSE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
