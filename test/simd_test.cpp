/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <math.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "rtarch.h"
#include "rtbase.h"

#define RUN_LEVEL           12
#define VERBOSE             RT_FALSE
#define CYC_SIZE            1000000

#define ARR_SIZE            12 /* hardcoded in asm sections */
#define MASK                (RT_SIMD_ALIGN - 1) /* SIMD alignment mask */

#define FRK(f)              (f < 10.0       ? 1.0           :               \
                             f < 100.0      ? 10.0          :               \
                             f < 1000.0     ? 100.0         :               \
                             f < 10000.0    ? 1000.0        :               \
                             f < 100000.0   ? 10000.0       :               \
                             f < 1000000.0  ? 100000.0      :               \
                                              1000000.0)

#define FEQ(f1, f2)         (fabs(f1 - f2) < 0.0002 * RT_MIN(FRK(f1), FRK(f2)))
#define IEQ(i1, i2)         (i1 == i2)

#define RT_LOGI             printf
#define RT_LOGE             printf

/*
 * Extended SIMD info structure for asm enter/leave.
 * Serves as container for test arrays and internal varibales.
 * Displacements start where rt_SIMD_INFO ends (at 0x100).
 */
struct rt_SIMD_INFOX : public rt_SIMD_INFO
{
    /* floating point arrays */

    rt_real*far0;
#define inf_FAR0            DP(0x100)

    rt_real*fco1;
#define inf_FCO1            DP(0x104)

    rt_real*fco2;
#define inf_FCO2            DP(0x108)

    rt_real*fso1;
#define inf_FSO1            DP(0x10C)

    rt_real*fso2;
#define inf_FSO2            DP(0x110)

    /* integer arrays */

    rt_cell*iar0;
#define inf_IAR0            DP(0x114)

    rt_cell*ico1;
#define inf_ICO1            DP(0x118)

    rt_cell*ico2;
#define inf_ICO2            DP(0x11C)

    rt_cell*iso1;
#define inf_ISO1            DP(0x120)

    rt_cell*iso2;
#define inf_ISO2            DP(0x124)

    /* internal variables */

    rt_cell cyc;
#define inf_CYC             DP(0x128)

    rt_cell tmp;
#define inf_TMP             DP(0x12C)

    rt_cell size;
#define inf_SIZE            DP(0x130)

    rt_pntr label;
#define inf_LABEL           DP(0x134)

};

/******************************************************************************/
/******************************   RUN LEVEL  1   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  1

rt_void c_test01(rt_SIMD_INFOX *info)
{
    rt_cell i, j, n = info->size;
    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            fco1[j] = far0[j] + far0[(j + 4) % n];
            fco2[j] = far0[j] - far0[(j + 4) % n];
        }
    }
}

rt_void s_test01(rt_SIMD_INFOX *info)
{
    rt_cell i;

#define AJ0                 DP(0x000)
#define AJ1                 DP(0x010)
#define AJ2                 DP(0x020)

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_FSO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        movpx_ld(Xmm0, Mecx, AJ0)
        movpx_ld(Xmm1, Mecx, AJ1)
        movpx_rr(Xmm2, Xmm0)
        addps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        subps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_ld(Xmm1, Mecx, AJ2)
        movpx_rr(Xmm2, Xmm0)
        addps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        subps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        addps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        subps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void p_test01(rt_SIMD_INFOX *info)
{
    rt_cell j, n;

    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;
    rt_real *fso1 = info->fso1;
    rt_real *fso2 = info->fso2;

    j = n = info->size;
    while (j-->0)
    {
        if (FEQ(fco1[j], fso1[j]) && FEQ(fco2[j], fso2[j]) && !VERBOSE)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + 4) % n, far0[(j + 4) % n]);

        RT_LOGI("C farr[%d]+farr[%d] = %e, farr[%d]-farr[%d] = %e\n",
                j, (j + 4) % n, fco1[j], j, (j + 4) % n, fco2[j]);

        RT_LOGI("S farr[%d]+farr[%d] = %e, farr[%d]-farr[%d] = %e\n",
                j, (j + 4) % n, fso1[j], j, (j + 4) % n, fso2[j]);
    }
}

#endif /* RUN_LEVEL  1 */

