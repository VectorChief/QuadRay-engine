/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include "tracer.h"
#include "format.h"
#include "system.h"

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

/*
 * Conditional compilation flags
 * for respective segments of code.
 */
#define RT_SHOW_TILES               0

#define RT_FEAT_TILING              1
#define RT_FEAT_MULTITHREADING      1
#define RT_FEAT_CLIPPING_MINMAX     1
#define RT_FEAT_CLIPPING_CUSTOM     1
#define RT_FEAT_CLIPPING_ACCUM      1
#define RT_FEAT_TEXTURING           1
#define RT_FEAT_NORMALS             1
#define RT_FEAT_LIGHTING            1
#define RT_FEAT_ATTENUATION         1
#define RT_FEAT_SPECULAR            1
#define RT_FEAT_SHADOWS             1
#define RT_FEAT_REFLECTIONS         1
#define RT_FEAT_TRANSPARENCY        1
#define RT_FEAT_REFRACTIONS         1
#define RT_FEAT_ANTIALIASING        1
#define RT_FEAT_TRANSFORM           1
#define RT_FEAT_TRANSFORM_ARRAY     1

/*
 * Byte-offsets within SIMD-field
 * for packed scalar fields.
 */
#define PTR   0x00 /* LOCAL, PARAM, MAT_P, SRF_P */
#define LGT   0x00 /* LST_P */
#define FLG   0x04 /* LOCAL, PARAM, MAT_P */
#define SRF   0x04 /* LST_P */
#define CLP   0x08 /* MSC_P, SRF_P */
#define LST   0x08 /* LOCAL, PARAM */
#define OBJ   0x0C /* LOCAL, PARAM, MSC_P */
#define TAG   0x0C /* SRF_P */

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
#define SHOW_TILES(lb, cl)                                                  \
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
#define INDEX_AXIS(nx)                                                      \
        movxx_ld(Reax, Mebx, srf_A_SGN(nx * 4))                             \
        movpx_ld(Xmm0, Iebx, srf_SBASE)                                     \
        movxx_ld(Reax, Mebx, srf_A_MAP(nx * 4))

#define MOVXR_LD(RG, RM, DP)                                                \
        movpx_ld(W(RG), W(RM), W(DP))                                       \
        xorpx_rr(W(RG), Xmm0)

#define MOVXR_ST(RG, RM, DP)                                                \
        xorpx_rr(W(RG), Xmm0)                                               \
        movpx_st(W(RG), W(RM), W(DP))

#define MOVZR_ST(RG, RM, DP)                                                \
        xorpx_rr(W(RG), W(RG))                                              \
        movpx_st(W(RG), W(RM), W(DP))

#define INDEX_TMAP(nx)                                                      \
        movxx_ld(Reax, Medx, mat_T_MAP(nx * 4))

/*
 * Axis clipping.
 * Check if axis clipping (minmax) is needed for given axis "nx",
 * jump to "lb" otherwise.
 */
#define CHECK_CLIP(lb, pl, nx)                                              \
        cmpxx_mi(Mebx, srf_##pl(nx * 4), IB(0))                             \
        jeqxx_lb(lb)

/*
 * Custom clipping.
 * Apply custom clipping (by surface) to ray's hit point
 * based on the side of the clipping surface.
 */
#define APPLY_CLIP(lb, RG, RM)                                              \
        movxx_ri(Reax, IB(1))                                               \
        subxx_ld(Reax, Medi, elm_DATA)                                      \
        shlxx_ri(Reax, IB(3))                                               \
        cmpxx_ri(Reax, IB(0))                                               \
        jgtxx_lb(lb##_cs1)                                                  \
        cleps_rr(W(RG), W(RM))                                              \
        jmpxx_lb(lb##_cs2)                                                  \
    LBL(lb##_cs1)                                                           \
        cgtps_rr(W(RG), W(RM))                                              \
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
#define CHECK_FLAG(lb, pl, fl)                                              \
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
#define CHECK_SIDE(lb, lo, sd)                                              \
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
#define CHECK_SHAD(lb)                                                      \
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
 * based on the currently set SIDE flag.
 * Load SIDE's sign into Xmm7 for normals.
 */
#define FETCH_PROP()                                                        \
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))                                \
        andxx_ri(Reax, IB(RT_FLAG_SIDE))                                    \
        shlxx_ri(Reax, IB(4))                                               \
        movpx_ld(Xmm7, Iebx, srf_SBASE)                                     \
        shrxx_ri(Reax, IB(1))                                               \
        movxx_ld(Reax, Iebx, srf_MAT_P(FLG))                                \
        orrxx_st(Reax, Mecx, ctx_LOCAL(FLG))

/*
 * Check if property "pr" previously
 * fetched from material is set in the context,
 * jump to "lb" otherwise.
 */
#define CHECK_PROP(lb, pr)                                                  \
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))                                \
        andxx_ri(Reax, IH(pr))                                              \
        cmpxx_ri(Reax, IB(0))                                               \
        jeqxx_lb(lb)

/*
 * Fetch pointer into given register "RG" from surface's field "pl"
 * based on the currently set SIDE flag.
 */
