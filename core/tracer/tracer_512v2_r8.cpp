/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_REGS
#define RT_REGS 8   /* define maximum of available SIMD registers for code */
#endif /* RT_REGS */

#undef  RT_SIMD
#define RT_SIMD 512  /* map vector-length-agnostic SIMD subsets to 512-bit */
#define RT_SIMD_CODE /* enable SIMD instruction definitions */

#if (defined RT_512_R8) && (RT_512_R8 & 2)
#define RT_RENDER_CODE /* enable contents of render0 routine */
#endif /* RT_512_R8 */

/*
 * Determine SIMD total-quads for backend structs (maximal for a given build).
 */
#if (defined RT_MAXQ)
/* RT_MAXQ is already defined outside */
#elif           (RT_2K8_R8)
#define RT_MAXQ 16
#elif (RT_1K4 || RT_1K4_R8)
#define RT_MAXQ 8
#elif (RT_512 || RT_512_R8)
#define RT_MAXQ 4
#elif (RT_256 || RT_256_R8)
#define RT_MAXQ 2
#elif (RT_128)
#define RT_MAXQ 1
#endif /* RT_MAXQ: 16, 8, 4, 2, 1 */

#undef  RT_2K8_R8
#undef  RT_1K4
#undef  RT_1K4_R8
#undef  RT_512
#undef  RT_512_R8
#undef  RT_256
#undef  RT_256_R8
#undef  RT_128

#define RT_512_R8 2

#include "tracer.h"
#include "format.h"

namespace simd_512v2_r8
{
#include "tracer.cpp"
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
