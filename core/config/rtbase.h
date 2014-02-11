/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTBASE_H
#define RT_RTBASE_H

#include <math.h>
#include <float.h>
#include <stdlib.h>

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtbase.h: Base type definitions file.
 *
 * Recommended naming scheme for C++ types and definitions:
 *
 * - All scalar type names start with rt_ followed by type's specific name
 *   in lower case in a form of rt_****. For example: rt_cell or rt_vec4.
 *
 * - All structure names start with rt_ followed by structure's specific name
 *   in upper case with _ used as separator for complex names.
 *   All SIMD-aligned structures used in backend start with rt_SIMD_ prefix.
 *   For example: rt_ELEM or rt_SIMD_INFOX.
 *
 * - All class names start with rt_ followed by class's specific name
 *   in camel case without separator. For example: rt_Scene or rt_SceneThread.
 *
 * - All function names including class methods are in lower case with _ used
 *   as separator for complex names. For example: update_slice or render_fps.
 *
 * - All function type names start with rt_FUNC_ followed by function type's
 *   specific name in upper case with _ used as separator for complex names.
 *   For example: rt_FUNC_INIT or rt_FUNC_UPDATE.
 *
 * - All preprocessor definition names and macros start with RT_ followed by
 *   specific name in upper case with _ used as separator for complex names.
 *   For example: RT_ALIGN or RT_ARR_SIZE.
 */

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

/* Generic types */

typedef float               rt_real;

typedef float               rt_vec2[2];
typedef float               rt_mat2[2][2];

typedef float               rt_vec3[3];
typedef float               rt_mat3[3][3];

typedef float               rt_vec4[4];
typedef float               rt_mat4[4][4];

typedef int                 rt_bool;

typedef char                rt_char;
typedef int                 rt_cell;
typedef long                rt_long; /* time */

typedef unsigned char       rt_byte;
typedef unsigned short      rt_half;
typedef unsigned int        rt_word;

typedef void                rt_void;
typedef void               *rt_pntr;

typedef const char          rt_astr[];
typedef const char         *rt_pstr;

/* Complex types */

struct rt_SIMD_INFO
{
    /* general purpose constants */

    rt_real gpc01[4];       /* +1.0 */
#define inf_GPC01           DP(0x000)

    rt_real gpc02[4];       /* -0.5 */
#define inf_GPC02           DP(0x010)

    rt_real gpc03[4];       /* +3.0 */
#define inf_GPC03           DP(0x020)

    rt_word gpc04[4];       /* 0x7FFFFFFF */
#define inf_GPC04           DP(0x030)

    rt_word gpc05[4];       /* 0x3F800000 */
#define inf_GPC05           DP(0x040)

    rt_real pad01[20];      /* reserved, do not use! */
#define inf_PAD01           DP(0x050)

    /* internal variables */

    rt_word fctrl;
#define inf_FCTRL           DP(0x0A0)

    rt_word pad02[23];      /* reserved, do not use! */
#define inf_PAD02           DP(0x0A4)

};

/* Generic definitions */

#define RT_NULL             0
#define RT_ALIGN            4

#define RT_FALSE            0
#define RT_TRUE             1

/* Generic macros */

#define RT_ARR_SIZE(a)      (sizeof(a) / sizeof(a[0]))

#define RT_MIN(a, b)        ((a) < (b) ? (a) : (b))
#define RT_MAX(a, b)        ((a) > (b) ? (a) : (b))

/* Vector components */

#define RT_X                0
#define RT_Y                1
#define RT_Z                2
#define RT_W                3   /* W - World coords */

#define RT_I                0
#define RT_J                1
#define RT_K                2
#define RT_L                3   /* L - Local coords */

#define RT_R                0
#define RT_G                1
#define RT_B                2
#define RT_A                3   /* A - Alpha channel */

/* Math definitions */

#define RT_INF              FLT_MAX

#define RT_PI               3.14159265358
#define RT_2_PI             (2.0 * RT_PI)
#define RT_PI_2             (RT_PI / 2.0)

/* Math macros */

#define RT_ABS(a)           (abs((rt_cell)(a)))

#define RT_FABS(a)          (fabsf(a))

#define RT_FLOOR(a)         ((rt_cell)floorf(a))

#define RT_SIGN(a)          ((a)  <    0.0f ? -1 :                          \
                             (a)  >    0.0f ? +1 :                          \
                              0)

#define RT_SQRT(a)          ((a)  <=   0.0f ?  0.0f :                       \
                             sqrtf(a))

#define RT_ASIN(a)          ((a)  <=  -1.0f ? -(rt_real)RT_PI_2 :           \
                             (a)  >=  +1.0f ? +(rt_real)RT_PI_2 :           \
                             asinf(a))

#define RT_ACOS(a)          ((a)  <=  -1.0f ? +(rt_real)RT_PI :             \
                             (a)  >=  +1.0f ?  0.0f :                       \
                             acosf(a))

#define RT_SINA(a)          ((a) == -270.0f ? +1.0f :                       \
                             (a) == -180.0f ?  0.0f :                       \
                             (a) ==  -90.0f ? -1.0f :                       \
                             (a) ==    0.0f ?  0.0f :                       \
                             (a) ==  +90.0f ? +1.0f :                       \
                             (a) == +180.0f ?  0.0f :                       \
                             (a) == +270.0f ? -1.0f :                       \
                             sinf((rt_real)((a) * RT_PI / 180.0)))

#define RT_COSA(a)          ((a) == -270.0f ?  0.0f :                       \
                             (a) == -180.0f ? -1.0f :                       \
                             (a) ==  -90.0f ?  0.0f :                       \
                             (a) ==    0.0f ? +1.0f :                       \
                             (a) ==  +90.0f ?  0.0f :                       \
                             (a) == +180.0f ? -1.0f :                       \
                             (a) == +270.0f ?  0.0f :                       \
                             cosf((rt_real)((a) * RT_PI / 180.0)))

#endif /* RT_RTBASE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
