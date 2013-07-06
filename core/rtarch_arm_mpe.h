/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_ARM_MPE_H
#define RT_RTARCH_ARM_MPE_H

#include "rtarch_arm.h"

/* registers    MOD,  REG,  SIB */

#define Xmm0    0x00, 0x00, EMPTY
#define Xmm1    0x00, 0x02, EMPTY
#define Xmm2    0x00, 0x04, EMPTY
#define Xmm3    0x00, 0x06, EMPTY
#define Xmm4    0x00, 0x08, EMPTY
#define Xmm5    0x00, 0x0A, EMPTY
#define Xmm6    0x00, 0x0C, EMPTY
#define Xmm7    0x00, 0x0E, EMPTY

#define T0      0x00, 0x00, EMPTY
#define T1      0x01, 0x00, EMPTY
#define T2      0x01, 0x02, EMPTY
#define T3      0x01, 0x04, EMPTY

#define RT_USE_VFP          0   /* use VFP instead of some MPE instructions */

/******************************************************************************/
/**********************************   MPE   ***********************************/
/******************************************************************************/

/* fpu */

#define movps_rr(RG, RM)                                                    \
        EMITW(0xF2200150 | MRM(REG(RG), REG(RM), REG(RM)))

#define movps_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE2800F00 | MRM(TEG(TP), MOD(RM), 0x00) | DP >> 2)           \
        EMITW(0xF4200AAF | MRM(REG(RG), TEG(TP), 0x00))

#define movps_st(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE2800F00 | MRM(TEG(TP), MOD(RM), 0x00) | DP >> 2)           \
        EMITW(0xF4000AAF | MRM(REG(RG), TEG(TP), 0x00))


#define addps_rr(RG, RM)                                                    \
        EMITW(0xF2000D40 | MRM(REG(RG), REG(RG), REG(RM)))

#define addps_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE2800F00 | MRM(TEG(TP), MOD(RM), 0x00) | DP >> 2)           \
        EMITW(0xF4600AAF | MRM(TEG(T1), TEG(TP), 0x00))                     \
        EMITW(0xF2000D60 | MRM(REG(RG), REG(RG), TEG(T1)))


#define subps_rr(RG, RM)                                                    \
        EMITW(0xF2200D40 | MRM(REG(RG), REG(RG), REG(RM)))

#define subps_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE2800F00 | MRM(TEG(TP), MOD(RM), 0x00) | DP >> 2)           \
        EMITW(0xF4600AAF | MRM(TEG(T1), TEG(TP), 0x00))                     \
        EMITW(0xF2200D60 | MRM(REG(RG), REG(RG), TEG(T1)))


#define mulps_rr(RG, RM)                                                    \
        EMITW(0xF3000D50 | MRM(REG(RG), REG(RG), REG(RM)))

#define mulps_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE2800F00 | MRM(TEG(TP), MOD(RM), 0x00) | DP >> 2)           \
        EMITW(0xF4600AAF | MRM(TEG(T1), TEG(TP), 0x00))                     \
        EMITW(0xF3000D70 | MRM(REG(RG), REG(RG), TEG(T1)))


#if RT_USE_VFP

#define divps_rr(RG, RM)                                                    \
        EMITW(0xEE800A00 | MRM(REG(RG)+0, REG(RG)+0, REG(RM)+0))            \
        EMITW(0xEEC00AA0 | MRM(REG(RG)+0, REG(RG)+0, REG(RM)+0))            \
        EMITW(0xEE800A00 | MRM(REG(RG)+1, REG(RG)+1, REG(RM)+1))            \
        EMITW(0xEEC00AA0 | MRM(REG(RG)+1, REG(RG)+1, REG(RM)+1))

#define divps_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE2800F00 | MRM(TEG(TP), MOD(RM), 0x00) | DP >> 2)           \
        EMITW(0xF4600AAF | MRM(TEG(T1), TEG(TP), 0x00))                     \
        EMITW(0xF3FB0560 | MRM(TEG(T2), 0x00, TEG(T1)))                     \
        EMITW(0xF2400FF0 | MRM(TEG(T3), TEG(T2), TEG(T1)))                  \
        EMITW(0xF3400DF0 | MRM(TEG(T2), TEG(T3), TEG(T2)))                  \
        EMITW(0xF3000D70 | MRM(REG(RG), REG(RG), TEG(T2)))

#else /* RT_USE_VFP */

#define divps_rr(RG, RM)                                                    \
        EMITW(0xF3FB0540 | MRM(TEG(T1), 0x00, REG(RM)))                     \
        EMITW(0xF2400FD0 | MRM(TEG(T2), TEG(T1), REG(RM)))                  \
        EMITW(0xF3400DF0 | MRM(TEG(T1), TEG(T2), TEG(T1)))                  \
        EMITW(0xF3000D70 | MRM(REG(RG), REG(RG), TEG(T1)))

#define divps_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE2800F00 | MRM(TEG(TP), MOD(RM), 0x00) | DP >> 2)           \
        EMITW(0xF4600AAF | MRM(TEG(T1), TEG(TP), 0x00))                     \
        EMITW(0xF3FB0560 | MRM(TEG(T2), 0x00, TEG(T1)))                     \
        EMITW(0xF2400FF0 | MRM(TEG(T3), TEG(T2), TEG(T1)))                  \
        EMITW(0xF3400DF0 | MRM(TEG(T2), TEG(T3), TEG(T2)))                  \
        EMITW(0xF3000D70 | MRM(REG(RG), REG(RG), TEG(T2)))

#endif /* RT_USE_VFP */


#define andps_rr(RG, RM)                                                    \
        EMITW(0xF2000150 | MRM(REG(RG), REG(RG), REG(RM)))                  \

#define andps_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE2800F00 | MRM(TEG(TP), MOD(RM), 0x00) | DP >> 2)           \
        EMITW(0xF4600AAF | MRM(TEG(T1), TEG(TP), 0x00))                     \
        EMITW(0xF2000170 | MRM(REG(RG), REG(RG), TEG(T1)))


#define annps_rr(RG, RM)                                                    \
        EMITW(0xF2100150 | MRM(REG(RG), REG(RM), REG(RG)))                  \

#define annps_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE2800F00 | MRM(TEG(TP), MOD(RM), 0x00) | DP >> 2)           \
        EMITW(0xF4600AAF | MRM(TEG(T1), TEG(TP), 0x00))                     \
        EMITW(0xF21001D0 | MRM(REG(RG), TEG(T1), REG(RG)))


#define orrps_rr(RG, RM)                                                    \
        EMITW(0xF2200150 | MRM(REG(RG), REG(RG), REG(RM)))                  \

#define orrps_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE2800F00 | MRM(TEG(TP), MOD(RM), 0x00) | DP >> 2)           \
        EMITW(0xF4600AAF | MRM(TEG(T1), TEG(TP), 0x00))                     \
        EMITW(0xF2200170 | MRM(REG(RG), REG(RG), TEG(T1)))


#define xorps_rr(RG, RM)                                                    \
        EMITW(0xF3000150 | MRM(REG(RG), REG(RG), REG(RM)))                  \

#define xorps_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE2800F00 | MRM(TEG(TP), MOD(RM), 0x00) | DP >> 2)           \
        EMITW(0xF4600AAF | MRM(TEG(T1), TEG(TP), 0x00))                     \
        EMITW(0xF3000170 | MRM(REG(RG), REG(RG), TEG(T1)))

#endif /* RT_RTARCH_ARM_MPE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
