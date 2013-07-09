/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_ARM_H
#define RT_RTARCH_ARM_H

#define EMPTY   ASM_BEG ASM_END

#define EMITW(w) /* little endian */                                        \
        EMITB((w) >> 0x00 & 0xFF)                                           \
        EMITB((w) >> 0x08 & 0xFF)                                           \
        EMITB((w) >> 0x10 & 0xFF)                                           \
        EMITB((w) >> 0x18 & 0xFF)

#define MRM(reg, ren, rem)                                                  \
        ((ren) << 16 | (reg) << 12 | (rem))

#define MOD(mod, reg, sib)  mod
#define REG(mod, reg, sib)  reg
#define SIB(mod, reg, sib)  sib

#define TEG(TG)             REG(TG)

/* registers    MOD,  REG,  SIB */

#define Reax    0x00, 0x00, EMPTY       /* r0 */
#define Recx    0x00, 0x01, EMPTY       /* r1 */
#define Redx    0x00, 0x02, EMPTY       /* r2 */
#define Rebx    0x00, 0x03, EMPTY       /* r3 */
#define Resp    0x00, 0x04, EMPTY       /* r4 */
#define Rebp    0x00, 0x05, EMPTY       /* r5 */
#define Resi    0x00, 0x06, EMPTY       /* r6 */
#define Redi    0x00, 0x07, EMPTY       /* r7 */

#define TM      0x00, 0x08, EMPTY       /* r8 */
#define TI      0x00, 0x09, EMPTY       /* r8 */
#define TP      0x00, 0x0A, EMPTY       /* r10 */
#define PC      0x00, 0x0F, EMPTY       /* r15 */

/* addressing   MOD,  REG,  SIB */

#define Oeax    0x00, 0x00, EMPTY       /* [r0] */

#define Mecx    0x01, 0x01, EMPTY       /* [r1, DP] */
#define Medx    0x02, 0x02, EMPTY       /* [r2, DP] */
#define Mebx    0x03, 0x03, EMPTY       /* [r3, DP] */
#define Mebp    0x05, 0x05, EMPTY       /* [r5, DP] */
#define Mesi    0x06, 0x06, EMPTY       /* [r6, DP] */
#define Medi    0x07, 0x07, EMPTY       /* [r7, DP] */

#define Iecx    0x0A, 0x01, EMITW(0xE0800000 | MRM(0x0A, 0x01, 0x00))
#define Iedx    0x0A, 0x02, EMITW(0xE0800000 | MRM(0x0A, 0x02, 0x00))
#define Iebx    0x0A, 0x03, EMITW(0xE0800000 | MRM(0x0A, 0x03, 0x00))
#define Iebp    0x0A, 0x05, EMITW(0xE0800000 | MRM(0x0A, 0x05, 0x00))
#define Iesi    0x0A, 0x06, EMITW(0xE0800000 | MRM(0x0A, 0x06, 0x00))
#define Iedi    0x0A, 0x07, EMITW(0xE0800000 | MRM(0x0A, 0x07, 0x00))

/* immediate */

#define IB(im)  ((im) & 0xFF)           /* not compatible with IM param */

#define IH(im)  EMITW(0xE3000000 | MRM(TEG(TI), 0x0, 0x0) |                 \
                     (0x000F0000 & (im) <<  4) | (0xFFF & (im)))

#define IW(im)  EMITW(0xE3000000 | MRM(TEG(TI), 0x0, 0x0) |                 \
                     (0x000F0000 & (im) <<  4) | (0xFFF & (im)))            \
                EMITW(0xE3400000 | MRM(TEG(TI), 0x0, 0x0) |                 \
                     (0x000F0000 & (im) >> 12) | (0xFFF & (im) >> 16))

#define DP(im)  ((im) & 0xFFF)          /* not compatible with IM param */

#define PLAIN   DP(0)

/******************************************************************************/
/**********************************   ARM   ***********************************/
/******************************************************************************/

/* mov */

