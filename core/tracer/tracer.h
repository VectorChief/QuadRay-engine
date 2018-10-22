/******************************************************************************/
/* Copyright (c) 2013-2018 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_TRACER_H
#define RT_TRACER_H

/*
 * RT_DATA determines the maximum load-level for data structures in code-base.
 * 1 - means full DP-level (12-bit displacements) is filled or exceeded (Q=1).
 * 2 - means 1/2  DP-level (11-bit displacements) has not been exceeded (Q=1).
 * 4 - means 1/4  DP-level (10-bit displacements) has not been exceeded (Q=1).
 * 8 - means 1/8  DP-level  (9-bit displacements) has not been exceeded (Q=1).
 * 16  means 1/16 DP-level  (8-bit displacements) has not been exceeded (Q=1).
 * NOTE: the built-in rt_SIMD_INFO structure is already filled at full 1/16th.
 */
#define RT_DATA 4

#include "rtbase.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * tracer.h: Interface for the raytracing rendering backend.
 *
 * More detailed description of this subsystem is given in tracer.cpp.
 * Recommended naming scheme for C++ types and definitions is given in rtbase.h.
 *
 * Note that DP offsets below accept only 12-bit values (0xFFF),
 * use DF, DG, DH and DV for 14, 15, 16 and 31-bit offsets respectively.
 * SIMD width is taken into account via S and Q from rtarch.h
 */

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

/*
 * Material properties.
 * Value bit-range must not overlap with context flags (defined in tracer.cpp),
 * as they are packed together into the same context field.
 * Current CHECK_PROP macro (in tracer.cpp) accepts values upto 16-bit.
 */
#define RT_PROP_LIGHT       0x00000010
#define RT_PROP_METAL       0x00000020
#define RT_PROP_NORMAL      0x00000100
#define RT_PROP_OPAQUE      0x00000200
#define RT_PROP_TRANSP      0x00000400
#define RT_PROP_TEXTURE     0x00000800
#define RT_PROP_REFLECT     0x00001000
#define RT_PROP_REFRACT     0x00002000
#define RT_PROP_DIFFUSE     0x00004000
#define RT_PROP_SPECULAR    0x00008000

/*
 * Clip accumulator markers,
 * some values are hardcoded in rendering backend,
 * change with care!
 */
#define RT_ACCUM_ENTER      (-1)
#define RT_ACCUM_LEAVE      (+1)

/*
 * Macros for packed 16-byte-aligned pointer and lower-4-bits flag.
 */
#define RT_GET_FLG(x)       ((rt_cell)(x) & 0xF)
#define RT_SET_FLG(x, t, v) x = (t)((rt_cell)(v) | (rt_cell)(x) & ~0xF)

#define RT_GET_PTR(x)       ((rt_ELEM *)((rt_cell)(x) & ~0xF))
#define RT_SET_PTR(x, t, v) x = (t)((rt_cell)(v) | RT_GET_FLG(x))

#define RT_GET_ADR(x)       ((rt_ELEM **)&(x))

/* Structures */

struct rt_ELEM;
struct rt_SIMD_INFOX;

struct rt_SIMD_CONTEXT;
struct rt_SIMD_CAMERA;
struct rt_SIMD_LIGHT;
struct rt_SIMD_SURFACE;

struct rt_SIMD_MATERIAL;

/******************************************************************************/
/***************************   GLOBAL ENTRY POINTS   **************************/
/******************************************************************************/

/*
 * Backend's global entry points (switch0, update0, render0)
 * are now defined within rt_Platform class in engine.h file.
 */

/******************************************************************************/
/**********************************   MISC   **********************************/
/******************************************************************************/

/*
 * Generic list element structure.
 * Field names explanation:
 *   data - aux data field (last element, clip side, accum marker, shadow list)
 *   simd - pointer to the SIMD structure (rt_SIMD_LIGHT, rt_SIMD_SURFACE)
 *   temp - aux temp field (high-level object, not used in backend)
 *   next - pointer to the next element
 * Structure is read-only in backend.
 */
struct rt_ELEM
{
    rt_cell data;
#define elm_DATA            DP(0x000*P+E)

    rt_pntr simd;
#define elm_SIMD            DP(0x004*P+E)

