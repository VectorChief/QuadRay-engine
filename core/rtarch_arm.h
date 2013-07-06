/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_ARM_H
#define RT_RTARCH_ARM_H

#define EMPTY   ASM_BEG ASM_END

#define MRM(reg, ren, rem)  ((ren) << 16 | (reg) << 12 | (rem))

#define MOD(mod, reg, sib)  mod
#define REG(mod, reg, sib)  reg
#define SIB(mod, reg, sib)  sib

#define TEG(TG)             REG(TG)

#define EMITW(im)                   \
        EMITB((im) >> 0x00 & 0xFF)  \
        EMITB((im) >> 0x08 & 0xFF)  \
        EMITB((im) >> 0x10 & 0xFF)  \
        EMITB((im) >> 0x18 & 0xFF)

/* registers    MOD,  REG,  SIB */

#define Reax    0x00, 0x00, EMPTY
#define Recx    0x00, 0x01, EMPTY
#define Redx    0x00, 0x02, EMPTY
#define Rebx    0x00, 0x03, EMPTY
#define Resp    0x00, 0x04, EMPTY
#define Rebp    0x00, 0x05, EMPTY
#define Resi    0x00, 0x06, EMPTY
#define Redi    0x00, 0x07, EMPTY

#define TM      0x00, 0x08, EMPTY
#define TI      0x00, 0x09, EMPTY
#define TP      0x00, 0x0A, EMPTY
#define PC      0x00, 0x0F, EMPTY

/* addressing   MOD,  REG,  SIB */

#define Oeax    0x00, 0x00, EMPTY

#define Mecx    0x01, 0x01, EMPTY
#define Medx    0x02, 0x02, EMPTY
#define Mebx    0x03, 0x03, EMPTY
#define Mebp    0x05, 0x05, EMPTY
#define Mesi    0x06, 0x06, EMPTY
#define Medi    0x07, 0x07, EMPTY

#define Iecx    0x0A, 0x01, EMITW(0xE0800000 | MRM(0x0A, 0x01, 0x00))
#define Iedx    0x0A, 0x02, EMITW(0xE0800000 | MRM(0x0A, 0x02, 0x00))
#define Iebx    0x0A, 0x03, EMITW(0xE0800000 | MRM(0x0A, 0x03, 0x00))
#define Iebp    0x0A, 0x05, EMITW(0xE0800000 | MRM(0x0A, 0x05, 0x00))
#define Iesi    0x0A, 0x06, EMITW(0xE0800000 | MRM(0x0A, 0x06, 0x00))
#define Iedi    0x0A, 0x07, EMITW(0xE0800000 | MRM(0x0A, 0x07, 0x00))

/* immediate */

#define IB(im)  (im)

#define IM(im)  EMITW(0xE3000000 | MRM(TEG(TI), 0x0, 0x0) |                 \
                     (0x000F0000 & (im) <<  4) | (0xFFF & (im)))

#define IW(im)  EMITW(0xE3000000 | MRM(TEG(TI), 0x0, 0x0) |                 \
                     (0x000F0000 & (im) <<  4) | (0xFFF & (im)))            \
                EMITW(0xE3400000 | MRM(TEG(TI), 0x0, 0x0) |                 \
                     (0x000F0000 & (im) >> 12) | (0xFFF & (im) >> 16))

#define DP(im)  (im)

#define PLAIN   DP(0)

/******************************************************************************/
/**********************************   ARM   ***********************************/
/******************************************************************************/

/* mov */

#define movxx_ri(RM, IM)                                                    \
        IM                                                                  \
        EMITW(0xE1A00000 | MRM(REG(RM), 0x00, TEG(TI)))

#define movxx_mi(RM, DP, IM)                                                \
        IM                                                                  \
        SIB(RM)                                                             \
        EMITW(0xE5800000 | MRM(TEG(TI), MOD(RM), 0x00) | DP)

#define movxx_rr(RG, RM)                                                    \
        EMITW(0xE1A00000 | MRM(REG(RG), 0x00, REG(RM)))

#define movxx_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE5900000 | MRM(REG(RG), MOD(RM), 0x00) | DP)

#define movxx_st(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE5800000 | MRM(REG(RG), MOD(RM), 0x00) | DP)

#define leaxx_ld(RG, RM, DP)                                                \
        SIB(RM)                                                             \
        EMITW(0xE2800F00 | MRM(REG(RG), MOD(RM), 0x00) | DP >> 2)

#define stack_sa()                                                          \
        EMITW(0xE92D07FF)

#define stack_la()                                                          \
        EMITW(0xE8BD07FF)

/* add */

#define addxx_ri(RM, IM)                                                    \
        IM                                                                  \
        EMITW(0xE0800000 | MRM(REG(RM), REG(RM), TEG(TI)))

#define addxx_mi(RM, DP, IM)                                                \
        IM                                                                  \
        SIB(RM)                                                             \
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
        IM                                                                  \
        SIB(RM)                                                             \
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
        IM                                                                  \
        SIB(RM)                                                             \
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
        IM                                                                  \
        SIB(RM)                                                             \
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

/* shr */

#define shrxx_ri(RM, IB)                                                    \
        EMITW(0xE1A00020 | MRM(REG(RM), 0x00, REG(RM)) | (0x1F & IB) << 7)

#define shrxx_mi(RM, DP, IB)                                                \
        SIB(RM)                                                             \
        EMITW(0xE5900000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)                \
        EMITW(0xE1A00020 | MRM(REG(RM), 0x00, REG(RM)) | (0x1F & IB) << 7)  \
        EMITW(0xE5800000 | MRM(TEG(TM), MOD(RM), 0x00) | DP)

/* cmp */

#define cmpxx_ri(RM, IM)                                                    \
        IM                                                                  \
        EMITW(0xE1500000 | MRM(0x00, REG(RM), TEG(TI)))

#define cmpxx_mi(RM, DP, IM)                                                \
        IM                                                                  \
        SIB(RM)                                                             \
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

#endif /* RT_RTARCH_ARM_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
