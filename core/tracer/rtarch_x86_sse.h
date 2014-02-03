/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X86_SSE_H
#define RT_RTARCH_X86_SSE_H

#include "rtarch_x86.h"

#define RT_SIMD_ALIGN       16
#define RT_SIMD_SET(a, v)   a[0] = v; a[1] = v; a[2] = v; a[3] = v

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

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
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/* cmdpx_ri - applies [cmd] to [r]egister from [i]mmediate  */
/* cmdpx_rr - applies [cmd] to [r]egister from [r]egister   */

/* cmdpx_rm - applies [cmd] to [r]egister from [m]emory     */
/* cmdpx_ld - applies [cmd] as above                        */
/* cmdpx_mr - applies [cmd] to [m]emory   from [r]egister   */
/* cmdpx_st - applies [cmd] as above (arg list as cmdxx_ld) */

/* cmdpx_** - applies [cmd] to packed unsigned integer args */
/* cmdpn_** - applies [cmd] to packed   signed integer args */
/* cmdps_** - applies [cmd] to packed single precision args */
/* cmdpd_** - applies [cmd] to packed double precision args */

/******************************************************************************/
/**********************************   SSE   ***********************************/
/******************************************************************************/

/**************************   packed generic (SSE1)   *************************/

/* mov */

#define movpx_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x28)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define movpx_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x28)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define movpx_st(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x29)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define adrpx_ld(RG, RM, DP) /* only for SIMD-aligned displacements */      \
        EMITB(0x8D)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), EMITW(VAL(DP) & 0xFFFFFFF0), EMPTY)

/* and */

#define andpx_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x54)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define andpx_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x54)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* ann */

#define annpx_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x55)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define annpx_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x55)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* orr */

#define orrpx_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x56)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define orrpx_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x56)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* xor */

#define xorpx_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x57)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define xorpx_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x57)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/**************   packed single precision floating point (SSE1)   *************/

/* add */

#define addps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x58)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define addps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x58)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5C)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define subps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5C)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* mul */

#define mulps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x59)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define mulps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x59)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* div */

#define divps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5E)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define divps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5E)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* sqr */

#define sqrps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x51)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define sqrps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x51)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* cbr,
 * original idea: Russell Borogove (kaleja[AT]estarcion[DOT]com)
 * posted at http://www.musicdsp.org/showone.php?id=206
 * converted to 4-way vector version by VectorChief */

#define cbrps_rr(RG, R1, R2, RM) /* destroys value in R1, R2 (temp regs) */ \
        /* cube root estimate, the exponent is divided by three             \
         * in such a way that remainder bits get shoved into                \
         * the top of the normalized mantissa */                            \
        movpx_ld(W(R2), Mebp, inf_GPC04)                                    \
        movpx_rr(W(RG), W(RM))                                              \
        andpx_rr(W(RG), W(R2))   /* exponent & mantissa in biased-127 */    \
        subpx_ld(W(RG), Mebp, inf_GPC05) /* convert to 2's complement */    \
        shrpn_ri(W(RG), IB(10))  /* RG / 1024 */                            \
        movpx_rr(W(R1), W(RG))   /* RG * 341 (next 8 ops) */                \
        shlpx_ri(W(R1), IB(2))                                              \
        addpx_rr(W(RG), W(R1))                                              \
        shlpx_ri(W(R1), IB(2))                                              \
        addpx_rr(W(RG), W(R1))                                              \
        shlpx_ri(W(R1), IB(2))                                              \
        addpx_rr(W(RG), W(R1))                                              \
        shlpx_ri(W(R1), IB(2))                                              \
        addpx_rr(W(RG), W(R1))   /* RG * (341/1024) ~= RG * (0.333) */      \
        addpx_ld(W(RG), Mebp, inf_GPC05) /* back to biased-127 */           \
        andpx_rr(W(RG), W(R2))   /* remask exponent & mantissa */           \
        annpx_rr(W(R2), W(RM))   /* original sign */                        \
        orrpx_rr(W(RG), W(R2))   /* new exponent & mantissa, old sign */    \
        /* 1st Newton-Raphson approx */                                     \
        movpx_rr(W(R1), W(RG))                                              \
        mulps_rr(W(R1), W(RG))                                              \
        movpx_rr(W(R2), W(R1))                                              \
        mulps_ld(W(R1), Mebp, inf_GPC03)                                    \
        rceps_rr(W(R1), W(R1))                                              \
        mulps_rr(W(R2), W(RG))                                              \
        subps_rr(W(R2), W(RM))                                              \
        mulps_rr(W(R2), W(R1))                                              \
        subps_rr(W(RG), W(R2))                                              \
        /* 2nd Newton-Raphson approx */                                     \
        movpx_rr(W(R1), W(RG))                                              \
        mulps_rr(W(R1), W(RG))                                              \
        movpx_rr(W(R2), W(R1))                                              \
        mulps_ld(W(R1), Mebp, inf_GPC03)                                    \
        rceps_rr(W(R1), W(R1))                                              \
        mulps_rr(W(R2), W(RG))                                              \
        subps_rr(W(R2), W(RM))                                              \
        mulps_rr(W(R2), W(R1))                                              \
        subps_rr(W(RG), W(R2))                                              \
        /* 3rd Newton-Raphson approx */                                     \
        movpx_rr(W(R1), W(RG))                                              \
        mulps_rr(W(R1), W(RG))                                              \
        movpx_rr(W(R2), W(R1))                                              \
        mulps_ld(W(R1), Mebp, inf_GPC03)                                    \
        rceps_rr(W(R1), W(R1))                                              \
        mulps_rr(W(R2), W(RG))                                              \
        subps_rr(W(R2), W(RM))                                              \
        mulps_rr(W(R2), W(R1))                                              \
        subps_rr(W(RG), W(R2))