    rt_pntr temp;
#define elm_TEMP            DP(0x008*P+E)

    rt_ELEM*next;
#define elm_NEXT            DP(0x00C*P+E)

};

/*
 * Extended SIMD info structure for ASM_ENTER/ASM_LEAVE
 * serves as a container for all other SIMD structures passed to backend,
 * contains backend's internal variables as well as local entry points.
 * Note that DP offsets below start where rt_SIMD_INFO ends (at Q*0x100).
 * Structure is read-write in backend.
 */
struct rt_SIMD_INFOX : public rt_SIMD_INFO
{
    /* external parameters */

    rt_pntr ctx;
#define inf_CTX             DP(Q*0x100+0x000*P+E)

    rt_pntr cam;
#define inf_CAM             DP(Q*0x100+0x004*P+E)

    rt_pntr lst;
#define inf_LST             DP(Q*0x100+0x008*P+E)

    rt_word pad10;
#define inf_PAD10           DP(Q*0x100+0x00C*P+E)


    rt_cell index;
#define inf_INDEX           DP(Q*0x100+0x010*P+E)

    rt_word thnum;
#define inf_THNUM           DP(Q*0x100+0x014*P+E)

    rt_word depth;
#define inf_DEPTH           DP(Q*0x100+0x018*P+E)

    rt_cell fsaa;
#define inf_FSAA            DP(Q*0x100+0x01C*P+E)


    rt_word frm_w;
#define inf_FRM_W           DP(Q*0x100+0x020*P+E)

    rt_word frm_h;
#define inf_FRM_H           DP(Q*0x100+0x024*P+E)

    rt_cell frm_row;
#define inf_FRM_ROW         DP(Q*0x100+0x028*P+E)

    rt_pntr frame;
#define inf_FRAME           DP(Q*0x100+0x02C*P+E)


    rt_word tile_w;
#define inf_TILE_W          DP(Q*0x100+0x030*P+E)

    rt_word tile_h;
#define inf_TILE_H          DP(Q*0x100+0x034*P+E)

    rt_cell tls_row;
#define inf_TLS_ROW         DP(Q*0x100+0x038*P+E)

    rt_pntr tiles;
#define inf_TILES           DP(Q*0x100+0x03C*P+E)

    /* internal variables */

    rt_word frm_x;
#define inf_FRM_X           DP(Q*0x100+0x040*P+E)

    rt_word frm_y;
#define inf_FRM_Y           DP(Q*0x100+0x044*P+E)

    rt_pntr frm;
#define inf_FRM             DP(Q*0x100+0x048*P+E)

    rt_word tls_x;
#define inf_TLS_X           DP(Q*0x100+0x04C*P+E)

    rt_pntr tls;
#define inf_TLS             DP(Q*0x100+0x050*P+E)

    rt_word pad11[7];
#define inf_PAD11           DP(Q*0x100+0x054*P+E)

    /* surface entry points */

    rt_pntr xpl_p[4];
#define inf_XPL_P(nx)       DP(Q*0x100+0x070*P + nx*P)

    rt_pntr xtp_p[4];
#define inf_XTP_P(nx)       DP(Q*0x100+0x080*P + nx*P)

    rt_pntr xqd_p[4];
#define inf_XQD_P(nx)       DP(Q*0x100+0x090*P + nx*P)

    rt_word pad12[4*6];
#define inf_PAD12           DP(Q*0x100+0x0A0*P)

#if RT_DEBUG >= 1

    /* quadric debug info */

    rt_real wmask[S];
#define inf_WMASK           DP(Q*0x100+0x100*P)


    rt_real dff_x[S];
#define inf_DFF_X           DP(Q*0x110+0x100*P)

    rt_real dff_y[S];
#define inf_DFF_Y           DP(Q*0x120+0x100*P)

    rt_real dff_z[S];
#define inf_DFF_Z           DP(Q*0x130+0x100*P)


    rt_real ray_x[S];
#define inf_RAY_X           DP(Q*0x140+0x100*P)

    rt_real ray_y[S];
#define inf_RAY_Y           DP(Q*0x150+0x100*P)

    rt_real ray_z[S];
#define inf_RAY_Z           DP(Q*0x160+0x100*P)


    rt_real a_val[S];
#define inf_A_VAL           DP(Q*0x170+0x100*P)

