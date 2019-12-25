/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_H
#define RT_RTARCH_H

/******************************************************************************/
/***************************   OS, COMPILER, ARCH   ***************************/
/******************************************************************************/

/*******************************   WIN32, MSC   *******************************/

#if   defined (WIN32)

/* ---------------------------------   X86   -------------------------------- */

#if   defined (RT_X86)

#define ASM_OP0(op)             op
#define ASM_OP1(op, p1)         op  p1
#define ASM_OP2(op, p1, p2)     op  p1, p2

#define ASM_BEG             __asm                                           \
                            {

#define ASM_END             }

#define ASM_ENTER(info)     __asm                                           \
                            {                                               \
                                stack_sa()                                  \
                                ASM_BEG ASM_OP2(lea, ebp, info) ASM_END

#define ASM_LEAVE(info)         stack_la()                                  \
                            }

#define EMITB(B)                ASM_BEG ASM_OP1(_emit, B) ASM_END
#define label_ld(LB)            ASM_BEG ASM_OP2(lea, eax, LB) ASM_END

#include "rtarch_x86_sse.h"

/* ---------------------------------   ARM   -------------------------------- */

#elif defined (RT_ARM)



#endif /* RT_X86, RT_ARM */

/*******************************   LINUX, GCC   *******************************/

#elif defined (linux)

/* ---------------------------------   X86   -------------------------------- */

#if   defined (RT_X86)

#define ASM_OP0(op)             #op
#define ASM_OP1(op, p1)         #op"  "#p1
#define ASM_OP2(op, p1, p2)     #op"  "#p2", "#p1

#define ASM_BEG                 ""

#define ASM_END                 "\n"

#define ASM_ENTER(info)     asm volatile                                    \
                            (                                               \
                                stack_sa()                                  \
                                ASM_BEG ASM_OP2(movl, %%ebp, %[info]) ASM_END

#define ASM_LEAVE(info)         stack_la()                                  \
                              :                                             \
                              : [info] "r" (&info)                          \
                              : "cc",  "memory"                             \
                            );

#define EMITB(B)                ASM_BEG ASM_OP1(.byte, B) ASM_END
#define label_ld(LB)            ASM_BEG ASM_OP2(leal, %%eax, LB) ASM_END

#include "rtarch_x86_sse.h"

/* ---------------------------------   ARM   -------------------------------- */

#elif defined (RT_ARM)

#define ASM_OP0(op)             #op
#define ASM_OP1(op, p1)         #op"  "#p1
#define ASM_OP2(op, p1, p2)     #op"  "#p1", "#p2

#define ASM_BEG                 ""

#define ASM_END                 "\n"

#define ASM_ENTER(info)     asm volatile                                    \
                            (                                               \
                                stack_sa()                                  \
                                ASM_BEG ASM_OP2(mov, r5, %[info]) ASM_END

#define ASM_LEAVE(info)         stack_la()                                  \
                              :                                             \
                              : [info] "r" (&info)                          \
                              : "cc",  "memory",                            \
                                "d0",  "d1",  "d2",  "d3",                  \
                                "d4",  "d5",  "d6",  "d7",                  \
                                "d8",  "d9",  "d10", "d11",                 \
                                "d12", "d13", "d14", "d15",                 \
                                "d16", "d17", "d18", "d19",                 \
                                "d20", "d21"                                \
                            );

#define EMITB(B)                ASM_BEG ASM_OP1(.byte, B) ASM_END
#define label_ld(LB)            ASM_BEG ASM_OP2(adr, r0, LB) ASM_END

#include "rtarch_arm_mpe.h"

#endif /* RT_X86, RT_ARM */

#endif /* OS, COMPILER, ARCH */

/******************************************************************************/
/********************************   LOGGING   *********************************/
/******************************************************************************/

#include "stdio.h"

#define RT_LOGI     printf
#define RT_LOGE     printf

#endif /* RT_RTARCH_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