/******************************************************************************/
/******************************   RUN LEVEL  2   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  2

rt_void c_test02(rt_SIMD_INFOX *info)
{
    rt_cell i, j, n = info->size;
    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            fco1[j] = far0[j] * far0[(j + 4) % n];
            fco2[j] = far0[j] / far0[(j + 4) % n];
        }
    }
}

rt_void s_test02(rt_SIMD_INFOX *info)
{
    rt_cell i;

#define AJ0                 DP(0x000)
#define AJ1                 DP(0x010)
#define AJ2                 DP(0x020)

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_FSO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        movpx_ld(Xmm0, Mecx, AJ0)
        movpx_ld(Xmm1, Mecx, AJ1)
        movpx_rr(Xmm2, Xmm0)
        mulps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        divps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_ld(Xmm1, Mecx, AJ2)
        movpx_rr(Xmm2, Xmm0)
        mulps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        divps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        mulps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        divps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void p_test02(rt_SIMD_INFOX *info)
{
    rt_cell j, n;

    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;
    rt_real *fso1 = info->fso1;
    rt_real *fso2 = info->fso2;

    j = n = info->size;
    while (j-->0)
    {
        if (FEQ(fco1[j], fso1[j]) && FEQ(fco2[j], fso2[j]) && !VERBOSE)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + 4) % n, far0[(j + 4) % n]);

        RT_LOGI("C farr[%d]*farr[%d] = %e, farr[%d]/farr[%d] = %e\n",
                j, (j + 4) % n, fco1[j], j, (j + 4) % n, fco2[j]);

        RT_LOGI("S farr[%d]*farr[%d] = %e, farr[%d]/farr[%d] = %e\n",
                j, (j + 4) % n, fso1[j], j, (j + 4) % n, fso2[j]);

    }
}

#endif /* RUN_LEVEL  2 */

/******************************************************************************/
/******************************   RUN LEVEL  3   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  3

rt_void c_test03(rt_SIMD_INFOX *info)
{
    rt_cell i, j, n = info->size;
    rt_real *far0 = info->far0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = (far0[j] >  far0[(j + 4) % n]) ? 0xFFFFFFFF : 0x00000000;
            ico2[j] = (far0[j] >= far0[(j + 4) % n]) ? 0xFFFFFFFF : 0x00000000;
        }
    }
}

rt_void s_test03(rt_SIMD_INFOX *info)
{
    rt_cell i;

#define AJ0                 DP(0x000)
#define AJ1                 DP(0x010)
#define AJ2                 DP(0x020)

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movpx_ld(Xmm0, Mecx, AJ0)
        movpx_ld(Xmm1, Mecx, AJ1)
        movpx_rr(Xmm2, Xmm0)
        cgtps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cgeps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_ld(Xmm1, Mecx, AJ2)
        movpx_rr(Xmm2, Xmm0)
        cgtps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cgeps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        cgtps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cgeps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void p_test03(rt_SIMD_INFOX *info)
{
    rt_cell j, n;

    rt_real *far0 = info->far0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;
    rt_cell *iso1 = info->iso1;
    rt_cell *iso2 = info->iso2;

    j = n = info->size;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !VERBOSE)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + 4) % n, far0[(j + 4) % n]);

        RT_LOGI("C (farr[%d] > farr[%d]) = %X, (farr[%d] >= farr[%d]) = %X\n",
                j, (j + 4) % n, ico1[j], j, (j + 4) % n, ico2[j]);

        RT_LOGI("S (farr[%d] > farr[%d]) = %X, (farr[%d] >= farr[%d]) = %X\n",
                j, (j + 4) % n, iso1[j], j, (j + 4) % n, iso2[j]);
    }
}

#endif /* RUN_LEVEL  3 */

/******************************************************************************/
/******************************   RUN LEVEL  4   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  4

rt_void c_test04(rt_SIMD_INFOX *info)
{
    rt_cell i, j, n = info->size;
    rt_real *far0 = info->far0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = (far0[j] <  far0[(j + 4) % n]) ? 0xFFFFFFFF : 0x00000000;
            ico2[j] = (far0[j] <= far0[(j + 4) % n]) ? 0xFFFFFFFF : 0x00000000;
        }
    }
}

rt_void s_test04(rt_SIMD_INFOX *info)
{
    rt_cell i;

#define AJ0                 DP(0x000)
#define AJ1                 DP(0x010)
#define AJ2                 DP(0x020)

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movpx_ld(Xmm0, Mecx, AJ0)
        movpx_ld(Xmm1, Mecx, AJ1)
        movpx_rr(Xmm2, Xmm0)
        cltps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cleps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_ld(Xmm1, Mecx, AJ2)
        movpx_rr(Xmm2, Xmm0)
        cltps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cleps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        cltps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cleps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void p_test04(rt_SIMD_INFOX *info)
{
    rt_cell j, n;

    rt_real *far0 = info->far0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;
    rt_cell *iso1 = info->iso1;
    rt_cell *iso2 = info->iso2;

    j = n = info->size;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !VERBOSE)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + 4) % n, far0[(j + 4) % n]);

        RT_LOGI("C (farr[%d] < farr[%d]) = %X, (farr[%d] <= farr[%d]) = %X\n",
                j, (j + 4) % n, ico1[j], j, (j + 4) % n, ico2[j]);

        RT_LOGI("S (farr[%d] < farr[%d]) = %X, (farr[%d] <= farr[%d]) = %X\n",
                j, (j + 4) % n, iso1[j], j, (j + 4) % n, iso2[j]);
    }
}

#endif /* RUN_LEVEL  4 */

