/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X86_H
#define RT_RTARCH_X86_H

#define EMPTY   ASM_BEG ASM_END

#define EMITW(w) /* little endian */                                        \
        EMITB((w) >> 0x00 & 0xFF)                                           \
        EMITB((w) >> 0x08 & 0xFF)                                           \
        EMITB((w) >> 0x10 & 0xFF)                                           \
        EMITB((w) >> 0x18 & 0xFF)

#define MRM(mod, reg, rem)                                                  \
        EMITB((mod) << 6 | (reg) << 3 | (rem))

#define REG(reg, mod, sib)  reg
#define MOD(reg, mod, sib)  mod
#define SIB(reg, mod, sib)  sib

/* registers    REG,  MOD,  SIB */

#define Reax    0x00, 0x03, EMPTY
#define Recx    0x01, 0x03, EMPTY
#define Redx    0x02, 0x03, EMPTY
#define Rebx    0x03, 0x03, EMPTY
#define Resp    0x04, 0x03, EMPTY
#define Rebp    0x05, 0x03, EMPTY
#define Resi    0x06, 0x03, EMPTY
#define Redi    0x07, 0x03, EMPTY

/* addressing   REG,  MOD,  SIB */

#define Oeax    0x00, 0x00, EMPTY       /* [eax] */

#define Mecx    0x01, 0x02, EMPTY       /* [ecx + DP] */
#define Medx    0x02, 0x02, EMPTY       /* [edx + DP] */
#define Mebx    0x03, 0x02, EMPTY       /* [ebx + DP] */
#define Mebp    0x05, 0x02, EMPTY       /* [ebp + DP] */
#define Mesi    0x06, 0x02, EMPTY       /* [esi + DP] */
#define Medi    0x07, 0x02, EMPTY       /* [edi + DP] */

#define Iecx    0x04, 0x02, EMITB(0x01) /* [ecx + eax + DP] */
#define Iedx    0x04, 0x02, EMITB(0x02) /* [edx + eax + DP] */
#define Iebx    0x04, 0x02, EMITB(0x03) /* [ebx + eax + DP] */
#define Iebp    0x04, 0x02, EMITB(0x05) /* [ebp + eax + DP] */
#define Iesi    0x04, 0x02, EMITB(0x06) /* [esi + eax + DP] */
#define Iedi    0x04, 0x02, EMITB(0x07) /* [edi + eax + DP] */

/* immediate */

#define VAL(val, typ, cmd)  val
#define TYP(val, typ, cmd)  typ
#define CMD(val, typ, cmd)  cmd

#define IB(im)  (im), 0x02, EMITB((im) & 0xFF)
#define IH(im)  (im), 0x00, EMITW((im) & 0xFFFF)
#define IW(im)  (im), 0x00, EMITW((im) & 0xFFFFFFFF)

#define DP(im)  (im), 0x00, EMITW((im) & 0xFFF)
#define DH(im)  (im), 0x00, EMITW((im) & 0xFFFF)
#define DW(im)  (im), 0x00, EMITW((im) & 0xFFFFFFFF)

#define PLAIN   0x00, 0x00, EMPTY

/* wrappers */

#define AUX(sib, dpt, imm)  sib  dpt  imm

/******************************************************************************/
/**********************************   X86   ***********************************/
/******************************************************************************/

/* mov */

#define movxx_ri(RM, IM)                                                    \
        EMITB(0xC7)                                                         \
        MRM(MOD(RM), 0x00,    REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITW(VAL(IM)))

#define movxx_mi(RM, DP, IM)                                                \
        EMITB(0xC7)                                                         \
        MRM(MOD(RM), 0x00,    REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITW(VAL(IM)))

#define movxx_rr(RG, RM)                                                    \
        EMITB(0x8B)                                                         \
        MRM(MOD(RM), REG(RG), REG(RM))

#define movxx_ld(RG, RM, DP)                                                \
        EMITB(0x8B)                                                         \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define movxx_st(RG, RM, DP)                                                \
        EMITB(0x89)                                                         \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define adrxx_ld(RG, RM, DP) /* only 10-bit offsets and 4-byte alignment */ \
        EMITB(0x8D)                                                         \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), EMITW(VAL(DP) & 0x3FC), EMPTY)

#define stack_sa()                                                          \
        EMITB(0x60) EMITB(0x9C) 

#define stack_la()                                                          \
        EMITB(0x9D) EMITB(0x61)

/* add */

