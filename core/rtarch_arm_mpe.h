/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_ARM_MPE_H
#define RT_RTARCH_ARM_MPE_H

#include "rtarch_arm.h"

/* registers    MOD,  REG,  SIB */

#define Xmm0    0x00, 0x00, EMPTY       /* q0 */
#define Xmm1    0x00, 0x02, EMPTY       /* q1 */
#define Xmm2    0x00, 0x04, EMPTY       /* q2 */
#define Xmm3    0x00, 0x06, EMPTY       /* q3 */
#define Xmm4    0x00, 0x08, EMPTY       /* q4 */
#define Xmm5    0x00, 0x0A, EMPTY       /* q5 */
#define Xmm6    0x00, 0x0C, EMPTY       /* q6 */
#define Xmm7    0x00, 0x0E, EMPTY       /* q7 */

#define T0      0x00, 0x00, EMPTY       /* s0, for integer div VFP fallback */
#define T1      0x01, 0x00, EMPTY       /* q8 */
#define T2      0x01, 0x02, EMPTY       /* q9 */
#define T3      0x01, 0x04, EMPTY       /* q10 */

#define RT_USE_VFP          0           /* use VFP (slow) for div, sqr, rsq */

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


#define sqrps_rr(RG, RM)                                                    \
        EMITW(0xEEB10AC0 | MRM(REG(RG)+0, 0x00, REG(RM)+0))                 \
        EMITW(0xEEF10AE0 | MRM(REG(RG)+0, 0x00, REG(RM)+0))                 \
        EMITW(0xEEB10AC0 | MRM(REG(RG)+1, 0x00, REG(RM)+1))                 \
        EMITW(0xEEF10AE0 | MRM(REG(RG)+1, 0x00, REG(RM)+1))

#define rsqps_rr(RG, RT, RM)                                                \
        EMITW(0xEEB10AC0 | MRM(REG(RT)+0, 0x00, REG(RM)+0))                 \
        EMITW(0xEEF10AE0 | MRM(REG(RT)+0, 0x00, REG(RM)+0))                 \
        EMITW(0xEEB10AC0 | MRM(REG(RT)+1, 0x00, REG(RM)+1))                 \
        EMITW(0xEEF10AE0 | MRM(REG(RT)+1, 0x00, REG(RM)+1))                 \
        EMITW(0xEE800A00 | MRM(REG(RG)+0, REG(RG)+0, REG(RT)+0))            \
        EMITW(0xEEC00AA0 | MRM(REG(RG)+0, REG(RG)+0, REG(RT)+0))            \
        EMITW(0xEE800A00 | MRM(REG(RG)+1, REG(RG)+1, REG(RT)+1))            \
        EMITW(0xEEC00AA0 | MRM(REG(RG)+1, REG(RG)+1, REG(RT)+1))

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


#define sqrps_rr(RG, RM)                                                    \
        EMITW(0xF3FB05C0 | MRM(TEG(T1), 0x00, REG(RM)))                     \
        EMITW(0xF3400DF0 | MRM(TEG(T2), TEG(T1), TEG(T1)))                  \
        EMITW(0xF2600FD0 | MRM(TEG(T2), TEG(T2), REG(RM)))                  \
        EMITW(0xF3400DF0 | MRM(TEG(T1), TEG(T2), TEG(T1)))                  \
        EMITW(0xF3FB0560 | MRM(TEG(T2), 0x00, TEG(T1)))                     \
        EMITW(0xF2400FF0 | MRM(TEG(T3), TEG(T2), TEG(T1)))                  \
        EMITW(0xF3000DF0 | MRM(REG(RG), TEG(T3), TEG(T2)))

#define rsqps_rr(RG, RT, RM)                                                \
        EMITW(0xF3FB05C0 | MRM(TEG(T1), 0x00, REG(RM)))                     \
        EMITW(0xF3400DF0 | MRM(TEG(T2), TEG(T1), TEG(T1)))                  \
        EMITW(0xF2600FD0 | MRM(TEG(T2), TEG(T2), REG(RM)))                  \
        EMITW(0xF3400DF0 | MRM(TEG(T1), TEG(T2), TEG(T1)))                  \
        EMITW(0xF3000D70 | MRM(REG(RG), REG(RG), TEG(T1)))

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


#define minps_rr(RG, RM)                                                    \
        EMITW(0xF2200F40 | MRM(REG(RG), REG(RG), REG(RM)))                  \

#define minps_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE2800F00 | MRM(TEG(TP), MOD(RM), 0x00) | DP >> 2)           \
        EMITW(0xF4600AAF | MRM(TEG(T1), TEG(TP), 0x00))                     \
        EMITW(0xF2200F60 | MRM(REG(RG), REG(RG), TEG(T1)))


#define maxps_rr(RG, RM)                                                    \
        EMITW(0xF2000F40 | MRM(REG(RG), REG(RG), REG(RM)))                  \

