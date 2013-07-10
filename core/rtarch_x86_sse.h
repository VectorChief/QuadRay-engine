/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X86_SSE_H
#define RT_RTARCH_X86_SSE_H

#include "rtarch_x86.h"

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
        MRM(MOD(RM), REG(RG), REG(RM))

#define movps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x28)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), DP,      EMPTY)

#define movps_st(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x29)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), DP,      EMPTY)


#define addps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x58)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))

#define addps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x58)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), DP,      EMPTY)


#define subps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5C)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))

#define subps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5C)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), DP,      EMPTY)


#define mulps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x59)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))

#define mulps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x59)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), DP,      EMPTY)


#define divps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5E)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))

#define divps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5E)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), DP,      EMPTY)


#define sqrps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x51)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))

#define rsqps_rr(RG, RT, RM)                                                \
        EMITB(0x0F) EMITB(0x51)                                             \
        MRM(MOD(RM), REG(RT), REG(RM))                                      \
        EMITB(0x0F) EMITB(0x5E)                                             \
        MRM(MOD(RT), REG(RG), REG(RT))


#define andps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x54)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))

#define andps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x54)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), DP,      EMPTY)


#define annps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x55)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))

#define annps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x55)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), DP,      EMPTY)


#define orrps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x56)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))

#define orrps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x56)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), DP,      EMPTY)


#define xorps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x57)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))

#define xorps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x57)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), DP,      EMPTY)


#define minps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5D)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))

#define minps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5D)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), DP,      EMPTY)


#define maxps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5F)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))

#define maxps_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5F)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), DP,      EMPTY)


#define ceqps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        EMITB(0x00)

#define cltps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        EMITB(0x01)

#define cleps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        EMITB(0x02)

#define cneps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        EMITB(0x04)

#define cgeps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        EMITB(0x05)

#define cgtps_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        EMITB(0x06)


#define movsn_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x50)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))

#define NONE  0x00
#define FULL  0x0F

#define CHECK_MASK(lb, cc)                                                  \
        movsn_rr(Reax, Xmm7)                                                \
        cmpxx_ri(Reax, IB(cc))                                              \
        jeqxx_lb(lb)


#define mxcsr_ld(RM, DP)                                                    \
        EMITB(0x0F) EMITB(0xAE)                                             \
        MRM(MOD(RM), 0x02,    REG(RM))                                      \
        AUX(SIB(RM), DP,      EMPTY)

#define mxcsr_st(RM, DP)                                                    \
        EMITB(0x0F) EMITB(0xAE)                                             \
        MRM(MOD(RM), 0x03,    REG(RM))                                      \
        AUX(SIB(RM), DP,      EMPTY)

#define FCTRL_ENTER()                                                       \
        mxcsr_st(Mebp, inf_FCTRL)                                           \
        movxx_ld(Reax, Mebp, inf_FCTRL)                                     \
        orrxx_mi(Mebp, inf_FCTRL, IH(1 << 13))                              \
        mxcsr_ld(Mebp, inf_FCTRL)                                           \
        movxx_st(Reax, Mebp, inf_FCTRL)

#define FCTRL_LEAVE()                                                       \
        mxcsr_ld(Mebp, inf_FCTRL)

/* int (SSE2) */

#define cvtpn_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5B)                                             \
        MRM(MOD(RM), REG(RG), REG(RM))

#define cvtps_rr(RG, RM)                                                    \
        EMITB(0x66) EMITB(0x0F) EMITB(0x5B)                                 \
        MRM(MOD(RM), REG(RG), REG(RM))


#define addpn_rr(RG, RM)                                                    \
        EMITB(0x66) EMITB(0x0F) EMITB(0xFE)                                 \
        MRM(MOD(RM), REG(RG), REG(RM))

#define addpn_ld(RG, RM, DP)                                                \
        EMITB(0x66) EMITB(0x0F) EMITB(0xFE)                                 \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), DP,      EMPTY)


#define shlpn_ri(RM, IM)                                                    \
        EMITB(0x66) EMITB(0x0F) EMITB(0x72)                                 \
        MRM(MOD(RM), 0x06,    REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM)))

#define shlpn_ld(RG, RM, DP)                                                \
        EMITB(0x66) EMITB(0x0F) EMITB(0xF2)                                 \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), DP,      EMPTY)


#define shrpn_ri(RM, IM)                                                    \
        EMITB(0x66) EMITB(0x0F) EMITB(0x72)                                 \
        MRM(MOD(RM), 0x02,    REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM)))

#define shrpn_ld(RG, RM, DP)                                                \
        EMITB(0x66) EMITB(0x0F) EMITB(0xD2)                                 \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), DP,      EMPTY)

#endif /* RT_RTARCH_X86_SSE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
