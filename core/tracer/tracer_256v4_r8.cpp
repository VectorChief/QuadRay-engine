/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_REGS
#define RT_REGS 8   /* define maximum of available SIMD registers for code */
#endif /* RT_REGS */

#undef  RT_SIMD
#define RT_SIMD 256  /* map vector-length-agnostic SIMD subsets to 256-bit */
#define RT_SIMD_CODE /* enable SIMD instruction definitions */

#if (defined RT_256_R8) && (RT_256_R8 & 4)
#define RT_RENDER_CODE /* enable contents of render0 routine */
#endif /* RT_256_R8 */

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

#define RT_256_R8 4

#include "tracer.h"
#include "format.h"

#undef  RT_FEAT_PT
#undef  RT_FEAT_BUFFERS
#define RT_FEAT_PT                  0
#define RT_FEAT_BUFFERS             0

#undef  STORE_SIMD
#undef  PAINT_FRAG
#undef  GET_RANDOM_I

namespace rt_simd_256v4_r8
{
#include "tracer.cpp"
}

#undef  RT_FEAT_PT
#undef  RT_FEAT_BUFFERS
#define RT_FEAT_PT                  1
#define RT_FEAT_BUFFERS             1

#undef  STORE_SIMD
#undef  PAINT_FRAG
#undef  GET_RANDOM_I

namespace pt_simd_256v4_r8
{
#include "tracer.cpp"
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
