/******************************************************************************/
/* Copyright (c) 2013-2015 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_ARM_H
#define RT_RTARCH_ARM_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_arm.h: Implementation of ARMv7/8:AArch32 core instructions.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdxx_ri - applies [cmd] to [r]egister from [i]mmediate
 * cmdxx_mi - applies [cmd] to [m]emory   from [i]mmediate
 *
 * cmdxx_rm - applies [cmd] to [r]egister from [m]emory
 * cmdxx_ld - applies [cmd] as above
 * cmdxx_mr - applies [cmd] to [m]emory   from [r]egister
 * cmdxx_st - applies [cmd] as above (arg list as cmdxx_ld)
 *
 * cmdxx_rr - applies [cmd] to [r]egister from [r]egister
 * cmdxx_mm - applies [cmd] to [m]emory   from [m]emory
 * cmdxx_rr - applies [cmd] to [r]egister (one operand cmd)
 * cmdxx_mm - applies [cmd] to [m]emory   (one operand cmd)
 *
 * cmdxx_rx - applies [cmd] to [r]egister from x-register
 * cmdxx_mx - applies [cmd] to [m]emory   from x-register
 * cmdxx_xr - applies [cmd] to x-register from [r]egister
 * cmdxx_xm - applies [cmd] to x-register from [m]emory
 *
 * cmdxx_rl - applies [cmd] to [r]egister from [l]abel
 * cmdxx_xl - applies [cmd] to x-register from [l]abel
 * cmdxx_lb - applies [cmd] as above
 * label_ld - applies [adr] as above
 *
 * stack_st - applies [mov] to stack from register (push)
 * stack_ld - applies [mov] to register from stack (pop)
 * stack_sa - applies [mov] to stack from all registers
 * stack_la - applies [mov] to all registers from stack
 *
 * cmdx*_** - applies [cmd] to core register/memory/immediate args
 * cmd*x_** - applies [cmd] to unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to   signed integer args, [n] - negatable
 *
 * Argument x-register (implied) is fixed by the implementation.
 * Some formal definitions are not given below to encourage
 * use of friendly aliases for better code readability.
 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* emitters */

#define EMPTY   ASM_BEG ASM_END

#define EMITW(w) /* little endian */                                        \
        EMITB((w) >> 0x00 & 0xFF)                                           \
        EMITB((w) >> 0x08 & 0xFF)                                           \
        EMITB((w) >> 0x10 & 0xFF)                                           \
        EMITB((w) >> 0x18 & 0xFF)

/* structural */

#define MRM(reg, ren, rem)                                                  \
        ((ren) << 16 | (reg) << 12 | (rem))

#define AUX(sib, cdp, cim)  sib  cdp  cim

/* selectors  */

#define REG(reg, mod, sib)  reg
#define MOD(reg, mod, sib)  mod
#define SIB(reg, mod, sib)  sib

#define VAL(val, typ, cmd)  val
#define TYP(val, typ, cmd)  typ
#define CMD(val, typ, cmd)  cmd

/* registers    REG */

#define TMxx    0x08                    /* r8 */
#define TIxx    0x09                    /* r9, not used together with TDxx */
#define TDxx    0x09                    /* r9, not used together with TIxx */
#define TPxx    0x0A                    /* r10 */
#define PCxx    0x0F                    /* r15 */

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Reax    0x00, 0x00, EMPTY       /* r0 */
#define Recx    0x01, 0x00, EMPTY       /* r1 */
#define Redx    0x02, 0x00, EMPTY       /* r2 */
#define Rebx    0x03, 0x00, EMPTY       /* r3 */
#define Rebp    0x05, 0x00, EMPTY       /* r5 */
#define Resi    0x06, 0x00, EMPTY       /* r6 */
#define Redi    0x07, 0x00, EMPTY       /* r7 */

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

/* immediate    VAL,  TYP,  CMD */

#define IB(im)  (im), 0x02000000 | ((im) & 0x7F),/* drop sign-ext in x86 */ \
                EMPTY          /* for compatibility with zero-ext in ARM */

#define IM(im)  (im), 0x00000000 | TIxx,                                    \
                EMITW(0xE3000000 | MRM(TIxx,    0x00,    0x00) |            \
                                                 (0xFFF & (im)))

#define IH(im)  (im), 0x00000000 | TIxx,                                    \
                EMITW(0xE3000000 | MRM(TIxx,    0x00,    0x00) |            \
                        (0xF0000 & (im) <<  4) | (0xFFF & (im)))