/******************************************************************************/
/******************************   RUN LEVEL  5   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  5

rt_void c_test05(rt_SIMD_INFOX *info)
{
    rt_cell i, j, n = info->size;
    rt_real *far0 = info->far0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = (far0[j] == far0[(j + 4) % n]) ? 0xFFFFFFFF : 0x00000000;
            ico2[j] = (far0[j] != far0[(j + 4) % n]) ? 0xFFFFFFFF : 0x00000000;
        }
    }
}

rt_void s_test05(rt_SIMD_INFOX *info)
{
    rt_cell i;

#define AJ0                 DP(0x000)
#define AJ1                 DP(0x010)
#define AJ2                 DP(0x020)

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movpx_ld(Xmm0, Mecx, AJ0)
        movpx_ld(Xmm1, Mecx, AJ1)
        movpx_rr(Xmm2, Xmm0)
        ceqps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cneps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_ld(Xmm1, Mecx, AJ2)
        movpx_rr(Xmm2, Xmm0)
        ceqps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cneps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        ceqps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        cneps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void p_test05(rt_SIMD_INFOX *info)
{
    rt_cell j, n;

    rt_real *far0 = info->far0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;
    rt_cell *iso1 = info->iso1;
    rt_cell *iso2 = info->iso2;

    j = n = info->size;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !VERBOSE)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + 4) % n, far0[(j + 4) % n]);

        RT_LOGI("C (farr[%d] == farr[%d]) = %X, (farr[%d] != farr[%d]) = %X\n",
                j, (j + 4) % n, ico1[j], j, (j + 4) % n, ico2[j]);

        RT_LOGI("S (farr[%d] == farr[%d]) = %X, (farr[%d] != farr[%d]) = %X\n",
                j, (j + 4) % n, iso1[j], j, (j + 4) % n, iso2[j]);
    }
}

#endif /* RUN_LEVEL  5 */

/******************************************************************************/
/******************************   RUN LEVEL  6   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  6

rt_void c_test06(rt_SIMD_INFOX *info)
{
    rt_cell i, j, n = info->size;
    rt_real *far0 = info->far0;
    rt_cell *iar0 = info->iar0;
    rt_cell *ico1 = info->ico1;
    rt_real *fco2 = info->fco2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = (rt_cell)far0[j];
            fco2[j] = (rt_real)iar0[j];
        }
    }
}

rt_void s_test06(rt_SIMD_INFOX *info)
{
    rt_cell i;

#define AJ0                 DP(0x000)
#define AJ1                 DP(0x010)
#define AJ2                 DP(0x020)

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        FCTRL_ENTER(ROUNDM)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Resi, Mebp, inf_IAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        movpx_ld(Xmm0, Mecx, AJ0)
        movpx_ld(Xmm1, Mesi, AJ0)
        cvtps_rr(Xmm2, Xmm0)
        cvtpn_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_ld(Xmm1, Mesi, AJ1)
        cvtps_rr(Xmm2, Xmm0)
        cvtpn_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mesi, AJ2)
        cvtps_rr(Xmm2, Xmm0)
        cvtpn_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        FCTRL_LEAVE(ROUNDM)

        ASM_LEAVE(info)
    }
}

rt_void p_test06(rt_SIMD_INFOX *info)
{
    rt_cell j, n;

    rt_real *far0 = info->far0;
    rt_cell *iar0 = info->iar0;
    rt_cell *ico1 = info->ico1;
    rt_real *fco2 = info->fco2;
    rt_cell *iso1 = info->iso1;
    rt_real *fso2 = info->fso2;

    j = n = info->size;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && FEQ(fco2[j], fso2[j]) && !VERBOSE)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, iarr[%d] = %d\n",
                j, far0[j], j, iar0[j]);

        RT_LOGI("C (rt_cell)farr[%d] = %d, (rt_real)iarr[%d] = %e\n",
                j, ico1[j], j, fco2[j]);

        RT_LOGI("S (rt_cell)farr[%d] = %d, (rt_real)iarr[%d] = %e\n",
                j, iso1[j], j, fso2[j]);
    }
}

#endif /* RUN_LEVEL  6 */

