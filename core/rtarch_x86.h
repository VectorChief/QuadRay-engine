/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X86_H
#define RT_RTARCH_X86_H

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

#define MRM(reg, mod, rem)                                                  \
        EMITB((mod) << 6 | (reg) << 3 | (rem))

#define AUX(sib, cdp, cim)  sib  cdp  cim

/* selectors  */

#define REG(reg, mod, sib)  reg
#define MOD(reg, mod, sib)  mod
#define SIB(reg, mod, sib)  sib

#define VAL(val, typ, cmd)  val
#define TYP(val, typ, cmd)  typ
#define CMD(val, typ, cmd)  cmd

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

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

/* immediate    VAL,  TYP,  CMD */

#define IB(im)  (im), 0x02, EMITB((im) & 0xFF)
#define IH(im)  (im), 0x00, EMITW((im) & 0xFFFF)
#define IW(im)  (im), 0x00, EMITW((im) & 0xFFFFFFFF)

/* displacement VAL,  TYP,  CMD */

#define DP(im)  (im), 0x00, EMITW((im) & 0xFFF)
#define DH(im)  (im), 0x00, EMITW((im) & 0xFFFF)
#define DW(im)  (im), 0x00, EMITW((im) & 0xFFFFFFFF)

#define PLAIN   0x00, 0x00, EMPTY

/* wrappers */

#define W(p1, p2, p3)       p1,  p2,  p3

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/* cmdxx_ri - applies [cmd] to [r]egister from [i]mmediate  */
/* cmdxx_mi - applies [cmd] to [m]emory   from [i]mmediate  */

/* cmdxx_rr - applies [cmd] to [r]egister from [r]egister   */
/* cmdxx_rl - applies [cmd] to [r]egister from [l]abel      */

/* cmdxx_rm - applies [cmd] to [r]egister from [m]emory     */
/* cmdxx_ld - applies [cmd] as above                        */

/* cmdxx_mr - applies [cmd] to [m]emory   from [r]egister   */
/* cmdxx_st - applies [cmd] as above (arg list as cmdxx_ld) */

/* cmdxx_rg - applies [cmd] to [reg]ister (one operand cmd) */
/* cmdxx_rr - applies [cmd] as above   (to/from [register]) */
/* cmdxx_mm - applies [cmd] to [mem]ory   (one operand cmd) */

/* cmdxx_rx - applies [cmd] to [r]egister from * register   */
/* cmdxx_mx - applies [cmd] to [m]emory   from * register   */

/* cmdxx_xr - applies [cmd] to * register from [r]egister   */
/* cmdxx_xm - applies [cmd] to * register from [m]emory     */
/* cmdxx_xl - applies [cmd] to * register from [l]abel      */
/* cmdxx_lb - applies [cmd] as above     (from [lab]el)     */
/* label_ld - applies [cmd] as above (load label to Reax)   */

/* stack_st - applies [cmd] to stack from register (push)   */
/* stack_ld - applies [cmd] to register from stack (pop)    */
/* stack_sa - applies [cmd] to stack from all registers     */
/* stack_la - applies [cmd] to all registers from stack     */

/* cmd*x_** - applies [cmd] to unsigned integer argument(s) */
/* cmd*n_** - applies [cmd] to   signed integer argument(s) */
/* cmdx*_** - applies [cmd] in default  mode                */
/* cmde*_** - applies [cmd] in extended mode (takes DH, DW) */

/******************************************************************************/
/**********************************   X86   ***********************************/
/******************************************************************************/

/* mov */

#define movxx_ri(RM, IM)                                                    \
        EMITB(0xC7)                                                         \
            MRM(0x00,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITW(VAL(IM)))

#define movxx_mi(RM, DP, IM)                                                \
        EMITB(0xC7)                                                         \
            MRM(0x00,    MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMITW(VAL(IM)))

#define movxx_rr(RG, RM)                                                    \
        EMITB(0x8B)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))

#define movxx_ld(RG, RM, DP)                                                \
        EMITB(0x8B)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define movxx_st(RG, RM, DP)                                                \
        EMITB(0x89)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define adrxx_ld(RG, RM, DP) /* only 10-bit offsets and 4-byte alignment */ \
        EMITB(0x8D)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), EMITW(VAL(DP) & 0x3FC), EMPTY)

#define adrxx_lb(lb) /* load label to Reax */                               \
        adrxx_xl(lb)

#define label_ld(lb) /* load label to Reax */                               \
        adrxx_xl(lb)

#define stack_st(RM)                                                        \
        EMITB(0xFF)                                                         \
            MRM(0x06,    MOD(RM), REG(RM))

#define stack_ld(RM)                                                        \
        EMITB(0x8F)                                                         \
            MRM(0x00,    MOD(RM), REG(RM))

#define stack_sa() /* save all [EAX - EDI], 8 regs in total */              \
        EMITB(0x60)

#define stack_la() /* load all [EAX - EDI], 8 regs in total */              \
        EMITB(0x61)

/* and */

#define andxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x04,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   CMD(IM))

#define andxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x04,    MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), CMD(IM))