/* rcp */

#define rceps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x53)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define rcpps_rr(RG, RM) /* destroys value in RM */                         \
        rceps_rr(W(RG), W(RM))                                              \
        mulps_rr(W(RM), W(RG))                                              \
        mulps_rr(W(RM), W(RG))                                              \
        addps_rr(W(RG), W(RG))                                              \
        subps_rr(W(RG), W(RM))

/* rsq */

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

/* min */

#define minps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5D)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define minps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5D)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* max */

#define maxps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5F)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define maxps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5F)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* cmp */

#define ceqps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMITB(0x00))

#define cneps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cneps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMITB(0x04))

#define cltps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMITB(0x01))

#define cleps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define cleps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMITB(0x02))

#define cgtps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMITB(0x06))

#define cgeps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgeps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMITB(0x05))

/**************************   packed integer (SSE2)   *************************/

/* cvt */

#define cvtps_rr(RG, RM)                                                    \
        EMITB(0x66) EMITB(0x0F) EMITB(0x5B)                                 \
            MRM(REG(RG), MOD(RM), REG(RM))

#define cvtps_ld(RG, RM, DP)                                                \
        EMITB(0x66) EMITB(0x0F) EMITB(0x5B)                                 \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define cvtpn_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5B)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define cvtpn_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5B)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* add */

#define addpx_rr(RG, RM)                                                    \
        EMITB(0x66) EMITB(0x0F) EMITB(0xFE)                                 \
            MRM(REG(RG), MOD(RM), REG(RM))

#define addpx_ld(RG, RM, DP)                                                \
        EMITB(0x66) EMITB(0x0F) EMITB(0xFE)                                 \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subpx_rr(RG, RM)                                                    \
        EMITB(0x66) EMITB(0x0F) EMITB(0xFA)                                 \
            MRM(REG(RG), MOD(RM), REG(RM))

#define subpx_ld(RG, RM, DP)                                                \
        EMITB(0x66) EMITB(0x0F) EMITB(0xFA)                                 \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* shl */

#define shlpx_ri(RM, IM)                                                    \
        EMITB(0x66) EMITB(0x0F) EMITB(0x72)                                 \
            MRM(0x06,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(VAL(IM)))

#define shlpx_ld(RG, RM, DP)                                                \
        EMITB(0x66) EMITB(0x0F) EMITB(0xF2)                                 \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* shr */

#define shrpx_ri(RM, IM)                                                    \
        EMITB(0x66) EMITB(0x0F) EMITB(0x72)                                 \
            MRM(0x02,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(VAL(IM)))

#define shrpx_ld(RG, RM, DP)                                                \
        EMITB(0x66) EMITB(0x0F) EMITB(0xD2)                                 \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define shrpn_ri(RM, IM)                                                    \
        EMITB(0x66) EMITB(0x0F) EMITB(0x72)                                 \
            MRM(0x04,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(VAL(IM)))

#define shrpn_ld(RG, RM, DP)                                                \
        EMITB(0x66) EMITB(0x0F) EMITB(0xE2)                                 \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/*****************************   helper macros   ******************************/

/* simd mask */

#define RT_SIMD_MASK_NONE       0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       0x0F    /*  all satisfy the condition */

#define movsn_rr(RG, RM) /* not portable, do not use outside */             \
        EMITB(0x0F) EMITB(0x50)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define CHECK_MASK(lb, mask, RG) /* destroys value in Reax */               \
        movsn_rr(Reax, W(RG))                                               \
        cmpxx_ri(Reax, IB(RT_SIMD_MASK_##mask))                             \
        jeqxx_lb(lb)

/* simd mode */

#define RT_SIMD_MODE_ROUNDN     0x00    /* round to nearest */
#define RT_SIMD_MODE_ROUNDM     0x01    /* round towards minus infinity */
#define RT_SIMD_MODE_ROUNDP     0x02    /* round towards plus  infinity */
#define RT_SIMD_MODE_ROUNDZ     0x03    /* round towards zero */

#define mxcsr_ld(RM, DP) /* not portable, do not use outside */             \
        EMITB(0x0F) EMITB(0xAE)                                             \
            MRM(0x02,    MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define mxcsr_st(RM, DP) /* not portable, do not use outside */             \
        EMITB(0x0F) EMITB(0xAE)                                             \
            MRM(0x03,    MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define FCTRL_ENTER(mode) /* destroys value in Reax */                      \
        mxcsr_st(Mebp, inf_FCTRL)                                           \
        movxx_ld(Reax, Mebp, inf_FCTRL)                                     \
        orrxx_mi(Mebp, inf_FCTRL, IH(RT_SIMD_MODE_##mode << 13))            \
        mxcsr_ld(Mebp, inf_FCTRL)                                           \
        movxx_st(Reax, Mebp, inf_FCTRL)

#define FCTRL_LEAVE(mode) /* destroys value in Reax (in ARM) */             \
        mxcsr_ld(Mebp, inf_FCTRL)

#endif /* RT_RTARCH_X86_SSE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
