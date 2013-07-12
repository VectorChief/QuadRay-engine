/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_ARM_MPE_H
#define RT_RTARCH_ARM_MPE_H

#include "rtarch_arm.h"

#define MTM(reg, ren, rem)                                                  \
        (((rem) & 0x0F) <<  0 | ((rem) & 0x10) <<  1 |                      \
         ((ren) & 0x0F) << 16 | ((ren) & 0x10) <<  3 |                      \
         ((reg) & 0x0F) << 12 | ((reg) & 0x10) << 18 )

/* registers    REG,  MOD,  SIB */

#define Xmm0    0x00, 0x00, EMPTY       /* q0 */
#define Xmm1    0x02, 0x00, EMPTY       /* q1 */
#define Xmm2    0x04, 0x00, EMPTY       /* q2 */
#define Xmm3    0x06, 0x00, EMPTY       /* q3 */
#define Xmm4    0x08, 0x00, EMPTY       /* q4 */
#define Xmm5    0x0A, 0x00, EMPTY       /* q5 */
#define Xmm6    0x0C, 0x00, EMPTY       /* q6 */
#define Xmm7    0x0E, 0x00, EMPTY       /* q7 */

#define Tmm0    0x00                    /* q0, for integer div VFP fallback */
#define Tmm1    0x10                    /* q8 */
#define Tmm2    0x12                    /* q9 */
#define Tmm3    0x14                    /* q10 */

/******************************************************************************/
/**********************************   MPE   ***********************************/
/******************************************************************************/

/* fpu */

#define movps_rr(RG, RM)                                                    \
        EMITW(0xF2200150 | MTM(REG(RG), REG(RM), REG(RM)))

#define movps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) |                    \
                           TYP(DP))                                         \
        EMITW(0xF4200AAF | MTM(REG(RG), TPxx,    0x00))

#define movps_st(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) |                    \
                           TYP(DP))                                         \
        EMITW(0xF4000AAF | MTM(REG(RG), TPxx,    0x00))

#define adrpx_ld(RG, RM, DP) /* only for quads (16-byte alignment) */       \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MTM(REG(RG), MOD(RM), 0x00) |                    \
                           TYP(DP))


#define addps_rr(RG, RM)                                                    \
        EMITW(0xF2000D40 | MTM(REG(RG), REG(RG), REG(RM)))

#define addps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) |                    \
                           TYP(DP))                                         \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2000D40 | MTM(REG(RG), REG(RG), Tmm1))


#define subps_rr(RG, RM)                                                    \
        EMITW(0xF2200D40 | MTM(REG(RG), REG(RG), REG(RM)))

#define subps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) |                    \
                           TYP(DP))                                         \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2200D40 | MTM(REG(RG), REG(RG), Tmm1))


#define mulps_rr(RG, RM)                                                    \
        EMITW(0xF3000D50 | MTM(REG(RG), REG(RG), REG(RM)))

#define mulps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) |                    \
                           TYP(DP))                                         \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3000D50 | MTM(REG(RG), REG(RG), Tmm1))


#define divps_rr(RG, RM)                                                    \
        EMITW(0xF3BB0540 | MTM(Tmm1,    0x00,    REG(RM)))                  \
        EMITW(0xF2000F50 | MTM(Tmm2,    Tmm1,    REG(RM)))                  \
        EMITW(0xF3000D50 | MTM(Tmm1,    Tmm1,    Tmm2))                     \
        EMITW(0xF3000D50 | MTM(REG(RG), REG(RG), Tmm1))

#define divps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) |                    \
                           TYP(DP))                                         \
        EMITW(0xF4200AAF | MTM(Tmm3,    TPxx,    0x00))                     \
        EMITW(0xF3BB0540 | MTM(Tmm1,    0x00,    Tmm3))                     \
        EMITW(0xF2000F50 | MTM(Tmm2,    Tmm1,    Tmm3))                     \
        EMITW(0xF3000D50 | MTM(Tmm1,    Tmm1,    Tmm2))                     \
        EMITW(0xF3000D50 | MTM(REG(RG), REG(RG), Tmm1))


#define sqrps_rr(RG, RM)                                                    \
        EMITW(0xF3BB05C0 | MTM(Tmm1,    0x00,    REG(RM)))                  \
        EMITW(0xF3000D50 | MTM(Tmm2,    Tmm1,    Tmm1))                     \
        EMITW(0xF2200F50 | MTM(Tmm2,    Tmm2,    REG(RM)))                  \
        EMITW(0xF3000D50 | MTM(Tmm1,    Tmm1,    Tmm2))                     \
        EMITW(0xF3000D50 | MTM(REG(RG), REG(RM), Tmm1))

#define sqrps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) |                    \
                           TYP(DP))                                         \
        EMITW(0xF4200AAF | MTM(Tmm3,    TPxx,    0x00))                     \
        EMITW(0xF3BB05C0 | MTM(Tmm1,    0x00,    Tmm3))                     \
        EMITW(0xF3000D50 | MTM(Tmm2,    Tmm1,    Tmm1))                     \
        EMITW(0xF2200F50 | MTM(Tmm2,    Tmm2,    Tmm3))                     \
        EMITW(0xF3000D50 | MTM(Tmm1,    Tmm1,    Tmm2))                     \
        EMITW(0xF3000D50 | MTM(REG(RG), Tmm3,    Tmm1))


#define rcpps_rr(RG, RM)             /* destroys value in RM */             \
        EMITW(0xF3BB0540 | MTM(REG(RG), 0x00,    REG(RM)))                  \
        EMITW(0xF2000F50 | MTM(REG(RM), REG(RG), REG(RM)))                  \
        EMITW(0xF3000D50 | MTM(REG(RG), REG(RG), REG(RM)))