#define movxx_ri(RM, IM)                                                    \
        IM                                                                  \
        EMITW(0xE1A00000 | MRM(REG(RM), 0x00, TEG(TI)))

#define movxx_mi(RM, DP, IM)                                                \
        SIB(RM) IM                                                          \
        EMITW(0xE5800000 | MRM(TEG(TI), MOD(RM), 0x00) | DP)

#define movxx_rr(RG, RM)                                                    \
        EMITW(0xE1A00000 | MRM(REG(RG), 0x00, REG(RM)))

#define movxx_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE5900000 | MRM(REG(RG), MOD(RM), 0x00) | DP)

#define movxx_st(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE5800000 | MRM(REG(RG), MOD(RM), 0x00) | DP)

#define leaxx_ld(RG, RM, DP) /* only for quads (16-byte alignment) */       \
        SIB(RM)                                                             \
        EMITW(0xE2800E00 | MRM(REG(RG), MOD(RM), 0x00) | DP >> 4)

#define stack_sa()                                                          \
        EMITW(0xE92D07FF)

#define stack_la()                                                          \
        EMITW(0xE8BD07FF)

/* add */

#define addxx_ri(RM, IM)                                                    \
        IM                                                                  \
        EMITW(0xE0800000 | MRM(REG(RM), REG(RM), TEG(TI)))

#define addxx_mi(RM, DP, IM)                                                \
        SIB(RM) IM                                                          \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE0800000 | MRM(TEG(TM), TEG(TM), TEG(TI)))                  \
        EMITW(0xE5800000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)

#define addxx_rr(RG, RM)                                                    \
        EMITW(0xE0800000 | MRM(REG(RG), REG(RG), REG(RM)))

#define addxx_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE0800000 | MRM(REG(RG), REG(RG), TEG(TM)))

#define addxx_st(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE0800000 | MRM(TEG(TM), TEG(TM), REG(RG)))                  \
        EMITW(0xE5800000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)

/* sub */

#define subxx_ri(RM, IM)                                                    \
        IM                                                                  \
        EMITW(0xE0400000 | MRM(REG(RM), REG(RM), TEG(TI)))

#define subxx_mi(RM, DP, IM)                                                \
        SIB(RM) IM                                                          \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE0400000 | MRM(TEG(TM), TEG(TM), TEG(TI)))                  \
        EMITW(0xE5800000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)

#define subxx_rr(RG, RM)                                                    \
        EMITW(0xE0400000 | MRM(REG(RG), REG(RG), REG(RM)))

#define subxx_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE0400000 | MRM(REG(RG), REG(RG), TEG(TM)))

#define subxx_st(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE0400000 | MRM(TEG(TM), TEG(TM), REG(RG)))                  \
        EMITW(0xE5800000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)

/* and */

#define andxx_ri(RM, IM)                                                    \
        IM                                                                  \
        EMITW(0xE0000000 | MRM(REG(RM), REG(RM), TEG(TI)))

#define andxx_mi(RM, DP, IM)                                                \
        SIB(RM) IM                                                          \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE0000000 | MRM(TEG(TM), TEG(TM), TEG(TI)))                  \
        EMITW(0xE5800000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)

#define andxx_rr(RG, RM)                                                    \
        EMITW(0xE0000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define andxx_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE0000000 | MRM(REG(RG), REG(RG), TEG(TM)))

#define andxx_st(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE0000000 | MRM(TEG(TM), TEG(TM), REG(RG)))                  \
        EMITW(0xE5800000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)

/* orr */

#define orrxx_ri(RM, IM)                                                    \
        IM                                                                  \
        EMITW(0xE1800000 | MRM(REG(RM), REG(RM), TEG(TI)))

#define orrxx_mi(RM, DP, IM)                                                \
        SIB(RM) IM                                                          \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE1800000 | MRM(TEG(TM), TEG(TM), TEG(TI)))                  \
        EMITW(0xE5800000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)

#define orrxx_rr(RG, RM)                                                    \
        EMITW(0xE1800000 | MRM(REG(RG), REG(RG), REG(RM)))