#define IW(im)  (im), 0x00000000 | TIxx,                                    \
                EMITW(0xE3000000 | MRM(TIxx,    0x00,    0x00) |            \
                        (0xF0000 & (im) <<  4) | (0xFFF & (im)))            \
                EMITW(0xE3400000 | MRM(TIxx,    0x00,    0x00) |            \
                        (0xF0000 & (im) >> 12) | (0xFFF & (im) >> 16))

/* displacement VAL,  TYP,  CMD */

#define DP(im)  (im), 0x02000E00 | ((im) >> 4 & 0xFF),                      \
                EMPTY

#define DH(im)  (im), 0x00000000 | TDxx,   /* only for SIMD instructions */ \
                EMITW(0xE3000000 | MRM(TDxx,    0x00,    0x00) |            \
                        (0xF0000 & (im) <<  4) | (0xFF0 & (im)))

#define PLAIN   DP(0)

/* triplet pass-through wrapper */

#define W(p1, p2, p3)       p1,  p2,  p3

/******************************************************************************/
/**********************************   ARM   ***********************************/
/******************************************************************************/

/* mov
 * set-flags: no */

#define movxx_ri(RM, IM)     /* one unnecessary op for IH, IW */            \
        AUX(EMPTY,   EMPTY,   CMD(IM))                                      \
        EMITW(0xE1A00000 | MRM(REG(RM), 0x00,    0x00) | TYP(IM))

