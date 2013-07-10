/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_ARM_MPE_H
#define RT_RTARCH_ARM_MPE_H

#include "rtarch_arm.h"

/* registers    REG,  MOD,  SIB */

#define Xmm0    0x00, 0x00, EMPTY       /* q0 */
#define Xmm1    0x02, 0x00, EMPTY       /* q1 */
#define Xmm2    0x04, 0x00, EMPTY       /* q2 */
#define Xmm3    0x06, 0x00, EMPTY       /* q3 */
#define Xmm4    0x08, 0x00, EMPTY       /* q4 */
#define Xmm5    0x0A, 0x00, EMPTY       /* q5 */
#define Xmm6    0x0C, 0x00, EMPTY       /* q6 */
#define Xmm7    0x0E, 0x00, EMPTY       /* q7 */

#define T0xx    0x00                    /* s0, for integer div VFP fallback */
#define T1xx    0x00                    /* q8 */
#define T2xx    0x02                    /* q9 */
#define T3xx    0x04                    /* q10 */

#define RT_USE_VFP          0           /* use VFP (slow) for div, sqr, rsq */

/******************************************************************************/
/**********************************   MPE   ***********************************/
/******************************************************************************/

/* fpu */

#define movps_rr(RG, RM)                                                    \
        EMITW(0xF2200150 | MRM(REG(RG), REG(RM), REG(RM)))

#define movps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE2800E00 | MRM(TPxx,    MOD(RM), 0x00) | DP >> 4)           \
        EMITW(0xF4200AAF | MRM(REG(RG), TPxx,    0x00))

#define movps_st(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE2800E00 | MRM(TPxx,    MOD(RM), 0x00) | DP >> 4)           \
        EMITW(0xF4000AAF | MRM(REG(RG), TPxx,    0x00))


#define addps_rr(RG, RM)                                                    \
        EMITW(0xF2000D40 | MRM(REG(RG), REG(RG), REG(RM)))

#define addps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE2800E00 | MRM(TPxx,    MOD(RM), 0x00) | DP >> 4)           \
        EMITW(0xF4600AAF | MRM(T1xx,    TPxx,    0x00))                     \
        EMITW(0xF2000D60 | MRM(REG(RG), REG(RG), T1xx))


#define subps_rr(RG, RM)                                                    \
        EMITW(0xF2200D40 | MRM(REG(RG), REG(RG), REG(RM)))

#define subps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE2800E00 | MRM(TPxx,    MOD(RM), 0x00) | DP >> 4)           \
        EMITW(0xF4600AAF | MRM(T1xx,    TPxx,    0x00))                     \
        EMITW(0xF2200D60 | MRM(REG(RG), REG(RG), T1xx))


#define mulps_rr(RG, RM)                                                    \
        EMITW(0xF3000D50 | MRM(REG(RG), REG(RG), REG(RM)))

#define mulps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE2800E00 | MRM(TPxx,    MOD(RM), 0x00) | DP >> 4)           \
        EMITW(0xF4600AAF | MRM(T1xx,    TPxx,    0x00))                     \
        EMITW(0xF3000D70 | MRM(REG(RG), REG(RG), T1xx))


#if RT_USE_VFP

#define divps_rr(RG, RM)                                                    \
        EMITW(0xEE800A00 | MRM(REG(RG)+0, REG(RG)+0, REG(RM)+0))            \
        EMITW(0xEEC00AA0 | MRM(REG(RG)+0, REG(RG)+0, REG(RM)+0))            \
        EMITW(0xEE800A00 | MRM(REG(RG)+1, REG(RG)+1, REG(RM)+1))            \
        EMITW(0xEEC00AA0 | MRM(REG(RG)+1, REG(RG)+1, REG(RM)+1))

#define divps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE2800E00 | MRM(TPxx,    MOD(RM), 0x00) | DP >> 4)           \
        EMITW(0xF4600AAF | MRM(T1xx,    TPxx,    0x00))                     \
        EMITW(0xF3FB0560 | MRM(T2xx,    0x00,    T1xx))                     \
        EMITW(0xF2400FF0 | MRM(T3xx,    T2xx,    T1xx))                     \
        EMITW(0xF3400DF0 | MRM(T2xx,    T3xx,    T2xx))                     \
        EMITW(0xF3000D70 | MRM(REG(RG), REG(RG), T2xx))