#define maxps_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE2800F00 | MRM(TEG(TP), MOD(RM), 0x00) | DP >> 2)           \
        EMITW(0xF4600AAF | MRM(TEG(T1), TEG(TP), 0x00))                     \
        EMITW(0xF2000F60 | MRM(REG(RG), REG(RG), TEG(T1)))


#define ceqps_rr(RG, RM)                                                    \
        EMITW(0xF2000E40 | MRM(REG(RG), REG(RG), REG(RM)))

#define cltps_rr(RG, RM)                                                    \
        EMITW(0xF3200E40 | MRM(REG(RG), REG(RM), REG(RG)))

#define cleps_rr(RG, RM)                                                    \
        EMITW(0xF3000E40 | MRM(REG(RG), REG(RM), REG(RG)))

#define cneps_rr(RG, RM)                                                    \
        EMITW(0xF2000E40 | MRM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0xF3B005C0 | MRM(REG(RG), 0x00, REG(RG)))

#define cgeps_rr(RG, RM)                                                    \
        EMITW(0xF3000E40 | MRM(REG(RG), REG(RG), REG(RM)))

#define cgtps_rr(RG, RM)                                                    \
        EMITW(0xF3200E40 | MRM(REG(RG), REG(RG), REG(RM)))


#define movms_rr(RG, RM)                                                    \
        EMITW(0xF3F60200 | MRM(TEG(T1)+0, 0x00, REG(RM)))                   \
        EMITW(0xF3F20220 | MRM(TEG(T1)+0, 0x00, TEG(T1)))                   \
        EMITW(0xEE100B90 | MRM(REG(RG), TEG(T1)+0, 0x00))

#define NONE  0x00
#define FULL  0x01

#define CHECK_MASK(lb, cc)                                                  \
        movms_rr(Reax, Xmm7)                                                \
        addxx_ri(Reax, IH(cc))                                              \
        cmpxx_ri(Reax, IH(0))                                               \
        jeqxx_lb(lb)


#define fpscr_ld(RG)                                                        \
        EMITW(0xEEE10A10 | MRM(REG(RG), 0x00, 0x00))

#define fpscr_st(RG)                                                        \
        EMITW(0xEEF10A10 | MRM(REG(RG), 0x00, 0x00))

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
        EMITW(0xF3BB0640 | MRM(REG(RG), 0x00, REG(RM)))

#define cvtps_rr(RG, RM) /* fallback to VFP for float-to-integer cvt */     \
        EMITW(0xEEBD0A40 | MRM(REG(RG)+0, 0x00, REG(RM)+0))                 \
        EMITW(0xEEFD0A60 | MRM(REG(RG)+0, 0x00, REG(RM)+0))                 \
        EMITW(0xEEBD0A40 | MRM(REG(RG)+1, 0x00, REG(RM)+1))                 \
        EMITW(0xEEFD0A60 | MRM(REG(RG)+1, 0x00, REG(RM)+1))


#define addpn_rr(RG, RM)                                                    \
        EMITW(0xF2200840 | MRM(REG(RG), REG(RG), REG(RM)))                  \

#define addpn_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE2800F00 | MRM(TEG(TP), MOD(RM), 0x00) | DP >> 2)           \
        EMITW(0xF4600AAF | MRM(TEG(T1), TEG(TP), 0x00))                     \
        EMITW(0xF2200860 | MRM(REG(RG), REG(RG), TEG(T1)))


#define shlpn_ri(RM, IB)                                                    \
        EMITW(0xF2A00550 | MRM(REG(RM), 0x00, REG(RM)) | (0x1F & IB) << 16)

#define shlpn_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE2800F00 | MRM(TEG(TP), MOD(RM), 0x00) | DP >> 2)           \
        EMITW(0xF4E00CBF | MRM(TEG(T1), TEG(TP), 0x00))                     \
        EMITW(0xF32004C0 | MRM(REG(RG), TEG(T1), REG(RG)))


#define shrpn_ri(RM, IB)                                                    \
        EMITW(0xE3A00000 | MRM(TEG(TI), 0x00, 0x00) | (0xFF & IB))          \
        EMITW(0xEEA00B90 | MRM(TEG(TI), TEG(T1), 0x00))                     \
        EMITW(0xF3F903E0 | MRM(TEG(T1), 0x00, TEG(T1)))                     \
        EMITW(0xF32004C0 | MRM(REG(RM), TEG(T1), REG(RM)))

#define shrpn_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE2800F00 | MRM(TEG(TP), MOD(RM), 0x00) | DP >> 2)           \
        EMITW(0xF4E00CBF | MRM(TEG(T1), TEG(TP), 0x00))                     \
        EMITW(0xF3F903E0 | MRM(TEG(T1), 0x00, TEG(T1)))                     \
        EMITW(0xF32004C0 | MRM(REG(RG), TEG(T1), REG(RG)))

#endif /* RT_RTARCH_ARM_MPE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
