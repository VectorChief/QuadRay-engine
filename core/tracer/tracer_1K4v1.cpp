/******************************************************************************/
/* Copyright (c) 2013-2017 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#undef  RT_SIMD
#define RT_SIMD 1024 /* map vector-length-agnostic SIMD subsets to 1024-bit */
#define RT_SIMD_CODE /* enable SIMD instruction definitions */

#if defined (RT_1K4) && (RT_1K4 & 1)
#undef  RT_1K4
#define RT_1K4 1
#define RT_RENDER_CODE /* enable contents of render0 routine */
#endif /* RT_1K4 */

#if defined (RT_X32)
#undef  RT_X32
#define RT_X32 2 /* enable BMI1+BMI2 for 1K4-bit AVX3 target on x32 */
#endif /* RT_X32 */

#if defined (RT_X64)
#undef  RT_X64
#define RT_X64 2 /* enable BMI1+BMI2 for 1K4-bit AVX3 target on x64 */
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

namespace simd_1K4v1
{
#include "tracer.cpp"
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/