#define orrxx_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE1800000 | MRM(REG(RG), REG(RG), TEG(TM)))

#define orrxx_st(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE1800000 | MRM(TEG(TM), TEG(TM), REG(RG)))                  \
        EMITW(0xE5800000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)

/* not */

#define notxx_rr(RM)                                                        \
        EMITW(0xE1E00000 | MRM(REG(RM), 0x00, REG(RM)))

#define notxx_mm(RM, DP)                                                    \
        SIB(RM)                                                             \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE1E00000 | MRM(TEG(TM), 0x00, TEG(TM)))                     \
        EMITW(0xE5800000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)

/* shl */

#define shlxx_ri(RM, IB)                                                    \
        EMITW(0xE1A00000 | MRM(REG(RM), 0x00, REG(RM)) | (0x1F & IB) << 7)

#define shlxx_mi(RM, DP, IB)                                                \
        SIB(RM)                                                             \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE1A00000 | MRM(REG(RM), 0x00, REG(RM)) | (0x1F & IB) << 7)  \
        EMITW(0xE5800000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)

#define mulxx_ld(RH, RL, RM, DP)                                            \
        SIB(RM)                                                             \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE0000090 | MRM(0x00, REG(RL), REG(RL)) | TEG(TM) << 8)

/* shr */

#define shrxx_ri(RM, IB)                                                    \
        EMITW(0xE1A00020 | MRM(REG(RM), 0x00, REG(RM)) | (0x1F & IB) << 7)

#define shrxx_mi(RM, DP, IB)                                                \
        SIB(RM)                                                             \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE1A00020 | MRM(REG(RM), 0x00, REG(RM)) | (0x1F & IB) << 7)  \
        EMITW(0xE5800000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)

#define divxx_ld(RH, RL, RM, DP) /* fallback to VFP for integer div */      \
        SIB(RM)                                                             \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xEC400B10 | MRM(REG(RL), TEG(TM), TEG(T0)+0))                \
        EMITW(0xF3BB0680 | MRM(TEG(T0)+1, 0x00, TEG(T0)+0))                 \
        EMITW(0xEE800A20 | MRM(TEG(T0)+1, TEG(T0)+1, TEG(T0)+1))            \
        EMITW(0xF3BB0780 | MRM(TEG(T0)+0, 0x00, TEG(T0)+1))                 \
        EMITW(0xEE100B10 | MRM(REG(RL), TEG(T0)+0, 0x00))

/* cmp */

#define cmpxx_ri(RM, IM)                                                    \
        IM                                                                  \
        EMITW(0xE1500000 | MRM(0x00, REG(RM), TEG(TI)))

#define cmpxx_mi(RM, DP, IM)                                                \
        SIB(RM) IM                                                          \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE1500000 | MRM(0x00, TEG(TM), TEG(TI)))

#define cmpxx_rr(RG, RM)                                                    \
        EMITW(0xE1500000 | MRM(0x00, REG(RG), REG(RM)))

#define cmpxx_rm(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE1500000 | MRM(0x00, REG(RG), TEG(TM)))

#define cmpxx_mr(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE1500000 | MRM(0x00, TEG(TM), REG(RG)))

/* jmp */

#define jmpxx_mm(RM, DP)                                                    \
        SIB(RM)                                                             \
        EMITW(0xE5900000 | MRM(TEG(PC), MOD(RM), 0x00) | DP)                \

#define jmpxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(b, lb) ASM_END

#define jeqxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(beq, lb) ASM_END

#define jnexx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(bne, lb) ASM_END

#define jnzxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(bne, lb) ASM_END

#define jltxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(blt, lb) ASM_END

#define jlexx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(ble, lb) ASM_END

#define jgtxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(bgt, lb) ASM_END

#define jgexx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(bge, lb) ASM_END

#define LBL(lb)                                                             \
        ASM_BEG ASM_OP0(lb:) ASM_END

#endif /* RT_RTARCH_ARM_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
