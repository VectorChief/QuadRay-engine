/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_TRACER_H
#define RT_TRACER_H

#include "rtarch.h"
#include "rtbase.h"

/******************************************************************************/
/**********************************   MISC   **********************************/
/******************************************************************************/

struct rt_ELEM
{
    rt_cell data;
#define elm_DATA            DP(0x000)

    rt_pntr simd;
#define elm_SIMD            DP(0x004)

    rt_pntr temp;
#define elm_TEMP            DP(0x008)

    rt_ELEM*next;
#define elm_NEXT            DP(0x00C)

};

struct rt_SIMD_INFOX : public rt_SIMD_INFO
{
    /* external parameters */

    rt_pntr ctx;
#define inf_CTX             DP(0x100)

    rt_pntr cam;
#define inf_CAM             DP(0x104)

    rt_pntr lst;
#define inf_LST             DP(0x108)

    rt_word depth;
#define inf_DEPTH           DP(0x10C)


    rt_word frm_w;
#define inf_FRM_W           DP(0x110)

    rt_word frm_h;
#define inf_FRM_H           DP(0x114)

    rt_cell frm_row;
#define inf_FRM_ROW         DP(0x118)

    rt_pntr frame;
#define inf_FRAME           DP(0x11C)

    /* internal variables */

    rt_word frm_x;
#define inf_FRM_X           DP(0x120)

    rt_word frm_y;
#define inf_FRM_Y           DP(0x124)

    rt_pntr frm;
#define inf_FRM             DP(0x128)

    rt_word pad11[53];
#define inf_PAD11           DP(0x12C)

    /* surface entry points */

    rt_pntr ptr_pl;
#define inf_PTR_PL          DP(0x200)

    rt_pntr clp_pl;
#define inf_CLP_PL          DP(0x204)

    rt_pntr ptr_cl;
#define inf_PTR_CL          DP(0x208)

    rt_pntr clp_cl;
#define inf_CLP_CL          DP(0x20C)

    rt_pntr ptr_sp;
#define inf_PTR_SP          DP(0x210)

    rt_pntr clp_sp;
#define inf_CLP_SP          DP(0x214)

    rt_pntr ptr_cn;
#define inf_PTR_CN          DP(0x218)

    rt_pntr clp_cn;
#define inf_CLP_CN          DP(0x21C)

    rt_pntr ptr_pb;
#define inf_PTR_PB          DP(0x220)

    rt_pntr clp_pb;
#define inf_CLP_PB          DP(0x224)

    rt_pntr ptr_hb;
#define inf_PTR_HB          DP(0x228)

    rt_pntr clp_hb;
#define inf_CLP_HB          DP(0x22C)

};

/******************************************************************************/
/*********************************   CONTEXT   ********************************/
/******************************************************************************/

struct rt_SIMD_CONTEXT
{
    /* origin */

    rt_real org_x[4];
#define ctx_ORG_X           DP(0x000)

    rt_real org_y[4];
#define ctx_ORG_Y           DP(0x010)

    rt_real org_z[4];
#define ctx_ORG_Z           DP(0x020)

    /* depth min */

    rt_real t_min[4];
#define ctx_T_MIN           DP(0x030)

    /* ray */

#define ctx_RAY_O           DP(0x040)

    rt_real ray_x[4];
#define ctx_RAY_X           DP(0x040)

    rt_real ray_y[4];
#define ctx_RAY_Y           DP(0x050)

    rt_real ray_z[4];
#define ctx_RAY_Z           DP(0x060)

    rt_word pad01[12];
#define ctx_PAD01           DP(0x070)

    /* diff */

#define ctx_DFF_O           DP(0x0A0)

    rt_real dff_x[4];
#define ctx_DFF_X           DP(0x0A0)

    rt_real dff_y[4];
#define ctx_DFF_Y           DP(0x0B0)

    rt_real dff_z[4];
#define ctx_DFF_Z           DP(0x0C0)

    rt_word pad02[12];
#define ctx_PAD02           DP(0x0D0)

    /* surface coords
     * for texturing */

#define ctx_TEX_O           DP(0x100)

    rt_real tex_u[4];
#define ctx_TEX_U           DP(0x100)

    rt_real tex_v[4];
#define ctx_TEX_V           DP(0x110)