    rt_real b_val[S];
#define inf_B_VAL           DP(Q*0x180+0x100*P)

    rt_real c_val[S];
#define inf_C_VAL           DP(Q*0x190+0x100*P)

    rt_real d_val[S];
#define inf_D_VAL           DP(Q*0x1A0+0x100*P)


    rt_real dmask[S];
#define inf_DMASK           DP(Q*0x1B0+0x100*P)


    rt_real t1nmr[S];
#define inf_T1NMR           DP(Q*0x1C0+0x100*P)

    rt_real t1dnm[S];
#define inf_T1DNM           DP(Q*0x1D0+0x100*P)

    rt_real t2nmr[S];
#define inf_T2NMR           DP(Q*0x1E0+0x100*P)

    rt_real t2dnm[S];
#define inf_T2DNM           DP(Q*0x1F0+0x100*P)


    rt_real t1val[S];
#define inf_T1VAL           DP(Q*0x200+0x100*P)

    rt_real t2val[S];
#define inf_T2VAL           DP(Q*0x210+0x100*P)

    rt_real t1srt[S];
#define inf_T1SRT           DP(Q*0x220+0x100*P)

    rt_real t2srt[S];
#define inf_T2SRT           DP(Q*0x230+0x100*P)

    rt_real t1msk[S];
#define inf_T1MSK           DP(Q*0x240+0x100*P)

    rt_real t2msk[S];
#define inf_T2MSK           DP(Q*0x250+0x100*P)


    rt_real tside[S];
#define inf_TSIDE           DP(Q*0x260+0x100*P)


    rt_real hit_x[S];
#define inf_HIT_X           DP(Q*0x270+0x100*P)

    rt_real hit_y[S];
#define inf_HIT_Y           DP(Q*0x280+0x100*P)

    rt_real hit_z[S];
#define inf_HIT_Z           DP(Q*0x290+0x100*P)


    rt_real adj_x[S];
#define inf_ADJ_X           DP(Q*0x2A0+0x100*P)

    rt_real adj_y[S];
#define inf_ADJ_Y           DP(Q*0x2B0+0x100*P)

    rt_real adj_z[S];
#define inf_ADJ_Z           DP(Q*0x2C0+0x100*P)


    rt_real nrm_x[S];
#define inf_NRM_X           DP(Q*0x2D0+0x100*P)

    rt_real nrm_y[S];
#define inf_NRM_Y           DP(Q*0x2E0+0x100*P)

    rt_real nrm_z[S];
#define inf_NRM_Z           DP(Q*0x2F0+0x100*P)


    rt_word q_dbg;
#define inf_Q_DBG           DP(Q*0x300+0x100*P+E)

    rt_word q_cnt;
#define inf_Q_CNT           DP(Q*0x300+0x104*P+E)

#endif /* RT_DEBUG */
};

/******************************************************************************/
/*********************************   CONTEXT   ********************************/
/******************************************************************************/

/*
 * SIMD context structure keeps track of current state. New contexts for
 * secondary rays can be stacked upon previous ones by shifting pointer with
 * some overlap (to reduce copying overhead) until max stack depth is reached.
 * Vector field names explanation:
 *   VEC_X, VEC_Y, VEC_Z - world space (reused for array trnode's sub-world org)
 *   VEC_I, VEC_J, VEC_K - surface's sub-world space after trnode's transform
 *   VEC_O - baseline offset for axis mapping.
 * Regular axis mapping fetches from main XYZ fields (no trnode's transform),
 * shifted axis mapping fetches from aux IJK fields after trnode's transform,
 * resulting (in both cases) in surface's local coords for canonical solvers.
 * Structure is read-write in backend.
 */
struct rt_SIMD_CONTEXT
{
    /* depth min */

    rt_real t_min[S];
#define ctx_T_MIN           DP(Q*0x000)

    /* origin */

    rt_real org_x[S];
#define ctx_ORG_X           DP(Q*0x010)

    rt_real org_y[S];
#define ctx_ORG_Y           DP(Q*0x020)

    rt_real org_z[S];
#define ctx_ORG_Z           DP(Q*0x030)

    /* ray */

#define ctx_RAY_O           DP(Q*0x040)

