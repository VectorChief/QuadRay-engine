/******************************************************************************/
/* Copyright (c) 2013-2015 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include "tracer.h"
#include "format.h"

#if RT_DEBUG == 1

#include "system.h"

#endif /* RT_DEBUG */

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
 *
 * At present, Intel SSE2 (32-bit x86 ISA) and ARM NEON (32-bit ARMv7 ISA)
 * are two primary targets, although wider SIMD, 64-bit addressing along with
 * more available registers, and other architectures can be supported by design.
 * Preliminary naming scheme for potential future targets as well as extended
 * core and SIMD register files can be found in core/config/rtarch.h file.
 */

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

/*
 * Conditional compilation flags
 * for respective segments of code.
 */
#define RT_SHOW_TILES               0

#define RT_FEAT_TILING              1
#define RT_FEAT_ANTIALIASING        1
#define RT_FEAT_MULTITHREADING      1
#define RT_FEAT_CLIPPING_MINMAX     1
#define RT_FEAT_CLIPPING_CUSTOM     1
#define RT_FEAT_CLIPPING_ACCUM      1
#define RT_FEAT_TEXTURING           1
#define RT_FEAT_NORMALS             1
#define RT_FEAT_LIGHTS              1
#define RT_FEAT_LIGHTS_COLORED      1
#define RT_FEAT_LIGHTS_AMBIENT      1
#define RT_FEAT_LIGHTS_SHADOWS      0
#define RT_FEAT_LIGHTS_DIFFUSE      1
#define RT_FEAT_LIGHTS_ATTENUATION  1
#define RT_FEAT_LIGHTS_SPECULAR     1
#define RT_FEAT_REFLECTIONS         1
#define RT_FEAT_TRANSPARENCY        1
#define RT_FEAT_REFRACTIONS         1
#define RT_FEAT_TRANSFORM           1
#define RT_FEAT_TRANSFORM_ARRAY     1
#define RT_FEAT_BOUND_VOL_ARRAY     1

/*
 * Byte-offsets within SIMD-field
 * for packed scalar fields.
 */
#define PTR   0x00 /* LOCAL, PARAM, MAT_P, SRF_P, XMISC */
#define LGT   0x00 /* LST_P */

#define FLG   0x04 /* LOCAL, PARAM, MAT_P, MSC_P, XMISC */
#define SRF   0x04 /* LST_P, SRF_P */

#define CLP   0x08 /* MSC_P, SRF_P */
#define LST   0x08 /* LOCAL, PARAM */

#define TAG   0x0C /* SRF_P, XMISC */
#define OBJ   0x0C /* LOCAL, PARAM, MSC_P */

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
#define SHOW_TILES(lb, cl) /* destroys Reax, Xmm0 */                        \
        movxx_ri(Reax, IW(cl))                                              \
        movxx_st(Reax, Mecx, ctx_C_BUF(0x00))                               \
        movxx_st(Reax, Mecx, ctx_C_BUF(0x04))                               \
        movxx_st(Reax, Mecx, ctx_C_BUF(0x08))                               \
        movxx_st(Reax, Mecx, ctx_C_BUF(0x0C))                               \
        PAINT_COLX(10, COL_R(0))                                            \
        PAINT_COLX(08, COL_G(0))                                            \
        PAINT_COLX(00, COL_B(0))

/*
 * Axis mapping.
 * Perform axis mapping when
 * transform is a multiple of 90 degree rotation.
 */
#define INDEX_AXIS(nx) /* destroys Reax, Xmm0 */                            \
        movxx_ld(Reax, Mebx, srf_A_SGN(nx*4))                               \
        movpx_ld(Xmm0, Iebx, srf_SBASE)                                     \
        movxx_ld(Reax, Mebx, srf_A_MAP(nx*4))

#define MOVXR_LD(RG, RM, DP) /* reads Xmm0 */                               \
        movpx_ld(W(RG), W(RM), W(DP))                                       \
        xorpx_rr(W(RG), Xmm0)

#define MOVXR_ST(RG, RM, DP) /* reads Xmm0 */                               \
        xorpx_rr(W(RG), Xmm0)                                               \
        movpx_st(W(RG), W(RM), W(DP))

#define MOVZR_ST(RG, RM, DP)                                                \
        xorpx_rr(W(RG), W(RG))                                              \
        movpx_st(W(RG), W(RM), W(DP))

#define INDEX_TMAP(nx) /* destroys Reax */                                  \
        movxx_ld(Reax, Medx, mat_T_MAP(nx*4))

/*
 * Axis clipping.
 * Check if axis clipping (minmax) is needed for given axis "nx",
 * jump to "lb" otherwise.
 */