    /* texture color */

    rt_real tex_r[4];
#define ctx_TEX_R           DP(0x120)

    rt_real tex_g[4];
#define ctx_TEX_G           DP(0x130)

    rt_real tex_b[4];
#define ctx_TEX_B           DP(0x140)

    /* color buffer */

    rt_cell c_ptr[4];
#define ctx_C_PTR(nx)       DP(0x150 + nx)

    rt_cell c_buf[4];
#define ctx_C_BUF(nx)       DP(0x160 + nx)

    /* result color */

    rt_real col_r[4];
#define ctx_COL_R(nx)       DP(0x170 + nx)

    rt_real col_g[4];
#define ctx_COL_G(nx)       DP(0x180 + nx)

    rt_real col_b[4];
#define ctx_COL_B(nx)       DP(0x190 + nx)

    /* misc */

    rt_cell param[4];
#define ctx_PARAM(nx)       DP(0x1A0 + nx)

    rt_cell local[4];
#define ctx_LOCAL(nx)       DP(0x1B0 + nx)

    rt_real t_val[4];
#define ctx_T_VAL(nx)       DP(0x1C0 + nx)

    rt_real t_buf[4];
#define ctx_T_BUF(nx)       DP(0x1D0 + nx)

    rt_cell tmask[4];
#define ctx_TMASK(nx)       DP(0x1E0 + nx)

    rt_cell xmask[4];
#define ctx_XMASK           DP(0x1F0)

    rt_cell xtmp1[4];
#define ctx_XTMP1           DP(0x200)

    rt_cell xtmp2[4];
#define ctx_XTMP2           DP(0x210)

    rt_cell xtmp3[4];
#define ctx_XTMP3           DP(0x220)

    rt_word pad04[4];
#define ctx_PAD04           DP(0x230)

    /* normal */

#define ctx_NRM_O           DP(0x240)

    rt_real nrm_x[4];
#define ctx_NRM_X           DP(0x240)

    rt_real nrm_y[4];
#define ctx_NRM_Y           DP(0x250)

    rt_real nrm_z[4];
#define ctx_NRM_Z           DP(0x260)

    rt_word pad05[36];
#define ctx_PAD05           DP(0x270)

    /* hit, overlapping next context,
     * new origin */

    rt_real hit_x[4];
#define ctx_HIT_X           DP(0x300)

    rt_real hit_y[4];
#define ctx_HIT_Y           DP(0x310)

    rt_real hit_z[4];
#define ctx_HIT_Z           DP(0x320)

    /* new depth min */

    rt_real t_new[4];
#define ctx_T_NEW           DP(0x330)

    /* new ray */

#define ctx_NEW_O           DP(0x340)

    rt_real new_x[4];
#define ctx_NEW_X           DP(0x340)

    rt_real new_y[4];
#define ctx_NEW_Y           DP(0x350)

    rt_real new_z[4];
#define ctx_NEW_Z           DP(0x360)

    rt_word pad06[12];
#define ctx_PAD06           DP(0x370)

};

#define RT_STACK_STEP       0x300

/******************************************************************************/
/*********************************   CAMERA   *********************************/
/******************************************************************************/

struct rt_SIMD_CAMERA
{
    /* ray initial direction */

    rt_real dir_x[4];
#define cam_DIR_X           DP(0x000)

    rt_real dir_y[4];
#define cam_DIR_Y           DP(0x010)

    rt_real dir_z[4];
#define cam_DIR_Z           DP(0x020)

    /* depth max value */

    rt_real t_max[4];
#define cam_T_MAX           DP(0x030)

    /* ray update horizontal */

    rt_real hor_x[4];
#define cam_HOR_X           DP(0x040)

    rt_real hor_y[4];
#define cam_HOR_Y           DP(0x050)

    rt_real hor_z[4];
#define cam_HOR_Z           DP(0x060)

    /* ray update vertical */

    rt_real ver_x[4];
#define cam_VER_X           DP(0x070)

    rt_real ver_y[4];
#define cam_VER_Y           DP(0x080)

    rt_real ver_z[4];
#define cam_VER_Z           DP(0x090)

    /* color masks */

    rt_real clamp[4];
#define cam_CLAMP           DP(0x0A0)

    rt_cell cmask[4];
#define cam_CMASK           DP(0x0B0)