/******************************************************************************/
/******************************   RUN LEVEL  7   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  7

rt_void c_test07(rt_SIMD_INFOX *info)
{
    rt_cell i, j, n = info->size;
    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            fco1[j] = sqrtf(far0[j]);
            fco2[j] = 1.0f / far0[j];
        }
    }
}

rt_void s_test07(rt_SIMD_INFOX *info)
{
    rt_cell i;

#define AJ0                 DP(0x000)
#define AJ1                 DP(0x010)
#define AJ2                 DP(0x020)

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_FSO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        movpx_ld(Xmm0, Mecx, AJ0)
        movpx_rr(Xmm1, Xmm0)
        rsqps_rr(Xmm2, Xmm0) /* destroys Xmm0 */
        mulps_rr(Xmm2, Xmm1)
        rcpps_rr(Xmm3, Xmm1) /* destroys Xmm1 */
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_rr(Xmm1, Xmm0)
        rsqps_rr(Xmm2, Xmm0) /* destroys Xmm0 */
        mulps_rr(Xmm2, Xmm1)
        rcpps_rr(Xmm3, Xmm1) /* destroys Xmm1 */
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_rr(Xmm1, Xmm0)
        rsqps_rr(Xmm2, Xmm0) /* destroys Xmm0 */
        mulps_rr(Xmm2, Xmm1)
        rcpps_rr(Xmm3, Xmm1) /* destroys Xmm1 */
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void p_test07(rt_SIMD_INFOX *info)
{
    rt_cell j, n;

    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;
    rt_real *fso1 = info->fso1;
    rt_real *fso2 = info->fso2;

    j = n = info->size;
    while (j-->0)
    {
        if (FEQ(fco1[j], fso1[j]) && FEQ(fco2[j], fso2[j]) && !VERBOSE)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e\n",
                j, far0[j]);

        RT_LOGI("C sqrt(farr[%d]) = %e, 1.0/farr[%d] = %e\n",
                j, fco1[j], j, fco2[j]);

        RT_LOGI("S sqrt(farr[%d]) = %e, 1.0/farr[%d] = %e\n",
                j, fso1[j], j, fso2[j]);
    }
}

#endif /* RUN_LEVEL  7 */

/******************************************************************************/
/******************************   RUN LEVEL  8   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  8

rt_void c_test08(rt_SIMD_INFOX *info)
{
    rt_cell i, j, n = info->size;
    rt_cell *iar0 = info->iar0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = iar0[j] + (iar0[j] << 1);
            ico2[j] = iar0[j] - (iar0[j] >> 2);
        }
    }
}

rt_void s_test08(rt_SIMD_INFOX *info)
{
    rt_cell i;

#define AJ0                 DP(0x000)
#define AJ1                 DP(0x010)
#define AJ2                 DP(0x020)

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Resi, Mebp, inf_IAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movpx_ld(Xmm0, Mesi, AJ0)
        movpx_ld(Xmm1, Mesi, AJ0)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(1))
        addpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpx_ri(Xmm1, IB(2))
        subpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mesi, AJ1)
        movpx_ld(Xmm1, Mesi, AJ1)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(1))
        addpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpx_ri(Xmm1, IB(2))
        subpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mesi, AJ2)
        movpx_ld(Xmm1, Mesi, AJ2)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(1))
        addpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpx_ri(Xmm1, IB(2))
        subpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void p_test08(rt_SIMD_INFOX *info)
{
    rt_cell j, n;

    rt_cell *iar0 = info->iar0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;
    rt_cell *iso1 = info->iso1;
    rt_cell *iso2 = info->iso2;

    j = n = info->size;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !VERBOSE)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %d\n",
                j, iar0[j]);

        RT_LOGI("C iarr[%d]+(iarr[%d]<<1) = %d, iarr[%d]+(iarr[%d]>>2) = %d\n",
                j, j, ico1[j], j, j, ico2[j]);

        RT_LOGI("S iarr[%d]+(iarr[%d]<<1) = %d, iarr[%d]+(iarr[%d]>>2) = %d\n",
                j, j, iso1[j], j, j, iso2[j]);

    }
}

#endif /* RUN_LEVEL  8 */

