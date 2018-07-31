/******************************************************************************/
/* Copyright (c) 2013-2018 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#undef  RT_REGS
#define RT_REGS 8   /* define maximum of available SIMD registers for code */

#undef  RT_SIMD
#define RT_SIMD 2048 /* map vector-length-agnostic SIMD subsets to 2K8-bit */
#define RT_SIMD_CODE /* enable SIMD instruction definitions */

#if (defined RT_2K8_R8) && (RT_2K8_R8 & 2)
#define RT_RENDER_CODE /* enable contents of render0 routine */
#endif /* RT_2K8_R8 */

#undef  RT_2K8_R8

#define RT_2K8_R8 2

#include "tracer.h"
#include "format.h"

namespace simd_2K8v2_r8
{
#include "tracer.cpp"
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
