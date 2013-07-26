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

/* Conditional compilation flags
 * for respective segments of code.
 */
#define RT_CLIPPING_MINMAX      1
#define RT_TEXTURING            1

/* Byte-offsets within SIMD-field
 * for packed scalar fields.
 */
#define PTR   0x00 /* LOCAL, PARAM, MAT_P, SRF_P */
#define FLG   0x04 /* LOCAL, MAT_P */
#define TAG   0x0C /* SRF_P */

/******************************************************************************/
/*********************************   MACROS   *********************************/
/******************************************************************************/

/* Axis mapping.
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

/* Axis clipping.
 * Check if axis clipping (minmax) is needed,
 * jump to "lb" otherwise.
 */
#define CHECK_CLIP(lb, pl, nx)                                              \
        cmpxx_mi(Mebx, srf_##pl(nx * 4), IB(0))                             \
        jeqxx_lb(lb)

/* Context flags.
 * Value bit-range must not overlap with material props (defined in tracer.h),
 * as they are packed together into the same context field.
 * Current CHECK_FLAG macro (defined below) accepts values upto 8-bit.
 */
#define FLAG_SIDE_OUTER     0
#define FLAG_SIDE_INNER     1
#define FLAG_SIDE           1

/* Check if flag "fl" is set in the context's field "pl",
 * jump to "lb" otherwise.
 */
#define CHECK_FLAG(lb, pl, fl)                                              \
        movxx_ld(Reax, Mecx, ctx_##pl(FLG))                                 \
        andxx_ri(Reax, IB(fl))                                              \
        cmpxx_ri(Reax, IB(0))                                               \
        jeqxx_lb(lb)

/* Material properties.
 * Fetch properties from material into the context's local FLG field
 * based on the currently set SIDE flag.
 */
#define FETCH_PROP()                                                        \
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))                                \
        andxx_ri(Reax, IB(FLAG_SIDE))                                       \
        shlxx_ri(Reax, IB(3))                                               \
        movxx_ld(Reax, Iebx, srf_MAT_P(FLG))                                \
        orrxx_st(Reax, Mecx, ctx_LOCAL(FLG))

/* Check if property "pr" previously
 * fetched from material is set in the context,
 * jump to "lb" otherwise.
 */
#define CHECK_PROP(lb, pr)                                                  \
        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))                                \
        andxx_ri(Reax, IH(pr))                                              \
        cmpxx_ri(Reax, IB(0))                                               \
        jeqxx_lb(lb)

/* Update relevant fragments of the
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

#define PAINT_SIMD(lb)                                                      \
        PAINT_FRAG(lb, 00)                                                  \
        PAINT_FRAG(lb, 04)                                                  \
        PAINT_FRAG(lb, 08)                                                  \
        PAINT_FRAG(lb, 0C)

/* Flush all fragments of
 * the fully computed color SIMD-field from the context
 * into the framebuffer.
 */
#define FRAME_SIMD()                                                        \
        movpx_ld(Xmm0, Mecx, ctx_C_BUF(0))                                  \
        movpx_st(Xmm0, Oeax, PLAIN)

/* Replicate subroutine calling behaviour
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

/* Local pointer tables
 * for quick entry point resolution.
 */
static
rt_pntr t_ptr[RT_TAG_SURFACE_MAX];

/* Backend's global entry point (hence 0).
 * Update surfaces's backend data.
 */
rt_void update0(rt_SIMD_SURFACE *s_srf)
{
    rt_word tag = (rt_word)s_srf->srf_p[3];

    if (tag >= RT_TAG_SURFACE_MAX)
    {
        return;
    }

    /* Save surface's entry point from local pointer table
     * filled during backend's one-time initialization */
    s_srf->srf_p[0] = t_ptr[tag];
}

/******************************************************************************/
/*********************************   RENDER   *********************************/
/******************************************************************************/

/* Backend's global entry point (hence 0).
 * Render the frame based on the data structures
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

        /* enable "rounding towards minus infinity"
         * for texture coords float-to-integer conversion */
        FCTRL_ENTER(ROUNDM)

        movxx_ld(Recx, Mebp, inf_CTX)
        movxx_ld(Redx, Mebp, inf_CAM)

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

        movxx_mi(Mebp, inf_FRM_Y, IB(0))

    LBL(YY_cyc)

        movxx_ld(Reax, Mebp, inf_FRM_Y)
        mulxn_ld(Reax, Mebp, inf_FRM_ROW)
        shlxx_ri(Reax, IB(2))
        addxx_ld(Reax, Mebp, inf_FRAME)
        movxx_st(Reax, Mebp, inf_FRM)

/******************************************************************************/
/********************************   HOR INIT   ********************************/
/******************************************************************************/

        movxx_mi(Mebp, inf_FRM_X, IB(0))

    LBL(XX_cyc)

        movxx_ld(Redx, Mebp, inf_CAM)

        movpx_ld(Xmm0, Medx, cam_T_MAX)         /* tmp_v <- T_MAX */
        movpx_st(Xmm0, Mecx, ctx_T_BUF(0))      /* tmp_v -> T_BUF */

        xorpx_rr(Xmm0, Xmm0)                    /* tmp_v <- 0     */
        movpx_st(Xmm0, Mecx, ctx_C_BUF(0))      /* tmp_v -> C_BUF */