/******************************************************************************/
/******************************   RUN LEVEL  9   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  9

rt_void c_test09(rt_SIMD_INFOX *info)
{
    rt_cell i, j, n = info->size;
    rt_cell *iar0 = info->iar0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = iar0[j] * iar0[(j + 4) % n];
            ico2[j] = iar0[j] / iar0[(j + 4) % n];
        }
    }
}

rt_void s_test09(rt_SIMD_INFOX *info)
{
    ASM_ENTER(info)

        label_ld(cyc_beg) /* load to Reax */
        movxx_st(Reax, Mebp, inf_LABEL)

        movxx_ld(Reax, Mebp, inf_CYC)
        movxx_st(Reax, Mebp, inf_TMP)

    LBL(cyc_beg)

        movxx_ld(Recx, Mebp, inf_IAR0)
        movxx_ld(Rebx, Mebp, inf_ISO1)
        movxx_ld(Resi, Mebp, inf_ISO2)
        movxx_ld(Redi, Mebp, inf_SIZE)

    LBL(loc_beg)

        movxx_ld(Reax, Mecx, DP(0x00))
        mulxn_xm(Mecx, DP(0x10))
        movxx_st(Reax, Mebx, DP(0x00))
        movxx_ld(Reax, Mecx, DP(0x00))
        movxx_ri(Redx, IB(0))
        divxn_xm(Mecx, DP(0x10))
        movxx_st(Reax, Mesi, DP(0x00))

        addxx_ri(Recx, IB(4))
        addxx_ri(Rebx, IB(4))
        addxx_ri(Resi, IB(4))
        subxx_ri(Redi, IB(1))
        cmpxx_ri(Redi, IB(4))
        jgtxx_lb(loc_beg)

        movxx_ld(Redi, Mebp, inf_IAR0)

        movxx_ld(Reax, Mecx, DP(0x00))
        mulxn_xm(Medi, DP(0x00))
        movxx_st(Reax, Mebx, DP(0x00))
        movxx_ld(Reax, Mecx, DP(0x00))
        movxx_ri(Redx, IB(0))
        divxn_xm(Medi, DP(0x00))
        movxx_st(Reax, Mesi, DP(0x00))

        movxx_ld(Reax, Mecx, DP(0x04))
        mulxn_xm(Medi, DP(0x04))
        movxx_st(Reax, Mebx, DP(0x04))
        movxx_ld(Reax, Mecx, DP(0x04))
        movxx_ri(Redx, IB(0))
        divxn_xm(Medi, DP(0x04))
        movxx_st(Reax, Mesi, DP(0x04))

        movxx_ld(Reax, Mecx, DP(0x08))
        mulxn_xm(Medi, DP(0x08))
        movxx_st(Reax, Mebx, DP(0x08))
        movxx_ld(Reax, Mecx, DP(0x08))
        movxx_ri(Redx, IB(0))
        divxn_xm(Medi, DP(0x08))
        movxx_st(Reax, Mesi, DP(0x08))

        movxx_ld(Reax, Mecx, DP(0x0C))
        mulxn_xm(Medi, DP(0x0C))
        movxx_st(Reax, Mebx, DP(0x0C))
        movxx_ld(Reax, Mecx, DP(0x0C))
        movxx_ri(Redx, IB(0))
        divxn_xm(Medi, DP(0x0C))
        movxx_st(Reax, Mesi, DP(0x0C))

        subxx_mi(Mebp, inf_TMP, IB(1))
        cmpxx_mi(Mebp, inf_TMP, IB(0))
        jeqxx_lb(cyc_end)
        jmpxx_mm(Mebp, inf_LABEL)
        jmpxx_lb(cyc_beg) /* the same jump as above */

    LBL(cyc_end)

    ASM_LEAVE(info)
}

rt_void p_test09(rt_SIMD_INFOX *info)
{
    rt_cell j, n;

    rt_cell *iar0 = info->iar0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;
    rt_cell *iso1 = info->iso1;
    rt_cell *iso2 = info->iso2;

    j = n = info->size;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !VERBOSE)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %d, iarr[%d] = %d\n",
                j, iar0[j], (j + 4) % n, iar0[(j + 4) % n]);

        RT_LOGI("C iarr[%d]*iarr[%d] = %d, iarr[%d]/iarr[%d] = %d\n",
                j, (j + 4) % n, ico1[j], j, (j + 4) % n, ico2[j]);

        RT_LOGI("S iarr[%d]*iarr[%d] = %d, iarr[%d]/iarr[%d] = %d\n",
                j, (j + 4) % n, iso1[j], j, (j + 4) % n, iso2[j]);

    }
}

#endif /* RUN_LEVEL  9 */

/******************************************************************************/
/******************************   RUN LEVEL 10   ******************************/
/******************************************************************************/

#if RUN_LEVEL >= 10

rt_void c_test10(rt_SIMD_INFOX *info)
{
    rt_cell i, j, n = info->size;
    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            fco1[j] = RT_MIN(far0[j], far0[(j + 4) % n]);
            fco2[j] = RT_MAX(far0[j], far0[(j + 4) % n]);
        }
    }
}

rt_void s_test10(rt_SIMD_INFOX *info)
{
    rt_cell i;

#define AJ0                 DP(0x000)
#define AJ1                 DP(0x010)
#define AJ2                 DP(0x020)

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_FSO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        movpx_ld(Xmm0, Mecx, AJ0)
        movpx_ld(Xmm1, Mecx, AJ1)
        movpx_rr(Xmm2, Xmm0)
        minps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        maxps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mecx, AJ1)
        movpx_ld(Xmm1, Mecx, AJ2)
        movpx_rr(Xmm2, Xmm0)
        minps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        maxps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mecx, AJ2)
        movpx_ld(Xmm1, Mecx, AJ0)
        movpx_rr(Xmm2, Xmm0)
        minps_rr(Xmm2, Xmm1)
        movpx_rr(Xmm3, Xmm0)
        maxps_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void p_test10(rt_SIMD_INFOX *info)
{
    rt_cell j, n;

    rt_real *far0 = info->far0;
    rt_real *fco1 = info->fco1;
    rt_real *fco2 = info->fco2;
    rt_real *fso1 = info->fso1;
    rt_real *fso2 = info->fso2;

    j = n = info->size;
    while (j-->0)
    {
        if (FEQ(fco1[j], fso1[j]) && FEQ(fco2[j], fso2[j]) && !VERBOSE)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, farr[%d] = %e\n",
                j, far0[j], (j + 4) % n, far0[(j + 4) % n]);

        RT_LOGI("C MIN(farr[%d],farr[%d]) = %e, MAX(farr[%d],farr[%d]) = %e\n",
                j, (j + 4) % n, fco1[j], j, (j + 4) % n, fco2[j]);

        RT_LOGI("S MIN(farr[%d],farr[%d]) = %e, MAX(farr[%d],farr[%d]) = %e\n",
                j, (j + 4) % n, fso1[j], j, (j + 4) % n, fso2[j]);
    }
}

