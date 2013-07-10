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

#define REG(reg, mod, sib)  reg
#define MOD(reg, mod, sib)  mod
#define SIB(reg, mod, sib)  sib

/* registers    REG,  MOD,  SIB */

#define Reax    0x00, 0x00, EMPTY       /* r0 */
#define Recx    0x01, 0x00, EMPTY       /* r1 */
#define Redx    0x02, 0x00, EMPTY       /* r2 */
#define Rebx    0x03, 0x00, EMPTY       /* r3 */
#define Resp    0x04, 0x00, EMPTY       /* r4 */
#define Rebp    0x05, 0x00, EMPTY       /* r5 */
#define Resi    0x06, 0x00, EMPTY       /* r6 */
#define Redi    0x07, 0x00, EMPTY       /* r7 */

#define TMxx    0x08                    /* r8 */
#define TIxx    0x09                    /* r8 */
#define TPxx    0x0A                    /* r10 */
#define TDxx    0x0B                    /* r11 */
#define PCxx    0x0F                    /* r15 */

/* addressing   REG,  MOD,  SIB */

#define Oeax    0x00, 0x00, EMPTY       /* [r0] */

#define Mecx    0x01, 0x01, EMPTY       /* [r1, DP] */
#define Medx    0x02, 0x02, EMPTY       /* [r2, DP] */
#define Mebx    0x03, 0x03, EMPTY       /* [r3, DP] */
#define Mebp    0x05, 0x05, EMPTY       /* [r5, DP] */
#define Mesi    0x06, 0x06, EMPTY       /* [r6, DP] */
#define Medi    0x07, 0x07, EMPTY       /* [r7, DP] */

#define Iecx    0x01, TPxx, EMITW(0xE0800000 | MRM(TPxx,    0x01,    0x00))
#define Iedx    0x02, TPxx, EMITW(0xE0800000 | MRM(TPxx,    0x02,    0x00))
#define Iebx    0x03, TPxx, EMITW(0xE0800000 | MRM(TPxx,    0x03,    0x00))
#define Iebp    0x05, TPxx, EMITW(0xE0800000 | MRM(TPxx,    0x05,    0x00))
#define Iesi    0x06, TPxx, EMITW(0xE0800000 | MRM(TPxx,    0x06,    0x00))
#define Iedi    0x07, TPxx, EMITW(0xE0800000 | MRM(TPxx,    0x07,    0x00))

/* immediate */

#define VAL(val, typ, cmd)  val
#define TYP(val, typ, cmd)  typ
#define CMD(val, typ, cmd)  cmd

#define IB(im)  (im), 0x02000000 | ((im) & 0xFF),                           \
                EMPTY

#define IH(im)  (im), 0x00000000 | TIxx,                                    \
                EMITW(0xE3000000 | MRM(TIxx,    0x00,    0x00) |            \
                     (0x000F0000 & (im) <<  4) | (0xFFF & (im)))

#define IW(im)  (im), 0x00000000 | TIxx,                                    \
                EMITW(0xE3000000 | MRM(TIxx,    0x00,    0x00) |            \
                     (0x000F0000 & (im) <<  4) | (0xFFF & (im)))            \
                EMITW(0xE3400000 | MRM(TIxx,    0x00,    0x00) |            \
                     (0x000F0000 & (im) >> 12) | (0xFFF & (im) >> 16))

#define DP(im)  (im), 0x02000E00 | ((im) >> 4 & 0xFF),                      \
                EMPTY

#define DH(im)  (im), 0x00000000 | TDxx,/* only for quads (16-byte align) */\
                EMITW(0xE3000000 | MRM(TDxx,    0x00,    0x00) |            \
                     (0x000F0000 & (im) <<  4) | (0xFFF & (im)))

#define DW(im)  (im), 0x00000000 | TDxx,/* only for quads (16-byte align) */\
                EMITW(0xE3000000 | MRM(TDxx,    0x00,    0x00) |            \
                     (0x000F0000 & (im) <<  4) | (0xFFF & (im)))            \
                EMITW(0xE3400000 | MRM(TDxx,    0x00,    0x00) |            \
                     (0x000F0000 & (im) >> 12) | (0xFFF & (im) >> 16))

#define PLAIN   DP(0)

/* wrappers */

#define AUX(sib, dpt, imm)  sib  dpt  imm

/******************************************************************************/
/**********************************   ARM   ***********************************/
/******************************************************************************/

/* mov */

#define movxx_ri(RM, IM)     /* one unnecessary op for IH, IW */            \
        AUX(EMPTY,   EMPTY,   CMD(IM))                                      \
        EMITW(0xE1A00000 | MRM(REG(RM), 0x00,    0x00) | TYP(IM))

#define movxx_mi(RM, DP, IM) /* one unnecessary op for IH, IW */            \
        EMITW(0xE1A00000 | MRM(TIxx,    0x00,    0x00) | TYP(IM))           \
        AUX(SIB(RM), EMPTY,   CMD(IM))                                      \
        EMITW(0xE5800000 | MRM(TIxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF))

#define movxx_rr(RG, RM)                                                    \
        EMITW(0xE1A00000 | MRM(REG(RG), 0x00,    REG(RM)))

#define movxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(REG(RG), MOD(RM), 0x00) | (VAL(DP) & 0xFFF))

#define movxx_st(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5800000 | MRM(REG(RG), MOD(RM), 0x00) | (VAL(DP) & 0xFFF))

#define adrxx_ld(RG, RM, DP) /* only 10-bit offsets and 4-byte alignment */ \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE2800F00 | MRM(REG(RG), MOD(RM), 0x00) |                    \
                                        (VAL(DP) >> 2 & 0xFF)