#define sqrps_rr(RG, RM)                                                    \
        EMITW(0xEEB10AC0 | MRM(REG(RG)+0, 0x00,  REG(RM)+0))                \
        EMITW(0xEEF10AE0 | MRM(REG(RG)+0, 0x00,  REG(RM)+0))                \
        EMITW(0xEEB10AC0 | MRM(REG(RG)+1, 0x00,  REG(RM)+1))                \
        EMITW(0xEEF10AE0 | MRM(REG(RG)+1, 0x00,  REG(RM)+1))

#define rsqps_rr(RG, RT, RM)                                                \
        EMITW(0xEEB10AC0 | MRM(REG(RT)+0, 0x00,  REG(RM)+0))                \
        EMITW(0xEEF10AE0 | MRM(REG(RT)+0, 0x00,  REG(RM)+0))                \
        EMITW(0xEEB10AC0 | MRM(REG(RT)+1, 0x00,  REG(RM)+1))                \
        EMITW(0xEEF10AE0 | MRM(REG(RT)+1, 0x00,  REG(RM)+1))                \
        EMITW(0xEE800A00 | MRM(REG(RG)+0, REG(RG)+0, REG(RT)+0))            \
        EMITW(0xEEC00AA0 | MRM(REG(RG)+0, REG(RG)+0, REG(RT)+0))            \
        EMITW(0xEE800A00 | MRM(REG(RG)+1, REG(RG)+1, REG(RT)+1))            \
        EMITW(0xEEC00AA0 | MRM(REG(RG)+1, REG(RG)+1, REG(RT)+1))

#else /* RT_USE_VFP */

#define divps_rr(RG, RM)                                                    \
        EMITW(0xF3FB0540 | MRM(T1xx,    0x00,    REG(RM)))                  \
        EMITW(0xF2400FD0 | MRM(T2xx,    T1xx,    REG(RM)))                  \
        EMITW(0xF3400DF0 | MRM(T1xx,    T2xx,    T1xx))                     \
        EMITW(0xF3000D70 | MRM(REG(RG), REG(RG), T1xx))

#define divps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE2800E00 | MRM(TPxx,    MOD(RM), 0x00) | DP >> 4)           \
        EMITW(0xF4600AAF | MRM(T1xx,    TPxx,    0x00))                     \
        EMITW(0xF3FB0560 | MRM(T2xx,    0x00,    T1xx))                     \
        EMITW(0xF2400FF0 | MRM(T3xx,    T2xx,    T1xx))                     \
        EMITW(0xF3400DF0 | MRM(T2xx,    T3xx,    T2xx))                     \
        EMITW(0xF3000D70 | MRM(REG(RG), REG(RG), T2xx))


#define sqrps_rr(RG, RM)                                                    \
        EMITW(0xF3FB05C0 | MRM(T1xx,    0x00,    REG(RM)))                  \
        EMITW(0xF3400DF0 | MRM(T2xx,    T1xx,    T1xx))                     \
        EMITW(0xF2600FD0 | MRM(T2xx,    T2xx,    REG(RM)))                  \
        EMITW(0xF3400DF0 | MRM(T1xx,    T2xx,    T1xx))                     \
        EMITW(0xF3FB0560 | MRM(T2xx,    0x00,    T1xx))                     \
        EMITW(0xF2400FF0 | MRM(T3xx,    T2xx,    T1xx))                     \
        EMITW(0xF3000DF0 | MRM(REG(RG), T3xx,    T2xx))

#define rsqps_rr(RG, RT, RM)                                                \
        EMITW(0xF3FB05C0 | MRM(T1xx,    0x00,    REG(RM)))                  \
        EMITW(0xF3400DF0 | MRM(T2xx,    T1xx,    T1xx))                     \
        EMITW(0xF2600FD0 | MRM(T2xx,    T2xx,    REG(RM)))                  \
        EMITW(0xF3400DF0 | MRM(T1xx,    T2xx,    T1xx))                     \
        EMITW(0xF3000D70 | MRM(REG(RG), REG(RG), T1xx))