    rt_real ray_x[S];
#define ctx_RAY_X           DP(Q*0x040)

    rt_real ray_y[S];
#define ctx_RAY_Y           DP(Q*0x050)

    rt_real ray_z[S];
#define ctx_RAY_Z           DP(Q*0x060)

    rt_real ray_i[S];
#define ctx_RAY_I           DP(Q*0x070)

    rt_real ray_j[S];
#define ctx_RAY_J           DP(Q*0x080)

    rt_real ray_k[S];
#define ctx_RAY_K           DP(Q*0x090)

    /* diff */

#define ctx_DFF_O           DP(Q*0x0A0)

    rt_real dff_x[S];
#define ctx_DFF_X           DP(Q*0x0A0)

    rt_real dff_y[S];
#define ctx_DFF_Y           DP(Q*0x0B0)

    rt_real dff_z[S];
#define ctx_DFF_Z           DP(Q*0x0C0)

    rt_real dff_i[S];
#define ctx_DFF_I           DP(Q*0x0D0)

    rt_real dff_j[S];
#define ctx_DFF_J           DP(Q*0x0E0)

    rt_real dff_k[S];
#define ctx_DFF_K           DP(Q*0x0F0)

    /* surface coords
     * for texturing */

#define ctx_TEX_O           DP(Q*0x100)

    rt_real tex_u[S];
#define ctx_TEX_U           DP(Q*0x100)

    rt_real tex_v[S];
#define ctx_TEX_V           DP(Q*0x110)

    /* color buffer */

    rt_elem c_ptr[S];
#define ctx_C_PTR(nx)       DP(Q*0x120 + nx)

    rt_elem c_buf[S];
#define ctx_C_BUF(nx)       DP(Q*0x130 + nx)

    /* texture color */

    rt_real tex_r[S];
#define ctx_TEX_R           DP(Q*0x140)

    rt_real tex_g[S];
#define ctx_TEX_G           DP(Q*0x150)

    rt_real tex_b[S];
#define ctx_TEX_B           DP(Q*0x160)

    /* result color */

    rt_real col_r[S];
#define ctx_COL_R(nx)       DP(Q*0x170 + nx)

    rt_real col_g[S];
#define ctx_COL_G(nx)       DP(Q*0x180 + nx)

    rt_real col_b[S];
#define ctx_COL_B(nx)       DP(Q*0x190 + nx)

    /* root sorting masks */

    rt_elem amask[S];
#define ctx_AMASK           DP(Q*0x1A0)

    rt_elem dmask[S];
#define ctx_DMASK           DP(Q*0x1B0)

    /* depth, masks, temps, misc */

    rt_real t_val[S];
#define ctx_T_VAL(nx)       DP(Q*0x1C0 + nx)

    rt_real t_buf[S];
#define ctx_T_BUF(nx)       DP(Q*0x1D0 + nx)

    rt_real tmask[S];
#define ctx_TMASK(nx)       DP(Q*0x1E0 + nx)

    rt_elem wmask[S];
#define ctx_WMASK           DP(Q*0x1F0)

    rt_elem xmask[S];
#define ctx_XMASK           DP(Q*0x200)

    rt_real xtmp1[S];
#define ctx_XTMP1           DP(Q*0x210)

    rt_real xtmp2[S];
#define ctx_XTMP2           DP(Q*0x220)

    rt_ui32 xmisc[R];
#define ctx_XMISC(nx)       DP(Q*0x230 + nx)

    /* normal */

#define ctx_NRM_O           DP(Q*0x240)

    rt_real nrm_x[S];
#define ctx_NRM_X           DP(Q*0x240)

    rt_real nrm_y[S];
#define ctx_NRM_Y           DP(Q*0x250)

    rt_real nrm_z[S];
#define ctx_NRM_Z           DP(Q*0x260)

    rt_real nrm_i[S];
#define ctx_NRM_I           DP(Q*0x270)

    rt_real nrm_j[S];
#define ctx_NRM_J           DP(Q*0x280)

    rt_real nrm_k[S];
#define ctx_NRM_K           DP(Q*0x290)

    /* packed scalar fields */

    rt_ui64 param[R];
#define ctx_PARAM(nx)       DP(Q*0x2A0 + nx*2)