/******************************************************************************/
/********************************   OBJ LIST   ********************************/
/******************************************************************************/

        movxx_ld(Resi, Mebp, inf_LST)

    LBL(OO_cyc)

        cmpxx_ri(Resi, IB(0))
        jeqxx_lb(OO_out)

        movxx_ld(Rebx, Mesi, elm_SIMD)

        movpx_ld(Xmm1, Mebx, srf_POS_X)
        movpx_ld(Xmm2, Mebx, srf_POS_Y)
        movpx_ld(Xmm3, Mebx, srf_POS_Z)

        subps_ld(Xmm1, Mecx, ctx_ORG_X)
        subps_ld(Xmm2, Mecx, ctx_ORG_Y)
        subps_ld(Xmm3, Mecx, ctx_ORG_Z)

        movpx_st(Xmm1, Mecx, ctx_DFF_X)
        movpx_st(Xmm2, Mecx, ctx_DFF_Y)
        movpx_st(Xmm3, Mecx, ctx_DFF_Z)

        jmpxx_mm(Mebx, srf_SRF_P(PTR))

    LBL(fetch_ptr)

        jmpxx_lb(fetch_PL_ptr)

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

#if RT_CLIPPING_MINMAX

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

#endif /* RT_CLIPPING_MINMAX */

        /* "y" section */
        movpx_ld(Xmm5, Mecx, ctx_RAY_Y)         /* ray_y <- RAY_Y */
        mulps_ld(Xmm5, Mecx, ctx_T_VAL(0))      /* ray_y *= t_rt1 */
        addps_ld(Xmm5, Mecx, ctx_ORG_Y)         /* hit_y += ORG_Y */
        movpx_st(Xmm5, Mecx, ctx_HIT_Y)         /* hit_y -> HIT_Y */
        subps_ld(Xmm5, Mebx, srf_POS_Y)         /* loc_y -= POS_Y */
        movpx_st(Xmm5, Mecx, ctx_NEW_Y)         /* loc_y -> NEW_Y */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

#if RT_CLIPPING_MINMAX

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

#endif /* RT_CLIPPING_MINMAX */

        /* "z" section */
        movpx_ld(Xmm6, Mecx, ctx_RAY_Z)         /* ray_z <- RAY_Z */
        mulps_ld(Xmm6, Mecx, ctx_T_VAL(0))      /* ray_z *= t_rt1 */
        addps_ld(Xmm6, Mecx, ctx_ORG_Z)         /* hit_z += ORG_Z */
        movpx_st(Xmm6, Mecx, ctx_HIT_Z)         /* hit_z -> HIT_Z */
        subps_ld(Xmm6, Mebx, srf_POS_Z)         /* loc_z -= POS_Z */
        movpx_st(Xmm6, Mecx, ctx_NEW_Z)         /* loc_z -> NEW_Z */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

#if RT_CLIPPING_MINMAX

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

#endif /* RT_CLIPPING_MINMAX */

        jmpxx_mm(Mecx, ctx_LOCAL(PTR))

/******************************************************************************/
/********************************   MATERIAL   ********************************/
/******************************************************************************/

    LBL(MT_mat)

        movxx_ld(Reax, Mecx, ctx_LOCAL(FLG))
        andxx_ri(Reax, IB(FLAG_SIDE))
        shlxx_ri(Reax, IB(3))
        movxx_ld(Redx, Iebx, srf_MAT_P(PTR))

        movpx_ld(Xmm0, Medx, mat_TEX_P)         /* tex_p <- TEX_P */

#if RT_TEXTURING

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
        cvtps_rr(Xmm1, Xmm4)                    /* tex_x ii tex_x */
        andpx_ld(Xmm1, Medx, mat_XMASK)         /* tex_y &= XMASK */

        cvtps_rr(Xmm2, Xmm5)                    /* tex_y ii tex_y */
        andpx_ld(Xmm2, Medx, mat_YMASK)         /* tex_y &= YMASK */
        shlpx_ld(Xmm2, Medx, mat_YSHFT)         /* tex_y << YSHFT */

        addpx_rr(Xmm1, Xmm2)                    /* tex_x += tex_y */
        shlpx_ri(Xmm1, IB(2))                   /* tex_x << 2     */
        addpx_rr(Xmm0, Xmm1)                    /* tex_x += tex_p */

    LBL(MT_tex)

#endif /* RT_TEXTURING */

        movpx_st(Xmm0, Mecx, ctx_C_PTR(0))      /* tex_p -> C_PTR */
        PAINT_SIMD(MT_rtx)

        jmpxx_mm(Mecx, ctx_LOCAL(PTR))