#endif /* RT_USE_VFP */


#define andps_rr(RG, RM)                                                    \
        EMITW(0xF2000150 | MRM(REG(RG), REG(RG), REG(RM)))                  \

#define andps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE2800E00 | MRM(TPxx,    MOD(RM), 0x00) | DP >> 4)           \
        EMITW(0xF4600AAF | MRM(T1xx,    TPxx,    0x00))                     \
        EMITW(0xF2000170 | MRM(REG(RG), REG(RG), T1xx))


#define annps_rr(RG, RM)                                                    \
        EMITW(0xF2100150 | MRM(REG(RG), REG(RM), REG(RG)))                  \

#define annps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE2800E00 | MRM(TPxx,    MOD(RM), 0x00) | DP >> 4)           \
        EMITW(0xF4600AAF | MRM(T1xx,    TPxx,    0x00))                     \
        EMITW(0xF21001D0 | MRM(REG(RG), T1xx,    REG(RG)))


#define orrps_rr(RG, RM)                                                    \
        EMITW(0xF2200150 | MRM(REG(RG), REG(RG), REG(RM)))                  \

#define orrps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE2800E00 | MRM(TPxx,    MOD(RM), 0x00) | DP >> 4)           \
        EMITW(0xF4600AAF | MRM(T1xx,    TPxx,    0x00))                     \
        EMITW(0xF2200170 | MRM(REG(RG), REG(RG), T1xx))


#define xorps_rr(RG, RM)                                                    \
        EMITW(0xF3000150 | MRM(REG(RG), REG(RG), REG(RM)))                  \

#define xorps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE2800E00 | MRM(TPxx,    MOD(RM), 0x00) | DP >> 4)           \
        EMITW(0xF4600AAF | MRM(T1xx,    TPxx,    0x00))                     \
        EMITW(0xF3000170 | MRM(REG(RG), REG(RG), T1xx))


#define minps_rr(RG, RM)                                                    \
        EMITW(0xF2200F40 | MRM(REG(RG), REG(RG), REG(RM)))                  \

#define minps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE2800E00 | MRM(TPxx,    MOD(RM), 0x00) | DP >> 4)           \
        EMITW(0xF4600AAF | MRM(T1xx,    TPxx,    0x00))                     \
        EMITW(0xF2200F60 | MRM(REG(RG), REG(RG), T1xx))


#define maxps_rr(RG, RM)                                                    \
        EMITW(0xF2000F40 | MRM(REG(RG), REG(RG), REG(RM)))                  \

#define maxps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE2800E00 | MRM(TPxx,    MOD(RM), 0x00) | DP >> 4)           \
        EMITW(0xF4600AAF | MRM(T1xx,    TPxx,    0x00))                     \
        EMITW(0xF2000F60 | MRM(REG(RG), REG(RG), T1xx))


#define ceqps_rr(RG, RM)                                                    \
        EMITW(0xF2000E40 | MRM(REG(RG), REG(RG), REG(RM)))

#define cltps_rr(RG, RM)                                                    \
        EMITW(0xF3200E40 | MRM(REG(RG), REG(RM), REG(RG)))

#define cleps_rr(RG, RM)                                                    \
        EMITW(0xF3000E40 | MRM(REG(RG), REG(RM), REG(RG)))

#define cneps_rr(RG, RM)                                                    \
        EMITW(0xF2000E40 | MRM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0xF3B005C0 | MRM(REG(RG), 0x00,    REG(RG)))

#define cgeps_rr(RG, RM)                                                    \
        EMITW(0xF3000E40 | MRM(REG(RG), REG(RG), REG(RM)))

#define cgtps_rr(RG, RM)                                                    \
        EMITW(0xF3200E40 | MRM(REG(RG), REG(RG), REG(RM)))