    rt_ui64 local[R];
#define ctx_LOCAL(nx)       DP(Q*0x2C0 + nx*2)

    /* custom clipping accum */

    rt_elem c_acc[S];
#define ctx_C_ACC           DP(Q*0x2E0)

    /* fresnel reflection term */

    rt_real f_rfl[S];
#define ctx_F_RFL           DP(Q*0x2F0)

    /* overlapping next context,
     * new depth min */

    rt_real t_new[S];
#define ctx_T_NEW           DP(Q*0x300)

    /* hit point,
     * new origin */

    rt_real hit_x[S];
#define ctx_HIT_X           DP(Q*0x310)

    rt_real hit_y[S];
#define ctx_HIT_Y           DP(Q*0x320)

    rt_real hit_z[S];
#define ctx_HIT_Z           DP(Q*0x330)

    /* new ray */

#define ctx_NEW_O           DP(Q*0x340)

    rt_real new_x[S];
#define ctx_NEW_X           DP(Q*0x340)

    rt_real new_y[S];
#define ctx_NEW_Y           DP(Q*0x350)

    rt_real new_z[S];
#define ctx_NEW_Z           DP(Q*0x360)

    rt_real new_i[S];
#define ctx_NEW_I           DP(Q*0x370)

    rt_real new_j[S];
#define ctx_NEW_J           DP(Q*0x380)

    rt_real new_k[S];
#define ctx_NEW_K           DP(Q*0x390)

};

/* context stack step for secondary rays */
#define RT_STACK_STEP       (Q * 0x300)

/******************************************************************************/
/*********************************   CAMERA   *********************************/
/******************************************************************************/

/*
 * SIMD camera structure with properties for
 * rays horizontal and vertical scanning, color masks,
 * accumulated ambient color.
 * Structure is read-only in backend.
 */
struct rt_SIMD_CAMERA
{
    /* depth max value */

    rt_real t_max[S];
#define cam_T_MAX           DP(Q*0x000)

    /* ray initial direction */

    rt_real dir_x[S];
#define cam_DIR_X           DP(Q*0x010)

    rt_real dir_y[S];
#define cam_DIR_Y           DP(Q*0x020)

    rt_real dir_z[S];
#define cam_DIR_Z           DP(Q*0x030)

    /* ray update horizontal */

    rt_real hor_x[S];
#define cam_HOR_X           DP(Q*0x040)

    rt_real hor_y[S];
#define cam_HOR_Y           DP(Q*0x050)

    rt_real hor_z[S];
#define cam_HOR_Z           DP(Q*0x060)

    /* ray update vertical */

    rt_real ver_x[S];
#define cam_VER_X           DP(Q*0x070)

    rt_real ver_y[S];
#define cam_VER_Y           DP(Q*0x080)

    rt_real ver_z[S];
#define cam_VER_Z           DP(Q*0x090)

    /* ambient color and intensity */

    rt_real col_r[S];
#define cam_COL_R           DP(Q*0x0A0)

    rt_real col_g[S];
#define cam_COL_G           DP(Q*0x0B0)

    rt_real col_b[S];
#define cam_COL_B           DP(Q*0x0C0)

    rt_real l_amb[S];
#define cam_L_AMB           DP(Q*0x0D0)

    /* color masks */

    rt_real clamp[S];
#define cam_CLAMP           DP(Q*0x0E0)

    rt_elem cmask[S];
#define cam_CMASK           DP(Q*0x0F0)

};

/******************************************************************************/
/**********************************   LIGHT   *********************************/
/******************************************************************************/

/*
 * SIMD light structure with properties.
 * Structure is read-only in backend.
 */
struct rt_SIMD_LIGHT
{
    /* depth max value */

    rt_real t_max[S];
#define lgt_T_MAX           DP(Q*0x000)

    /* light position */

    rt_real pos_x[S];
#define lgt_POS_X           DP(Q*0x010)

    rt_real pos_y[S];
#define lgt_POS_Y           DP(Q*0x020)

    rt_real pos_z[S];
#define lgt_POS_Z           DP(Q*0x030)

    /* light color and intensity */

    rt_real col_r[S];
#define lgt_COL_R           DP(Q*0x040)

    rt_real col_g[S];
#define lgt_COL_G           DP(Q*0x050)

