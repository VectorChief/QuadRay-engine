/******************************************************************************/
/* Copyright (c) 2013-2017 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#undef  RT_SIMD
#define RT_SIMD 128 /* map vector-length-agnostic SIMD subsets to 128-bit */
#define RT_SIMD_CODE /* enable SIMD instruction definitions */

#if defined (RT_128) && (RT_128 & 4)
#undef  RT_128
#define RT_128 4
#define RT_RENDER_CODE /* enable contents of render0 routine */
#endif /* RT_128 */

#if defined (RT_ARM)
#undef  RT_ARM
#define RT_ARM 2 /* enable int-div for ASIMDv2 target on ARM */
#endif /* RT_ARM */

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

namespace simd_128v4
{
#include "tracer.cpp"
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