    /* ambient color */

    rt_real col_r[4];
#define cam_COL_R           DP(0x0C0)

    rt_real col_g[4];
#define cam_COL_G           DP(0x0D0)

    rt_real col_b[4];
#define cam_COL_B           DP(0x0E0)

};

/******************************************************************************/
/**********************************   LIGHT   *********************************/
/******************************************************************************/

struct rt_SIMD_LIGHT
{
    /* light position */

    rt_real pos_x[4];
#define lgt_POS_X           DP(0x000)

    rt_real pos_y[4];
#define lgt_POS_Y           DP(0x010)

    rt_real pos_z[4];
#define lgt_POS_Z           DP(0x020)

    /* depth max value */

    rt_real t_max[4];
#define lgt_T_MAX           DP(0x030)

    /* light color */

    rt_real col_r[4];
#define lgt_COL_R           DP(0x040)

    rt_real col_g[4];
#define lgt_COL_G           DP(0x050)

    rt_real col_b[4];
#define lgt_COL_B           DP(0x060)

};

/******************************************************************************/
/*********************************   SURFACE   ********************************/
/******************************************************************************/

struct rt_SIMD_SURFACE
{
    /* surface position */

    rt_real pos_x[4];
#define srf_POS_X           DP(0x000)

    rt_real pos_y[4];
#define srf_POS_Y           DP(0x010)

    rt_real pos_z[4];
#define srf_POS_Z           DP(0x020)

    /* reserved area 1 */

    rt_word pad01[4];
#define srf_PAD01           DP(0x030)

    /* axis min clippers */

    rt_real min_x[4];
#define srf_MIN_X           DP(0x040)

    rt_real min_y[4];
#define srf_MIN_Y           DP(0x050)

    rt_real min_z[4];
#define srf_MIN_Z           DP(0x060)

    /* axis max clippers */

    rt_real max_x[4];
#define srf_MAX_X           DP(0x070)

    rt_real max_y[4];
#define srf_MAX_Y           DP(0x080)

    rt_real max_z[4];
#define srf_MAX_Z           DP(0x090)

    /* axis clippers toggles (on/off) */

    rt_cell min_t[4];
#define srf_MIN_T(nx)       DP(0x0A0 + nx)

    rt_cell max_t[4];
#define srf_MAX_T(nx)       DP(0x0B0 + nx)

    /* surface axis mapping */

    rt_cell a_map[4];
#define srf_A_MAP(nx)       DP(0x0C0 + nx)

    rt_cell a_sgn[4];
#define srf_A_SGN(nx)       DP(0x0D0 + nx)

    /* sign masks */

    rt_cell sbase[4];
#define srf_SBASE           DP(0x0E0)

    rt_cell smask[4];
#define srf_SMASK           DP(0x0F0)

    /* misc pointers */

    rt_pntr mat_p[4];
#define srf_MAT_P(nx)       DP(0x100 + nx)

    rt_pntr srf_p[4];
#define srf_SRF_P(nx)       DP(0x110 + nx)

    rt_pntr msc_p[4];
#define srf_MSC_P(nx)       DP(0x120 + nx)

    rt_pntr lst_p[4];
#define srf_LST_P(nx)       DP(0x130 + nx)

    /* reserved area 2 */

    rt_word pad02[48];
#define srf_PAD02           DP(0x140)

};

/******************************************************************************/
/**********************************   PLANE   *********************************/
/******************************************************************************/

struct rt_SIMD_PLANE : public rt_SIMD_SURFACE
{
    rt_real nrm_k[4];
#define xpl_NRM_K           DP(0x200)

    rt_cell msc_1[4];
#define xpl_MSC_1           DP(0x210)

    rt_cell msc_2[4];
#define xpl_MSC_2           DP(0x220)

    rt_real msc_3[4];
#define xpl_MSC_3           DP(0x230)

};

/******************************************************************************/
/********************************   CYLINDER   ********************************/
/******************************************************************************/

struct rt_SIMD_CYLINDER : public rt_SIMD_SURFACE
{
    rt_real rad_2[4];
#define xcl_RAD_2           DP(0x200)

    rt_real i_rad[4];
#define xcl_I_RAD           DP(0x210)

    rt_real msc_2[4];
#define xcl_MSC_2           DP(0x220)