    rt_real col_b[S];
#define lgt_COL_B           DP(Q*0x060)

    rt_real l_src[S];
#define lgt_L_SRC           DP(Q*0x070)

    /* light attenuation */

    rt_real a_qdr[S];
#define lgt_A_QDR           DP(Q*0x080)

    rt_real a_lnr[S];
#define lgt_A_LNR           DP(Q*0x090)

    rt_real a_cnt[S];
#define lgt_A_CNT           DP(Q*0x0A0)

    rt_real a_rng[S];
#define lgt_A_RNG           DP(Q*0x0B0)

};

/******************************************************************************/
/*********************************   SURFACE   ********************************/
/******************************************************************************/

/*
 * SIMD surface structure with properties.
 * Structure is read-only in backend.
 */
struct rt_SIMD_SURFACE
{
    /* clipping accum default */

    rt_elem c_def[S];
#define srf_C_DEF           DP(Q*0x000)

    /* surface position */

    rt_real pos_x[S];
#define srf_POS_X           DP(Q*0x010)

    rt_real pos_y[S];
#define srf_POS_Y           DP(Q*0x020)

    rt_real pos_z[S];
#define srf_POS_Z           DP(Q*0x030)

    /* axis min clippers */

    rt_real min_x[S];
#define srf_MIN_X           DP(Q*0x040)

    rt_real min_y[S];
#define srf_MIN_Y           DP(Q*0x050)

    rt_real min_z[S];
#define srf_MIN_Z           DP(Q*0x060)

    /* axis max clippers */

    rt_real max_x[S];
#define srf_MAX_X           DP(Q*0x070)

    rt_real max_y[S];
#define srf_MAX_Y           DP(Q*0x080)

    rt_real max_z[S];
#define srf_MAX_Z           DP(Q*0x090)

    /* axis clipping toggles (on/off) */

    rt_si32 min_t[R];
#define srf_MIN_T(nx)       DP(Q*0x0A0 + nx)

    rt_si32 max_t[R];
#define srf_MAX_T(nx)       DP(Q*0x0B0 + nx)

    /* surface axis mapping */

    rt_si32 a_map[R];
#define srf_A_MAP(nx)       DP(Q*0x0C0 + nx)

    rt_si32 a_sgn[R];
#define srf_A_SGN(nx)       DP(Q*0x0D0 + nx)

    /* sign masks */

    rt_elem sbase[S];
#define srf_SBASE           DP(Q*0x0E0)

    rt_elem smask[S];
#define srf_SMASK           DP(Q*0x0F0)

    /* root sorting thresholds */

    rt_real d_eps[S];
#define srf_D_EPS           DP(Q*0x100)

    rt_real t_eps[S];
#define srf_T_EPS           DP(Q*0x110)

    /* reserved area 1 */

    rt_elem pad01[S*2];
#define srf_PAD01           DP(Q*0x120)

    /* transform coeffs */

    rt_real tci_x[S];
#define srf_TCI_X           DP(Q*0x140)

    rt_real tci_y[S];
#define srf_TCI_Y           DP(Q*0x150)

    rt_real tci_z[S];
#define srf_TCI_Z           DP(Q*0x160)


    rt_real tcj_x[S];
#define srf_TCJ_X           DP(Q*0x170)

    rt_real tcj_y[S];
#define srf_TCJ_Y           DP(Q*0x180)

    rt_real tcj_z[S];
#define srf_TCJ_Z           DP(Q*0x190)


    rt_real tck_x[S];
#define srf_TCK_X           DP(Q*0x1A0)

    rt_real tck_y[S];
#define srf_TCK_Y           DP(Q*0x1B0)

    rt_real tck_z[S];
#define srf_TCK_Z           DP(Q*0x1C0)

    /* geometry scaling coeffs */

#define srf_SCI_O           DP(Q*0x1D0)

    rt_real sci_x[S];
#define srf_SCI_X           DP(Q*0x1D0)

    rt_real sci_y[S];
#define srf_SCI_Y           DP(Q*0x1E0)

    rt_real sci_z[S];
#define srf_SCI_Z           DP(Q*0x1F0)

    rt_real sci_w[S];
#define srf_SCI_W           DP(Q*0x200)