#define andxx_rr(RG, RM)                                                    \
        EMITB(0x23)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))

#define andxx_ld(RG, RM, DP)                                                \
        EMITB(0x23)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define andxx_st(RG, RM, DP)                                                \
        EMITB(0x21)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* orr */

#define orrxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x01,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   CMD(IM))

#define orrxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x01,    MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), CMD(IM))

#define orrxx_rr(RG, RM)                                                    \
        EMITB(0x0B)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))

#define orrxx_ld(RG, RM, DP)                                                \
        EMITB(0x0B)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define orrxx_st(RG, RM, DP)                                                \
        EMITB(0x09)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* not */

#define notxx_rg(RM)                                                        \
        EMITB(0xF7)                                                         \
            MRM(0x02,    MOD(RM), REG(RM))

#define notxx_rr(RM)                                                        \
        notxx_rg(W(RM))

#define notxx_mm(RM, DP)                                                    \
        EMITB(0xF7)                                                         \
            MRM(0x02,    MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* add */

#define addxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x00,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   CMD(IM))

#define addxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x00,    MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), CMD(IM))

#define addxx_rr(RG, RM)                                                    \
        EMITB(0x03)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))

#define addxx_ld(RG, RM, DP)                                                \
        EMITB(0x03)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define addxx_st(RG, RM, DP)                                                \
        EMITB(0x01)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x05,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   CMD(IM))

#define subxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x05,    MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), CMD(IM))

#define subxx_rr(RG, RM)                                                    \
        EMITB(0x2B)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))

#define subxx_ld(RG, RM, DP)                                                \
        EMITB(0x2B)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define subxx_st(RG, RM, DP)                                                \
        EMITB(0x29)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define subxx_mr(RM, DP, RG)                                                \
        subxx_st(W(RG), W(RM), W(DP))

/* shl */

#define shlxx_ri(RM, IM)                                                    \
        EMITB(0xC1)                                                         \
            MRM(0x04,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(VAL(IM)))

#define shlxx_mi(RM, DP, IM)                                                \
        EMITB(0xC1)                                                         \
            MRM(0x04,    MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMITB(VAL(IM)))

/* shr */

#define shrxx_ri(RM, IM)                                                    \
        EMITB(0xC1)                                                         \
            MRM(0x05,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(VAL(IM)))

#define shrxx_mi(RM, DP, IM)                                                \
        EMITB(0xC1)                                                         \
            MRM(0x05,    MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMITB(VAL(IM)))

#define shrxn_ri(RM, IM)                                                    \
        EMITB(0xC1)                                                         \
            MRM(0x07,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   EMITB(VAL(IM)))

#define shrxn_mi(RM, DP, IM)                                                \
        EMITB(0xC1)                                                         \
            MRM(0x07,    MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMITB(VAL(IM)))

/* mul */

#define mulxn_ri(RM, IM)                                                    \
        EMITB(0x69 | TYP(IM))                                               \
            MRM(0x00,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   CMD(IM))

#define mulxn_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xAF)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))

#define mulxn_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xAF)                                             \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define mulxn_xm(RM, DP) /* Reax is in/out, destroys Redx */                \
        EMITB(0xF7)                                                         \
            MRM(0x05,    MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* div */

#define divxn_xm(RM, DP) /* Reax is in/out, Redx is Reax-sign-extended */   \
        EMITB(0xF7)      /* destroys Xmm0 (in ARM) */                       \
            MRM(0x07,    MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* cmp */

#define cmpxx_ri(RM, IM)                                                    \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x07,    MOD(RM), REG(RM))                                  \
            AUX(EMPTY,   EMPTY,   CMD(IM))

#define cmpxx_mi(RM, DP, IM)                                                \
        EMITB(0x81 | TYP(IM))                                               \
            MRM(0x07,    MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), CMD(IM))

#define cmpxx_rr(RG, RM)                                                    \
        EMITB(0x3B)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))

#define cmpxx_rm(RG, RM, DP)                                                \
        EMITB(0x3B)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

#define cmpxx_mr(RM, DP, RG)                                                \
        EMITB(0x39)                                                         \
            MRM(REG(RG), MOD(RM), REG(RM))                                  \
            AUX(SIB(RM), CMD(DP), EMPTY)

/* jmp */

#define jmpxx_mm(RM, DP)                                                    \
        EMITB(0xFF)                                                         \
            MRM(0x04,    MOD(RM), REG(RM))                                  \
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
        ASM_BEG ASM_OP1(jb,  lb) ASM_END

#define jlexx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jbe, lb) ASM_END

#define jgtxx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(ja,  lb) ASM_END

#define jgexx_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jae, lb) ASM_END

#define jltxn_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jl,  lb) ASM_END

#define jlexn_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jle, lb) ASM_END

#define jgtxn_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jg,  lb) ASM_END

#define jgexn_lb(lb)                                                        \
        ASM_BEG ASM_OP1(jge, lb) ASM_END

#define LBL(lb)                                                             \
        ASM_BEG ASM_OP0(lb:) ASM_END

#endif /* RT_RTARCH_X86_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