#define stack_sa()                                                          \
        EMITW(0xE92D07FF)

#define stack_la()                                                          \
        EMITW(0xE8BD07FF)

/* add */

#define addxx_ri(RM, IM)                                                    \
        AUX(EMPTY,   EMPTY,   CMD(IM))                                      \
        EMITW(0xE0800000 | MRM(REG(RM), REG(RM), 0x00) | TYP(IM))

#define addxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   CMD(IM))                                      \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xE0800000 | MRM(TMxx,    TMxx,    0x00) | TYP(IM))           \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF))

#define addxx_rr(RG, RM)                                                    \
        EMITW(0xE0800000 | MRM(REG(RG), REG(RG), REG(RM)))

#define addxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xE0800000 | MRM(REG(RG), REG(RG), TMxx))

#define addxx_st(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xE0800000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF))

/* sub */

#define subxx_ri(RM, IM)                                                    \
        AUX(EMPTY,   EMPTY,   CMD(IM))                                      \
        EMITW(0xE0400000 | MRM(REG(RM), REG(RM), 0x00) | TYP(IM))

#define subxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   CMD(IM))                                      \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xE0400000 | MRM(TMxx,    TMxx,    0x00) | TYP(IM))           \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF))

#define subxx_rr(RG, RM)                                                    \
        EMITW(0xE0400000 | MRM(REG(RG), REG(RG), REG(RM)))

#define subxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xE0400000 | MRM(REG(RG), REG(RG), TMxx))

#define subxx_st(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xE0400000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF))

/* and */

#define andxx_ri(RM, IM)                                                    \
        AUX(EMPTY,   EMPTY,   CMD(IM))                                      \
        EMITW(0xE0000000 | MRM(REG(RM), REG(RM), 0x00) | TYP(IM))

#define andxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   CMD(IM))                                      \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xE0000000 | MRM(TMxx,    TMxx,    0x00) | TYP(IM))           \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF))

#define andxx_rr(RG, RM)                                                    \
        EMITW(0xE0000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define andxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xE0000000 | MRM(REG(RG), REG(RG), TMxx))

#define andxx_st(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xE0000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF))

/* orr */

#define orrxx_ri(RM, IM)                                                    \
        AUX(EMPTY,   EMPTY,   CMD(IM))                                      \
        EMITW(0xE1800000 | MRM(REG(RM), REG(RM), 0x00) | TYP(IM))

#define orrxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   CMD(IM))                                      \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xE1800000 | MRM(TMxx,    TMxx,    0x00) | TYP(IM))           \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF))

#define orrxx_rr(RG, RM)                                                    \
        EMITW(0xE1800000 | MRM(REG(RG), REG(RG), REG(RM)))

#define orrxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xE1800000 | MRM(REG(RG), REG(RG), TMxx))

#define orrxx_st(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xE1800000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF))

/* not */

#define notxx_rr(RM)                                                        \
        EMITW(0xE1E00000 | MRM(REG(RM), 0x00,    REG(RM)))

#define notxx_mm(RM, DP)                                                    \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF))

/* shl */

#define shlxx_ri(RM, IM)                                                    \
        EMITW(0xE1A00000 | MRM(REG(RM), 0x00,    REG(RM)) |                 \
                                        (0x1F & VAL(IM)) << 7)

#define shlxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xE1A00000 | MRM(REG(RM), 0x00,    REG(RM)) |                 \
                                        (0x1F & VAL(IM)) << 7)              \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF))

#define mulxx_ld(RH, RL, RM, DP)                                            \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xE0000090 | MRM(0x00,    REG(RL), REG(RL)) |                 \
                                        TMxx << 8)

/* shr */

#define shrxx_ri(RM, IM)                                                    \
        EMITW(0xE1A00020 | MRM(REG(RM), 0x00,    REG(RM)) |                 \
                                        (VAL(IM) & 0x1F) << 7)

#define shrxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xE1A00020 | MRM(REG(RM), 0x00,    REG(RM)) |                 \
                                        (VAL(IM) & 0x1F) << 7)              \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF))

#define divxx_ld(RH, RL, RM, DP) /* fallback to VFP for integer div */      \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xEC400B10 | MRM(REG(RL), TMxx,    T0xx+0))                   \
        EMITW(0xF3BB0680 | MRM(T0xx+1,  0x00,    T0xx+0))                   \
        EMITW(0xEE800A20 | MRM(T0xx+1,  T0xx+1,  T0xx+1))                   \
        EMITW(0xF3BB0780 | MRM(T0xx+0,  0x00,    T0xx+1))                   \
        EMITW(0xEE100B10 | MRM(REG(RL), T0xx+0,  0x00))

/* cmp */

#define cmpxx_ri(RM, IM)                                                    \
        AUX(EMPTY,   EMPTY,   CMD(IM))                                      \
        EMITW(0xE1500000 | MRM(0x00,    REG(RM), 0x00) | TYP(IM))

#define cmpxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   CMD(IM))                                      \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xE1500000 | MRM(0x00,    TMxx,    0x00) | TYP(IM))

#define cmpxx_rr(RG, RM)                                                    \
        EMITW(0xE1500000 | MRM(0x00,    REG(RG), REG(RM)))

#define cmpxx_rm(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xE1500000 | MRM(0x00,    REG(RG), TMxx))

#define cmpxx_mr(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \
        EMITW(0xE1500000 | MRM(0x00,    TMxx,    REG(RG)))

/* jmp */

#define jmpxx_mm(RM, DP)                                                    \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(PCxx,    MOD(RM), 0x00) | (VAL(DP) & 0xFFF)) \

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