    rt_real scj_x[S];
#define srf_SCJ_X           DP(Q*0x210)

    rt_real scj_y[S];
#define srf_SCJ_Y           DP(Q*0x220)

    rt_real scj_z[S];
#define srf_SCJ_Z           DP(Q*0x230)

    /* misc tags/pointers */

    rt_si32 srf_t[4];
#define srf_SRF_T(nx)       DP(Q*0x240 + nx)

    rt_pntr msc_p[4];
#define srf_MSC_P(nx)       DP(Q*0x240+0x010+0x000*P+E + nx*P)

    rt_pntr mat_p[4];
#define srf_MAT_P(nx)       DP(Q*0x240+0x010+0x010*P+E + nx*P)

    rt_pntr lst_p[4];
#define srf_LST_P(nx)       DP(Q*0x240+0x010+0x020*P+E + nx*P)

};

/******************************************************************************/
/********************************   MATERIAL   ********************************/
/******************************************************************************/

/*
 * SIMD material structure with properties.
 * Structure is read-only in backend.
 */
struct rt_SIMD_MATERIAL
{
    /* texture transform */

    rt_real xscal[S];
#define mat_XSCAL           DP(Q*0x000)

    rt_real yscal[S];
#define mat_YSCAL           DP(Q*0x010)

    rt_real xoffs[S];
#define mat_XOFFS           DP(Q*0x020)

    rt_real yoffs[S];
#define mat_YOFFS           DP(Q*0x030)

    /* texture mapping */

    rt_elem xmask[S];
#define mat_XMASK           DP(Q*0x040)

    rt_elem ymask[S];
#define mat_YMASK           DP(Q*0x050)

    rt_elem yshft[S];
#define mat_YSHFT           DP(Q*0x060)

    rt_pntr tex_p[R/P];
#define mat_TEX_P           DP(Q*0x070+E)

    /* texture axis mapping */

    rt_si32 t_map[R];
#define mat_T_MAP(nx)       DP(Q*0x080 + nx)

    rt_elem pad01[S];
#define mat_PAD01           DP(Q*0x090)

    /* properties */

    rt_real l_dff[S];
#define mat_L_DFF           DP(Q*0x0A0)

    rt_real l_spc[S];
#define mat_L_SPC           DP(Q*0x0B0)

    rt_ui32 l_pow[R];
#define mat_L_POW           DP(Q*0x0C0)

    rt_elem pad02[S];
#define mat_PAD02           DP(Q*0x0D0)


    rt_real c_rfl[S];
#define mat_C_RFL           DP(Q*0x0E0)

    rt_real c_trn[S];
#define mat_C_TRN           DP(Q*0x0F0)

    rt_real c_rfr[S];
#define mat_C_RFR           DP(Q*0x100)

    rt_real rfr_2[S];
#define mat_RFR_2           DP(Q*0x110)


    rt_real c_rcp[S];
#define mat_C_RCP           DP(Q*0x120)

    rt_real ext_2[S];
#define mat_EXT_2           DP(Q*0x130)

    /* color masks */

    rt_real clamp[S];
#define mat_CLAMP           DP(Q*0x140)

    rt_elem cmask[S];
#define mat_CMASK           DP(Q*0x150)

};

/*
 * Extended SIMD info structure for ASM_ENTER/ASM_LEAVE in plot functions.
 * Note that DP offsets below start where rt_SIMD_INFO ends (at Q*0x100).
 * Structure is read-write in backend.
 */
struct rt_SIMD_INFOP : public rt_SIMD_INFO
{
    /* external parameters */

    rt_fp32 i_cos[R];
#define inf_I_COS           DP(Q*0x100)

    rt_fp32 o_rfl[R];
#define inf_O_RFL           DP(Q*0x110)


    rt_fp32 c_rfr[R];
#define inf_C_RFR           DP(Q*0x120)

    rt_fp32 rfr_2[R];
#define inf_RFR_2           DP(Q*0x130)


    rt_fp32 c_rcp[R];
#define inf_C_RCP           DP(Q*0x140)

    rt_fp32 ext_2[R];
#define inf_EXT_2           DP(Q*0x150)


    rt_fp32 t_new[R];
#define inf_T_NEW           DP(Q*0x160)

};

#endif /* RT_TRACER_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