#define CHECK_CLIP(lb, pl, nx)                                              \
        cmpxx_mi(Mebx, srf_##pl(nx*4), IB(0))                               \
        jeqxx_lb(lb)

/*
 * Custom clipping.
 * Apply custom clipping (by surface) to ray's hit point
 * based on the side of the clipping surface.
 */
#define APPLY_CLIP(lb, RG, RM) /* destroys Reax */                          \
        movxx_ld(Reax, Medi, elm_DATA)                                      \
        cmpxx_ri(Reax, IB(0))                                               \
        jltxn_lb(lb##_cs1)                      /* signed comparison */     \
        cleps_rr(W(RG), W(RM))                                              \
        jmpxx_lb(lb##_cs2)                                                  \
    LBL(lb##_cs1)                                                           \
        cgeps_rr(W(RG), W(RM))                                              \
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
        andxx_ri(Reax, IB(fl))                                              \
        cmpxx_ri(Reax, IB(0))                                               \
        jeqxx_lb(lb)

/*
 * Combine ray's attack side with ray's pass behavior (back or thru),
 * so that secondary ray emitted from the same surface doesn't
 * hit it from the wrong side due to computational inaccuracy,
 * jump to "lb" if ray doesn't originate from the same surface,
 * jump to "lo" if ray cannot possibly hit the same surface from side "sd"
 * under given circumstances (even though it would due to hit inaccuracy).
 */
#define CHECK_SIDE(lb, lo, sd) /* destroys Reax */                          \
        cmpxx_rm(Rebx, Mecx, ctx_PARAM(OBJ))                                \
        jnexx_lb(lb)                                                        \
        movxx_ld(Reax, Mecx, ctx_PARAM(FLG))                                \
        andxx_ri(Reax, IB(RT_FLAG_SIDE | RT_FLAG_PASS))                     \
        cmpxx_ri(Reax, IB(1 - sd))                                          \
        jeqxx_lb(lo)                                                        \
        cmpxx_ri(Reax, IB(2 + sd))                                          \
        jeqxx_lb(lo)                                                        \
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
        jmpxx_mm(Mecx, ctx_LOCAL(PTR))                                      \
    LBL(lb##_lgt)                                                           \
        CHECK_PROP(lb##_trn, RT_PROP_TRANSP)                                \
        jmpxx_mm(Mecx, ctx_LOCAL(PTR))                                      \
    LBL(lb##_trn)                                                           \
        movpx_ld(Xmm7, Mecx, ctx_C_BUF(0))                                  \
        orrpx_ld(Xmm7, Mecx, ctx_TMASK(0))                                  \
        movpx_st(Xmm7, Mecx, ctx_C_BUF(0))                                  \
        CHECK_MASK(OO_out, FULL, Xmm7)                                      \
        jmpxx_mm(Mecx, ctx_LOCAL(PTR))                                      \
    LBL(lb)

/*
 * Material properties.
 * Fetch properties from material into the context's local FLG field
 * based on the currently set SIDE flag, also load SIDE flag
 * as sign into Xmm7 for normals.
 */
#define FETCH_PROP() /* destroys Reax, Xmm7 */                              \
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))                                \
        mulxn_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_ld(Xmm7, Iebx, srf_SBASE)                                     \
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))                                \
        shlxx_ri(Reax, IB(3))                                               \
        movxx_ld(Reax, Iebx, srf_MAT_P(FLG))                                \
        orrxx_st(Reax, Mecx, ctx_LOCAL(FLG))

/*
 * Check if property "pr" previously
 * fetched from material is set in the context,
 * jump to "lb" otherwise.
 */
#define CHECK_PROP(lb, pr) /* destroys Reax */                              \
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))                                \
        andxx_ri(Reax, IH(pr))                                              \
        cmpxx_ri(Reax, IB(0))                                               \
        jeqxx_lb(lb)

/*
 * Fetch pointer into given register "RG" from surface's field "pl"
 * based on the currently set SIDE flag.
 */
#define FETCH_XPTR(RG, pl) /* destroys Reax */                              \
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))                                \
        andxx_ri(Reax, IB(RT_FLAG_SIDE))                                    \
        shlxx_ri(Reax, IB(3))                                               \
        movxx_ld(W(RG), Iebx, srf_##pl)

/*
 * Fetch pointer into given register "RG" from surface's field "pl"
 * based on the currently set inverted SIDE flag.
 */
#define FETCH_IPTR(RG, pl) /* destroys Reax */                              \
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))                                \
        notxx_rr(Reax)                                                      \
        andxx_ri(Reax, IB(RT_FLAG_SIDE))                                    \
        shlxx_ri(Reax, IB(3))                                               \
        movxx_ld(W(RG), Iebx, srf_##pl)

/*
 * Update only relevant fragments of a given
 * SIMD-field accumulating values over multiple passes
 * from the temporary SIMD-field in the context
 * based on the current SIMD-mask.
 */
#define STORE_FRAG(lb, pn, pl) /* destroys Reax */                          \
        cmpxx_mi(Mecx, ctx_TMASK(0x##pn), IB(0))                            \
        jeqxx_lb(lb##pn)                                                    \
        movxx_ld(Reax, Mecx, ctx_C_PTR(0x##pn))                             \
        movxx_st(Reax, Mecx, ctx_##pl(0x##pn))                              \
    LBL(lb##pn)

#define STORE_SIMD(lb, pl) /* destroys Reax */                              \
        STORE_FRAG(lb, 00, pl)                                              \
        STORE_FRAG(lb, 04, pl)                                              \
        STORE_FRAG(lb, 08, pl)                                              \
        STORE_FRAG(lb, 0C, pl)

/*
 * Update relevant fragments of the
 * color and depth SIMD-fields accumulating values
 * over multiple passes from the respective SIMD-fields
 * in the context based on the current SIMD-mask and
 * the current depth values. Also perform
 * pointer dereferencing for color fetching.
 */
#define PAINT_FRAG(lb, pn) /* destroys Reax */                              \
        cmpxx_mi(Mecx, ctx_TMASK(0x##pn), IB(0))                            \
        jeqxx_lb(lb##pn)                                                    \
        movxx_ld(Reax, Mecx, ctx_T_VAL(0x##pn))                             \
        movxx_st(Reax, Mecx, ctx_T_BUF(0x##pn))                             \
        movxx_ld(Reax, Mecx, ctx_C_PTR(0x##pn))                             \
        movxx_ld(Reax, Oeax, PLAIN)                                         \
        movxx_st(Reax, Mecx, ctx_C_BUF(0x##pn))                             \
    LBL(lb##pn)

#define PAINT_COLX(cl, pl) /* destroys Xmm0 */                              \
        movpx_ld(Xmm0, Mecx, ctx_C_BUF(0))                                  \
        shrpx_ri(Xmm0, IB(0x##cl))                                          \
        andpx_rr(Xmm0, Xmm7)                                                \
        cvtpn_rr(Xmm0, Xmm0)                                                \
        movpx_st(Xmm0, Mecx, ctx_##pl)

#define PAINT_SIMD(lb) /* destroys Reax, Xmm0, Xmm7 */                      \
        PAINT_FRAG(lb, 00)                                                  \
        PAINT_FRAG(lb, 04)                                                  \
        PAINT_FRAG(lb, 08)                                                  \
        PAINT_FRAG(lb, 0C)                                                  \
        movpx_ld(Xmm7, Medx, mat_CMASK)                                     \
        PAINT_COLX(10, TEX_R)                                               \
        PAINT_COLX(08, TEX_G)                                               \
        PAINT_COLX(00, TEX_B)

/*
 * Flush all fragments of
 * the fully computed color SIMD-field from the context
 * into the framebuffer.
 */
#define FRAME_COLX(cl, pl) /* destroys Xmm0, Xmm1 */                        \
        movpx_ld(Xmm1, Mecx, ctx_##pl(0))                                   \
        minps_rr(Xmm1, Xmm2)                                                \
        cvtps_rr(Xmm1, Xmm1)                                                \
        andpx_rr(Xmm1, Xmm7)                                                \
        shlpx_ri(Xmm1, IB(0x##cl))                                          \
        addpx_rr(Xmm0, Xmm1)

#define FRAME_SIMD() /* destroys Xmm0, Xmm1, Xmm2, Xmm7, reads Reax */      \
        xorpx_rr(Xmm0, Xmm0)                                                \
        movpx_ld(Xmm2, Medx, cam_CLAMP)                                     \
        movpx_ld(Xmm7, Medx, cam_CMASK)                                     \
        FRAME_COLX(10, COL_R)                                               \
        FRAME_COLX(08, COL_G)                                               \
        FRAME_COLX(00, COL_B)                                               \
        movpx_st(Xmm0, Oeax, PLAIN)

/*
 * Replicate subroutine calling behaviour
 * by saving a given return address "lb" in the context's
 * local PTR field, then jumping to the destination address "to".
 * The destination code segment uses saved return address
 * to jump back after processing is finished. Parameters are
 * passed via context's local FLG field.
 */
#define SUBROUTINE(lb, to) /* destroys Reax */                              \
        adrxx_lb(lb)     /* load lb to Reax */                              \
        movxx_st(Reax, Mecx, ctx_LOCAL(PTR))                                \
        jmpxx_lb(to)                                                        \
    LBL(lb)

/******************************************************************************/
/*********************************   UPDATE   *********************************/
/******************************************************************************/

/*
 * Local pointer tables
 * for quick entry point resolution.
 */
static
rt_pntr t_ptr[3];

static
rt_pntr t_mat[3];

static
rt_pntr t_clp[3];

static
rt_pntr t_pow[6];

/*
 * Update material's backend-specific fields.
 */
static
rt_void update_mat(rt_SIMD_MATERIAL *s_mat)
{
    if (s_mat == RT_NULL || s_mat->pow_p[0] != RT_NULL)
    {
        return;
    }

    rt_cell i;
    rt_word pow = s_mat->l_pow[0], exp = 0;

    for (i = 0; i < 32; i++)
    {
        if (pow == ((rt_word)1 << i))
        {
            exp = i;
            break;
        }
    }

    if (i < 32)
    {
        pow = exp / 4;
        exp = exp % 4;

        if (pow > 0 && exp == 0)
        {
            pow--;
            exp = 4;
        }
    }
    else
    {
        exp = 5;
    }

    s_mat->l_pow[0] = pow;
    s_mat->pow_p[0] = t_pow[exp];
}

/*
 * Backend's global entry point (hence 0).
 * Update surfaces's backend-specific fields.
 */
rt_void update0(rt_SIMD_SURFACE *s_srf)
{
    rt_word tag = (rt_word)s_srf->srf_p[3];

    if (tag >= RT_TAG_SURFACE_MAX)
    {
        return;
    }

    /* save surface's entry points from local pointer tables
     * filled during backend's one-time initialization */
    s_srf->srf_p[0] = t_ptr[tag > RT_TAG_PLANE ? 
                            tag == RT_TAG_HYPERCYLINDER &&
                            s_srf->sci_w[0] == 0.0f ?
                            RT_TAG_PLANE + 1 : RT_TAG_PLANE + 2 : tag];

    s_srf->srf_p[1] = t_mat[tag > RT_TAG_PLANE ? 
                            tag != RT_TAG_PARABOLOID &&
                            tag != RT_TAG_PARACYLINDER &&
                            tag != RT_TAG_HYPERPARABOLOID ?
                            RT_TAG_PLANE + 1 : RT_TAG_PLANE + 2 : tag];

    s_srf->srf_p[2] = t_clp[tag > RT_TAG_PLANE ? 
                            tag != RT_TAG_PARABOLOID &&
                            tag != RT_TAG_PARACYLINDER &&
                            tag != RT_TAG_HYPERPARABOLOID ?
                            RT_TAG_PLANE + 1 : RT_TAG_PLANE + 2 : tag];

    s_srf->msc_p[1] =       tag == RT_TAG_CONE || 
                           (tag == RT_TAG_HYPERBOLOID ||
                            tag == RT_TAG_HYPERCYLINDER) &&
                            s_srf->sci_w[0] == 0.0f ?
                            (rt_pntr)1 : (rt_pntr)0;

    /* update surface's materials for each side */
    update_mat((rt_SIMD_MATERIAL *)s_srf->mat_p[0]);
    update_mat((rt_SIMD_MATERIAL *)s_srf->mat_p[2]);
}

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

#if RT_DEBUG == 1

    s_inf->q_dbg = (g_print != 0);

#endif /* RT_DEBUG */

/******************************************************************************/
/**********************************   ENTER   *********************************/
/******************************************************************************/

    ASM_ENTER(s_inf)

        /* if CTX is NULL fetch surface entry points
         * into the local pointer tables
         * as a part of backend's one-time initialization */
        cmpxx_mi(Mebp, inf_CTX, IB(0))
        jeqxx_lb(fetch_ptr)

        movxx_ld(Recx, Mebp, inf_CTX)
        movxx_ld(Redx, Mebp, inf_CAM)

        xorpx_rr(Xmm0, Xmm0)                    /* tmp_v <-     0 */
        movpx_st(Xmm0, Mecx, ctx_PARAM(0))      /* tmp_v -> PARAM */
        movpx_st(Xmm0, Mecx, ctx_LOCAL(0))      /* tmp_v -> LOCAL */

        jmpxx_lb(XX_set)

    LBL(XX_ret)

/******************************************************************************/
/********************************   RAY INIT   ********************************/
/******************************************************************************/

        movpx_ld(Xmm0, Medx, cam_DIR_X)         /* ray_x <- DIR_X */
        movpx_st(Xmm0, Mecx, ctx_RAY_X)         /* ray_x -> RAY_X */

        movpx_ld(Xmm0, Medx, cam_DIR_Y)         /* ray_y <- DIR_Y */
        movpx_st(Xmm0, Mecx, ctx_RAY_Y)         /* ray_y -> RAY_Y */

        movpx_ld(Xmm0, Medx, cam_DIR_Z)         /* ray_z <- DIR_Z */
        movpx_st(Xmm0, Mecx, ctx_RAY_Z)         /* ray_z -> RAY_Z */

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
        mulxn_ld(Reax, Mebp, inf_FRM_ROW)
        shlxx_ri(Reax, IB(2))
        addxx_ld(Reax, Mebp, inf_FRAME)
        movxx_st(Reax, Mebp, inf_FRM)

/******************************************************************************/
/********************************   HOR INIT   ********************************/
/******************************************************************************/

#if RT_FEAT_TILING

        movxx_ld(Reax, Mebp, inf_FRM_Y)
        movxx_ri(Redx, IB(0))
        divxn_xm(Mebp, inf_TILE_H)
        mulxn_xm(Mebp, inf_TLS_ROW)
        shlxx_ri(Reax, IB(2))
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
/********************************   OBJ LIST   ********************************/
/******************************************************************************/

#if RT_FEAT_TILING

        movxx_ld(Reax, Mebp, inf_TLS_X)
        shlxx_ri(Reax, IB(2))
        addxx_ld(Reax, Mebp, inf_TLS)
        movxx_ld(Resi, Oeax, PLAIN)

#else /* RT_FEAT_TILING */

        movxx_ld(Resi, Mebp, inf_LST)

#endif /* RT_FEAT_TILING */

    LBL(OO_cyc)

        cmpxx_ri(Resi, IB(0))
        jeqxx_lb(OO_out)

        movxx_ld(Rebx, Mesi, elm_SIMD)

#if RT_FEAT_TRANSFORM_ARRAY

        /* ctx_LOCAL(OBJ) holds trnode's
         * last element for transform caching,
         * caching is not applied if NULL */
        cmpxx_mi(Mebx, srf_SRF_P(TAG), IB(0))
        jltxn_lb(OO_dff)                        /* signed comparison */
        cmpxx_mi(Mecx, ctx_LOCAL(OBJ), IB(0))
        jeqxx_lb(OO_dff)

        movpx_ld(Xmm1, Mecx, ctx_DFF_X)
        movpx_ld(Xmm2, Mecx, ctx_DFF_Y)
        movpx_ld(Xmm3, Mecx, ctx_DFF_Z)

        /* contribute to trnode's POS,
         * add as ORG is subtracted from it */
        addps_ld(Xmm1, Mebx, srf_POS_X)
        addps_ld(Xmm2, Mebx, srf_POS_Y)
        addps_ld(Xmm3, Mebx, srf_POS_Z)

        movpx_st(Xmm1, Mecx, ctx_DFF_I)
        movpx_st(Xmm2, Mecx, ctx_DFF_J)
        movpx_st(Xmm3, Mecx, ctx_DFF_K)

        /* check if surface is trnode's
         * last element for transform caching */
        cmpxx_rm(Resi, Mecx, ctx_LOCAL(OBJ))
        jnexx_lb(OO_trm)

        /* reset ctx_LOCAL(OBJ) if so */
        movxx_mi(Mecx, ctx_LOCAL(OBJ), IB(0))
        jmpxx_lb(OO_trm)

    LBL(OO_dff)

#endif /* RT_FEAT_TRANSFORM_ARRAY */

#if RT_FEAT_BOUND_VOL_ARRAY

        /* only arrays are allowed to have
         * non-zero lower two bits in DATA field
         * for regular surface lists */
        movxx_ld(Reax, Mesi, elm_DATA)
        andxx_ri(Reax, IB(3))
        cmpxx_ri(Reax, IB(1))
        jeqxx_lb(AR_ptr)

#endif /* RT_FEAT_BOUND_VOL_ARRAY */

        /* compute negated diff
         * to compensate for minus in solvers */
        movpx_ld(Xmm1, Mebx, srf_POS_X)
        movpx_ld(Xmm2, Mebx, srf_POS_Y)
        movpx_ld(Xmm3, Mebx, srf_POS_Z)

        subps_ld(Xmm1, Mecx, ctx_ORG_X)
        subps_ld(Xmm2, Mecx, ctx_ORG_Y)
        subps_ld(Xmm3, Mecx, ctx_ORG_Z)

        movpx_st(Xmm1, Mecx, ctx_DFF_X)
        movpx_st(Xmm2, Mecx, ctx_DFF_Y)
        movpx_st(Xmm3, Mecx, ctx_DFF_Z)

#if RT_FEAT_TRANSFORM

        cmpxx_mi(Mebx, srf_A_MAP(RT_L*4), IB(0))
        jeqxx_lb(OO_trm)

        /* transform diff */
        movpx_ld(Xmm4, Mebx, srf_TCI_X)
        mulps_rr(Xmm4, Xmm1)
        movpx_ld(Xmm5, Mebx, srf_TCJ_Y)
        mulps_rr(Xmm5, Xmm2)
        movpx_ld(Xmm6, Mebx, srf_TCK_Z)
        mulps_rr(Xmm6, Xmm3)

        /* bypass non-diagonal terms
         * in transform matrix for scaling fastpath */
        cmpxx_mi(Mebx, srf_A_MAP(RT_L*4), IB(1))
        jeqxx_lb(OO_trd)

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

        cmpxx_mi(Mebx, srf_SRF_P(TAG), IB(0))
        jgexn_lb(OO_srf)                        /* signed comparison */

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

    LBL(OO_ray)

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
        cmpxx_mi(Mebx, srf_A_MAP(RT_L*4), IB(1))
        jeqxx_lb(OO_trr)

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

    LBL(OO_trm)

#endif /* RT_FEAT_TRANSFORM */

#if RT_FEAT_TRANSFORM_ARRAY

        /* skip trnode elements from the list */
        cmpxx_mi(Mebx, srf_SRF_P(PTR), IB(0))
        jeqxx_lb(OO_end)

#endif /* RT_FEAT_TRANSFORM_ARRAY */

        jmpxx_mm(Mebx, srf_SRF_P(PTR))

/******************************************************************************/
    LBL(fetch_ptr)

        jmpxx_lb(fetch_PL_ptr)

    LBL(fetch_mat)

        jmpxx_lb(fetch_PL_mat)

    LBL(fetch_clp)

#if RT_FEAT_CLIPPING_CUSTOM

        jmpxx_lb(fetch_PL_clp)

#endif /* RT_FEAT_CLIPPING_CUSTOM */

    LBL(fetch_pow)

#if RT_FEAT_LIGHTS && RT_FEAT_LIGHTS_SPECULAR

        jmpxx_lb(fetch_PW_ptr)

#endif /* RT_FEAT_LIGHTS, RT_FEAT_LIGHTS_SPECULAR */

        jmpxx_lb(fetch_end)

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
        subps_ld(Xmm4, Mebx, srf_POS_X)         /* loc_x -= POS_X */
        movpx_st(Xmm4, Mecx, ctx_NEW_X)         /* loc_x -> NEW_X */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

        /* "y" section */
        movpx_ld(Xmm5, Mecx, ctx_RAY_Y)         /* ray_y <- RAY_Y */
        mulps_rr(Xmm5, Xmm1)                    /* ray_y *= t_val */
        addps_ld(Xmm5, Mecx, ctx_ORG_Y)         /* hit_y += ORG_Y */
        movpx_st(Xmm5, Mecx, ctx_HIT_Y)         /* hit_y -> HIT_Y */
        subps_ld(Xmm5, Mebx, srf_POS_Y)         /* loc_y -= POS_Y */
        movpx_st(Xmm5, Mecx, ctx_NEW_Y)         /* loc_y -> NEW_Y */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

        /* "z" section */
        movpx_ld(Xmm6, Mecx, ctx_RAY_Z)         /* ray_z <- RAY_Z */
        mulps_rr(Xmm6, Xmm1)                    /* ray_z *= t_val */
        addps_ld(Xmm6, Mecx, ctx_ORG_Z)         /* hit_z += ORG_Z */
        movpx_st(Xmm6, Mecx, ctx_HIT_Z)         /* hit_z -> HIT_Z */
        subps_ld(Xmm6, Mebx, srf_POS_Z)         /* loc_z -= POS_Z */
        movpx_st(Xmm6, Mecx, ctx_NEW_Z)         /* loc_z -> NEW_Z */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

#if RT_FEAT_TRANSFORM

        cmpxx_mi(Mebx, srf_A_MAP(RT_L*4), IB(0))
        jeqxx_lb(CC_loc)

        /* "x" section */
        movpx_ld(Xmm4, Mecx, ctx_RAY_I)         /* ray_i <- RAY_I */
        mulps_rr(Xmm4, Xmm1)                    /* ray_i *= t_val */
        subps_ld(Xmm4, Mecx, ctx_DFF_I)         /* ray_i -= DFF_I */
        movpx_st(Xmm4, Mecx, ctx_NEW_I)         /* loc_i -> NEW_I */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

        /* "y" section */
        movpx_ld(Xmm5, Mecx, ctx_RAY_J)         /* ray_j <- RAY_J */
        mulps_rr(Xmm5, Xmm1)                    /* ray_j *= t_val */
        subps_ld(Xmm5, Mecx, ctx_DFF_J)         /* ray_j -= DFF_J */
        movpx_st(Xmm5, Mecx, ctx_NEW_J)         /* loc_j -> NEW_J */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

        /* "z" section */
        movpx_ld(Xmm6, Mecx, ctx_RAY_K)         /* ray_k <- RAY_K */
        mulps_rr(Xmm6, Xmm1)                    /* ray_k *= t_val */
        subps_ld(Xmm6, Mecx, ctx_DFF_K)         /* ray_k -= DFF_K */
        movpx_st(Xmm6, Mecx, ctx_NEW_K)         /* loc_k -> NEW_K */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

    LBL(CC_loc)

#endif /* RT_FEAT_TRANSFORM */

#if RT_DEBUG == 1

        cmpxx_mi(Mebp, inf_Q_DBG, IB(4))
        jnexx_lb(QD_go4)
        movxx_mi(Mebp, inf_Q_DBG, IB(5))

        movpx_st(Xmm4, Mebp, inf_HIT_X)
        movpx_st(Xmm5, Mebp, inf_HIT_Y)
        movpx_st(Xmm6, Mebp, inf_HIT_Z)

    LBL(QD_go4)

#endif /* RT_DEBUG */

        /* test for singularity */
        cmpxx_mi(Mebx, srf_MSC_P(FLG), IB(0))
        jeqxx_lb(CC_adj)

        cmpxx_mi(Mecx, ctx_XMISC(PTR), IB(0))
        jeqxx_lb(CC_adj)

        /* compute squares */
        movpx_rr(Xmm1, Xmm4)
        movpx_rr(Xmm2, Xmm5)
        movpx_rr(Xmm3, Xmm6)

        mulps_rr(Xmm1, Xmm4)
        mulps_rr(Xmm2, Xmm5)
        mulps_rr(Xmm3, Xmm6)

        /* mask out J axis */
        xorpx_rr(Xmm0, Xmm0)
        cneps_ld(Xmm0, Mebx, srf_SCI_X)
        andpx_rr(Xmm1, Xmm0)

        xorpx_rr(Xmm0, Xmm0)
        cneps_ld(Xmm0, Mebx, srf_SCI_Y)
        andpx_rr(Xmm2, Xmm0)

        xorpx_rr(Xmm0, Xmm0)
        cneps_ld(Xmm0, Mebx, srf_SCI_Z)
        andpx_rr(Xmm3, Xmm0)

        /* add squares */
        movpx_rr(Xmm0, Xmm1)
        addps_rr(Xmm0, Xmm2)
        addps_rr(Xmm0, Xmm3)

#if RT_DEBUG == 1

        cmpxx_mi(Mebp, inf_Q_DBG, IB(5))
        jnexx_lb(QD_go4a)

        movpx_st(Xmm0, Mebp, inf_HIT_R)

    LBL(QD_go4a)

#endif /* RT_DEBUG */

        /* check distance */
        cltps_ld(Xmm0, Mebx, srf_H_EPS)
        andpx_ld(Xmm0, Mecx, ctx_DMASK)
        CHECK_MASK(CC_adj, NONE, Xmm0)

        /* compute squares */
        movxx_ld(Reax, Mebx, srf_A_MAP(RT_I*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm1, Iecx, ctx_DFF_O)         /* dff_i <- DFF_I */
        subxx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iebx */
        movpx_rr(Xmm4, Xmm1)
        mulps_rr(Xmm4, Xmm1)
        movpx_rr(Xmm3, Xmm4)
        mulps_ld(Xmm3, Iebx, srf_SCI_O)         /* dff_i *= SCI_I */

        movxx_ld(Reax, Mebx, srf_A_MAP(RT_J*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm2, Iecx, ctx_DFF_O)         /* dff_j <- DFF_J */
        subxx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iebx */
        xorpx_rr(Xmm6, Xmm6)
        cneps_ld(Xmm6, Iebx, srf_SCI_O)
        andpx_rr(Xmm2, Xmm6)                    /* mask out J axis */
        movpx_rr(Xmm5, Xmm2)
        mulps_rr(Xmm5, Xmm2)
        movpx_rr(Xmm6, Xmm5)
        mulps_ld(Xmm6, Iebx, srf_SCI_O)         /* dff_j *= SCI_J */

        /* add squares */
        addps_rr(Xmm4, Xmm5)
        movpx_rr(Xmm5, Xmm4)

        /* compensate negated diff */
        movpx_ld(Xmm5, Mebx, srf_SMASK)
        xorpx_rr(Xmm1, Xmm5)                    /* dff_i ^= smask */
        xorpx_rr(Xmm2, Xmm5)                    /* dff_j ^= smask */

        /* check distance */
        cltps_ld(Xmm5, Mebx, srf_D_EPS)
        andpx_rr(Xmm5, Xmm0)
        CHECK_MASK(CC_sgn, NONE, Xmm5)

        /* substitute signs */
        movpx_ld(Xmm5, Mebx, srf_SMASK)
        andpx_rr(Xmm1, Xmm5)                    /* dff_i &= smask */
        andpx_rr(Xmm2, Xmm5)                    /* dff_j &= smask */
        movpx_ld(Xmm5, Mebp, inf_GPC01)
        xorpx_rr(Xmm1, Xmm5)                    /* dff_i ^= +1.0f */
        xorpx_rr(Xmm2, Xmm5)                    /* dff_j ^= +1.0f */

        /* compute squares */
        movxx_ld(Reax, Mebx, srf_A_MAP(RT_I*4)) /* Reax is used in Iecx */
        subxx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iebx */
        movpx_rr(Xmm4, Xmm5)
        movpx_ld(Xmm3, Iebx, srf_SCI_O)         /* dff_i <- SCI_I */

        movxx_ld(Reax, Mebx, srf_A_MAP(RT_J*4)) /* Reax is used in Iecx */
        subxx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iebx */
        xorpx_rr(Xmm6, Xmm6)
        cneps_ld(Xmm6, Iebx, srf_SCI_O)
        andpx_rr(Xmm2, Xmm6)                    /* mask out J axis */
        andpx_rr(Xmm5, Xmm6)
        movpx_ld(Xmm6, Iebx, srf_SCI_O)         /* dff_j <- SCI_J */

        /* add squares */
        addps_rr(Xmm4, Xmm5)

    LBL(CC_sgn)

        /* evaluate surface */
        movxx_ld(Reax, Mebx, srf_A_MAP(RT_K*4)) /* Reax is used in Iecx */
        addps_rr(Xmm3, Xmm6)
        subxx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iebx */
        divps_ld(Xmm3, Iebx, srf_SCI_O)
        andpx_ld(Xmm3, Mebp, inf_GPC04)
        movpx_rr(Xmm6, Xmm3)
        sqrps_rr(Xmm3, Xmm3)

        /* normalize point */
        addps_rr(Xmm6, Xmm4)
        rsqps_rr(Xmm4, Xmm6)
        mulps_ld(Xmm4, Mebx, srf_H_EPS)
        mulps_rr(Xmm1, Xmm4)
        mulps_rr(Xmm2, Xmm4)
        mulps_rr(Xmm3, Xmm4)

#if RT_DEBUG == 1

        cmpxx_mi(Mebp, inf_Q_DBG, IB(5))
        jnexx_lb(QD_go4b)

        movpx_st(Xmm4, Mebp, inf_ADJ_R)

    LBL(QD_go4b)

#endif /* RT_DEBUG */

        /* apply signs */
        movpx_ld(Xmm4, Mecx, ctx_AMASK)

        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))
        mulxn_ri(Reax, IB(RT_SIMD_WIDTH*4))
        movpx_ld(Xmm5, Iebx, srf_SBASE)

        movxx_ld(Reax, Mebx, srf_A_MAP(RT_K*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm6, Iecx, ctx_DFF_O)         /* dff_k <- DFF_K */
        andpx_ld(Xmm6, Mebx, srf_SMASK)
        xorpx_ld(Xmm6, Mebx, srf_SMASK)         /* compensate negated diff */

        xorpx_rr(Xmm3, Xmm6)

        movpx_rr(Xmm6, Xmm5)
        andpx_rr(Xmm6, Xmm4)
        xorpx_rr(Xmm6, Xmm4)
        xorpx_rr(Xmm3, Xmm6)

        orrpx_rr(Xmm5, Xmm4)
        xorpx_rr(Xmm5, Xmm4)
        xorpx_rr(Xmm1, Xmm5)
        xorpx_rr(Xmm2, Xmm5)

        /* recombine points */
        movxx_ld(Reax, Mebx, srf_A_MAP(RT_I*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm4, Iecx, ctx_NEW_O)         /* loc_i <- NEW_I */
        orrpx_rr(Xmm4, Xmm0)
        xorpx_rr(Xmm4, Xmm0)
        andpx_rr(Xmm1, Xmm0)
        orrpx_rr(Xmm4, Xmm1)
        movpx_st(Xmm4, Iecx, ctx_NEW_O)         /* loc_i -> NEW_I */

        subxx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iebx */
        movpx_ld(Xmm1, Iebx, srf_POS_X)         /* pos_i <- POS_I */
        movpx_ld(Xmm4, Iecx, ctx_HIT_X)         /* hit_i <- HIT_I */
        orrpx_rr(Xmm4, Xmm0)
        xorpx_rr(Xmm4, Xmm0)
        andpx_rr(Xmm1, Xmm0)
        orrpx_rr(Xmm4, Xmm1)
        movpx_st(Xmm4, Iecx, ctx_HIT_X)         /* hit_i -> HIT_I */

        movxx_ld(Reax, Mebx, srf_A_MAP(RT_K*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm6, Iecx, ctx_NEW_O)         /* loc_k <- NEW_K */
        orrpx_rr(Xmm6, Xmm0)
        xorpx_rr(Xmm6, Xmm0)
        andpx_rr(Xmm3, Xmm0)
        orrpx_rr(Xmm6, Xmm3)
        movpx_st(Xmm6, Iecx, ctx_NEW_O)         /* loc_k -> NEW_K */

        subxx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iebx */
        movpx_ld(Xmm3, Iebx, srf_POS_X)         /* pos_k <- POS_K */
        movpx_ld(Xmm6, Iecx, ctx_HIT_X)         /* hit_k <- HIT_K */
        orrpx_rr(Xmm6, Xmm0)
        xorpx_rr(Xmm6, Xmm0)
        andpx_rr(Xmm3, Xmm0)
        orrpx_rr(Xmm6, Xmm3)
        movpx_st(Xmm6, Iecx, ctx_HIT_X)         /* hit_k -> HIT_K */

        movxx_ld(Reax, Mebx, srf_A_MAP(RT_J*4)) /* Reax is used in Iecx */
        subxx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iebx */
        xorpx_rr(Xmm6, Xmm6)
        cneps_ld(Xmm6, Iebx, srf_SCI_O)
        andpx_rr(Xmm0, Xmm6)                    /* mask out J axis */

        movxx_ld(Reax, Mebx, srf_A_MAP(RT_J*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm5, Iecx, ctx_NEW_O)         /* loc_j <- NEW_J */
        orrpx_rr(Xmm5, Xmm0)
        xorpx_rr(Xmm5, Xmm0)
        andpx_rr(Xmm2, Xmm0)
        orrpx_rr(Xmm5, Xmm2)
        movpx_st(Xmm5, Iecx, ctx_NEW_O)         /* loc_j -> NEW_J */

        subxx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iebx */
        movpx_ld(Xmm2, Iebx, srf_POS_X)         /* hit_j <- POS_J */
        movpx_ld(Xmm5, Iecx, ctx_HIT_X)         /* hit_j <- HIT_J */
        orrpx_rr(Xmm5, Xmm0)
        xorpx_rr(Xmm5, Xmm0)
        andpx_rr(Xmm2, Xmm0)
        orrpx_rr(Xmm5, Xmm2)
        movpx_st(Xmm5, Iecx, ctx_HIT_X)         /* hit_j -> HIT_J */

        /* load adjusted point */
        movxx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm4, Iecx, ctx_NEW_X)         /* loc_x <- NEW_X */
        movpx_ld(Xmm5, Iecx, ctx_NEW_Y)         /* loc_y <- NEW_Y */
        movpx_ld(Xmm6, Iecx, ctx_NEW_Z)         /* loc_z <- NEW_Z */

    LBL(CC_adj)

#if RT_DEBUG == 1

        cmpxx_mi(Mebp, inf_Q_DBG, IB(5))
        jnexx_lb(QD_go5)
        movxx_mi(Mebp, inf_Q_DBG, IB(6))

        movpx_st(Xmm4, Mebp, inf_ADJ_X)
        movpx_st(Xmm5, Mebp, inf_ADJ_Y)
        movpx_st(Xmm6, Mebp, inf_ADJ_Z)

    LBL(QD_go5)

#endif /* RT_DEBUG */

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

#if RT_FEAT_CLIPPING_CUSTOM

        movxx_ld(Reax, Mebx, srf_MSC_P(OBJ))    /* load trnode's simd ptr */
        movxx_st(Reax, Mecx, ctx_LOCAL(LST))    /* save trnode's simd ptr */

        movxx_ri(Redx, IB(0))
        movxx_ld(Redi, Mebx, srf_MSC_P(CLP))

    LBL(CC_cyc)

        cmpxx_ri(Redi, IB(0))
        jeqxx_lb(CC_out)

        movxx_ld(Rebx, Medi, elm_SIMD)

#if RT_FEAT_CLIPPING_ACCUM

        cmpxx_ri(Rebx, IB(0))                   /* check accum marker */
        jnexx_lb(CC_acc)

        cmpxx_mi(Medi, elm_DATA, IB(0))         /* check accum enter/leave */
        jgtxn_lb(CC_acl)                        /* signed comparison */

        movpx_st(Xmm7, Mecx, ctx_C_TMP)         /* save current clip mask */
        movxx_ld(Rebx, Mesi, elm_SIMD)
        movpx_ld(Xmm7, Mebx, srf_C_TMP)         /* load default clip mask */
        jmpxx_lb(CC_end)                        /* accum enter */

    LBL(CC_acl)

        annpx_ld(Xmm7, Mecx, ctx_C_TMP)         /* apply accum clip mask */
        jmpxx_lb(CC_end)                        /* accum leave */

    LBL(CC_acc)

#endif /* RT_FEAT_CLIPPING_ACCUM */

#if RT_FEAT_TRANSFORM_ARRAY

        /* Redx holds trnode's
         * last element for transform caching,
         * caching is not applied if NULL */
        cmpxx_mi(Mebx, srf_SRF_P(TAG), IB(0))
        jltxn_lb(CC_arr)                        /* signed comparison */
        cmpxx_ri(Redx, IB(0))
        jeqxx_lb(CC_dff)

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
        cmpxx_rr(Redi, Redx)
        jnexx_lb(CC_trm)

        /* reset Redx if so */
        movxx_ri(Redx, IB(0))
        jmpxx_lb(CC_trm)

    LBL(CC_arr)

        /* handle the case when surface and its clippers
         * belong to the same trnode, shortcut transform */
        cmpxx_rm(Rebx, Mecx, ctx_LOCAL(LST))
        jnexx_lb(CC_dff)

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

        /* unlike for solvers
         * compute regular diff for clippers */
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

        cmpxx_mi(Mebx, srf_A_MAP(RT_L*4), IB(0))
        jeqxx_lb(CC_trm)

        /* transform clip */
        movpx_ld(Xmm4, Mebx, srf_TCI_X)
        mulps_rr(Xmm4, Xmm1)
        movpx_ld(Xmm5, Mebx, srf_TCJ_Y)
        mulps_rr(Xmm5, Xmm2)
        movpx_ld(Xmm6, Mebx, srf_TCK_Z)
        mulps_rr(Xmm6, Xmm3)

        /* bypass non-diagonal terms
         * in transform matrix for scaling fastpath */
        cmpxx_mi(Mebx, srf_A_MAP(RT_L*4), IB(1))
        jeqxx_lb(CC_trc)

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

        cmpxx_mi(Mebx, srf_SRF_P(TAG), IB(0))
        jgexn_lb(CC_srf)                        /* signed comparison */

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

    LBL(CC_trm)

#endif /* RT_FEAT_TRANSFORM */

        jmpxx_mm(Mebx, srf_SRF_P(CLP))

    LBL(CC_ret)

        andpx_rr(Xmm7, Xmm4)

    LBL(CC_end)

        movxx_ld(Redi, Medi, elm_NEXT)
        jmpxx_lb(CC_cyc)

    LBL(CC_out)

        movxx_ld(Rebx, Mesi, elm_SIMD)

#endif /* RT_FEAT_CLIPPING_CUSTOM */

        jmpxx_mm(Mecx, ctx_LOCAL(PTR))

/******************************************************************************/
/********************************   MATERIAL   ********************************/
/******************************************************************************/

#if RT_FEAT_NORMALS

    LBL(MT_nrm)

#if RT_FEAT_TRANSFORM

        cmpxx_mi(Mebx, srf_A_MAP(RT_L*4), IB(0))
        jeqxx_lb(MT_mat)

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
        cmpxx_mi(Mebx, srf_A_MAP(RT_L*4), IB(1))
        jeqxx_lb(MT_trn)

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
        cmpxx_mi(Mebx, srf_A_MAP(RT_L*4), IB(2))
        jeqxx_lb(MT_rnm)

    LBL(MT_trn)

        /* renormalize normal */
        movpx_rr(Xmm1, Xmm4)
        movpx_rr(Xmm2, Xmm5)
        movpx_rr(Xmm3, Xmm6)

        mulps_rr(Xmm1, Xmm4)
        mulps_rr(Xmm2, Xmm5)
        mulps_rr(Xmm3, Xmm6)

        addps_rr(Xmm1, Xmm2)
        addps_rr(Xmm1, Xmm3)
        rsqps_rr(Xmm0, Xmm1)

        mulps_rr(Xmm4, Xmm0)
        mulps_rr(Xmm5, Xmm0)
        mulps_rr(Xmm6, Xmm0)

    LBL(MT_rnm)

        /* store normal */
        movpx_st(Xmm4, Mecx, ctx_NRM_X)
        movpx_st(Xmm5, Mecx, ctx_NRM_Y)
        movpx_st(Xmm6, Mecx, ctx_NRM_Z)

        movxx_ld(Rebx, Mesi, elm_SIMD)          /* load surface's simd ptr */

#endif /* RT_FEAT_TRANSFORM */

#endif /* RT_FEAT_NORMALS */

    LBL(MT_mat)

        movxx_st(Resi, Mecx, ctx_LOCAL(LST))

        FETCH_XPTR(Redx, MAT_P(PTR))

        movpx_ld(Xmm0, Medx, mat_TEX_P)         /* tex_p <- TEX_P */

#if RT_FEAT_TEXTURING

        CHECK_PROP(MT_tex, RT_PROP_TEXTURE)

        /* enable "rounding towards minus infinity"
         * for texture coords float-to-integer conversion */
        FCTRL_ENTER(ROUNDM)

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
        cvtps_rr(Xmm1, Xmm4)                    /* tex_x ii tex_x */
        andpx_ld(Xmm1, Medx, mat_XMASK)         /* tex_y &= XMASK */

        cvtps_rr(Xmm2, Xmm5)                    /* tex_y ii tex_y */
        andpx_ld(Xmm2, Medx, mat_YMASK)         /* tex_y &= YMASK */
        shlpx_ld(Xmm2, Medx, mat_YSHFT)         /* tex_y << YSHFT */

        addpx_rr(Xmm1, Xmm2)                    /* tex_x += tex_y */
        shlpx_ri(Xmm1, IB(2))                   /* tex_x <<     2 */
        addpx_rr(Xmm0, Xmm1)                    /* tex_x += tex_p */

        /* restore default "rounding to nearest" */
        FCTRL_LEAVE(ROUNDM)

    LBL(MT_tex)

#endif /* RT_FEAT_TEXTURING */

        movpx_st(Xmm0, Mecx, ctx_C_PTR(0))      /* tex_p -> C_PTR */
        PAINT_SIMD(MT_rtx)

/******************************************************************************/
/*********************************   LIGHTS   *********************************/
/******************************************************************************/

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

        /* ambient R */
        movpx_st(Xmm1, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_amR, COL_R)

        /* ambient G */
        movpx_st(Xmm2, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_amG, COL_G)

        /* ambient B */
        movpx_st(Xmm3, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_amB, COL_B)

#endif /* RT_FEAT_LIGHTS_AMBIENT */

#if RT_FEAT_LIGHTS_DIFFUSE || RT_FEAT_LIGHTS_SPECULAR

        FETCH_XPTR(Redi, LST_P(LGT))

    LBL(LT_cyc)

        cmpxx_ri(Redi, IB(0))
        jeqxx_lb(LT_end)
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
        orrxx_ri(Reax, IB(RT_FLAG_SHAD | RT_FLAG_PASS_BACK))
        addxx_ri(Recx, IH(RT_STACK_STEP))
        subxx_mi(Mebp, inf_DEPTH, IB(1))

        movxx_st(Reax, Mecx, ctx_PARAM(FLG))    /* context flags */
        movxx_st(Redi, Mecx, ctx_PARAM(LST))    /* save light/shadow list */
        movxx_st(Rebx, Mecx, ctx_PARAM(OBJ))    /* originating surface */
        adrxx_lb(LT_ret)                        /* load LT_ret to Reax */
        movxx_st(Reax, Mecx, ctx_PARAM(PTR))    /* return ptr */
        movpx_st(Xmm0, Mecx, ctx_WMASK)         /* tmask -> WMASK */

        movpx_ld(Xmm0, Medx, lgt_T_MAX)         /* tmp_v <- T_MAX */
        movpx_st(Xmm0, Mecx, ctx_T_BUF(0))      /* tmp_v -> T_BUF */

        xorpx_rr(Xmm0, Xmm0)                    /* tmp_v <-     0 */
        movpx_st(Xmm7, Mecx, ctx_C_BUF(0))      /* hmask -> C_BUF */
        movpx_st(Xmm0, Mecx, ctx_COL_R(0))      /* tmp_v -> COL_R */
        movpx_st(Xmm0, Mecx, ctx_COL_G(0))      /* tmp_v -> COL_G */
        movpx_st(Xmm0, Mecx, ctx_COL_B(0))      /* tmp_v -> COL_B */

        movpx_st(Xmm0, Mecx, ctx_T_MIN)         /* tmp_v -> T_MIN */
        movpx_st(Xmm0, Mecx, ctx_LOCAL(0))      /* tmp_v -> LOCAL */

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

        rsqps_rr(Xmm5, Xmm4)                    /* Xmm5  <-   1/r */

#if RT_FEAT_LIGHTS_ATTENUATION

        /* compute attenuation */
        movpx_rr(Xmm4, Xmm5)                    /* Xmm4  <-   1/r */
        mulps_rr(Xmm4, Xmm6)                    /* Xmm4  <-   r^1 */

        movxx_ld(Redx, Medi, elm_SIMD)

        mulps_ld(Xmm6, Medx, lgt_A_QDR)
        mulps_ld(Xmm4, Medx, lgt_A_LNR)
        addps_ld(Xmm6, Medx, lgt_A_CNT)
        addps_rr(Xmm6, Xmm4)
        rsqps_rr(Xmm4, Xmm6)                    /* Xmm4  <-   1/a */

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
        rsqps_rr(Xmm5, Xmm6)
        mulps_rr(Xmm1, Xmm5)
        rsqps_rr(Xmm5, Xmm4)
        mulps_rr(Xmm1, Xmm5)

        /* compute specular pow,
         * integers only for now */
        movxx_ld(Reax, Medx, mat_L_POW)
        jmpxx_mm(Medx, mat_POW_P)

    LBL(fetch_PW_ptr)

        adrxx_lb(LT_pw4)                        /* load LT_pw4 to Reax */
        movxx_st(Reax, Mebp, inf_POW_E4)

        adrxx_lb(LT_pw3)                        /* load LT_pw3 to Reax */
        movxx_st(Reax, Mebp, inf_POW_E3)

        adrxx_lb(LT_pw2)                        /* load LT_pw2 to Reax */
        movxx_st(Reax, Mebp, inf_POW_E2)

        adrxx_lb(LT_pw1)                        /* load LT_pw1 to Reax */
        movxx_st(Reax, Mebp, inf_POW_E1)

        adrxx_lb(LT_pw0)                        /* load LT_pw0 to Reax */
        movxx_st(Reax, Mebp, inf_POW_E0)

        adrxx_lb(LT_pwn)                        /* load LT_pwn to Reax */
        movxx_st(Reax, Mebp, inf_POW_EN)

        jmpxx_lb(fetch_end)

    LBL(LT_pw4)

        mulps_rr(Xmm1, Xmm1)

    LBL(LT_pw3)

        mulps_rr(Xmm1, Xmm1)

    LBL(LT_pw2)

        mulps_rr(Xmm1, Xmm1)

    LBL(LT_pw1)

        mulps_rr(Xmm1, Xmm1)

        cmpxx_ri(Reax, IB(0))
        jeqxx_lb(LT_pwe)
        subxx_ri(Reax, IB(1))
        jmpxx_lb(LT_pw4)

    LBL(LT_pw0)

        jmpxx_lb(LT_pwe)

    LBL(LT_pwn)

        movpx_rr(Xmm2, Xmm1)
        movpx_ld(Xmm1, Mebp, inf_GPC01)

    LBL(LT_pwc)

        movxx_ri(Resi, IB(1))
        andxx_rr(Resi, Reax)
        shrxx_ri(Reax, IB(1))
        cmpxx_ri(Resi, IB(0))
        jeqxx_lb(LT_pws)
        mulps_rr(Xmm1, Xmm2)

    LBL(LT_pws)

        mulps_rr(Xmm2, Xmm2)
        cmpxx_ri(Reax, IB(0))
        jnexx_lb(LT_pwc)

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
        movpx_st(Xmm1, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_mcR, COL_R)

        /* diffuse + "metal" specular G */
        movpx_st(Xmm2, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_mcG, COL_G)

        /* diffuse + "metal" specular B */
        movpx_st(Xmm3, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_mcB, COL_B)

        jmpxx_lb(LT_amb)

#if RT_FEAT_LIGHTS_SPECULAR

    LBL(LT_mtl)

        movpx_rr(Xmm7, Xmm1)

        movxx_ld(Redx, Mebp, inf_CAM)
        mulps_ld(Xmm7, Medx, cam_CLAMP)

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
        movpx_st(Xmm1, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_pcR, COL_R)

        /* diffuse + "plain" specular G */
        movpx_st(Xmm2, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_pcG, COL_G)

        /* diffuse + "plain" specular B */
        movpx_st(Xmm3, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_pcB, COL_B)

#endif /* RT_FEAT_LIGHTS_SPECULAR */

    LBL(LT_amb)

        movxx_ld(Redi, Medi, elm_NEXT)
        jmpxx_lb(LT_cyc)

#endif /* RT_FEAT_LIGHTS_DIFFUSE, RT_FEAT_LIGHTS_SPECULAR */

        jmpxx_lb(LT_end)

#endif /* RT_FEAT_LIGHTS */

    LBL(LT_set)

        movpx_ld(Xmm1, Mecx, ctx_TEX_R)
        movpx_ld(Xmm2, Mecx, ctx_TEX_G)
        movpx_ld(Xmm3, Mecx, ctx_TEX_B)

        /* texture R */
        movpx_st(Xmm1, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_txR, COL_R)

        /* texture G */
        movpx_st(Xmm2, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_txG, COL_G)

        /* texture B */
        movpx_st(Xmm3, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_txB, COL_B)

    LBL(LT_end)

/******************************************************************************/
/*******************************   REFLECTIONS   ******************************/
/******************************************************************************/

#if RT_FEAT_REFLECTIONS

        FETCH_XPTR(Redx, MAT_P(PTR))

        CHECK_PROP(RF_end, RT_PROP_REFLECT)

        /* compute reflection */
        xorpx_rr(Xmm0, Xmm0)
        movpx_st(Xmm0, Mecx, ctx_T_NEW)

        movpx_ld(Xmm1, Mecx, ctx_RAY_X)
        movpx_ld(Xmm4, Mecx, ctx_NRM_X)
        movpx_rr(Xmm7, Xmm1)
        mulps_rr(Xmm7, Xmm4)
        addps_rr(Xmm0, Xmm7)

        movpx_ld(Xmm2, Mecx, ctx_RAY_Y)
        movpx_ld(Xmm5, Mecx, ctx_NRM_Y)
        movpx_rr(Xmm7, Xmm2)
        mulps_rr(Xmm7, Xmm5)
        addps_rr(Xmm0, Xmm7)

        movpx_ld(Xmm3, Mecx, ctx_RAY_Z)
        movpx_ld(Xmm6, Mecx, ctx_NRM_Z)
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

        xorpx_rr(Xmm1, Xmm1)
        xorpx_rr(Xmm2, Xmm2)
        xorpx_rr(Xmm3, Xmm3)

/************************************ ENTER ***********************************/

        cmpxx_mi(Mebp, inf_DEPTH, IB(0))
        jeqxx_lb(RF_mix)

        FETCH_XPTR(Resi, LST_P(SRF))

        movpx_ld(Xmm0, Mecx, ctx_TMASK(0))      /* load tmask */
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))
        orrxx_ri(Reax, IB(RT_FLAG_PASS_BACK))
        addxx_ri(Recx, IH(RT_STACK_STEP))
        subxx_mi(Mebp, inf_DEPTH, IB(1))

        movxx_st(Reax, Mecx, ctx_PARAM(FLG))    /* context flags */
        movxx_st(Redx, Mecx, ctx_PARAM(LST))    /* save material */
        movxx_st(Rebx, Mecx, ctx_PARAM(OBJ))    /* originating surface */
        adrxx_lb(RF_ret)                        /* load RF_ret to Reax */
        movxx_st(Reax, Mecx, ctx_PARAM(PTR))    /* return pointer */
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
        movpx_st(Xmm0, Mecx, ctx_LOCAL(0))      /* tmp_v -> LOCAL */

        jmpxx_lb(OO_cyc)

    LBL(RF_ret)

        movxx_ld(Redx, Mecx, ctx_PARAM(LST))    /* restore material */
        movxx_ld(Rebx, Mecx, ctx_PARAM(OBJ))    /* restore surface */

        movpx_ld(Xmm0, Medx, mat_C_RFL)

        movpx_ld(Xmm1, Mecx, ctx_COL_R(0))
        movpx_ld(Xmm2, Mecx, ctx_COL_G(0))
        movpx_ld(Xmm3, Mecx, ctx_COL_B(0))

        mulps_rr(Xmm1, Xmm0)
        mulps_rr(Xmm2, Xmm0)
        mulps_rr(Xmm3, Xmm0)

        addxx_mi(Mebp, inf_DEPTH, IB(1))
        subxx_ri(Recx, IH(RT_STACK_STEP))

/************************************ LEAVE ***********************************/

    LBL(RF_mix)

        movpx_ld(Xmm0, Mebp, inf_GPC01)
        subps_ld(Xmm0, Medx, mat_C_RFL)

        movpx_ld(Xmm4, Mecx, ctx_COL_R(0))
        movpx_ld(Xmm5, Mecx, ctx_COL_G(0))
        movpx_ld(Xmm6, Mecx, ctx_COL_B(0))

        mulps_rr(Xmm4, Xmm0)
        mulps_rr(Xmm5, Xmm0)
        mulps_rr(Xmm6, Xmm0)

        addps_rr(Xmm1, Xmm4)
        addps_rr(Xmm2, Xmm5)
        addps_rr(Xmm3, Xmm6)

        /* reflection R */
        movpx_st(Xmm1, Mecx, ctx_C_PTR(0))
        STORE_SIMD(RF_clR, COL_R)

        /* reflection G */
        movpx_st(Xmm2, Mecx, ctx_C_PTR(0))
        STORE_SIMD(RF_clG, COL_G)

        /* reflection B */
        movpx_st(Xmm3, Mecx, ctx_C_PTR(0))
        STORE_SIMD(RF_clB, COL_B)

    LBL(RF_end)

#endif /* RT_FEAT_REFLECTIONS */

/******************************************************************************/
/******************************   TRANSPARENCY   ******************************/
/******************************************************************************/

#if RT_FEAT_TRANSPARENCY

        FETCH_XPTR(Redx, MAT_P(PTR))

        CHECK_PROP(TR_opq, RT_PROP_OPAQUE)

        jmpxx_lb(TR_end)

    LBL(TR_opq)

#if RT_FEAT_REFRACTIONS

        CHECK_PROP(TR_rfr, RT_PROP_REFRACT)

        /* compute refraction */
        xorpx_rr(Xmm0, Xmm0)
        movpx_st(Xmm0, Mecx, ctx_T_NEW)

        movpx_ld(Xmm1, Mecx, ctx_RAY_X)
        movpx_ld(Xmm4, Mecx, ctx_NRM_X)
        movpx_rr(Xmm7, Xmm1)
        mulps_rr(Xmm7, Xmm4)
        addps_rr(Xmm0, Xmm7)

        movpx_ld(Xmm2, Mecx, ctx_RAY_Y)
        movpx_ld(Xmm5, Mecx, ctx_NRM_Y)
        movpx_rr(Xmm7, Xmm2)
        mulps_rr(Xmm7, Xmm5)
        addps_rr(Xmm0, Xmm7)

        movpx_ld(Xmm3, Mecx, ctx_RAY_Z)
        movpx_ld(Xmm6, Mecx, ctx_NRM_Z)
        movpx_rr(Xmm7, Xmm3)
        mulps_rr(Xmm7, Xmm6)
        addps_rr(Xmm0, Xmm7)

        mulps_ld(Xmm0, Medx, mat_C_RFR)
        movpx_rr(Xmm7, Xmm0)
        mulps_rr(Xmm7, Xmm7)
        addps_ld(Xmm7, Mebp, inf_GPC01)
        subps_ld(Xmm7, Medx, mat_RFR_2)
        sqrps_rr(Xmm7, Xmm7)
        addps_rr(Xmm0, Xmm7)
        movpx_ld(Xmm7, Medx, mat_C_RFR)

        mulps_rr(Xmm4, Xmm0)
        mulps_rr(Xmm1, Xmm7)
        subps_rr(Xmm1, Xmm4)
        movpx_st(Xmm1, Mecx, ctx_NEW_X)

        mulps_rr(Xmm5, Xmm0)
        mulps_rr(Xmm2, Xmm7)
        subps_rr(Xmm2, Xmm5)
        movpx_st(Xmm2, Mecx, ctx_NEW_Y)

        mulps_rr(Xmm6, Xmm0)
        mulps_rr(Xmm3, Xmm7)
        subps_rr(Xmm3, Xmm6)
        movpx_st(Xmm3, Mecx, ctx_NEW_Z)

        jmpxx_lb(TR_beg)

    LBL(TR_rfr)

#endif /* RT_FEAT_REFRACTIONS */

        /* propagate ray */
        xorpx_rr(Xmm0, Xmm0)
        movpx_st(Xmm0, Mecx, ctx_T_NEW)

        movpx_ld(Xmm1, Mecx, ctx_RAY_X)
        movpx_ld(Xmm2, Mecx, ctx_RAY_Y)
        movpx_ld(Xmm3, Mecx, ctx_RAY_Z)

        movpx_st(Xmm1, Mecx, ctx_NEW_X)
        movpx_st(Xmm2, Mecx, ctx_NEW_Y)
        movpx_st(Xmm3, Mecx, ctx_NEW_Z)

    LBL(TR_beg)

        xorpx_rr(Xmm1, Xmm1)
        xorpx_rr(Xmm2, Xmm2)
        xorpx_rr(Xmm3, Xmm3)

/************************************ ENTER ***********************************/

        cmpxx_mi(Mebp, inf_DEPTH, IB(0))
        jeqxx_lb(TR_mix)

        FETCH_IPTR(Resi, LST_P(SRF))

        movpx_ld(Xmm0, Mecx, ctx_TMASK(0))      /* load tmask */
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))
        orrxx_ri(Reax, IB(RT_FLAG_PASS_THRU))
        addxx_ri(Recx, IH(RT_STACK_STEP))
        subxx_mi(Mebp, inf_DEPTH, IB(1))

        movxx_st(Reax, Mecx, ctx_PARAM(FLG))    /* context flags */
        movxx_st(Redx, Mecx, ctx_PARAM(LST))    /* save material */
        movxx_st(Rebx, Mecx, ctx_PARAM(OBJ))    /* originating surface */
        adrxx_lb(TR_ret)                        /* load TR_ret to Reax */
        movxx_st(Reax, Mecx, ctx_PARAM(PTR))    /* return pointer */
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
        movpx_st(Xmm0, Mecx, ctx_LOCAL(0))      /* tmp_v -> LOCAL */

        jmpxx_lb(OO_cyc)

    LBL(TR_ret)

        movxx_ld(Redx, Mecx, ctx_PARAM(LST))    /* restore material */
        movxx_ld(Rebx, Mecx, ctx_PARAM(OBJ))    /* restore surface */

        movpx_ld(Xmm0, Medx, mat_C_TRN)

        movpx_ld(Xmm1, Mecx, ctx_COL_R(0))
        movpx_ld(Xmm2, Mecx, ctx_COL_G(0))
        movpx_ld(Xmm3, Mecx, ctx_COL_B(0))

        mulps_rr(Xmm1, Xmm0)
        mulps_rr(Xmm2, Xmm0)
        mulps_rr(Xmm3, Xmm0)

        addxx_mi(Mebp, inf_DEPTH, IB(1))
        subxx_ri(Recx, IH(RT_STACK_STEP))

/************************************ LEAVE ***********************************/

    LBL(TR_mix)

        movpx_ld(Xmm0, Mebp, inf_GPC01)
        subps_ld(Xmm0, Medx, mat_C_TRN)

        movpx_ld(Xmm4, Mecx, ctx_COL_R(0))
        movpx_ld(Xmm5, Mecx, ctx_COL_G(0))
        movpx_ld(Xmm6, Mecx, ctx_COL_B(0))

        mulps_rr(Xmm4, Xmm0)
        mulps_rr(Xmm5, Xmm0)
        mulps_rr(Xmm6, Xmm0)

        addps_rr(Xmm1, Xmm4)
        addps_rr(Xmm2, Xmm5)
        addps_rr(Xmm3, Xmm6)

        /* transparent R */
        movpx_st(Xmm1, Mecx, ctx_C_PTR(0))
        STORE_SIMD(TR_clR, COL_R)

        /* transparent G */
        movpx_st(Xmm2, Mecx, ctx_C_PTR(0))
        STORE_SIMD(TR_clG, COL_G)

        /* transparent B */
        movpx_st(Xmm3, Mecx, ctx_C_PTR(0))
        STORE_SIMD(TR_clB, COL_B)

    LBL(TR_end)

#endif /* RT_FEAT_TRANSPARENCY */

/******************************************************************************/
/********************************   MATERIAL   ********************************/
/******************************************************************************/

        movxx_ld(Resi, Mecx, ctx_LOCAL(LST))

        jmpxx_mm(Mecx, ctx_LOCAL(PTR))

/******************************************************************************/
/**********************************   ARRAY   *********************************/
/******************************************************************************/

#if RT_FEAT_BOUND_VOL_ARRAY

    LBL(AR_ptr)

        /* "x" section */
        movpx_ld(Xmm1, Mecx, ctx_RAY_X)         /* ray_x <- RAY_X */
        movpx_ld(Xmm5, Mebx, srf_POS_X)         /* dff_x <- POS_X */
        subps_ld(Xmm5, Mecx, ctx_ORG_X)         /* dff_x -= ORG_X */
        movpx_rr(Xmm3, Xmm1)                    /* ray_x <- ray_x */
        mulps_rr(Xmm3, Xmm5)                    /* ray_x *= dff_x */
        mulps_rr(Xmm1, Xmm1)                    /* ray_x *= ray_x */
        mulps_rr(Xmm5, Xmm5)                    /* dff_x *= dff_x */

        /* "y" section */
        movpx_ld(Xmm2, Mecx, ctx_RAY_Y)         /* ray_y <- RAY_Y */
        movpx_ld(Xmm6, Mebx, srf_POS_Y)         /* dff_y <- POS_Y */
        subps_ld(Xmm6, Mecx, ctx_ORG_Y)         /* dff_y -= ORG_Y */
        movpx_rr(Xmm4, Xmm2)                    /* ray_y <- ray_y */
        mulps_rr(Xmm4, Xmm6)                    /* ray_y *= dff_y */
        mulps_rr(Xmm2, Xmm2)                    /* ray_y *= ray_y */
        mulps_rr(Xmm6, Xmm6)                    /* dff_y *= dff_y */

        /* "+" section */
        addps_rr(Xmm1, Xmm2)                    /* axx_x += axx_y */
        addps_rr(Xmm3, Xmm4)                    /* bxx_x += bxx_y */
        addps_rr(Xmm5, Xmm6)                    /* cxx_x += cxx_y */

        /* "z" section */
        movpx_ld(Xmm2, Mecx, ctx_RAY_Z)         /* ray_z <- RAY_Z */
        movpx_ld(Xmm6, Mebx, srf_POS_Z)         /* dff_z <- POS_Z */
        subps_ld(Xmm6, Mecx, ctx_ORG_Z)         /* dff_z -= ORG_Z */
        movpx_rr(Xmm4, Xmm2)                    /* ray_z <- ray_z */
        mulps_rr(Xmm4, Xmm6)                    /* ray_z *= dff_z */
        mulps_rr(Xmm2, Xmm2)                    /* ray_z *= ray_z */
        mulps_rr(Xmm6, Xmm6)                    /* dff_z *= dff_z */

        /* "+" section */
        addps_rr(Xmm1, Xmm2)                    /* axx_t += axx_z */
        addps_rr(Xmm3, Xmm4)                    /* bxx_t += bxx_z */
        addps_rr(Xmm5, Xmm6)                    /* cxx_t += cxx_z */

        subps_ld(Xmm5, Mebx, srf_SCI_W)         /* cxx_t -= RAD_2 */

        /* "d" section */
        mulps_rr(Xmm5, Xmm1)                    /* c_val *= a_val */
        mulps_rr(Xmm3, Xmm3)                    /* b_val *= b_val */
        subps_rr(Xmm3, Xmm5)                    /* d_bxb -= d_axc */

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
        cmpxx_rm(Resi, Mecx, ctx_LOCAL(OBJ))
        jnexx_lb(OO_end)

        /* reset ctx_LOCAL(OBJ) if so */
        movxx_mi(Mecx, ctx_LOCAL(OBJ), IB(0))
        jmpxx_lb(OO_end)

#endif /* RT_FEAT_BOUND_VOL_ARRAY */

/******************************************************************************/
/**********************************   PLANE   *********************************/
/******************************************************************************/

    LBL(fetch_PL_ptr)

        adrxx_lb(PL_ptr)                        /* load PL_ptr to Reax */
        movxx_st(Reax, Mebp, inf_XPL_P(PTR))
        jmpxx_lb(fetch_TP_ptr)

    LBL(PL_ptr)

#if RT_SHOW_TILES

        SHOW_TILES(PL, 0x00880000)

#endif /* RT_SHOW_TILES */

        /* plane ignores secondary rays originating from itself
         * as it cannot possibly interact with them directly */
        cmpxx_rm(Rebx, Mecx, ctx_PARAM(OBJ))
        jeqxx_lb(OO_end)

        /* "k" section */
        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        MOVXR_LD(Xmm4, Iecx, ctx_DFF_O)         /* dff_k <- DFF_K */
        MOVXR_LD(Xmm3, Iecx, ctx_RAY_O)         /* ray_k <- RAY_K */

        /* create xmask */
        xorpx_rr(Xmm7, Xmm7)                    /* xmask <-     0 */
        cneps_rr(Xmm7, Xmm3)                    /* xmask != ray_k */
        andpx_ld(Xmm7, Mecx, ctx_WMASK)         /* xmask &= WMASK */

        /* "tt" section */
        divps_rr(Xmm4, Xmm3)                    /* dff_k /= ray_k */
        movpx_st(Xmm4, Mecx, ctx_T_VAL(0))      /* t_rt1 -> T_VAL */

        /* clipping */
        SUBROUTINE(PL_cp1, CC_clp)
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
        SUBROUTINE(PL_mt1, PL_mat)

/******************************************************************************/
    LBL(PL_rt2)

        /* inner side */
        movpx_ld(Xmm7, Mecx, ctx_TMASK(0))      /* tmask <- TMASK */
        xorpx_ld(Xmm7, Mecx, ctx_XMASK)         /* tmask ^= XMASK */
        CHECK_MASK(OO_end, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_TMASK(0))      /* tmask -> TMASK */

        /* material */
        movxx_mi(Mecx, ctx_LOCAL(FLG), IB(RT_FLAG_SIDE_INNER))
        SUBROUTINE(PL_mt2, PL_mat)

        jmpxx_lb(OO_end)

/******************************************************************************/
    LBL(fetch_PL_mat)

        adrxx_lb(PL_mat)                        /* load PL_mat to Reax */
        movxx_st(Reax, Mebp, inf_XPL_P(SRF))
        jmpxx_lb(fetch_TP_mat)

    LBL(PL_mat)

        FETCH_PROP()                            /* Xmm7  <- ssign */

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
        xorpx_rr(Xmm6, Xmm7)                    /* tmp_v ^= ssign */
        MOVXR_ST(Xmm6, Iecx, ctx_NRM_O)         /* tmp_v -> NRM_K */

        jmpxx_lb(MT_nrm)

    LBL(PL_nrm)

#endif /* RT_FEAT_NORMALS */

        jmpxx_lb(MT_mat)

/******************************************************************************/
#if RT_FEAT_CLIPPING_CUSTOM

    LBL(fetch_PL_clp)

        adrxx_lb(PL_clp)                        /* load PL_clp to Reax */
        movxx_st(Reax, Mebp, inf_XPL_P(CLP))
        jmpxx_lb(fetch_TP_clp)

    LBL(PL_clp)

        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        MOVXR_LD(Xmm4, Iecx, ctx_NRM_O)         /* dff_k <- NRM_K */
        xorpx_rr(Xmm6, Xmm6)                    /* tmp_k <-     0 */

        APPLY_CLIP(PL, Xmm4, Xmm6)

        jmpxx_lb(CC_ret)

#endif /* RT_FEAT_CLIPPING_CUSTOM */

/******************************************************************************/
/********************************   TWO-PLANE   *******************************/
/******************************************************************************/

    LBL(fetch_TP_ptr)

        adrxx_lb(TP_ptr)                        /* load TP_ptr to Reax */
        movxx_st(Reax, Mebp, inf_XTP_P(PTR))
        jmpxx_lb(fetch_QD_ptr)

    LBL(TP_ptr)

#if RT_SHOW_TILES

        SHOW_TILES(TP, 0x00884444)

#endif /* RT_SHOW_TILES */

        /* "i" section */
        movxx_ld(Reax, Mebx, srf_A_MAP(RT_I*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm1, Iecx, ctx_RAY_O)         /* ray_i <- RAY_I */
        movpx_ld(Xmm5, Iecx, ctx_DFF_O)         /* dff_i <- DFF_I */
        subxx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iebx */
        movpx_ld(Xmm3, Iebx, srf_SCI_O)         /* sci_i <- SCI_I */

        /* "k" section */
        movxx_ld(Reax, Mebx, srf_A_MAP(RT_K*4)) /* Reax is used in Iecx */
        movpx_ld(Xmm2, Iecx, ctx_RAY_O)         /* ray_k <- RAY_K */
        movpx_ld(Xmm6, Iecx, ctx_DFF_O)         /* dff_k <- DFF_K */
        subxx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iebx */
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
        movpx_ld(Xmm4, Iebx, srf_SCI_O)         /* sci_k <- SCI_K */

        /* "c" section */
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
    LBL(fetch_TP_mat)

        adrxx_lb(TP_mat)                        /* load TP_mat to Reax */
        movxx_st(Reax, Mebp, inf_XTP_P(SRF))
        jmpxx_lb(fetch_QD_mat)

    LBL(TP_mat)

#if RT_DEBUG == 1

        cmpxx_mi(Mebp, inf_Q_DBG, IB(6))
        jnexx_lb(QD_go6a)

        jmpxx_lb(QD_mat)

    LBL(QD_go6a)

#endif /* RT_DEBUG */

        FETCH_PROP()                            /* Xmm7  <- ssign */

#if RT_FEAT_LIGHTS_SHADOWS

        CHECK_SHAD(TP_shd)

#endif /* RT_FEAT_LIGHTS_SHADOWS */

#if RT_FEAT_NORMALS

        /* compute normal, if enabled */
        CHECK_PROP(TP_nrm, RT_PROP_NORMAL)

        movxx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iecx */

        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        movpx_ld(Xmm4, Iecx, ctx_NEW_X)         /* loc_x <- NEW_X */
        movpx_ld(Xmm5, Iecx, ctx_NEW_Y)         /* loc_y <- NEW_Y */
        movpx_ld(Xmm6, Iecx, ctx_NEW_Z)         /* loc_z <- NEW_Z */

        mulps_ld(Xmm4, Mebx, srf_SCI_X)         /* loc_x *= SCI_X */
        mulps_ld(Xmm5, Mebx, srf_SCI_Y)         /* loc_y *= SCI_Y */
        mulps_ld(Xmm6, Mebx, srf_SCI_Z)         /* loc_z *= SCI_Z */

        /* normalize normal */
        movpx_rr(Xmm1, Xmm4)
        movpx_rr(Xmm2, Xmm5)
        movpx_rr(Xmm3, Xmm6)

        mulps_rr(Xmm1, Xmm4)
        mulps_rr(Xmm2, Xmm5)
        mulps_rr(Xmm3, Xmm6)

        addps_rr(Xmm1, Xmm2)
        addps_rr(Xmm1, Xmm3)
        rsqps_rr(Xmm0, Xmm1)
        xorpx_rr(Xmm0, Xmm7)

        mulps_rr(Xmm4, Xmm0)
        mulps_rr(Xmm5, Xmm0)
        mulps_rr(Xmm6, Xmm0)

        /* store normal */
        movpx_st(Xmm4, Iecx, ctx_NRM_X)
        movpx_st(Xmm5, Iecx, ctx_NRM_Y)
        movpx_st(Xmm6, Iecx, ctx_NRM_Z)

        jmpxx_lb(MT_nrm)

    LBL(TP_nrm)

#endif /* RT_FEAT_NORMALS */

        jmpxx_lb(MT_mat)

/******************************************************************************/
#if RT_FEAT_CLIPPING_CUSTOM

    LBL(fetch_TP_clp)

        adrxx_lb(TP_clp)                        /* load TP_clp to Reax */
        movxx_st(Reax, Mebp, inf_XTP_P(CLP))
        jmpxx_lb(fetch_QD_clp)

    LBL(TP_clp)

        movxx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iecx */

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

        /* apply clip, equivalent to
         * APPLY_CLIP(QD, Xmm4, Xmm0) */
        movxx_ri(Reax, IB(1))
        subxx_ld(Reax, Medi, elm_DATA)
        shlxx_ri(Reax, IB(3))
        movpx_ld(Xmm1, Iebx, srf_SBASE)
        xorpx_rr(Xmm4, Xmm1)
        cleps_rr(Xmm4, Xmm0)

        jmpxx_lb(CC_ret)

#endif /* RT_FEAT_CLIPPING_CUSTOM */

/******************************************************************************/
/*********************************   QUADRIC   ********************************/
/******************************************************************************/

    LBL(fetch_QD_ptr)

        adrxx_lb(QD_ptr)                        /* load QD_ptr to Reax */
        movxx_st(Reax, Mebp, inf_XQD_P(PTR))
        jmpxx_lb(fetch_mat)

    LBL(QD_ptr)

#if RT_SHOW_TILES

        SHOW_TILES(QD, 0x00448844)

#endif /* RT_SHOW_TILES */

        movxx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iecx */

        /* "x" section */
        movpx_ld(Xmm1, Iecx, ctx_RAY_X)         /* ray_x <- RAY_X */
        movpx_ld(Xmm0, Mebx, srf_SCI_X)         /* sri_x <- SCI_X */
        mulps_rr(Xmm0, Xmm1)                    /* sri_x *= ray_x */
        movpx_ld(Xmm5, Iecx, ctx_DFF_X)         /* dff_x <- DFF_X */
        movpx_ld(Xmm7, Mebx, srf_SCI_X)         /* sdi_x <- SCI_X */
        mulps_rr(Xmm7, Xmm5)                    /* sdi_x *= dff_x */
        addps_ld(Xmm7, Mebx, srf_SCJ_X)         /* sdi_x += SCJ_X */
        movpx_rr(Xmm3, Xmm1)                    /* ray_x <- ray_x */
        mulps_rr(Xmm1, Xmm0)                    /* ray_x *= sri_x */
        mulps_rr(Xmm3, Xmm7)                    /* ray_x *= sdi_x */
        addps_ld(Xmm7, Mebx, srf_SCJ_X)         /* sdi_x += SCJ_X */
        mulps_rr(Xmm5, Xmm7)                    /* dff_x *= sdi_x */

        /* "y" section */
        movpx_ld(Xmm2, Iecx, ctx_RAY_Y)         /* ray_y <- RAY_Y */
        movpx_ld(Xmm0, Mebx, srf_SCI_Y)         /* sri_y <- SCI_Y */
        mulps_rr(Xmm0, Xmm2)                    /* sri_y *= ray_y */
        movpx_ld(Xmm6, Iecx, ctx_DFF_Y)         /* dff_y <- DFF_Y */
        movpx_ld(Xmm7, Mebx, srf_SCI_Y)         /* sdi_y <- SCI_Y */
        mulps_rr(Xmm7, Xmm6)                    /* sdi_y *= dff_y */
        addps_ld(Xmm7, Mebx, srf_SCJ_Y)         /* sdi_y += SCJ_Y */
        movpx_rr(Xmm4, Xmm2)                    /* ray_y <- ray_y */
        mulps_rr(Xmm2, Xmm0)                    /* ray_y *= sri_y */
        mulps_rr(Xmm4, Xmm7)                    /* ray_y *= sdi_y */
        addps_ld(Xmm7, Mebx, srf_SCJ_Y)         /* sdi_y += SCJ_Y */
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
        addps_ld(Xmm7, Mebx, srf_SCJ_Z)         /* sdi_z += SCJ_Z */
        movpx_rr(Xmm4, Xmm2)                    /* ray_z <- ray_z */
        mulps_rr(Xmm2, Xmm0)                    /* ray_z *= sri_z */
        mulps_rr(Xmm4, Xmm7)                    /* ray_z *= sdi_z */
        addps_ld(Xmm7, Mebx, srf_SCJ_Z)         /* sdi_z += SCJ_Z */
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

        movpx_st(Xmm3, Mecx, ctx_XTMP0)

#if RT_DEBUG == 1

        cmpxx_mi(Mebp, inf_FRM_X, IH(316))        /* <- pin point buggy quad */
        jnexx_lb(QD_go1)
        cmpxx_mi(Mebp, inf_FRM_Y, IH(419))        /* <- pin point buggy quad */
        jnexx_lb(QD_go1)

        cmpxx_mi(Mebp, inf_DEPTH, IB(0))
        jnexx_lb(QD_go1)

        cmpxx_mi(Mebp, inf_Q_DBG, IB(1))
        jnexx_lb(QD_go1)
        movxx_mi(Mebp, inf_Q_DBG, IB(2))

        movpx_st(Xmm1, Mebp, inf_A_VAL)
        movpx_st(Xmm4, Mebp, inf_B_VAL)
        movpx_st(Xmm6, Mebp, inf_C_VAL)
        movpx_st(Xmm3, Mebp, inf_D_VAL)

    LBL(QD_go1)

#endif /* RT_DEBUG */

        /* "tt" section */
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

#if RT_DEBUG == 1

        cmpxx_mi(Mebp, inf_Q_DBG, IB(2))
        jnexx_lb(QD_go2)
        movxx_mi(Mebp, inf_Q_DBG, IB(3))

        movpx_st(Xmm4, Mebp, inf_T1NMR)
        movpx_st(Xmm1, Mebp, inf_T1DNM)
        movpx_st(Xmm6, Mebp, inf_T2NMR)
        movpx_st(Xmm3, Mebp, inf_T2DNM)

        movpx_rr(Xmm2, Xmm4)
        divps_rr(Xmm2, Xmm1)                    /* bdval /= a_val */
        movpx_st(Xmm2, Mebp, inf_T1VAL)

        movpx_rr(Xmm5, Xmm6)
        divps_rr(Xmm5, Xmm3)                    /* c_val /= bdval */
        movpx_st(Xmm5, Mebp, inf_T2VAL)

    LBL(QD_go2)

#endif /* RT_DEBUG */

        /* "dd" section */
        movxx_mi(Mecx, ctx_XMISC(PTR), IB(0))

        movpx_ld(Xmm5, Mecx, ctx_XTMP0)         /* dmask <- d_val */
        cltps_ld(Xmm5, Mebx, srf_D_EPS)         /* dmask <! d_eps */
        andpx_rr(Xmm5, Xmm7)                    /* dmask &= xmask */
        CHECK_MASK(QD_dzr, NONE, Xmm5)

        divps_rr(Xmm4, Xmm1)                    /* bdval /= a_val */
        movpx_ld(Xmm2, Mebx, srf_SMASK)         /* amask <- smask */
        andpx_rr(Xmm2, Xmm0)                    /* amask &= a_val */
        divps_rr(Xmm6, Xmm3)                    /* c_val /= bdval */

        movpx_st(Xmm2, Mecx, ctx_AMASK)         /* amask -> AMASK */
        movpx_st(Xmm5, Mecx, ctx_DMASK)         /* dmask -> DMASK */

        andpx_rr(Xmm1, Xmm5)                    /* f1msk &= dmask */
        andpx_rr(Xmm3, Xmm5)                    /* f2msk &= dmask */
        movpx_ld(Xmm5, Mebp, inf_GPC04)         /* tmp_v <- GPC04 */
        andpx_rr(Xmm1, Xmm5)                    /* f1msk &= tmp_v */
        andpx_rr(Xmm3, Xmm5)                    /* f2msk &= tmp_v */
        xorpx_rr(Xmm5, Xmm5)                    /* tmp_v <-     0 */
        cneps_rr(Xmm1, Xmm5)                    /* f1msk != tmp_v */
        cneps_rr(Xmm3, Xmm5)                    /* f2msk != tmp_v */
        andpx_rr(Xmm1, Xmm3)                    /* f1msk &= f2msk */

        movpx_rr(Xmm3, Xmm6)                    /* t2tmp <- t2val */
        subps_rr(Xmm3, Xmm4)                    /* t2tmp -= t1val */
        xorpx_rr(Xmm3, Xmm2)                    /* t_dff ^= amask */
        cgeps_rr(Xmm5, Xmm3)                    /* tmp_v >= t_dff */
        andpx_rr(Xmm1, Xmm5)                    /* fmask &= tmp_v */
        xorpx_rr(Xmm3, Xmm2)                    /* t_dff ^= amask */
        andpx_rr(Xmm3, Xmm1)                    /* t_dff &= fmask */
        addps_rr(Xmm4, Xmm3)                    /* t1val += t_dff */
        subps_rr(Xmm6, Xmm3)                    /* t2val -= t_dff */

        movxx_mi(Mecx, ctx_XMISC(PTR), IB(1))

    LBL(QD_dzr)

#if RT_DEBUG == 1

        cmpxx_mi(Mebp, inf_Q_DBG, IB(3))
        jnexx_lb(QD_go3)
        movxx_mi(Mebp, inf_Q_DBG, IB(4))

        movpx_st(Xmm4, Mebp, inf_T1SRT)
        movpx_st(Xmm6, Mebp, inf_T2SRT)

    LBL(QD_go3)

#endif /* RT_DEBUG */

        /* "aa" section */
        movxx_mi(Mecx, ctx_XMISC(FLG), IB(2))
        movxx_mi(Mecx, ctx_XMISC(TAG), IB(0))

        xorpx_rr(Xmm5, Xmm5)                    /* tmp_v <-     0 */
        cgtps_rr(Xmm5, Xmm0)                    /* tmp_v >! a_val */
        andpx_rr(Xmm5, Xmm7)                    /* amask &= xmask */
        CHECK_MASK(QD_rc1, NONE, Xmm5)
        xorpx_rr(Xmm5, Xmm7)                    /* amask ^= xmask */
        CHECK_MASK(QD_rc2, NONE, Xmm5)

        movxx_mi(Mecx, ctx_XMISC(TAG), IB(1))
        jmpxx_lb(QD_rc1)

/******************************************************************************/
    LBL(QD_rs1)

        movpx_ld(Xmm4, Mecx, ctx_XTMP1)         /* bdval <- XTMP1 */
        movpx_ld(Xmm1, Mecx, ctx_XTMP2)         /* a_val <- XTMP2 */
        movpx_ld(Xmm7, Mecx, ctx_XMASK)         /* xmask <- XMASK */

    LBL(QD_rt1)

        /* side count check */
        cmpxx_mi(Mecx, ctx_XMISC(FLG), IB(0))
        jeqxx_lb(OO_end)

    LBL(QD_rc1)

        subxx_mi(Mecx, ctx_XMISC(FLG), IB(1))

        /* outer side */
        CHECK_SIDE(QD_sd1, QD_rt2, RT_FLAG_SIDE_OUTER)

        /* division check */
        cmpxx_mi(Mecx, ctx_XMISC(PTR), IB(0))
        jnexx_lb(QD_rd1)

        /* "t1" section */
        divps_rr(Xmm4, Xmm1)                    /* bdval /= a_val */
        movpx_st(Xmm3, Mecx, ctx_XTMP2)         /* bdval -> XTMP2 */

    LBL(QD_rd1)

        movpx_st(Xmm6, Mecx, ctx_XTMP1)         /* c_val -> XTMP1 */
        movpx_st(Xmm7, Mecx, ctx_XMASK)         /* xmask -> XMASK */
        movpx_st(Xmm4, Mecx, ctx_T_VAL(0))      /* t_rt1 -> T_VAL */

        movxx_mi(Mecx, ctx_LOCAL(FLG), IB(RT_FLAG_SIDE_OUTER))

        /* clipping */
        SUBROUTINE(QD_cp1, CC_clp)
        andpx_ld(Xmm7, Mecx, ctx_XMASK)         /* tmask &= XMASK */
        CHECK_MASK(QD_rs2, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_TMASK(0))      /* tmask -> TMASK */

        /* material */
        SUBROUTINE(QD_mt1, QD_rdr)

        /* side count check */
        cmpxx_mi(Mecx, ctx_XMISC(FLG), IB(0))
        jeqxx_lb(OO_end)

        /* overdraw check */
        cmpxx_mi(Mecx, ctx_XMISC(TAG), IB(0))
        jnexx_lb(QD_rs2)

        /* optimize overdraw */
        movpx_ld(Xmm7, Mecx, ctx_TMASK(0))      /* tmask <- TMASK */
        xorpx_ld(Xmm7, Mecx, ctx_XMASK)         /* tmask ^= XMASK */
        CHECK_MASK(OO_end, NONE, Xmm7)

/******************************************************************************/
    LBL(QD_rs2)

        movpx_ld(Xmm6, Mecx, ctx_XTMP1)         /* c_val <- XTMP1 */
        movpx_ld(Xmm3, Mecx, ctx_XTMP2)         /* bdval <- XTMP2 */
        movpx_ld(Xmm7, Mecx, ctx_XMASK)         /* xmask <- XMASK */

    LBL(QD_rt2)

        /* side count check */
        cmpxx_mi(Mecx, ctx_XMISC(FLG), IB(0))
        jeqxx_lb(OO_end)

    LBL(QD_rc2)

        subxx_mi(Mecx, ctx_XMISC(FLG), IB(1))

        /* inner side */
        CHECK_SIDE(QD_sd2, QD_rt1, RT_FLAG_SIDE_INNER)

        /* division check */
        cmpxx_mi(Mecx, ctx_XMISC(PTR), IB(0))
        jnexx_lb(QD_rd2)

        /* "t2" section */
        divps_rr(Xmm6, Xmm3)                    /* c_val /= bdval */
        movpx_st(Xmm1, Mecx, ctx_XTMP2)         /* a_val -> XTMP2 */

    LBL(QD_rd2)

        movpx_st(Xmm4, Mecx, ctx_XTMP1)         /* bdval -> XTMP1 */
        movpx_st(Xmm7, Mecx, ctx_XMASK)         /* xmask -> XMASK */
        movpx_st(Xmm6, Mecx, ctx_T_VAL(0))      /* t_rt2 -> T_VAL */

        movxx_mi(Mecx, ctx_LOCAL(FLG), IB(RT_FLAG_SIDE_INNER))

        /* clipping */
        SUBROUTINE(QD_cp2, CC_clp)
        andpx_ld(Xmm7, Mecx, ctx_XMASK)         /* tmask &= XMASK */
        CHECK_MASK(QD_rs1, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_TMASK(0))      /* tmask -> TMASK */

        /* material */
        SUBROUTINE(QD_mt2, QD_rdr)

        /* side count check */
        cmpxx_mi(Mecx, ctx_XMISC(FLG), IB(0))
        jeqxx_lb(OO_end)

        /* overdraw check */
        cmpxx_mi(Mecx, ctx_XMISC(TAG), IB(0))
        jnexx_lb(QD_rs1)

        /* optimize overdraw */
        movpx_ld(Xmm7, Mecx, ctx_TMASK(0))      /* tmask <- TMASK */
        xorpx_ld(Xmm7, Mecx, ctx_XMASK)         /* tmask ^= XMASK */
        CHECK_MASK(OO_end, NONE, Xmm7)

        jmpxx_lb(QD_rs1)

    LBL(QD_rdr)

        jmpxx_mm(Mebx, srf_SRF_P(SRF))

/******************************************************************************/
    LBL(fetch_QD_mat)

        adrxx_lb(QD_mat)                        /* load QD_mat to Reax */
        movxx_st(Reax, Mebp, inf_XQD_P(SRF))
        jmpxx_lb(fetch_clp)

    LBL(QD_mat)

        FETCH_PROP()                            /* Xmm7  <- ssign */

#if RT_FEAT_LIGHTS_SHADOWS

        CHECK_SHAD(QD_shd)

#endif /* RT_FEAT_LIGHTS_SHADOWS */

#if RT_FEAT_NORMALS

        /* compute normal, if enabled */
        CHECK_PROP(QD_nrm, RT_PROP_NORMAL)

        movxx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iecx */

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
        movpx_rr(Xmm1, Xmm4)
        movpx_rr(Xmm2, Xmm5)
        movpx_rr(Xmm3, Xmm6)

        mulps_rr(Xmm1, Xmm4)
        mulps_rr(Xmm2, Xmm5)
        mulps_rr(Xmm3, Xmm6)

        addps_rr(Xmm1, Xmm2)
        addps_rr(Xmm1, Xmm3)
        rsqps_rr(Xmm0, Xmm1)
        xorpx_rr(Xmm0, Xmm7)

        mulps_rr(Xmm4, Xmm0)
        mulps_rr(Xmm5, Xmm0)
        mulps_rr(Xmm6, Xmm0)

        /* store normal */
        movpx_st(Xmm4, Iecx, ctx_NRM_X)
        movpx_st(Xmm5, Iecx, ctx_NRM_Y)
        movpx_st(Xmm6, Iecx, ctx_NRM_Z)

#if RT_DEBUG == 1

        cmpxx_mi(Mebp, inf_Q_DBG, IB(6))
        jnexx_lb(QD_go6)
        movxx_mi(Mebp, inf_Q_DBG, IB(7))

        movpx_st(Xmm4, Mebp, inf_NRM_X)
        movpx_st(Xmm5, Mebp, inf_NRM_Y)
        movpx_st(Xmm6, Mebp, inf_NRM_Z)

    LBL(QD_go6)

#endif /* RT_DEBUG */

        jmpxx_lb(MT_nrm)

    LBL(QD_nrm)

#endif /* RT_FEAT_NORMALS */

        jmpxx_lb(MT_mat)

/******************************************************************************/
#if RT_FEAT_CLIPPING_CUSTOM

    LBL(fetch_QD_clp)

        adrxx_lb(QD_clp)                        /* load QD_clp to Reax */
        movxx_st(Reax, Mebp, inf_XQD_P(CLP))
        jmpxx_lb(fetch_pow)

    LBL(QD_clp)

        movxx_ld(Reax, Mebx, srf_A_SGN(RT_L*4)) /* Reax is used in Iecx */

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

        /* apply clip, equivalent to
         * APPLY_CLIP(QD, Xmm4, Xmm0) */
        movxx_ri(Reax, IB(1))
        subxx_ld(Reax, Medi, elm_DATA)
        shlxx_ri(Reax, IB(3))
        movpx_ld(Xmm1, Iebx, srf_SBASE)
        xorpx_rr(Xmm4, Xmm1)
        cleps_rr(Xmm4, Xmm0)

        jmpxx_lb(CC_ret)

#endif /* RT_FEAT_CLIPPING_CUSTOM */

/******************************************************************************/
/********************************   OBJ DONE   ********************************/
/******************************************************************************/

    LBL(OO_end)

        movxx_ld(Resi, Mesi, elm_NEXT)
        jmpxx_lb(OO_cyc)

    LBL(OO_out)

        jmpxx_mm(Mecx, ctx_PARAM(PTR))

/******************************************************************************/
/********************************   HOR SCAN   ********************************/
/******************************************************************************/

    LBL(XX_set)

        adrxx_lb(XX_end)                        /* load XX_end to Reax */
        movxx_st(Reax, Mecx, ctx_PARAM(PTR))
        jmpxx_lb(XX_ret)

    LBL(XX_end)

        movxx_ld(Redx, Mebp, inf_CAM)           /* edx needed in FRAME_SIMD */

#if RT_FEAT_ANTIALIASING

        cmpxx_mi(Mebp, inf_FSAA, IB(0))
        jeqxx_lb(FF_put)

        adrpx_ld(Reax, Mecx, ctx_C_BUF(0))
        FRAME_SIMD()

        movxx_ld(Rebx, Mecx, ctx_C_BUF(0x00))
        andxx_ri(Rebx, IW(0x00F0F0F0))
        movxx_rr(Reax, Rebx)
        movxx_ld(Rebx, Mecx, ctx_C_BUF(0x04))
        andxx_ri(Rebx, IW(0x00F0F0F0))
        addxx_rr(Reax, Rebx)
        movxx_ld(Rebx, Mecx, ctx_C_BUF(0x08))
        andxx_ri(Rebx, IW(0x00F0F0F0))
        addxx_rr(Reax, Rebx)
        movxx_ld(Rebx, Mecx, ctx_C_BUF(0x0C))
        andxx_ri(Rebx, IW(0x00F0F0F0))
        addxx_rr(Reax, Rebx)
        shrxx_ri(Reax, IB(2))

        movxx_ld(Rebx, Mecx, ctx_C_BUF(0x00))
        andxx_ri(Rebx, IW(0x000F0F0F))
        movxx_rr(Redx, Rebx)
        movxx_ld(Rebx, Mecx, ctx_C_BUF(0x04))
        andxx_ri(Rebx, IW(0x000F0F0F))
        addxx_rr(Redx, Rebx)
        movxx_ld(Rebx, Mecx, ctx_C_BUF(0x08))
        andxx_ri(Rebx, IW(0x000F0F0F))
        addxx_rr(Redx, Rebx)
        movxx_ld(Rebx, Mecx, ctx_C_BUF(0x0C))
        andxx_ri(Rebx, IW(0x000F0F0F))
        addxx_rr(Redx, Rebx)
        shrxx_ri(Redx, IB(2))

        andxx_ri(Redx, IW(0x000F0F0F))
        addxx_rr(Redx, Reax)

        movxx_ld(Reax, Mebp, inf_FRM_X)
        shlxx_ri(Reax, IB(2))
        addxx_ld(Reax, Mebp, inf_FRM)
        movxx_st(Redx, Oeax, PLAIN)
        addxx_mi(Mebp, inf_FRM_X, IB(1))

        jmpxx_lb(FF_end)

    LBL(FF_put)

#endif /* RT_FEAT_ANTIALIASING */

        movxx_ld(Reax, Mebp, inf_FRM_X)
        shlxx_ri(Reax, IB(2))
        addxx_ld(Reax, Mebp, inf_FRM)
        FRAME_SIMD()
        addxx_mi(Mebp, inf_FRM_X, IB(4))

    LBL(FF_end)

        movxx_ld(Reax, Mebp, inf_FRM_X)
        cmpxx_rm(Reax, Mebp, inf_FRM_W)
        jeqxx_lb(YY_end)

        movxx_ld(Redx, Mebp, inf_CAM)

        movpx_ld(Xmm0, Mecx, ctx_RAY_X)         /* ray_x <- RAY_X */
        addps_ld(Xmm0, Medx, cam_HOR_X)         /* ray_x += HOR_X */
        movpx_st(Xmm0, Mecx, ctx_RAY_X)         /* ray_x -> RAY_X */

        movpx_ld(Xmm1, Mecx, ctx_RAY_Y)         /* ray_y <- RAY_Y */
        addps_ld(Xmm1, Medx, cam_HOR_Y)         /* ray_y += HOR_Y */
        movpx_st(Xmm1, Mecx, ctx_RAY_Y)         /* ray_y -> RAY_Y */

        movpx_ld(Xmm2, Mecx, ctx_RAY_Z)         /* ray_z <- RAY_Z */
        addps_ld(Xmm2, Medx, cam_HOR_Z)         /* ray_z += HOR_Z */
        movpx_st(Xmm2, Mecx, ctx_RAY_Z)         /* ray_z -> RAY_Z */

#if RT_FEAT_TILING

        movxx_ld(Reax, Mebp, inf_FRM_X)
        movxx_ri(Redx, IB(0))
        divxn_xm(Mebp, inf_TILE_W)
        movxx_st(Reax, Mebp, inf_TLS_X)

#endif /* RT_FEAT_TILING */

        jmpxx_lb(XX_cyc)

/******************************************************************************/
/********************************   VER SCAN   ********************************/
/******************************************************************************/

    LBL(YY_end)

        movxx_ld(Redx, Mebp, inf_CAM)

        movpx_ld(Xmm0, Medx, cam_DIR_X)         /* ray_x <- DIR_X */
        addps_ld(Xmm0, Medx, cam_VER_X)         /* ray_x += VER_X */
        movpx_st(Xmm0, Medx, cam_DIR_X)         /* ray_x -> DIR_X */
        movpx_st(Xmm0, Mecx, ctx_RAY_X)         /* ray_x -> RAY_X */

        movpx_ld(Xmm1, Medx, cam_DIR_Y)         /* ray_y <- DIR_Y */
        addps_ld(Xmm1, Medx, cam_VER_Y)         /* ray_y += VER_Y */
        movpx_st(Xmm1, Medx, cam_DIR_Y)         /* ray_y -> DIR_Y */
        movpx_st(Xmm1, Mecx, ctx_RAY_Y)         /* ray_y -> RAY_Y */

        movpx_ld(Xmm2, Medx, cam_DIR_Z)         /* ray_z <- DIR_Z */
        addps_ld(Xmm2, Medx, cam_VER_Z)         /* ray_z += VER_Z */
        movpx_st(Xmm2, Medx, cam_DIR_Z)         /* ray_z -> DIR_Z */
        movpx_st(Xmm2, Mecx, ctx_RAY_Z)         /* ray_z -> RAY_Z */

#if RT_FEAT_MULTITHREADING

        movxx_ld(Reax, Mebp, inf_THNUM)
        addxx_st(Reax, Mebp, inf_FRM_Y)

#else /* RT_FEAT_MULTITHREADING */

        addxx_mi(Mebp, inf_FRM_Y, IB(1))

#endif /* RT_FEAT_MULTITHREADING */

        movxx_ld(Reax, Mebp, inf_FRM_Y)
        cmpxx_rm(Reax, Mebp, inf_FRM_H)
        jltxx_lb(YY_cyc)

    LBL(fetch_end)

    ASM_LEAVE(s_inf)

/******************************************************************************/
/**********************************   LEAVE   *********************************/
/******************************************************************************/

    if (s_inf->ctx != RT_NULL)
    {
#if RT_DEBUG == 1

        if (s_inf->q_dbg == 7)
        {
            RT_LOGE("---------------------------------------------");
            RT_LOGE("------------- quadric debug info ------------");
            RT_LOGE("---------------------------------------------");
            RT_LOGE("\n");
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

            RT_LOGE("\n");

            RT_LOGE("    HIT_X = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->hit_x[0], s_inf->hit_x[1], s_inf->hit_x[2], s_inf->hit_x[3]);

            RT_LOGE("    HIT_Y = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->hit_y[0], s_inf->hit_y[1], s_inf->hit_y[2], s_inf->hit_y[3]);

            RT_LOGE("    HIT_Z = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->hit_z[0], s_inf->hit_z[1], s_inf->hit_z[2], s_inf->hit_z[3]);

            RT_LOGE("    HIT_R = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->hit_r[0], s_inf->hit_r[1], s_inf->hit_r[2], s_inf->hit_r[3]);

            RT_LOGE("\n");

            RT_LOGE("    ADJ_X = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->adj_x[0], s_inf->adj_x[1], s_inf->adj_x[2], s_inf->adj_x[3]);

            RT_LOGE("    ADJ_Y = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->adj_y[0], s_inf->adj_y[1], s_inf->adj_y[2], s_inf->adj_y[3]);

            RT_LOGE("    ADJ_Z = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->adj_z[0], s_inf->adj_z[1], s_inf->adj_z[2], s_inf->adj_z[3]);

            RT_LOGE("    ADJ_R = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->adj_r[0], s_inf->adj_r[1], s_inf->adj_r[2], s_inf->adj_r[3]);

            RT_LOGE("\n");

            RT_LOGE("    NRM_X = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->nrm_x[0], s_inf->nrm_x[1], s_inf->nrm_x[2], s_inf->nrm_x[3]);

            RT_LOGE("    NRM_Y = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->nrm_y[0], s_inf->nrm_y[1], s_inf->nrm_y[2], s_inf->nrm_y[3]);

            RT_LOGE("    NRM_Z = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->nrm_z[0], s_inf->nrm_z[1], s_inf->nrm_z[2], s_inf->nrm_z[3]);

            RT_LOGE("    NRM_R = { %+27.20f, %+27.20f, %+27.20f, %+27.20f }\n",
            s_inf->nrm_r[0], s_inf->nrm_r[1], s_inf->nrm_r[2], s_inf->nrm_r[3]);

            RT_LOGE("\n");
        }

#endif /* RT_DEBUG */

        return;
    }

    t_ptr[RT_TAG_PLANE]             = s_inf->xpl_p[0];
    t_ptr[RT_TAG_PLANE + 1]         = s_inf->xtp_p[0];
    t_ptr[RT_TAG_PLANE + 2]         = s_inf->xqd_p[0];

    t_mat[RT_TAG_PLANE]             = s_inf->xpl_p[1];
    t_mat[RT_TAG_PLANE + 1]         = s_inf->xtp_p[1];
    t_mat[RT_TAG_PLANE + 2]         = s_inf->xqd_p[1];

    t_clp[RT_TAG_PLANE]             = s_inf->xpl_p[2];
    t_clp[RT_TAG_PLANE + 1]         = s_inf->xtp_p[2];
    t_clp[RT_TAG_PLANE + 2]         = s_inf->xqd_p[2];

    t_pow[0]                        = s_inf->pow_e0;
    t_pow[1]                        = s_inf->pow_e1;
    t_pow[2]                        = s_inf->pow_e2;
    t_pow[3]                        = s_inf->pow_e3;
    t_pow[4]                        = s_inf->pow_e4;
    t_pow[5]                        = s_inf->pow_en;

#if RT_DEBUG == 1

    RT_LOGI("PL ptr = %p\n", s_inf->xpl_p[0]);
    RT_LOGI("TP ptr = %p\n", s_inf->xtp_p[0]);
    RT_LOGI("QD ptr = %p\n", s_inf->xqd_p[0]);

    RT_LOGI("PL mat = %p\n", s_inf->xpl_p[1]);
    RT_LOGI("TP mat = %p\n", s_inf->xtp_p[1]);
    RT_LOGI("QD mat = %p\n", s_inf->xqd_p[1]);

    RT_LOGI("PL clp = %p\n", s_inf->xpl_p[2]);
    RT_LOGI("TP clp = %p\n", s_inf->xtp_p[2]);
    RT_LOGI("QD clp = %p\n", s_inf->xqd_p[2]);

    RT_LOGI("E0 pow = %p\n", s_inf->pow_e0);
    RT_LOGI("E1 pow = %p\n", s_inf->pow_e1);
    RT_LOGI("E2 pow = %p\n", s_inf->pow_e2);
    RT_LOGI("E3 pow = %p\n", s_inf->pow_e3);
    RT_LOGI("E4 pow = %p\n", s_inf->pow_e4);
    RT_LOGI("EN pow = %p\n", s_inf->pow_en);

#endif /* RT_DEBUG */
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