#define addxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(MOD(RM), 0x00,    REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define addxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(MOD(RM), 0x00,    REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define addxx_rr(RG, RM)                                                    \
        EMITB(0x03)                                                         \
        MRM(MOD(RM), REG(RG), REG(RM))

#define addxx_ld(RG, RM, DP)                                                \
        EMITB(0x03)                                                         \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define addxx_st(RG, RM, DP)                                                \
        EMITB(0x01)                                                         \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(MOD(RM), 0x05,    REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define subxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(MOD(RM), 0x05,    REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define subxx_rr(RG, RM)                                                    \
        EMITB(0x2B)                                                         \
        MRM(MOD(RM), REG(RG), REG(RM))

#define subxx_ld(RG, RM, DP)                                                \
        EMITB(0x2B)                                                         \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define subxx_st(RG, RM, DP)                                                \
        EMITB(0x29)                                                         \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* and */

#define andxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(MOD(RM), 0x04,    REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define andxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(MOD(RM), 0x04,    REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define andxx_rr(RG, RM)                                                    \
        EMITB(0x23)                                                         \
        MRM(MOD(RM), REG(RG), REG(RM))

#define andxx_ld(RG, RM, DP)                                                \
        EMITB(0x23)                                                         \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define andxx_st(RG, RM, DP)                                                \
        EMITB(0x21)                                                         \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* orr */

#define orrxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(MOD(RM), 0x01,    REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define orrxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(MOD(RM), 0x01,    REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define orrxx_rr(RG, RM)                                                    \
        EMITB(0x0B)                                                         \
        MRM(MOD(RM), REG(RG), REG(RM))

#define orrxx_ld(RG, RM, DP)                                                \
        EMITB(0x0B)                                                         \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define orrxx_st(RG, RM, DP)                                                \
        EMITB(0x09)                                                         \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* not */

#define notxx_rr(RM)                                                        \
        EMITB(0xF7)                                                         \
        MRM(MOD(RM), 0x02,    REG(RM))

#define notxx_mm(RM, DP)                                                    \
        EMITB(0xF7)                                                         \
        MRM(MOD(RM), 0x02,    REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shl */

#define shlxx_ri(RM, IM)                                                    \
        EMITB(0xC1)                                                         \
        MRM(MOD(RM), 0x04,    REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM)))

#define shlxx_mi(RM, DP, IM)                                                \
        EMITB(0xC1)                                                         \
        MRM(MOD(RM), 0x04,    REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(VAL(IM)))

#define mulxx_ld(RH, RL, RM, DP)                                            \
        EMITB(0xF7)                                                         \
        MRM(MOD(RM), 0x05,    REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shr */

#define shrxx_ri(RM, IM)                                                    \
        EMITB(0xC1)                                                         \
        MRM(MOD(RM), 0x05,    REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM)))

#define shrxx_mi(RM, DP, IM)                                                \
        EMITB(0xC1)                                                         \
        MRM(MOD(RM), 0x05,    REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(VAL(IM)))

#define divxx_ld(RH, RL, RM, DP)                                            \
        EMITB(0xF7)                                                         \
        MRM(MOD(RM), 0x07,    REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cmp */

#define cmpxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(MOD(RM), 0x07,    REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define cmpxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
        MRM(MOD(RM), 0x07,    REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define cmpxx_rr(RG, RM)                                                    \
        EMITB(0x3B)                                                         \
        MRM(MOD(RM), REG(RG), REG(RM))

#define cmpxx_rm(RG, RM, DP)                                                \
        EMITB(0x3B)                                                         \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define cmpxx_mr(RG, RM, DP)                                                \
        EMITB(0x39)                                                         \
        MRM(MOD(RM), REG(RG), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* jmp */

#define jmpxx_mm(RM, DP)                                                    \
        EMITB(0xFF)                                                         \
        MRM(MOD(RM), 0x04,    REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define jmpxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jmp, lb) ASM_END

#define jeqxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(je,  lb) ASM_END

#define jnexx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jne, lb) ASM_END

#define jnzxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jnz, lb) ASM_END

#define jltxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jl,  lb) ASM_END

#define jlexx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jle, lb) ASM_END

#define jgtxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jg,  lb) ASM_END

#define jgexx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jge, lb) ASM_END

#define LBL(lb)                                                             \
        ASM_BEG ASM_OP0(lb:) ASM_END

#endif /* RT_RTARCH_X86_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
