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

    rt_pntr pad10;
#define inf_PAD10           DP(0x10C)


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

    rt_word pad11;
#define inf_PAD11           DP(0x12C)

    /* surface entry points */

    rt_pntr ptr_pl;
#define inf_PTR_PL          DP(0x130)

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

    rt_word pad02[20];
#define ctx_PAD02           DP(0x0D0)

    /* color buffer */

    rt_cell c_ptr[4];
#define ctx_C_PTR(nx)       DP(0x120 + nx)

    rt_cell c_buf[4];
#define ctx_C_BUF(nx)       DP(0x130 + nx)

    rt_word pad03[24];
#define ctx_PAD03           DP(0x140 + nx)

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

    rt_word pad04[64];
#define ctx_PAD04           DP(0x200)

    /* hit, overlapping next context */

    rt_real hit_x[4];
#define ctx_HIT_X           DP(0x300)

    rt_real hit_y[4];
#define ctx_HIT_Y           DP(0x310)

    rt_real hit_z[4];
#define ctx_HIT_Z           DP(0x320)


    rt_real t_new[4];
#define ctx_T_NEW           DP(0x330)

    /* new */

#define ctx_NEW_O           DP(0x340)

    rt_real new_x[4];
#define ctx_NEW_X           DP(0x340)

    rt_real new_y[4];
#define ctx_NEW_Y           DP(0x350)

    rt_real new_z[4];
#define ctx_NEW_Z           DP(0x360)


    rt_word pad05[12];
#define ctx_PAD05           DP(0x370)

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

    /* reserved area 2 */

    rt_word pad02[56];
#define srf_PAD02           DP(0x120)

};

/******************************************************************************/
/**********************************   PLANE   *********************************/
/******************************************************************************/

struct rt_SIMD_PLANE: public rt_SIMD_SURFACE
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
/********************************   MATERIAL   ********************************/
/******************************************************************************/

struct rt_SIMD_MATERIAL
{
    rt_pntr tex_p[4];
#define mat_TEX_P           DP(0x000)

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