#endif /* RUN_LEVEL 10 */

/******************************************************************************/
/******************************   RUN LEVEL 11   ******************************/
/******************************************************************************/

#if RUN_LEVEL >= 11

rt_void c_test11(rt_SIMD_INFOX *info)
{
    rt_cell i, j, n = info->size;
    rt_cell *iar0 = info->iar0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = iar0[j] | (iar0[j] << 7);
            ico2[j] = iar0[j] ^ (iar0[j] >> 3);
        }
    }
}

rt_void s_test11(rt_SIMD_INFOX *info)
{
    rt_cell i;

#define AJ0                 DP(0x000)
#define AJ1                 DP(0x010)
#define AJ2                 DP(0x020)

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Resi, Mebp, inf_IAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movpx_ld(Xmm0, Mesi, AJ0)
        movpx_ld(Xmm1, Mesi, AJ0)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(7))
        orrpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpx_ri(Xmm1, IB(3))
        xorpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mesi, AJ1)
        movpx_ld(Xmm1, Mesi, AJ1)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(7))
        orrpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpx_ri(Xmm1, IB(3))
        xorpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mesi, AJ2)
        movpx_ld(Xmm1, Mesi, AJ2)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(7))
        orrpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpx_ri(Xmm1, IB(3))
        xorpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void p_test11(rt_SIMD_INFOX *info)
{
    rt_cell j, n;

    rt_cell *iar0 = info->iar0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;
    rt_cell *iso1 = info->iso1;
    rt_cell *iso2 = info->iso2;

    j = n = info->size;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !VERBOSE)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %d\n",
                j, iar0[j]);

        RT_LOGI("C iarr[%d]|(iarr[%d]<<7) = %d, iarr[%d]^(iarr[%d]>>3) = %d\n",
                j, j, ico1[j], j, j, ico2[j]);

        RT_LOGI("S iarr[%d]|(iarr[%d]<<7) = %d, iarr[%d]^(iarr[%d]>>3) = %d\n",
                j, j, iso1[j], j, j, iso2[j]);

    }
}

#endif /* RUN_LEVEL 11 */

/******************************************************************************/
/******************************   RUN LEVEL 12   ******************************/
/******************************************************************************/

#if RUN_LEVEL >= 12

rt_void c_test12(rt_SIMD_INFOX *info)
{
    rt_cell i, j, n = info->size;
    rt_cell *iar0 = info->iar0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;

    i = info->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] =  iar0[j] & (iar0[j] << 7);
            ico2[j] = ~iar0[j] & (iar0[j] >> 3);
        }
    }
}

rt_void s_test12(rt_SIMD_INFOX *info)
{
    rt_cell i;

#define AJ0                 DP(0x000)
#define AJ1                 DP(0x010)
#define AJ2                 DP(0x020)

    i = info->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Resi, Mebp, inf_IAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)


        movpx_ld(Xmm0, Mesi, AJ0)
        movpx_ld(Xmm1, Mesi, AJ0)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(7))
        andpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpx_ri(Xmm1, IB(3))
        annpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ0)
        movpx_st(Xmm3, Mebx, AJ0)

        movpx_ld(Xmm0, Mesi, AJ1)
        movpx_ld(Xmm1, Mesi, AJ1)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(7))
        andpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpx_ri(Xmm1, IB(3))
        annpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ1)
        movpx_st(Xmm3, Mebx, AJ1)

        movpx_ld(Xmm0, Mesi, AJ2)
        movpx_ld(Xmm1, Mesi, AJ2)
        movpx_rr(Xmm2, Xmm0)
        shlpx_ri(Xmm0, IB(7))
        andpx_rr(Xmm2, Xmm0)
        movpx_rr(Xmm3, Xmm1)
        shrpx_ri(Xmm1, IB(3))
        annpx_rr(Xmm3, Xmm1)
        movpx_st(Xmm2, Medx, AJ2)
        movpx_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void p_test12(rt_SIMD_INFOX *info)
{
    rt_cell j, n;

    rt_cell *iar0 = info->iar0;
    rt_cell *ico1 = info->ico1;
    rt_cell *ico2 = info->ico2;
    rt_cell *iso1 = info->iso1;
    rt_cell *iso2 = info->iso2;

    j = n = info->size;
    while (j-->0)
    {
        if (IEQ(ico1[j], iso1[j]) && IEQ(ico2[j], iso2[j]) && !VERBOSE)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %d\n",
                j, iar0[j]);

        RT_LOGI("C iarr[%d]&(iarr[%d]<<7) = %d, ~iarr[%d]&(iarr[%d]>>3) = %d\n",
                j, j, ico1[j], j, j, ico2[j]);

        RT_LOGI("S iarr[%d]&(iarr[%d]<<7) = %d, ~iarr[%d]&(iarr[%d]>>3) = %d\n",
                j, j, iso1[j], j, j, iso2[j]);

    }
}

