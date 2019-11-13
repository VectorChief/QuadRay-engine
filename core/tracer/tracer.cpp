/******************************************************************************/
/* Copyright (c) 2013-2019 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifdef RT_SIMD_CODE

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * tracer.cpp: Implementation of the raytracing rendering backend.
 *
 * Raytracing rendering subsystem of the engine responsible for determining
 * pixel colors in the framebuffer by tracing rays of light back from camera
 * through scene objects (surfaces) to light sources.
 *
 * Computation of ray intersections with scene surfaces is written on
 * a unified SIMD macro assembler (rtarch.h) for maximum performance.
 *
 * The efficient use of SIMD is achieved by processing four rays at a time
 * to match SIMD register width (hence the name of the project) as well as
 * implementing carefully crafted SIMD-aligned data structures used together
 * with manual register allocation scheme to avoid unnecessary copying.
 *
 * Unified SIMD macro assembler is designed to be compatible with different
 * processor architectures, while maintaining strictly defined common API,
 * thus application logic can be written and maintained in one place (here).
 */

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

/*
 * Conditional compilation flags
 * for respective segments of code.
 */
#define RT_SHOW_TILES               0
#define RT_SHOW_BOUND               0   /* <- needs RT_OPTS_TILING to be 0 */
#define RT_QUAD_DEBUG               0   /* <- needs RT_DEBUG to be enabled
                                               with RT_THREADS_NUM equal 1 */
#define RT_PLOT_FUNCS_REF           0   /* set to 1 to plot reference code */

#define RT_FEAT_TILING              1
#define RT_FEAT_ANTIALIASING        1   /* <- breaks AA in the engine if 0 */
#define RT_FEAT_MULTITHREADING      1   /* <- breaks MT in the engine if 0 */
#define RT_FEAT_CLIPPING_MINMAX     1   /* <- breaks BB in the engine if 0 */
#define RT_FEAT_CLIPPING_CUSTOM     1   /* <- breaks BB in the engine if 0 */
#define RT_FEAT_CLIPPING_ACCUM      1   /* <- breaks AC in the engine if 0 */
#define RT_FEAT_TEXTURING           1
#define RT_FEAT_NORMALS             1   /* <- breaks LT in the engine if 0 */
#define RT_FEAT_LIGHTS              1
#define RT_FEAT_LIGHTS_COLORED      1
#define RT_FEAT_LIGHTS_AMBIENT      1
#define RT_FEAT_LIGHTS_SHADOWS      1
#define RT_FEAT_LIGHTS_DIFFUSE      1
#define RT_FEAT_LIGHTS_ATTENUATION  1
#define RT_FEAT_LIGHTS_SPECULAR     1
#define RT_FEAT_TRANSPARENCY        1
#define RT_FEAT_REFRACTIONS         1
#define RT_FEAT_REFLECTIONS         1
#define RT_FEAT_FRESNEL             1   /* <- slows down refraction when 1 */
#define RT_FEAT_SCHLICK             0   /* <- low precision Fresnel when 1 */
#define RT_FEAT_FRESNEL_METAL       1   /* apply Fresnel on metal surfaces */
#define RT_FEAT_FRESNEL_METAL_SLOW  0   /* more accurate Fresnel, but slow */
#define RT_FEAT_FRESNEL_PLAIN       1   /* apply Fresnel on plain surfaces */
#define RT_FEAT_GAMMA               1   /* gamma->linear->gamma space if 1 */
#define RT_FEAT_TRANSFORM           1   /* <- breaks TM in the engine if 0 */
#define RT_FEAT_TRANSFORM_ARRAY     1   /* <- breaks TA in the engine if 0 */
#define RT_FEAT_BOUND_VOL_ARRAY     1

#if RT_FEAT_GAMMA
#define GAMMA(x)    x
#else /* RT_FEAT_GAMMA */
#define GAMMA(x)
#endif /* RT_FEAT_GAMMA */

/*
 * Byte-offsets within SIMD-field
 * for packed scalar fields.
 */
#define PTR   0x00 /* LOCAL, PARAM, MAT_P, SRF_T, XMISC */
#define LGT   0x00 /* LST_P */

#define FLG   0x04 /* LOCAL, PARAM, MAT_P, MSC_P, XMISC */
#define SRF   0x04 /* LST_P, SRF_T */

#define LST   0x08 /* LOCAL, PARAM */
#define CLP   0x08 /* MSC_P, SRF_T */

#define OBJ   0x0C /* LOCAL, PARAM, MSC_P */
#define TAG   0x0C /* SRF_T, XMISC */

/*
 * Manual register allocation table
 * for respective segments of code
 * with the following legend:
 *   field above - load register in the middle from
 *   field below - save register in the middle into
 * If register is loaded in the end of one code section
 * it may appear in the table in the beginning of the next section.
 */
/******************************************************************************/
/*    **      list      **    aux-list    **     object     **   aux-object   */
/******************************************************************************/
/*    **    inf_TLS     **                ** elm_SIMD(Mesi) **    inf_CAM     */
/* OO **      Resi      **      Redi      **      Rebx      **      Redx      */
/*    **                **                **                **                */
/******************************************************************************/
/*    **                ** srf_MSC_P(CLP) ** elm_SIMD(Medi) **                */
/* CC **      Resi      **      Redi      **      Rebx      **      Redx      */
/*    **                **                **                **                */
/******************************************************************************/
/*    **                **                ** elm_SIMD(Mesi) ** srf_MAT_P(PTR) */
/* MT **      Resi      **      Redi      **      Rebx      **      Redx      */
/*    ** ctx_LOCAL(LST) **                **                **                */
/******************************************************************************/
/*    **                ** srf_LST_P(LGT) **                **    inf_CAM     */
/* LT **      Resi      **      Redi      **      Rebx      **      Redx      */
/*    **                **                **                **                */
/******************************************************************************/
/*    ** elm_DATA(Medi) **                **                ** elm_SIMD(Medi) */
/* SH **      Resi      **      Redi      **      Rebx      **      Redx      */
/*    **                ** ctx_PARAM(LST) ** ctx_PARAM(OBJ) **                */
/******************************************************************************/
/*    ** srf_LST_P(SRF) ** ctx_PARAM(LST) ** ctx_PARAM(OBJ) ** srf_MAT_P(PTR) */
/* RF **      Resi      **      Redi      **      Rebx      **      Redx      */
/*    **                **                ** ctx_PARAM(OBJ) ** ctx_PARAM(LST) */
/******************************************************************************/
/*    ** srf_LST_P(SRF) **                ** ctx_PARAM(OBJ) ** srf_MAT_P(PTR) */
/* TR **      Resi      **      Redi      **      Rebx      **      Redx      */
/*    **                **                ** ctx_PARAM(OBJ) ** ctx_PARAM(LST) */
/******************************************************************************/
/*    ** ctx_LOCAL(LST) **                ** ctx_PARAM(OBJ) ** ctx_PARAM(LST) */
/* MT **      Resi      **      Redi      **      Rebx      **      Redx      */
/*    **                **                **                **                */
/******************************************************************************/

/******************************************************************************/
/*********************************   MACROS   *********************************/
/******************************************************************************/

/*
 * Highlight frame tiles occupied by surfaces
 * of different types with different colors.
 */
#if   RT_SIMD_QUADS == 1

#if   RT_ELEMENT == 32

#define SHOW_TILES(lb, cl) /* destroys Reax, Xmm0 */                        \
        movyx_ri(Reax, IV(cl))                                              \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x00))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x04))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x08))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x0C))                               \
        PAINT_COLX(10, COL_R(0))                                            \
        PAINT_COLX(08, COL_G(0))                                            \
        PAINT_COLX(00, COL_B(0))

#elif RT_ELEMENT == 64

#define SHOW_TILES(lb, cl) /* destroys Reax, Xmm0 */                        \
        movyx_ri(Reax, IV(cl))                                              \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x00))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x08))                               \
        PAINT_COLX(10, COL_R(0))                                            \
        PAINT_COLX(08, COL_G(0))                                            \
        PAINT_COLX(00, COL_B(0))

#endif /* RT_ELEMENT */

#elif RT_SIMD_QUADS == 2

#if   RT_ELEMENT == 32

#define SHOW_TILES(lb, cl) /* destroys Reax, Xmm0 */                        \
        movyx_ri(Reax, IV(cl))                                              \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x00))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x04))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x08))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x0C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x10))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x14))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x18))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x1C))                               \
        PAINT_COLX(10, COL_R(0))                                            \
        PAINT_COLX(08, COL_G(0))                                            \
        PAINT_COLX(00, COL_B(0))

#elif RT_ELEMENT == 64

#define SHOW_TILES(lb, cl) /* destroys Reax, Xmm0 */                        \
        movyx_ri(Reax, IV(cl))                                              \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x00))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x08))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x10))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x18))                               \
        PAINT_COLX(10, COL_R(0))                                            \
        PAINT_COLX(08, COL_G(0))                                            \
        PAINT_COLX(00, COL_B(0))

#endif /* RT_ELEMENT */

#elif RT_SIMD_QUADS == 4

#if   RT_ELEMENT == 32

#define SHOW_TILES(lb, cl) /* destroys Reax, Xmm0 */                        \
        movyx_ri(Reax, IV(cl))                                              \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x00))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x04))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x08))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x0C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x10))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x14))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x18))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x1C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x20))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x24))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x28))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x2C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x30))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x34))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x38))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x3C))                               \
        PAINT_COLX(10, COL_R(0))                                            \
        PAINT_COLX(08, COL_G(0))                                            \
        PAINT_COLX(00, COL_B(0))

#elif RT_ELEMENT == 64

#define SHOW_TILES(lb, cl) /* destroys Reax, Xmm0 */                        \
        movyx_ri(Reax, IV(cl))                                              \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x00))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x08))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x10))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x18))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x20))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x28))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x30))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x38))                               \
        PAINT_COLX(10, COL_R(0))                                            \
        PAINT_COLX(08, COL_G(0))                                            \
        PAINT_COLX(00, COL_B(0))

#endif /* RT_ELEMENT */

#elif RT_SIMD_QUADS == 8

#if   RT_ELEMENT == 32

#define SHOW_TILES(lb, cl) /* destroys Reax, Xmm0 */                        \
        movyx_ri(Reax, IV(cl))                                              \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x00))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x04))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x08))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x0C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x10))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x14))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x18))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x1C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x20))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x24))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x28))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x2C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x30))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x34))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x38))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x3C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x40))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x44))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x48))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x4C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x50))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x54))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x58))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x5C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x60))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x64))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x68))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x6C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x70))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x74))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x78))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x7C))                               \
        PAINT_COLX(10, COL_R(0))                                            \
        PAINT_COLX(08, COL_G(0))                                            \
        PAINT_COLX(00, COL_B(0))

#elif RT_ELEMENT == 64

#define SHOW_TILES(lb, cl) /* destroys Reax, Xmm0 */                        \
        movyx_ri(Reax, IV(cl))                                              \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x00))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x08))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x10))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x18))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x20))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x28))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x30))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x38))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x40))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x48))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x50))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x58))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x60))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x68))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x70))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x78))                               \
        PAINT_COLX(10, COL_R(0))                                            \
        PAINT_COLX(08, COL_G(0))                                            \
        PAINT_COLX(00, COL_B(0))

#endif /* RT_ELEMENT */

#elif RT_SIMD_QUADS == 16

#if   RT_ELEMENT == 32

#define SHOW_TILES(lb, cl) /* destroys Reax, Xmm0 */                        \
        movyx_ri(Reax, IV(cl))                                              \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x00))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x04))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x08))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x0C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x10))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x14))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x18))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x1C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x20))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x24))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x28))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x2C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x30))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x34))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x38))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x3C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x40))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x44))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x48))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x4C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x50))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x54))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x58))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x5C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x60))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x64))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x68))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x6C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x70))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x74))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x78))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x7C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x80))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x84))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x88))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x8C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x90))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x94))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x98))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x9C))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xA0))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xA4))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xA8))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xAC))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xB0))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xB4))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xB8))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xBC))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xC0))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xC4))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xC8))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xCC))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xD0))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xD4))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xD8))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xDC))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xE0))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xE4))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xE8))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xEC))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xF0))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xF4))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xF8))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xFC))                               \
        PAINT_COLX(10, COL_R(0))                                            \
        PAINT_COLX(08, COL_G(0))                                            \
        PAINT_COLX(00, COL_B(0))

#elif RT_ELEMENT == 64

#define SHOW_TILES(lb, cl) /* destroys Reax, Xmm0 */                        \
        movyx_ri(Reax, IV(cl))                                              \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x00))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x08))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x10))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x18))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x20))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x28))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x30))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x38))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x40))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x48))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x50))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x58))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x60))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x68))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x70))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x78))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x80))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x88))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x90))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x98))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xA0))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xA8))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xB0))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xB8))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xC0))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xC8))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xD0))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xD8))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xE0))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xE8))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xF0))                               \
        movyx_st(Reax, Mecx, ctx_C_BUF(0xF8))                               \
        PAINT_COLX(10, COL_R(0))                                            \
        PAINT_COLX(08, COL_G(0))                                            \
        PAINT_COLX(00, COL_B(0))

#endif /* RT_ELEMENT */

#endif /* RT_SIMD_QUADS */

/*
 * Axis mapping.
 * Perform axis mapping when
 * transform is a multiple of 90 degree rotation.
 */
#define INDEX_AXIS(nx) /* destroys Reax, Xmm0 */                            \
        movwx_ld(Reax, Mebx, srf_A_SGN(nx*4))                               \
        movpx_ld(Xmm0, Iebx, srf_SBASE)                                     \
        movwx_ld(Reax, Mebx, srf_A_MAP(nx*4))

#define MOVXR_LD(XD, MS, DS) /* reads Xmm0 */                               \
        movpx_ld(W(XD), W(MS), W(DS))                                       \
        xorpx_rr(W(XD), Xmm0)

#define MOVXR_ST(XG, MD, DD) /* reads Xmm0, XG, destroys XG */              \
        xorpx_rr(W(XG), Xmm0)                                               \
        movpx_st(W(XG), W(MD), W(DD))

#define MOVZR_ST(XG, MD, DD) /* destroys XG */                              \
        xorpx_rr(W(XG), W(XG))                                              \
        movpx_st(W(XG), W(MD), W(DD))

#define INDEX_TMAP(nx) /* destroys Reax */                                  \
        movwx_ld(Reax, Medx, mat_T_MAP(nx*4))

/*
 * Axis clipping.
 * Check if axis clipping (minmax) is needed for given axis "nx",
 * jump to "lb" otherwise.
 */
#define CHECK_CLIP(lb, pl, nx)                                              \
        cmjwx_mz(Mebx, srf_##pl(nx*4),                                      \
                 EQ_x, lb)

/*
 * Custom clipping.
 * Apply custom clipping (by surface) to ray's hit point
 * based on the side of the clipping surface.
 */
