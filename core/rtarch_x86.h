/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X86_H
#define RT_RTARCH_X86_H

#define EMPTY   ASM_BEG ASM_END

#define MRM(mod, reg, rem, sib)   EMITB((mod) << 6 | (reg) << 3 | (rem)) sib

#define MOD(mod, reg, sib)  mod
#define REG(mod, reg, sib)  reg
#define SIB(mod, reg, sib)  sib

/* registers    MOD,  REG,  SIB */

#define Reax    0x03, 0x00, EMPTY
#define Recx    0x03, 0x01, EMPTY
#define Redx    0x03, 0x02, EMPTY
#define Rebx    0x03, 0x03, EMPTY
#define Resp    0x03, 0x04, EMPTY
#define Rebp    0x03, 0x05, EMPTY
#define Resi    0x03, 0x06, EMPTY
#define Redi    0x03, 0x07, EMPTY

/* addressing   MOD,  REG,  SIB */

#define Oeax    0x00, 0x00, EMPTY       /* [eax] */

#define Mecx    0x02, 0x01, EMPTY       /* [ecx + DP] */
#define Medx    0x02, 0x02, EMPTY       /* [edx + DP] */
#define Mebx    0x02, 0x03, EMPTY       /* [ebx + DP] */
#define Mebp    0x02, 0x05, EMPTY       /* [ebp + DP] */
#define Mesi    0x02, 0x06, EMPTY       /* [esi + DP] */
#define Medi    0x02, 0x07, EMPTY       /* [edi + DP] */

#define Iecx    0x02, 0x04, EMITB(0x01) /* [ecx + eax + DP] */
#define Iedx    0x02, 0x04, EMITB(0x02) /* [edx + eax + DP] */
#define Iebx    0x02, 0x04, EMITB(0x03) /* [ebx + eax + DP] */
#define Iebp    0x02, 0x04, EMITB(0x05) /* [ebp + eax + DP] */
#define Iesi    0x02, 0x04, EMITB(0x06) /* [esi + eax + DP] */
#define Iedi    0x02, 0x04, EMITB(0x07) /* [edi + eax + DP] */

/* immediate */

#define IB(im)                      \
        EMITB(im)

#define IM(im)                      \
        EMITB((im) >> 0x00 & 0xFF)  \
        EMITB((im) >> 0x08 & 0xFF)  \
        EMITB((im) >> 0x10 & 0xFF)  \
        EMITB((im) >> 0x18 & 0xFF)

#define IW(im)  IM(im)

#define DP(im)  IM(im)

#define PLAIN   EMPTY

/******************************************************************************/
/**********************************   X86   ***********************************/
/******************************************************************************/

/* mov */

#define movxx_ri(RM, IM)                                                    \
        EMITB(0xC7) MRM(MOD(RM),    0x00, REG(RM), SIB(RM)) IM

#define movxx_mi(RM, DP, IM)                                                \
        EMITB(0xC7) MRM(MOD(RM),    0x00, REG(RM), SIB(RM)) DP IM

#define movxx_rr(RG, RM)                                                    \
        EMITB(0x8B) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define movxx_ld(RG, RM, DP)                                                \
        EMITB(0x8B) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP

#define movxx_st(RG, RM, DP)                                                \
        EMITB(0x89) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP

#define leaxx_ld(RG, RM, DP)                                                \
        EMITB(0x8D) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP

#define stack_sa()                                                          \
        EMITB(0x60) EMITB(0x9C) 

#define stack_la()                                                          \
        EMITB(0x9D) EMITB(0x61)

/* add */

#define addxx_ri(RM, IM)                                                    \
        EMITB(0x81) MRM(MOD(RM),    0x00, REG(RM), SIB(RM)) IM

#define addxx_mi(RM, DP, IM)                                                \
        EMITB(0x81) MRM(MOD(RM),    0x00, REG(RM), SIB(RM)) DP IM

#define addxx_rr(RG, RM)                                                    \
        EMITB(0x03) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define addxx_ld(RG, RM, DP)                                                \
        EMITB(0x03) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP

#define addxx_st(RG, RM, DP)                                                \
        EMITB(0x01) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP

/* sub */

#define subxx_ri(RM, IM)                                                    \
        EMITB(0x81) MRM(MOD(RM),    0x05, REG(RM), SIB(RM)) IM

#define subxx_mi(RM, DP, IM)                                                \
        EMITB(0x81) MRM(MOD(RM),    0x05, REG(RM), SIB(RM)) DP IM

#define subxx_rr(RG, RM)                                                    \
        EMITB(0x2B) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define subxx_ld(RG, RM, DP)                                                \
        EMITB(0x2B) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP

#define subxx_st(RG, RM, DP)                                                \
        EMITB(0x29) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP

/* and */

#define andxx_ri(RM, IM)                                                    \
        EMITB(0x81) MRM(MOD(RM),    0x04, REG(RM), SIB(RM)) IM

#define andxx_mi(RM, DP, IM)                                                \
        EMITB(0x81) MRM(MOD(RM),    0x04, REG(RM), SIB(RM)) DP IM

#define andxx_rr(RG, RM)                                                    \
        EMITB(0x23) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define andxx_ld(RG, RM, DP)                                                \
        EMITB(0x23) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP

#define andxx_st(RG, RM, DP)                                                \
        EMITB(0x21) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP

/* orr */

#define orrxx_ri(RM, IM)                                                    \
        EMITB(0x81) MRM(MOD(RM),    0x01, REG(RM), SIB(RM)) IM

#define orrxx_mi(RM, DP, IM)                                                \
        EMITB(0x81) MRM(MOD(RM),    0x01, REG(RM), SIB(RM)) DP IM

#define orrxx_rr(RG, RM)                                                    \
        EMITB(0x0B) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM))

#define orrxx_ld(RG, RM, DP)                                                \
        EMITB(0x0B) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP

#define orrxx_st(RG, RM, DP)                                                \
        EMITB(0x09) MRM(MOD(RM), REG(RG), REG(RM), SIB(RM)) DP

/* shl */

#define shlxx_ri(RM, IB)                                                    \
        EMITB(0xC1) MRM(MOD(RM),    0x04, REG(RM), SIB(RM)) IB

#define shlxx_mi(RM, DP, IB)                                                \
        EMITB(0xC1) MRM(MOD(RM),    0x04, REG(RM), SIB(RM)) DP IB

/* shr */

#define shrxx_ri(RM, IB)                                                    \
        EMITB(0xC1) MRM(MOD(RM),    0x05, REG(RM), SIB(RM)) IB

#define shrxx_mi(RM, DP, IB)                                                \
        EMITB(0xC1) MRM(MOD(RM),    0x05, REG(RM), SIB(RM)) DP IB

#endif /* RT_RTARCH_X86_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
