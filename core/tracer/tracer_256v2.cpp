/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#undef  RT_SIMD
#define RT_SIMD 256 /* map vector-length-agnostic SIMD subsets to 256-bit */
#define RT_SIMD_CODE /* enable SIMD instruction definitions */

#if defined (RT_256) && (RT_256 & 2)
#undef  RT_256
#define RT_256 2
#define RT_RENDER_CODE /* enable contents of render0 routine */
#endif /* RT_256 */

#if defined (RT_X86)
#undef  RT_X86
#define RT_X86 2 /* enable BMI1+BMI2 for 256-bit AVX2 target on x86 */
#endif /* RT_X86 */

#if defined (RT_X32)
#undef  RT_X32
#define RT_X32 2 /* enable BMI1+BMI2 for 256-bit AVX2 target on x32 */
#endif /* RT_X32 */

#if defined (RT_X64)
#undef  RT_X64
#define RT_X64 2 /* enable BMI1+BMI2 for 256-bit AVX2 target on x64 */
#endif /* RT_X64 */

#include "tracer.h"
#include "format.h"
#if RT_DEBUG >= 1
#include "system.h"
#endif /* RT_DEBUG */

/*
 * Global pointer tables
 * for quick entry point resolution.
 */
extern
rt_pntr t_ptr[3];
extern
rt_pntr t_mat[3];
extern
rt_pntr t_clp[3];
extern
rt_pntr t_pow[6];

namespace simd_256v2
{
#include "tracer.cpp"
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