#endif /* RUN_LEVEL 12 */

/******************************************************************************/
/*********************************   TABLES   *********************************/
/******************************************************************************/

typedef rt_void (*testXX)(rt_SIMD_INFOX *);

testXX c_test[RUN_LEVEL] =
{
#if RUN_LEVEL >=  1
    c_test01,
#endif /* RUN_LEVEL  1 */

#if RUN_LEVEL >=  2
    c_test02,
#endif /* RUN_LEVEL  2 */

#if RUN_LEVEL >=  3
    c_test03,
#endif /* RUN_LEVEL  3 */

#if RUN_LEVEL >=  4
    c_test04,
#endif /* RUN_LEVEL  4 */

#if RUN_LEVEL >=  5
    c_test05,
#endif /* RUN_LEVEL  5 */

#if RUN_LEVEL >=  6
    c_test06,
#endif /* RUN_LEVEL  6 */

#if RUN_LEVEL >=  7
    c_test07,
#endif /* RUN_LEVEL  7 */

#if RUN_LEVEL >=  8
    c_test08,
#endif /* RUN_LEVEL  8 */

#if RUN_LEVEL >=  9
    c_test09,
#endif /* RUN_LEVEL  9 */

#if RUN_LEVEL >= 10
    c_test10,
#endif /* RUN_LEVEL 10 */

#if RUN_LEVEL >= 11
    c_test11,
#endif /* RUN_LEVEL 11 */

#if RUN_LEVEL >= 12
    c_test12,
#endif /* RUN_LEVEL 12 */
};

testXX s_test[RUN_LEVEL] =
{
#if RUN_LEVEL >=  1
    s_test01,
#endif /* RUN_LEVEL  1 */

#if RUN_LEVEL >=  2
    s_test02,
#endif /* RUN_LEVEL  2 */

#if RUN_LEVEL >=  3
    s_test03,
#endif /* RUN_LEVEL  3 */

#if RUN_LEVEL >=  4
    s_test04,
#endif /* RUN_LEVEL  4 */

#if RUN_LEVEL >=  5
    s_test05,
#endif /* RUN_LEVEL  5 */

#if RUN_LEVEL >=  6
    s_test06,
#endif /* RUN_LEVEL  6 */

#if RUN_LEVEL >=  7
    s_test07,
#endif /* RUN_LEVEL  7 */

#if RUN_LEVEL >=  8
    s_test08,
#endif /* RUN_LEVEL  8 */

#if RUN_LEVEL >=  9
    s_test09,
#endif /* RUN_LEVEL  9 */

#if RUN_LEVEL >= 10
    s_test10,
#endif /* RUN_LEVEL 10 */

#if RUN_LEVEL >= 11
    s_test11,
#endif /* RUN_LEVEL 11 */

#if RUN_LEVEL >= 12
    s_test12,
#endif /* RUN_LEVEL 12 */
};

testXX p_test[RUN_LEVEL] =
{
#if RUN_LEVEL >=  1
    p_test01,
#endif /* RUN_LEVEL  1 */

#if RUN_LEVEL >=  2
    p_test02,
#endif /* RUN_LEVEL  2 */

#if RUN_LEVEL >=  3
    p_test03,
#endif /* RUN_LEVEL  3 */

#if RUN_LEVEL >=  4
    p_test04,
#endif /* RUN_LEVEL  4 */

#if RUN_LEVEL >=  5
    p_test05,
#endif /* RUN_LEVEL  5 */

#if RUN_LEVEL >=  6
    p_test06,
#endif /* RUN_LEVEL  6 */

#if RUN_LEVEL >=  7
    p_test07,
#endif /* RUN_LEVEL  7 */

#if RUN_LEVEL >=  8
    p_test08,
#endif /* RUN_LEVEL  8 */

#if RUN_LEVEL >=  9
    p_test09,
#endif /* RUN_LEVEL  9 */

#if RUN_LEVEL >= 10
    p_test10,
#endif /* RUN_LEVEL 10 */

#if RUN_LEVEL >= 11
    p_test11,
#endif /* RUN_LEVEL 11 */

#if RUN_LEVEL >= 12
    p_test12,
#endif /* RUN_LEVEL 12 */
};

/******************************************************************************/
/**********************************   MAIN   **********************************/
/******************************************************************************/

rt_long get_time();

/*
 * info - info original pointer
 * inf0 - info aligned pointer
 * marr - memory original pointer
 * mar0 - memory aligned pointer
 *
 * farr - float original array
 * far0 - float aligned array 0
 * fco1 - float aligned C out 1
 * fco2 - float aligned C out 2
 * fso1 - float aligned S out 1
 * fso2 - float aligned S out 2
 *
 * iarr - int original array
 * iar0 - int aligned array 0
 * ico1 - int aligned C out 1
 * ico2 - int aligned C out 2
 * iso1 - int aligned S out 1
 * iso2 - int aligned S out 2
 */