#define APPLY_CLIP(lb, XG, XS) /* destroys Reax, XG */                      \
        movxx_ld(Reax, Medi, elm_DATA)                                      \
        cmjxx_rz(Reax,                                                      \
                 LT_n, lb##_cs1)                /* signed comparison */     \
        cleps_rr(W(XG), W(XS))                                              \
        jmpxx_lb(lb##_cs2)                                                  \
    LBL(lb##_cs1)                                                           \
        cgeps_rr(W(XG), W(XS))                                              \
    LBL(lb##_cs2)

/*
 * Context flags.
 * Value bit-range must not overlap with material props (defined in tracer.h),
 * as they are packed together into the same context field.
 * Current CHECK_FLAG macro (defined below) accepts values upto 8-bit.
 */
#define RT_FLAG_SIDE_OUTER      0
#define RT_FLAG_SIDE_INNER      1
#define RT_FLAG_SIDE            1

#define RT_FLAG_PASS_BACK       0
#define RT_FLAG_PASS_THRU       2
#define RT_FLAG_PASS            2

#define RT_FLAG_SHAD            4

/*
 * Check if flag "fl" is set in the context's field "pl",
 * jump to "lb" otherwise.
 */
#define CHECK_FLAG(lb, pl, fl) /* destroys Reax */                          \
        movxx_ld(Reax, Mecx, ctx_##pl(FLG))                                 \
        arjxx_ri(Reax, IB(fl),                                              \
        and_x,   EZ_x, lb)

/*
 * Combine ray's attack side with ray's pass behavior (back or thru),
 * so that secondary ray emitted from the same surface doesn't
 * hit it from the wrong side due to computational inaccuracy,
 * jump to "lb" if ray doesn't originate from the same surface,
 * jump to "lo" if ray cannot possibly hit the same surface from side "sd"
 * under given circumstances (even though it would due to hit inaccuracy).
 */
#define CHECK_SIDE(lb, lo, sd) /* destroys Reax */                          \
        cmjxx_rm(Rebx, Mecx, ctx_PARAM(OBJ),                                \
                 NE_x, lb)                                                  \
        movxx_ld(Reax, Mecx, ctx_PARAM(FLG))                                \
        andxx_ri(Reax, IB(RT_FLAG_SIDE | RT_FLAG_PASS))                     \
        cmjxx_ri(Reax, IB(1 - sd),                                          \
                 EQ_x, lo)                                                  \
        cmjxx_ri(Reax, IB(2 + sd),                                          \
                 EQ_x, lo)                                                  \
    LBL(lb)

/*
 * Check if ray is a shadow ray, then check
 * material properties to see if shadow is applicable.
 * After applying previously computed shadow mask
 * check if all rays within SIMD are already in the shadow,
 * if so skip the rest of the shadow list.
 */
#define CHECK_SHAD(lb) /* destroys Reax, Xmm7 */                            \
        CHECK_FLAG(lb, PARAM, RT_FLAG_SHAD)                                 \
        CHECK_PROP(lb##_lgt, RT_PROP_LIGHT)                                 \
        movwx_ld(Reax, Mecx, ctx_LOCAL(PTR))                                \
        cmjwx_ri(Reax, IB(1),                                               \
                 EQ_x, SR_rt1)                                              \
        cmjwx_ri(Reax, IB(2),                                               \
                 EQ_x, SR_rt2)                                              \
        cmjwx_ri(Reax, IB(4),                                               \
                 EQ_x, SR_rt4)                                              \
        cmjwx_ri(Reax, IB(6),                                               \
                 EQ_x, SR_rt6)                                              \
    LBL(lb##_lgt)                                                           \
        CHECK_PROP(lb##_trn, RT_PROP_TRANSP)                                \
        CHECK_PROP(lb##_rfr, RT_PROP_REFRACT)                               \
        jmpxx_lb(lb##_trn)                                                  \
    LBL(lb##_rfr)                                                           \
        movwx_ld(Reax, Mecx, ctx_LOCAL(PTR))                                \
        cmjwx_ri(Reax, IB(1),                                               \
                 EQ_x, SR_rt1)                                              \
        cmjwx_ri(Reax, IB(2),                                               \
                 EQ_x, SR_rt2)                                              \
        cmjwx_ri(Reax, IB(4),                                               \
                 EQ_x, SR_rt4)                                              \
        cmjwx_ri(Reax, IB(6),                                               \
                 EQ_x, SR_rt6)                                              \
    LBL(lb##_trn)                                                           \
        movpx_ld(Xmm7, Mecx, ctx_C_BUF(0))                                  \
        orrpx_ld(Xmm7, Mecx, ctx_TMASK(0))                                  \
        movpx_st(Xmm7, Mecx, ctx_C_BUF(0))                                  \
        CHECK_MASK(OO_out, FULL, Xmm7)                                      \
        movwx_ld(Reax, Mecx, ctx_LOCAL(PTR))                                \
        cmjwx_ri(Reax, IB(1),                                               \
                 EQ_x, SR_rt1)                                              \
        cmjwx_ri(Reax, IB(2),                                               \
                 EQ_x, SR_rt2)                                              \
        cmjwx_ri(Reax, IB(4),                                               \
                 EQ_x, SR_rt4)                                              \
        cmjwx_ri(Reax, IB(6),                                               \
                 EQ_x, SR_rt6)                                              \
    LBL(lb)

/*
 * Material properties.
 * Fetch properties from material into the context's local FLG field
 * based on the currently set SIDE flag, also load SIDE flag
 * as sign into Xmm7 for normals.
 */
#define FETCH_PROP() /* destroys Reax, Xmm7 */                              \
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))                                \
        mulxx_ri(Reax, IM(Q*16))                                            \
        movpx_ld(Xmm7, Iebx, srf_SBASE)                                     \
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))                                \
        shlxx_ri(Reax, IB(2+P))                                             \
        movxx_ld(Reax, Iebx, srf_MAT_P(FLG))                                \
        orrxx_st(Reax, Mecx, ctx_LOCAL(FLG))

/*
 * Check if property "pr" previously
 * fetched from material is set in the context,
 * jump to "lb" otherwise.
 */
#define CHECK_PROP(lb, pr) /* destroys Reax */                              \
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))                                \
        arjxx_ri(Reax, IH(pr),                                              \
        and_x,   EZ_x, lb)

/*
 * Fetch pointer into given register "RD" from surface's field "pl"
 * based on the currently set original SIDE flag.
 */
#define FETCH_XPTR(RD, pl) /* destroys Reax */                              \
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))                                \
        andxx_ri(Reax, IB(RT_FLAG_SIDE))                                    \
        shlxx_ri(Reax, IB(2+P))                                             \
        movxx_ld(W(RD), Iebx, srf_##pl)

/*
 * Fetch pointer into given register "RD" from surface's field "pl"
 * based on the currently set inverted SIDE flag.
 */
#define FETCH_IPTR(RD, pl) /* destroys Reax */                              \
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))                                \
        notxx_rx(Reax)                                                      \
        andxx_ri(Reax, IB(RT_FLAG_SIDE))                                    \
        shlxx_ri(Reax, IB(2+P))                                             \
        movxx_ld(W(RD), Iebx, srf_##pl)

/*
 * Update relevant fragments of the
 * given SIMD-field based on the current SIMD-mask.
 */
#define STORE_SIMD(pl, XS) /* destroys Xmm0, 0-masked XS frags */           \
        movpx_ld(Xmm0, Mecx, ctx_TMASK(0))                                  \
        mmvpx_st(W(XS), Mecx, ctx_##pl(0))

/*
 * Update relevant fragments of the
 * color and depth SIMD-fields accumulating values
 * over multiple surfaces from the respective SIMD-fields
 * in the context based on the current SIMD-mask and
 * the current depth values. Also perform
 * pointer dereferencing for color fetching.
 */
#define PAINT_FRAG(lb, pn) /* destroys Reax */                              \
        cmjyx_mz(Mecx, ctx_TMASK(0x##pn),                                   \
                 EQ_x, lb##pn)                                              \
        movyx_ld(Reax, Mecx, ctx_T_VAL(0x##pn))                             \
        movyx_st(Reax, Mecx, ctx_T_BUF(0x##pn))                             \
        movyx_ld(Reax, Mecx, ctx_C_PTR(0x##pn))                             \
        addxx_ld(Reax, Medx, mat_TEX_P)                                     \
        movwx_ld(Reax, Oeax, PLAIN)                                         \
        movyx_st(Reax, Mecx, ctx_C_BUF(0x##pn))                             \
    LBL(lb##pn)

#define PAINT_COLX(cl, pl) /* destroys Reax, Xmm0, reads Xmm2, Xmm7 */      \
        movpx_ld(Xmm0, Mecx, ctx_C_BUF(0))                                  \
        shrpx_ri(Xmm0, IB(0x##cl))                                          \
        andpx_rr(Xmm0, Xmm7)                                                \
        cvnpn_rr(Xmm0, Xmm0)                                                \
        divps_rr(Xmm0, Xmm2)                                                \
        CHECK_PROP(GM_p##cl, RT_PROP_GAMMA)                                 \
  GAMMA(mulps_rr(Xmm0, Xmm0)) /* gamma-to-linear colorspace conversion */   \
    LBL(GM_p##cl)                                                           \
        movpx_st(Xmm0, Mecx, ctx_##pl)

#if   RT_SIMD_QUADS == 1

#if   RT_ELEMENT == 32

#define PAINT_SIMD(lb, XS) /* destroys Reax, Xmm0, Xmm2, Xmm7 */            \
        movpx_st(W(XS), Mecx, ctx_C_PTR(0))                                 \
        PAINT_FRAG(lb, 00)                                                  \
        PAINT_FRAG(lb, 04)                                                  \
        PAINT_FRAG(lb, 08)                                                  \
        PAINT_FRAG(lb, 0C)                                                  \
        movpx_ld(Xmm2, Medx, mat_CLAMP)                                     \
        movpx_ld(Xmm7, Medx, mat_CMASK)                                     \
        PAINT_COLX(10, TEX_R)                                               \
        PAINT_COLX(08, TEX_G)                                               \
        PAINT_COLX(00, TEX_B)

#elif RT_ELEMENT == 64

#define PAINT_SIMD(lb, XS) /* destroys Reax, Xmm0, Xmm2, Xmm7 */            \
        movpx_st(W(XS), Mecx, ctx_C_PTR(0))                                 \
        PAINT_FRAG(lb, 00)                                                  \
        PAINT_FRAG(lb, 08)                                                  \
        movpx_ld(Xmm2, Medx, mat_CLAMP)                                     \
        movpx_ld(Xmm7, Medx, mat_CMASK)                                     \
        PAINT_COLX(10, TEX_R)                                               \
        PAINT_COLX(08, TEX_G)                                               \
        PAINT_COLX(00, TEX_B)

#endif /* RT_ELEMENT */

#elif RT_SIMD_QUADS == 2

#if   RT_ELEMENT == 32

#define PAINT_SIMD(lb, XS) /* destroys Reax, Xmm0, Xmm2, Xmm7 */            \
        movpx_st(W(XS), Mecx, ctx_C_PTR(0))                                 \
        PAINT_FRAG(lb, 00)                                                  \
        PAINT_FRAG(lb, 04)                                                  \
        PAINT_FRAG(lb, 08)                                                  \
        PAINT_FRAG(lb, 0C)                                                  \
        PAINT_FRAG(lb, 10)                                                  \
        PAINT_FRAG(lb, 14)                                                  \
        PAINT_FRAG(lb, 18)                                                  \
        PAINT_FRAG(lb, 1C)                                                  \
        movpx_ld(Xmm2, Medx, mat_CLAMP)                                     \
        movpx_ld(Xmm7, Medx, mat_CMASK)                                     \
        PAINT_COLX(10, TEX_R)                                               \
        PAINT_COLX(08, TEX_G)                                               \
        PAINT_COLX(00, TEX_B)

#elif RT_ELEMENT == 64

#define PAINT_SIMD(lb, XS) /* destroys Reax, Xmm0, Xmm2, Xmm7 */            \
        movpx_st(W(XS), Mecx, ctx_C_PTR(0))                                 \
        PAINT_FRAG(lb, 00)                                                  \
        PAINT_FRAG(lb, 08)                                                  \
        PAINT_FRAG(lb, 10)                                                  \
        PAINT_FRAG(lb, 18)                                                  \
        movpx_ld(Xmm2, Medx, mat_CLAMP)                                     \
        movpx_ld(Xmm7, Medx, mat_CMASK)                                     \
        PAINT_COLX(10, TEX_R)                                               \
        PAINT_COLX(08, TEX_G)                                               \
        PAINT_COLX(00, TEX_B)

#endif /* RT_ELEMENT */

#elif RT_SIMD_QUADS == 4

#if   RT_ELEMENT == 32

#define PAINT_SIMD(lb, XS) /* destroys Reax, Xmm0, Xmm2, Xmm7 */            \
        movpx_st(W(XS), Mecx, ctx_C_PTR(0))                                 \
        PAINT_FRAG(lb, 00)                                                  \
        PAINT_FRAG(lb, 04)                                                  \
        PAINT_FRAG(lb, 08)                                                  \
        PAINT_FRAG(lb, 0C)                                                  \
        PAINT_FRAG(lb, 10)                                                  \
        PAINT_FRAG(lb, 14)                                                  \
        PAINT_FRAG(lb, 18)                                                  \
        PAINT_FRAG(lb, 1C)                                                  \
        PAINT_FRAG(lb, 20)                                                  \
        PAINT_FRAG(lb, 24)                                                  \
        PAINT_FRAG(lb, 28)                                                  \
        PAINT_FRAG(lb, 2C)                                                  \
        PAINT_FRAG(lb, 30)                                                  \
        PAINT_FRAG(lb, 34)                                                  \
        PAINT_FRAG(lb, 38)                                                  \
        PAINT_FRAG(lb, 3C)                                                  \
        movpx_ld(Xmm2, Medx, mat_CLAMP)                                     \
        movpx_ld(Xmm7, Medx, mat_CMASK)                                     \
        PAINT_COLX(10, TEX_R)                                               \
        PAINT_COLX(08, TEX_G)                                               \
        PAINT_COLX(00, TEX_B)

#elif RT_ELEMENT == 64

#define PAINT_SIMD(lb, XS) /* destroys Reax, Xmm0, Xmm2, Xmm7 */            \
        movpx_st(W(XS), Mecx, ctx_C_PTR(0))                                 \
        PAINT_FRAG(lb, 00)                                                  \
        PAINT_FRAG(lb, 08)                                                  \
        PAINT_FRAG(lb, 10)                                                  \
        PAINT_FRAG(lb, 18)                                                  \
        PAINT_FRAG(lb, 20)                                                  \
        PAINT_FRAG(lb, 28)                                                  \
        PAINT_FRAG(lb, 30)                                                  \
        PAINT_FRAG(lb, 38)                                                  \
        movpx_ld(Xmm2, Medx, mat_CLAMP)                                     \
        movpx_ld(Xmm7, Medx, mat_CMASK)                                     \
        PAINT_COLX(10, TEX_R)                                               \
        PAINT_COLX(08, TEX_G)                                               \
        PAINT_COLX(00, TEX_B)

#endif /* RT_ELEMENT */

#elif RT_SIMD_QUADS == 8

#if   RT_ELEMENT == 32

#define PAINT_SIMD(lb, XS) /* destroys Reax, Xmm0, Xmm2, Xmm7 */            \
        movpx_st(W(XS), Mecx, ctx_C_PTR(0))                                 \
        PAINT_FRAG(lb, 00)                                                  \
        PAINT_FRAG(lb, 04)                                                  \
        PAINT_FRAG(lb, 08)                                                  \
        PAINT_FRAG(lb, 0C)                                                  \
        PAINT_FRAG(lb, 10)                                                  \
        PAINT_FRAG(lb, 14)                                                  \
        PAINT_FRAG(lb, 18)                                                  \
        PAINT_FRAG(lb, 1C)                                                  \
        PAINT_FRAG(lb, 20)                                                  \
        PAINT_FRAG(lb, 24)                                                  \
        PAINT_FRAG(lb, 28)                                                  \
        PAINT_FRAG(lb, 2C)                                                  \
        PAINT_FRAG(lb, 30)                                                  \
        PAINT_FRAG(lb, 34)                                                  \
        PAINT_FRAG(lb, 38)                                                  \
        PAINT_FRAG(lb, 3C)                                                  \
        PAINT_FRAG(lb, 40)                                                  \
        PAINT_FRAG(lb, 44)                                                  \
        PAINT_FRAG(lb, 48)                                                  \
        PAINT_FRAG(lb, 4C)                                                  \
        PAINT_FRAG(lb, 50)                                                  \
        PAINT_FRAG(lb, 54)                                                  \
        PAINT_FRAG(lb, 58)                                                  \
        PAINT_FRAG(lb, 5C)                                                  \
        PAINT_FRAG(lb, 60)                                                  \
        PAINT_FRAG(lb, 64)                                                  \
        PAINT_FRAG(lb, 68)                                                  \
        PAINT_FRAG(lb, 6C)                                                  \
        PAINT_FRAG(lb, 70)                                                  \
        PAINT_FRAG(lb, 74)                                                  \
        PAINT_FRAG(lb, 78)                                                  \
        PAINT_FRAG(lb, 7C)                                                  \
        movpx_ld(Xmm2, Medx, mat_CLAMP)                                     \
        movpx_ld(Xmm7, Medx, mat_CMASK)                                     \
        PAINT_COLX(10, TEX_R)                                               \
        PAINT_COLX(08, TEX_G)                                               \
        PAINT_COLX(00, TEX_B)

#elif RT_ELEMENT == 64

#define PAINT_SIMD(lb, XS) /* destroys Reax, Xmm0, Xmm2, Xmm7 */            \
        movpx_st(W(XS), Mecx, ctx_C_PTR(0))                                 \
        PAINT_FRAG(lb, 00)                                                  \
        PAINT_FRAG(lb, 08)                                                  \
        PAINT_FRAG(lb, 10)                                                  \
        PAINT_FRAG(lb, 18)                                                  \
        PAINT_FRAG(lb, 20)                                                  \
        PAINT_FRAG(lb, 28)                                                  \
        PAINT_FRAG(lb, 30)                                                  \
        PAINT_FRAG(lb, 38)                                                  \
        PAINT_FRAG(lb, 40)                                                  \
        PAINT_FRAG(lb, 48)                                                  \
        PAINT_FRAG(lb, 50)                                                  \
        PAINT_FRAG(lb, 58)                                                  \
        PAINT_FRAG(lb, 60)                                                  \
        PAINT_FRAG(lb, 68)                                                  \
        PAINT_FRAG(lb, 70)                                                  \
        PAINT_FRAG(lb, 78)                                                  \
        movpx_ld(Xmm2, Medx, mat_CLAMP)                                     \
        movpx_ld(Xmm7, Medx, mat_CMASK)                                     \
        PAINT_COLX(10, TEX_R)                                               \
        PAINT_COLX(08, TEX_G)                                               \
        PAINT_COLX(00, TEX_B)

#endif /* RT_ELEMENT */

#elif RT_SIMD_QUADS == 16

#if   RT_ELEMENT == 32

#define PAINT_SIMD(lb, XS) /* destroys Reax, Xmm0, Xmm2, Xmm7 */            \
        movpx_st(W(XS), Mecx, ctx_C_PTR(0))                                 \
        PAINT_FRAG(lb, 00)                                                  \
        PAINT_FRAG(lb, 04)                                                  \
        PAINT_FRAG(lb, 08)                                                  \
        PAINT_FRAG(lb, 0C)                                                  \
        PAINT_FRAG(lb, 10)                                                  \
        PAINT_FRAG(lb, 14)                                                  \
        PAINT_FRAG(lb, 18)                                                  \
        PAINT_FRAG(lb, 1C)                                                  \
        PAINT_FRAG(lb, 20)                                                  \
        PAINT_FRAG(lb, 24)                                                  \
        PAINT_FRAG(lb, 28)                                                  \
        PAINT_FRAG(lb, 2C)                                                  \
        PAINT_FRAG(lb, 30)                                                  \
        PAINT_FRAG(lb, 34)                                                  \
        PAINT_FRAG(lb, 38)                                                  \
        PAINT_FRAG(lb, 3C)                                                  \
        PAINT_FRAG(lb, 40)                                                  \
        PAINT_FRAG(lb, 44)                                                  \
        PAINT_FRAG(lb, 48)                                                  \
        PAINT_FRAG(lb, 4C)                                                  \
        PAINT_FRAG(lb, 50)                                                  \
        PAINT_FRAG(lb, 54)                                                  \
        PAINT_FRAG(lb, 58)                                                  \
        PAINT_FRAG(lb, 5C)                                                  \
        PAINT_FRAG(lb, 60)                                                  \
        PAINT_FRAG(lb, 64)                                                  \
        PAINT_FRAG(lb, 68)                                                  \
        PAINT_FRAG(lb, 6C)                                                  \
        PAINT_FRAG(lb, 70)                                                  \
        PAINT_FRAG(lb, 74)                                                  \
        PAINT_FRAG(lb, 78)                                                  \
        PAINT_FRAG(lb, 7C)                                                  \
        PAINT_FRAG(lb, 80)                                                  \
        PAINT_FRAG(lb, 84)                                                  \
        PAINT_FRAG(lb, 88)                                                  \
        PAINT_FRAG(lb, 8C)                                                  \
        PAINT_FRAG(lb, 90)                                                  \
        PAINT_FRAG(lb, 94)                                                  \
        PAINT_FRAG(lb, 98)                                                  \
        PAINT_FRAG(lb, 9C)                                                  \
        PAINT_FRAG(lb, A0)                                                  \
        PAINT_FRAG(lb, A4)                                                  \
        PAINT_FRAG(lb, A8)                                                  \
        PAINT_FRAG(lb, AC)                                                  \
        PAINT_FRAG(lb, B0)                                                  \
        PAINT_FRAG(lb, B4)                                                  \
        PAINT_FRAG(lb, B8)                                                  \
        PAINT_FRAG(lb, BC)                                                  \
        PAINT_FRAG(lb, C0)                                                  \
        PAINT_FRAG(lb, C4)                                                  \
        PAINT_FRAG(lb, C8)                                                  \
        PAINT_FRAG(lb, CC)                                                  \
        PAINT_FRAG(lb, D0)                                                  \
        PAINT_FRAG(lb, D4)                                                  \
        PAINT_FRAG(lb, D8)                                                  \
        PAINT_FRAG(lb, DC)                                                  \
        PAINT_FRAG(lb, E0)                                                  \
        PAINT_FRAG(lb, E4)                                                  \
        PAINT_FRAG(lb, E8)                                                  \
        PAINT_FRAG(lb, EC)                                                  \
        PAINT_FRAG(lb, F0)                                                  \
        PAINT_FRAG(lb, F4)                                                  \
        PAINT_FRAG(lb, F8)                                                  \
        PAINT_FRAG(lb, FC)                                                  \
        movpx_ld(Xmm2, Medx, mat_CLAMP)                                     \
        movpx_ld(Xmm7, Medx, mat_CMASK)                                     \
        PAINT_COLX(10, TEX_R)                                               \
        PAINT_COLX(08, TEX_G)                                               \
        PAINT_COLX(00, TEX_B)

#elif RT_ELEMENT == 64

#define PAINT_SIMD(lb, XS) /* destroys Reax, Xmm0, Xmm2, Xmm7 */            \
        movpx_st(W(XS), Mecx, ctx_C_PTR(0))                                 \
        PAINT_FRAG(lb, 00)                                                  \
        PAINT_FRAG(lb, 08)                                                  \
        PAINT_FRAG(lb, 10)                                                  \
        PAINT_FRAG(lb, 18)                                                  \
        PAINT_FRAG(lb, 20)                                                  \
        PAINT_FRAG(lb, 28)                                                  \
        PAINT_FRAG(lb, 30)                                                  \
        PAINT_FRAG(lb, 38)                                                  \
        PAINT_FRAG(lb, 40)                                                  \
        PAINT_FRAG(lb, 48)                                                  \
        PAINT_FRAG(lb, 50)                                                  \
        PAINT_FRAG(lb, 58)                                                  \
        PAINT_FRAG(lb, 60)                                                  \
        PAINT_FRAG(lb, 68)                                                  \
        PAINT_FRAG(lb, 70)                                                  \
        PAINT_FRAG(lb, 78)                                                  \
        PAINT_FRAG(lb, 80)                                                  \
        PAINT_FRAG(lb, 88)                                                  \
        PAINT_FRAG(lb, 90)                                                  \
        PAINT_FRAG(lb, 98)                                                  \
        PAINT_FRAG(lb, A0)                                                  \
        PAINT_FRAG(lb, A8)                                                  \
        PAINT_FRAG(lb, B0)                                                  \
        PAINT_FRAG(lb, B8)                                                  \
        PAINT_FRAG(lb, C0)                                                  \
        PAINT_FRAG(lb, C8)                                                  \
        PAINT_FRAG(lb, D0)                                                  \
        PAINT_FRAG(lb, D8)                                                  \
        PAINT_FRAG(lb, E0)                                                  \
        PAINT_FRAG(lb, E8)                                                  \
        PAINT_FRAG(lb, F0)                                                  \
        PAINT_FRAG(lb, F8)                                                  \
        movpx_ld(Xmm2, Medx, mat_CLAMP)                                     \
        movpx_ld(Xmm7, Medx, mat_CMASK)                                     \
        PAINT_COLX(10, TEX_R)                                               \
        PAINT_COLX(08, TEX_G)                                               \
        PAINT_COLX(00, TEX_B)

#endif /* RT_ELEMENT */

#endif /* RT_SIMD_QUADS */

/*
 * Prepare all fragments (in packed integer 3-byte form) of
 * the fully computed color values from the context's
 * COL_R, COL_G, COL_B SIMD-fields into the specified location.
 */
#define FRAME_COLX(cl, pl) /* destroys Reax, Xmm0, Xmm1, reads Xmm2, Xmm7 */\
        movpx_ld(Xmm1, Mecx, ctx_##pl(0))                                   \
        CHECK_PROP(GM_f##cl, RT_PROP_GAMMA)                                 \
  GAMMA(sqrps_rr(Xmm1, Xmm1)) /* linear-to-gamma colorspace conversion */   \
    LBL(GM_f##cl)                                                           \
        mulps_rr(Xmm1, Xmm2)                                                \
        cvnps_rr(Xmm1, Xmm1)                                                \
        andpx_rr(Xmm1, Xmm7)                                                \
        shlpx_ri(Xmm1, IB(0x##cl))                                          \
        orrpx_rr(Xmm0, Xmm1)

#define FRAME_SIMD() /* destroys Reax, Xmm0, Xmm1, Xmm2, Xmm7, reads Redx */\
        xorpx_rr(Xmm0, Xmm0)                                                \
        movpx_ld(Xmm2, Medx, cam_CLAMP)                                     \
        movpx_ld(Xmm7, Medx, cam_CMASK)                                     \
        FRAME_COLX(10, COL_R)                                               \
        FRAME_COLX(08, COL_G)                                               \
        FRAME_COLX(00, COL_B)                                               \
        movpx_st(Xmm0, Mecx, ctx_C_BUF(0))

/*
 * Generate next random number (Xmm0, fp: 0.0-1.0) using 32-bit LCG method.
 * Seed (inf_PRNGS) must be initialized outside along with other constants.
 * Only applies to active SIMD elements according to current TMASK.
 */
#define GET_RANDOM() /* destroys Xmm0, Xmm7, Reax */                        \
        movpx_ld(Xmm0, Mecx, ctx_TMASK(0))                                  \
        movxx_ld(Reax, Mebp, inf_PRNGS)                                     \
        movpx_ld(Xmm7, Oeax, PLAIN)                                         \
        mulpx_ld(Xmm7, Mebp, inf_PRNGF)                                     \
        addpx_ld(Xmm7, Mebp, inf_PRNGA)                                     \
        mmvpx_st(Xmm7, Oeax, PLAIN)                                         \
        movpx_rr(Xmm0, Xmm7)                                                \
        movpx_ld(Xmm7, Mebp, inf_PRNGM)                                     \
        shrpx_ri(Xmm0, IB(16))                                              \
        andpx_rr(Xmm0, Xmm7)                                                \
        cvnpn_rr(Xmm0, Xmm0)                                                \
        cvnpn_rr(Xmm7, Xmm7)                                                \
        addps_ld(Xmm7, Mebp, inf_GPC01)                                     \
        divps_rr(Xmm0, Xmm7)

/*
 * Calculate power series approximation for sin.
 */
#define sinps_rr(XD, XS, T1) /* destroys XS, T1 */                          \
        mulps3rr(W(T1), W(XS), W(XS))                                       \
        movpx_rr(W(XD), W(XS))                                              \
        mulps3rr(W(XS), W(XD), W(T1))                                       \
        fmaps_ld(W(XD), W(XS), Mebp, inf_SIN_3)                             \
        mulps3rr(W(XS), W(XS), W(T1))                                       \
        fmaps_ld(W(XD), W(XS), Mebp, inf_SIN_5)                             \
        mulps3rr(W(XS), W(XS), W(T1))                                       \
        fmaps_ld(W(XD), W(XS), Mebp, inf_SIN_7)                             \
        mulps3rr(W(XS), W(XS), W(T1))                                       \
        fmaps_ld(W(XD), W(XS), Mebp, inf_SIN_9)

/*
 * Calculate power series approximation for cos.
 */
#define cosps_rr(XD, XS, T1) /* destroys XS, T1 */                          \
        mulps3rr(W(T1), W(XS), W(XS))                                       \
        movpx_ld(W(XD), Mebp, inf_GPC01)                                    \
        mulps3rr(W(XS), W(XD), W(T1))                                       \
        fmaps_ld(W(XD), W(XS), Mebp, inf_GPC02)                             \
        mulps3rr(W(XS), W(XS), W(T1))                                       \
        fmaps_ld(W(XD), W(XS), Mebp, inf_COS_4)                             \
        mulps3rr(W(XS), W(XS), W(T1))                                       \
        fmaps_ld(W(XD), W(XS), Mebp, inf_COS_6)                             \
        mulps3rr(W(XS), W(XS), W(T1))                                       \
        fmaps_ld(W(XD), W(XS), Mebp, inf_COS_8)

/*
 * Replicate subroutine calling behaviour
 * by saving a given return address tag "tg" in the context's
 * local PTR field, then jumping to the destination address "to".
 * The destination code segment uses saved return address tag
 * to jump back after processing is finished. Parameters are
 * passed via context's local FLG field.
 */
#define SUBROUTINE(tg, to) /* destroys Reax */                              \
        movwx_mi(Mecx, ctx_LOCAL(PTR), IB(tg))                              \
        jmpxx_lb(to)                                                        \
    LBL(SR_rt##tg)

/******************************************************************************/
/*********************************   RENDER   *********************************/
/******************************************************************************/

/*
 * Backend's global entry point (hence 0).
 * Render frame based on the data structures
 * prepared by the engine.
 */
rt_void render0(rt_SIMD_INFOX *s_inf)
{
#ifdef RT_RENDER_CODE

#if RT_QUAD_DEBUG == 1

    s_inf->q_dbg = 1;
    s_inf->q_cnt = 0;

#endif /* RT_QUAD_DEBUG */

/******************************************************************************/
/**********************************   ENTER   *********************************/
/******************************************************************************/

    ASM_ENTER(s_inf)

        movxx_ld(Recx, Mebp, inf_CTX)
        movxx_ld(Redx, Mebp, inf_CAM)

        movwx_mi(Mecx, ctx_PARAM(PTR), IB(0))   /* mark XX_end with tag 0 */
        /* ctx_PARAM(FLG) is initialized outside */
        movxx_mi(Mecx, ctx_PARAM(LST), IB(0))
        movxx_mi(Mecx, ctx_PARAM(OBJ), IB(0))

        xorpx_rr(Xmm0, Xmm0)                    /* tmp_v <-     0 */
        adrpx_ld(Reax, Mecx, ctx_LOCAL(0))
        movpx_st(Xmm0, Oeax, PLAIN)             /* tmp_v -> LOCAL */
        addxx_ri(Reax, IM(RT_SIMD_QUADS*16))
        movpx_st(Xmm0, Oeax, PLAIN)             /* tmp_v -> LOCAL */

        /* calculate number of path-tracer samples */
        cmjxx_mz(Mebp, inf_PT_ON,
                 EQ_x, FF_ini)

        movpx_ld(Xmm0, Mebp, inf_PTS_C)
        movpx_ld(Xmm1, Mebp, inf_GPC01)
        addps_rr(Xmm0, Xmm1)
        movpx_st(Xmm0, Mebp, inf_PTS_C)
        rcpps_rr(Xmm2, Xmm0) /* destroys Xmm0 */
        movpx_st(Xmm2, Mebp, inf_PTS_O)
        subps_rr(Xmm1, Xmm2)
        movpx_st(Xmm1, Mebp, inf_PTS_U)

        jmpxx_lb(FF_pts)

    LBL(FF_ini)

        xorpx_rr(Xmm0, Xmm0)
        movpx_st(Xmm0, Mebp, inf_PTS_C)

    LBL(FF_pts)

/******************************************************************************/
/********************************   VER INIT   ********************************/
/******************************************************************************/

#if RT_FEAT_MULTITHREADING

        movxx_ld(Reax, Mebp, inf_INDEX)
        movxx_st(Reax, Mebp, inf_FRM_Y)

#else /* RT_FEAT_MULTITHREADING */

        movxx_mi(Mebp, inf_FRM_Y, IB(0))

#endif /* RT_FEAT_MULTITHREADING */

    LBL(YY_cyc)

        movxx_ld(Reax, Mebp, inf_FRM_Y)
        cmjxx_rm(Reax, Mebp, inf_FRM_H,
                 LT_x, YY_ini)

        jmpxx_lb(YY_out)

    LBL(YY_ini)

        mulxx_ld(Reax, Mebp, inf_FRM_ROW)
        shlxx_ri(Reax, IB(2))
        addxx_ld(Reax, Mebp, inf_FRAME)
        movxx_st(Reax, Mebp, inf_FRM)
        subxx_ld(Reax, Mebp, inf_FRAME)
        shlxx_ri(Reax, IB(L-1))
        addxx_ld(Reax, Mebp, inf_PSEED)
        movxx_st(Reax, Mebp, inf_PRNGS)

/******************************************************************************/
/********************************   HOR INIT   ********************************/
/******************************************************************************/

#if RT_FEAT_TILING

        movxx_ld(Reax, Mebp, inf_FRM_Y)
        prexx_xx()
        divxx_xm(Mebp, inf_TILE_H)
        mulxx_ld(Reax, Mebp, inf_TLS_ROW)
        shlxx_ri(Reax, IB(1+P))
        addxx_ri(Reax, IB(E))
        addxx_ld(Reax, Mebp, inf_TILES)
        movxx_st(Reax, Mebp, inf_TLS)
        movxx_mi(Mebp, inf_TLS_X, IB(0))

#endif /* RT_FEAT_TILING */

        movxx_mi(Mebp, inf_FRM_X, IB(0))

    LBL(XX_cyc)

        movxx_ld(Redx, Mebp, inf_CAM)

        movpx_ld(Xmm0, Medx, cam_T_MAX)         /* tmp_v <- T_MAX */
        movpx_st(Xmm0, Mecx, ctx_T_BUF(0))      /* tmp_v -> T_BUF */

        xorpx_rr(Xmm0, Xmm0)                    /* tmp_v <-     0 */
        movpx_st(Xmm0, Mecx, ctx_C_BUF(0))      /* tmp_v -> C_BUF */
        movpx_st(Xmm0, Mecx, ctx_COL_R(0))      /* tmp_v -> COL_R */
        movpx_st(Xmm0, Mecx, ctx_COL_G(0))      /* tmp_v -> COL_G */
        movpx_st(Xmm0, Mecx, ctx_COL_B(0))      /* tmp_v -> COL_B */

/******************************************************************************/
/********************************   RAY INIT   ********************************/
/******************************************************************************/

        movpx_ld(Xmm0, Mebp, inf_HOR_I)         /* hor_s <- HOR_I */
        movpx_ld(Xmm7, Mebp, inf_VER_I)         /* ver_s <- VER_I */

        addps_ld(Xmm0, Medx, cam_HOR_A)         /* hor_s += HOR_A */
        addps_ld(Xmm7, Medx, cam_VER_A)         /* ver_s += VER_A */

        movpx_ld(Xmm1, Medx, cam_HOR_X)         /* hor_x <- HOR_X */
        movpx_ld(Xmm2, Medx, cam_HOR_Y)         /* hor_y <- HOR_Y */
        movpx_ld(Xmm3, Medx, cam_HOR_Z)         /* hor_z <- HOR_Z */

        mulps_rr(Xmm1, Xmm0)                    /* hor_x *= hor_s */
        mulps_rr(Xmm2, Xmm0)                    /* hor_y *= hor_s */
        mulps_rr(Xmm3, Xmm0)                    /* hor_z *= hor_s */

        movpx_ld(Xmm4, Medx, cam_VER_X)         /* ver_x <- VER_X */
        movpx_ld(Xmm5, Medx, cam_VER_Y)         /* ver_y <- VER_Y */
        movpx_ld(Xmm6, Medx, cam_VER_Z)         /* ver_z <- VER_Z */

        mulps_rr(Xmm4, Xmm7)                    /* ver_x *= ver_s */
        mulps_rr(Xmm5, Xmm7)                    /* ver_y *= ver_s */
        mulps_rr(Xmm6, Xmm7)                    /* ver_z *= ver_s */

        addps_rr(Xmm1, Xmm4)                    /* hor_x += ver_x */
        addps_rr(Xmm2, Xmm5)                    /* hor_y += ver_y */
        addps_rr(Xmm3, Xmm6)                    /* hor_z += ver_z */

        addps_ld(Xmm1, Medx, cam_DIR_X)         /* ray_x += DIR_X */
        addps_ld(Xmm2, Medx, cam_DIR_Y)         /* ray_y += DIR_Y */
        addps_ld(Xmm3, Medx, cam_DIR_Z)         /* ray_z += DIR_Z */

        movpx_st(Xmm1, Mecx, ctx_RAY_X)         /* ray_x -> RAY_X */
        movpx_st(Xmm2, Mecx, ctx_RAY_Y)         /* ray_y -> RAY_Y */
        movpx_st(Xmm3, Mecx, ctx_RAY_Z)         /* ray_z -> RAY_Z */

/******************************************************************************/
/********************************   OBJ LIST   ********************************/
/******************************************************************************/

#if RT_FEAT_TILING

        movxx_ld(Reax, Mebp, inf_TLS_X)
        shlxx_ri(Reax, IB(1+P))
        addxx_ld(Reax, Mebp, inf_TLS)
        movxx_ld(Resi, Oeax, PLAIN)

#else /* RT_FEAT_TILING */

        movxx_ld(Resi, Mebp, inf_LST)

#endif /* RT_FEAT_TILING */

    LBL(OO_cyc)

        cmjxx_rz(Resi,
                 NE_x, OO_ini)

        jmpxx_lb(OO_out)

    LBL(OO_ini)

        movxx_ld(Rebx, Mesi, elm_SIMD)

        /* use local (potentially adjusted)
         * hit point (from unused normal fields)
         * as local diff for secondary rays
         * when originating from the same surface */
        cmjxx_rm(Rebx, Mecx, ctx_PARAM(OBJ),
                 NE_x, OO_loc)

        subxx_ri(Recx, IH(RT_STACK_STEP))

        movpx_ld(Xmm1, Mecx, ctx_NRM_I)
        movpx_ld(Xmm2, Mecx, ctx_NRM_J)
        movpx_ld(Xmm3, Mecx, ctx_NRM_K)
        /* use context's normal fields (NRM)
         * as temporary storage for local HIT */

        addxx_ri(Recx, IH(RT_STACK_STEP))

        movwx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iecx */

        movpx_st(Xmm1, Iecx, ctx_DFF_X)
        movpx_st(Xmm2, Iecx, ctx_DFF_Y)
        movpx_st(Xmm3, Iecx, ctx_DFF_Z)

    LBL(OO_loc)

#if RT_FEAT_TRANSFORM_ARRAY

        cmjwx_mz(Mebx, srf_SRF_T(TAG),
                 LT_n, OO_dff)                  /* signed comparison */

        /* ctx_LOCAL(OBJ) holds trnode's
         * last element for transform caching,
         * caching is not applied if NULL */
        cmjxx_mz(Mecx, ctx_LOCAL(OBJ),
                 EQ_x, OO_dff)

        /* bypass computation for local diff
         * when used with secondary rays
         * originating from the same surface */
        cmjxx_rm(Rebx, Mecx, ctx_PARAM(OBJ),
                 EQ_x, OO_elm)

        movpx_ld(Xmm1, Mecx, ctx_DFF_X)
        movpx_ld(Xmm2, Mecx, ctx_DFF_Y)
        movpx_ld(Xmm3, Mecx, ctx_DFF_Z)

        /* contribute to trnode's POS,
         * subtract as it is subtracted from ORG */
        subps_ld(Xmm1, Mebx, srf_POS_X)
        subps_ld(Xmm2, Mebx, srf_POS_Y)
        subps_ld(Xmm3, Mebx, srf_POS_Z)

        movpx_st(Xmm1, Mecx, ctx_DFF_I)
        movpx_st(Xmm2, Mecx, ctx_DFF_J)
        movpx_st(Xmm3, Mecx, ctx_DFF_K)

    LBL(OO_elm)

        /* check if surface is trnode's
         * last element for transform caching */
        cmjxx_rm(Resi, Mecx, ctx_LOCAL(OBJ),
                 NE_x, OO_trm)

        /* reset ctx_LOCAL(OBJ) if so */
        movxx_mi(Mecx, ctx_LOCAL(OBJ), IB(0))
        jmpxx_lb(OO_trm)

    LBL(OO_dff)

#endif /* RT_FEAT_TRANSFORM_ARRAY */

        /* bypass computation for local diff
         * when used with secondary rays
         * originating from the same surface */
        cmjxx_rm(Rebx, Mecx, ctx_PARAM(OBJ),
                 EQ_x, OO_ray)

        /* compute diff */
        movpx_ld(Xmm1, Mecx, ctx_ORG_X)
        movpx_ld(Xmm2, Mecx, ctx_ORG_Y)
        movpx_ld(Xmm3, Mecx, ctx_ORG_Z)

        subps_ld(Xmm1, Mebx, srf_POS_X)
        subps_ld(Xmm2, Mebx, srf_POS_Y)
        subps_ld(Xmm3, Mebx, srf_POS_Z)

        movpx_st(Xmm1, Mecx, ctx_DFF_X)
        movpx_st(Xmm2, Mecx, ctx_DFF_Y)
        movpx_st(Xmm3, Mecx, ctx_DFF_Z)

#if RT_FEAT_TRANSFORM

        cmjwx_mz(Mebx, srf_A_MAP(RT_L*4),
                 EQ_x, OO_trm)

        /* transform diff */
        movpx_ld(Xmm4, Mebx, srf_TCI_X)
        mulps_rr(Xmm4, Xmm1)
        movpx_ld(Xmm5, Mebx, srf_TCJ_Y)
        mulps_rr(Xmm5, Xmm2)
        movpx_ld(Xmm6, Mebx, srf_TCK_Z)
        mulps_rr(Xmm6, Xmm3)

        /* bypass non-diagonal terms
         * in transform matrix for scaling fastpath */
        cmjwx_mi(Mebx, srf_A_MAP(RT_L*4), IB(1),
                 EQ_x, OO_trd)

        movpx_ld(Xmm0, Mebx, srf_TCI_Y)
        mulps_rr(Xmm0, Xmm2)
        addps_rr(Xmm4, Xmm0)
        movpx_ld(Xmm0, Mebx, srf_TCI_Z)
        mulps_rr(Xmm0, Xmm3)
        addps_rr(Xmm4, Xmm0)

        movpx_ld(Xmm0, Mebx, srf_TCJ_X)
        mulps_rr(Xmm0, Xmm1)
        addps_rr(Xmm5, Xmm0)
        movpx_ld(Xmm0, Mebx, srf_TCJ_Z)
        mulps_rr(Xmm0, Xmm3)
        addps_rr(Xmm5, Xmm0)

        movpx_ld(Xmm0, Mebx, srf_TCK_X)
        mulps_rr(Xmm0, Xmm1)
        addps_rr(Xmm6, Xmm0)
        movpx_ld(Xmm0, Mebx, srf_TCK_Y)
        mulps_rr(Xmm0, Xmm2)
        addps_rr(Xmm6, Xmm0)

    LBL(OO_trd)

#if RT_FEAT_TRANSFORM_ARRAY

        cmjwx_mz(Mebx, srf_SRF_T(TAG),
                 GE_n, OO_srf)                  /* signed comparison */

        movpx_st(Xmm4, Mecx, ctx_DFF_X)
        movpx_st(Xmm5, Mecx, ctx_DFF_Y)
        movpx_st(Xmm6, Mecx, ctx_DFF_Z)

        /* enable transform caching from trnode,
         * init ctx_LOCAL(OBJ) as last element */
        movxx_ld(Reax, Mesi, elm_DATA)
        movxx_st(Reax, Mecx, ctx_LOCAL(OBJ))
        jmpxx_lb(OO_ray)

    LBL(OO_srf)

#endif /* RT_FEAT_TRANSFORM_ARRAY */

        movpx_st(Xmm4, Mecx, ctx_DFF_I)
        movpx_st(Xmm5, Mecx, ctx_DFF_J)
        movpx_st(Xmm6, Mecx, ctx_DFF_K)

#endif /* RT_FEAT_TRANSFORM */

    LBL(OO_ray)

#if RT_FEAT_TRANSFORM

        /* transform ray */
        movpx_ld(Xmm1, Mecx, ctx_RAY_X)
        movpx_ld(Xmm2, Mecx, ctx_RAY_Y)
        movpx_ld(Xmm3, Mecx, ctx_RAY_Z)

        movpx_ld(Xmm4, Mebx, srf_TCI_X)
        mulps_rr(Xmm4, Xmm1)
        movpx_ld(Xmm5, Mebx, srf_TCJ_Y)
        mulps_rr(Xmm5, Xmm2)
        movpx_ld(Xmm6, Mebx, srf_TCK_Z)
        mulps_rr(Xmm6, Xmm3)

        /* bypass non-diagonal terms
         * in transform matrix for scaling fastpath */
        cmjwx_mi(Mebx, srf_A_MAP(RT_L*4), IB(1),
                 EQ_x, OO_trr)

        movpx_ld(Xmm0, Mebx, srf_TCI_Y)
        mulps_rr(Xmm0, Xmm2)
        addps_rr(Xmm4, Xmm0)
        movpx_ld(Xmm0, Mebx, srf_TCI_Z)
        mulps_rr(Xmm0, Xmm3)
        addps_rr(Xmm4, Xmm0)

        movpx_ld(Xmm0, Mebx, srf_TCJ_X)
        mulps_rr(Xmm0, Xmm1)
        addps_rr(Xmm5, Xmm0)
        movpx_ld(Xmm0, Mebx, srf_TCJ_Z)
        mulps_rr(Xmm0, Xmm3)
        addps_rr(Xmm5, Xmm0)

        movpx_ld(Xmm0, Mebx, srf_TCK_X)
        mulps_rr(Xmm0, Xmm1)
        addps_rr(Xmm6, Xmm0)
        movpx_ld(Xmm0, Mebx, srf_TCK_Y)
        mulps_rr(Xmm0, Xmm2)
        addps_rr(Xmm6, Xmm0)

    LBL(OO_trr)

        movpx_st(Xmm4, Mecx, ctx_RAY_I)
        movpx_st(Xmm5, Mecx, ctx_RAY_J)
        movpx_st(Xmm6, Mecx, ctx_RAY_K)

#endif /* RT_FEAT_TRANSFORM */

    LBL(OO_trm)

#if RT_FEAT_BOUND_VOL_ARRAY

        /* only arrays are allowed to have
         * non-zero lower two bits in DATA field
         * for regular surface lists */
        movxx_ld(Reax, Mesi, elm_DATA)
        andxx_ri(Reax, IB(3))
        cmjxx_ri(Reax, IB(1),
                 EQ_x, AR_ptr)

#endif /* RT_FEAT_BOUND_VOL_ARRAY */

        movwx_ld(Reax, Mebx, srf_SRF_T(PTR))

#if RT_FEAT_TRANSFORM_ARRAY

        /* skip trnode elements from the list */
        cmjwx_rz(Reax,
                 NE_x, OO_trl)

        jmpxx_lb(OO_end)

    LBL(OO_trl)

#endif /* RT_FEAT_TRANSFORM_ARRAY */

        cmjwx_ri(Reax, IB(1),
                 EQ_x, PL_ptr)
        cmjwx_ri(Reax, IB(2),
                 EQ_x, QD_ptr)
        cmjwx_ri(Reax, IB(3),
                 EQ_x, TP_ptr)

/******************************************************************************/
/********************************   CLIPPING   ********************************/
/******************************************************************************/

    LBL(CC_clp)

        /* load "t_val" */
        movpx_ld(Xmm1, Mecx, ctx_T_VAL(0))      /* t_val <- T_VAL */

        /* depth testing */
        movpx_ld(Xmm0, Mecx, ctx_T_BUF(0))      /* t_buf <- T_BUF */
        cgtps_rr(Xmm0, Xmm1)                    /* t_buf >! t_val */
        andpx_rr(Xmm7, Xmm0)                    /* tmask &= gmask */

        /* near plane clipping */
        movpx_ld(Xmm0, Mecx, ctx_T_MIN)         /* t_min <- T_MIN */
        cltps_rr(Xmm0, Xmm1)                    /* t_min <! t_val */
        andpx_rr(Xmm7, Xmm0)                    /* tmask &= lmask */

        /* "x" section */
        movpx_ld(Xmm4, Mecx, ctx_RAY_X)         /* ray_x <- RAY_X */
        mulps_rr(Xmm4, Xmm1)                    /* ray_x *= t_val */
        addps_ld(Xmm4, Mecx, ctx_ORG_X)         /* hit_x += ORG_X */
        movpx_st(Xmm4, Mecx, ctx_HIT_X)         /* hit_x -> HIT_X */

        /* "y" section */
        movpx_ld(Xmm5, Mecx, ctx_RAY_Y)         /* ray_y <- RAY_Y */
        mulps_rr(Xmm5, Xmm1)                    /* ray_y *= t_val */
        addps_ld(Xmm5, Mecx, ctx_ORG_Y)         /* hit_y += ORG_Y */
        movpx_st(Xmm5, Mecx, ctx_HIT_Y)         /* hit_y -> HIT_Y */

        /* "z" section */
        movpx_ld(Xmm6, Mecx, ctx_RAY_Z)         /* ray_z <- RAY_Z */
        mulps_rr(Xmm6, Xmm1)                    /* ray_z *= t_val */
        addps_ld(Xmm6, Mecx, ctx_ORG_Z)         /* hit_z += ORG_Z */
        movpx_st(Xmm6, Mecx, ctx_HIT_Z)         /* hit_z -> HIT_Z */

#if RT_FEAT_TRANSFORM

        cmjwx_mz(Mebx, srf_A_MAP(RT_L*4),
                 EQ_x, CC_loc)

        /* "x" section */
        movpx_ld(Xmm4, Mecx, ctx_RAY_I)         /* ray_i <- RAY_I */
        mulps_rr(Xmm4, Xmm1)                    /* ray_i *= t_val */
        addps_ld(Xmm4, Mecx, ctx_DFF_I)         /* ray_i += DFF_I */
        movpx_st(Xmm4, Mecx, ctx_NEW_I)         /* loc_i -> NEW_I */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

        /* "y" section */
        movpx_ld(Xmm5, Mecx, ctx_RAY_J)         /* ray_j <- RAY_J */
        mulps_rr(Xmm5, Xmm1)                    /* ray_j *= t_val */
        addps_ld(Xmm5, Mecx, ctx_DFF_J)         /* ray_j += DFF_J */
        movpx_st(Xmm5, Mecx, ctx_NEW_J)         /* loc_j -> NEW_J */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

        /* "z" section */
        movpx_ld(Xmm6, Mecx, ctx_RAY_K)         /* ray_k <- RAY_K */
        mulps_rr(Xmm6, Xmm1)                    /* ray_k *= t_val */
        addps_ld(Xmm6, Mecx, ctx_DFF_K)         /* ray_k += DFF_K */
        movpx_st(Xmm6, Mecx, ctx_NEW_K)         /* loc_k -> NEW_K */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

        jmpxx_lb(CC_glb)

    LBL(CC_loc)

#endif /* RT_FEAT_TRANSFORM */

        /* "x" section */
        subps_ld(Xmm4, Mebx, srf_POS_X)         /* loc_x -= POS_X */
        movpx_st(Xmm4, Mecx, ctx_NEW_X)         /* loc_x -> NEW_X */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

        /* "y" section */
        subps_ld(Xmm5, Mebx, srf_POS_Y)         /* loc_y -= POS_Y */
        movpx_st(Xmm5, Mecx, ctx_NEW_Y)         /* loc_y -> NEW_Y */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

        /* "z" section */
        subps_ld(Xmm6, Mebx, srf_POS_Z)         /* loc_z -= POS_Z */
        movpx_st(Xmm6, Mecx, ctx_NEW_Z)         /* loc_z -> NEW_Z */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

    LBL(CC_glb)

#if RT_QUAD_DEBUG == 1

        cmjxx_mi(Mebp, inf_Q_DBG, IB(4),
                 NE_x, QD_go4)
        movxx_mi(Mebp, inf_Q_DBG, IB(5))

        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))
        mulxx_ri(Reax, IM(Q*16))
        movpx_ld(Xmm1, Iebx, srf_SBASE)
        movpx_st(Xmm1, Mebp, inf_TSIDE)

        movpx_st(Xmm4, Mebp, inf_HIT_X)
        movpx_st(Xmm5, Mebp, inf_HIT_Y)
        movpx_st(Xmm6, Mebp, inf_HIT_Z)

    LBL(QD_go4)

#endif /* RT_QUAD_DEBUG */

/******************************************************************************/

        /* conic singularity solver */
        cmjxx_mz(Mebx, srf_MSC_P(FLG),
                 EQ_x, CC_adj)

        /* check near-zero determinant */
        cmjwx_mz(Mecx, ctx_XMISC(PTR),
                 EQ_x, CC_adj)

        /* load local point */
        movwx_ld(Reax, Mebx, srf_A_MAP(RT_I*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm1, Iecx, ctx_NEW_O)         /* loc_i <- NEW_I */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        mulps_rr(Xmm1, Xmm1)                    /* loc_i *= loc_i */
        movpx_rr(Xmm0, Xmm1)                    /* loc_r <- lc2_i */

        cmjxx_mi(Mebx, srf_MSC_P(FLG), IB(2),
                 EQ_x, CC_js1)                  /* mask out J axis */

        movwx_ld(Reax, Mebx, srf_A_MAP(RT_J*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm2, Iecx, ctx_NEW_O)         /* loc_j <- NEW_J */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        mulps_rr(Xmm2, Xmm2)                    /* loc_j *= loc_j */
        addps_rr(Xmm0, Xmm2)                    /* loc_r += lc2_j */

    LBL(CC_js1)

        movwx_ld(Reax, Mebx, srf_A_MAP(RT_K*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm3, Iecx, ctx_NEW_O)         /* loc_k <- NEW_K */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        mulps_rr(Xmm3, Xmm3)                    /* loc_k *= loc_k */
        addps_rr(Xmm0, Xmm3)                    /* loc_r += lc2_k */

        /* check distance */
        cltps_ld(Xmm0, Mebx, srf_T_EPS)         /* loc_r <! T_EPS */
        andpx_ld(Xmm0, Mecx, ctx_DMASK)         /* hmask &= DMASK */
        CHECK_MASK(CC_adj, NONE, Xmm0)

        /* prepare values */
        movpx_ld(Xmm6, Mebx, srf_SMASK)         /* smask <- SMASK */
        movpx_ld(Xmm5, Mebp, inf_GPC01)         /* tmp_v <- +1.0f */
        xorpx_rr(Xmm2, Xmm2)                    /* loc_j <-  0.0f */

        /* load diffs & scalers */
        movwx_ld(Reax, Mebx, srf_A_MAP(RT_I*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm1, Iecx, ctx_DFF_O)         /* loc_i <- DFF_I */
        andpx_rr(Xmm1, Xmm6)                    /* loc_i &= smask */
        xorpx_rr(Xmm1, Xmm5)                    /* loc_i ^= tmp_v */
        subwx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iebx */
        movpx_ld(Xmm3, Iebx, srf_SCI_O)         /* sci_k <- SCI_I */
        movpx_rr(Xmm4, Xmm5)                    /* loc_r <- tmp_v */

        cmjxx_mi(Mebx, srf_MSC_P(FLG), IB(2),
                 EQ_x, CC_js2)                  /* mask out J axis */

        movwx_ld(Reax, Mebx, srf_A_MAP(RT_J*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm2, Iecx, ctx_DFF_O)         /* loc_j <- DFF_J */
        andpx_rr(Xmm2, Xmm6)                    /* loc_j &= smask */
        xorpx_rr(Xmm2, Xmm5)                    /* loc_j ^= tmp_v */
        subwx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iebx */
        addps_ld(Xmm3, Iebx, srf_SCI_O)         /* sci_k += SCI_J */
        addps_rr(Xmm4, Xmm5)                    /* loc_r += tmp_v */

    LBL(CC_js2)

        /* evaluate surface */
        movwx_ld(Reax, Mebx, srf_A_MAP(RT_K*4)) /* Reax is used in Iecx */
        subwx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iebx */
        divps_ld(Xmm3, Iebx, srf_SCI_O)         /* sci_k /= SCI_K */
        xorpx_rr(Xmm3, Xmm6)                    /* sci_k = |sci_k|*/
        movpx_rr(Xmm6, Xmm3)                    /* lc2_k <- lc2_k */
        sqrps_rr(Xmm3, Xmm3)                    /* loc_k sq lc2_k */

        /* normalize point */
        addps_rr(Xmm6, Xmm4)                    /* lc2_k += loc_r */
        rsqps_rr(Xmm4, Xmm6) /* destroys Xmm6 *//* inv_r rs loc_r */
        mulps_ld(Xmm4, Mebx, srf_T_EPS)         /* inv_r *= T_EPS */
        mulps_rr(Xmm1, Xmm4)                    /* loc_i *= inv_r */
        mulps_rr(Xmm2, Xmm4)                    /* loc_j *= inv_r */
        mulps_rr(Xmm3, Xmm4)                    /* loc_k *= inv_r */

        /* apply signs */
        movpx_ld(Xmm4, Mecx, ctx_AMASK)         /* amask <- AMASK */
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))
        mulxx_ri(Reax, IM(Q*16))
        movpx_ld(Xmm5, Iebx, srf_SBASE)         /* tside <- TSIDE */

        movwx_ld(Reax, Mebx, srf_A_MAP(RT_K*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm6, Iecx, ctx_DFF_O)         /* dff_k <- DFF_K */
        andpx_ld(Xmm6, Mebx, srf_SMASK)         /* dff_k &= SMASK */
        xorpx_rr(Xmm3, Xmm6)                    /* loc_k ^= signk */

        /* for K axis */
        movpx_rr(Xmm6, Xmm5)                    /* signk <- tside */
        andpx_rr(Xmm6, Xmm4)                    /* signk &= amask */
        xorpx_rr(Xmm6, Xmm4)                    /* signk ^= amask */
        xorpx_rr(Xmm3, Xmm6)                    /* loc_k ^= signk */

        /* for I, J axes */
        orrpx_rr(Xmm5, Xmm4)                    /* tside |= amask */
        xorpx_rr(Xmm5, Xmm4)                    /* tside ^= amask */
        xorpx_rr(Xmm1, Xmm5)                    /* loc_i ^= tsign */
        xorpx_rr(Xmm2, Xmm5)                    /* loc_j ^= tsign */

        /* recombine points */
        movwx_ld(Reax, Mebx, srf_A_MAP(RT_I*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm4, Iecx, ctx_NEW_O)         /* new_i <- NEW_I */
        orrpx_rr(Xmm4, Xmm0)                    /* new_i |= hmask */
        xorpx_rr(Xmm4, Xmm0)                    /* new_i ^= hmask */
        andpx_rr(Xmm1, Xmm0)                    /* loc_i &= hmask */
        orrpx_rr(Xmm4, Xmm1)                    /* new_i |= loc_i */
        movpx_st(Xmm4, Iecx, ctx_NEW_O)         /* new_i -> NEW_I */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

        cmjxx_mi(Mebx, srf_MSC_P(FLG), IB(2),
                 EQ_x, CC_js3)                  /* mask out J axis */

        movwx_ld(Reax, Mebx, srf_A_MAP(RT_J*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm5, Iecx, ctx_NEW_O)         /* new_j <- NEW_J */
        orrpx_rr(Xmm5, Xmm0)                    /* new_j |= hmask */
        xorpx_rr(Xmm5, Xmm0)                    /* new_j ^= hmask */
        andpx_rr(Xmm2, Xmm0)                    /* loc_j &= hmask */
        orrpx_rr(Xmm5, Xmm2)                    /* new_j |= loc_j */
        movpx_st(Xmm5, Iecx, ctx_NEW_O)         /* new_j -> NEW_J */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

    LBL(CC_js3)

        movwx_ld(Reax, Mebx, srf_A_MAP(RT_K*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm6, Iecx, ctx_NEW_O)         /* new_k <- NEW_K */
        orrpx_rr(Xmm6, Xmm0)                    /* new_k |= hmask */
        xorpx_rr(Xmm6, Xmm0)                    /* new_k ^= hmask */
        andpx_rr(Xmm3, Xmm0)                    /* loc_k &= hmask */
        orrpx_rr(Xmm6, Xmm3)                    /* new_k |= loc_k */
        movpx_st(Xmm6, Iecx, ctx_NEW_O)         /* new_k -> NEW_K */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

        /* load adjusted point */
        movwx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm4, Iecx, ctx_NEW_X)         /* loc_x <- NEW_X */
        movpx_ld(Xmm5, Iecx, ctx_NEW_Y)         /* loc_y <- NEW_Y */
        movpx_ld(Xmm6, Iecx, ctx_NEW_Z)         /* loc_z <- NEW_Z */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

    LBL(CC_adj)

#if RT_QUAD_DEBUG == 1

        cmjxx_mi(Mebp, inf_Q_DBG, IB(5),
                 NE_x, QD_go5)
        movxx_mi(Mebp, inf_Q_DBG, IB(6))

        movpx_st(Xmm4, Mebp, inf_ADJ_X)
        movpx_st(Xmm5, Mebp, inf_ADJ_Y)
        movpx_st(Xmm6, Mebp, inf_ADJ_Z)

    LBL(QD_go5)

#endif /* RT_QUAD_DEBUG */

/******************************************************************************/

#if RT_FEAT_CLIPPING_MINMAX

        /* "x" section */
        CHECK_CLIP(CX_min, MIN_T, RT_X)

        movpx_ld(Xmm0, Mebx, srf_MIN_X)         /* min_x <- MIN_X */
        cleps_rr(Xmm0, Xmm4)                    /* min_x <= pos_x */
        andpx_rr(Xmm7, Xmm0)                    /* tmask &= lmask */

    LBL(CX_min)

        CHECK_CLIP(CX_max, MAX_T, RT_X)

        movpx_ld(Xmm0, Mebx, srf_MAX_X)         /* max_x <- MAX_X */
        cgeps_rr(Xmm0, Xmm4)                    /* max_x >= pos_x */
        andpx_rr(Xmm7, Xmm0)                    /* tmask &= gmask */

    LBL(CX_max)

        /* "y" section */
        CHECK_CLIP(CY_min, MIN_T, RT_Y)

        movpx_ld(Xmm0, Mebx, srf_MIN_Y)         /* min_y <- MIN_Y */
        cleps_rr(Xmm0, Xmm5)                    /* min_y <= pos_y */
        andpx_rr(Xmm7, Xmm0)                    /* tmask &= lmask */

    LBL(CY_min)

        CHECK_CLIP(CY_max, MAX_T, RT_Y)

        movpx_ld(Xmm0, Mebx, srf_MAX_Y)         /* max_y <- MAX_Y */
        cgeps_rr(Xmm0, Xmm5)                    /* max_y >= pos_y */
        andpx_rr(Xmm7, Xmm0)                    /* tmask &= gmask */

    LBL(CY_max)

        /* "z" section */
        CHECK_CLIP(CZ_min, MIN_T, RT_Z)

        movpx_ld(Xmm0, Mebx, srf_MIN_Z)         /* min_z <- MIN_Z */
        cleps_rr(Xmm0, Xmm6)                    /* min_z <= pos_z */
        andpx_rr(Xmm7, Xmm0)                    /* tmask &= lmask */

    LBL(CZ_min)

        CHECK_CLIP(CZ_max, MAX_T, RT_Z)

        movpx_ld(Xmm0, Mebx, srf_MAX_Z)         /* max_z <- MAX_Z */
        cgeps_rr(Xmm0, Xmm6)                    /* max_z >= pos_z */
        andpx_rr(Xmm7, Xmm0)                    /* tmask &= gmask */

    LBL(CZ_max)

#endif /* RT_FEAT_CLIPPING_MINMAX */

/******************************************************************************/

#if RT_FEAT_CLIPPING_CUSTOM

        movxx_ld(Reax, Mebx, srf_MSC_P(OBJ))    /* load trnode's simd ptr */
        movxx_st(Reax, Mecx, ctx_LOCAL(LST))    /* save trnode's simd ptr */

        movxx_ri(Redx, IB(0))
        movxx_ld(Redi, Mebx, srf_MSC_P(CLP))

    LBL(CC_cyc)

        cmjxx_rz(Redi,
                 EQ_x, CC_out)

        movxx_ld(Rebx, Medi, elm_SIMD)

#if RT_FEAT_CLIPPING_ACCUM

        cmjxx_rz(Rebx,
                 NE_x, CC_acc)                  /* check accum marker */

        cmjxx_mz(Medi, elm_DATA,                /* check accum enter/leave */
                 GT_n, CC_acl)                  /* signed comparison */

        movpx_st(Xmm7, Mecx, ctx_C_ACC)         /* save current clip mask */
        movxx_ld(Rebx, Mesi, elm_SIMD)
        movpx_ld(Xmm7, Mebx, srf_C_DEF)         /* load default clip mask */
        jmpxx_lb(CC_end)                        /* accum enter */

    LBL(CC_acl)

        annpx_ld(Xmm7, Mecx, ctx_C_ACC)         /* apply accum clip mask */
        jmpxx_lb(CC_end)                        /* accum leave */

    LBL(CC_acc)

#endif /* RT_FEAT_CLIPPING_ACCUM */

#if RT_FEAT_TRANSFORM_ARRAY

        cmjwx_mz(Mebx, srf_SRF_T(TAG),
                 LT_n, CC_arr)                  /* signed comparison */

        /* Redx holds trnode's
         * last element for transform caching,
         * caching is not applied if NULL */
        cmjxx_rz(Redx,
                 EQ_x, CC_dff)

        movpx_ld(Xmm1, Mecx, ctx_NRM_X)
        movpx_ld(Xmm2, Mecx, ctx_NRM_Y)
        movpx_ld(Xmm3, Mecx, ctx_NRM_Z)
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */

        /* contribute to trnode's POS,
         * subtract as it is subtracted from HIT */
        subps_ld(Xmm1, Mebx, srf_POS_X)
        subps_ld(Xmm2, Mebx, srf_POS_Y)
        subps_ld(Xmm3, Mebx, srf_POS_Z)

        movpx_st(Xmm1, Mecx, ctx_NRM_I)
        movpx_st(Xmm2, Mecx, ctx_NRM_J)
        movpx_st(Xmm3, Mecx, ctx_NRM_K)
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */

        /* check if clipper is trnode's
         * last element for transform caching */
        cmjxx_rr(Redi, Redx,
                 NE_x, CC_trm)

        /* reset Redx if so */
        movxx_ri(Redx, IB(0))
        jmpxx_lb(CC_trm)

    LBL(CC_arr)

        /* handle the case when surface and its clippers
         * belong to the same trnode, shortcut transform */
        cmjxx_rm(Rebx, Mecx, ctx_LOCAL(LST),
                 NE_x, CC_dff)

        /* load surface's simd ptr */
        movxx_ld(Rebx, Mesi, elm_SIMD)

        movpx_ld(Xmm1, Mecx, ctx_NEW_I)
        movpx_ld(Xmm2, Mecx, ctx_NEW_J)
        movpx_ld(Xmm3, Mecx, ctx_NEW_K)
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

        /* translate local HIT back to trnode's space,
         * add to cancel surface's POS in NEW (in DFF) */
        addps_ld(Xmm1, Mebx, srf_POS_X)
        addps_ld(Xmm2, Mebx, srf_POS_Y)
        addps_ld(Xmm3, Mebx, srf_POS_Z)

        movpx_st(Xmm1, Mecx, ctx_NRM_X)
        movpx_st(Xmm2, Mecx, ctx_NRM_Y)
        movpx_st(Xmm3, Mecx, ctx_NRM_Z)
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */

        /* enable transform caching from trnode,
         * init Redx as last element */
        movxx_ld(Redx, Medi, elm_DATA)
        jmpxx_lb(CC_end)

    LBL(CC_dff)

#endif /* RT_FEAT_TRANSFORM_ARRAY */

        /* compute diff */
        movpx_ld(Xmm1, Mecx, ctx_HIT_X)
        movpx_ld(Xmm2, Mecx, ctx_HIT_Y)
        movpx_ld(Xmm3, Mecx, ctx_HIT_Z)

        subps_ld(Xmm1, Mebx, srf_POS_X)
        subps_ld(Xmm2, Mebx, srf_POS_Y)
        subps_ld(Xmm3, Mebx, srf_POS_Z)

        movpx_st(Xmm1, Mecx, ctx_NRM_X)
        movpx_st(Xmm2, Mecx, ctx_NRM_Y)
        movpx_st(Xmm3, Mecx, ctx_NRM_Z)
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */

#if RT_FEAT_TRANSFORM

        cmjwx_mz(Mebx, srf_A_MAP(RT_L*4),
                 EQ_x, CC_trm)

        /* transform clip */
        movpx_ld(Xmm4, Mebx, srf_TCI_X)
        mulps_rr(Xmm4, Xmm1)
        movpx_ld(Xmm5, Mebx, srf_TCJ_Y)
        mulps_rr(Xmm5, Xmm2)
        movpx_ld(Xmm6, Mebx, srf_TCK_Z)
        mulps_rr(Xmm6, Xmm3)

        /* bypass non-diagonal terms
         * in transform matrix for scaling fastpath */
        cmjwx_mi(Mebx, srf_A_MAP(RT_L*4), IB(1),
                 EQ_x, CC_trc)

        movpx_ld(Xmm0, Mebx, srf_TCI_Y)
        mulps_rr(Xmm0, Xmm2)
        addps_rr(Xmm4, Xmm0)
        movpx_ld(Xmm0, Mebx, srf_TCI_Z)
        mulps_rr(Xmm0, Xmm3)
        addps_rr(Xmm4, Xmm0)

        movpx_ld(Xmm0, Mebx, srf_TCJ_X)
        mulps_rr(Xmm0, Xmm1)
        addps_rr(Xmm5, Xmm0)
        movpx_ld(Xmm0, Mebx, srf_TCJ_Z)
        mulps_rr(Xmm0, Xmm3)
        addps_rr(Xmm5, Xmm0)

        movpx_ld(Xmm0, Mebx, srf_TCK_X)
        mulps_rr(Xmm0, Xmm1)
        addps_rr(Xmm6, Xmm0)
        movpx_ld(Xmm0, Mebx, srf_TCK_Y)
        mulps_rr(Xmm0, Xmm2)
        addps_rr(Xmm6, Xmm0)

    LBL(CC_trc)

#if RT_FEAT_TRANSFORM_ARRAY

        cmjwx_mz(Mebx, srf_SRF_T(TAG),
                 GE_n, CC_srf)                  /* signed comparison */

        movpx_st(Xmm4, Mecx, ctx_NRM_X)
        movpx_st(Xmm5, Mecx, ctx_NRM_Y)
        movpx_st(Xmm6, Mecx, ctx_NRM_Z)
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */

        /* enable transform caching from trnode,
         * init Redx as last element */
        movxx_ld(Redx, Medi, elm_DATA)
        jmpxx_lb(CC_end)

    LBL(CC_srf)

#endif /* RT_FEAT_TRANSFORM_ARRAY */

        movpx_st(Xmm4, Mecx, ctx_NRM_I)
        movpx_st(Xmm5, Mecx, ctx_NRM_J)
        movpx_st(Xmm6, Mecx, ctx_NRM_K)
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */

#endif /* RT_FEAT_TRANSFORM */

    LBL(CC_trm)

        movwx_ld(Reax, Mebx, srf_SRF_T(CLP))

        cmjwx_ri(Reax, IB(1),
                 EQ_x, PL_clp)
        cmjwx_ri(Reax, IB(2),
                 EQ_x, QD_clp)
        cmjwx_ri(Reax, IB(3),
                 EQ_x, TP_clp)

    LBL(CC_ret)

        andpx_rr(Xmm7, Xmm4)

    LBL(CC_end)

        movxx_ld(Redi, Medi, elm_NEXT)
        jmpxx_lb(CC_cyc)

    LBL(CC_out)

        movxx_ld(Rebx, Mesi, elm_SIMD)

#endif /* RT_FEAT_CLIPPING_CUSTOM */

        movwx_ld(Reax, Mecx, ctx_LOCAL(PTR))

        cmjwx_ri(Reax, IB(0),
                 EQ_x, SR_rt0)
        cmjwx_ri(Reax, IB(3),
                 EQ_x, SR_rt3)
        cmjwx_ri(Reax, IB(5),
                 EQ_x, SR_rt5)

/******************************************************************************/
/********************************   MATERIAL   ********************************/
/******************************************************************************/

#if RT_FEAT_NORMALS

    LBL(MT_nrm)

#if RT_QUAD_DEBUG == 1

        cmjxx_mi(Mebp, inf_Q_DBG, IB(6),
                 NE_x, QD_go6)
        movxx_mi(Mebp, inf_Q_DBG, IB(7))

        movpx_st(Xmm4, Mebp, inf_NRM_X)
        movpx_st(Xmm5, Mebp, inf_NRM_Y)
        movpx_st(Xmm6, Mebp, inf_NRM_Z)

    LBL(QD_go6)

#endif /* RT_QUAD_DEBUG */

#if RT_FEAT_TRANSFORM

        cmjwx_mz(Mebx, srf_A_MAP(RT_L*4),
                 EQ_x, MT_mat)

        movxx_ld(Rebx, Mebx, srf_MSC_P(OBJ))    /* load trnode's simd ptr */

        /* transform normal,
         * apply transposed matrix */
        movpx_ld(Xmm1, Mecx, ctx_NRM_I)
        movpx_ld(Xmm2, Mecx, ctx_NRM_J)
        movpx_ld(Xmm3, Mecx, ctx_NRM_K)

        movpx_ld(Xmm4, Mebx, srf_TCI_X)
        mulps_rr(Xmm4, Xmm1)
        movpx_ld(Xmm5, Mebx, srf_TCJ_Y)
        mulps_rr(Xmm5, Xmm2)
        movpx_ld(Xmm6, Mebx, srf_TCK_Z)
        mulps_rr(Xmm6, Xmm3)

        /* bypass non-diagonal terms
         * in transform matrix for scaling fastpath */
        cmjwx_mi(Mebx, srf_A_MAP(RT_L*4), IB(1),
                 EQ_x, MT_trn)

        movpx_ld(Xmm0, Mebx, srf_TCJ_X)
        mulps_rr(Xmm0, Xmm2)
        addps_rr(Xmm4, Xmm0)
        movpx_ld(Xmm0, Mebx, srf_TCK_X)
        mulps_rr(Xmm0, Xmm3)
        addps_rr(Xmm4, Xmm0)

        movpx_ld(Xmm0, Mebx, srf_TCI_Y)
        mulps_rr(Xmm0, Xmm1)
        addps_rr(Xmm5, Xmm0)
        movpx_ld(Xmm0, Mebx, srf_TCK_Y)
        mulps_rr(Xmm0, Xmm3)
        addps_rr(Xmm5, Xmm0)

        movpx_ld(Xmm0, Mebx, srf_TCI_Z)
        mulps_rr(Xmm0, Xmm1)
        addps_rr(Xmm6, Xmm0)
        movpx_ld(Xmm0, Mebx, srf_TCJ_Z)
        mulps_rr(Xmm0, Xmm2)
        addps_rr(Xmm6, Xmm0)

        /* bypass normal renormalization
         * if scaling is not present in transform matrix */
        cmjwx_mi(Mebx, srf_A_MAP(RT_L*4), IB(2),
                 EQ_x, MT_rnm)

    LBL(MT_trn)

        /* renormalize normal */
        movpx_rr(Xmm1, Xmm4)                    /* loc_x <- loc_x */
        movpx_rr(Xmm2, Xmm5)                    /* loc_y <- loc_y */
        movpx_rr(Xmm3, Xmm6)                    /* loc_z <- loc_z */

        mulps_rr(Xmm1, Xmm4)                    /* loc_x *= loc_x */
        mulps_rr(Xmm2, Xmm5)                    /* loc_y *= loc_y */
        mulps_rr(Xmm3, Xmm6)                    /* loc_z *= loc_z */

        addps_rr(Xmm1, Xmm2)                    /* lc2_x += lc2_y */
        addps_rr(Xmm1, Xmm3)                    /* lc2_t += lc2_z */
        rsqps_rr(Xmm0, Xmm1) /* destroys Xmm1 *//* inv_r rs loc_r */

        mulps_rr(Xmm4, Xmm0)                    /* loc_x *= inv_r */
        mulps_rr(Xmm5, Xmm0)                    /* loc_y *= inv_r */
        mulps_rr(Xmm6, Xmm0)                    /* loc_z *= inv_r */

    LBL(MT_rnm)

        /* store normal */
        movpx_st(Xmm4, Mecx, ctx_NRM_X)         /* loc_x -> NRM_X */
        movpx_st(Xmm5, Mecx, ctx_NRM_Y)         /* loc_y -> NRM_Y */
        movpx_st(Xmm6, Mecx, ctx_NRM_Z)         /* loc_z -> NRM_Z */

        movxx_ld(Rebx, Mesi, elm_SIMD)          /* load surface's simd ptr */

#endif /* RT_FEAT_TRANSFORM */

#endif /* RT_FEAT_NORMALS */

    LBL(MT_mat)

        /* preserve local (potentially adjusted)
         * hit point to unused normal fields
         * for use in secondary rays' contexts */
        movwx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iecx */

        movpx_ld(Xmm4, Iecx, ctx_NEW_X)         /* loc_x <- NEW_X */
        movpx_ld(Xmm5, Iecx, ctx_NEW_Y)         /* loc_y <- NEW_Y */
        movpx_ld(Xmm6, Iecx, ctx_NEW_Z)         /* loc_z <- NEW_Z */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

        movpx_st(Xmm4, Mecx, ctx_NRM_I)         /* loc_x -> NRM_I */
        movpx_st(Xmm5, Mecx, ctx_NRM_J)         /* loc_y -> NRM_J */
        movpx_st(Xmm6, Mecx, ctx_NRM_K)         /* loc_z -> NRM_K */
        /* use context's normal fields (NRM)
         * as temporary storage for local HIT */

        /* process material */
        movxx_st(Resi, Mecx, ctx_LOCAL(LST))

        FETCH_XPTR(Redx, MAT_P(PTR))

        xorpx_rr(Xmm1, Xmm1)                    /* tex_p <-     0 */

#if RT_FEAT_TEXTURING

        CHECK_PROP(MT_tex, RT_PROP_TEXTURE)

        /* transform surface's UV coords
         *        to texture's XY coords */
        INDEX_TMAP(RT_X)
        movpx_ld(Xmm4, Iecx, ctx_TEX_O)         /* tex_x <- TEX_X */
        INDEX_TMAP(RT_Y)
        movpx_ld(Xmm5, Iecx, ctx_TEX_O)         /* tex_y <- TEX_Y */

        /* texture offset */
        subps_ld(Xmm4, Medx, mat_XOFFS)         /* tex_x -= XOFFS */
        subps_ld(Xmm5, Medx, mat_YOFFS)         /* tex_y -= YOFFS */

        /* texture scale */
        mulps_ld(Xmm4, Medx, mat_XSCAL)         /* tex_x *= XSCAL */
        mulps_ld(Xmm5, Medx, mat_YSCAL)         /* tex_y *= YSCAL */

        /* texture mapping */
        cvmps_rr(Xmm1, Xmm4)                    /* tex_x ii tex_x */
        andpx_ld(Xmm1, Medx, mat_XMASK)         /* tex_y &= XMASK */

        cvmps_rr(Xmm2, Xmm5)                    /* tex_y ii tex_y */
        andpx_ld(Xmm2, Medx, mat_YMASK)         /* tex_y &= YMASK */
        shlpx_ld(Xmm2, Medx, mat_YSHFT)         /* tex_y << YSHFT */

        addpx_rr(Xmm1, Xmm2)                    /* tex_x += tex_y */
        shlpx_ri(Xmm1, IB(2))                   /* tex_x <<     2 */

    LBL(MT_tex)

#endif /* RT_FEAT_TEXTURING */

        PAINT_SIMD(MT_rtx, Xmm1)

/******************************************************************************/
/*********************************   LIGHTS   *********************************/
/******************************************************************************/

        cmjxx_mz(Mebp, inf_PT_ON,
                 EQ_x, LT_reg)

        cmjxx_mi(Mebp, inf_DEPTH, IB(RT_STACK_DEPTH - 5),
                 GT_x, PT_cnt)

        movpx_ld(Xmm1, Mecx, ctx_TEX_R)
        movpx_ld(Xmm2, Mecx, ctx_TEX_G)
        movpx_ld(Xmm3, Mecx, ctx_TEX_B)

        movpx_rr(Xmm0, Xmm1)
        cgtps_rr(Xmm0, Xmm2)
        cgtps_rr(Xmm1, Xmm3)
        andpx_rr(Xmm1, Xmm0)

        CHECK_MASK(PT_clR, NONE, Xmm1)

        movpx_ld(Xmm4, Mecx, ctx_TEX_R)
        jmpxx_lb(PT_clB)

    LBL(PT_clR)

        cgtps_rr(Xmm2, Xmm3)

        CHECK_MASK(PT_clG, NONE, Xmm2)

        movpx_ld(Xmm4, Mecx, ctx_TEX_G)
        jmpxx_lb(PT_clB)

    LBL(PT_clG)

        movpx_ld(Xmm4, Mecx, ctx_TEX_B)

    LBL(PT_clB)

        GET_RANDOM() /* -> Xmm0, destroys Xmm7, Reax */

        cltps_rr(Xmm0, Xmm4)
        andpx_ld(Xmm0, Mecx, ctx_TMASK(0))

        CHECK_MASK(PT_chk, NONE, Xmm0)

        jmpxx_lb(PT_tex)

    LBL(PT_chk)

        xorpx_rr(Xmm1, Xmm1)
        xorpx_rr(Xmm2, Xmm2)
        xorpx_rr(Xmm3, Xmm3)

        jmpxx_lb(PT_skp)

    LBL(PT_tex)

        movpx_ld(Xmm1, Mecx, ctx_TEX_R)
        movpx_ld(Xmm2, Mecx, ctx_TEX_G)
        movpx_ld(Xmm3, Mecx, ctx_TEX_B)

        rcpps_rr(Xmm5, Xmm4)
        andpx_rr(Xmm5, Xmm0)

        mulps_rr(Xmm1, Xmm5)
        mulps_rr(Xmm2, Xmm5)
        mulps_rr(Xmm3, Xmm5)

        movpx_st(Xmm1, Mecx, ctx_TEX_R)
        movpx_st(Xmm2, Mecx, ctx_TEX_G)
        movpx_st(Xmm3, Mecx, ctx_TEX_B)

    LBL(PT_cnt)

        /* compute orthonormal basis relative to normal */

        movpx_ld(Xmm1, Mecx, ctx_NRM_X)
        movpx_ld(Xmm2, Mecx, ctx_NRM_Y)
        movpx_ld(Xmm3, Mecx, ctx_NRM_Z)

        movpx_ld(Xmm4, Mecx, ctx_RAY_X)
        movpx_ld(Xmm5, Mecx, ctx_RAY_Y)
        movpx_ld(Xmm6, Mecx, ctx_RAY_Z)

        /* compute 1st cross-product */

        mulps3rr(Xmm0, Xmm2, Xmm6)
        mulps3rr(Xmm7, Xmm3, Xmm5)
        subps_rr(Xmm0, Xmm7)
        movpx_st(Xmm0, Mecx, ctx_TEX_U)         /* 1st vec, X */
        /* use context's available fields
         * as temporary storage for basis */

        mulps3rr(Xmm0, Xmm3, Xmm4)
        mulps3rr(Xmm7, Xmm1, Xmm6)
        subps_rr(Xmm0, Xmm7)
        movpx_st(Xmm0, Mecx, ctx_TEX_V)         /* 1st vec, Y */
        /* use context's available fields
         * as temporary storage for basis */

        mulps3rr(Xmm0, Xmm1, Xmm5)
        mulps3rr(Xmm7, Xmm2, Xmm4)
        subps_rr(Xmm0, Xmm7)
        movpx_st(Xmm0, Mecx, ctx_C_PTR(0))      /* 1st vec, Z */
        /* use context's available fields
         * as temporary storage for basis */

        movpx_ld(Xmm4, Mecx, ctx_TEX_U)         /* 1st vec, X */
        movpx_ld(Xmm5, Mecx, ctx_TEX_V)         /* 1st vec, Y */
        movpx_ld(Xmm6, Mecx, ctx_C_PTR(0))      /* 1st vec, Z */

        /* normalize 1st vector */

        movpx_rr(Xmm1, Xmm4)                    /* loc_x <- loc_x */
        movpx_rr(Xmm2, Xmm5)                    /* loc_y <- loc_y */
        movpx_rr(Xmm3, Xmm6)                    /* loc_z <- loc_z */

        mulps_rr(Xmm1, Xmm4)                    /* loc_x *= loc_x */
        mulps_rr(Xmm2, Xmm5)                    /* loc_y *= loc_y */
        mulps_rr(Xmm3, Xmm6)                    /* loc_z *= loc_z */

        addps_rr(Xmm1, Xmm2)                    /* lc2_x += lc2_y */
        addps_rr(Xmm1, Xmm3)                    /* lc2_t += lc2_z */
        rsqps_rr(Xmm0, Xmm1) /* destroys Xmm1 *//* inv_r rs loc_r */

        mulps_rr(Xmm4, Xmm0)                    /* loc_x *= inv_r */
        mulps_rr(Xmm5, Xmm0)                    /* loc_y *= inv_r */
        mulps_rr(Xmm6, Xmm0)                    /* loc_z *= inv_r */

        movpx_st(Xmm4, Mecx, ctx_TEX_U)         /* 1st vec, X */
        movpx_st(Xmm5, Mecx, ctx_TEX_V)         /* 1st vec, Y */
        movpx_st(Xmm6, Mecx, ctx_C_PTR(0))      /* 1st vec, Z */

        movpx_ld(Xmm1, Mecx, ctx_NRM_X)
        movpx_ld(Xmm2, Mecx, ctx_NRM_Y)
        movpx_ld(Xmm3, Mecx, ctx_NRM_Z)

        /* compute 2nd cross-product */

        mulps3rr(Xmm0, Xmm2, Xmm6)
        mulps3rr(Xmm7, Xmm3, Xmm5)
        subps_rr(Xmm0, Xmm7)
        movpx_st(Xmm0, Mecx, ctx_C_ACC)         /* 2nd vec, X */
        /* use context's available fields
         * as temporary storage for basis */

        mulps3rr(Xmm0, Xmm3, Xmm4)
        mulps3rr(Xmm7, Xmm1, Xmm6)
        subps_rr(Xmm0, Xmm7)
        movpx_st(Xmm0, Mecx, ctx_F_RFL)         /* 2nd vec, Y */
        /* use context's available fields
         * as temporary storage for basis */

        mulps3rr(Xmm0, Xmm1, Xmm5)
        mulps3rr(Xmm7, Xmm2, Xmm4)
        subps_rr(Xmm0, Xmm7)
        movpx_st(Xmm0, Mecx, ctx_T_VAL(0))      /* 2nd vec, Z */
        /* use context's available fields
         * as temporary storage for basis */

        /* compute random sample over hemisphere */

        GET_RANDOM() /* -> Xmm0, destroys Xmm7, Reax */

        movpx_rr(Xmm6, Xmm0)
        movpx_ld(Xmm0, Mebp, inf_GPC01)
        subps_rr(Xmm0, Xmm6)
        sqrps_rr(Xmm6, Xmm6)
        sqrps_rr(Xmm0, Xmm0)

        mulps_rr(Xmm1, Xmm0)
        mulps_rr(Xmm2, Xmm0)
        mulps_rr(Xmm3, Xmm0)

        GET_RANDOM() /* -> Xmm0, destroys Xmm7, Reax */

        addps_rr(Xmm0, Xmm0)
        mulps_ld(Xmm0, Medx, mat_GPC10)
        subps_ld(Xmm0, Medx, mat_GPC10)

        movpx_rr(Xmm5, Xmm0)
        cosps_rr(Xmm4, Xmm5, Xmm7)
        mulps_rr(Xmm4, Xmm6)

        movpx_ld(Xmm5, Mecx, ctx_TEX_U)         /* 1st vec, X */
        mulps_rr(Xmm5, Xmm4)
        addps_rr(Xmm1, Xmm5)
        movpx_ld(Xmm5, Mecx, ctx_TEX_V)         /* 1st vec, Y */
        mulps_rr(Xmm5, Xmm4)
        addps_rr(Xmm2, Xmm5)
        movpx_ld(Xmm5, Mecx, ctx_C_PTR(0))      /* 1st vec, Z */
        mulps_rr(Xmm5, Xmm4)
        addps_rr(Xmm3, Xmm5)

        movpx_rr(Xmm5, Xmm0)
        sinps_rr(Xmm4, Xmm5, Xmm7)
        mulps_rr(Xmm4, Xmm6)

        movpx_ld(Xmm5, Mecx, ctx_C_ACC)         /* 2nd vec, X */
        mulps_rr(Xmm5, Xmm4)
        addps_rr(Xmm1, Xmm5)
        movpx_ld(Xmm5, Mecx, ctx_F_RFL)         /* 2nd vec, Y */
        mulps_rr(Xmm5, Xmm4)
        addps_rr(Xmm2, Xmm5)
        movpx_ld(Xmm5, Mecx, ctx_T_VAL(0))      /* 2nd vec, Z */
        mulps_rr(Xmm5, Xmm4)
        addps_rr(Xmm3, Xmm5)

        movpx_st(Xmm1, Mecx, ctx_NEW_X)         /* new ray, X */
        movpx_st(Xmm2, Mecx, ctx_NEW_Y)         /* new ray, Y */
        movpx_st(Xmm3, Mecx, ctx_NEW_Z)         /* new ray, Z */

        /* recursive light sampling for path-tracer */

        /* prepare default values */
        xorpx_rr(Xmm0, Xmm0)
        movpx_st(Xmm0, Mecx, ctx_T_NEW)

        xorpx_rr(Xmm1, Xmm1)
        xorpx_rr(Xmm2, Xmm2)
        xorpx_rr(Xmm3, Xmm3)

/************************************ ENTER ***********************************/

        cmjxx_mz(Mebp, inf_DEPTH,
                 EQ_x, PT_mix)

        /* consider evaluating multiple samples
         * per hit to speed up image convergence */

        FETCH_XPTR(Resi, LST_P(SRF))

        movpx_ld(Xmm0, Mecx, ctx_TMASK(0))      /* load tmask */
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))
        orrxx_ri(Reax, IB(RT_FLAG_PASS_BACK))
        addxx_ri(Recx, IH(RT_STACK_STEP))
        subxx_mi(Mebp, inf_DEPTH, IB(1))

        movxx_st(Reax, Mecx, ctx_PARAM(FLG))    /* context flags */
        movxx_st(Redx, Mecx, ctx_PARAM(LST))    /* save material */
        movxx_st(Rebx, Mecx, ctx_PARAM(OBJ))    /* originating surface */
        movwx_mi(Mecx, ctx_PARAM(PTR), IB(4))   /* mark PT_ret with tag 4 */
        movpx_st(Xmm0, Mecx, ctx_WMASK)         /* tmask -> WMASK */

        movxx_ld(Redx, Mebp, inf_CAM)
        movpx_ld(Xmm0, Medx, cam_T_MAX)         /* tmp_v <- T_MAX */
        movpx_st(Xmm0, Mecx, ctx_T_BUF(0))      /* tmp_v -> T_BUF */

        xorpx_rr(Xmm0, Xmm0)                    /* tmp_v <-     0 */
        movpx_st(Xmm0, Mecx, ctx_C_BUF(0))      /* tmp_v -> C_BUF */
        movpx_st(Xmm0, Mecx, ctx_COL_R(0))      /* tmp_v -> COL_R */
        movpx_st(Xmm0, Mecx, ctx_COL_G(0))      /* tmp_v -> COL_G */
        movpx_st(Xmm0, Mecx, ctx_COL_B(0))      /* tmp_v -> COL_B */

        movpx_st(Xmm0, Mecx, ctx_T_MIN)         /* tmp_v -> T_MIN */
        adrpx_ld(Reax, Mecx, ctx_LOCAL(0))
        movpx_st(Xmm0, Oeax, PLAIN)             /* tmp_v -> LOCAL */
        addxx_ri(Reax, IM(RT_SIMD_QUADS*16))
        movpx_st(Xmm0, Oeax, PLAIN)             /* tmp_v -> LOCAL */

        jmpxx_lb(OO_cyc)

    LBL(PT_ret)

        movxx_ld(Redx, Mecx, ctx_PARAM(LST))    /* restore material */
        movxx_ld(Rebx, Mecx, ctx_PARAM(OBJ))    /* restore surface */

        movpx_ld(Xmm1, Mecx, ctx_COL_R(0))
        movpx_ld(Xmm2, Mecx, ctx_COL_G(0))
        movpx_ld(Xmm3, Mecx, ctx_COL_B(0))

        addxx_mi(Mebp, inf_DEPTH, IB(1))
        subxx_ri(Recx, IH(RT_STACK_STEP))

        movpx_ld(Xmm0, Medx, mat_L_DFF)

        mulps_rr(Xmm1, Xmm0)
        mulps_rr(Xmm2, Xmm0)
        mulps_rr(Xmm3, Xmm0)

/************************************ LEAVE ***********************************/

    LBL(PT_mix)

        /* modulate with surface color */
        mulps_ld(Xmm1, Mecx, ctx_TEX_R)
        mulps_ld(Xmm2, Mecx, ctx_TEX_G)
        mulps_ld(Xmm3, Mecx, ctx_TEX_B)

    LBL(PT_skp)

        /* add self-emission */
        addps_ld(Xmm1, Medx, mat_COL_R)
        addps_ld(Xmm2, Medx, mat_COL_G)
        addps_ld(Xmm3, Medx, mat_COL_B)

        /* radiance R */
        STORE_SIMD(COL_R, Xmm1)
        /* radiance G */
        STORE_SIMD(COL_G, Xmm2)
        /* radiance B */
        STORE_SIMD(COL_B, Xmm3)

        jmpxx_lb(LT_end)

/******************************************************************************/

        /* regular lighting for ray-tracer */

    LBL(LT_reg)

        CHECK_PROP(LT_lgt, RT_PROP_LIGHT)

        jmpxx_lb(LT_set)

    LBL(LT_lgt)

#if RT_FEAT_LIGHTS

#if RT_FEAT_LIGHTS_AMBIENT

        movxx_ld(Redx, Mebp, inf_CAM)

        movpx_ld(Xmm1, Mecx, ctx_TEX_R)
        movpx_ld(Xmm2, Mecx, ctx_TEX_G)
        movpx_ld(Xmm3, Mecx, ctx_TEX_B)

#if RT_FEAT_LIGHTS_COLORED

        mulps_ld(Xmm1, Medx, cam_COL_R)
        mulps_ld(Xmm2, Medx, cam_COL_G)
        mulps_ld(Xmm3, Medx, cam_COL_B)

#else /* RT_FEAT_LIGHTS_COLORED */

        movpx_ld(Xmm0, Medx, cam_L_AMB)

        mulps_rr(Xmm1, Xmm0)
        mulps_rr(Xmm2, Xmm0)
        mulps_rr(Xmm3, Xmm0)

#endif /* RT_FEAT_LIGHTS_COLORED */

#else /* RT_FEAT_LIGHTS_AMBIENT */

        xorpx_rr(Xmm1, Xmm1)
        xorpx_rr(Xmm2, Xmm2)
        xorpx_rr(Xmm3, Xmm3)

#endif /* RT_FEAT_LIGHTS_AMBIENT */

        /* ambient R */
        STORE_SIMD(COL_R, Xmm1)
        /* ambient G */
        STORE_SIMD(COL_G, Xmm2)
        /* ambient B */
        STORE_SIMD(COL_B, Xmm3)

#if RT_FEAT_LIGHTS_DIFFUSE || RT_FEAT_LIGHTS_SPECULAR

        FETCH_XPTR(Redi, LST_P(LGT))

    LBL(LT_cyc)

        cmjxx_rz(Redi,
                 EQ_x, LT_end)

        movxx_ld(Redx, Medi, elm_SIMD)

        /* compute common */
        movpx_ld(Xmm1, Medx, lgt_POS_X)         /* hit_x <- POS_X */
        subps_ld(Xmm1, Mecx, ctx_HIT_X)         /* hit_x -= HIT_X */
        movpx_st(Xmm1, Mecx, ctx_NEW_X)         /* hit_x -> NEW_X */
        mulps_ld(Xmm1, Mecx, ctx_NRM_X)         /* hit_x *= NRM_X */

        movpx_ld(Xmm2, Medx, lgt_POS_Y)         /* hit_y <- POS_Y */
        subps_ld(Xmm2, Mecx, ctx_HIT_Y)         /* hit_y -= HIT_Y */
        movpx_st(Xmm2, Mecx, ctx_NEW_Y)         /* hit_y -> NEW_Y */
        mulps_ld(Xmm2, Mecx, ctx_NRM_Y)         /* hit_y *= NRM_Y */

        movpx_ld(Xmm3, Medx, lgt_POS_Z)         /* hit_z <- POS_Z */
        subps_ld(Xmm3, Mecx, ctx_HIT_Z)         /* hit_z -= HIT_Z */
        movpx_st(Xmm3, Mecx, ctx_NEW_Z)         /* hit_z -> NEW_Z */
        mulps_ld(Xmm3, Mecx, ctx_NRM_Z)         /* hit_z *= NRM_Z */

        movpx_rr(Xmm0, Xmm1)
        addps_rr(Xmm0, Xmm2)
        addps_rr(Xmm0, Xmm3)

        xorpx_rr(Xmm7, Xmm7)                    /* tmp_v <-     0 */
        cltps_rr(Xmm7, Xmm0)                    /* tmp_v <! r_dot */
        andpx_ld(Xmm7, Mecx, ctx_TMASK(0))      /* lmask &= TMASK */
        CHECK_MASK(LT_amb, NONE, Xmm7)

#if RT_FEAT_LIGHTS_SHADOWS

        xorpx_rr(Xmm6, Xmm6)                    /* init shadow mask (hmask) */
        ceqps_rr(Xmm7, Xmm6)                    /* with inverted lmask */

        movpx_st(Xmm0, Mecx, ctx_C_PTR(0))      /* save dot product */

/************************************ ENTER ***********************************/

        movpx_ld(Xmm0, Mecx, ctx_TMASK(0))      /* load tmask */
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))
        orrxx_ri(Reax, IB(RT_FLAG_PASS_BACK | RT_FLAG_SHAD))
        addxx_ri(Recx, IH(RT_STACK_STEP))
        subxx_mi(Mebp, inf_DEPTH, IB(1))

        movxx_st(Reax, Mecx, ctx_PARAM(FLG))    /* context flags */
        movxx_st(Redi, Mecx, ctx_PARAM(LST))    /* save light/shadow list */
        movxx_st(Rebx, Mecx, ctx_PARAM(OBJ))    /* originating surface */
        movwx_mi(Mecx, ctx_PARAM(PTR), IB(1))   /* mark LT_ret with tag 1 */
        movpx_st(Xmm0, Mecx, ctx_WMASK)         /* tmask -> WMASK */

        movpx_ld(Xmm0, Medx, lgt_T_MAX)         /* tmp_v <- T_MAX */
        movpx_st(Xmm0, Mecx, ctx_T_BUF(0))      /* tmp_v -> T_BUF */

        xorpx_rr(Xmm0, Xmm0)                    /* tmp_v <-     0 */
        movpx_st(Xmm7, Mecx, ctx_C_BUF(0))      /* hmask -> C_BUF */
        movpx_st(Xmm0, Mecx, ctx_COL_R(0))      /* tmp_v -> COL_R */
        movpx_st(Xmm0, Mecx, ctx_COL_G(0))      /* tmp_v -> COL_G */
        movpx_st(Xmm0, Mecx, ctx_COL_B(0))      /* tmp_v -> COL_B */

        movpx_st(Xmm0, Mecx, ctx_T_MIN)         /* tmp_v -> T_MIN */
        adrpx_ld(Reax, Mecx, ctx_LOCAL(0))
        movpx_st(Xmm0, Oeax, PLAIN)             /* tmp_v -> LOCAL */
        addxx_ri(Reax, IM(RT_SIMD_QUADS*16))
        movpx_st(Xmm0, Oeax, PLAIN)             /* tmp_v -> LOCAL */

        movxx_ld(Resi, Medi, elm_DATA)          /* load shadow list */
        jmpxx_lb(OO_cyc)

    LBL(LT_ret)

        movxx_ld(Redi, Mecx, ctx_PARAM(LST))    /* restore light/shadow list */
        movxx_ld(Rebx, Mecx, ctx_PARAM(OBJ))    /* restore surface */

        movpx_ld(Xmm7, Mecx, ctx_C_BUF(0))      /* load shadow mask (hmask) */

        addxx_mi(Mebp, inf_DEPTH, IB(1))
        subxx_ri(Recx, IH(RT_STACK_STEP))

/************************************ LEAVE ***********************************/

        CHECK_MASK(LT_amb, FULL, Xmm7)

        movpx_ld(Xmm0, Mecx, ctx_C_PTR(0))      /* restore dot product */

        xorpx_rr(Xmm6, Xmm6)                    
        ceqps_rr(Xmm7, Xmm6)                    /* invert shadow mask (hmask) */

#endif /* RT_FEAT_LIGHTS_SHADOWS */

        /* compute common */
        movpx_ld(Xmm1, Mecx, ctx_NEW_X)
        movpx_rr(Xmm4, Xmm1)
        mulps_rr(Xmm4, Xmm4)

        movpx_ld(Xmm2, Mecx, ctx_NEW_Y)
        movpx_rr(Xmm5, Xmm2)
        mulps_rr(Xmm5, Xmm5)

        movpx_ld(Xmm3, Mecx, ctx_NEW_Z)
        movpx_rr(Xmm6, Xmm3)
        mulps_rr(Xmm6, Xmm6)

        addps_rr(Xmm4, Xmm5)
        addps_rr(Xmm4, Xmm6)

        movpx_st(Xmm4, Mecx, ctx_C_PTR(0))

#if RT_FEAT_LIGHTS_DIFFUSE

        CHECK_PROP(LT_dfs, RT_PROP_DIFFUSE)

        /* compute diffuse */
        andpx_rr(Xmm0, Xmm7)                    /* r_dot &= hmask */

#if RT_FEAT_LIGHTS_ATTENUATION

        /* prepare attenuation */
        movpx_rr(Xmm6, Xmm4)                    /* Xmm6  <-   r^2 */

#endif /* RT_FEAT_LIGHTS_ATTENUATION */

        rsqps_rr(Xmm5, Xmm4) /* destroys Xmm4 *//* Xmm5  <-   1/r */

#if RT_FEAT_LIGHTS_ATTENUATION

        /* compute attenuation */
        movpx_rr(Xmm4, Xmm5)                    /* Xmm4  <-   1/r */
        mulps_rr(Xmm4, Xmm6)                    /* Xmm4  <-   r^1 */

        movxx_ld(Redx, Medi, elm_SIMD)

        mulps_ld(Xmm6, Medx, lgt_A_QDR)
        mulps_ld(Xmm4, Medx, lgt_A_LNR)
        addps_ld(Xmm6, Medx, lgt_A_CNT)
        addps_rr(Xmm6, Xmm4)
        rsqps_rr(Xmm4, Xmm6) /* destroys Xmm6 *//* Xmm4  <-   1/a */

#endif /* RT_FEAT_LIGHTS_ATTENUATION */

        movpx_rr(Xmm6, Xmm0)

#if RT_FEAT_LIGHTS_ATTENUATION

        /* apply attenuation */
        mulps_rr(Xmm0, Xmm4)                    /* r_dot *=   1/a */

#endif /* RT_FEAT_LIGHTS_ATTENUATION */

        mulps_rr(Xmm0, Xmm5)                    /* r_dot *=   1/r */

        FETCH_XPTR(Redx, MAT_P(PTR))

        mulps_ld(Xmm0, Medx, mat_L_DFF)
        jmpxx_lb(LT_dfn)

    LBL(LT_dfs)

#endif /* RT_FEAT_LIGHTS_DIFFUSE */

        movpx_rr(Xmm6, Xmm0)
        xorpx_rr(Xmm0, Xmm0)

        FETCH_XPTR(Redx, MAT_P(PTR))

    LBL(LT_dfn)

#if RT_FEAT_LIGHTS_SPECULAR

        CHECK_PROP(LT_spc, RT_PROP_SPECULAR)

        movpx_rr(Xmm4, Xmm6)
        movpx_rr(Xmm5, Xmm6)

        /* compute specular */
        mulps_ld(Xmm4, Mecx, ctx_NRM_X)
        subps_rr(Xmm1, Xmm4)
        subps_rr(Xmm1, Xmm4)

        mulps_ld(Xmm5, Mecx, ctx_NRM_Y)
        subps_rr(Xmm2, Xmm5)
        subps_rr(Xmm2, Xmm5)

        mulps_ld(Xmm6, Mecx, ctx_NRM_Z)
        subps_rr(Xmm3, Xmm6)
        subps_rr(Xmm3, Xmm6)

        movpx_ld(Xmm4, Mecx, ctx_RAY_X)
        mulps_rr(Xmm1, Xmm4)
        mulps_rr(Xmm4, Xmm4)

        movpx_ld(Xmm5, Mecx, ctx_RAY_Y)
        mulps_rr(Xmm2, Xmm5)
        mulps_rr(Xmm5, Xmm5)

        movpx_ld(Xmm6, Mecx, ctx_RAY_Z)
        mulps_rr(Xmm3, Xmm6)
        mulps_rr(Xmm6, Xmm6)

        addps_rr(Xmm6, Xmm4)
        addps_rr(Xmm6, Xmm5)

        addps_rr(Xmm1, Xmm2)
        addps_rr(Xmm1, Xmm3)

        xorpx_rr(Xmm2, Xmm2)                    /* tmp_v <-     0 */
        cltps_rr(Xmm2, Xmm1)                    /* tmp_v <! r_dot */
        andpx_rr(Xmm2, Xmm7)                    /* lmask &= hmask */
        andpx_rr(Xmm1, Xmm2)                    /* r_dot &= lmask */
        CHECK_MASK(LT_spc, NONE, Xmm2)

        movpx_ld(Xmm4, Mecx, ctx_C_PTR(0))
        rsqps_rr(Xmm5, Xmm6) /* destroys Xmm6 */
        mulps_rr(Xmm1, Xmm5)
        rsqps_rr(Xmm5, Xmm4) /* destroys Xmm4 */
        mulps_rr(Xmm1, Xmm5)

        /* compute specular pow,
         * fixed-point 28.4-bit */
        movwx_ld(Reax, Medx, mat_L_POW)
        andwx_ri(Reax, IB(0xF))

        movpx_rr(Xmm2, Xmm1)
        movpx_rr(Xmm4, Xmm1)
        movpx_ld(Xmm1, Mebp, inf_GPC01)

        cmjwx_rz(Reax,
                 EQ_x, LT_pwi)

    LBL(LT_frc)

        sqrps_rr(Xmm4, Xmm4)
        movwx_ri(Resi, IB(0x8))
        andwx_rr(Resi, Reax)
        shlwx_ri(Reax, IB(1))
        andwx_ri(Reax, IB(0xF))
        cmjwx_rz(Resi,
                 EQ_x, LT_frs)

        mulps_rr(Xmm1, Xmm4)

    LBL(LT_frs)

        cmjwx_rz(Reax,
                 NE_x, LT_frc)

    LBL(LT_pwi)

        movwx_ld(Reax, Medx, mat_L_POW)
        shrwx_ri(Reax, IB(4))

        cmjwx_rz(Reax,
                 EQ_x, LT_pwe)

        movpx_rr(Xmm3, Xmm1)
        movpx_ld(Xmm1, Mebp, inf_GPC01)

    LBL(LT_pwc)

        movwx_ri(Resi, IB(1))
        andwx_rr(Resi, Reax)
        shrwx_ri(Reax, IB(1))
        cmjwx_rz(Resi,
                 EQ_x, LT_pws)

        mulps_rr(Xmm1, Xmm2)

    LBL(LT_pws)

        mulps_rr(Xmm2, Xmm2)
        cmjwx_rz(Reax,
                 NE_x, LT_pwc)

        mulps_rr(Xmm1, Xmm3)

    LBL(LT_pwe)

        mulps_ld(Xmm1, Medx, mat_L_SPC)

        CHECK_PROP(LT_mtl, RT_PROP_METAL)

        addps_rr(Xmm0, Xmm1)

    LBL(LT_spc)

#endif /* RT_FEAT_LIGHTS_SPECULAR */

        /* apply lighting to "metal" color,
         * only affects diffuse-specular blending */
        movxx_ld(Redx, Medi, elm_SIMD)

        movpx_ld(Xmm1, Mecx, ctx_TEX_R)
        movpx_ld(Xmm2, Mecx, ctx_TEX_G)
        movpx_ld(Xmm3, Mecx, ctx_TEX_B)

#if RT_FEAT_LIGHTS_COLORED

        mulps_ld(Xmm1, Medx, lgt_COL_R)
        mulps_ld(Xmm2, Medx, lgt_COL_G)
        mulps_ld(Xmm3, Medx, lgt_COL_B)

#else /* RT_FEAT_LIGHTS_COLORED */

        mulps_ld(Xmm0, Medx, lgt_L_SRC)

#endif /* RT_FEAT_LIGHTS_COLORED */

        mulps_rr(Xmm1, Xmm0)
        mulps_rr(Xmm2, Xmm0)
        mulps_rr(Xmm3, Xmm0)

        addps_ld(Xmm1, Mecx, ctx_COL_R(0))
        addps_ld(Xmm2, Mecx, ctx_COL_G(0))
        addps_ld(Xmm3, Mecx, ctx_COL_B(0))

        /* diffuse + "metal" specular R */
        STORE_SIMD(COL_R, Xmm1)
        /* diffuse + "metal" specular G */
        STORE_SIMD(COL_G, Xmm2)
        /* diffuse + "metal" specular B */
        STORE_SIMD(COL_B, Xmm3)

        jmpxx_lb(LT_amb)

#if RT_FEAT_LIGHTS_SPECULAR

    LBL(LT_mtl)

        movpx_rr(Xmm7, Xmm1)

        /* apply lighting to "plain" color,
         * only affects diffuse-specular blending */
        movxx_ld(Redx, Medi, elm_SIMD)

        movpx_ld(Xmm1, Mecx, ctx_TEX_R)
        movpx_ld(Xmm2, Mecx, ctx_TEX_G)
        movpx_ld(Xmm3, Mecx, ctx_TEX_B)

#if RT_FEAT_LIGHTS_COLORED

        movpx_ld(Xmm4, Medx, lgt_COL_R)
        movpx_ld(Xmm5, Medx, lgt_COL_G)
        movpx_ld(Xmm6, Medx, lgt_COL_B)

        mulps_rr(Xmm1, Xmm0)
        mulps_rr(Xmm2, Xmm0)
        mulps_rr(Xmm3, Xmm0)

        mulps_rr(Xmm1, Xmm4)
        mulps_rr(Xmm2, Xmm5)
        mulps_rr(Xmm3, Xmm6)

        mulps_rr(Xmm4, Xmm7)
        mulps_rr(Xmm5, Xmm7)
        mulps_rr(Xmm6, Xmm7)

        addps_rr(Xmm1, Xmm4)
        addps_rr(Xmm2, Xmm5)
        addps_rr(Xmm3, Xmm6)

#else /* RT_FEAT_LIGHTS_COLORED */

        movpx_ld(Xmm6, Medx, lgt_L_SRC)
        mulps_rr(Xmm0, Xmm6)
        mulps_rr(Xmm7, Xmm6)

        mulps_rr(Xmm1, Xmm0)
        mulps_rr(Xmm2, Xmm0)
        mulps_rr(Xmm3, Xmm0)

        addps_rr(Xmm1, Xmm7)
        addps_rr(Xmm2, Xmm7)
        addps_rr(Xmm3, Xmm7)

#endif /* RT_FEAT_LIGHTS_COLORED */

        addps_ld(Xmm1, Mecx, ctx_COL_R(0))
        addps_ld(Xmm2, Mecx, ctx_COL_G(0))
        addps_ld(Xmm3, Mecx, ctx_COL_B(0))

        /* diffuse + "plain" specular R */
        STORE_SIMD(COL_R, Xmm1)
        /* diffuse + "plain" specular G */
        STORE_SIMD(COL_G, Xmm2)
        /* diffuse + "plain" specular B */
        STORE_SIMD(COL_B, Xmm3)

#endif /* RT_FEAT_LIGHTS_SPECULAR */

    LBL(LT_amb)

        movxx_ld(Redi, Medi, elm_NEXT)
        jmpxx_lb(LT_cyc)

#endif /* RT_FEAT_LIGHTS_DIFFUSE, RT_FEAT_LIGHTS_SPECULAR */

        jmpxx_lb(LT_end)

#endif /* RT_FEAT_LIGHTS */

    LBL(LT_set)

        /* consider merging with "ambient"
         * section at the top of "lights" */
        movpx_ld(Xmm1, Mecx, ctx_TEX_R)
        movpx_ld(Xmm2, Mecx, ctx_TEX_G)
        movpx_ld(Xmm3, Mecx, ctx_TEX_B)

        /* texture R */
        STORE_SIMD(COL_R, Xmm1)
        /* texture G */
        STORE_SIMD(COL_G, Xmm2)
        /* texture B */
        STORE_SIMD(COL_B, Xmm3)

    LBL(LT_end)

/******************************************************************************/
/******************************   TRANSPARENCY   ******************************/
/******************************************************************************/

        FETCH_XPTR(Redx, MAT_P(PTR))

        movpx_ld(Xmm0, Medx, mat_C_TRN)
        movpx_st(Xmm0, Mecx, ctx_C_TRN)
        movpx_ld(Xmm0, Medx, mat_C_RFL)
        movpx_st(Xmm0, Mecx, ctx_C_RFL)

        movpx_ld(Xmm0, Mebp, inf_GPC07)
        andpx_ld(Xmm0, Mecx, ctx_TMASK(0))
        movpx_st(Xmm0, Mecx, ctx_M_RFL)

#if RT_FEAT_TRANSPARENCY

        CHECK_PROP(TR_opq, RT_PROP_OPAQUE)

        jmpxx_lb(TR_end)

    LBL(TR_opq)

#if RT_FEAT_REFRACTIONS || RT_FEAT_FRESNEL

        CHECK_PROP(TR_rfr, RT_PROP_REFRACT)

    LBL(TR_rfi)

        /* compute refraction, Fresnel
         * requires normalized ray */
        movpx_ld(Xmm1, Mecx, ctx_RAY_X)
        movpx_rr(Xmm7, Xmm1)
        mulps_rr(Xmm7, Xmm1)
        movpx_rr(Xmm0, Xmm7)

        movpx_ld(Xmm2, Mecx, ctx_RAY_Y)
        movpx_rr(Xmm7, Xmm2)
        mulps_rr(Xmm7, Xmm2)
        addps_rr(Xmm0, Xmm7)

        movpx_ld(Xmm3, Mecx, ctx_RAY_Z)
        movpx_rr(Xmm7, Xmm3)
        mulps_rr(Xmm7, Xmm3)
        addps_rr(Xmm0, Xmm7)

        rsqps_rr(Xmm7, Xmm0) /* destroys Xmm0 */
        mulps_rr(Xmm1, Xmm7)
        mulps_rr(Xmm2, Xmm7)
        mulps_rr(Xmm3, Xmm7)

        movpx_rr(Xmm7, Xmm1)
        mulps_ld(Xmm7, Mecx, ctx_NRM_X)
        movpx_rr(Xmm0, Xmm7)

        movpx_rr(Xmm7, Xmm2)
        mulps_ld(Xmm7, Mecx, ctx_NRM_Y)
        addps_rr(Xmm0, Xmm7)

        movpx_rr(Xmm7, Xmm3)
        mulps_ld(Xmm7, Mecx, ctx_NRM_Z)
        addps_rr(Xmm0, Xmm7)

#if RT_FEAT_FRESNEL

        /* move dot-product temporarily */
        movpx_rr(Xmm4, Xmm0)

#endif /* RT_FEAT_FRESNEL */

        movpx_ld(Xmm6, Medx, mat_C_RFR)
        mulps_rr(Xmm0, Xmm6)
        movpx_rr(Xmm7, Xmm0)
        mulps_rr(Xmm7, Xmm7)
        addps_ld(Xmm7, Mebp, inf_GPC01)
        subps_ld(Xmm7, Medx, mat_RFR_2)

#if RT_FEAT_FRESNEL

        CHECK_PROP(TR_cnt, RT_PROP_FRESNEL)

        /* check total inner reflection */
        xorpx_rr(Xmm5, Xmm5)
        cleps_rr(Xmm5, Xmm7)
        andpx_ld(Xmm5, Mecx, ctx_TMASK(0))

        CHECK_MASK(TR_tir, NONE, Xmm5)

        movpx_st(Xmm5, Mecx, ctx_T_NEW)
        jmpxx_lb(TR_cnt)

    LBL(TR_tir)

        movpx_ld(Xmm0, Medx, mat_C_TRN)

        xorpx_rr(Xmm4, Xmm4)
        movpx_st(Xmm4, Mecx, ctx_C_TRN)
        movpx_ld(Xmm5, Medx, mat_C_RFL)
        addps_rr(Xmm5, Xmm0)
        movpx_st(Xmm5, Mecx, ctx_C_RFL)
        jmpxx_lb(TR_end)

    LBL(TR_cnt)

#endif /* RT_FEAT_FRESNEL */

        sqrps_rr(Xmm7, Xmm7)
        addps_rr(Xmm0, Xmm7)

        CHECK_PROP(TR_rfe, RT_PROP_REFRACT)

        movpx_ld(Xmm5, Mecx, ctx_NRM_X)
        mulps_rr(Xmm5, Xmm0)
        mulps_rr(Xmm1, Xmm6)
        subps_rr(Xmm1, Xmm5)
        movpx_st(Xmm1, Mecx, ctx_NEW_X)

        movpx_ld(Xmm5, Mecx, ctx_NRM_Y)
        mulps_rr(Xmm5, Xmm0)
        mulps_rr(Xmm2, Xmm6)
        subps_rr(Xmm2, Xmm5)
        movpx_st(Xmm2, Mecx, ctx_NEW_Y)

        movpx_ld(Xmm5, Mecx, ctx_NRM_Z)
        mulps_rr(Xmm5, Xmm0)
        mulps_rr(Xmm3, Xmm6)
        subps_rr(Xmm3, Xmm5)
        movpx_st(Xmm3, Mecx, ctx_NEW_Z)

        jmpxx_lb(TR_ini)

    LBL(TR_rfr)

#if RT_FEAT_FRESNEL

        CHECK_PROP(TR_rfe, RT_PROP_FRESNEL)

        jmpxx_lb(TR_rfi)

#endif /* RT_FEAT_FRESNEL */

    LBL(TR_rfe)

#endif /* RT_FEAT_REFRACTIONS || RT_FEAT_FRESNEL */

        /* propagate ray */
        movpx_ld(Xmm1, Mecx, ctx_RAY_X)
        movpx_ld(Xmm2, Mecx, ctx_RAY_Y)
        movpx_ld(Xmm3, Mecx, ctx_RAY_Z)

        movpx_st(Xmm1, Mecx, ctx_NEW_X)
        movpx_st(Xmm2, Mecx, ctx_NEW_Y)
        movpx_st(Xmm3, Mecx, ctx_NEW_Z)

    LBL(TR_ini)

#if RT_FEAT_FRESNEL

        CHECK_PROP(TR_frn, RT_PROP_FRESNEL)

#if RT_FEAT_SCHLICK

        /* compute Schlick */
        movpx_ld(Xmm1, Mebp, inf_GPC01)
        movpx_rr(Xmm5, Xmm6)
        cgtps_rr(Xmm5, Xmm1)

        CHECK_MASK(TR_inv, NONE, Xmm5)

        xorpx_rr(Xmm4, Xmm4)
        subps_rr(Xmm4, Xmm7)

    LBL(TR_inv)

        addps_rr(Xmm4, Xmm1)
        movpx_rr(Xmm0, Xmm6)
        subps_rr(Xmm0, Xmm1)
        addps_rr(Xmm6, Xmm1)
        divps_rr(Xmm0, Xmm6)
        mulps_rr(Xmm0, Xmm0)
        subps_rr(Xmm1, Xmm0)
        movpx_rr(Xmm5, Xmm4)
        mulps_rr(Xmm4, Xmm4)
        mulps_rr(Xmm4, Xmm4)
        mulps_rr(Xmm5, Xmm4)
        mulps_rr(Xmm1, Xmm5)
        addps_rr(Xmm0, Xmm1)

#else /* RT_FEAT_SCHLICK */

        /* compute Fresnel */
        movpx_rr(Xmm1, Xmm4)
        movpx_rr(Xmm2, Xmm1)
        mulps_rr(Xmm2, Xmm6)
        subps_rr(Xmm2, Xmm7)
        mulps_rr(Xmm7, Xmm6)
        movpx_rr(Xmm3, Xmm1)
        addps_rr(Xmm1, Xmm7)
        subps_rr(Xmm3, Xmm7)
        divps_rr(Xmm0, Xmm2)
        divps_rr(Xmm1, Xmm3)
        mulps_rr(Xmm0, Xmm0)
        mulps_rr(Xmm1, Xmm1)
        addps_rr(Xmm0, Xmm1)
        mulps_ld(Xmm0, Mebp, inf_GPC02)
        andpx_ld(Xmm0, Mebp, inf_GPC04)

#endif /* RT_FEAT_SCHLICK */

        /* store Fresnel reflectance */
        movpx_ld(Xmm5, Mecx, ctx_T_NEW)
        andpx_rr(Xmm0, Xmm5)
        movpx_ld(Xmm4, Medx, mat_C_TRN)
        mulps_rr(Xmm0, Xmm4)
        annpx_rr(Xmm5, Xmm4)
        orrpx_rr(Xmm0, Xmm5)

        movpx_ld(Xmm4, Medx, mat_C_TRN)
        subps_rr(Xmm4, Xmm0)
        movpx_st(Xmm4, Mecx, ctx_C_TRN)
        movpx_ld(Xmm5, Medx, mat_C_RFL)
        addps_rr(Xmm5, Xmm0)
        movpx_st(Xmm5, Mecx, ctx_C_RFL)

        cmjxx_mz(Mebp, inf_PT_ON,
                 EQ_x, TR_frn)

        cmjxx_mi(Mebp, inf_DEPTH, IB(RT_STACK_DEPTH - 2),
                 GT_x, TR_frn)

        GET_RANDOM() /* -> Xmm0, destroys Xmm7, Reax */

        movpx_rr(Xmm6, Xmm5)
        addps3rr(Xmm7, Xmm4, Xmm5)
        divps_rr(Xmm5, Xmm7)
        movpx_ld(Xmm7, Mebp, inf_GPC02)
        andpx_ld(Xmm7, Mebp, inf_GPC04)
        mulps_rr(Xmm5, Xmm7)
        mulps_rr(Xmm7, Xmm7)
        addps_rr(Xmm7, Xmm5)

        movpx_rr(Xmm5, Xmm0)
        cgeps_rr(Xmm0, Xmm7)
        andpx_ld(Xmm0, Mecx, ctx_TMASK(0))
        movpx_st(Xmm0, Mecx, ctx_M_TRN)
        movpx_rr(Xmm0, Xmm5)
        cltps_rr(Xmm0, Xmm7)
        andpx_ld(Xmm0, Mecx, ctx_TMASK(0))
        movpx_st(Xmm0, Mecx, ctx_M_RFL)

        movpx_rr(Xmm5, Xmm4)
        movpx_ld(Xmm0, Mebp, inf_GPC01)
        subps_rr(Xmm0, Xmm7)
        divps_rr(Xmm5, Xmm0)
        divps_rr(Xmm6, Xmm7)
        andpx_ld(Xmm5, Mecx, ctx_M_TRN)
        movpx_st(Xmm5, Mecx, ctx_C_TRN)
        andpx_ld(Xmm6, Mecx, ctx_M_RFL)
        movpx_st(Xmm6, Mecx, ctx_C_RFL)

        movpx_ld(Xmm0, Mecx, ctx_M_TRN)
        CHECK_MASK(TR_end, NONE, Xmm0)

    LBL(TR_frn)

#endif /* RT_FEAT_FRESNEL */

        /* prepare default values */
        xorpx_rr(Xmm0, Xmm0)
        movpx_st(Xmm0, Mecx, ctx_T_NEW)

        xorpx_rr(Xmm1, Xmm1)
        xorpx_rr(Xmm2, Xmm2)
        xorpx_rr(Xmm3, Xmm3)

/************************************ ENTER ***********************************/

        cmjxx_mz(Mebp, inf_DEPTH,
                 EQ_x, TR_mix)

        FETCH_IPTR(Resi, LST_P(SRF))

#if RT_SHOW_BOUND

        cmjwx_mi(Mebx, srf_SRF_T(TAG), IB(RT_TAG_SURFACE_MAX),
                 NE_x, TR_arr)

        movxx_ld(Resi, Mebp, inf_LST)

    LBL(TR_arr)

#endif /* RT_SHOW_BOUND */

        movpx_ld(Xmm0, Mecx, ctx_TMASK(0))      /* load tmask */
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))
        orrxx_ri(Reax, IB(RT_FLAG_PASS_THRU))
        addxx_ri(Recx, IH(RT_STACK_STEP))
        subxx_mi(Mebp, inf_DEPTH, IB(1))

        movxx_st(Reax, Mecx, ctx_PARAM(FLG))    /* context flags */
        movxx_st(Redx, Mecx, ctx_PARAM(LST))    /* save material */
        movxx_st(Rebx, Mecx, ctx_PARAM(OBJ))    /* originating surface */
        movwx_mi(Mecx, ctx_PARAM(PTR), IB(3))   /* mark TR_ret with tag 3 */
        movpx_st(Xmm0, Mecx, ctx_WMASK)         /* tmask -> WMASK */

        movxx_ld(Redx, Mebp, inf_CAM)
        movpx_ld(Xmm0, Medx, cam_T_MAX)         /* tmp_v <- T_MAX */
        movpx_st(Xmm0, Mecx, ctx_T_BUF(0))      /* tmp_v -> T_BUF */

        xorpx_rr(Xmm0, Xmm0)                    /* tmp_v <-     0 */
        movpx_st(Xmm0, Mecx, ctx_C_BUF(0))      /* tmp_v -> C_BUF */
        movpx_st(Xmm0, Mecx, ctx_COL_R(0))      /* tmp_v -> COL_R */
        movpx_st(Xmm0, Mecx, ctx_COL_G(0))      /* tmp_v -> COL_G */
        movpx_st(Xmm0, Mecx, ctx_COL_B(0))      /* tmp_v -> COL_B */

        movpx_st(Xmm0, Mecx, ctx_T_MIN)         /* tmp_v -> T_MIN */
        adrpx_ld(Reax, Mecx, ctx_LOCAL(0))
        movpx_st(Xmm0, Oeax, PLAIN)             /* tmp_v -> LOCAL */
        addxx_ri(Reax, IM(RT_SIMD_QUADS*16))
        movpx_st(Xmm0, Oeax, PLAIN)             /* tmp_v -> LOCAL */

        jmpxx_lb(OO_cyc)

    LBL(TR_ret)

        movxx_ld(Redx, Mecx, ctx_PARAM(LST))    /* restore material */
        movxx_ld(Rebx, Mecx, ctx_PARAM(OBJ))    /* restore surface */

        movpx_ld(Xmm1, Mecx, ctx_COL_R(0))
        movpx_ld(Xmm2, Mecx, ctx_COL_G(0))
        movpx_ld(Xmm3, Mecx, ctx_COL_B(0))

        addxx_mi(Mebp, inf_DEPTH, IB(1))
        subxx_ri(Recx, IH(RT_STACK_STEP))

        movpx_ld(Xmm0, Mecx, ctx_C_TRN)

        mulps_rr(Xmm1, Xmm0)
        mulps_rr(Xmm2, Xmm0)
        mulps_rr(Xmm3, Xmm0)

        jmpxx_lb(TR_mix)

/************************************ LEAVE ***********************************/

    LBL(TR_end)

#endif /* RT_FEAT_TRANSPARENCY */

        xorpx_rr(Xmm1, Xmm1)
        xorpx_rr(Xmm2, Xmm2)
        xorpx_rr(Xmm3, Xmm3)

    LBL(TR_mix)

        /* consider making format parameters
         * cumulative to avoid modulation */
        movpx_ld(Xmm0, Mebp, inf_GPC01)
        subps_ld(Xmm0, Medx, mat_C_TRN)
        subps_ld(Xmm0, Medx, mat_C_RFL)
        xorpx_rr(Xmm7, Xmm7)
        cleps_rr(Xmm7, Xmm0)
        andpx_rr(Xmm0, Xmm7)

        movpx_ld(Xmm4, Mecx, ctx_COL_R(0))
        movpx_ld(Xmm5, Mecx, ctx_COL_G(0))
        movpx_ld(Xmm6, Mecx, ctx_COL_B(0))

        /* modulate light with surface props */
        mulps_rr(Xmm4, Xmm0)
        mulps_rr(Xmm5, Xmm0)
        mulps_rr(Xmm6, Xmm0)

        /* accumulate colors */
        addps_rr(Xmm1, Xmm4)
        addps_rr(Xmm2, Xmm5)
        addps_rr(Xmm3, Xmm6)

        /* transparent R */
        STORE_SIMD(COL_R, Xmm1)
        /* transparent G */
        STORE_SIMD(COL_G, Xmm2)
        /* transparent B */
        STORE_SIMD(COL_B, Xmm3)

/******************************************************************************/
/*******************************   REFLECTIONS   ******************************/
/******************************************************************************/

#if RT_FEAT_REFLECTIONS || RT_FEAT_FRESNEL

        FETCH_XPTR(Redx, MAT_P(PTR))

        CHECK_PROP(RF_end, RT_PROP_REFLECT)

    LBL(RF_ini)

        movpx_ld(Xmm0, Mecx, ctx_M_RFL)
        CHECK_MASK(RF_out, NONE, Xmm0)

        /* compute reflection, Fresnel
         * requires normalized ray */
        movpx_ld(Xmm1, Mecx, ctx_RAY_X)
        movpx_ld(Xmm4, Mecx, ctx_NRM_X)
        movpx_rr(Xmm7, Xmm1)
        mulps_rr(Xmm7, Xmm1)
        movpx_rr(Xmm0, Xmm7)

        movpx_ld(Xmm2, Mecx, ctx_RAY_Y)
        movpx_ld(Xmm5, Mecx, ctx_NRM_Y)
        movpx_rr(Xmm7, Xmm2)
        mulps_rr(Xmm7, Xmm2)
        addps_rr(Xmm0, Xmm7)

        movpx_ld(Xmm3, Mecx, ctx_RAY_Z)
        movpx_ld(Xmm6, Mecx, ctx_NRM_Z)
        movpx_rr(Xmm7, Xmm3)
        mulps_rr(Xmm7, Xmm3)
        addps_rr(Xmm0, Xmm7)

        rsqps_rr(Xmm7, Xmm0) /* destroys Xmm0 */
        mulps_rr(Xmm1, Xmm7)
        mulps_rr(Xmm2, Xmm7)
        mulps_rr(Xmm3, Xmm7)

        movpx_rr(Xmm7, Xmm1)
        mulps_rr(Xmm7, Xmm4)
        movpx_rr(Xmm0, Xmm7)

        movpx_rr(Xmm7, Xmm2)
        mulps_rr(Xmm7, Xmm5)
        addps_rr(Xmm0, Xmm7)

        movpx_rr(Xmm7, Xmm3)
        mulps_rr(Xmm7, Xmm6)
        addps_rr(Xmm0, Xmm7)

        mulps_rr(Xmm4, Xmm0)
        subps_rr(Xmm1, Xmm4)
        subps_rr(Xmm1, Xmm4)
        movpx_st(Xmm1, Mecx, ctx_NEW_X)

        mulps_rr(Xmm5, Xmm0)
        subps_rr(Xmm2, Xmm5)
        subps_rr(Xmm2, Xmm5)
        movpx_st(Xmm2, Mecx, ctx_NEW_Y)

        mulps_rr(Xmm6, Xmm0)
        subps_rr(Xmm3, Xmm6)
        subps_rr(Xmm3, Xmm6)
        movpx_st(Xmm3, Mecx, ctx_NEW_Z)

#if RT_FEAT_FRESNEL

        CHECK_PROP(RF_frn, RT_PROP_FRESNEL)

        CHECK_PROP(RF_frn, RT_PROP_OPAQUE)

#if RT_FEAT_FRESNEL_METAL

        CHECK_PROP(RF_mtl, RT_PROP_METAL)

#if RT_FEAT_FRESNEL_METAL_SLOW

        /* compute Fresnel for metals slow */
        movpx_rr(Xmm4, Xmm0)
        mulps_rr(Xmm0, Xmm0)
        movpx_ld(Xmm1, Mebp, inf_GPC01)
        subps_rr(Xmm1, Xmm0)
        movpx_ld(Xmm6, Medx, mat_C_RCP)
        movpx_ld(Xmm5, Medx, mat_EXT_2)
        mulps_rr(Xmm6, Xmm6)
        movpx_rr(Xmm7, Xmm6)
        subps_rr(Xmm6, Xmm5)
        subps_rr(Xmm6, Xmm1)
        mulps_rr(Xmm5, Xmm7)
        movpx_ld(Xmm7, Mebp, inf_GPC01)
        addps_ld(Xmm7, Mebp, inf_GPC03)
        mulps_rr(Xmm5, Xmm7)
        movpx_rr(Xmm7, Xmm6)
        mulps_rr(Xmm7, Xmm7)
        addps_rr(Xmm7, Xmm5)
        sqrps_rr(Xmm7, Xmm7)
        addps_rr(Xmm6, Xmm7)
        mulps_ld(Xmm6, Mebp, inf_GPC02)
        andpx_ld(Xmm6, Mebp, inf_GPC04)
        sqrps_rr(Xmm6, Xmm6)
        mulps_rr(Xmm6, Xmm4)
        addps_rr(Xmm6, Xmm6)
        movpx_rr(Xmm2, Xmm0)
        addps_rr(Xmm0, Xmm7)
        mulps_rr(Xmm2, Xmm7)
        movpx_rr(Xmm3, Xmm6)
        mulps_rr(Xmm3, Xmm1)
        mulps_rr(Xmm1, Xmm1)
        addps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm1, Xmm0)
        addps_rr(Xmm0, Xmm6)
        subps_rr(Xmm1, Xmm6)
        divps_rr(Xmm0, Xmm1)
        movpx_rr(Xmm1, Xmm2)
        addps_rr(Xmm2, Xmm3)
        subps_rr(Xmm1, Xmm3)
        divps_rr(Xmm2, Xmm1)
        mulps_rr(Xmm2, Xmm0)
        addps_rr(Xmm0, Xmm2)

#else /* RT_FEAT_FRESNEL_METAL_SLOW */

        /* compute Fresnel for metals fast */
        movpx_ld(Xmm6, Medx, mat_C_RCP)
        movpx_rr(Xmm4, Xmm0)
        mulps_rr(Xmm4, Xmm6)
        addps_rr(Xmm4, Xmm4)
        mulps_rr(Xmm0, Xmm0)
        mulps_rr(Xmm6, Xmm6)
        addps_ld(Xmm6, Medx, mat_EXT_2)
        movpx_rr(Xmm1, Xmm0)
        mulps_rr(Xmm1, Xmm6)
        addps_rr(Xmm0, Xmm6)
        addps_ld(Xmm1, Mebp, inf_GPC01)
        movpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        addps_rr(Xmm0, Xmm4)
        addps_rr(Xmm1, Xmm4)
        subps_rr(Xmm2, Xmm4)
        subps_rr(Xmm3, Xmm4)
        divps_rr(Xmm0, Xmm2)
        divps_rr(Xmm1, Xmm3)
        addps_rr(Xmm0, Xmm1)

#endif /* RT_FEAT_FRESNEL_METAL_SLOW */

        jmpxx_lb(RF_pre)

    LBL(RF_mtl)

#endif /* RT_FEAT_FRESNEL_METAL */

#if RT_FEAT_FRESNEL_PLAIN

        /* move dot-product temporarily */
        movpx_rr(Xmm4, Xmm0)

        movpx_ld(Xmm6, Medx, mat_C_RFR)
        mulps_rr(Xmm0, Xmm6)
        movpx_rr(Xmm7, Xmm0)
        mulps_rr(Xmm7, Xmm7)
        addps_ld(Xmm7, Mebp, inf_GPC01)
        subps_ld(Xmm7, Medx, mat_RFR_2)
        sqrps_rr(Xmm7, Xmm7)
        addps_rr(Xmm0, Xmm7)

        /* compute Fresnel for opaque */
        movpx_rr(Xmm1, Xmm4)
        movpx_rr(Xmm2, Xmm1)
        mulps_rr(Xmm2, Xmm6)
        subps_rr(Xmm2, Xmm7)
        mulps_rr(Xmm7, Xmm6)
        movpx_rr(Xmm3, Xmm1)
        addps_rr(Xmm1, Xmm7)
        subps_rr(Xmm3, Xmm7)
        divps_rr(Xmm0, Xmm2)
        divps_rr(Xmm1, Xmm3)
        mulps_rr(Xmm0, Xmm0)
        mulps_rr(Xmm1, Xmm1)
        addps_rr(Xmm0, Xmm1)

#endif /* RT_FEAT_FRESNEL_PLAIN */

    LBL(RF_pre)

        /* store Fresnel reflectance */
        mulps_ld(Xmm0, Mebp, inf_GPC02)
        andpx_ld(Xmm0, Mebp, inf_GPC04)
        subps_ld(Xmm0, Mebp, inf_GPC01)
        mulps_ld(Xmm0, Medx, mat_C_RFL)

        movpx_ld(Xmm5, Medx, mat_C_RFL)
        addps_rr(Xmm5, Xmm0)
        movpx_st(Xmm5, Mecx, ctx_C_RFL)

    LBL(RF_frn)

#endif /* RT_FEAT_FRESNEL */

        /* prepare default values */
        xorpx_rr(Xmm0, Xmm0)
        movpx_st(Xmm0, Mecx, ctx_T_NEW)

        xorpx_rr(Xmm1, Xmm1)
        xorpx_rr(Xmm2, Xmm2)
        xorpx_rr(Xmm3, Xmm3)

/************************************ ENTER ***********************************/

        cmjxx_mz(Mebp, inf_DEPTH,
                 EQ_x, RF_mix)

        FETCH_XPTR(Resi, LST_P(SRF))

        movpx_ld(Xmm0, Mecx, ctx_TMASK(0))      /* load tmask */
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))
        orrxx_ri(Reax, IB(RT_FLAG_PASS_BACK))
        addxx_ri(Recx, IH(RT_STACK_STEP))
        subxx_mi(Mebp, inf_DEPTH, IB(1))

        movxx_st(Reax, Mecx, ctx_PARAM(FLG))    /* context flags */
        movxx_st(Redx, Mecx, ctx_PARAM(LST))    /* save material */
        movxx_st(Rebx, Mecx, ctx_PARAM(OBJ))    /* originating surface */
        movwx_mi(Mecx, ctx_PARAM(PTR), IB(2))   /* mark RF_ret with tag 2 */
        movpx_st(Xmm0, Mecx, ctx_WMASK)         /* tmask -> WMASK */

        movxx_ld(Redx, Mebp, inf_CAM)
        movpx_ld(Xmm0, Medx, cam_T_MAX)         /* tmp_v <- T_MAX */
        movpx_st(Xmm0, Mecx, ctx_T_BUF(0))      /* tmp_v -> T_BUF */

        xorpx_rr(Xmm0, Xmm0)                    /* tmp_v <-     0 */
        movpx_st(Xmm0, Mecx, ctx_C_BUF(0))      /* tmp_v -> C_BUF */
        movpx_st(Xmm0, Mecx, ctx_COL_R(0))      /* tmp_v -> COL_R */
        movpx_st(Xmm0, Mecx, ctx_COL_G(0))      /* tmp_v -> COL_G */
        movpx_st(Xmm0, Mecx, ctx_COL_B(0))      /* tmp_v -> COL_B */

        movpx_st(Xmm0, Mecx, ctx_T_MIN)         /* tmp_v -> T_MIN */
        adrpx_ld(Reax, Mecx, ctx_LOCAL(0))
        movpx_st(Xmm0, Oeax, PLAIN)             /* tmp_v -> LOCAL */
        addxx_ri(Reax, IM(RT_SIMD_QUADS*16))
        movpx_st(Xmm0, Oeax, PLAIN)             /* tmp_v -> LOCAL */

        jmpxx_lb(OO_cyc)

    LBL(RF_ret)

        movxx_ld(Redx, Mecx, ctx_PARAM(LST))    /* restore material */
        movxx_ld(Rebx, Mecx, ctx_PARAM(OBJ))    /* restore surface */

        movpx_ld(Xmm1, Mecx, ctx_COL_R(0))
        movpx_ld(Xmm2, Mecx, ctx_COL_G(0))
        movpx_ld(Xmm3, Mecx, ctx_COL_B(0))

        addxx_mi(Mebp, inf_DEPTH, IB(1))
        subxx_ri(Recx, IH(RT_STACK_STEP))

        movpx_ld(Xmm0, Mecx, ctx_C_RFL)

        mulps_rr(Xmm1, Xmm0)
        mulps_rr(Xmm2, Xmm0)
        mulps_rr(Xmm3, Xmm0)

/************************************ LEAVE ***********************************/

    LBL(RF_mix)

        movpx_ld(Xmm4, Mecx, ctx_COL_R(0))
        movpx_ld(Xmm5, Mecx, ctx_COL_G(0))
        movpx_ld(Xmm6, Mecx, ctx_COL_B(0))

        /* accumulate colors */
        addps_rr(Xmm1, Xmm4)
        addps_rr(Xmm2, Xmm5)
        addps_rr(Xmm3, Xmm6)

        /* reflection R */
        STORE_SIMD(COL_R, Xmm1)
        /* reflection G */
        STORE_SIMD(COL_G, Xmm2)
        /* reflection B */
        STORE_SIMD(COL_B, Xmm3)

        jmpxx_lb(RF_out)

    LBL(RF_end)

#if RT_FEAT_FRESNEL

        CHECK_PROP(RF_opq, RT_PROP_OPAQUE)

        jmpxx_lb(RF_out)

    LBL(RF_opq)

        CHECK_PROP(RF_out, RT_PROP_FRESNEL)

        jmpxx_lb(RF_ini)

#endif /* RT_FEAT_FRESNEL */

    LBL(RF_out)

#endif /* RT_FEAT_REFLECTIONS || RT_FEAT_FRESNEL */

/******************************************************************************/
/********************************   MATERIAL   ********************************/
/******************************************************************************/

        movxx_ld(Resi, Mecx, ctx_LOCAL(LST))

        movwx_ld(Reax, Mecx, ctx_LOCAL(PTR))

        cmjwx_ri(Reax, IB(1),
                 EQ_x, SR_rt1)
        cmjwx_ri(Reax, IB(2),
                 EQ_x, SR_rt2)
        cmjwx_ri(Reax, IB(4),
                 EQ_x, SR_rt4)
        cmjwx_ri(Reax, IB(6),
                 EQ_x, SR_rt6)

/******************************************************************************/
/**********************************   ARRAY   *********************************/
/******************************************************************************/

#if RT_FEAT_BOUND_VOL_ARRAY

    LBL(AR_ptr)

        movwx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iecx */

        /* "x" section */
        movpx_ld(Xmm1, Iecx, ctx_RAY_X)         /* ray_x <- RAY_X */
        movpx_ld(Xmm0, Mebx, srf_SCI_X)         /* sri_x <- SCI_X */
        mulps_rr(Xmm0, Xmm1)                    /* sri_x *= ray_x */
        movpx_ld(Xmm5, Iecx, ctx_DFF_X)         /* dff_x <- DFF_X */
        movpx_ld(Xmm7, Mebx, srf_SCI_X)         /* sdi_x <- SCI_X */
        mulps_rr(Xmm7, Xmm5)                    /* sdi_x *= dff_x */
        movpx_rr(Xmm3, Xmm1)                    /* ray_x <- ray_x */
        mulps_rr(Xmm1, Xmm0)                    /* ray_x *= sri_x */
        mulps_rr(Xmm3, Xmm7)                    /* ray_x *= sdi_x */
        mulps_rr(Xmm5, Xmm7)                    /* dff_x *= sdi_x */

        /* "y" section */
        movpx_ld(Xmm2, Iecx, ctx_RAY_Y)         /* ray_y <- RAY_Y */
        movpx_ld(Xmm0, Mebx, srf_SCI_Y)         /* sri_y <- SCI_Y */
        mulps_rr(Xmm0, Xmm2)                    /* sri_y *= ray_y */
        movpx_ld(Xmm6, Iecx, ctx_DFF_Y)         /* dff_y <- DFF_Y */
        movpx_ld(Xmm7, Mebx, srf_SCI_Y)         /* sdi_y <- SCI_Y */
        mulps_rr(Xmm7, Xmm6)                    /* sdi_y *= dff_y */
        movpx_rr(Xmm4, Xmm2)                    /* ray_y <- ray_y */
        mulps_rr(Xmm2, Xmm0)                    /* ray_y *= sri_y */
        mulps_rr(Xmm4, Xmm7)                    /* ray_y *= sdi_y */
        mulps_rr(Xmm6, Xmm7)                    /* dff_y *= sdi_y */

        /* "+" section */
        addps_rr(Xmm1, Xmm2)                    /* axx_x += axx_y */
        addps_rr(Xmm3, Xmm4)                    /* bxx_x += bxx_y */
        addps_rr(Xmm5, Xmm6)                    /* cxx_x += cxx_y */

        /* "z" section */
        movpx_ld(Xmm2, Iecx, ctx_RAY_Z)         /* ray_z <- RAY_Z */
        movpx_ld(Xmm0, Mebx, srf_SCI_Z)         /* sri_z <- SCI_Z */
        mulps_rr(Xmm0, Xmm2)                    /* sri_z *= ray_z */
        movpx_ld(Xmm6, Iecx, ctx_DFF_Z)         /* dff_z <- DFF_Z */
        movpx_ld(Xmm7, Mebx, srf_SCI_Z)         /* sdi_z <- SCI_Z */
        mulps_rr(Xmm7, Xmm6)                    /* sdi_z *= dff_z */
        movpx_rr(Xmm4, Xmm2)                    /* ray_z <- ray_z */
        mulps_rr(Xmm2, Xmm0)                    /* ray_z *= sri_z */
        mulps_rr(Xmm4, Xmm7)                    /* ray_z *= sdi_z */
        mulps_rr(Xmm6, Xmm7)                    /* dff_z *= sdi_z */

        /* "+" section */
        addps_rr(Xmm1, Xmm2)                    /* axx_t += axx_z */
        addps_rr(Xmm3, Xmm4)                    /* bxx_t += bxx_z */
        addps_rr(Xmm5, Xmm6)                    /* cxx_t += cxx_z */

        subps_ld(Xmm5, Mebx, srf_SCI_W)         /* cxx_t -= RAD_2 */

        /* "d" section */
        movpx_rr(Xmm6, Xmm5)                    /* c_val <- c_val */
        mulps_rr(Xmm5, Xmm1)                    /* c_val *= a_val */
        movpx_rr(Xmm4, Xmm3)                    /* b_val <- b_val */
        mulps_rr(Xmm3, Xmm3)                    /* b_val *= b_val */
        subps_rr(Xmm3, Xmm5)                    /* d_bxb -= d_axc */

#if RT_SHOW_BOUND

        movxx_ld(Redx, Mecx, ctx_PARAM(OBJ))
        cmjxx_rz(Redx,
                 EQ_x, AR_shb)

        cmjwx_mi(Medx, srf_SRF_T(TAG), IB(RT_TAG_SURFACE_MAX),
                 NE_x, AR_shn)

    LBL(AR_shb)

        jmpxx_lb(QD_rts)

    LBL(AR_shn)

#endif /* RT_SHOW_BOUND */

        /* create xmask */
        xorpx_rr(Xmm7, Xmm7)                    /* d_min <-     0 */
        cleps_rr(Xmm7, Xmm3)                    /* d_min <= d_val */
        andpx_ld(Xmm7, Mecx, ctx_WMASK)         /* xmask &= WMASK */
        CHECK_MASK(AR_skp, NONE, Xmm7)
        jmpxx_lb(OO_end)

    LBL(AR_skp)

        /* if all rays within SIMD missed the bounding volume,
         * skip array's contents in the list completely */
        movxx_ld(Reax, Mesi, elm_DATA)
        shrxx_ri(Reax, IB(2))
        shlxx_ri(Reax, IB(2))
        movxx_rr(Resi, Reax)

        /* check if surface is trnode's
         * last element for transform caching */
        cmjxx_rm(Resi, Mecx, ctx_LOCAL(OBJ),
                 NE_x, OO_end)

        /* reset ctx_LOCAL(OBJ) if so */
        movxx_mi(Mecx, ctx_LOCAL(OBJ), IB(0))
        jmpxx_lb(OO_end)

#endif /* RT_FEAT_BOUND_VOL_ARRAY */

/******************************************************************************/
/**********************************   PLANE   *********************************/
/******************************************************************************/

    LBL(PL_ptr)

#if RT_QUAD_DEBUG == 1

        /* reset debug info if not complete */
        cmjxx_mi(Mebp, inf_Q_DBG, IB(7),
                 EQ_x, PL_go1)
        movxx_mi(Mebp, inf_Q_DBG, IB(1))

    LBL(PL_go1)

#endif /* RT_QUAD_DEBUG */

#if RT_SHOW_TILES

        SHOW_TILES(PL, 0x00880000)

#endif /* RT_SHOW_TILES */

        /* plane ignores secondary rays originating from itself
         * as it cannot possibly interact with them directly */
        cmjxx_rm(Rebx, Mecx, ctx_PARAM(OBJ),
                 EQ_x, OO_end)

        /* "k" section */
        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        MOVXR_LD(Xmm4, Iecx, ctx_DFF_O)         /* dff_k <- DFF_K */
        MOVXR_LD(Xmm3, Iecx, ctx_RAY_O)         /* ray_k <- RAY_K */
        xorpx_ld(Xmm4, Mebx, srf_SMASK)         /* dff_k = -dff_k */

        /* create xmask */
        xorpx_rr(Xmm7, Xmm7)                    /* xmask <-     0 */
        cneps_rr(Xmm7, Xmm3)                    /* xmask != ray_k */
        andpx_ld(Xmm7, Mecx, ctx_WMASK)         /* xmask &= WMASK */

        /* "tt" section */
        divps_rr(Xmm4, Xmm3)                    /* dff_k /= ray_k */
        movpx_st(Xmm4, Mecx, ctx_T_VAL(0))      /* t_rt1 -> T_VAL */

        /* clipping */
        SUBROUTINE(0, CC_clp)
        CHECK_MASK(OO_end, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_XMASK)         /* xmask -> XMASK */

        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        MOVXR_LD(Xmm3, Iecx, ctx_RAY_O)         /* ray_k <- RAY_K */
        xorpx_rr(Xmm0, Xmm0)                    /* tmp_v <-     0 */

/******************************************************************************/
/*  LBL(PL_rt1)  */

        /* outer side */
        cltps_rr(Xmm3, Xmm0)                    /* ray_k <! tmp_v */
        andpx_rr(Xmm7, Xmm3)                    /* tmask &= lmask */
        movpx_st(Xmm7, Mecx, ctx_TMASK(0))      /* tmask -> TMASK */
        CHECK_MASK(PL_rt2, NONE, Xmm7)

        /* material */
        movxx_mi(Mecx, ctx_LOCAL(FLG), IB(RT_FLAG_SIDE_OUTER))
        SUBROUTINE(1, PL_mat)

/******************************************************************************/
    LBL(PL_rt2)

        /* inner side */
        movpx_ld(Xmm7, Mecx, ctx_TMASK(0))      /* tmask <- TMASK */
        xorpx_ld(Xmm7, Mecx, ctx_XMASK)         /* tmask ^= XMASK */
        CHECK_MASK(OO_end, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_TMASK(0))      /* tmask -> TMASK */

        /* material */
        movxx_mi(Mecx, ctx_LOCAL(FLG), IB(RT_FLAG_SIDE_INNER))
        SUBROUTINE(2, PL_mat)

        jmpxx_lb(OO_end)

/******************************************************************************/
    LBL(PL_mat)

        FETCH_PROP()                            /* Xmm7  <- tside */

#if RT_FEAT_LIGHTS_SHADOWS

        CHECK_SHAD(PL_shd)

#endif /* RT_FEAT_LIGHTS_SHADOWS */

#if RT_FEAT_TEXTURING

        /* compute surface's UV coords
         * for texturing, if enabled */
        CHECK_PROP(PL_tex, RT_PROP_TEXTURE)

        INDEX_AXIS(RT_I)                        /* eax   <-     i */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        MOVXR_LD(Xmm4, Iecx, ctx_NEW_O)         /* loc_i <- NEW_I */
        movpx_st(Xmm4, Mecx, ctx_TEX_U)         /* loc_i -> TEX_U */

        INDEX_AXIS(RT_J)                        /* eax   <-     j */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        MOVXR_LD(Xmm5, Iecx, ctx_NEW_O)         /* loc_j <- NEW_J */
        movpx_st(Xmm5, Mecx, ctx_TEX_V)         /* loc_j -> TEX_V */

    LBL(PL_tex)

#endif /* RT_FEAT_TEXTURING */

#if RT_FEAT_NORMALS

        /* compute normal, if enabled */
        CHECK_PROP(PL_nrm, RT_PROP_NORMAL)

        INDEX_AXIS(RT_I)                        /* eax   <-     i */
        MOVZR_ST(Xmm4, Iecx, ctx_NRM_O)         /* 0     -> NRM_I */

        INDEX_AXIS(RT_J)                        /* eax   <-     j */
        MOVZR_ST(Xmm5, Iecx, ctx_NRM_O)         /* 0     -> NRM_J */

        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        movpx_ld(Xmm6, Mebp, inf_GPC01)         /* tmp_v <- +1.0f */
        xorpx_rr(Xmm6, Xmm7)                    /* tmp_v ^= tside */
        MOVXR_ST(Xmm6, Iecx, ctx_NRM_O)         /* tmp_v -> NRM_K */

        jmpxx_lb(MT_nrm)

    LBL(PL_nrm)

#endif /* RT_FEAT_NORMALS */

        jmpxx_lb(MT_mat)

/******************************************************************************/
#if RT_FEAT_CLIPPING_CUSTOM

    LBL(PL_clp)

        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        MOVXR_LD(Xmm4, Iecx, ctx_NRM_O)         /* dff_k <- NRM_K */
        xorpx_rr(Xmm0, Xmm0)                    /* tmp_v <-     0 */

        APPLY_CLIP(PL, Xmm4, Xmm0)

        jmpxx_lb(CC_ret)

#endif /* RT_FEAT_CLIPPING_CUSTOM */

/******************************************************************************/
/********************************   TWO-PLANE   *******************************/
/******************************************************************************/

    LBL(TP_ptr)

#if RT_SHOW_TILES

        SHOW_TILES(TP, 0x00884444)

#endif /* RT_SHOW_TILES */

        /* "i" section */
        movwx_ld(Reax, Mebx, srf_A_MAP(RT_I*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm1, Iecx, ctx_RAY_O)         /* ray_i <- RAY_I */
        movpx_ld(Xmm5, Iecx, ctx_DFF_O)         /* dff_i <- DFF_I */
        subwx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iebx */
        movpx_ld(Xmm3, Iebx, srf_SCI_O)         /* sci_i <- SCI_I */

        /* "k" section */
        movwx_ld(Reax, Mebx, srf_A_MAP(RT_K*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm2, Iecx, ctx_RAY_O)         /* ray_k <- RAY_K */
        movpx_ld(Xmm6, Iecx, ctx_DFF_O)         /* dff_k <- DFF_K */
        subwx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iebx */
        movpx_ld(Xmm4, Iebx, srf_SCI_O)         /* sci_k <- SCI_K */

        /* "d" section */
        movpx_rr(Xmm0, Xmm5)                    /* dff_i <- dff_i */
        movpx_rr(Xmm7, Xmm6)                    /* dff_k <- dff_k */
        mulps_rr(Xmm6, Xmm1)                    /* dff_k *= ray_i */
        mulps_rr(Xmm5, Xmm2)                    /* dff_i *= ray_k */
        subps_rr(Xmm5, Xmm6)                    /* dxx_i -= dxx_k */
        mulps_rr(Xmm5, Xmm5)                    /* dxx_t *= dxx_t */
        mulps_rr(Xmm5, Xmm3)                    /* dxx_t *= sci_i */
        mulps_rr(Xmm5, Xmm4)                    /* dxx_t *= sci_k */
        andpx_ld(Xmm5, Mebp, inf_GPC04)         /* dxx_t = |dxx_t|*/

        /* "b" section */
        movpx_rr(Xmm6, Xmm3)                    /* sci_i <- sci_i */
        mulps_rr(Xmm3, Xmm0)                    /* sci_i *= dff_i */
        mulps_rr(Xmm4, Xmm7)                    /* sci_k *= dff_k */
        mulps_rr(Xmm3, Xmm1)                    /* bxx_i *= ray_i */
        mulps_rr(Xmm4, Xmm2)                    /* bxx_k *= ray_k */
        addps_rr(Xmm3, Xmm4)                    /* bxx_i += bxx_k */

        /* "c" section */
        movpx_ld(Xmm4, Iebx, srf_SCI_O)         /* sci_k <- SCI_K */
        mulps_rr(Xmm0, Xmm0)                    /* dff_i *= dff_i */
        mulps_rr(Xmm7, Xmm7)                    /* dff_k *= dff_k */
        mulps_rr(Xmm0, Xmm6)                    /* dff_i *= sci_i */
        mulps_rr(Xmm7, Xmm4)                    /* dff_k *= sci_k */
        addps_rr(Xmm0, Xmm7)                    /* cxx_i += cxx_k */

        /* "a" section */
        mulps_rr(Xmm1, Xmm1)                    /* ray_i *= ray_i */
        mulps_rr(Xmm2, Xmm2)                    /* ray_k *= ray_k */
        mulps_rr(Xmm1, Xmm6)                    /* ray_i *= sci_i */
        mulps_rr(Xmm2, Xmm4)                    /* ray_k *= sci_k */
        addps_rr(Xmm1, Xmm2)                    /* axx_i += axx_k */

        /* mov section */
        movpx_rr(Xmm6, Xmm0)                    /* c_val <- c_val */
        movpx_rr(Xmm4, Xmm3)                    /* b_val <- b_val */
        movpx_rr(Xmm3, Xmm5)                    /* d_val <- d_val */

        jmpxx_lb(QD_rts)                        /* quadric  roots */

/******************************************************************************/
    LBL(TP_mat)

        FETCH_PROP()                            /* Xmm7  <- tside */

#if RT_FEAT_LIGHTS_SHADOWS

        CHECK_SHAD(TP_shd)

#endif /* RT_FEAT_LIGHTS_SHADOWS */

#if RT_FEAT_NORMALS

        /* compute normal, if enabled */
        CHECK_PROP(TP_nrm, RT_PROP_NORMAL)

        movwx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iecx */

        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        movpx_ld(Xmm4, Iecx, ctx_NEW_X)         /* loc_x <- NEW_X */
        movpx_ld(Xmm5, Iecx, ctx_NEW_Y)         /* loc_y <- NEW_Y */
        movpx_ld(Xmm6, Iecx, ctx_NEW_Z)         /* loc_z <- NEW_Z */

        mulps_ld(Xmm4, Mebx, srf_SCI_X)         /* loc_x *= SCI_X */
        mulps_ld(Xmm5, Mebx, srf_SCI_Y)         /* loc_y *= SCI_Y */
        mulps_ld(Xmm6, Mebx, srf_SCI_Z)         /* loc_z *= SCI_Z */

        /* normalize normal */
        movpx_rr(Xmm1, Xmm4)                    /* loc_x <- loc_x */
        movpx_rr(Xmm2, Xmm5)                    /* loc_y <- loc_y */
        movpx_rr(Xmm3, Xmm6)                    /* loc_z <- loc_z */

        mulps_rr(Xmm1, Xmm4)                    /* loc_x *= loc_x */
        mulps_rr(Xmm2, Xmm5)                    /* loc_y *= loc_y */
        mulps_rr(Xmm3, Xmm6)                    /* loc_z *= loc_z */

        addps_rr(Xmm1, Xmm2)                    /* lc2_x += lc2_y */
        addps_rr(Xmm1, Xmm3)                    /* lc2_t += lc2_z */
        rsqps_rr(Xmm0, Xmm1) /* destroys Xmm1 *//* inv_r rs loc_r */
        xorpx_rr(Xmm0, Xmm7)                    /* inv_r ^= tside */

        mulps_rr(Xmm4, Xmm0)                    /* loc_x *= inv_r */
        mulps_rr(Xmm5, Xmm0)                    /* loc_y *= inv_r */
        mulps_rr(Xmm6, Xmm0)                    /* loc_z *= inv_r */

        /* store normal */
        movpx_st(Xmm4, Iecx, ctx_NRM_X)         /* loc_x -> NRM_X */
        movpx_st(Xmm5, Iecx, ctx_NRM_Y)         /* loc_y -> NRM_Y */
        movpx_st(Xmm6, Iecx, ctx_NRM_Z)         /* loc_z -> NRM_Z */

        jmpxx_lb(MT_nrm)

    LBL(TP_nrm)

#endif /* RT_FEAT_NORMALS */

        jmpxx_lb(MT_mat)

/******************************************************************************/
#if RT_FEAT_CLIPPING_CUSTOM

    LBL(TP_clp)

        movwx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iecx */

        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        movpx_ld(Xmm4, Iecx, ctx_NRM_X)         /* dff_x <- DFF_X */
        mulps_rr(Xmm4, Xmm4)                    /* df2_x *= dff_x */
        mulps_ld(Xmm4, Mebx, srf_SCI_X)         /* df2_x *= SCI_X */

        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        movpx_ld(Xmm5, Iecx, ctx_NRM_Y)         /* dff_y <- DFF_Y */
        mulps_rr(Xmm5, Xmm5)                    /* df2_y *= dff_y */
        mulps_ld(Xmm5, Mebx, srf_SCI_Y)         /* df2_y *= SCI_Y */

        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        movpx_ld(Xmm6, Iecx, ctx_NRM_Z)         /* dff_z <- DFF_Z */
        mulps_rr(Xmm6, Xmm6)                    /* df2_z *= dff_z */
        mulps_ld(Xmm6, Mebx, srf_SCI_Z)         /* df2_z *= SCI_Z */

        subps_ld(Xmm4, Mebx, srf_SCI_W)         /* dff_x -= SCI_W */
        addps_rr(Xmm4, Xmm5)                    /* dff_x += dff_y */
        addps_rr(Xmm4, Xmm6)                    /* dff_x += dff_z */
        xorpx_rr(Xmm0, Xmm0)                    /* tmp_v <-     0 */

        APPLY_CLIP(TP, Xmm4, Xmm0)

        jmpxx_lb(CC_ret)

#endif /* RT_FEAT_CLIPPING_CUSTOM */

/******************************************************************************/
/*********************************   QUADRIC   ********************************/
/******************************************************************************/

    LBL(QD_ptr)

#if RT_SHOW_TILES

        SHOW_TILES(QD, 0x00448844)

#endif /* RT_SHOW_TILES */

        movwx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iecx */

        /* "x" section */
        movpx_ld(Xmm1, Iecx, ctx_RAY_X)         /* ray_x <- RAY_X */
        movpx_ld(Xmm0, Mebx, srf_SCI_X)         /* sri_x <- SCI_X */
        mulps_rr(Xmm0, Xmm1)                    /* sri_x *= ray_x */
        movpx_ld(Xmm5, Iecx, ctx_DFF_X)         /* dff_x <- DFF_X */
        movpx_ld(Xmm7, Mebx, srf_SCI_X)         /* sdi_x <- SCI_X */
        mulps_rr(Xmm7, Xmm5)                    /* sdi_x *= dff_x */
        subps_ld(Xmm7, Mebx, srf_SCJ_X)         /* sdi_x -= SCJ_X */
        movpx_rr(Xmm3, Xmm1)                    /* ray_x <- ray_x */
        mulps_rr(Xmm1, Xmm0)                    /* ray_x *= sri_x */
        mulps_rr(Xmm3, Xmm7)                    /* ray_x *= sdi_x */
        subps_ld(Xmm7, Mebx, srf_SCJ_X)         /* sdi_x -= SCJ_X */
        mulps_rr(Xmm5, Xmm7)                    /* dff_x *= sdi_x */

        /* "y" section */
        movpx_ld(Xmm2, Iecx, ctx_RAY_Y)         /* ray_y <- RAY_Y */
        movpx_ld(Xmm0, Mebx, srf_SCI_Y)         /* sri_y <- SCI_Y */
        mulps_rr(Xmm0, Xmm2)                    /* sri_y *= ray_y */
        movpx_ld(Xmm6, Iecx, ctx_DFF_Y)         /* dff_y <- DFF_Y */
        movpx_ld(Xmm7, Mebx, srf_SCI_Y)         /* sdi_y <- SCI_Y */
        mulps_rr(Xmm7, Xmm6)                    /* sdi_y *= dff_y */
        subps_ld(Xmm7, Mebx, srf_SCJ_Y)         /* sdi_y -= SCJ_Y */
        movpx_rr(Xmm4, Xmm2)                    /* ray_y <- ray_y */
        mulps_rr(Xmm2, Xmm0)                    /* ray_y *= sri_y */
        mulps_rr(Xmm4, Xmm7)                    /* ray_y *= sdi_y */
        subps_ld(Xmm7, Mebx, srf_SCJ_Y)         /* sdi_y -= SCJ_Y */
        mulps_rr(Xmm6, Xmm7)                    /* dff_y *= sdi_y */

        /* "+" section */
        addps_rr(Xmm1, Xmm2)                    /* axx_x += axx_y */
        addps_rr(Xmm3, Xmm4)                    /* bxx_x += bxx_y */
        addps_rr(Xmm5, Xmm6)                    /* cxx_x += cxx_y */

        /* "z" section */
        movpx_ld(Xmm2, Iecx, ctx_RAY_Z)         /* ray_z <- RAY_Z */
        movpx_ld(Xmm0, Mebx, srf_SCI_Z)         /* sri_z <- SCI_Z */
        mulps_rr(Xmm0, Xmm2)                    /* sri_z *= ray_z */
        movpx_ld(Xmm6, Iecx, ctx_DFF_Z)         /* dff_z <- DFF_Z */
        movpx_ld(Xmm7, Mebx, srf_SCI_Z)         /* sdi_z <- SCI_Z */
        mulps_rr(Xmm7, Xmm6)                    /* sdi_z *= dff_z */
        subps_ld(Xmm7, Mebx, srf_SCJ_Z)         /* sdi_z -= SCJ_Z */
        movpx_rr(Xmm4, Xmm2)                    /* ray_z <- ray_z */
        mulps_rr(Xmm2, Xmm0)                    /* ray_z *= sri_z */
        mulps_rr(Xmm4, Xmm7)                    /* ray_z *= sdi_z */
        subps_ld(Xmm7, Mebx, srf_SCJ_Z)         /* sdi_z -= SCJ_Z */
        mulps_rr(Xmm6, Xmm7)                    /* dff_z *= sdi_z */

        /* "+" section */
        addps_rr(Xmm1, Xmm2)                    /* axx_t += axx_z */
        addps_rr(Xmm3, Xmm4)                    /* bxx_t += bxx_z */
        addps_rr(Xmm5, Xmm6)                    /* cxx_t += cxx_z */

        subps_ld(Xmm5, Mebx, srf_SCI_W)         /* cxx_t -= SCI_W */

        /* "d" section */
        movpx_rr(Xmm6, Xmm5)                    /* c_val <- c_val */
        mulps_rr(Xmm5, Xmm1)                    /* c_val *= a_val */
        movpx_rr(Xmm4, Xmm3)                    /* b_val <- b_val */
        mulps_rr(Xmm3, Xmm3)                    /* b_val *= b_val */
        subps_rr(Xmm3, Xmm5)                    /* d_bxb -= d_axc */

    LBL(QD_rts)

        /* create xmask */
        xorpx_rr(Xmm7, Xmm7)                    /* d_min <-     0 */
        cleps_rr(Xmm7, Xmm3)                    /* d_min <= d_val */
        andpx_ld(Xmm7, Mecx, ctx_WMASK)         /* xmask &= WMASK */
        CHECK_MASK(OO_end, NONE, Xmm7)

        xorpx_ld(Xmm4, Mebx, srf_SMASK)         /* b_val = -b_val */

        /* compute dmask */
        movpx_rr(Xmm5, Xmm3)                    /* dmask <- d_val */
        cltps_ld(Xmm5, Mebx, srf_D_EPS)         /* dmask <! D_EPS */
        andpx_rr(Xmm5, Xmm7)                    /* dmask &= xmask */
        movpx_st(Xmm5, Mecx, ctx_DMASK)         /* dmask -> DMASK */

#if RT_QUAD_DEBUG == 1

#if 0
        cmjxx_mi(Mebp, inf_FRM_X, IH(0),        /* <- pin point buggy quad */
                 NE_x, QD_go1)
        cmjxx_mi(Mebp, inf_FRM_Y, IH(0),        /* <- pin point buggy quad */
                 NE_x, QD_go1)

        xorpx_rr(Xmm7, Xmm7)      /* <- mark buggy quad, remove when found */
#else
        /* reset debug info if not complete */
        cmjxx_mi(Mebp, inf_Q_DBG, IB(7),
                 EQ_x, QD_go1)
        movxx_mi(Mebp, inf_Q_DBG, IB(1))

        CHECK_MASK(QD_go1, NONE, Xmm5)
#endif

        cmjxx_mi(Mebp, inf_Q_DBG, IB(1),
                 NE_x, QD_go1)
        movxx_mi(Mebp, inf_Q_DBG, IB(2))

        movxx_ld(Reax, Mebp, inf_DEPTH)
        movxx_st(Reax, Mebp, inf_Q_CNT)

        movpx_ld(Xmm5, Mecx, ctx_WMASK)
        movpx_st(Xmm5, Mebp, inf_WMASK)

        movwx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iecx */

        movpx_ld(Xmm2, Iecx, ctx_DFF_X)
        movpx_st(Xmm2, Mebp, inf_DFF_X)
        movpx_ld(Xmm2, Iecx, ctx_DFF_Y)
        movpx_st(Xmm2, Mebp, inf_DFF_Y)
        movpx_ld(Xmm2, Iecx, ctx_DFF_Z)
        movpx_st(Xmm2, Mebp, inf_DFF_Z)

        movpx_ld(Xmm5, Iecx, ctx_RAY_X)
        movpx_st(Xmm5, Mebp, inf_RAY_X)
        movpx_ld(Xmm5, Iecx, ctx_RAY_Y)
        movpx_st(Xmm5, Mebp, inf_RAY_Y)
        movpx_ld(Xmm5, Iecx, ctx_RAY_Z)
        movpx_st(Xmm5, Mebp, inf_RAY_Z)

        movpx_st(Xmm1, Mebp, inf_A_VAL)
        movpx_st(Xmm4, Mebp, inf_B_VAL)
        movpx_st(Xmm6, Mebp, inf_C_VAL)
        movpx_st(Xmm3, Mebp, inf_D_VAL)

    LBL(QD_go1)

#endif /* RT_QUAD_DEBUG */

        /* process b-mixed quads */
        movpx_ld(Xmm5, Mebx, srf_SMASK)         /* smask <- SMASK */
        andpx_rr(Xmm5, Xmm4)                    /* smask &= b_val */
        sqrps_rr(Xmm3, Xmm3)                    /* d_val sq d_val */
        xorpx_rr(Xmm3, Xmm5)                    /* d_val ^= bsign */
        addps_rr(Xmm4, Xmm3)                    /* b_val += sdval */

        xorpx_rr(Xmm2, Xmm2)                    /* tmp_v <-     0 */
        cleps_rr(Xmm2, Xmm3)                    /* tmp_v <= sdval */
        xorpx_rr(Xmm5, Xmm5)                    /* tmp_v <-     0 */
        cgtps_rr(Xmm5, Xmm3)                    /* tmp_v >! sdval */
        movpx_rr(Xmm3, Xmm4)                    /* bdval <- bdval */

        movpx_rr(Xmm0, Xmm6)                    /* c_val <- c_val */
        andpx_rr(Xmm6, Xmm5)                    /* c_val &= m_neg */
        andpx_rr(Xmm0, Xmm2)                    /* c_val &= m_pos */
        andpx_rr(Xmm4, Xmm5)                    /* bdval &= m_neg */
        andpx_rr(Xmm3, Xmm2)                    /* bdval &= m_pos */
        orrpx_rr(Xmm6, Xmm3)                    /* c_neg |= bdpos */
        orrpx_rr(Xmm3, Xmm4)                    /* bdpos |= bdneg */
        orrpx_rr(Xmm4, Xmm0)                    /* bdneg |= c_pos */

        movpx_rr(Xmm0, Xmm1)                    /* a_val <- a_val */
        andpx_rr(Xmm1, Xmm5)                    /* a_val &= m_neg */
        andpx_rr(Xmm0, Xmm2)                    /* a_val &= m_pos */
        andpx_rr(Xmm2, Xmm3)                    /* bdval &= m_pos */
        andpx_rr(Xmm3, Xmm5)                    /* bdval &= m_neg */
        orrpx_rr(Xmm3, Xmm0)                    /* bdneg |= a_pos */
        orrpx_rr(Xmm0, Xmm1)                    /* a_pos |= a_neg */
        orrpx_rr(Xmm1, Xmm2)                    /* a_neg |= bdpos */

#if RT_QUAD_DEBUG == 1

        cmjxx_mi(Mebp, inf_Q_DBG, IB(2),
                 NE_x, QD_go2)
        movxx_mi(Mebp, inf_Q_DBG, IB(3))

        movpx_st(Xmm4, Mebp, inf_T1NMR)
        movpx_st(Xmm1, Mebp, inf_T1DNM)
        movpx_st(Xmm6, Mebp, inf_T2NMR)
        movpx_st(Xmm3, Mebp, inf_T2DNM)

        movpx_rr(Xmm2, Xmm4)
        divps_rr(Xmm2, Xmm1)                    /* t1nmr /= t1dnm */
        movpx_st(Xmm2, Mebp, inf_T1VAL)

        movpx_rr(Xmm5, Xmm6)
        divps_rr(Xmm5, Xmm3)                    /* t2nmr /= t2dnm */
        movpx_st(Xmm5, Mebp, inf_T2VAL)

    LBL(QD_go2)

#endif /* RT_QUAD_DEBUG */

        /* root sorting
         * for near-zero determinant */
        movwx_mi(Mecx, ctx_XMISC(PTR), IB(0))
        movpx_ld(Xmm5, Mecx, ctx_DMASK)         /* dmask <- DMASK */
        CHECK_MASK(QD_srt, NONE, Xmm5)
        movwx_mi(Mecx, ctx_XMISC(PTR), IB(1))

        /* compute amask */
        movpx_ld(Xmm2, Mebx, srf_SMASK)         /* amask <- smask */
        andpx_rr(Xmm2, Xmm0)                    /* amask &= a_val */
        movpx_st(Xmm2, Mecx, ctx_AMASK)         /* amask -> AMASK */
        xorpx_rr(Xmm5, Xmm5)                    /* tmp_v <-     0 */

        /* convert nan root to zero */
        movpx_rr(Xmm2, Xmm4)                    /* t1zro <- t1nmr */
        ceqps_rr(Xmm2, Xmm5)                    /* t1zro == tmp_v */
        orrpx_rr(Xmm1, Xmm2)                    /* t1dnm |= t1zro */
        xorpx_rr(Xmm1, Xmm2)                    /* t1dnm ^= t1zro */
        andpx_ld(Xmm2, Mebp, inf_GPC01)         /* t1zro &= +1.0f */
        orrpx_rr(Xmm1, Xmm2)                    /* t1dnm |= t1zro */

        /* convert nan root to zero */
        movpx_rr(Xmm2, Xmm6)                    /* t2zro <- t2nmr */
        ceqps_rr(Xmm2, Xmm5)                    /* t2zro == tmp_v */
        orrpx_rr(Xmm3, Xmm2)                    /* t2dnm |= t2zro */
        xorpx_rr(Xmm3, Xmm2)                    /* t2dnm ^= t2zro */
        andpx_ld(Xmm2, Mebp, inf_GPC01)         /* t2zro &= +1.0f */
        orrpx_rr(Xmm3, Xmm2)                    /* t2dnm |= t2zro */

        /* compute both roots */
        divps_rr(Xmm4, Xmm1)                    /* t1nmr /= t1dnm */
        divps_rr(Xmm6, Xmm3)                    /* t2nmr /= t2dnm */
        cneps_rr(Xmm1, Xmm5)                    /* t1msk != tmp_v */
        cneps_rr(Xmm3, Xmm5)                    /* t2msk != tmp_v */

        /* equate then split roots */
        movpx_rr(Xmm2, Xmm4)                    /* t_dff <- t1val */
        subps_rr(Xmm2, Xmm6)                    /* t_dff -= t2val */
        xorpx_ld(Xmm2, Mecx, ctx_AMASK)         /* t_dff ^= amask */
        cleps_rr(Xmm5, Xmm2)                    /* tmp_v <= t_dff */
        andpx_rr(Xmm2, Xmm5)                    /* t_dff &= fmask */
        andpx_ld(Xmm5, Mebx, srf_T_EPS)         /* fmask &= T_EPS */
        mulps_rr(Xmm5, Xmm4)                    /* t_eps *= t1val */
        andpx_ld(Xmm5, Mebp, inf_GPC04)         /* t_eps = |t_eps|*/
        mulps_ld(Xmm2, Mebp, inf_GPC02)         /* t_dff *= -0.5f */
        subps_rr(Xmm2, Xmm5)                    /* t_dff -= t_eps */
        xorpx_ld(Xmm2, Mecx, ctx_AMASK)         /* t_dff ^= amask */
        andpx_rr(Xmm2, Xmm1)                    /* t_dff &= t1msk */
        andpx_rr(Xmm2, Xmm3)                    /* t_dff &= t2msk */
        andpx_ld(Xmm2, Mecx, ctx_DMASK)         /* t_dff &= DMASK */
        addps_rr(Xmm4, Xmm2)                    /* t1val += t_dff */
        subps_rr(Xmm6, Xmm2)                    /* t2val -= t_dff */

    LBL(QD_srt)

#if RT_QUAD_DEBUG == 1

        cmjxx_mi(Mebp, inf_Q_DBG, IB(3),
                 NE_x, QD_go3)
        movxx_mi(Mebp, inf_Q_DBG, IB(4))

        movpx_ld(Xmm2, Mecx, ctx_DMASK)
        movpx_st(Xmm2, Mebp, inf_DMASK)

        movpx_st(Xmm4, Mebp, inf_T1SRT)
        movpx_st(Xmm6, Mebp, inf_T2SRT)

        movpx_st(Xmm1, Mebp, inf_T1MSK)
        movpx_st(Xmm3, Mebp, inf_T2MSK)

    LBL(QD_go3)

#endif /* RT_QUAD_DEBUG */

        /* process a-mixed quads */
        movwx_mi(Mecx, ctx_XMISC(FLG), IB(2))
        movwx_mi(Mecx, ctx_XMISC(TAG), IB(0))

        xorpx_rr(Xmm5, Xmm5)                    /* tmp_v <-     0 */
        cgtps_rr(Xmm5, Xmm0)                    /* tmp_v >! a_val */
        andpx_rr(Xmm5, Xmm7)                    /* amask &= xmask */
        CHECK_MASK(QD_rc1, NONE, Xmm5)
        xorpx_rr(Xmm5, Xmm7)                    /* amask ^= xmask */
        CHECK_MASK(QD_rc2, NONE, Xmm5)

        movwx_mi(Mecx, ctx_XMISC(TAG), IB(1))
        jmpxx_lb(QD_rc1)

/******************************************************************************/
    LBL(QD_rs1)

#if RT_QUAD_DEBUG == 1

        movxx_ld(Reax, Mebp, inf_DEPTH)
        cmjxx_rm(Reax, Mebp, inf_Q_CNT,
                 NE_x, QD_gs1)

        cmjxx_mi(Mebp, inf_Q_DBG, IB(6),
                 NE_x, QD_gs1)
        movxx_mi(Mebp, inf_Q_DBG, IB(4))

        jmpxx_lb(QD_gr1)        

    LBL(QD_gs1)

        cmjxx_mi(Mebp, inf_Q_DBG, IB(7),
                 EQ_x, QD_gr1)
        movxx_mi(Mebp, inf_Q_DBG, IB(1))

    LBL(QD_gr1)

#endif /* RT_QUAD_DEBUG */

        movpx_ld(Xmm4, Mecx, ctx_XTMP1)         /* t1nmr <- XTMP1 */
        movpx_ld(Xmm1, Mecx, ctx_XTMP2)         /* t1dnm <- XTMP2 */
        movpx_ld(Xmm7, Mecx, ctx_XMASK)         /* xmask <- XMASK */

    LBL(QD_rt1)

        /* side count check */
        cmjwx_mz(Mecx, ctx_XMISC(FLG),
                 EQ_x, OO_end)

    LBL(QD_rc1)

        subwx_mi(Mecx, ctx_XMISC(FLG), IB(1))

        /* outer side */
        CHECK_SIDE(QD_sd1, QD_rt2, RT_FLAG_SIDE_OUTER)

        /* division check */
        cmjwx_mz(Mecx, ctx_XMISC(PTR),
                 NE_x, QD_rd1)

        /* "t1" section */
        divps_rr(Xmm4, Xmm1)                    /* t1nmr /= t1dnm */
        xorpx_rr(Xmm5, Xmm5)                    /* tmp_v <-     0 */
        cneps_rr(Xmm1, Xmm5)                    /* t1msk != tmp_v */

    LBL(QD_rd1)

        movpx_st(Xmm6, Mecx, ctx_XTMP1)         /* t2nmr -> XTMP1 */
        movpx_st(Xmm3, Mecx, ctx_XTMP2)         /* t2dnm -> XTMP2 */
        movpx_st(Xmm7, Mecx, ctx_XMASK)         /* xmask -> XMASK */

        andpx_rr(Xmm7, Xmm1)                    /* tmask &= t1msk */
        movpx_st(Xmm4, Mecx, ctx_T_VAL(0))      /* t_rt1 -> T_VAL */
        movxx_mi(Mecx, ctx_LOCAL(FLG), IB(RT_FLAG_SIDE_OUTER))

        /* clipping */
        SUBROUTINE(3, CC_clp)
        CHECK_MASK(QD_rs2, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_TMASK(0))      /* tmask -> TMASK */

        /* material */
        SUBROUTINE(4, QD_mtr)

        /* side count check */
        cmjwx_mz(Mecx, ctx_XMISC(FLG),
                 EQ_x, OO_end)

        /* overdraw check */
        cmjwx_mz(Mecx, ctx_XMISC(TAG),
                 NE_x, QD_rs2)

        /* optimize overdraw */
        movpx_ld(Xmm7, Mecx, ctx_TMASK(0))      /* tmask <- TMASK */
        xorpx_ld(Xmm7, Mecx, ctx_XMASK)         /* tmask ^= XMASK */
        CHECK_MASK(OO_end, NONE, Xmm7)

/******************************************************************************/
    LBL(QD_rs2)

#if RT_QUAD_DEBUG == 1

        movxx_ld(Reax, Mebp, inf_DEPTH)
        cmjxx_rm(Reax, Mebp, inf_Q_CNT,
                 NE_x, QD_gs2)

        cmjxx_mi(Mebp, inf_Q_DBG, IB(6),
                 NE_x, QD_gs2)
        movxx_mi(Mebp, inf_Q_DBG, IB(4))

        jmpxx_lb(QD_gr2)        

    LBL(QD_gs2)

        cmjxx_mi(Mebp, inf_Q_DBG, IB(7),
                 EQ_x, QD_gr2)
        movxx_mi(Mebp, inf_Q_DBG, IB(1))

    LBL(QD_gr2)

#endif /* RT_QUAD_DEBUG */

        movpx_ld(Xmm6, Mecx, ctx_XTMP1)         /* t2nmr <- XTMP1 */
        movpx_ld(Xmm3, Mecx, ctx_XTMP2)         /* t2dnm <- XTMP2 */
        movpx_ld(Xmm7, Mecx, ctx_XMASK)         /* xmask <- XMASK */

    LBL(QD_rt2)

        /* side count check */
        cmjwx_mz(Mecx, ctx_XMISC(FLG),
                 EQ_x, OO_end)

    LBL(QD_rc2)

        subwx_mi(Mecx, ctx_XMISC(FLG), IB(1))

        /* inner side */
        CHECK_SIDE(QD_sd2, QD_rt1, RT_FLAG_SIDE_INNER)

        /* division check */
        cmjwx_mz(Mecx, ctx_XMISC(PTR),
                 NE_x, QD_rd2)

        /* "t2" section */
        divps_rr(Xmm6, Xmm3)                    /* t2nmr /= t2dnm */
        xorpx_rr(Xmm5, Xmm5)                    /* tmp_v <-     0 */
        cneps_rr(Xmm3, Xmm5)                    /* t2msk != tmp_v */

    LBL(QD_rd2)

        movpx_st(Xmm4, Mecx, ctx_XTMP1)         /* t1nmr -> XTMP1 */
        movpx_st(Xmm1, Mecx, ctx_XTMP2)         /* t1dnm -> XTMP2 */
        movpx_st(Xmm7, Mecx, ctx_XMASK)         /* xmask -> XMASK */

        andpx_rr(Xmm7, Xmm3)                    /* tmask &= t2msk */
        movpx_st(Xmm6, Mecx, ctx_T_VAL(0))      /* t_rt2 -> T_VAL */
        movxx_mi(Mecx, ctx_LOCAL(FLG), IB(RT_FLAG_SIDE_INNER))

        /* clipping */
        SUBROUTINE(5, CC_clp)
        CHECK_MASK(QD_rs1, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_TMASK(0))      /* tmask -> TMASK */

        /* material */
        SUBROUTINE(6, QD_mtr)

        /* side count check */
        cmjwx_mz(Mecx, ctx_XMISC(FLG),
                 EQ_x, OO_end)

        /* overdraw check */
        cmjwx_mz(Mecx, ctx_XMISC(TAG),
                 NE_x, QD_rs1)

        /* optimize overdraw */
        movpx_ld(Xmm7, Mecx, ctx_TMASK(0))      /* tmask <- TMASK */
        xorpx_ld(Xmm7, Mecx, ctx_XMASK)         /* tmask ^= XMASK */
        CHECK_MASK(OO_end, NONE, Xmm7)

        jmpxx_lb(QD_rs1)

    LBL(QD_mtr)

#if RT_SHOW_BOUND

        cmjwx_mi(Mebx, srf_SRF_T(TAG), IB(RT_TAG_SURFACE_MAX),
                 EQ_x, QD_mat)

#endif /* RT_SHOW_BOUND */

        movwx_ld(Reax, Mebx, srf_SRF_T(SRF))    /* material redirect */

        cmjwx_ri(Reax, IB(1),
                 EQ_x, PL_mat)
        cmjwx_ri(Reax, IB(2),
                 EQ_x, QD_mat)
        cmjwx_ri(Reax, IB(3),
                 EQ_x, TP_mat)

/******************************************************************************/
    LBL(QD_mat)

        FETCH_PROP()                            /* Xmm7  <- tside */

#if RT_FEAT_LIGHTS_SHADOWS

        CHECK_SHAD(QD_shd)

#endif /* RT_FEAT_LIGHTS_SHADOWS */

#if RT_FEAT_NORMALS

        /* compute normal, if enabled */
        CHECK_PROP(QD_nrm, RT_PROP_NORMAL)

        movwx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iecx */

        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        movpx_ld(Xmm4, Iecx, ctx_NEW_X)         /* loc_x <- NEW_X */
        movpx_ld(Xmm5, Iecx, ctx_NEW_Y)         /* loc_y <- NEW_Y */
        movpx_ld(Xmm6, Iecx, ctx_NEW_Z)         /* loc_z <- NEW_Z */

        mulps_ld(Xmm4, Mebx, srf_SCI_X)         /* loc_x *= SCI_X */
        mulps_ld(Xmm5, Mebx, srf_SCI_Y)         /* loc_y *= SCI_Y */
        mulps_ld(Xmm6, Mebx, srf_SCI_Z)         /* loc_z *= SCI_Z */

        subps_ld(Xmm4, Mebx, srf_SCJ_X)         /* loc_x -= SCJ_X */
        subps_ld(Xmm5, Mebx, srf_SCJ_Y)         /* loc_y -= SCJ_Y */
        subps_ld(Xmm6, Mebx, srf_SCJ_Z)         /* loc_z -= SCJ_Z */

        /* normalize normal */
        movpx_rr(Xmm1, Xmm4)                    /* loc_x <- loc_x */
        movpx_rr(Xmm2, Xmm5)                    /* loc_y <- loc_y */
        movpx_rr(Xmm3, Xmm6)                    /* loc_z <- loc_z */

        mulps_rr(Xmm1, Xmm4)                    /* loc_x *= loc_x */
        mulps_rr(Xmm2, Xmm5)                    /* loc_y *= loc_y */
        mulps_rr(Xmm3, Xmm6)                    /* loc_z *= loc_z */

        addps_rr(Xmm1, Xmm2)                    /* lc2_x += lc2_y */
        addps_rr(Xmm1, Xmm3)                    /* lc2_t += lc2_z */
        rsqps_rr(Xmm0, Xmm1) /* destroys Xmm1 *//* inv_r rs loc_r */
        xorpx_rr(Xmm0, Xmm7)                    /* inv_r ^= tside */

        mulps_rr(Xmm4, Xmm0)                    /* loc_x *= inv_r */
        mulps_rr(Xmm5, Xmm0)                    /* loc_y *= inv_r */
        mulps_rr(Xmm6, Xmm0)                    /* loc_z *= inv_r */

        /* store normal */
        movpx_st(Xmm4, Iecx, ctx_NRM_X)         /* loc_x -> NRM_X */
        movpx_st(Xmm5, Iecx, ctx_NRM_Y)         /* loc_y -> NRM_Y */
        movpx_st(Xmm6, Iecx, ctx_NRM_Z)         /* loc_z -> NRM_Z */

        jmpxx_lb(MT_nrm)

    LBL(QD_nrm)

#endif /* RT_FEAT_NORMALS */

        jmpxx_lb(MT_mat)

/******************************************************************************/
#if RT_FEAT_CLIPPING_CUSTOM

    LBL(QD_clp)

        movwx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iecx */

        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        movpx_ld(Xmm4, Iecx, ctx_NRM_X)         /* dff_x <- DFF_X */
        movpx_ld(Xmm1, Mebx, srf_SCJ_X)         /* df1_x <- SCJ_X */
        addps_rr(Xmm1, Xmm1)                    /* df1_x += df1_x */
        mulps_rr(Xmm1, Xmm4)                    /* df1_x *= dff_x */
        mulps_rr(Xmm4, Xmm4)                    /* df2_x *= dff_x */
        mulps_ld(Xmm4, Mebx, srf_SCI_X)         /* df2_x *= SCI_X */
        subps_rr(Xmm4, Xmm1)                    /* df2_x -= df1_x */

        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        movpx_ld(Xmm5, Iecx, ctx_NRM_Y)         /* dff_y <- DFF_Y */
        movpx_ld(Xmm2, Mebx, srf_SCJ_Y)         /* df1_y <- SCJ_Y */
        addps_rr(Xmm2, Xmm2)                    /* df1_y += df1_y */
        mulps_rr(Xmm2, Xmm5)                    /* df1_y *= dff_y */
        mulps_rr(Xmm5, Xmm5)                    /* df2_y *= dff_y */
        mulps_ld(Xmm5, Mebx, srf_SCI_Y)         /* df2_y *= SCI_Y */
        subps_rr(Xmm5, Xmm2)                    /* df2_y -= df1_y */

        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        movpx_ld(Xmm6, Iecx, ctx_NRM_Z)         /* dff_z <- DFF_Z */
        movpx_ld(Xmm3, Mebx, srf_SCJ_Z)         /* df1_z <- SCJ_Z */
        addps_rr(Xmm3, Xmm3)                    /* df1_z += df1_z */
        mulps_rr(Xmm3, Xmm6)                    /* df1_z *= dff_z */
        mulps_rr(Xmm6, Xmm6)                    /* df2_z *= dff_z */
        mulps_ld(Xmm6, Mebx, srf_SCI_Z)         /* df2_z *= SCI_Z */
        subps_rr(Xmm6, Xmm3)                    /* df2_z -= df1_z */

        subps_ld(Xmm4, Mebx, srf_SCI_W)         /* dff_x -= SCI_W */
        addps_rr(Xmm4, Xmm5)                    /* dff_x += dff_y */
        addps_rr(Xmm4, Xmm6)                    /* dff_x += dff_z */
        xorpx_rr(Xmm0, Xmm0)                    /* tmp_v <-     0 */

        APPLY_CLIP(QD, Xmm4, Xmm0)

        jmpxx_lb(CC_ret)

#endif /* RT_FEAT_CLIPPING_CUSTOM */

/******************************************************************************/
/********************************   OBJ DONE   ********************************/
/******************************************************************************/

    LBL(OO_end)

        movxx_ld(Resi, Mesi, elm_NEXT)
        jmpxx_lb(OO_cyc)

    LBL(OO_out)

        movwx_ld(Reax, Mecx, ctx_PARAM(PTR))

        cmjwx_ri(Reax, IB(0),
                 EQ_x, XX_end)
        cmjwx_ri(Reax, IB(4),
                 EQ_x, PT_ret)
        cmjwx_ri(Reax, IB(1),
                 EQ_x, LT_ret)
        cmjwx_ri(Reax, IB(2),
                 EQ_x, RF_ret)
        cmjwx_ri(Reax, IB(3),
                 EQ_x, TR_ret)

/******************************************************************************/
/********************************   HOR SCAN   ********************************/
/******************************************************************************/

    LBL(XX_end)

        /* introduce an intermediate fp32
         * color-buffer here to implement
         * tasks H,D and other 2D effects */

        /* implement precision-converters
         * to fp32 from fp64, fp16, fp128
         * to enable limited SPMD-targets
         * like fp16/fp128 in render core */

        /* accumulate path-tracer samples */
        cmjxx_mz(Mebp, inf_PT_ON,
                 EQ_x, FF_clm)

        movxx_ld(Reax, Mebp, inf_FRM_Y)
        mulxx_ld(Reax, Mebp, inf_FRM_ROW)
        addxx_ld(Reax, Mebp, inf_FRM_X)
        shlxx_ri(Reax, IB(L+1))

        movxx_ld(Rebx, Mebp, inf_PTR_R)
        movpx_ld(Xmm0, Mecx, ctx_COL_R(0))
        mulps_ld(Xmm0, Mebp, inf_PTS_O)
        movpx_ld(Xmm1, Iebx, DP(0))
        mulps_ld(Xmm1, Mebp, inf_PTS_U)
        addps_rr(Xmm0, Xmm1)
        movpx_st(Xmm0, Iebx, DP(0))
        movpx_st(Xmm0, Mecx, ctx_COL_R(0))

        movxx_ld(Rebx, Mebp, inf_PTR_G)
        movpx_ld(Xmm0, Mecx, ctx_COL_G(0))
        mulps_ld(Xmm0, Mebp, inf_PTS_O)
        movpx_ld(Xmm1, Iebx, DP(0))
        mulps_ld(Xmm1, Mebp, inf_PTS_U)
        addps_rr(Xmm0, Xmm1)
        movpx_st(Xmm0, Iebx, DP(0))
        movpx_st(Xmm0, Mecx, ctx_COL_G(0))

        movxx_ld(Rebx, Mebp, inf_PTR_B)
        movpx_ld(Xmm0, Mecx, ctx_COL_B(0))
        mulps_ld(Xmm0, Mebp, inf_PTS_O)
        movpx_ld(Xmm1, Iebx, DP(0))
        mulps_ld(Xmm1, Mebp, inf_PTS_U)
        addps_rr(Xmm0, Xmm1)
        movpx_st(Xmm0, Iebx, DP(0))
        movpx_st(Xmm0, Mecx, ctx_COL_B(0))

    LBL(FF_clm)

        /* clamp fp colors to 1.0 limit */
        movpx_ld(Xmm1, Mebp, inf_GPC01)

        movpx_ld(Xmm0, Mecx, ctx_COL_R(0))
        minps_rr(Xmm0, Xmm1)
        movpx_st(Xmm0, Mecx, ctx_COL_R(0))

        movpx_ld(Xmm0, Mecx, ctx_COL_G(0))
        minps_rr(Xmm0, Xmm1)
        movpx_st(Xmm0, Mecx, ctx_COL_G(0))

        movpx_ld(Xmm0, Mecx, ctx_COL_B(0))
        minps_rr(Xmm0, Xmm1)
        movpx_st(Xmm0, Mecx, ctx_COL_B(0))

        movxx_ri(Resi, IM(RT_SIMD_QUADS*16))

#if RT_FEAT_ANTIALIASING

        movxx_ld(Rebx, Mebp, inf_FSAA)

        cmjxx_rz(Rebx,
                 EQ_x, AA_out)

    LBL(AA_cyc)

        /* half fp colors for each pass */
        movpx_ld(Xmm1, Mebp, inf_GPC02)
        andpx_ld(Xmm1, Mebp, inf_GPC04)

        movpx_ld(Xmm0, Mecx, ctx_COL_R(0))
        mulps_rr(Xmm0, Xmm1)
        movpx_st(Xmm0, Mecx, ctx_COL_R(0))

        movpx_ld(Xmm0, Mecx, ctx_COL_G(0))
        mulps_rr(Xmm0, Xmm1)
        movpx_st(Xmm0, Mecx, ctx_COL_G(0))

        movpx_ld(Xmm0, Mecx, ctx_COL_B(0))
        mulps_rr(Xmm0, Xmm1)
        movpx_st(Xmm0, Mecx, ctx_COL_B(0))

        /* select code-paths from quads */
        xorxx_rr(Reax, Reax)
        movxx_rr(Redx, Resi)

        cmjxx_ri(Redx, IB(16),
                 GT_x, AA_qds)

        xorlx_rr(Xmm0, Xmm0)

        movlx_ld(Xmm1, Mecx, ctx_COL_R(0x00))
        adpls_rr(Xmm1, Xmm0)
        movlx_ld(Xmm2, Mecx, ctx_COL_G(0x00))
        adpls_rr(Xmm2, Xmm0)
        movlx_ld(Xmm3, Mecx, ctx_COL_B(0x00))
        adpls_rr(Xmm3, Xmm0)

        movlx_st(Xmm1, Mecx, ctx_COL_R(0x00))
        movlx_st(Xmm2, Mecx, ctx_COL_G(0x00))
        movlx_st(Xmm3, Mecx, ctx_COL_B(0x00))

        jmpxx_lb(AA_end)

    LBL(AA_qds)

        movlx_ld(Xmm1, Iecx, ctx_COL_R(0x00))
        adpls_ld(Xmm1, Iecx, ctx_COL_R(0x10))
        movlx_ld(Xmm2, Iecx, ctx_COL_G(0x00))
        adpls_ld(Xmm2, Iecx, ctx_COL_G(0x10))
        movlx_ld(Xmm3, Iecx, ctx_COL_B(0x00))
        adpls_ld(Xmm3, Iecx, ctx_COL_B(0x10))

        shrxx_ri(Reax, IB(1))
        movlx_st(Xmm1, Iecx, ctx_COL_R(0x00))
        movlx_st(Xmm2, Iecx, ctx_COL_G(0x00))
        movlx_st(Xmm3, Iecx, ctx_COL_B(0x00))
        shlxx_ri(Reax, IB(1))

        addxx_ri(Reax, IB(32))
        subxx_ri(Redx, IB(32))

        cmjxx_rz(Redx,
                 NE_x, AA_qds)

    LBL(AA_end)

        subxx_ri(Rebx, IB(1))
        shrxx_ri(Resi, IB(1))

        cmjxx_rz(Rebx,
                 NE_x, AA_cyc)

    LBL(AA_out)

#endif /* RT_FEAT_ANTIALIASING */

        /* convert fp colors to integer */
        movxx_ld(Redx, Mebp, inf_CAM)           /* edx needed in FRAME_SIMD */

        movxx_ld(Reax, Mecx, ctx_PARAM(FLG))    /* load Gamma prop */
        movxx_st(Reax, Mecx, ctx_LOCAL(FLG))    /* save Gamma prop */

        FRAME_SIMD()

        movxx_ld(Rebx, Mebp, inf_FRM_X)
        shlxx_ri(Rebx, IB(2))
        addxx_ld(Rebx, Mebp, inf_FRM)

        xorxx_rr(Reax, Reax)

    LBL(FF_cyc)

        shlxx_ri(Reax, IB(L-1))
        movyx_ld(Redx, Iecx, ctx_C_BUF(0))
        shrxx_ri(Reax, IB(L-1))

        movwx_st(Redx, Iebx, DP(0))

        subxx_ri(Resi, IB(4*L))
        addxx_ri(Reax, IB(4))

        cmjxx_rz(Resi,
                 NE_x, FF_cyc)

        shlxx_ri(Reax, IB(L-1))
        addxx_st(Reax, Mebp, inf_PRNGS)
        shrxx_ri(Reax, IB(L+1))
        addxx_st(Reax, Mebp, inf_FRM_X)

        movxx_ld(Reax, Mebp, inf_FRM_X)
        cmjxx_rm(Reax, Mebp, inf_FRM_W,
                 GE_x, YY_end)

        /* advance primary rays horizontally */
        movxx_ld(Redx, Mebp, inf_CAM)

        movpx_ld(Xmm0, Mebp, inf_HOR_I)         /* hor_i <- HOR_I */
        addps_ld(Xmm0, Medx, cam_HOR_U)         /* hor_i += HOR_U */
        movpx_st(Xmm0, Mebp, inf_HOR_I)         /* hor_i -> HOR_I */

#if RT_FEAT_TILING

        movxx_ld(Reax, Mebp, inf_FRM_X)
        prexx_xx()
        divxx_xm(Mebp, inf_TILE_W)
        movxx_st(Reax, Mebp, inf_TLS_X)

#endif /* RT_FEAT_TILING */

        jmpxx_lb(XX_cyc)

/******************************************************************************/
/********************************   VER SCAN   ********************************/
/******************************************************************************/

    LBL(YY_end)

        /* advance primary rays vertically */
        movxx_ld(Redx, Mebp, inf_CAM)

        movpx_ld(Xmm0, Mebp, inf_VER_I)         /* ver_i <- VER_I */
        addps_ld(Xmm0, Medx, cam_VER_U)         /* ver_i += VER_U */
        movpx_st(Xmm0, Mebp, inf_VER_I)         /* ver_i -> VER_I */

        movpx_ld(Xmm0, Mebp, inf_HOR_C)         /* hor_i <- HOR_C */
        movpx_st(Xmm0, Mebp, inf_HOR_I)         /* hor_i -> HOR_I */

#if RT_FEAT_MULTITHREADING

        movxx_ld(Reax, Mebp, inf_THNUM)
        addxx_st(Reax, Mebp, inf_FRM_Y)

#else /* RT_FEAT_MULTITHREADING */

        addxx_mi(Mebp, inf_FRM_Y, IB(1))

#endif /* RT_FEAT_MULTITHREADING */

        jmpxx_lb(YY_cyc)

    LBL(YY_out)

    ASM_LEAVE(s_inf)

/******************************************************************************/
/**********************************   LEAVE   *********************************/
/******************************************************************************/

#endif /* RT_RENDER_CODE */
}

/*
 * Fresnel code was inspired by 2006--degreve--reflection_refraction.pdf paper.
 * Almost identical code is used for calculations in render0 routine above.
 */
rt_void plot_fresnel(rt_SIMD_INFOP *s_inf)
{
#if RT_PLOT_FUNCS_REF

    rt_si32 i;

    for (i = 0; i < 4; i++)
    {
        float n = s_inf->c_rfr[i];
        float cosI = -s_inf->i_cos[i];
        float sinT2 = n * n * (1 - cosI * cosI);

        if (sinT2 > 1)
        {
            s_inf->o_rfl[i] = 1;
            continue;
        }

        float cosT = sqrt(1 - sinT2);
        float rOrth = (n * cosI - cosT) / (n * cosI + cosT);
        float rPar = (cosI - n * cosT) / (cosI + n * cosT);

        s_inf->o_rfl[i] = (rOrth * rOrth + rPar * rPar) * 0.5;
    }

#else /* RT_PLOT_FUNCS_REF */

    ASM_ENTER(s_inf)

        movix_ld(Xmm0, Mebp, inf_I_COS)
        movix_rr(Xmm4, Xmm0)

        movix_ld(Xmm6, Mebp, inf_C_RFR)
        mulis_rr(Xmm0, Xmm6)
        movix_rr(Xmm7, Xmm0)
        mulis_rr(Xmm7, Xmm7)
        addis_ld(Xmm7, Mebp, inf_GPC01_32)
        subis_ld(Xmm7, Mebp, inf_RFR_2)

        /* check total inner reflection */
        xorix_rr(Xmm5, Xmm5)
        cleis_rr(Xmm5, Xmm7)

        mkjix_rx(Xmm5, NONE, FR_tir)

        movix_st(Xmm5, Mebp, inf_T_NEW)
        jmpxx_lb(FR_cnt)

    LBL(FR_tir)

        movix_ld(Xmm5, Mebp, inf_GPC01_32)
        movix_st(Xmm5, Mebp, inf_O_RFL)
        jmpxx_lb(FR_end)

    LBL(FR_cnt)

        sqris_rr(Xmm7, Xmm7)
        addis_rr(Xmm0, Xmm7)

        /* compute Fresnel */
        movix_rr(Xmm1, Xmm4)
        movix_rr(Xmm2, Xmm1)
        mulis_rr(Xmm2, Xmm6)
        subis_rr(Xmm2, Xmm7)
        mulis_rr(Xmm7, Xmm6)
        movix_rr(Xmm3, Xmm1)
        addis_rr(Xmm1, Xmm7)
        subis_rr(Xmm3, Xmm7)
        divis_rr(Xmm0, Xmm2)
        divis_rr(Xmm1, Xmm3)
        mulis_rr(Xmm0, Xmm0)
        mulis_rr(Xmm1, Xmm1)
        addis_rr(Xmm0, Xmm1)
        mulis_ld(Xmm0, Mebp, inf_GPC02_32)
        andix_ld(Xmm0, Mebp, inf_GPC04_32)

        /* store Fresnel reflectance */
        movix_ld(Xmm5, Mebp, inf_T_NEW)
        andix_rr(Xmm0, Xmm5)
        movix_ld(Xmm4, Mebp, inf_GPC01_32)
        mulis_rr(Xmm0, Xmm4)
        annix_rr(Xmm5, Xmm4)
        orrix_rr(Xmm0, Xmm5)
        movix_st(Xmm0, Mebp, inf_O_RFL)

    LBL(FR_end)

    ASM_LEAVE(s_inf)

#endif /* RT_PLOT_FUNCS_REF */
}

/*
 * Schlick code was inspired by 2006--degreve--reflection_refraction.pdf paper.
 * Almost identical code is used for calculations in render0 routine above.
 */
rt_void plot_schlick(rt_SIMD_INFOP *s_inf)
{
#if RT_PLOT_FUNCS_REF

    rt_si32 i;

    for (i = 0; i < 4; i++)
    {
        float n = s_inf->c_rfr[i];
        float r0 = (n - 1) / (n + 1);
        r0 *= r0;
        float cosX = -s_inf->i_cos[i];

        if (n > 1)
        {
            float sinT2 = n * n * (1 - cosX * cosX);
            if (sinT2 > 1)
            {
                s_inf->o_rfl[i] = 1;
                continue;
            }
            cosX = sqrt(1 - sinT2);
        }

        float x = 1 - cosX;
        s_inf->o_rfl[i] = r0 + (1 - r0) * x * x * x * x * x;
    }

#else /* RT_PLOT_FUNCS_REF */

    ASM_ENTER(s_inf)

        movix_ld(Xmm0, Mebp, inf_I_COS)
        movix_rr(Xmm4, Xmm0)

        movix_ld(Xmm6, Mebp, inf_C_RFR)
        mulis_rr(Xmm0, Xmm6)
        movix_rr(Xmm7, Xmm0)
        mulis_rr(Xmm7, Xmm7)
        addis_ld(Xmm7, Mebp, inf_GPC01_32)
        subis_ld(Xmm7, Mebp, inf_RFR_2)

        /* check total inner reflection */
        xorix_rr(Xmm5, Xmm5)
        cleis_rr(Xmm5, Xmm7)

        mkjix_rx(Xmm5, NONE, SC_tir)

        movix_st(Xmm5, Mebp, inf_T_NEW)
        jmpxx_lb(SC_cnt)

    LBL(SC_tir)

        movix_ld(Xmm5, Mebp, inf_GPC01_32)
        movix_st(Xmm5, Mebp, inf_O_RFL)
        jmpxx_lb(SC_end)

    LBL(SC_cnt)

        sqris_rr(Xmm7, Xmm7)
        addis_rr(Xmm0, Xmm7)

        /* compute Schlick */
        movix_ld(Xmm1, Mebp, inf_GPC01_32)
        movix_rr(Xmm5, Xmm6)
        cgtis_rr(Xmm5, Xmm1)

        mkjix_rx(Xmm5, NONE, SC_inv)

        xorix_rr(Xmm4, Xmm4)
        subis_rr(Xmm4, Xmm7)

    LBL(SC_inv)

        addis_rr(Xmm4, Xmm1)
        movix_rr(Xmm0, Xmm6)
        subis_rr(Xmm0, Xmm1)
        addis_rr(Xmm6, Xmm1)
        divis_rr(Xmm0, Xmm6)
        mulis_rr(Xmm0, Xmm0)
        subis_rr(Xmm1, Xmm0)
        movix_rr(Xmm5, Xmm4)
        mulis_rr(Xmm4, Xmm4)
        mulis_rr(Xmm4, Xmm4)
        mulis_rr(Xmm5, Xmm4)
        mulis_rr(Xmm1, Xmm5)
        addis_rr(Xmm0, Xmm1)

        /* store Fresnel reflectance */
        movix_ld(Xmm5, Mebp, inf_T_NEW)
        andix_rr(Xmm0, Xmm5)
        movix_ld(Xmm4, Mebp, inf_GPC01_32)
        mulis_rr(Xmm0, Xmm4)
        annix_rr(Xmm5, Xmm4)
        orrix_rr(Xmm0, Xmm5)
        movix_st(Xmm0, Mebp, inf_O_RFL)

    LBL(SC_end)

    ASM_LEAVE(s_inf)

#endif /* RT_PLOT_FUNCS_REF */
}

/*
 * Fresnel metal code was inspired by memo-on-fresnel-equations by S. Lagarde.
 * Almost identical code is used for calculations in render0 routine above.
 */
rt_void plot_fresnel_metal_fast(rt_SIMD_INFOP *s_inf)
{
#if RT_PLOT_FUNCS_REF

    rt_si32 i;

    for (i = 0; i < 4; i++)
    {
        float CosTheta2 = s_inf->i_cos[i] * s_inf->i_cos[i];
        float TwoEtaCosTheta = 2 * s_inf->c_rcp[i] * -s_inf->i_cos[i];

        float t0 = s_inf->c_rcp[i] * s_inf->c_rcp[i] + s_inf->ext_2[i];
        float t1 = t0 * CosTheta2;
        float Rs = (t0 - TwoEtaCosTheta + CosTheta2) /
                   (t0 + TwoEtaCosTheta + CosTheta2);
        float Rp = (t1 - TwoEtaCosTheta + 1) / (t1 + TwoEtaCosTheta + 1);

        s_inf->o_rfl[i] = 0.5 * (Rp + Rs);
    }

#else /* RT_PLOT_FUNCS_REF */

    ASM_ENTER(s_inf)

        movix_ld(Xmm0, Mebp, inf_I_COS)
        movix_ld(Xmm6, Mebp, inf_C_RCP)
        movix_rr(Xmm4, Xmm0)
        mulis_rr(Xmm4, Xmm6)
        addis_rr(Xmm4, Xmm4)
        mulis_rr(Xmm0, Xmm0)
        mulis_rr(Xmm6, Xmm6)
        addis_ld(Xmm6, Mebp, inf_EXT_2)
        movix_rr(Xmm1, Xmm0)
        mulis_rr(Xmm1, Xmm6)
        addis_rr(Xmm0, Xmm6)
        addis_ld(Xmm1, Mebp, inf_GPC01_32)
        movix_rr(Xmm2, Xmm0)
        movix_rr(Xmm3, Xmm1)
        addis_rr(Xmm0, Xmm4)
        addis_rr(Xmm1, Xmm4)
        subis_rr(Xmm2, Xmm4)
        subis_rr(Xmm3, Xmm4)
        divis_rr(Xmm0, Xmm2)
        divis_rr(Xmm1, Xmm3)
        addis_rr(Xmm0, Xmm1)
        mulis_ld(Xmm0, Mebp, inf_GPC02_32)
        andix_ld(Xmm0, Mebp, inf_GPC04_32)
        movix_st(Xmm0, Mebp, inf_O_RFL)

    ASM_LEAVE(s_inf)

#endif /* RT_PLOT_FUNCS_REF */
}

/*
 * Fresnel metal code was inspired by memo-on-fresnel-equations by S. Lagarde.
 * Almost identical code is used for calculations in render0 routine above.
 */
rt_void plot_fresnel_metal_slow(rt_SIMD_INFOP *s_inf)
{
#if RT_PLOT_FUNCS_REF

    rt_si32 i;

    for (i = 0; i < 4; i++)
    {
        float CosTheta = -s_inf->i_cos[i];
        float Eta2 = s_inf->c_rcp[i] * s_inf->c_rcp[i];
        float Etak2 = s_inf->ext_2[i];

        float CosTheta2 = CosTheta * CosTheta;
        float SinTheta2 = 1 - CosTheta2;

        float t0 = Eta2 - Etak2 - SinTheta2;
        float a2plusb2 = sqrt(t0 * t0 + 4 * Eta2 * Etak2);
        float t1 = a2plusb2 + CosTheta2;
        float a = sqrt(0.5f * (a2plusb2 + t0));
        float t2 = 2 * a * CosTheta;
        float Rs = (t1 - t2) / (t1 + t2);

        float t3 = CosTheta2 * a2plusb2 + SinTheta2 * SinTheta2;
        float t4 = t2 * SinTheta2;
        float Rp = Rs * (t3 - t4) / (t3 + t4);

        s_inf->o_rfl[i] = 0.5 * (Rp + Rs);
    }

#else /* RT_PLOT_FUNCS_REF */

    ASM_ENTER(s_inf)

        movix_ld(Xmm0, Mebp, inf_I_COS)
        movix_rr(Xmm4, Xmm0)
        mulis_rr(Xmm0, Xmm0)
        movix_ld(Xmm1, Mebp, inf_GPC01_32)
        subis_rr(Xmm1, Xmm0)
        movix_ld(Xmm6, Mebp, inf_C_RCP)
        movix_ld(Xmm5, Mebp, inf_EXT_2)
        mulis_rr(Xmm6, Xmm6)
        movix_rr(Xmm7, Xmm6)
        subis_rr(Xmm6, Xmm5)
        subis_rr(Xmm6, Xmm1)
        mulis_rr(Xmm5, Xmm7)
        movix_ld(Xmm7, Mebp, inf_GPC01_32)
        addis_ld(Xmm7, Mebp, inf_GPC03_32)
        mulis_rr(Xmm5, Xmm7)
        movix_rr(Xmm7, Xmm6)
        mulis_rr(Xmm7, Xmm7)
        addis_rr(Xmm7, Xmm5)
        sqris_rr(Xmm7, Xmm7)
        addis_rr(Xmm6, Xmm7)
        mulis_ld(Xmm6, Mebp, inf_GPC02_32)
        andix_ld(Xmm6, Mebp, inf_GPC04_32)
        sqris_rr(Xmm6, Xmm6)
        mulis_rr(Xmm6, Xmm4)
        addis_rr(Xmm6, Xmm6)
        movix_rr(Xmm2, Xmm0)
        addis_rr(Xmm0, Xmm7)
        mulis_rr(Xmm2, Xmm7)
        movix_rr(Xmm3, Xmm6)
        mulis_rr(Xmm3, Xmm1)
        mulis_rr(Xmm1, Xmm1)
        addis_rr(Xmm2, Xmm1)
        movix_rr(Xmm1, Xmm0)
        addis_rr(Xmm0, Xmm6)
        subis_rr(Xmm1, Xmm6)
        divis_rr(Xmm0, Xmm1)
        movix_rr(Xmm1, Xmm2)
        addis_rr(Xmm2, Xmm3)
        subis_rr(Xmm1, Xmm3)
        divis_rr(Xmm2, Xmm1)
        mulis_rr(Xmm2, Xmm0)
        addis_rr(Xmm0, Xmm2)
        mulis_ld(Xmm0, Mebp, inf_GPC02_32)
        andix_ld(Xmm0, Mebp, inf_GPC04_32)
        movix_st(Xmm0, Mebp, inf_O_RFL)

    ASM_LEAVE(s_inf)

#endif /* RT_PLOT_FUNCS_REF */
}

#else /* RT_SIMD_CODE */

#include <string.h>

#include "tracer.h"
#include "format.h"
#include "engine.h"

/******************************************************************************/
/*********************************   UPDATE   *********************************/
/******************************************************************************/

/*
 * Backend's global entry point (hence 0).
 * Update surface's backend-specific fields
 * from its internal state.
 */
rt_void rt_Platform::update0(rt_SIMD_SURFACE *s_srf)
{
    rt_ui32 tag = (rt_ui32)(rt_word)s_srf->srf_t[3];

    if (tag >= RT_TAG_SURFACE_MAX)
    {
        return;
    }

    /* set surface's tags */
    s_srf->srf_t[0] = tag > RT_TAG_PLANE ?
                     (tag == RT_TAG_HYPERCYLINDER &&
                      s_srf->sci_w[0] == 0.0f) ?
                      3 : 2 : 1;

    s_srf->srf_t[1] = tag > RT_TAG_PLANE ?
                     (tag != RT_TAG_PARABOLOID &&
                      tag != RT_TAG_PARACYLINDER &&
                      tag != RT_TAG_HYPERPARABOLOID) ?
                      3 : 2 : 1;

    s_srf->srf_t[2] = tag > RT_TAG_PLANE ?
                     (tag != RT_TAG_PARABOLOID &&
                      tag != RT_TAG_PARACYLINDER &&
                      tag != RT_TAG_HYPERPARABOLOID) ?
                      3 : 2 : 1;

    s_srf->msc_p[1] = tag == RT_TAG_CONE ||
                     (tag == RT_TAG_HYPERBOLOID &&
                      s_srf->sci_w[0] == 0.0f) ?
                      (rt_pntr)1 :
                     (tag == RT_TAG_HYPERCYLINDER &&
                      s_srf->sci_w[0] == 0.0f) ?
                      (rt_pntr)2 : (rt_pntr)0;
}

/******************************************************************************/
/*********************************   SWITCH   *********************************/
/******************************************************************************/

#if (RT_POINTER - RT_ADDRESS) != 0

#include "system.h"

#endif /* (RT_POINTER - RT_ADDRESS) */

/*
 * Backend's global entry point (hence 0).
 * Set current runtime SIMD target with "simd" equal to
 * SIMD native-size (1,..,16) in 0th (lowest) byte
 * SIMD type (1,2,4,8, 16,32) in 1st (higher) byte
 * SIMD size-factor (1, 2, 4) in 2nd (higher) byte
 */
rt_si32 rt_Platform::switch0(rt_SIMD_INFOX *s_inf, rt_si32 simd)
{
#if (RT_POINTER - RT_ADDRESS) != 0 && RT_DEBUG >= 2

    RT_LOGI("S_INF PTR = %016" PR_Z "X\n", (rt_full)s_inf);

#endif /* (RT_POINTER - RT_ADDRESS) && RT_DEBUG */

#if (RT_POINTER - RT_ADDRESS) != 0

    if ((rt_full)s_inf >= (rt_full)(0x80000000 - sizeof(rt_SIMD_INFOX)))
    {
        throw rt_Exception("address exceeded allowed range in switch0");
    }

#endif /* (RT_POINTER - RT_ADDRESS) */

    simd_version(s_inf);

    s_mask = s_inf->ver;

    s_mask &= (RT_128) | (RT_256_R8) << 4 | (RT_256) << 8 | (RT_512_R8) << 12 |
       (RT_512) << 16 | (RT_1K4_R8) << 20 | (RT_1K4) << 24 | (RT_2K8_R8) << 28;

    rt_si32 i, mask;

    mask = mask_init(simd);

    mask &= s_mask;

    for (i = 31; i >= 0; i--)
    {
        if (mask & (1 << i))
        {
            mask = (1 << i);
            break;
        }
    }

    if (i < 0)
    {
        mask = s_mode;
    }

    s_mode = mask;

    simd = from_mask(mask);

    return simd;
}

/******************************************************************************/
/*********************************   RENDER   *********************************/
/******************************************************************************/

namespace simd_128v1
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_128v2
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_128v4
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_128v8
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_256v4_r8
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_256v1
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_256v2
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_256v4
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_256v8
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_512v1_r8
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_512v2_r8
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_512v1
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_512v2
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_512v4
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_512v8
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_1K4v1
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_1K4v2
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_1K4v4
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_2K8v1_r8
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_2K8v2_r8
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

namespace simd_2K8v4_r8
{
rt_void render0(rt_SIMD_INFOX *s_inf);
}

/*
 * Backend's global entry point (hence 0).
 * Render frame based on the data structures
 * prepared by the engine.
 */
rt_void rt_Platform::render0(rt_SIMD_INFOX *s_inf)
{
    switch (s_mode)
    {
#if (RT_2K8_R8 & 4)
        case 0x40000000:
        simd_2K8v4_r8::render0(s_inf);
        break;
#endif /* RT_2K8_R8 & 4 */
#if (RT_2K8_R8 & 2)
        case 0x20000000:
        simd_2K8v2_r8::render0(s_inf);
        break;
#endif /* RT_2K8_R8 & 2 */
#if (RT_2K8_R8 & 1)
        case 0x10000000:
        simd_2K8v1_r8::render0(s_inf);
        break;
#endif /* RT_2K8_R8 & 1 */
#if (RT_1K4 & 4)
        case 0x04000000:
        simd_1K4v4::render0(s_inf);
        break;
#endif /* RT_1K4 & 4 */
#if (RT_1K4 & 2)
        case 0x02000000:
        simd_1K4v2::render0(s_inf);
        break;
#endif /* RT_1K4 & 2 */
#if (RT_1K4 & 1)
        case 0x01000000:
        simd_1K4v1::render0(s_inf);
        break;
#endif /* RT_1K4 & 1 */
#if (RT_512 & 8)
        case 0x00080000:
        simd_512v8::render0(s_inf);
        break;
#endif /* RT_512 & 8 */
#if (RT_512 & 4)
        case 0x00040000:
        simd_512v4::render0(s_inf);
        break;
#endif /* RT_512 & 4 */
#if (RT_512 & 2)
        case 0x00020000:
        simd_512v2::render0(s_inf);
        break;
#endif /* RT_512 & 2 */
#if (RT_512 & 1)
        case 0x00010000:
        simd_512v1::render0(s_inf);
        break;
#endif /* RT_512 & 1 */
#if (RT_512_R8 & 2)
        case 0x00002000:
        simd_512v2_r8::render0(s_inf);
        break;
#endif /* RT_512_R8 & 2 */
#if (RT_512_R8 & 1)
        case 0x00001000:
        simd_512v1_r8::render0(s_inf);
        break;
#endif /* RT_512_R8 & 1 */
#if (RT_256 & 8)
        case 0x00000800:
        simd_256v8::render0(s_inf);
        break;
#endif /* RT_256 & 8 */
#if (RT_256 & 4)
        case 0x00000400:
        simd_256v4::render0(s_inf);
        break;
#endif /* RT_256 & 4 */
#if (RT_256 & 2)
        case 0x00000200:
        simd_256v2::render0(s_inf);
        break;
#endif /* RT_256 & 2 */
#if (RT_256 & 1)
        case 0x00000100:
        simd_256v1::render0(s_inf);
        break;
#endif /* RT_256 & 1 */
#if (RT_256_R8 & 4)
        case 0x00000040:
        simd_256v4_r8::render0(s_inf);
        break;
#endif /* RT_256_R8 & 4 */
#if (RT_128 & 8)
        case 0x00000008:
        simd_128v8::render0(s_inf);
        break;
#endif /* RT_128 & 8 */
#if (RT_128 & 4)
        case 0x00000004:
        simd_128v4::render0(s_inf);
        break;
#endif /* RT_128 & 4 */
#if (RT_128 & 2)
        case 0x00000002:
        simd_128v2::render0(s_inf);
        break;
#endif /* RT_128 & 2 */
#if (RT_128 & 1)
        case 0x00000001:
        simd_128v1::render0(s_inf);
        break;
#endif /* RT_128 & 1 */

        default:
        break;
    }

    if (s_inf->ctx != RT_NULL)
    {
#if RT_QUAD_DEBUG == 1

        if (s_inf->q_dbg == 7)
        {
            RT_LOGE("---------------------------------------------");
            RT_LOGE("------------- quadric debug info ------------");
            RT_LOGE("--------------------- depth: %d --------------",
                                        s_inf->depth - s_inf->q_cnt);
            RT_LOGE("\n");
            RT_LOGE("\n");

            RT_LOGE("    WMASK = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->wmask[0], s_inf->wmask[1], s_inf->wmask[2], s_inf->wmask[3]);

            RT_LOGE("\n");

            RT_LOGE("    DFF_X = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->dff_x[0], s_inf->dff_x[1], s_inf->dff_x[2], s_inf->dff_x[3]);

            RT_LOGE("    DFF_Y = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->dff_y[0], s_inf->dff_y[1], s_inf->dff_y[2], s_inf->dff_y[3]);

            RT_LOGE("    DFF_Z = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->dff_z[0], s_inf->dff_z[1], s_inf->dff_z[2], s_inf->dff_z[3]);

            RT_LOGE("\n");

            RT_LOGE("    RAY_X = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->ray_x[0], s_inf->ray_x[1], s_inf->ray_x[2], s_inf->ray_x[3]);

            RT_LOGE("    RAY_Y = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->ray_y[0], s_inf->ray_y[1], s_inf->ray_y[2], s_inf->ray_y[3]);

            RT_LOGE("    RAY_Z = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->ray_z[0], s_inf->ray_z[1], s_inf->ray_z[2], s_inf->ray_z[3]);

            RT_LOGE("\n");

            RT_LOGE("    A_VAL = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->a_val[0], s_inf->a_val[1], s_inf->a_val[2], s_inf->a_val[3]);

            RT_LOGE("    B_VAL = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->b_val[0], s_inf->b_val[1], s_inf->b_val[2], s_inf->b_val[3]);

            RT_LOGE("    C_VAL = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->c_val[0], s_inf->c_val[1], s_inf->c_val[2], s_inf->c_val[3]);

            RT_LOGE("    D_VAL = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->d_val[0], s_inf->d_val[1], s_inf->d_val[2], s_inf->d_val[3]);

            RT_LOGE("\n");

            RT_LOGE("    DMASK = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->dmask[0], s_inf->dmask[1], s_inf->dmask[2], s_inf->dmask[3]);

            RT_LOGE("\n");

            RT_LOGE("    T1NMR = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->t1nmr[0], s_inf->t1nmr[1], s_inf->t1nmr[2], s_inf->t1nmr[3]);

            RT_LOGE("    T1DNM = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->t1dnm[0], s_inf->t1dnm[1], s_inf->t1dnm[2], s_inf->t1dnm[3]);

            RT_LOGE("    T2NMR = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->t2nmr[0], s_inf->t2nmr[1], s_inf->t2nmr[2], s_inf->t2nmr[3]);

            RT_LOGE("    T2DNM = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->t2dnm[0], s_inf->t2dnm[1], s_inf->t2dnm[2], s_inf->t2dnm[3]);

            RT_LOGE("\n");

            RT_LOGE("    T1VAL = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->t1val[0], s_inf->t1val[1], s_inf->t1val[2], s_inf->t1val[3]);

            RT_LOGE("    T2VAL = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->t2val[0], s_inf->t2val[1], s_inf->t2val[2], s_inf->t2val[3]);

            RT_LOGE("    T1SRT = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->t1srt[0], s_inf->t1srt[1], s_inf->t1srt[2], s_inf->t1srt[3]);

            RT_LOGE("    T2SRT = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->t2srt[0], s_inf->t2srt[1], s_inf->t2srt[2], s_inf->t2srt[3]);

            RT_LOGE("    T1MSK = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->t1msk[0], s_inf->t1msk[1], s_inf->t1msk[2], s_inf->t1msk[3]);

            RT_LOGE("    T2MSK = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->t2msk[0], s_inf->t2msk[1], s_inf->t2msk[2], s_inf->t2msk[3]);

            RT_LOGE("\n");

            RT_LOGE("    TSIDE = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->tside[0], s_inf->tside[1], s_inf->tside[2], s_inf->tside[3]);

            RT_LOGE("\n");

            RT_LOGE("    HIT_X = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->hit_x[0], s_inf->hit_x[1], s_inf->hit_x[2], s_inf->hit_x[3]);

            RT_LOGE("    HIT_Y = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->hit_y[0], s_inf->hit_y[1], s_inf->hit_y[2], s_inf->hit_y[3]);

            RT_LOGE("    HIT_Z = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->hit_z[0], s_inf->hit_z[1], s_inf->hit_z[2], s_inf->hit_z[3]);

            RT_LOGE("\n");

            RT_LOGE("    ADJ_X = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->adj_x[0], s_inf->adj_x[1], s_inf->adj_x[2], s_inf->adj_x[3]);

            RT_LOGE("    ADJ_Y = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->adj_y[0], s_inf->adj_y[1], s_inf->adj_y[2], s_inf->adj_y[3]);

            RT_LOGE("    ADJ_Z = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->adj_z[0], s_inf->adj_z[1], s_inf->adj_z[2], s_inf->adj_z[3]);

            RT_LOGE("\n");

            RT_LOGE("    NRM_X = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->nrm_x[0], s_inf->nrm_x[1], s_inf->nrm_x[2], s_inf->nrm_x[3]);

            RT_LOGE("    NRM_Y = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->nrm_y[0], s_inf->nrm_y[1], s_inf->nrm_y[2], s_inf->nrm_y[3]);

            RT_LOGE("    NRM_Z = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->nrm_z[0], s_inf->nrm_z[1], s_inf->nrm_z[2], s_inf->nrm_z[3]);

            RT_LOGE("\n");
        }

#endif /* RT_QUAD_DEBUG */
    }
}

#endif /* RT_SIMD_CODE */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
