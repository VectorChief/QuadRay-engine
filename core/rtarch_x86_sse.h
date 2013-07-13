/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X86_SSE_H
#define RT_RTARCH_X86_SSE_H

#include "rtarch_x86.h"

#define RT_SIMD_ALIGN       16
#define RT_SIMD_SET(a, v)   a[0] = v; a[1] = v; a[2] = v; a[3] = v

/* registers    REG,  MOD,  SIB */

#define Xmm0    0x00, 0x03, EMPTY
#define Xmm1    0x01, 0x03, EMPTY
#define Xmm2    0x02, 0x03, EMPTY
#define Xmm3    0x03, 0x03, EMPTY
#define Xmm4    0x04, 0x03, EMPTY
#define Xmm5    0x05, 0x03, EMPTY
#define Xmm6    0x06, 0x03, EMPTY
#define Xmm7    0x07, 0x03, EMPTY

/******************************************************************************/
/**********************************   SSE   ***********************************/
/******************************************************************************/

/* fpu (SSE1) */

#define movps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x28)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define movps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x28)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define movps_st(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x29)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define adrpx_ld(RG, RM, DP) /* only for SIMD-aligned displacements */      \
        EMITB(0x8D)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), EMITW(VAL(DP) & 0xFFFFFFF0), EMPTY)


#define addps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x58)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define addps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x58)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)


#define subps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5C)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define subps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5C)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)


#define mulps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x59)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define mulps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x59)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)


#define divps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5E)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define divps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5E)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)


#define sqrps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x51)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define sqrps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x51)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)


#define rceps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x53)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define rcpps_rr(RG, RM) /* destroys value in RM */                         \
        rceps_rr(W(RG), W(RM))                                              \
        mulps_rr(W(RM), W(RG))                                              \
        mulps_rr(W(RM), W(RG))                                              \
        addps_rr(W(RG), W(RG))                                              \
        subps_rr(W(RG), W(RM))


#define rseps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x52)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define rsqps_rr(RG, RM) /* destroys value in RM */                         \
        rseps_rr(W(RG), W(RM))                                              \
        mulps_rr(W(RM), W(RG))                                              \
        mulps_rr(W(RM), W(RG))                                              \
        subps_ld(W(RM), Mebp, inf_GPC03)                                    \
        mulps_rr(W(RG), W(RM))                                              \
        mulps_ld(W(RG), Mebp, inf_GPC02)


#define andps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x54)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define andps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x54)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)


#define annps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x55)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define annps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x55)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)


#define orrps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x56)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define orrps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x56)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)


#define xorps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x57)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define xorps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x57)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)


#define minps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5D)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define minps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5D)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)


#define maxps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5F)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define maxps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5F)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)


#define ceqps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define cltps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cleps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define cneps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cgeps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgtps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(0x06))


#define movsn_rr(RG, RM) /* not portable, do not use outside */             \
        EMITB(0x0F) EMITB(0x50)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define RT_SIMD_MASK_NONE       0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       0x0F    /*  all satisfy the condition */

#define CHECK_MASK(lb, mask, RG) /* destroys value in Reax */               \
        movsn_rr(Reax, W(RG))                                               \
        cmpxx_ri(Reax, IB(RT_SIMD_MASK_##mask))                             \
        jeqxx_lb(lb)


#define mxcsr_ld(RM, DP) /* not portable, do not use outside */             \
        EMITB(0x0F) EMITB(0xAE)                                             \
            MRM(0x02,    MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define mxcsr_st(RM, DP) /* not portable, do not use outside */             \
        EMITB(0x0F) EMITB(0xAE)                                             \
            MRM(0x03,    MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define RT_SIMD_MODE_ROUNDN     0x00    /* round to nearest */
#define RT_SIMD_MODE_ROUNDM     0x01    /* round towards minus infinity */
#define RT_SIMD_MODE_ROUNDP     0x02    /* round towards plus  infinity */
#define RT_SIMD_MODE_ROUNDZ     0x03    /* round towards zero */

#define FCTRL_ENTER(mode) /* destroys value in Reax */                      \
        mxcsr_st(Mebp, inf_FCTRL)                                           \
        movxx_ld(Reax, Mebp, inf_FCTRL)                                     \
        orrxx_mi(Mebp, inf_FCTRL, IH(RT_SIMD_MODE_##mode << 13))            \
        mxcsr_ld(Mebp, inf_FCTRL)                                           \
        movxx_st(Reax, Mebp, inf_FCTRL)

#define FCTRL_LEAVE(mode) /* destroys value in Reax (in ARM version) */     \
        mxcsr_ld(Mebp, inf_FCTRL)

/* int (SSE2) */

#define cvtpn_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5B)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define cvtps_rr(RG, RM)                                                    \
        EMITB(0x66) EMITB(0x0F) EMITB(0x5B)                                 \
            MRM(REG(RG), MOD(RM), REG(RM))


#define addpn_rr(RG, RM)                                                    \
        EMITB(0x66) EMITB(0x0F) EMITB(0xFE)                                 \
            MRM(REG(RG), MOD(RM), REG(RM))

#define addpn_ld(RG, RM, DP)                                                \
        EMITB(0x66) EMITB(0x0F) EMITB(0xFE)                                 \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)


#define shlpn_ri(RM, IM)                                                    \
        EMITB(0x66) EMITB(0x0F) EMITB(0x72)                                 \
            MRM(0x06,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(VAL(IM)))

#define shlpn_ld(RG, RM, DP)                                                \
        EMITB(0x66) EMITB(0x0F) EMITB(0xF2)                                 \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)


#define shrpn_ri(RM, IM)                                                    \
        EMITB(0x66) EMITB(0x0F) EMITB(0x72)                                 \
            MRM(0x02,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(VAL(IM)))

#define shrpn_ld(RG, RM, DP)                                                \
        EMITB(0x66) EMITB(0x0F) EMITB(0xD2)                                 \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#endif /* RT_RTARCH_X86_SSE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