rt_cell main()
{
    rt_pntr marr = malloc(10 * ARR_SIZE * sizeof(rt_word) + MASK);
    rt_pntr mar0 = (rt_pntr)(((rt_word)marr + MASK) & ~MASK);

    rt_real farr[ARR_SIZE] =
    {
        34.2785,
        113.98764,
        0.65434,
        1.4687656,
        0.0032467,
        2.175953,
        0.65434,
        578986.23,
        8764.7534,
        113.98764,
        0.0765376,
        43187.487,
    };

    rt_real *far0 = (rt_real *)mar0 + ARR_SIZE * 0;
    rt_real *fco1 = (rt_real *)mar0 + ARR_SIZE * 1;
    rt_real *fco2 = (rt_real *)mar0 + ARR_SIZE * 2;
    rt_real *fso1 = (rt_real *)mar0 + ARR_SIZE * 3;
    rt_real *fso2 = (rt_real *)mar0 + ARR_SIZE * 4;

    memcpy(far0, farr, sizeof(farr));

    rt_cell iarr[ARR_SIZE] =
    {
        285,
        113,
        65,
        14,
        3,
        1,
        7,
        57896,
        2347875,
        87647531,
        7665,
        318773,
    };

    rt_cell *iar0 = (rt_cell *)mar0 + ARR_SIZE * 5;
    rt_cell *ico1 = (rt_cell *)mar0 + ARR_SIZE * 6;
    rt_cell *ico2 = (rt_cell *)mar0 + ARR_SIZE * 7;
    rt_cell *iso1 = (rt_cell *)mar0 + ARR_SIZE * 8;
    rt_cell *iso2 = (rt_cell *)mar0 + ARR_SIZE * 9;

    memcpy(iar0, iarr, sizeof(iarr));

    rt_pntr info = malloc(sizeof(rt_SIMD_INFOX) + MASK);
    rt_SIMD_INFOX *inf0 = (rt_SIMD_INFOX *)(((rt_word)info + MASK) & ~MASK);

    RT_SIMD_SET(inf0->gpc01, +1.0f);
    RT_SIMD_SET(inf0->gpc02, -0.5f);
    RT_SIMD_SET(inf0->gpc03, +3.0f);

    inf0->far0 = far0;
    inf0->fco1 = fco1;
    inf0->fco2 = fco2;
    inf0->fso1 = fso1;
    inf0->fso2 = fso2;

    inf0->iar0 = iar0;
    inf0->ico1 = ico1;
    inf0->ico2 = ico2;
    inf0->iso1 = iso1;
    inf0->iso2 = iso2;

    inf0->cyc  = CYC_SIZE;
    inf0->size = ARR_SIZE;

    rt_long time1 = 0;
    rt_long time2 = 0;
    rt_long tC = 0;
    rt_long tS = 0;

    rt_cell i;

    for (i = 0; i < RUN_LEVEL; i++)
    {
        RT_LOGI("-----------------  RUN LEVEL = %2d  -----------------\n", i+1);

        time1 = get_time();

        c_test[i](inf0);

        time2 = get_time();
        tC = time2 - time1;
        RT_LOGI("Time C = %d\n", (rt_cell)tC);

        /* --------------------------------- */

        time1 = get_time();

        s_test[i](inf0);

        time2 = get_time();
        tS = time2 - time1;
        RT_LOGI("Time S = %d\n", (rt_cell)tS);

        /* --------------------------------- */

        p_test[i](inf0);

        RT_LOGI("----------------------------------------------------\n");
    }

    free(info);
    free(marr);

#if   defined (RT_WIN32) /* Win32, MSVC ------------------------------------- */

    RT_LOGI("Type any letter and press ENTER to exit:");
    rt_char str[256]; /* not secure, do not inherit this practice */
    scanf("%s", str); /* not secure, do not inherit this practice */

#endif /* ------------- OS specific ----------------------------------------- */

    return 0;
}

#if   defined (RT_WIN32) /* Win32, MSVC ------------------------------------- */

#include <windows.h>

rt_long get_time()
{
    LARGE_INTEGER fr;
    QueryPerformanceFrequency(&fr);
    LARGE_INTEGER tm;
    QueryPerformanceCounter(&tm);
    return (rt_long)(tm.QuadPart * 1000 / fr.QuadPart);
}

#elif defined (RT_LINUX) /* Linux, GCC -------------------------------------- */

#include <sys/time.h>

rt_long get_time()
{
    timeval tm;
    gettimeofday(&tm, NULL);
    return (rt_long)(tm.tv_sec * 1000 + tm.tv_usec / 1000);
}

#endif /* ------------- OS specific ----------------------------------------- */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