/******************************************************************************/
/**********************************   PLANE   *********************************/
/******************************************************************************/

    LBL(fetch_PL_ptr)

        adrxx_lb(PL_ptr)
        movxx_st(Reax, Mebp, inf_PTR_PL)
        jmpxx_lb(fetch_end)

    LBL(PL_ptr)

        /* "k" section */
        INDEX_AXIS(RT_K)                        /* eax   <-     k */
        MOVXR_LD(Xmm4, Iecx, ctx_DFF_O)         /* dff_k <- DFF_K */
        MOVXR_LD(Xmm3, Iecx, ctx_RAY_O)         /* ray_k <- RAY_K */

        /* create xmask */
        xorpx_rr(Xmm7, Xmm7)                    /* xmask <- 0     */
        cneps_rr(Xmm7, Xmm3)                    /* xmask != ray_k */

        /* "tt" section */
        divps_rr(Xmm4, Xmm3)                    /* dff_k /= ray_k */
        movpx_st(Xmm4, Mecx, ctx_T_VAL(0))      /* t_rt1 -> T_VAL */

        /* clipping */
        SUBROUTINE(PL_cp1, CC_clp)
        CHECK_MASK(OO_end, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_XMASK)         /* xmask -> XMASK */

        INDEX_AXIS(RT_K)
        MOVXR_LD(Xmm3, Iecx, ctx_RAY_O)         /* ray_k <- RAY_K */
        xorpx_rr(Xmm0, Xmm0)                    /* tmp_v <- 0     */

/******************************************************************************/
/*  LBL(PL_rt1)  */

        /* outer side */
        cltps_rr(Xmm3, Xmm0)                    /* ray_k <! tmp_v */
        andpx_rr(Xmm7, Xmm3)                    /* tmask &= lmask */
        movpx_st(Xmm7, Mecx, ctx_TMASK(0))      /* tmask -> TMASK */
        CHECK_MASK(PL_rt2, NONE, Xmm7)

        /* material */
        movxx_mi(Mecx, ctx_LOCAL(FLG), IB(FLAG_SIDE_OUTER))
        SUBROUTINE(PL_mt1, PL_mat)

/******************************************************************************/
    LBL(PL_rt2)

        /* inner side */
        movpx_ld(Xmm7, Mecx, ctx_TMASK(0))      /* tmask <- TMASK */
        xorpx_ld(Xmm7, Mecx, ctx_XMASK)         /* tmask ^= XMASK */
        CHECK_MASK(OO_end, NONE, Xmm7)
        movpx_st(Xmm7, Mecx, ctx_TMASK(0))      /* tmask -> TMASK */

        /* material */
        movxx_mi(Mecx, ctx_LOCAL(FLG), IB(FLAG_SIDE_INNER))
        SUBROUTINE(PL_mt2, PL_mat)

        jmpxx_lb(OO_end)

/******************************************************************************/
    LBL(PL_mat)

        FETCH_PROP()

#if RT_TEXTURING

        CHECK_PROP(PL_tex, RT_PROP_TEXTURE)

        /* compute surface's UV coords
         * for texturing */

        INDEX_AXIS(RT_I)                        /* eax   <-     i */
        MOVXR_LD(Xmm4, Iecx, ctx_NEW_O)         /* loc_i <- NEW_I */
        movpx_st(Xmm4, Mecx, ctx_TEX_U)         /* loc_i -> TEX_U */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

        INDEX_AXIS(RT_J)                        /* eax   <-     j */
        MOVXR_LD(Xmm5, Iecx, ctx_NEW_O)         /* loc_j <- NEW_J */
        movpx_st(Xmm5, Mecx, ctx_TEX_V)         /* loc_j -> TEX_V */
        /* use next context's RAY fields (NEW)
         * as temporary storage for local HIT */

    LBL(PL_tex)

#endif /* RT_TEXTURING */

        jmpxx_lb(MT_mat)

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

        movxx_ld(Redx, Mebp, inf_CAM)

        movxx_ld(Reax, Mebp, inf_FRM_X)
        shlxx_ri(Reax, IB(2))
        addxx_ld(Reax, Mebp, inf_FRM)
        FRAME_SIMD()
        addxx_mi(Mebp, inf_FRM_X, IB(4))

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

        movxx_ri(Reax, IB(1))
        addxx_st(Reax, Mebp, inf_FRM_Y)

        movxx_ld(Reax, Mebp, inf_FRM_Y)
        cmpxx_rm(Reax, Mebp, inf_FRM_H)
        jltxx_lb(YY_cyc)

        /* restore default "rounding to nearest" */
        FCTRL_LEAVE(ROUNDM)

    LBL(fetch_end)

    ASM_LEAVE(s_inf)

/******************************************************************************/
/**********************************   LEAVE   *********************************/
/******************************************************************************/

    if (s_inf->ctx != RT_NULL)
    {
        return;
    }

    t_ptr[RT_TAG_PLANE] = s_inf->ptr_pl;

    /* RT_LOGI("PL ptr = %p\n", s_inf->ptr_pl); */
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
