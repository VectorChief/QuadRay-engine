/******************************************************************************/
/* Copyright (c) 2013-2015 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include "tracer.h"
#include "format.h"
#if RT_DEBUG >= 1
#include "system.h"
#endif /* RT_DEBUG */

#undef  RT_SIMD_WIDTH
#undef  RT_SIMD_ALIGN
#undef  RT_SIMD_SET
#define RT_SIMD_CODE /* enable SIMD instructions definitions */

#if   defined (RT_256) && (RT_256 & 2)
#undef  RT_256
#define RT_256 2
#define RT_RENDER_CODE /* enable contents of render0 routine */
#endif /* RT_256 */

#if   defined (RT_X86)
#undef RT_RTARCH_X86_256_H
#include "rtarch_x86_256.h"
#elif defined (RT_X32)
#undef RT_RTARCH_X32_256_H
#include "rtarch_x32_256.h"
#elif defined (RT_ARM)
#error "AArch32 doesn't support SIMD wider than 4, \
exclude this file from compilation"
#elif defined (RT_A32)
#error "AArch64 doesn't support SIMD wider than 4, \
exclude this file from compilation"
#elif defined (RT_M32)
#error "mipsMSA doesn't support SIMD wider than 4, \
exclude this file from compilation"
#endif /* RT_X86, RT_X32, RT_ARM, RT_A32, RT_M32 */

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