#define FETCH_XPTR(RG, pl)                                                  \
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))                                \
        andxx_ri(Reax, IB(RT_FLAG_SIDE))                                    \
        shlxx_ri(Reax, IB(3))                                               \
        movxx_ld(W(RG), Iebx, srf_##pl)

/*
 * Fetch pointer into given register "RG" from surface's field "pl"
 * based on the currently set inverted SIDE flag.
 */
#define FETCH_IPTR(RG, pl)                                                  \
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
#define STORE_FRAG(lb, pn, pl)                                              \
        cmpxx_mi(Mecx, ctx_TMASK(0x##pn), IB(0))                            \
        jeqxx_lb(lb##pn)                                                    \
        movxx_ld(Reax, Mecx, ctx_C_PTR(0x##pn))                             \
        movxx_st(Reax, Mecx, ctx_##pl(0x##pn))                              \
    LBL(lb##pn)

#define STORE_SIMD(lb, pl)                                                  \
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
#define PAINT_FRAG(lb, pn)                                                  \
        cmpxx_mi(Mecx, ctx_TMASK(0x##pn), IB(0))                            \
        jeqxx_lb(lb##pn)                                                    \
        movxx_ld(Reax, Mecx, ctx_T_VAL(0x##pn))                             \
        movxx_st(Reax, Mecx, ctx_T_BUF(0x##pn))                             \
        movxx_ld(Reax, Mecx, ctx_C_PTR(0x##pn))                             \
        movxx_ld(Reax, Oeax, PLAIN)                                         \
        movxx_st(Reax, Mecx, ctx_C_BUF(0x##pn))                             \
    LBL(lb##pn)

#define PAINT_COLX(cl, pl)                                                  \
        movpx_ld(Xmm0, Mecx, ctx_C_BUF(0))                                  \
        shrpx_ri(Xmm0, IB(0x##cl))                                          \
        andpx_rr(Xmm0, Xmm7)                                                \
        cvtpn_rr(Xmm0, Xmm0)                                                \
        movpx_st(Xmm0, Mecx, ctx_##pl)

#define PAINT_SIMD(lb)                                                      \
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
#define FRAME_COLX(cl, pl)                                                  \
        movpx_ld(Xmm1, Mecx, ctx_##pl(0))                                   \
        minps_rr(Xmm1, Xmm2)                                                \
        cvtps_rr(Xmm1, Xmm1)                                                \
        andpx_rr(Xmm1, Xmm7)                                                \
        shlpx_ri(Xmm1, IB(0x##cl))                                          \
        addpx_rr(Xmm0, Xmm1)

#define FRAME_SIMD()                                                        \
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
#define SUBROUTINE(lb, to)                                                  \
        adrxx_lb(lb)                                                        \
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
rt_pntr t_ptr[RT_TAG_SURFACE_MAX];

static
rt_pntr t_clp[RT_TAG_SURFACE_MAX];

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
    s_srf->srf_p[0] = t_ptr[tag];
    s_srf->srf_p[2] = t_clp[tag];

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

        cmpxx_mi(Mebx, srf_A_MAP(RT_W * 4), IB(0))
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
        cmpxx_mi(Mebx, srf_A_MAP(RT_W * 4), IB(1))
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
        cmpxx_mi(Mebx, srf_A_MAP(RT_W * 4), IB(1))
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

    LBL(fetch_ptr)

#if RT_FEAT_SPECULAR

        jmpxx_lb(fetch_PW_ptr)

#else /* RT_FEAT_SPECULAR */

        jmpxx_lb(fetch_PL_ptr)

#endif /* RT_FEAT_SPECULAR */

/******************************************************************************/
/********************************   CLIPPING   ********************************/
/******************************************************************************/

    LBL(CC_clp)

        /* depth testing */
        movpx_ld(Xmm0, Mecx, ctx_T_BUF(0))      /* t_buf <- T_BUF */
        cgtps_rr(Xmm0, Xmm4)                    /* t_buf >! t_rt1 */
        andpx_rr(Xmm7, Xmm0)                    /* tmask &= gmask */

        /* near plane clipping */
        movpx_ld(Xmm0, Mecx, ctx_T_MIN)         /* t_min <- T_MIN */
        cltps_rr(Xmm0, Xmm4)                    /* t_min <! t_rt1 */
        andpx_rr(Xmm7, Xmm0)                    /* tmask &= lmask */

        /* "x" section */
        movpx_ld(Xmm4, Mecx, ctx_RAY_X)         /* ray_x <- RAY_X */
        mulps_ld(Xmm4, Mecx, ctx_T_VAL(0))      /* ray_x *= t_rt1 */
        addps_ld(Xmm4, Mecx, ctx_ORG_X)         /* hit_x += ORG_X */
        movpx_st(Xmm4, Mecx, ctx_HIT_X)         /* hit_x -> HIT_X */
        subps_ld(Xmm4, Mebx, srf_POS_X)         /* loc_x -= POS_X */
        movpx_st(Xmm4, Mecx, ctx_NEW_X)         /* loc_x -> NEW_X */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

#if RT_FEAT_TRANSFORM

        cmpxx_mi(Mebx, srf_A_MAP(RT_W * 4), IB(0))
        jeqxx_lb(CX_trm)

        movpx_ld(Xmm4, Mecx, ctx_RAY_I)         /* ray_i <- RAY_I */
        mulps_ld(Xmm4, Mecx, ctx_T_VAL(0))      /* ray_i *= t_rt1 */
        subps_ld(Xmm4, Mecx, ctx_DFF_I)         /* ray_i -= DFF_I */
        movpx_st(Xmm4, Mecx, ctx_NEW_I)         /* loc_i -> NEW_I */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

    LBL(CX_trm)

#endif /* RT_FEAT_TRANSFORM */

#if RT_FEAT_CLIPPING_MINMAX

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

#endif /* RT_FEAT_CLIPPING_MINMAX */

        /* "y" section */
        movpx_ld(Xmm5, Mecx, ctx_RAY_Y)         /* ray_y <- RAY_Y */
        mulps_ld(Xmm5, Mecx, ctx_T_VAL(0))      /* ray_y *= t_rt1 */
        addps_ld(Xmm5, Mecx, ctx_ORG_Y)         /* hit_y += ORG_Y */
        movpx_st(Xmm5, Mecx, ctx_HIT_Y)         /* hit_y -> HIT_Y */
        subps_ld(Xmm5, Mebx, srf_POS_Y)         /* loc_y -= POS_Y */
        movpx_st(Xmm5, Mecx, ctx_NEW_Y)         /* loc_y -> NEW_Y */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

#if RT_FEAT_TRANSFORM

        cmpxx_mi(Mebx, srf_A_MAP(RT_W * 4), IB(0))
        jeqxx_lb(CY_trm)

        movpx_ld(Xmm5, Mecx, ctx_RAY_J)         /* ray_j <- RAY_J */
        mulps_ld(Xmm5, Mecx, ctx_T_VAL(0))      /* ray_j *= t_rt1 */
        subps_ld(Xmm5, Mecx, ctx_DFF_J)         /* ray_j -= DFF_J */
        movpx_st(Xmm5, Mecx, ctx_NEW_J)         /* loc_j -> NEW_J */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

    LBL(CY_trm)

#endif /* RT_FEAT_TRANSFORM */

#if RT_FEAT_CLIPPING_MINMAX

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

#endif /* RT_FEAT_CLIPPING_MINMAX */

        /* "z" section */
        movpx_ld(Xmm6, Mecx, ctx_RAY_Z)         /* ray_z <- RAY_Z */
        mulps_ld(Xmm6, Mecx, ctx_T_VAL(0))      /* ray_z *= t_rt1 */
        addps_ld(Xmm6, Mecx, ctx_ORG_Z)         /* hit_z += ORG_Z */
        movpx_st(Xmm6, Mecx, ctx_HIT_Z)         /* hit_z -> HIT_Z */
        subps_ld(Xmm6, Mebx, srf_POS_Z)         /* loc_z -= POS_Z */
        movpx_st(Xmm6, Mecx, ctx_NEW_Z)         /* loc_z -> NEW_Z */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

#if RT_FEAT_TRANSFORM

        cmpxx_mi(Mebx, srf_A_MAP(RT_W * 4), IB(0))
        jeqxx_lb(CZ_trm)

        movpx_ld(Xmm6, Mecx, ctx_RAY_K)         /* ray_k <- RAY_K */
        mulps_ld(Xmm6, Mecx, ctx_T_VAL(0))      /* ray_k *= t_rt1 */
        subps_ld(Xmm6, Mecx, ctx_DFF_K)         /* ray_k -= DFF_K */
        movpx_st(Xmm6, Mecx, ctx_NEW_K)         /* loc_k -> NEW_K */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

    LBL(CZ_trm)

#endif /* RT_FEAT_TRANSFORM */

#if RT_FEAT_CLIPPING_MINMAX

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

        cmpxx_mi(Mebx, srf_A_MAP(RT_W * 4), IB(0))
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
        cmpxx_mi(Mebx, srf_A_MAP(RT_W * 4), IB(1))
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

        cmpxx_mi(Mebx, srf_A_MAP(RT_W * 4), IB(0))
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
        cmpxx_mi(Mebx, srf_A_MAP(RT_W * 4), IB(1))
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
        cmpxx_mi(Mebx, srf_A_MAP(RT_W * 4), IB(2))
        jeqxx_lb(MT_rnm)

    LBL(MT_trn)

        /* renormalize normal */

        movpx_rr(Xmm1, Xmm4)
        mulps_rr(Xmm1, Xmm4)

        movpx_rr(Xmm2, Xmm5)
        mulps_rr(Xmm2, Xmm5)

        movpx_rr(Xmm3, Xmm6)
        mulps_rr(Xmm3, Xmm6)

        addps_rr(Xmm1, Xmm2)
        addps_rr(Xmm1, Xmm3)

        rsqps_rr(Xmm0, Xmm1)

        mulps_rr(Xmm4, Xmm0)
        mulps_rr(Xmm5, Xmm0)
        mulps_rr(Xmm6, Xmm0)

    LBL(MT_rnm)

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
/********************************   LIGHTING   ********************************/
/******************************************************************************/

        CHECK_PROP(LT_lgt, RT_PROP_LIGHT)

        jmpxx_lb(LT_set)

    LBL(LT_lgt)

#if RT_FEAT_LIGHTING

        movxx_ld(Redx, Mebp, inf_CAM)

        movpx_ld(Xmm0, Mecx, ctx_TEX_R)
        mulps_ld(Xmm0, Medx, cam_COL_R)
        movpx_st(Xmm0, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_amR, COL_R)

        movpx_ld(Xmm0, Mecx, ctx_TEX_G)
        mulps_ld(Xmm0, Medx, cam_COL_G)
        movpx_st(Xmm0, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_amG, COL_G)

        movpx_ld(Xmm0, Mecx, ctx_TEX_B)
        mulps_ld(Xmm0, Medx, cam_COL_B)
        movpx_st(Xmm0, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_amB, COL_B)

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
        andpx_rr(Xmm0, Xmm7)                    /* r_dot &= lmask */

        xorpx_rr(Xmm6, Xmm6)                    /* init shadow mask */
        ceqps_rr(Xmm7, Xmm6)                    /* with inverted lmask */

#if RT_FEAT_SHADOWS

        movpx_st(Xmm0, Mecx, ctx_C_PTR(0))      /* save dot product */

/************************************ ENTER ***********************************/

        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))
        orrxx_ri(Reax, IB(RT_FLAG_SHAD | RT_FLAG_PASS_BACK))
        addxx_ri(Recx, IH(RT_STACK_STEP))
        subxx_mi(Mebp, inf_DEPTH, IB(1))

        movxx_st(Reax, Mecx, ctx_PARAM(FLG))    /* context flags */
        movxx_st(Redi, Mecx, ctx_PARAM(LST))    /* save light/shadow list */
        movxx_st(Rebx, Mecx, ctx_PARAM(OBJ))    /* originating surface */
        adrxx_lb(LT_ret)
        movxx_st(Reax, Mecx, ctx_PARAM(PTR))    /* return ptr */

        movpx_ld(Xmm0, Medx, lgt_T_MAX)         /* tmp_v <- T_MAX */
        movpx_st(Xmm0, Mecx, ctx_T_BUF(0))      /* tmp_v -> T_BUF */

        xorpx_rr(Xmm0, Xmm0)                    /* tmp_v <-     0 */
        movpx_st(Xmm7, Mecx, ctx_C_BUF(0))      /* lmask -> C_BUF */
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

        movpx_ld(Xmm7, Mecx, ctx_C_BUF(0))      /* load shadow mask */

        addxx_mi(Mebp, inf_DEPTH, IB(1))
        subxx_ri(Recx, IH(RT_STACK_STEP))

/************************************ LEAVE ***********************************/

        movpx_ld(Xmm0, Mecx, ctx_C_PTR(0))      /* restore dot product */

#endif /* RT_FEAT_SHADOWS */

        CHECK_MASK(LT_amb, FULL, Xmm7)

        /* compute diffuse */

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

#if RT_FEAT_ATTENUATION

        movpx_rr(Xmm6, Xmm4)                    /* Xmm6  <-   r^2 */

#endif /* RT_FEAT_ATTENUATION */

        rsqps_rr(Xmm5, Xmm4)                    /* Xmm5  <-   1/r */

#if RT_FEAT_ATTENUATION

        /* compute attenuation */

        movpx_rr(Xmm4, Xmm5)                    /* Xmm4  <-   1/r */
        mulps_rr(Xmm4, Xmm6)                    /* Xmm4  <-   r^1 */

        movxx_ld(Redx, Medi, elm_SIMD)

        mulps_ld(Xmm6, Medx, lgt_A_QDR)
        mulps_ld(Xmm4, Medx, lgt_A_LNR)
        addps_ld(Xmm6, Medx, lgt_A_CNT)
        addps_rr(Xmm6, Xmm4)
        rsqps_rr(Xmm4, Xmm6)                    /* Xmm4  <-   1/a */

#endif /* RT_FEAT_ATTENUATION */

        movpx_rr(Xmm6, Xmm0)

#if RT_FEAT_ATTENUATION

        mulps_rr(Xmm0, Xmm4)                    /* r_dot *=   1/a */

#endif /* RT_FEAT_ATTENUATION */

        mulps_rr(Xmm0, Xmm5)                    /* r_dot *=   1/r */

        FETCH_XPTR(Redx, MAT_P(PTR))

        mulps_ld(Xmm0, Medx, mat_L_DFF)

        movpx_rr(Xmm4, Xmm6)
        movpx_rr(Xmm5, Xmm6)

#if RT_FEAT_SPECULAR

        CHECK_PROP(LT_spc, RT_PROP_SPECULAR)

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
        andpx_rr(Xmm1, Xmm2)                    /* r_dot &= lmask */

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

        adrxx_lb(LT_pw4)
        movxx_st(Reax, Mebp, inf_POW_E4)

        adrxx_lb(LT_pw3)
        movxx_st(Reax, Mebp, inf_POW_E3)

        adrxx_lb(LT_pw2)
        movxx_st(Reax, Mebp, inf_POW_E2)

        adrxx_lb(LT_pw1)
        movxx_st(Reax, Mebp, inf_POW_E1)

        adrxx_lb(LT_pw0)
        movxx_st(Reax, Mebp, inf_POW_E0)

        adrxx_lb(LT_pwn)
        movxx_st(Reax, Mebp, inf_POW_EN)

        jmpxx_lb(fetch_PL_ptr)

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
        movpx_ld(Xmm1, Medx, mat_C_ONE)

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

#endif /* RT_FEAT_SPECULAR */

        annpx_rr(Xmm7, Xmm0)

        /* apply lighting to metal color,
         * only affects diffuse-specular blending */

        movxx_ld(Redx, Medi, elm_SIMD)

        movpx_ld(Xmm0, Mecx, ctx_TEX_R)
        mulps_ld(Xmm0, Medx, lgt_COL_R)
        mulps_rr(Xmm0, Xmm7)
        addps_ld(Xmm0, Mecx, ctx_COL_R(0))
        movpx_st(Xmm0, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_mcR, COL_R)

        movpx_ld(Xmm0, Mecx, ctx_TEX_G)
        mulps_ld(Xmm0, Medx, lgt_COL_G)
        mulps_rr(Xmm0, Xmm7)
        addps_ld(Xmm0, Mecx, ctx_COL_G(0))
        movpx_st(Xmm0, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_mcG, COL_G)

        movpx_ld(Xmm0, Mecx, ctx_TEX_B)
        mulps_ld(Xmm0, Medx, lgt_COL_B)
        mulps_rr(Xmm0, Xmm7)
        addps_ld(Xmm0, Mecx, ctx_COL_B(0))
        movpx_st(Xmm0, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_mcB, COL_B)

        jmpxx_lb(LT_amb)

#if RT_FEAT_SPECULAR

    LBL(LT_mtl)

        movpx_rr(Xmm6, Xmm7)
        annpx_rr(Xmm7, Xmm0)
        annpx_rr(Xmm6, Xmm1)

        movxx_ld(Redx, Mebp, inf_CAM)
        mulps_ld(Xmm6, Medx, cam_CLAMP)

        /* apply lighting to plain color,
         * only affects diffuse-specular blending */

        movxx_ld(Redx, Medi, elm_SIMD)

        movpx_ld(Xmm0, Mecx, ctx_TEX_R)
        movpx_ld(Xmm1, Medx, lgt_COL_R)
        mulps_rr(Xmm0, Xmm7)
        mulps_rr(Xmm0, Xmm1)
        mulps_rr(Xmm1, Xmm6)
        addps_rr(Xmm0, Xmm1)
        addps_ld(Xmm0, Mecx, ctx_COL_R(0))
        movpx_st(Xmm0, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_pcR, COL_R)

        movpx_ld(Xmm0, Mecx, ctx_TEX_G)
        movpx_ld(Xmm1, Medx, lgt_COL_G)
        mulps_rr(Xmm0, Xmm7)
        mulps_rr(Xmm0, Xmm1)
        mulps_rr(Xmm1, Xmm6)
        addps_rr(Xmm0, Xmm1)
        addps_ld(Xmm0, Mecx, ctx_COL_G(0))
        movpx_st(Xmm0, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_pcG, COL_G)

        movpx_ld(Xmm0, Mecx, ctx_TEX_B)
        movpx_ld(Xmm1, Medx, lgt_COL_B)
        mulps_rr(Xmm0, Xmm7)
        mulps_rr(Xmm0, Xmm1)
        mulps_rr(Xmm1, Xmm6)
        addps_rr(Xmm0, Xmm1)
        addps_ld(Xmm0, Mecx, ctx_COL_B(0))
        movpx_st(Xmm0, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_pcB, COL_B)

#endif /* RT_FEAT_SPECULAR */

    LBL(LT_amb)

        movxx_ld(Redi, Medi, elm_NEXT)
        jmpxx_lb(LT_cyc)

#endif /* RT_FEAT_LIGHTING */

    LBL(LT_set)

        movpx_ld(Xmm0, Mecx, ctx_TEX_R)
        movpx_st(Xmm0, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_txR, COL_R)

        movpx_ld(Xmm0, Mecx, ctx_TEX_G)
        movpx_st(Xmm0, Mecx, ctx_C_PTR(0))
        STORE_SIMD(LT_txG, COL_G)

        movpx_ld(Xmm0, Mecx, ctx_TEX_B)
        movpx_st(Xmm0, Mecx, ctx_C_PTR(0))
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

        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))
        orrxx_ri(Reax, IB(RT_FLAG_PASS_BACK))
        addxx_ri(Recx, IH(RT_STACK_STEP))
        subxx_mi(Mebp, inf_DEPTH, IB(1))

        movxx_st(Reax, Mecx, ctx_PARAM(FLG))    /* context flags */
        movxx_st(Redx, Mecx, ctx_PARAM(LST))    /* save material */
        movxx_st(Rebx, Mecx, ctx_PARAM(OBJ))    /* originating surface */
        adrxx_lb(RF_ret)
        movxx_st(Reax, Mecx, ctx_PARAM(PTR))    /* return pointer */

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
        mulps_rr(Xmm1, Xmm0)

        movpx_ld(Xmm2, Mecx, ctx_COL_G(0))
        mulps_rr(Xmm2, Xmm0)

        movpx_ld(Xmm3, Mecx, ctx_COL_B(0))
        mulps_rr(Xmm3, Xmm0)

        addxx_mi(Mebp, inf_DEPTH, IB(1))
        subxx_ri(Recx, IH(RT_STACK_STEP))

/************************************ LEAVE ***********************************/

    LBL(RF_mix)

        movpx_ld(Xmm0, Medx, mat_C_ONE)
        subps_ld(Xmm0, Medx, mat_C_RFL)

        movpx_ld(Xmm4, Mecx, ctx_COL_R(0))
        mulps_rr(Xmm4, Xmm0)
        addps_rr(Xmm1, Xmm4)
        movpx_st(Xmm1, Mecx, ctx_C_PTR(0))
        STORE_SIMD(RF_clR, COL_R)

        movpx_ld(Xmm5, Mecx, ctx_COL_G(0))
        mulps_rr(Xmm5, Xmm0)
        addps_rr(Xmm2, Xmm5)
        movpx_st(Xmm2, Mecx, ctx_C_PTR(0))
        STORE_SIMD(RF_clG, COL_G)

        movpx_ld(Xmm6, Mecx, ctx_COL_B(0))
        mulps_rr(Xmm6, Xmm0)
        addps_rr(Xmm3, Xmm6)
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
        addps_ld(Xmm7, Medx, mat_C_ONE)
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

        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))
        orrxx_ri(Reax, IB(RT_FLAG_PASS_THRU))
        addxx_ri(Recx, IH(RT_STACK_STEP))
        subxx_mi(Mebp, inf_DEPTH, IB(1))

        movxx_st(Reax, Mecx, ctx_PARAM(FLG))    /* context flags */
        movxx_st(Redx, Mecx, ctx_PARAM(LST))    /* save material */
        movxx_st(Rebx, Mecx, ctx_PARAM(OBJ))    /* originating surface */
        adrxx_lb(TR_ret)
        movxx_st(Reax, Mecx, ctx_PARAM(PTR))    /* return pointer */

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
        mulps_rr(Xmm1, Xmm0)

        movpx_ld(Xmm2, Mecx, ctx_COL_G(0))
        mulps_rr(Xmm2, Xmm0)

        movpx_ld(Xmm3, Mecx, ctx_COL_B(0))
        mulps_rr(Xmm3, Xmm0)

        addxx_mi(Mebp, inf_DEPTH, IB(1))
        subxx_ri(Recx, IH(RT_STACK_STEP))

/************************************ LEAVE ***********************************/

    LBL(TR_mix)

        movpx_ld(Xmm0, Medx, mat_C_ONE)
        subps_ld(Xmm0, Medx, mat_C_TRN)

        movpx_ld(Xmm4, Mecx, ctx_COL_R(0))
        mulps_rr(Xmm4, Xmm0)
        addps_rr(Xmm1, Xmm4)
        movpx_st(Xmm1, Mecx, ctx_C_PTR(0))
        STORE_SIMD(TR_clR, COL_R)

        movpx_ld(Xmm5, Mecx, ctx_COL_G(0))
        mulps_rr(Xmm5, Xmm0)
        addps_rr(Xmm2, Xmm5)
        movpx_st(Xmm2, Mecx, ctx_C_PTR(0))
        STORE_SIMD(TR_clG, COL_G)

        movpx_ld(Xmm6, Mecx, ctx_COL_B(0))
        mulps_rr(Xmm6, Xmm0)
        addps_rr(Xmm3, Xmm6)
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
/**********************************   PLANE   *********************************/
/******************************************************************************/

    LBL(fetch_PL_ptr)

        adrxx_lb(PL_ptr)
        movxx_st(Reax, Mebp, inf_PTR_PL)
        jmpxx_lb(fetch_PL_clp)

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
    LBL(PL_mat)

        FETCH_PROP()                            /* Xmm7  <- ssign */

#if RT_FEAT_SHADOWS

        CHECK_SHAD(PL_shd)

#endif /* RT_FEAT_SHADOWS */

#if RT_FEAT_TEXTURING

        CHECK_PROP(PL_tex, RT_PROP_TEXTURE)

        /* compute surface's UV coords
         * for texturing */

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

        CHECK_PROP(PL_nrm, RT_PROP_NORMAL)

        /* compute normal */

        INDEX_AXIS(RT_I)                        /* eax   <-     i */
        MOVZR_ST(Xmm4, Iecx, ctx_NRM_O)         /* 0     -> NRM_I */

        INDEX_AXIS(RT_J)                        /* eax   <-     j */
        MOVZR_ST(Xmm5, Iecx, ctx_NRM_O)         /* 0     -> NRM_J */

        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        movpx_ld(Xmm6, Mebx, xpl_NRM_K)         /* tmp_v <-     1 */
        xorpx_rr(Xmm6, Xmm7)                    /* tmp_v ^= ssign */
        MOVXR_ST(Xmm6, Iecx, ctx_NRM_O)         /* tmp_v -> NRM_K */

        jmpxx_lb(MT_nrm)

    LBL(PL_nrm)

#endif /* RT_FEAT_NORMALS */

        jmpxx_lb(MT_mat)

/******************************************************************************/
    LBL(fetch_PL_clp)

        adrxx_lb(PL_clp)
        movxx_st(Reax, Mebp, inf_CLP_PL)
        jmpxx_lb(fetch_CL_ptr)

    LBL(PL_clp)

#if RT_FEAT_CLIPPING_CUSTOM

        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        MOVXR_LD(Xmm4, Iecx, ctx_NRM_O)         /* dff_k <- NRM_K */
        xorpx_rr(Xmm6, Xmm6)                    /* tmp_k <-     0 */

        APPLY_CLIP(PL, Xmm4, Xmm6)

        jmpxx_lb(CC_ret)

#endif /* RT_FEAT_CLIPPING_CUSTOM */

/******************************************************************************/
/********************************   CYLINDER   ********************************/
/******************************************************************************/

    LBL(fetch_CL_ptr)

        adrxx_lb(CL_ptr)
        movxx_st(Reax, Mebp, inf_PTR_CL)
        jmpxx_lb(fetch_CL_clp)

    LBL(CL_ptr)

#if RT_SHOW_TILES

        SHOW_TILES(CL, 0x00008800)

#endif /* RT_SHOW_TILES */

        /* "i" section */
        INDEX_AXIS(RT_I)                        /* eax   <-     i */
        MOVXR_LD(Xmm1, Iecx, ctx_RAY_O)         /* ray_i <- RAY_I */
        MOVXR_LD(Xmm5, Iecx, ctx_DFF_O)         /* dff_i <- DFF_I */
        movpx_rr(Xmm3, Xmm1)                    /* ray_i <- ray_i */
        mulps_rr(Xmm3, Xmm5)                    /* ray_i *= dff_i */
        mulps_rr(Xmm1, Xmm1)                    /* ray_i *= ray_i */
        mulps_rr(Xmm5, Xmm5)                    /* dff_i *= dff_i */

        /* "j" section */
        INDEX_AXIS(RT_J)                        /* eax   <-     j */
        MOVXR_LD(Xmm2, Iecx, ctx_RAY_O)         /* ray_j <- RAY_J */
        MOVXR_LD(Xmm6, Iecx, ctx_DFF_O)         /* dff_j <- DFF_J */
        movpx_rr(Xmm4, Xmm2)                    /* ray_j <- ray_j */
        mulps_rr(Xmm4, Xmm6)                    /* ray_j *= dff_j */
        mulps_rr(Xmm2, Xmm2)                    /* ray_j *= ray_j */
        mulps_rr(Xmm6, Xmm6)                    /* dff_j *= dff_j */

        /* "+" section */
        addps_rr(Xmm1, Xmm2)                    /* axx_j += axx_i */
        addps_rr(Xmm3, Xmm4)                    /* bxx_j += bxx_i */
        addps_rr(Xmm5, Xmm6)                    /* cxx_j += cxx_i */

        subps_ld(Xmm5, Mebx, xcl_RAD_2)         /* cxx_t -= RAD_2 */

        /* "d" section */
        mulps_rr(Xmm5, Xmm1)                    /* c_val *= a_val */
        movpx_rr(Xmm2, Xmm3)                    /* b_val <- b_val */
        mulps_rr(Xmm3, Xmm3)                    /* b_val *= b_val */
        subps_rr(Xmm3, Xmm5)                    /* d_bxb -= d_axc */

        /* create xmask */
        xorpx_rr(Xmm7, Xmm7)                    /* d_min <-     0 */
        cleps_rr(Xmm7, Xmm3)                    /* d_min <= d_val */
        CHECK_MASK(OO_end, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_XMASK)         /* xmask -> XMASK */

        /* "tt" section */
        sqrps_rr(Xmm3, Xmm3)                    /* d_val sq d_val */

        movpx_st(Xmm1, Mecx, ctx_XTMP1)         /* a_val -> XTMP1 */
        movpx_st(Xmm2, Mecx, ctx_XTMP2)         /* b_val -> XTMP2 */
        movpx_st(Xmm3, Mecx, ctx_XTMP3)         /* d_val -> XTMP3 */

/******************************************************************************/
/*  LBL(CL_rt1)  */

        /* outer side */
        CHECK_SIDE(CL_sd1, CL_rt2, RT_FLAG_SIDE_OUTER)

        /* "t1" section */
        movpx_rr(Xmm4, Xmm2)                    /* b_val <- b_val */
        subps_rr(Xmm4, Xmm3)                    /* b_val -= d_val */
        divps_rr(Xmm4, Xmm1)                    /* t_rt1 /= a_val */
        movpx_st(Xmm4, Mecx, ctx_T_VAL(0))      /* t_rt1 -> T_VAL */

        /* clipping */
        SUBROUTINE(CL_cp1, CC_clp)
        CHECK_MASK(CL_rt2, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_TMASK(0))      /* tmask -> TMASK */

        /* material */
        movxx_mi(Mecx, ctx_LOCAL(FLG), IB(RT_FLAG_SIDE_OUTER))
        SUBROUTINE(CL_mt1, CL_mat)

        /* optimize overdraw */
        movpx_ld(Xmm7, Mecx, ctx_TMASK(0))      /* tmask <- TMASK */
        CHECK_MASK(OO_end, FULL, Xmm7)

/******************************************************************************/
    LBL(CL_rt2)

        /* inner side */
        CHECK_SIDE(CL_sd2, OO_end, RT_FLAG_SIDE_INNER)

        /* "t2" section */
        movpx_ld(Xmm4, Mecx, ctx_XTMP2)         /* b_val <- b_val */
        addps_ld(Xmm4, Mecx, ctx_XTMP3)         /* b_val += d_val */
        divps_ld(Xmm4, Mecx, ctx_XTMP1)         /* t_rt2 /= a_val */
        movpx_st(Xmm4, Mecx, ctx_T_VAL(0))      /* t_rt2 -> T_VAL */

        /* clipping */
        movpx_ld(Xmm7, Mecx, ctx_XMASK)         /* xmask <- XMASK */
        SUBROUTINE(CL_cp2, CC_clp)
        CHECK_MASK(OO_end, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_TMASK(0))      /* tmask -> TMASK */

        /* material */
        movxx_mi(Mecx, ctx_LOCAL(FLG), IB(RT_FLAG_SIDE_INNER))
        SUBROUTINE(CL_mt2, CL_mat)

        jmpxx_lb(OO_end)

/******************************************************************************/
    LBL(CL_mat)

        FETCH_PROP()                            /* Xmm7  <- ssign */

#if RT_FEAT_SHADOWS

        CHECK_SHAD(CL_shd)

#endif /* RT_FEAT_SHADOWS */

#if RT_FEAT_NORMALS

        CHECK_PROP(CL_nrm, RT_PROP_NORMAL)

        /* compute normal */

        INDEX_AXIS(RT_I)                        /* eax   <-     i */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        MOVXR_LD(Xmm4, Iecx, ctx_NEW_O)         /* loc_i <- NEW_I */
        mulps_ld(Xmm4, Mebx, xcl_I_RAD)         /* loc_i *= i_rad */
        xorpx_rr(Xmm4, Xmm7)                    /* loc_i ^= ssign */
        MOVXR_ST(Xmm4, Iecx, ctx_NRM_O)         /* nrm_i -> NRM_I */

        INDEX_AXIS(RT_J)                        /* eax   <-     j */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        MOVXR_LD(Xmm5, Iecx, ctx_NEW_O)         /* loc_j <- NEW_J */
        mulps_ld(Xmm5, Mebx, xcl_I_RAD)         /* loc_j *= i_rad */
        xorpx_rr(Xmm5, Xmm7)                    /* loc_j ^= ssign */
        MOVXR_ST(Xmm5, Iecx, ctx_NRM_O)         /* nrm_j -> NRM_J */

        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        MOVZR_ST(Xmm6, Iecx, ctx_NRM_O)         /* 0     -> NRM_K */

        jmpxx_lb(MT_nrm)

    LBL(CL_nrm)

#endif /* RT_FEAT_NORMALS */

        jmpxx_lb(MT_mat)

/******************************************************************************/
    LBL(fetch_CL_clp)

        adrxx_lb(CL_clp)
        movxx_st(Reax, Mebp, inf_CLP_CL)
        jmpxx_lb(fetch_SP_ptr)

    LBL(CL_clp)

#if RT_FEAT_CLIPPING_CUSTOM

        INDEX_AXIS(RT_I)                        /* eax   <-     i */
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        MOVXR_LD(Xmm4, Iecx, ctx_NRM_O)         /* dff_i <- NRM_I */
        mulps_rr(Xmm4, Xmm4)                    /* dff_i *= dff_i */

        INDEX_AXIS(RT_J)                        /* eax   <-     j */
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        MOVXR_LD(Xmm5, Iecx, ctx_NRM_O)         /* dff_j <- NRM_J */
        mulps_rr(Xmm5, Xmm5)                    /* dff_j *= dff_j */

        addps_rr(Xmm4, Xmm5)
        movpx_ld(Xmm6, Mebx, xcl_RAD_2)

        APPLY_CLIP(CL, Xmm4, Xmm6)

        jmpxx_lb(CC_ret)

#endif /* RT_FEAT_CLIPPING_CUSTOM */

/******************************************************************************/
/*********************************   SPHERE   *********************************/
/******************************************************************************/

    LBL(fetch_SP_ptr)

        adrxx_lb(SP_ptr)
        movxx_st(Reax, Mebp, inf_PTR_SP)
        jmpxx_lb(fetch_SP_clp)

    LBL(SP_ptr)

#if RT_SHOW_TILES

        SHOW_TILES(SP, 0x00000088)

#endif /* RT_SHOW_TILES */

        /* "i" section */
        INDEX_AXIS(RT_I)                        /* eax   <-     i */
        MOVXR_LD(Xmm1, Iecx, ctx_RAY_O)         /* ray_i <- RAY_I */
        MOVXR_LD(Xmm5, Iecx, ctx_DFF_O)         /* dff_i <- DFF_I */
        movpx_rr(Xmm3, Xmm1)                    /* ray_i <- ray_i */
        mulps_rr(Xmm3, Xmm5)                    /* ray_i *= dff_i */
        mulps_rr(Xmm1, Xmm1)                    /* ray_i *= ray_i */
        mulps_rr(Xmm5, Xmm5)                    /* dff_i *= dff_i */

        /* "j" section */
        INDEX_AXIS(RT_J)                        /* eax   <-     j */
        MOVXR_LD(Xmm2, Iecx, ctx_RAY_O)         /* ray_j <- RAY_J */
        MOVXR_LD(Xmm6, Iecx, ctx_DFF_O)         /* dff_j <- DFF_J */
        movpx_rr(Xmm4, Xmm2)                    /* ray_j <- ray_j */
        mulps_rr(Xmm4, Xmm6)                    /* ray_j *= dff_j */
        mulps_rr(Xmm2, Xmm2)                    /* ray_j *= ray_j */
        mulps_rr(Xmm6, Xmm6)                    /* dff_j *= dff_j */

        /* "+" section */
        addps_rr(Xmm1, Xmm2)                    /* axx_i += axx_j */
        addps_rr(Xmm3, Xmm4)                    /* bxx_i += bxx_j */
        addps_rr(Xmm5, Xmm6)                    /* cxx_i += cxx_j */

        /* "k" section */
        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        MOVXR_LD(Xmm2, Iecx, ctx_RAY_O)         /* ray_k <- RAY_K */
        MOVXR_LD(Xmm6, Iecx, ctx_DFF_O)         /* dff_k <- DFF_K */
        movpx_rr(Xmm4, Xmm2)                    /* ray_k <- ray_k */
        mulps_rr(Xmm4, Xmm6)                    /* ray_k *= dff_k */
        mulps_rr(Xmm2, Xmm2)                    /* ray_k *= ray_k */
        mulps_rr(Xmm6, Xmm6)                    /* dff_k *= dff_k */

        /* "+" section */
        addps_rr(Xmm1, Xmm2)                    /* axx_t += axx_k */
        addps_rr(Xmm3, Xmm4)                    /* bxx_t += bxx_k */
        addps_rr(Xmm5, Xmm6)                    /* cxx_t += cxx_k */

        subps_ld(Xmm5, Mebx, xsp_RAD_2)         /* cxx_t -= RAD_2 */

        /* "d" section */
        mulps_rr(Xmm5, Xmm1)                    /* c_val *= a_val */
        movpx_rr(Xmm2, Xmm3)                    /* b_val <- b_val */
        mulps_rr(Xmm3, Xmm3)                    /* b_val *= b_val */
        subps_rr(Xmm3, Xmm5)                    /* d_bxb -= d_axc */

        /* create xmask */
        xorpx_rr(Xmm7, Xmm7)                    /* d_min <-     0 */
        cleps_rr(Xmm7, Xmm3)                    /* d_min <= d_val */
        CHECK_MASK(OO_end, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_XMASK)         /* xmask -> XMASK */

        /* "tt" section */
        sqrps_rr(Xmm3, Xmm3)                    /* d_val sq d_val */

        movpx_st(Xmm1, Mecx, ctx_XTMP1)         /* a_val -> XTMP1 */
        movpx_st(Xmm2, Mecx, ctx_XTMP2)         /* b_val -> XTMP2 */
        movpx_st(Xmm3, Mecx, ctx_XTMP3)         /* d_val -> XTMP3 */

/******************************************************************************/
/*  LBL(SP_rt1)  */

        /* outer side */
        CHECK_SIDE(SP_sd1, SP_rt2, RT_FLAG_SIDE_OUTER)

        /* "t1" section */
        movpx_rr(Xmm4, Xmm2)                    /* b_val <- b_val */
        subps_rr(Xmm4, Xmm3)                    /* b_val -= d_val */
        divps_rr(Xmm4, Xmm1)                    /* t_rt1 /= a_val */
        movpx_st(Xmm4, Mecx, ctx_T_VAL(0))      /* t_rt1 -> T_VAL */

        /* clipping */
        SUBROUTINE(SP_cp1, CC_clp)
        CHECK_MASK(SP_rt2, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_TMASK(0))      /* tmask -> TMASK */

        /* material */
        movxx_mi(Mecx, ctx_LOCAL(FLG), IB(RT_FLAG_SIDE_OUTER))
        SUBROUTINE(SP_mt1, SP_mat)

        /* optimize overdraw */
        movpx_ld(Xmm7, Mecx, ctx_TMASK(0))      /* tmask <- TMASK */
        CHECK_MASK(OO_end, FULL, Xmm7)

/******************************************************************************/
    LBL(SP_rt2)

        /* inner side */
        CHECK_SIDE(SP_sd2, OO_end, RT_FLAG_SIDE_INNER)

        /* "t2" section */
        movpx_ld(Xmm4, Mecx, ctx_XTMP2)         /* b_val <- b_val */
        addps_ld(Xmm4, Mecx, ctx_XTMP3)         /* b_val += d_val */
        divps_ld(Xmm4, Mecx, ctx_XTMP1)         /* t_rt2 /= a_val */
        movpx_st(Xmm4, Mecx, ctx_T_VAL(0))      /* t_rt2 -> T_VAL */

        /* clipping */
        movpx_ld(Xmm7, Mecx, ctx_XMASK)         /* xmask <- XMASK */
        SUBROUTINE(SP_cp2, CC_clp)
        CHECK_MASK(OO_end, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_TMASK(0))      /* tmask -> TMASK */

        /* material */
        movxx_mi(Mecx, ctx_LOCAL(FLG), IB(RT_FLAG_SIDE_INNER))
        SUBROUTINE(SP_mt2, SP_mat)

        jmpxx_lb(OO_end)

/******************************************************************************/
    LBL(SP_mat)

        FETCH_PROP()                            /* Xmm7  <- ssign */

#if RT_FEAT_SHADOWS

        CHECK_SHAD(SP_shd)

#endif /* RT_FEAT_SHADOWS */

#if RT_FEAT_NORMALS

        CHECK_PROP(SP_nrm, RT_PROP_NORMAL)

        /* compute normal */

        INDEX_AXIS(RT_I)                        /* eax   <-     i */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        MOVXR_LD(Xmm4, Iecx, ctx_NEW_O)         /* loc_i <- NEW_I */
        mulps_ld(Xmm4, Mebx, xsp_I_RAD)         /* loc_i *= i_rad */
        xorpx_rr(Xmm4, Xmm7)                    /* loc_i ^= ssign */
        MOVXR_ST(Xmm4, Iecx, ctx_NRM_O)         /* nrm_i -> NRM_I */

        INDEX_AXIS(RT_J)                        /* eax   <-     j */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        MOVXR_LD(Xmm5, Iecx, ctx_NEW_O)         /* loc_j <- NEW_J */
        mulps_ld(Xmm5, Mebx, xsp_I_RAD)         /* loc_j *= i_rad */
        xorpx_rr(Xmm5, Xmm7)                    /* loc_j ^= ssign */
        MOVXR_ST(Xmm5, Iecx, ctx_NRM_O)         /* nrm_j -> NRM_J */

        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        MOVXR_LD(Xmm6, Iecx, ctx_NEW_O)         /* loc_k <- NEW_K */
        mulps_ld(Xmm6, Mebx, xsp_I_RAD)         /* loc_k *= i_rad */
        xorpx_rr(Xmm6, Xmm7)                    /* loc_k ^= ssign */
        MOVXR_ST(Xmm6, Iecx, ctx_NRM_O)         /* nrm_k -> NRM_K */

        jmpxx_lb(MT_nrm)

    LBL(SP_nrm)

#endif /* RT_FEAT_NORMALS */

        jmpxx_lb(MT_mat)

/******************************************************************************/
    LBL(fetch_SP_clp)

        adrxx_lb(SP_clp)
        movxx_st(Reax, Mebp, inf_CLP_SP)
        jmpxx_lb(fetch_CN_ptr)

    LBL(SP_clp)

#if RT_FEAT_CLIPPING_CUSTOM

        INDEX_AXIS(RT_I)                        /* eax   <-     i */
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        MOVXR_LD(Xmm4, Iecx, ctx_NRM_O)         /* dff_i <- NRM_I */
        mulps_rr(Xmm4, Xmm4)                    /* dff_i *= dff_i */

        INDEX_AXIS(RT_J)                        /* eax   <-     j */
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        MOVXR_LD(Xmm5, Iecx, ctx_NRM_O)         /* dff_j <- NRM_J */
        mulps_rr(Xmm5, Xmm5)                    /* dff_j *= dff_j */

        addps_rr(Xmm4, Xmm5)

        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        MOVXR_LD(Xmm6, Iecx, ctx_NRM_O)         /* dff_k <- NRM_K */
        mulps_rr(Xmm6, Xmm6)                    /* dff_k *= dff_k */

        addps_rr(Xmm4, Xmm6)
        movpx_ld(Xmm6, Mebx, xsp_RAD_2)

        APPLY_CLIP(SP, Xmm4, Xmm6)

        jmpxx_lb(CC_ret)

#endif /* RT_FEAT_CLIPPING_CUSTOM */

/******************************************************************************/
/**********************************   CONE   **********************************/
/******************************************************************************/

    LBL(fetch_CN_ptr)

        adrxx_lb(CN_ptr)
        movxx_st(Reax, Mebp, inf_PTR_CN)
        jmpxx_lb(fetch_CN_clp)

    LBL(CN_ptr)

#if RT_SHOW_TILES

        SHOW_TILES(CN, 0x00008888)

#endif /* RT_SHOW_TILES */

        /* "i" section */
        INDEX_AXIS(RT_I)                        /* eax   <-     i */
        MOVXR_LD(Xmm1, Iecx, ctx_RAY_O)         /* ray_i <- RAY_I */
        MOVXR_LD(Xmm5, Iecx, ctx_DFF_O)         /* dff_i <- DFF_I */
        movpx_rr(Xmm3, Xmm1)                    /* ray_i <- ray_i */
        mulps_rr(Xmm3, Xmm5)                    /* ray_i *= dff_i */
        mulps_rr(Xmm1, Xmm1)                    /* ray_i *= ray_i */
        mulps_rr(Xmm5, Xmm5)                    /* dff_i *= dff_i */

        /* "j" section */
        INDEX_AXIS(RT_J)                        /* eax   <-     j */
        MOVXR_LD(Xmm2, Iecx, ctx_RAY_O)         /* ray_j <- RAY_J */
        MOVXR_LD(Xmm6, Iecx, ctx_DFF_O)         /* dff_j <- DFF_J */
        movpx_rr(Xmm4, Xmm2)                    /* ray_j <- ray_j */
        mulps_rr(Xmm4, Xmm6)                    /* ray_j *= dff_j */
        mulps_rr(Xmm2, Xmm2)                    /* ray_j *= ray_j */
        mulps_rr(Xmm6, Xmm6)                    /* dff_j *= dff_j */

        /* "+" section */
        addps_rr(Xmm1, Xmm2)                    /* axx_i += axx_j */
        addps_rr(Xmm3, Xmm4)                    /* bxx_i += bxx_j */
        addps_rr(Xmm5, Xmm6)                    /* cxx_i += cxx_j */

        /* "k" section */
        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        MOVXR_LD(Xmm2, Iecx, ctx_RAY_O)         /* ray_k <- RAY_K */
        MOVXR_LD(Xmm6, Iecx, ctx_DFF_O)         /* dff_k <- DFF_K */
        movpx_rr(Xmm4, Xmm2)                    /* ray_k <- ray_k */
        mulps_rr(Xmm4, Xmm6)                    /* ray_k *= dff_k */
        mulps_rr(Xmm2, Xmm2)                    /* ray_k *= ray_k */
        mulps_rr(Xmm6, Xmm6)                    /* dff_k *= dff_k */
        movpx_ld(Xmm7, Mebx, xcn_RAT_2)         /* rat_2 <- RAT_2 */
        mulps_rr(Xmm4, Xmm7)                    /* bxx_k *= rat_2 */
        mulps_rr(Xmm2, Xmm7)                    /* axx_k *= rat_2 */
        mulps_rr(Xmm6, Xmm7)                    /* cxx_k *= rat_2 */

        /* "-" section */
        subps_rr(Xmm1, Xmm2)                    /* axx_t -= axx_k */
        subps_rr(Xmm3, Xmm4)                    /* bxx_t -= bxx_k */
        subps_rr(Xmm5, Xmm6)                    /* cxx_t -= cxx_k */

        /* "d" section */
        mulps_rr(Xmm5, Xmm1)                    /* c_val *= a_val */
        movpx_rr(Xmm2, Xmm3)                    /* b_val <- b_val */
        mulps_rr(Xmm3, Xmm3)                    /* b_val *= b_val */
        subps_rr(Xmm3, Xmm5)                    /* d_bxb -= d_axc */

        /* create xmask */
        xorpx_rr(Xmm7, Xmm7)                    /* d_min <-     0 */
        cltps_rr(Xmm7, Xmm3)                    /* d_min <! d_val */
        CHECK_MASK(OO_end, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_XMASK)         /* xmask -> XMASK */

        /* "tt" section */
        sqrps_rr(Xmm3, Xmm3)                    /* d_val sq d_val */

        movpx_st(Xmm1, Mecx, ctx_XTMP1)         /* a_val -> XTMP1 */
        movpx_st(Xmm2, Mecx, ctx_XTMP2)         /* b_val -> XTMP2 */
        movpx_st(Xmm3, Mecx, ctx_XTMP3)         /* d_val -> XTMP3 */

/******************************************************************************/
/*  LBL(CN_rt1)  */

        /* outer side */
        CHECK_SIDE(CN_sd1, CN_rt2, RT_FLAG_SIDE_OUTER)

        /* "t1" section */
        movpx_rr(Xmm4, Xmm2)                    /* b_val <- b_val */
        subps_rr(Xmm4, Xmm3)                    /* b_val -= d_val */
        divps_rr(Xmm4, Xmm1)                    /* t_rt1 /= a_val */
        movpx_st(Xmm4, Mecx, ctx_T_VAL(0))      /* t_rt1 -> T_VAL */

        /* clipping */
        SUBROUTINE(CN_cp1, CC_clp)
        CHECK_MASK(CN_rt2, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_TMASK(0))      /* tmask -> TMASK */

        /* material */
        movxx_mi(Mecx, ctx_LOCAL(FLG), IB(RT_FLAG_SIDE_OUTER))
        SUBROUTINE(CN_mt1, CN_mat)

/******************************************************************************/
    LBL(CN_rt2)

        /* inner side */
        CHECK_SIDE(CN_sd2, OO_end, RT_FLAG_SIDE_INNER)

        /* "t2" section */
        movpx_ld(Xmm4, Mecx, ctx_XTMP2)         /* b_val <- b_val */
        addps_ld(Xmm4, Mecx, ctx_XTMP3)         /* b_val += d_val */
        divps_ld(Xmm4, Mecx, ctx_XTMP1)         /* t_rt2 /= a_val */
        movpx_st(Xmm4, Mecx, ctx_T_VAL(0))      /* t_rt2 -> T_VAL */

        /* clipping */
        movpx_ld(Xmm7, Mecx, ctx_XMASK)         /* xmask <- XMASK */
        SUBROUTINE(CN_cp2, CC_clp)
        CHECK_MASK(OO_end, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_TMASK(0))      /* tmask -> TMASK */

        /* material */
        movxx_mi(Mecx, ctx_LOCAL(FLG), IB(RT_FLAG_SIDE_INNER))
        SUBROUTINE(CN_mt2, CN_mat)

        jmpxx_lb(OO_end)

/******************************************************************************/
    LBL(CN_mat)

        FETCH_PROP()                            /* Xmm7  <- ssign */

#if RT_FEAT_SHADOWS

        CHECK_SHAD(CN_shd)

#endif /* RT_FEAT_SHADOWS */

#if RT_FEAT_NORMALS

        CHECK_PROP(CN_nrm, RT_PROP_NORMAL)

        /* compute normal */

        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        MOVXR_LD(Xmm6, Iecx, ctx_NEW_O)         /* loc_k <- NEW_K */
        movpx_ld(Xmm1, Mebx, srf_SMASK)         /* smask <- SMASK */
        xorpx_rr(Xmm6, Xmm1)                    /* loc_k = -loc_k */
        annpx_rr(Xmm1, Xmm6)                    /* tmp_v = |loc_k|*/
        movpx_ld(Xmm3, Mebx, xcn_I_RAT)         /* i_rat <- I_RAT */
        divps_rr(Xmm3, Xmm1)                    /* i_rat /= tmp_v */
        mulps_ld(Xmm6, Mebx, xcn_RAT_2)         /* loc_k *= rat_2 */
        mulps_rr(Xmm6, Xmm3)                    /* loc_k *= i_rat */
        xorpx_rr(Xmm6, Xmm7)                    /* loc_k ^= ssign */
        MOVXR_ST(Xmm6, Iecx, ctx_NRM_O)         /* nrm_k -> NRM_K */

        INDEX_AXIS(RT_I)                        /* eax   <-     i */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        MOVXR_LD(Xmm4, Iecx, ctx_NEW_O)         /* loc_i <- NEW_I */
        mulps_rr(Xmm4, Xmm3)                    /* loc_i *= i_rat */
        xorpx_rr(Xmm4, Xmm7)                    /* loc_i ^= ssign */
        MOVXR_ST(Xmm4, Iecx, ctx_NRM_O)         /* nrm_i -> NRM_I */

        INDEX_AXIS(RT_J)                        /* eax   <-     j */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        MOVXR_LD(Xmm5, Iecx, ctx_NEW_O)         /* loc_j <- NEW_J */
        mulps_rr(Xmm5, Xmm3)                    /* loc_j *= i_rat */
        xorpx_rr(Xmm5, Xmm7)                    /* loc_j ^= ssign */
        MOVXR_ST(Xmm5, Iecx, ctx_NRM_O)         /* nrm_j -> NRM_J */

        jmpxx_lb(MT_nrm)

    LBL(CN_nrm)

#endif /* RT_FEAT_NORMALS */

        jmpxx_lb(MT_mat)

/******************************************************************************/
    LBL(fetch_CN_clp)

        adrxx_lb(CN_clp)
        movxx_st(Reax, Mebp, inf_CLP_CN)
        jmpxx_lb(fetch_PB_ptr)

    LBL(CN_clp)

#if RT_FEAT_CLIPPING_CUSTOM

        INDEX_AXIS(RT_I)                        /* eax   <-     i */
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        MOVXR_LD(Xmm4, Iecx, ctx_NRM_O)         /* dff_i <- NRM_I */
        mulps_rr(Xmm4, Xmm4)                    /* dff_i *= dff_i */

        INDEX_AXIS(RT_J)                        /* eax   <-     j */
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        MOVXR_LD(Xmm5, Iecx, ctx_NRM_O)         /* dff_j <- NRM_J */
        mulps_rr(Xmm5, Xmm5)                    /* dff_j *= dff_j */

        addps_rr(Xmm4, Xmm5)

        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        MOVXR_LD(Xmm6, Iecx, ctx_NRM_O)         /* dff_k <- NRM_K */
        mulps_rr(Xmm6, Xmm6)                    /* dff_k *= dff_k */
        mulps_ld(Xmm6, Mebx, xcn_RAT_2)

        APPLY_CLIP(CN, Xmm4, Xmm6)

        jmpxx_lb(CC_ret)

#endif /* RT_FEAT_CLIPPING_CUSTOM */

/******************************************************************************/
/*******************************   PARABOLOID   *******************************/
/******************************************************************************/

    LBL(fetch_PB_ptr)

        adrxx_lb(PB_ptr)
        movxx_st(Reax, Mebp, inf_PTR_PB)
        jmpxx_lb(fetch_PB_clp)

    LBL(PB_ptr)

#if RT_SHOW_TILES

        SHOW_TILES(PB, 0x00880088)

#endif /* RT_SHOW_TILES */

        /* "i" section */
        INDEX_AXIS(RT_I)                        /* eax   <-     i */
        MOVXR_LD(Xmm1, Iecx, ctx_RAY_O)         /* ray_i <- RAY_I */
        MOVXR_LD(Xmm5, Iecx, ctx_DFF_O)         /* dff_i <- DFF_I */
        movpx_rr(Xmm3, Xmm1)                    /* ray_i <- ray_i */
        mulps_rr(Xmm3, Xmm5)                    /* ray_i *= dff_i */
        mulps_rr(Xmm1, Xmm1)                    /* ray_i *= ray_i */
        mulps_rr(Xmm5, Xmm5)                    /* dff_i *= dff_i */

        /* "j" section */
        INDEX_AXIS(RT_J)                        /* eax   <-     j */
        MOVXR_LD(Xmm2, Iecx, ctx_RAY_O)         /* ray_j <- RAY_J */
        MOVXR_LD(Xmm6, Iecx, ctx_DFF_O)         /* dff_j <- DFF_J */
        movpx_rr(Xmm4, Xmm2)                    /* ray_j <- ray_j */
        mulps_rr(Xmm4, Xmm6)                    /* ray_j *= dff_j */
        mulps_rr(Xmm2, Xmm2)                    /* ray_j *= ray_j */
        mulps_rr(Xmm6, Xmm6)                    /* dff_j *= dff_j */

        /* "+" section */
        addps_rr(Xmm1, Xmm2)                    /* axx_i += axx_j */
        addps_rr(Xmm3, Xmm4)                    /* bxx_i += bxx_j */
        addps_rr(Xmm5, Xmm6)                    /* cxx_i += cxx_j */

        /* "k" section */
        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        MOVXR_LD(Xmm2, Iecx, ctx_RAY_O)         /* ray_k <- RAY_K */
        MOVXR_LD(Xmm6, Iecx, ctx_DFF_O)         /* dff_k <- DFF_K */
        movpx_ld(Xmm4, Mebx, xpb_PAR_2)         /* par_2 <- PAR_2 */
        mulps_rr(Xmm2, Xmm4)                    /* ray_k *= par_2 */
        addps_rr(Xmm4, Xmm4)                    /* par_2 += par_2 */
        mulps_rr(Xmm6, Xmm4)                    /* dff_k *= par_k */

        /* "+" section */
        addps_rr(Xmm3, Xmm2)                    /* bxx_t += bxx_k */
        addps_rr(Xmm5, Xmm6)                    /* cxx_t += cxx_k */

        /* "d" section */
        mulps_rr(Xmm5, Xmm1)                    /* c_val *= a_val */
        movpx_rr(Xmm2, Xmm3)                    /* b_val <- b_val */
        mulps_rr(Xmm3, Xmm3)                    /* b_val *= b_val */
        subps_rr(Xmm3, Xmm5)                    /* d_bxb -= d_axc */

        /* create xmask */
        xorpx_rr(Xmm7, Xmm7)                    /* d_min <-     0 */
        cltps_rr(Xmm7, Xmm3)                    /* d_min <! d_val */
        CHECK_MASK(OO_end, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_XMASK)         /* xmask -> XMASK */

        /* "tt" section */
        sqrps_rr(Xmm3, Xmm3)                    /* d_val sq d_val */

        movpx_st(Xmm1, Mecx, ctx_XTMP1)         /* a_val -> XTMP1 */
        movpx_st(Xmm2, Mecx, ctx_XTMP2)         /* b_val -> XTMP2 */
        movpx_st(Xmm3, Mecx, ctx_XTMP3)         /* d_val -> XTMP3 */

/******************************************************************************/
/*  LBL(PB_rt1)  */

        /* outer side */
        CHECK_SIDE(PB_sd1, PB_rt2, RT_FLAG_SIDE_OUTER)

        /* "t1" section */
        movpx_rr(Xmm4, Xmm2)                    /* b_val <- b_val */
        subps_rr(Xmm4, Xmm3)                    /* b_val -= d_val */
        divps_rr(Xmm4, Xmm1)                    /* t_rt1 /= a_val */
        movpx_st(Xmm4, Mecx, ctx_T_VAL(0))      /* t_rt1 -> T_VAL */

        /* clipping */
        SUBROUTINE(PB_cp1, CC_clp)
        CHECK_MASK(PB_rt2, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_TMASK(0))      /* tmask -> TMASK */

        /* material */
        movxx_mi(Mecx, ctx_LOCAL(FLG), IB(RT_FLAG_SIDE_OUTER))
        SUBROUTINE(PB_mt1, PB_mat)

/******************************************************************************/
    LBL(PB_rt2)

        /* inner side */
        CHECK_SIDE(PB_sd2, OO_end, RT_FLAG_SIDE_INNER)

        /* "t2" section */
        movpx_ld(Xmm4, Mecx, ctx_XTMP2)         /* b_val <- b_val */
        addps_ld(Xmm4, Mecx, ctx_XTMP3)         /* b_val += d_val */
        divps_ld(Xmm4, Mecx, ctx_XTMP1)         /* t_rt2 /= a_val */
        movpx_st(Xmm4, Mecx, ctx_T_VAL(0))      /* t_rt2 -> T_VAL */

        /* clipping */
        movpx_ld(Xmm7, Mecx, ctx_XMASK)         /* xmask <- XMASK */
        SUBROUTINE(PB_cp2, CC_clp)
        CHECK_MASK(OO_end, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_TMASK(0))      /* tmask -> TMASK */

        /* material */
        movxx_mi(Mecx, ctx_LOCAL(FLG), IB(RT_FLAG_SIDE_INNER))
        SUBROUTINE(PB_mt2, PB_mat)

        jmpxx_lb(OO_end)

/******************************************************************************/
    LBL(PB_mat)

        FETCH_PROP()                            /* Xmm7  <- ssign */

#if RT_FEAT_SHADOWS

        CHECK_SHAD(PB_shd)

#endif /* RT_FEAT_SHADOWS */

#if RT_FEAT_NORMALS

        CHECK_PROP(PB_nrm, RT_PROP_NORMAL)

        /* compute normal */

        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        MOVXR_LD(Xmm1, Iecx, ctx_NEW_O)         /* loc_k <- NEW_K */
        mulps_ld(Xmm1, Mebx, xpb_PAR_K)         /* loc_k *= PAR_K */
        addps_ld(Xmm1, Mebx, xpb_I_PAR)         /* loc_k += I_PAR */
        sqrps_rr(Xmm1, Xmm1)                    /* loc_k sq loc_k */
        movpx_ld(Xmm3, Mebx, xpb_ONE_K)         /* i_par <-     1 */
        divps_rr(Xmm3, Xmm1)                    /* i_par /= loc_k */
        movpx_ld(Xmm6, Mebx, xpb_PAR_2)         /* par_2 <- PAR_2 */
        xorpx_ld(Xmm6, Mebx, srf_SMASK)         /* par_2 = -par_2 */
        mulps_rr(Xmm6, Xmm3)                    /* par_2 *= i_par */
        xorpx_rr(Xmm6, Xmm7)                    /* par_2 ^= ssign */
        MOVXR_ST(Xmm6, Iecx, ctx_NRM_O)         /* nrm_k -> NRM_K */

        INDEX_AXIS(RT_I)                        /* eax   <-     i */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        MOVXR_LD(Xmm4, Iecx, ctx_NEW_O)         /* loc_i <- NEW_I */
        mulps_rr(Xmm4, Xmm3)                    /* loc_i *= i_par */
        xorpx_rr(Xmm4, Xmm7)                    /* loc_i ^= ssign */
        MOVXR_ST(Xmm4, Iecx, ctx_NRM_O)         /* nrm_i -> NRM_I */

        INDEX_AXIS(RT_J)                        /* eax   <-     j */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        MOVXR_LD(Xmm5, Iecx, ctx_NEW_O)         /* loc_j <- NEW_J */
        mulps_rr(Xmm5, Xmm3)                    /* loc_j *= i_par */
        xorpx_rr(Xmm5, Xmm7)                    /* loc_j ^= ssign */
        MOVXR_ST(Xmm5, Iecx, ctx_NRM_O)         /* nrm_j -> NRM_J */

        jmpxx_lb(MT_nrm)

    LBL(PB_nrm)

#endif /* RT_FEAT_NORMALS */

        jmpxx_lb(MT_mat)

/******************************************************************************/
    LBL(fetch_PB_clp)

        adrxx_lb(PB_clp)
        movxx_st(Reax, Mebp, inf_CLP_PB)
        jmpxx_lb(fetch_HB_ptr)

    LBL(PB_clp)

#if RT_FEAT_CLIPPING_CUSTOM

        INDEX_AXIS(RT_I)                        /* eax   <-     i */
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        MOVXR_LD(Xmm4, Iecx, ctx_NRM_O)         /* dff_i <- NRM_I */
        mulps_rr(Xmm4, Xmm4)                    /* dff_i *= dff_i */

        INDEX_AXIS(RT_J)                        /* eax   <-     j */
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        MOVXR_LD(Xmm5, Iecx, ctx_NRM_O)         /* dff_j <- NRM_J */
        mulps_rr(Xmm5, Xmm5)                    /* dff_j *= dff_j */

        addps_rr(Xmm4, Xmm5)

        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        MOVXR_LD(Xmm6, Iecx, ctx_NRM_O)         /* dff_k <- NRM_K */
        mulps_ld(Xmm6, Mebx, xpb_PAR_K)

        APPLY_CLIP(PB, Xmm4, Xmm6)

        jmpxx_lb(CC_ret)

#endif /* RT_FEAT_CLIPPING_CUSTOM */

/******************************************************************************/
/*******************************   HYPERBOLOID   ******************************/
/******************************************************************************/

    LBL(fetch_HB_ptr)

        adrxx_lb(HB_ptr)
        movxx_st(Reax, Mebp, inf_PTR_HB)
        jmpxx_lb(fetch_HB_clp)

    LBL(HB_ptr)

#if RT_SHOW_TILES

        SHOW_TILES(HB, 0x00888800)

#endif /* RT_SHOW_TILES */

        /* "i" section */
        INDEX_AXIS(RT_I)                        /* eax   <-     i */
        MOVXR_LD(Xmm1, Iecx, ctx_RAY_O)         /* ray_i <- RAY_I */
        MOVXR_LD(Xmm5, Iecx, ctx_DFF_O)         /* dff_i <- DFF_I */
        movpx_rr(Xmm3, Xmm1)                    /* ray_i <- ray_i */
        mulps_rr(Xmm3, Xmm5)                    /* ray_i *= dff_i */
        mulps_rr(Xmm1, Xmm1)                    /* ray_i *= ray_i */
        mulps_rr(Xmm5, Xmm5)                    /* dff_i *= dff_i */

        /* "j" section */
        INDEX_AXIS(RT_J)                        /* eax   <-     j */
        MOVXR_LD(Xmm2, Iecx, ctx_RAY_O)         /* ray_j <- RAY_J */
        MOVXR_LD(Xmm6, Iecx, ctx_DFF_O)         /* dff_j <- DFF_J */
        movpx_rr(Xmm4, Xmm2)                    /* ray_j <- ray_j */
        mulps_rr(Xmm4, Xmm6)                    /* ray_j *= dff_j */
        mulps_rr(Xmm2, Xmm2)                    /* ray_j *= ray_j */
        mulps_rr(Xmm6, Xmm6)                    /* dff_j *= dff_j */

        /* "+" section */
        addps_rr(Xmm1, Xmm2)                    /* axx_i += axx_j */
        addps_rr(Xmm3, Xmm4)                    /* bxx_i += bxx_j */
        addps_rr(Xmm5, Xmm6)                    /* cxx_i += cxx_j */

        /* "k" section */
        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        MOVXR_LD(Xmm2, Iecx, ctx_RAY_O)         /* ray_k <- RAY_K */
        MOVXR_LD(Xmm6, Iecx, ctx_DFF_O)         /* dff_k <- DFF_K */
        movpx_rr(Xmm4, Xmm2)                    /* ray_k <- ray_k */
        mulps_rr(Xmm4, Xmm6)                    /* ray_k *= dff_k */
        mulps_rr(Xmm2, Xmm2)                    /* ray_k *= ray_k */
        mulps_rr(Xmm6, Xmm6)                    /* dff_k *= dff_k */
        movpx_ld(Xmm7, Mebx, xhb_RAT_2)         /* rat_2 <- RAT_2 */
        mulps_rr(Xmm4, Xmm7)                    /* bxx_k *= rat_2 */
        mulps_rr(Xmm2, Xmm7)                    /* axx_k *= rat_2 */
        mulps_rr(Xmm6, Xmm7)                    /* cxx_k *= rat_2 */
        addps_ld(Xmm6, Mebx, xhb_HYP_K)         /* cxx_k += HYP_K */

        /* "-" section */
        subps_rr(Xmm1, Xmm2)                    /* axx_t -= axx_k */
        subps_rr(Xmm3, Xmm4)                    /* bxx_t -= bxx_k */
        subps_rr(Xmm5, Xmm6)                    /* cxx_t -= cxx_k */

        /* "d" section */
        mulps_rr(Xmm5, Xmm1)                    /* c_val *= a_val */
        movpx_rr(Xmm2, Xmm3)                    /* b_val <- b_val */
        mulps_rr(Xmm3, Xmm3)                    /* b_val *= b_val */
        subps_rr(Xmm3, Xmm5)                    /* d_bxb -= d_axc */

        /* create xmask */
        xorpx_rr(Xmm7, Xmm7)                    /* d_min <-     0 */
        cltps_rr(Xmm7, Xmm3)                    /* d_min <! d_val */
        CHECK_MASK(OO_end, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_XMASK)         /* xmask -> XMASK */

        /* "tt" section */
        sqrps_rr(Xmm3, Xmm3)                    /* d_val sq d_val */

        movpx_st(Xmm1, Mecx, ctx_XTMP1)         /* a_val -> XTMP1 */
        movpx_st(Xmm2, Mecx, ctx_XTMP2)         /* b_val -> XTMP2 */
        movpx_st(Xmm3, Mecx, ctx_XTMP3)         /* d_val -> XTMP3 */

/******************************************************************************/
/*  LBL(HB_rt1)  */

        /* outer side */
        CHECK_SIDE(HB_sd1, HB_rt2, RT_FLAG_SIDE_OUTER)

        /* "t1" section */
        movpx_rr(Xmm4, Xmm2)                    /* b_val <- b_val */
        subps_rr(Xmm4, Xmm3)                    /* b_val -= d_val */
        divps_rr(Xmm4, Xmm1)                    /* t_rt1 /= a_val */
        movpx_st(Xmm4, Mecx, ctx_T_VAL(0))      /* t_rt1 -> T_VAL */

        /* clipping */
        SUBROUTINE(HB_cp1, CC_clp)
        CHECK_MASK(HB_rt2, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_TMASK(0))      /* tmask -> TMASK */

        /* material */
        movxx_mi(Mecx, ctx_LOCAL(FLG), IB(RT_FLAG_SIDE_OUTER))
        SUBROUTINE(HB_mt1, HB_mat)

/******************************************************************************/
    LBL(HB_rt2)

        /* inner side */
        CHECK_SIDE(HB_sd2, OO_end, RT_FLAG_SIDE_INNER)

        /* "t2" section */
        movpx_ld(Xmm4, Mecx, ctx_XTMP2)         /* b_val <- b_val */
        addps_ld(Xmm4, Mecx, ctx_XTMP3)         /* b_val += d_val */
        divps_ld(Xmm4, Mecx, ctx_XTMP1)         /* t_rt2 /= a_val */
        movpx_st(Xmm4, Mecx, ctx_T_VAL(0))      /* t_rt2 -> T_VAL */

        /* clipping */
        movpx_ld(Xmm7, Mecx, ctx_XMASK)         /* xmask <- XMASK */

        SUBROUTINE(HB_cp2, CC_clp)
        CHECK_MASK(OO_end, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_TMASK(0))      /* tmask -> TMASK */

        /* material */
        movxx_mi(Mecx, ctx_LOCAL(FLG), IB(RT_FLAG_SIDE_INNER))
        SUBROUTINE(HB_mt2, HB_mat)

        jmpxx_lb(OO_end)

/******************************************************************************/
    LBL(HB_mat)

        FETCH_PROP()                            /* Xmm7  <- ssign */

#if RT_FEAT_SHADOWS

        CHECK_SHAD(HB_shd)

#endif /* RT_FEAT_SHADOWS */

#if RT_FEAT_NORMALS

        CHECK_PROP(HB_nrm, RT_PROP_NORMAL)

        /* compute normal */

        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        MOVXR_LD(Xmm6, Iecx, ctx_NEW_O)         /* loc_k <- NEW_K */
        xorpx_ld(Xmm6, Mebx, srf_SMASK)         /* loc_k = -loc_k */
        movpx_ld(Xmm1, Mebx, xhb_I_RAT)         /* hyp_k <- I_RAT */
        mulps_rr(Xmm1, Xmm6)                    /* hyp_k *= loc_k */
        mulps_rr(Xmm1, Xmm6)                    /* hyp_k *= loc_k */
        addps_ld(Xmm1, Mebx, xhb_HYP_K)         /* hyp_k += HYP_K */
        sqrps_rr(Xmm1, Xmm1)                    /* hyp_k sq hyp_k */
        movpx_ld(Xmm3, Mebx, xhb_ONE_K)         /* i_rat <-     1 */
        divps_rr(Xmm3, Xmm1)                    /* i_rat /= hyp_k */
        mulps_ld(Xmm6, Mebx, xhb_RAT_2)         /* rat_2 <- RAT_2 */
        mulps_rr(Xmm6, Xmm3)                    /* rat_2 *= i_rat */
        xorpx_rr(Xmm6, Xmm7)                    /* rat_2 ^= ssign */
        MOVXR_ST(Xmm6, Iecx, ctx_NRM_O)         /* nrm_k -> NRM_K */

        INDEX_AXIS(RT_I)                        /* eax   <-     i */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        MOVXR_LD(Xmm4, Iecx, ctx_NEW_O)         /* loc_i <- NEW_I */
        mulps_rr(Xmm4, Xmm3)                    /* loc_i *= i_rat */
        xorpx_rr(Xmm4, Xmm7)                    /* loc_i ^= ssign */
        MOVXR_ST(Xmm4, Iecx, ctx_NRM_O)         /* nrm_i -> NRM_I */

        INDEX_AXIS(RT_J)                        /* eax   <-     j */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */
        MOVXR_LD(Xmm5, Iecx, ctx_NEW_O)         /* loc_j <- NEW_J */
        mulps_rr(Xmm5, Xmm3)                    /* loc_j *= i_rat */
        xorpx_rr(Xmm5, Xmm7)                    /* loc_j ^= ssign */
        MOVXR_ST(Xmm5, Iecx, ctx_NRM_O)         /* nrm_j -> NRM_J */

        jmpxx_lb(MT_nrm)

    LBL(HB_nrm)

#endif /* RT_FEAT_NORMALS */

        jmpxx_lb(MT_mat)

/******************************************************************************/
    LBL(fetch_HB_clp)

        adrxx_lb(HB_clp)
        movxx_st(Reax, Mebp, inf_CLP_HB)
        jmpxx_lb(fetch_end)

    LBL(HB_clp)

#if RT_FEAT_CLIPPING_CUSTOM

        INDEX_AXIS(RT_I)                        /* eax   <-     i */
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        MOVXR_LD(Xmm4, Iecx, ctx_NRM_O)         /* dff_i <- NRM_I */
        mulps_rr(Xmm4, Xmm4)                    /* dff_i *= dff_i */

        INDEX_AXIS(RT_J)                        /* eax   <-     j */
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        MOVXR_LD(Xmm5, Iecx, ctx_NRM_O)         /* dff_j <- NRM_J */
        mulps_rr(Xmm5, Xmm5)                    /* dff_j *= dff_j */

        addps_rr(Xmm4, Xmm5)

        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        /* use context's normal fields (NRM)
         * as temporary storage for clipping */
        MOVXR_LD(Xmm6, Iecx, ctx_NRM_O)         /* dff_k <- NRM_K */
        mulps_rr(Xmm6, Xmm6)                    /* dff_k *= dff_k */
        mulps_ld(Xmm6, Mebx, xhb_RAT_2)
        addps_ld(Xmm6, Mebx, xhb_HYP_K)

        APPLY_CLIP(HB, Xmm4, Xmm6)

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

        adrxx_lb(XX_end)
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
        return;
    }

    t_ptr[RT_TAG_PLANE]             = s_inf->ptr_pl;
    t_ptr[RT_TAG_CYLINDER]          = s_inf->ptr_cl;
    t_ptr[RT_TAG_SPHERE]            = s_inf->ptr_sp;
    t_ptr[RT_TAG_CONE]              = s_inf->ptr_cn;
    t_ptr[RT_TAG_PARABOLOID]        = s_inf->ptr_pb;
    t_ptr[RT_TAG_HYPERBOLOID]       = s_inf->ptr_hb;

    t_clp[RT_TAG_PLANE]             = s_inf->clp_pl;
    t_clp[RT_TAG_CYLINDER]          = s_inf->clp_cl;
    t_clp[RT_TAG_SPHERE]            = s_inf->clp_sp;
    t_clp[RT_TAG_CONE]              = s_inf->clp_cn;
    t_clp[RT_TAG_PARABOLOID]        = s_inf->clp_pb;
    t_clp[RT_TAG_HYPERBOLOID]       = s_inf->clp_hb;

    t_pow[0]                        = s_inf->pow_e0;
    t_pow[1]                        = s_inf->pow_e1;
    t_pow[2]                        = s_inf->pow_e2;
    t_pow[3]                        = s_inf->pow_e3;
    t_pow[4]                        = s_inf->pow_e4;
    t_pow[5]                        = s_inf->pow_en;

    /* RT_LOGI("PL ptr = %p\n", s_inf->ptr_pl); */
    /* RT_LOGI("CL ptr = %p\n", s_inf->ptr_cl); */
    /* RT_LOGI("SP ptr = %p\n", s_inf->ptr_sp); */
    /* RT_LOGI("CN ptr = %p\n", s_inf->ptr_cn); */
    /* RT_LOGI("PB ptr = %p\n", s_inf->ptr_pb); */
    /* RT_LOGI("HB ptr = %p\n", s_inf->ptr_hb); */

    /* RT_LOGI("PL clp = %p\n", s_inf->clp_pl); */
    /* RT_LOGI("CL clp = %p\n", s_inf->clp_cl); */
    /* RT_LOGI("SP clp = %p\n", s_inf->clp_sp); */
    /* RT_LOGI("CN clp = %p\n", s_inf->clp_cn); */
    /* RT_LOGI("PB clp = %p\n", s_inf->clp_pb); */
    /* RT_LOGI("HB clp = %p\n", s_inf->clp_hb); */

    /* RT_LOGI("E0 pow = %p\n", s_inf->pow_e0); */
    /* RT_LOGI("E1 pow = %p\n", s_inf->pow_e1); */
    /* RT_LOGI("E2 pow = %p\n", s_inf->pow_e2); */
    /* RT_LOGI("E3 pow = %p\n", s_inf->pow_e3); */
    /* RT_LOGI("E4 pow = %p\n", s_inf->pow_e4); */
    /* RT_LOGI("EN pow = %p\n", s_inf->pow_en); */
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