#define movxx_mi(RM, DP, IM) /* one unnecessary op for IH, IW */            \
        EMITW(0xE1A00000 | MRM(TIxx,    0x00,    0x00) | TYP(IM))           \
        AUX(SIB(RM), EMPTY,   CMD(IM))                                      \
        EMITW(0xE5800000 | MRM(TIxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

#define movxx_rr(RG, RM)                                                    \
        EMITW(0xE1A00000 | MRM(REG(RG), 0x00,    REG(RM)))

#define movxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(REG(RG), MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

#define movxx_st(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5800000 | MRM(REG(RG), MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

#define adrxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE3000000 | MRM(TDxx,    0x00,    0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE0800000 | MRM(REG(RG), MOD(RM), TDxx))

#define adrxx_lb(lb) /* load label to Reax */                               \
        label_ld(lb)

#define stack_st(RM)                                                        \
        EMITW(0xE52D0004 | MRM(REG(RM), 0x00,    0x00))

#define stack_ld(RM)                                                        \
        EMITW(0xE49D0004 | MRM(REG(RM), 0x00,    0x00))

#define stack_sa() /* save all [r0 - r11], 12 regs in total */              \
        EMITW(0xE92D0FFF)

#define stack_la() /* load all [r0 - r11], 12 regs in total */              \
        EMITW(0xE8BD0FFF)

/* and
 * set-flags: yes */

#define andxx_ri(RM, IM)                                                    \
        AUX(EMPTY,   EMPTY,   CMD(IM))                                      \
        EMITW(0xE0100000 | MRM(REG(RM), REG(RM), 0x00) | TYP(IM))

#define andxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   CMD(IM))                                      \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE0100000 | MRM(TMxx,    TMxx,    0x00) | TYP(IM))           \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

#define andxx_rr(RG, RM)                                                    \
        EMITW(0xE0100000 | MRM(REG(RG), REG(RG), REG(RM)))

#define andxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE0100000 | MRM(REG(RG), REG(RG), TMxx))

#define andxx_st(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE0100000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

/* orr
 * set-flags: no */

#define orrxx_ri(RM, IM)                                                    \
        AUX(EMPTY,   EMPTY,   CMD(IM))                                      \
        EMITW(0xE1800000 | MRM(REG(RM), REG(RM), 0x00) | TYP(IM))

#define orrxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   CMD(IM))                                      \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE1800000 | MRM(TMxx,    TMxx,    0x00) | TYP(IM))           \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

#define orrxx_rr(RG, RM)                                                    \
        EMITW(0xE1800000 | MRM(REG(RG), REG(RG), REG(RM)))

#define orrxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE1800000 | MRM(REG(RG), REG(RG), TMxx))

#define orrxx_st(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE1800000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

/* xor
 * set-flags: no */

#define xorxx_ri(RM, IM)                                                    \
        AUX(EMPTY,   EMPTY,   CMD(IM))                                      \
        EMITW(0xE0200000 | MRM(REG(RM), REG(RM), 0x00) | TYP(IM))

#define xorxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   CMD(IM))                                      \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE0200000 | MRM(TMxx,    TMxx,    0x00) | TYP(IM))           \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

#define xorxx_rr(RG, RM)                                                    \
        EMITW(0xE0200000 | MRM(REG(RG), REG(RG), REG(RM)))

#define xorxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE0200000 | MRM(REG(RG), REG(RG), TMxx))

#define xorxx_st(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE0200000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

/* not
 * set-flags: no */

#define notxx_rr(RM)                                                        \
        EMITW(0xE1E00000 | MRM(REG(RM), 0x00,    REG(RM)))

#define notxx_mm(RM, DP)                                                    \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE1E00000 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

/* neg
 * set-flags: yes */

#define negxx_rr(RM)                                                        \
        EMITW(0xE2700000 | MRM(REG(RM), REG(RM), 0x00))

#define negxx_mm(RM, DP)                                                    \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE2700000 | MRM(TMxx,    TMxx,    0x00))                     \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

/* add
 * set-flags: yes */

#define addxx_ri(RM, IM)                                                    \
        AUX(EMPTY,   EMPTY,   CMD(IM))                                      \
        EMITW(0xE0900000 | MRM(REG(RM), REG(RM), 0x00) | TYP(IM))

#define addxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   CMD(IM))                                      \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE0900000 | MRM(TMxx,    TMxx,    0x00) | TYP(IM))           \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

#define addxx_rr(RG, RM)                                                    \
        EMITW(0xE0900000 | MRM(REG(RG), REG(RG), REG(RM)))

#define addxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE0900000 | MRM(REG(RG), REG(RG), TMxx))

#define addxx_st(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE0900000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

/* sub
 * set-flags: yes */

#define subxx_ri(RM, IM)                                                    \
        AUX(EMPTY,   EMPTY,   CMD(IM))                                      \
        EMITW(0xE0500000 | MRM(REG(RM), REG(RM), 0x00) | TYP(IM))

#define subxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   CMD(IM))                                      \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE0500000 | MRM(TMxx,    TMxx,    0x00) | TYP(IM))           \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

#define subxx_rr(RG, RM)                                                    \
        EMITW(0xE0500000 | MRM(REG(RG), REG(RG), REG(RM)))

#define subxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE0500000 | MRM(REG(RG), REG(RG), TMxx))

#define subxx_st(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE0500000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

#define subxx_mr(RM, DP, RG)                                                \
        subxx_st(W(RG), W(RM), W(DP))

/* shl
 * set-flags: no */

#define shlxx_ri(RM, IM)                                                    \
        EMITW(0xE1A00000 | MRM(REG(RM), 0x00,    REG(RM)) |                 \
                                                 (0x1F & VAL(IM)) << 7)

#define shlxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE1A00000 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IM)) << 7)     \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

#define shlxx_rx(RM)     /* reads Recx for shift value */                   \
        EMITW(0xE1A00110 | MRM(REG(RM), 0x00,    REG(RM)))

#define shlxx_mx(RM, DP) /* reads Recx for shift value */                   \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE1A00110 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

/* shr
 * set-flags: no */

#define shrxx_ri(RM, IM)                                                    \
        EMITW(0xE1A00020 | MRM(REG(RM), 0x00,    REG(RM)) |                 \
                                                 (0x1F & VAL(IM)) << 7)

#define shrxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE1A00020 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IM)) << 7)     \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

#define shrxx_rx(RM)     /* reads Recx for shift value */                   \
        EMITW(0xE1A00130 | MRM(REG(RM), 0x00,    REG(RM)))

#define shrxx_mx(RM, DP) /* reads Recx for shift value */                   \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE1A00130 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

#define shrxn_ri(RM, IM)                                                    \
        EMITW(0xE1A00040 | MRM(REG(RM), 0x00,    REG(RM)) |                 \
                                                 (0x1F & VAL(IM)) << 7)

#define shrxn_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE1A00040 | MRM(TMxx,    0x00,    TMxx) |                    \
                                                 (0x1F & VAL(IM)) << 7)     \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

#define shrxn_rx(RM)     /* reads Recx for shift value */                   \
        EMITW(0xE1A00150 | MRM(REG(RM), 0x00,    REG(RM)))

#define shrxn_mx(RM, DP) /* reads Recx for shift value */                   \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE1A00150 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xE5800000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

/* mul
 * set-flags: no */

#define mulxx_ri(RM, IM) /* one unnecessary op for IH, IW */                \
        EMITW(0xE1A00000 | MRM(TIxx,    0x00,    0x00) | TYP(IM))           \
        AUX(EMPTY,   EMPTY,   CMD(IM))                                      \
        EMITW(0xE0000090 | REG(RM) << 16 | REG(RM) << 8| TIxx)

#define mulxx_rr(RG, RM)                                                    \
        EMITW(0xE0000090 | REG(RG) << 16 | REG(RG) << 8| REG(RM))

#define mulxx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE0000090 | REG(RG) << 16 | REG(RG) << 8| TMxx)

#define mulxn_ri(RM, IM)                                                    \
        mulxx_ri(W(RM), W(IM))

#define mulxn_rr(RG, RM)                                                    \
        mulxx_rr(W(RG), W(RM))

#define mulxn_ld(RG, RM, DP)                                                \
        mulxx_ld(W(RG), W(RM), W(DP))

#define mulxx_xr(RM)     /* Reax is in/out, Redx is zero-ext-out(high) */   \
        EMITW(0xE0800090 | MRM(0x00,    0x02,    REG(RM)))

#define mulxx_xm(RM, DP) /* Reax is in/out, Redx is zero-ext-out(high) */   \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE0800090 | MRM(0x00,    0x02,    TMxx))

#define mulxn_xr(RM)     /* Reax is in/out, Redx is sign-ext-out(high) */   \
        EMITW(0xE0C00090 | MRM(0x00,    0x02,    REG(RM)))

#define mulxn_xm(RM, DP) /* Reax is in/out, Redx is sign-ext-out(high) */   \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE0C00090 | MRM(0x00,    0x02,    TMxx))

/* div
 * set-flags: no */

#if (RT_128 < 2)

#define divxx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0xEC400B10 | MRM(0x00,    REG(RM), Tmm0+0))                   \
        EMITW(0xEEB80B60 | MRM(Tmm0+1,  0x00,    Tmm0+0))                   \
        EMITW(0xEEB80B40 | MRM(Tmm0+0,  0x00,    Tmm0+0))  /* full-range */ \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1)) /* <- fp64 div */ \
        EMITW(0xEEBC0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(0x00,    Tmm0+0,  0x00)) /* destroys Redx */