#define movms_rr(RG, RM)                                                    \
        EMITW(0xF3F60200 | MRM(T1xx+0,  0x00,    REG(RM)))                  \
        EMITW(0xF3F20220 | MRM(T1xx+0,  0x00,    T1xx))                     \
        EMITW(0xEE100B90 | MRM(REG(RG), T1xx+0,  0x00))

#define NONE  0x00
#define FULL  0x01

#define CHECK_MASK(lb, cc)                                                  \
        movms_rr(Reax, Xmm7)                                                \
        addxx_ri(Reax, IB(cc))                                              \
        cmpxx_ri(Reax, IB(0))                                               \
        jeqxx_lb(lb)


#define fpscr_ld(RG)                                                        \
        EMITW(0xEEE10A10 | MRM(REG(RG), 0x00,    0x00))

#define fpscr_st(RG)                                                        \
        EMITW(0xEEF10A10 | MRM(REG(RG), 0x00,    0x00))

#define FCTRL_ENTER()                                                       \
        fpscr_st(Reax)                                                      \
        movxx_st(Reax, Mebp, inf_FCTRL)                                     \
        movxx_ri(Redx, IW(1 << 23))                                         \
        orrxx_rr(Reax, Redx)                                                \
        fpscr_ld(Reax)

#define FCTRL_LEAVE()                                                       \
        movxx_ld(Reax, Mebp, inf_FCTRL)                                     \
        fpscr_ld(Reax)

/* int */

#define cvtpn_rr(RG, RM)                                                    \
        EMITW(0xF3BB0640 | MRM(REG(RG),   0x00,  REG(RM)))

#define cvtps_rr(RG, RM) /* fallback to VFP for float-to-integer cvt */     \
        EMITW(0xEEBD0A40 | MRM(REG(RG)+0, 0x00,  REG(RM)+0))                \
        EMITW(0xEEFD0A60 | MRM(REG(RG)+0, 0x00,  REG(RM)+0))                \
        EMITW(0xEEBD0A40 | MRM(REG(RG)+1, 0x00,  REG(RM)+1))                \
        EMITW(0xEEFD0A60 | MRM(REG(RG)+1, 0x00,  REG(RM)+1))


#define addpn_rr(RG, RM)                                                    \
        EMITW(0xF2200840 | MRM(REG(RG), REG(RG), REG(RM)))                  \

#define addpn_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE2800E00 | MRM(TPxx,    MOD(RM), 0x00) | DP >> 4)           \
        EMITW(0xF4600AAF | MRM(T1xx,    TPxx,    0x00))                     \
        EMITW(0xF2200860 | MRM(REG(RG), REG(RG), T1xx))


#define shlpn_ri(RM, IM)                                                    \
        EMITW(0xF2A00550 | MRM(REG(RM), 0x00,    REG(RM)) |                 \
                                        (0x1F & VAL(IM)) << 16)

#define shlpn_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE2800E00 | MRM(TPxx,    MOD(RM), 0x00) | DP >> 4)           \
        EMITW(0xF4E00CBF | MRM(T1xx,    TPxx,    0x00))                     \
        EMITW(0xF32004C0 | MRM(REG(RG), T1xx,    REG(RG)))


#define shrpn_ri(RM, IM)                                                    \
        EMITW(0xE3A00000 | MRM(TIxx,    0x00,    0x00) |                    \
                                        (0xFF & VAL(IM)))                   \
        EMITW(0xEEA00B90 | MRM(TIxx,    T1xx,    0x00))                     \
        EMITW(0xF3F903E0 | MRM(T1xx,    0x00,    T1xx))                     \
        EMITW(0xF32004C0 | MRM(REG(RM), T1xx,    REG(RM)))

#define shrpn_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE2800E00 | MRM(TPxx,    MOD(RM), 0x00) | DP >> 4)           \
        EMITW(0xF4E00CBF | MRM(T1xx,    TPxx,    0x00))                     \
        EMITW(0xF3F903E0 | MRM(T1xx,    0x00,    T1xx))                     \
        EMITW(0xF32004C0 | MRM(REG(RG), T1xx,    REG(RG)))

#endif /* RT_RTARCH_ARM_MPE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
