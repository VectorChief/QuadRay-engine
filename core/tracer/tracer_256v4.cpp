/******************************************************************************/
/* Copyright (c) 2013-2018 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#undef  RT_REGS
#define RT_REGS 32  /* define maximum of available SIMD registers for code */

#undef  RT_SIMD
#define RT_SIMD 256  /* map vector-length-agnostic SIMD subsets to 256-bit */
#define RT_SIMD_CODE /* enable SIMD instruction definitions */

#if (defined RT_256) && (RT_256 & 4)
#define RT_RENDER_CODE /* enable contents of render0 routine */
#endif /* RT_256 */

#undef  RT_256
#define RT_256 4

#include "tracer.h"
#include "format.h"

namespace simd_256v4
{
#include "tracer.cpp"
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