#define divxx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUX(SIB(RM), EMPTY,   EMPTY) /* destroys Xmm0, fallback to VFP */   \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xEC400B10 | MRM(0x00,    TMxx,    Tmm0+0))                   \
        EMITW(0xEEB80B60 | MRM(Tmm0+1,  0x00,    Tmm0+0))                   \
        EMITW(0xEEB80B40 | MRM(Tmm0+0,  0x00,    Tmm0+0))  /* full-range */ \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1)) /* <- fp64 div */ \
        EMITW(0xEEBC0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(0x00,    Tmm0+0,  0x00)) /* destroys Redx */

#define divxn_xr(RM)     /* Reax is in/out, Redx is sign-ext-(Reax)-in */   \
        EMITW(0xEC400B10 | MRM(0x00,    REG(RM), Tmm0+0))                   \
        EMITW(0xEEB80BE0 | MRM(Tmm0+1,  0x00,    Tmm0+0))                   \
        EMITW(0xEEB80BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))  /* full-range */ \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1)) /* <- fp64 div */ \
        EMITW(0xEEBD0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(0x00,    Tmm0+0,  0x00)) /* destroys Redx */

#define divxn_xm(RM, DP) /* Reax is in/out, Redx is sign-ext-(Reax)-in */   \
        AUX(SIB(RM), EMPTY,   EMPTY) /* destroys Xmm0, fallback to VFP */   \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xEC400B10 | MRM(0x00,    TMxx,    Tmm0+0))                   \
        EMITW(0xEEB80BE0 | MRM(Tmm0+1,  0x00,    Tmm0+0))                   \
        EMITW(0xEEB80BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))  /* full-range */ \
        EMITW(0xEE800B00 | MRM(Tmm0+0,  Tmm0+0,  Tmm0+1)) /* <- fp64 div */ \
        EMITW(0xEEBD0BC0 | MRM(Tmm0+0,  0x00,    Tmm0+0))                   \
        EMITW(0xEE100B10 | MRM(0x00,    Tmm0+0,  0x00)) /* destroys Redx */

#define divxp_xr(RM)     /* Reax is in/out, Redx is sign-ext-(Reax)-in */   \
        EMITW(0xEC400B10 | MRM(0x00,    REG(RM), Tmm0+0))                   \
        EMITW(0xF3BB0600 | MRM(Tmm0+1,  0x00,    Tmm0+0))  /* part-range */ \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1)) /* <- fp32 div */ \
        EMITW(0xF3BB0700 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(0x00,    Tmm0+0,  0x00)) /* destroys Redx */