    rt_real msc_3[4];
#define xcl_MSC_3           DP(0x230)

};

/******************************************************************************/
/*********************************   SPHERE   *********************************/
/******************************************************************************/

struct rt_SIMD_SPHERE : public rt_SIMD_SURFACE
{
    rt_real rad_2[4];
#define xsp_RAD_2           DP(0x200)

    rt_real i_rad[4];
#define xsp_I_RAD           DP(0x210)

    rt_real msc_2[4];
#define xsp_MSC_2           DP(0x220)

    rt_real msc_3[4];
#define xsp_MSC_3           DP(0x230)

};

/******************************************************************************/
/**********************************   CONE   **********************************/
/******************************************************************************/

struct rt_SIMD_CONE : public rt_SIMD_SURFACE
{
    rt_real rat_2[4];
#define xcn_RAT_2           DP(0x200)

    rt_real i_rat[4];
#define xcn_I_RAT           DP(0x210)

    rt_real msc_2[4];
#define xcn_MSC_2           DP(0x220)

    rt_real msc_3[4];
#define xcn_MSC_3           DP(0x230)

};

/******************************************************************************/
/*******************************   PARABOLOID   *******************************/
/******************************************************************************/

struct rt_SIMD_PARABOLOID : public rt_SIMD_SURFACE
{
    rt_real par_2[4];
#define xpb_PAR_2           DP(0x200)

    rt_real i_par[4];
#define xpb_I_PAR           DP(0x210)

    rt_real par_k[4];
#define xpb_PAR_K           DP(0x220)

    rt_real one_k[4];
#define xpb_ONE_K           DP(0x230)

};

/******************************************************************************/
/*******************************   HYPERBOLOID   ******************************/
/******************************************************************************/

struct rt_SIMD_HYPERBOLOID : public rt_SIMD_SURFACE
{
    rt_real rat_2[4];
#define xhb_RAT_2           DP(0x200)

    rt_real i_rat[4];
#define xhb_I_RAT           DP(0x210)

    rt_real hyp_k[4];
#define xhb_HYP_K           DP(0x220)

    rt_real one_k[4];
#define xhb_ONE_K           DP(0x230)

};

/******************************************************************************/
/********************************   MATERIAL   ********************************/
/******************************************************************************/

/* Material properties.
 * Value bit-range must not overlap with context flags (defined in tracer.cpp),
 * as they are packed together into the same context field.
 * Current CHECK_PROP macro (defined in tracer.cpp) accepts values upto 16-bit.
 */
#define RT_PROP_TEXTURE     0x00000010
#define RT_PROP_REFLECT     0x00000020
#define RT_PROP_NORMAL      0x00000100
#define RT_PROP_LIGHT       0x00001000

struct rt_SIMD_MATERIAL
{
    /* texture scaling */

    rt_real xscal[4];
#define mat_XSCAL           DP(0x000)

    rt_real yscal[4];
#define mat_YSCAL           DP(0x010)

    rt_real xoffs[4];
#define mat_XOFFS           DP(0x020)

    rt_real yoffs[4];
#define mat_YOFFS           DP(0x030)

    /* texture mapping */

    rt_cell xmask[4];
#define mat_XMASK           DP(0x040)

    rt_cell ymask[4];
#define mat_YMASK           DP(0x050)

    rt_cell yshft[4];
#define mat_YSHFT           DP(0x060)

    rt_pntr tex_p[4];
#define mat_TEX_P           DP(0x070)

    /* texture axis mapping */

    rt_cell t_map[4];
#define mat_T_MAP(nx)       DP(0x080 + nx)

    /* color masks */

    rt_cell cmask[4];
#define mat_CMASK           DP(0x090)

    /* properties */

    rt_real l_dff[4];
#define mat_L_DFF           DP(0x0A0)

    rt_real c_rfl[4];
#define mat_C_RFL           DP(0x0B0)

    rt_real c_one[4];
#define mat_C_ONE           DP(0x0C0)

};

/******************************************************************************/
/***************************   GLOBAL ENTRY POINTS   **************************/
/******************************************************************************/

rt_void update0(rt_SIMD_SURFACE *s_srf);

rt_void render0(rt_SIMD_INFOX *s_inf);

#endif /* RT_TRACER_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