#define rsqps_rr(RG, RM, RD, D1, D2) /* destroys value in RM */             \
        EMITW(0xF3BB05C0 | MTM(REG(RG), 0x00,    REG(RM)))                  \
        EMITW(0xF3000D50 | MTM(REG(RM), REG(RM), REG(RG)))                  \
        EMITW(0xF2200F50 | MTM(REG(RM), REG(RM), REG(RG)))                  \
        EMITW(0xF3000D50 | MTM(REG(RG), REG(RG), REG(RM)))


#define andps_rr(RG, RM)                                                    \
        EMITW(0xF2000150 | MTM(REG(RG), REG(RG), REG(RM)))                  \

#define andps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) |                    \
                           TYP(DP))                                         \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2000150 | MTM(REG(RG), REG(RG), Tmm1))


#define annps_rr(RG, RM)                                                    \
        EMITW(0xF2100150 | MTM(REG(RG), REG(RM), REG(RG)))                  \

#define annps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) |                    \
                           TYP(DP))                                         \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2100150 | MTM(REG(RG), Tmm1,    REG(RG)))


#define orrps_rr(RG, RM)                                                    \
        EMITW(0xF2200150 | MTM(REG(RG), REG(RG), REG(RM)))                  \

#define orrps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) |                    \
                           TYP(DP))                                         \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2200150 | MTM(REG(RG), REG(RG), Tmm1))


#define xorps_rr(RG, RM)                                                    \
        EMITW(0xF3000150 | MTM(REG(RG), REG(RG), REG(RM)))                  \

#define xorps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) |                    \
                           TYP(DP))                                         \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3000150 | MTM(REG(RG), REG(RG), Tmm1))


#define minps_rr(RG, RM)                                                    \
        EMITW(0xF2200F40 | MTM(REG(RG), REG(RG), REG(RM)))                  \

#define minps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) |                    \
                           TYP(DP))                                         \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2200F40 | MTM(REG(RG), REG(RG), Tmm1))


#define maxps_rr(RG, RM)                                                    \
        EMITW(0xF2000F40 | MTM(REG(RG), REG(RG), REG(RM)))                  \

#define maxps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) |                    \
                           TYP(DP))                                         \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2000F40 | MTM(REG(RG), REG(RG), Tmm1))


#define ceqps_rr(RG, RM)                                                    \
        EMITW(0xF2000E40 | MTM(REG(RG), REG(RG), REG(RM)))

#define cltps_rr(RG, RM)                                                    \
        EMITW(0xF3200E40 | MTM(REG(RG), REG(RM), REG(RG)))

#define cleps_rr(RG, RM)                                                    \
        EMITW(0xF3000E40 | MTM(REG(RG), REG(RM), REG(RG)))

#define cneps_rr(RG, RM)                                                    \
        EMITW(0xF2000E40 | MTM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0xF3B005C0 | MTM(REG(RG), 0x00,    REG(RG)))

#define cgeps_rr(RG, RM)                                                    \
        EMITW(0xF3000E40 | MTM(REG(RG), REG(RG), REG(RM)))

#define cgtps_rr(RG, RM)                                                    \
        EMITW(0xF3200E40 | MTM(REG(RG), REG(RG), REG(RM)))


#define movms_rr(RG, RM)                                                    \
        EMITW(0xF3B60200 | MTM(Tmm1+0,  0x00,    REG(RM)))                  \
        EMITW(0xF3B20200 | MTM(Tmm1+0,  0x00,    Tmm1))                     \
        EMITW(0xEE100B10 | MTM(REG(RG), Tmm1+0,  0x00))

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
        EMITW(0xF3BB0640 | MTM(REG(RG),   0x00,  REG(RM)))

#define cvtps_rr(RG, RM) /* fallback to VFP for float-to-integer cvt */     \
        EMITW(0xEEBD0A40 | MTM(REG(RG)+0, 0x00,  REG(RM)+0))                \
        EMITW(0xEEFD0A60 | MTM(REG(RG)+0, 0x00,  REG(RM)+0))                \
        EMITW(0xEEBD0A40 | MTM(REG(RG)+1, 0x00,  REG(RM)+1))                \
        EMITW(0xEEFD0A60 | MTM(REG(RG)+1, 0x00,  REG(RM)+1))


#define addpn_rr(RG, RM)                                                    \
        EMITW(0xF2200840 | MTM(REG(RG), REG(RG), REG(RM)))                  \

#define addpn_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) |                    \
                           TYP(DP))                                         \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2200840 | MTM(REG(RG), REG(RG), Tmm1))


#define shlpn_ri(RM, IM)                                                    \
        EMITW(0xF2A00550 | MTM(REG(RM), 0x00,    REG(RM)) |                 \
             (0x0000001F & VAL(IM)) << 16)

#define shlpn_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) |                    \
                           TYP(DP))                                         \
        EMITW(0xF4A00CBF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3200440 | MTM(REG(RG), Tmm1,    REG(RG)))


#define shrpn_ri(RM, IM)                                                    \
        EMITW(0xE3A00000 | MRM(TIxx,    0x00,    0x00) |                    \
             (0x000000FF & VAL(IM)))                                        \
        EMITW(0xEEA00B10 | MTM(TIxx,    Tmm1,    0x00))                     \
        EMITW(0xF3B903C0 | MTM(Tmm1,    0x00,    Tmm1))                     \
        EMITW(0xF3200440 | MTM(REG(RM), Tmm1,    REG(RM)))

#define shrpn_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) |                    \
                           TYP(DP))                                         \
        EMITW(0xF4A00CBF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3B903C0 | MTM(Tmm1,    0x00,    Tmm1))                     \
        EMITW(0xF3200440 | MTM(REG(RG), Tmm1,    REG(RG)))

#endif /* RT_RTARCH_ARM_MPE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