#define divxp_xm(RM, DP) /* Reax is in/out, Redx is sign-ext-(Reax)-in */   \
        AUX(SIB(RM), EMPTY,   EMPTY) /* destroys Xmm0, fallback to VFP */   \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xEC400B10 | MRM(0x00,    TMxx,    Tmm0+0))                   \
        EMITW(0xF3BB0600 | MRM(Tmm0+1,  0x00,    Tmm0+0))  /* part-range */ \
        EMITW(0xEE800A20 | MRM(Tmm0+1,  Tmm0+1,  Tmm0+1)) /* <- fp32 div */ \
        EMITW(0xF3BB0700 | MRM(Tmm0+0,  0x00,    Tmm0+1))                   \
        EMITW(0xEE100B10 | MRM(0x00,    Tmm0+0,  0x00)) /* destroys Redx */

#else /* RT_128 >= 2 */

#define divxx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0xE730F010 | MRM(0x00,    0x00,    0x00) | REG(RM) << 8)

#define divxx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUX(SIB(RM), EMPTY,   EMPTY) /* destroys Redx, Xmm0 (in ARMv7) */   \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE730F010 | MRM(0x00,    0x00,    0x00) | TMxx << 8)

#define divxn_xr(RM)     /* Reax is in/out, Redx is sign-ext-(Reax)-in */   \
        EMITW(0xE710F010 | MRM(0x00,    0x00,    0x00) | REG(RM) << 8)

#define divxn_xm(RM, DP) /* Reax is in/out, Redx is sign-ext-(Reax)-in */   \
        AUX(SIB(RM), EMPTY,   EMPTY) /* destroys Redx, Xmm0 (in ARMv7) */   \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE710F010 | MRM(0x00,    0x00,    0x00) | TMxx << 8)

#define divxp_xr(RM)     /* Reax is in/out, Redx is sign-ext-(Reax)-in */   \
        divxn_xr(W(RM))              /* part-range fp32 div (in ARMv7) */

#define divxp_xm(RM, DP) /* Reax is in/out, Redx is sign-ext-(Reax)-in */   \
        divxn_xm(W(RM), W(DP))       /* part-range fp32 div (in ARMv7) */

#endif /* RT_128 >= 2 */

/* cmp
 * set-flags: yes */

#define cmpxx_ri(RM, IM)                                                    \
        AUX(EMPTY,   EMPTY,   CMD(IM))                                      \
        EMITW(0xE1500000 | MRM(0x00,    REG(RM), 0x00) | TYP(IM))

#define cmpxx_mi(RM, DP, IM)                                                \
        AUX(SIB(RM), EMPTY,   CMD(IM))                                      \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE1500000 | MRM(0x00,    TMxx,    0x00) | TYP(IM))

#define cmpxx_rr(RG, RM)                                                    \
        EMITW(0xE1500000 | MRM(0x00,    REG(RG), REG(RM)))

#define cmpxx_rm(RG, RM, DP)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE1500000 | MRM(0x00,    REG(RG), TMxx))

#define cmpxx_mr(RM, DP, RG)                                                \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(TMxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))  \
        EMITW(0xE1500000 | MRM(0x00,    TMxx,    REG(RG)))

/* jmp
 * set-flags: no */

#define jmpxx_mm(RM, DP)                                                    \
        AUX(SIB(RM), EMPTY,   EMPTY)                                        \
        EMITW(0xE5900000 | MRM(PCxx,    MOD(RM), 0x00) |(VAL(DP) & 0xFFC))

#define jmpxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(b,   lb) ASM_END

#define jeqxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(beq, lb) ASM_END

#define jezxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(beq, lb) ASM_END

#define jnexx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(bne, lb) ASM_END

#define jnzxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(bne, lb) ASM_END

#define jltxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(blo, lb) ASM_END

#define jlexx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(bls, lb) ASM_END

#define jgtxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(bhi, lb) ASM_END

#define jgexx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(bhs, lb) ASM_END

#define jltxn_lb(lb)                                                        \
        ASM_BEG ASM_OP1(blt, lb) ASM_END

#define jlexn_lb(lb)                                                        \
        ASM_BEG ASM_OP1(ble, lb) ASM_END

#define jgtxn_lb(lb)                                                        \
        ASM_BEG ASM_OP1(bgt, lb) ASM_END

#define jgexn_lb(lb)                                                        \
        ASM_BEG ASM_OP1(bge, lb) ASM_END

#define LBL(lb)                                                             \
        ASM_BEG ASM_OP0(lb:) ASM_END

/* ver
 * set-flags: no */

#define verxx_xx() /* destroys Reax, Recx, Rebx, Redx, Resi, Redi (in x86)*/\
        movxx_mi(Mebp, inf_VER, IB(1)) /* <- NEON to bit0, without checks */

#endif /* RT_RTARCH_ARM_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
