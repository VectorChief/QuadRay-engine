/******************************************************************************/
/* Copyright (c) 2013-2018 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#undef  RT_REGS
#define RT_REGS 16  /* define maximum of available SIMD registers for code */

#undef  RT_SIMD
#define RT_SIMD 1024 /* map vector-length-agnostic SIMD subsets to 1K4-bit */
#define RT_SIMD_CODE /* enable SIMD instruction definitions */

#if (defined RT_1K4) && (RT_1K4 & 1)
#define RT_RENDER_CODE /* enable contents of render0 routine */
#endif /* RT_1K4 */

#undef  RT_1K4
#define RT_1K4 1

#include "tracer.h"
#include "format.h"

namespace simd_1K4v1
{
#include "tracer.cpp"
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
