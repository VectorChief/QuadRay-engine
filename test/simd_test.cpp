/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <math.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "rtarch.h"
#include "rtbase.h"

#define RUN_LEVEL       9
#define VERBOSE         RT_FALSE
#define CYC_SIZE        1000000

#define ARR_SIZE        12 /* hardcoded in asm sections */
#define MASK            15 /* hardcoded quad alignment */


struct rt_INFO
{
    rt_real*far0;
#define inf_FAR0        DP(0x0000)

    rt_real*fco1;
#define inf_FCO1        DP(0x0004)

    rt_real*fco2;
#define inf_FCO2        DP(0x0008)

    rt_real*fso1;
#define inf_FSO1        DP(0x000C)

    rt_real*fso2;
#define inf_FSO2        DP(0x0010)


    rt_cell*iar0;
#define inf_IAR0        DP(0x0014)

    rt_cell*ico1;
#define inf_ICO1        DP(0x0018)

    rt_cell*ico2;
#define inf_ICO2        DP(0x001C)

    rt_cell*iso1;
#define inf_ISO1        DP(0x0020)

    rt_cell*iso2;
#define inf_ISO2        DP(0x0024)


    rt_cell cyc;
#define inf_CYC         DP(0x0028)

    rt_cell size;
#define inf_SIZE        DP(0x002C)

    rt_cell fctrl;
#define inf_FCTRL       DP(0x0030)

    rt_pntr label;
#define inf_LABEL       DP(0x0034)

};


#if RUN_LEVEL >= 1

rt_void C_run_level1(rt_INFO *p)
{
    rt_cell i, j, n = p->size;
    rt_real *far0 = p->far0;
    rt_real *fco1 = p->fco1;
    rt_real *fco2 = p->fco2;

    i = p->cyc;
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

rt_void S_run_level1(rt_INFO *p)
{
    rt_cell i;
    rt_INFO info = *p;

#define AJ0         DP(0x0000)
#define AJ1         DP(0x0010)
#define AJ2         DP(0x0020)

    i = p->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_FSO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        movps_ld(Xmm0, Mecx, AJ0)
        movps_ld(Xmm1, Mecx, AJ1)
        movps_rr(Xmm2, Xmm0)
        addps_rr(Xmm2, Xmm1)
        movps_rr(Xmm3, Xmm0)
        subps_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ0)
        movps_st(Xmm3, Mebx, AJ0)

        movps_ld(Xmm0, Mecx, AJ1)
        movps_ld(Xmm1, Mecx, AJ2)
        movps_rr(Xmm2, Xmm0)
        addps_rr(Xmm2, Xmm1)
        movps_rr(Xmm3, Xmm0)
        subps_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ1)
        movps_st(Xmm3, Mebx, AJ1)

        movps_ld(Xmm0, Mecx, AJ2)
        movps_ld(Xmm1, Mecx, AJ0)
        movps_rr(Xmm2, Xmm0)
        addps_rr(Xmm2, Xmm1)
        movps_rr(Xmm3, Xmm0)
        subps_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ2)
        movps_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void P_run_level1(rt_INFO *p, rt_long tC, rt_long tS, rt_bool v)
{
    rt_cell j, n;

    rt_real *far0 = p->far0;
    rt_real *fco1 = p->fco1;
    rt_real *fco2 = p->fco2;
    rt_real *fso1 = p->fso1;
    rt_real *fso2 = p->fso2;

    RT_LOGI("-----------------  RUN LEVEL = %d  ------------------\n", 1);

    j = n = p->size;
    while (j-->0)
    {
        if (fco1[j] == fso1[j] && fco2[j] == fso2[j] && !v)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e\n", j, far0[j]);

        RT_LOGI("C farr[%d]+farr[%d] = %e, farr[%d]-farr[%d] = %e\n",
                j, (j + 4) % n, fco1[j], j, (j + 4) % n, fco2[j]);

        RT_LOGI("S farr[%d]+farr[%d] = %e, farr[%d]-farr[%d] = %e\n",
                j, (j + 4) % n, fso1[j], j, (j + 4) % n, fso2[j]);
    }

    RT_LOGI("Time C = %d\n", (rt_cell)tC);
    RT_LOGI("Time S = %d\n", (rt_cell)tS);

    RT_LOGI("----------------------------------------------------\n");
}

#endif /* RUN_LEVEL 1 */


#if RUN_LEVEL >= 2

rt_void C_run_level2(rt_INFO *p)
{
    rt_cell i, j, n = p->size;
    rt_real *far0 = p->far0;
    rt_real *fco1 = p->fco1;
    rt_real *fco2 = p->fco2;

    i = p->cyc;
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

rt_void S_run_level2(rt_INFO *p)
{
    rt_cell i;
    rt_INFO info = *p;

#define AJ0         DP(0x0000)
#define AJ1         DP(0x0010)
#define AJ2         DP(0x0020)

    i = p->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_FSO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        movps_ld(Xmm0, Mecx, AJ0)
        movps_ld(Xmm1, Mecx, AJ1)
        movps_rr(Xmm2, Xmm0)
        mulps_rr(Xmm2, Xmm1)
        movps_rr(Xmm3, Xmm0)
        divps_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ0)
        movps_st(Xmm3, Mebx, AJ0)

        movps_ld(Xmm0, Mecx, AJ1)
        movps_ld(Xmm1, Mecx, AJ2)
        movps_rr(Xmm2, Xmm0)
        mulps_rr(Xmm2, Xmm1)
        movps_rr(Xmm3, Xmm0)
        divps_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ1)
        movps_st(Xmm3, Mebx, AJ1)

        movps_ld(Xmm0, Mecx, AJ2)
        movps_ld(Xmm1, Mecx, AJ0)
        movps_rr(Xmm2, Xmm0)
        mulps_rr(Xmm2, Xmm1)
        movps_rr(Xmm3, Xmm0)
        divps_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ2)
        movps_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void P_run_level2(rt_INFO *p, rt_long tC, rt_long tS, rt_bool v)
{
    rt_cell j, n;

    rt_real *far0 = p->far0;
    rt_real *fco1 = p->fco1;
    rt_real *fco2 = p->fco2;
    rt_real *fso1 = p->fso1;
    rt_real *fso2 = p->fso2;

    RT_LOGI("-----------------  RUN LEVEL = %d  ------------------\n", 2);

    j = n = p->size;
    while (j-->0)
    {
        if (fco1[j] == fso1[j] && fco2[j] == fso2[j] && !v)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e\n", j, far0[j]);

        RT_LOGI("C farr[%d]*farr[%d] = %e, farr[%d]/farr[%d] = %e\n",
                j, (j + 4) % n, fco1[j], j, (j + 4) % n, fco2[j]);

        RT_LOGI("S farr[%d]*farr[%d] = %e, farr[%d]/farr[%d] = %e\n",
                j, (j + 4) % n, fso1[j], j, (j + 4) % n, fso2[j]);

    }

    RT_LOGI("Time C = %d\n", (rt_cell)tC);
    RT_LOGI("Time S = %d\n", (rt_cell)tS);

    RT_LOGI("----------------------------------------------------\n");
}

#endif /* RUN_LEVEL 2 */


#if RUN_LEVEL >= 3

rt_void C_run_level3(rt_INFO *p)
{
    rt_cell i, j, n = p->size;
    rt_real *far0 = p->far0;
    rt_cell *ico1 = p->ico1;
    rt_cell *ico2 = p->ico2;

    i = p->cyc;
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

rt_void S_run_level3(rt_INFO *p)
{
    rt_cell i;
    rt_INFO info = *p;

#define AJ0         DP(0x0000)
#define AJ1         DP(0x0010)
#define AJ2         DP(0x0020)

    i = p->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movps_ld(Xmm0, Mecx, AJ0)
        movps_ld(Xmm1, Mecx, AJ1)
        movps_rr(Xmm2, Xmm0)
        cgtps_rr(Xmm2, Xmm1)
        movps_rr(Xmm3, Xmm0)
        cgeps_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ0)
        movps_st(Xmm3, Mebx, AJ0)

        movps_ld(Xmm0, Mecx, AJ1)
        movps_ld(Xmm1, Mecx, AJ2)
        movps_rr(Xmm2, Xmm0)
        cgtps_rr(Xmm2, Xmm1)
        movps_rr(Xmm3, Xmm0)
        cgeps_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ1)
        movps_st(Xmm3, Mebx, AJ1)

        movps_ld(Xmm0, Mecx, AJ2)
        movps_ld(Xmm1, Mecx, AJ0)
        movps_rr(Xmm2, Xmm0)
        cgtps_rr(Xmm2, Xmm1)
        movps_rr(Xmm3, Xmm0)
        cgeps_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ2)
        movps_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void P_run_level3(rt_INFO *p, rt_long tC, rt_long tS, rt_bool v)
{
    rt_cell j, n;

    rt_real *far0 = p->far0;
    rt_cell *ico1 = p->ico1;
    rt_cell *ico2 = p->ico2;
    rt_cell *iso1 = p->iso1;
    rt_cell *iso2 = p->iso2;

    RT_LOGI("-----------------  RUN LEVEL = %d  ------------------\n", 3);

    j = n = p->size;
    while (j-->0)
    {
        if (ico1[j] == iso1[j] && ico2[j] == iso2[j] && !v)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e\n", j, far0[j]);

        RT_LOGI("C (farr[%d] > farr[%d]) = %X, (farr[%d] >= farr[%d]) = %X\n",
                j, (j + 4) % n, ico1[j], j, (j + 4) % n, ico2[j]);

        RT_LOGI("S (farr[%d] > farr[%d]) = %X, (farr[%d] >= farr[%d]) = %X\n",
                j, (j + 4) % n, iso1[j], j, (j + 4) % n, iso2[j]);
    }

    RT_LOGI("Time C = %d\n", (rt_cell)tC);
    RT_LOGI("Time S = %d\n", (rt_cell)tS);

    RT_LOGI("----------------------------------------------------\n");
}

#endif /* RUN_LEVEL 3 */


#if RUN_LEVEL >= 4

rt_void C_run_level4(rt_INFO *p)
{
    rt_cell i, j, n = p->size;
    rt_real *far0 = p->far0;
    rt_cell *ico1 = p->ico1;
    rt_cell *ico2 = p->ico2;

    i = p->cyc;
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

rt_void S_run_level4(rt_INFO *p)
{
    rt_cell i;
    rt_INFO info = *p;

#define AJ0         DP(0x0000)
#define AJ1         DP(0x0010)
#define AJ2         DP(0x0020)

    i = p->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movps_ld(Xmm0, Mecx, AJ0)
        movps_ld(Xmm1, Mecx, AJ1)
        movps_rr(Xmm2, Xmm0)
        cltps_rr(Xmm2, Xmm1)
        movps_rr(Xmm3, Xmm0)
        cleps_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ0)
        movps_st(Xmm3, Mebx, AJ0)

        movps_ld(Xmm0, Mecx, AJ1)
        movps_ld(Xmm1, Mecx, AJ2)
        movps_rr(Xmm2, Xmm0)
        cltps_rr(Xmm2, Xmm1)
        movps_rr(Xmm3, Xmm0)
        cleps_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ1)
        movps_st(Xmm3, Mebx, AJ1)

        movps_ld(Xmm0, Mecx, AJ2)
        movps_ld(Xmm1, Mecx, AJ0)
        movps_rr(Xmm2, Xmm0)
        cltps_rr(Xmm2, Xmm1)
        movps_rr(Xmm3, Xmm0)
        cleps_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ2)
        movps_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void P_run_level4(rt_INFO *p, rt_long tC, rt_long tS, rt_bool v)
{
    rt_cell j, n;

    rt_real *far0 = p->far0;
    rt_cell *ico1 = p->ico1;
    rt_cell *ico2 = p->ico2;
    rt_cell *iso1 = p->iso1;
    rt_cell *iso2 = p->iso2;

    RT_LOGI("-----------------  RUN LEVEL = %d  ------------------\n", 4);

    j = n = p->size;
    while (j-->0)
    {
        if (ico1[j] == iso1[j] && ico2[j] == iso2[j] && !v)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e\n", j, far0[j]);

        RT_LOGI("C (farr[%d] < farr[%d]) = %X, (farr[%d] <= farr[%d]) = %X\n",
                j, (j + 4) % n, ico1[j], j, (j + 4) % n, ico2[j]);

        RT_LOGI("S (farr[%d] < farr[%d]) = %X, (farr[%d] <= farr[%d]) = %X\n",
                j, (j + 4) % n, iso1[j], j, (j + 4) % n, iso2[j]);
    }

    RT_LOGI("Time C = %d\n", (rt_cell)tC);
    RT_LOGI("Time S = %d\n", (rt_cell)tS);

    RT_LOGI("----------------------------------------------------\n");
}

#endif /* RUN_LEVEL 4 */


#if RUN_LEVEL >= 5

rt_void C_run_level5(rt_INFO *p)
{
    rt_cell i, j, n = p->size;
    rt_real *far0 = p->far0;
    rt_cell *ico1 = p->ico1;
    rt_cell *ico2 = p->ico2;

    i = p->cyc;
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

rt_void S_run_level5(rt_INFO *p)
{
    rt_cell i;
    rt_INFO info = *p;

#define AJ0         DP(0x0000)
#define AJ1         DP(0x0010)
#define AJ2         DP(0x0020)

    i = p->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movps_ld(Xmm0, Mecx, AJ0)
        movps_ld(Xmm1, Mecx, AJ1)
        movps_rr(Xmm2, Xmm0)
        ceqps_rr(Xmm2, Xmm1)
        movps_rr(Xmm3, Xmm0)
        cneps_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ0)
        movps_st(Xmm3, Mebx, AJ0)

        movps_ld(Xmm0, Mecx, AJ1)
        movps_ld(Xmm1, Mecx, AJ2)
        movps_rr(Xmm2, Xmm0)
        ceqps_rr(Xmm2, Xmm1)
        movps_rr(Xmm3, Xmm0)
        cneps_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ1)
        movps_st(Xmm3, Mebx, AJ1)

        movps_ld(Xmm0, Mecx, AJ2)
        movps_ld(Xmm1, Mecx, AJ0)
        movps_rr(Xmm2, Xmm0)
        ceqps_rr(Xmm2, Xmm1)
        movps_rr(Xmm3, Xmm0)
        cneps_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ2)
        movps_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void P_run_level5(rt_INFO *p, rt_long tC, rt_long tS, rt_bool v)
{
    rt_cell j, n;

    rt_real *far0 = p->far0;
    rt_cell *ico1 = p->ico1;
    rt_cell *ico2 = p->ico2;
    rt_cell *iso1 = p->iso1;
    rt_cell *iso2 = p->iso2;

    RT_LOGI("-----------------  RUN LEVEL = %d  ------------------\n", 5);

    j = n = p->size;
    while (j-->0)
    {
        if (ico1[j] == iso1[j] && ico2[j] == iso2[j] && !v)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e\n", j, far0[j]);

        RT_LOGI("C (farr[%d] == farr[%d]) = %X, (farr[%d] != farr[%d]) = %X\n",
                j, (j + 4) % n, ico1[j], j, (j + 4) % n, ico2[j]);

        RT_LOGI("S (farr[%d] == farr[%d]) = %X, (farr[%d] != farr[%d]) = %X\n",
                j, (j + 4) % n, iso1[j], j, (j + 4) % n, iso2[j]);
    }

    RT_LOGI("Time C = %d\n", (rt_cell)tC);
    RT_LOGI("Time S = %d\n", (rt_cell)tS);

    RT_LOGI("----------------------------------------------------\n");
}

#endif /* RUN_LEVEL 5 */


#if RUN_LEVEL >= 6

rt_void C_run_level6(rt_INFO *p)
{
    rt_cell i, j, n = p->size;
    rt_real *far0 = p->far0;
    rt_cell *iar0 = p->iar0;
    rt_cell *ico1 = p->ico1;
    rt_real *fco2 = p->fco2;

    i = p->cyc;
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

rt_void S_run_level6(rt_INFO *p)
{
    rt_cell i;
    rt_INFO info = *p;

#define AJ0         DP(0x0000)
#define AJ1         DP(0x0010)
#define AJ2         DP(0x0020)

    i = p->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        FCTRL_ENTER()

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Resi, Mebp, inf_IAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        movps_ld(Xmm0, Mecx, AJ0)
        movps_ld(Xmm1, Mesi, AJ0)
        cvtps_rr(Xmm2, Xmm0)
        cvtpn_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ0)
        movps_st(Xmm3, Mebx, AJ0)

        movps_ld(Xmm0, Mecx, AJ1)
        movps_ld(Xmm1, Mesi, AJ1)
        cvtps_rr(Xmm2, Xmm0)
        cvtpn_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ1)
        movps_st(Xmm3, Mebx, AJ1)

        movps_ld(Xmm0, Mecx, AJ2)
        movps_ld(Xmm1, Mesi, AJ2)
        cvtps_rr(Xmm2, Xmm0)
        cvtpn_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ2)
        movps_st(Xmm3, Mebx, AJ2)

        FCTRL_LEAVE()

        ASM_LEAVE(info)
    }
}

rt_void P_run_level6(rt_INFO *p, rt_long tC, rt_long tS, rt_bool v)
{
    rt_cell j, n;

    rt_real *far0 = p->far0;
    rt_cell *iar0 = p->iar0;
    rt_cell *ico1 = p->ico1;
    rt_real *fco2 = p->fco2;
    rt_cell *iso1 = p->iso1;
    rt_real *fso2 = p->fso2;

    RT_LOGI("-----------------  RUN LEVEL = %d  ------------------\n", 6);

    j = n = p->size;
    while (j-->0)
    {
        if (ico1[j] == iso1[j] && fco2[j] == fso2[j] && !v)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e, iarr[%d] = %d\n", j, far0[j], j, iar0[j]);

        RT_LOGI("C (rt_cell)farr[%d] = %d, (rt_real)iarr[%d] = %e\n",
                j, ico1[j], j, fco2[j]);

        RT_LOGI("S (rt_cell)farr[%d] = %d, (rt_real)iarr[%d] = %e\n",
                j, iso1[j], j, fso2[j]);
    }

    RT_LOGI("Time C = %d\n", (rt_cell)tC);
    RT_LOGI("Time S = %d\n", (rt_cell)tS);

    RT_LOGI("----------------------------------------------------\n");
}

#endif /* RUN_LEVEL 6 */


#if RUN_LEVEL >= 7

rt_void C_run_level7(rt_INFO *p)
{
    rt_cell i, j, n = p->size;
    rt_real *far0 = p->far0;
    rt_real *fco1 = p->fco1;
    rt_real *fco2 = p->fco2;

    i = p->cyc;
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

rt_void S_run_level7(rt_INFO *p)
{
    rt_cell i;
    rt_INFO info = *p;

#define AJ0         DP(0x0000)
#define AJ1         DP(0x0010)
#define AJ2         DP(0x0020)

    i = p->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Recx, Mebp, inf_FAR0)
        movxx_ld(Redx, Mebp, inf_FSO1)
        movxx_ld(Rebx, Mebp, inf_FSO2)

        movps_ld(Xmm0, Mecx, AJ0)
        sqrps_rr(Xmm2, Xmm0)
        rcpps_rr(Xmm3, Xmm0)
        movps_st(Xmm2, Medx, AJ0)
        movps_st(Xmm3, Mebx, AJ0)

        movps_ld(Xmm0, Mecx, AJ1)
        sqrps_rr(Xmm2, Xmm0)
        rcpps_rr(Xmm3, Xmm0)
        movps_st(Xmm2, Medx, AJ1)
        movps_st(Xmm3, Mebx, AJ1)

        movps_ld(Xmm0, Mecx, AJ2)
        sqrps_rr(Xmm2, Xmm0)
        rcpps_rr(Xmm3, Xmm0)
        movps_st(Xmm2, Medx, AJ2)
        movps_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void P_run_level7(rt_INFO *p, rt_long tC, rt_long tS, rt_bool v)
{
    rt_cell j, n;

    rt_real *far0 = p->far0;
    rt_real *fco1 = p->fco1;
    rt_real *fco2 = p->fco2;
    rt_real *fso1 = p->fso1;
    rt_real *fso2 = p->fso2;

    RT_LOGI("-----------------  RUN LEVEL = %d  ------------------\n", 7);

    j = n = p->size;
    while (j-->0)
    {
        if (fco1[j] == fso1[j] && fco2[j] == fso2[j] && !v)
        {
            continue;
        }

        RT_LOGI("farr[%d] = %e\n", j, far0[j]);

        RT_LOGI("C sqrt(farr[%d]) = %e, 1.0/farr[%d] = %e\n",
                j, fco1[j], j, fco2[j]);

        RT_LOGI("S sqrt(farr[%d]) = %e, 1.0/farr[%d] = %e\n",
                j, fso1[j], j, fso2[j]);
    }

    RT_LOGI("Time C = %d\n", (rt_cell)tC);
    RT_LOGI("Time S = %d\n", (rt_cell)tS);

    RT_LOGI("----------------------------------------------------\n");
}

#endif /* RUN_LEVEL 7 */


#if RUN_LEVEL >= 8

rt_void C_run_level8(rt_INFO *p)
{
    rt_cell i, j, n = p->size;
    rt_cell *iar0 = p->iar0;
    rt_cell *ico1 = p->ico1;
    rt_cell *ico2 = p->ico2;

    i = p->cyc;
    while (i-->0)
    {
        j = n;
        while (j-->0)
        {
            ico1[j] = iar0[j] + (iar0[j] << 1);
            ico2[j] = iar0[j] + (iar0[j] >> 2);
        }
    }
}

rt_void S_run_level8(rt_INFO *p)
{
    rt_cell i;
    rt_INFO info = *p;

#define AJ0         DP(0x0000)
#define AJ1         DP(0x0010)
#define AJ2         DP(0x0020)

    i = p->cyc;
    while (i-->0)
    {
        ASM_ENTER(info)

        movxx_ld(Resi, Mebp, inf_IAR0)
        movxx_ld(Redx, Mebp, inf_ISO1)
        movxx_ld(Rebx, Mebp, inf_ISO2)

        movps_ld(Xmm0, Mesi, AJ0)
        movps_ld(Xmm1, Mesi, AJ0)
        movps_rr(Xmm2, Xmm0)
        shlpn_ri(Xmm0, IB(1))
        addpn_rr(Xmm2, Xmm0)
        movps_rr(Xmm3, Xmm1)
        shrpn_ri(Xmm1, IB(2))
        addpn_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ0)
        movps_st(Xmm3, Mebx, AJ0)

        movps_ld(Xmm0, Mesi, AJ1)
        movps_ld(Xmm1, Mesi, AJ1)
        movps_rr(Xmm2, Xmm0)
        shlpn_ri(Xmm0, IB(1))
        addpn_rr(Xmm2, Xmm0)
        movps_rr(Xmm3, Xmm1)
        shrpn_ri(Xmm1, IB(2))
        addpn_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ1)
        movps_st(Xmm3, Mebx, AJ1)

        movps_ld(Xmm0, Mesi, AJ2)
        movps_ld(Xmm1, Mesi, AJ2)
        movps_rr(Xmm2, Xmm0)
        shlpn_ri(Xmm0, IB(1))
        addpn_rr(Xmm2, Xmm0)
        movps_rr(Xmm3, Xmm1)
        shrpn_ri(Xmm1, IB(2))
        addpn_rr(Xmm3, Xmm1)
        movps_st(Xmm2, Medx, AJ2)
        movps_st(Xmm3, Mebx, AJ2)

        ASM_LEAVE(info)
    }
}

rt_void P_run_level8(rt_INFO *p, rt_long tC, rt_long tS, rt_bool v)
{
    rt_cell j, n;

    rt_cell *iar0 = p->iar0;
    rt_cell *ico1 = p->ico1;
    rt_cell *ico2 = p->ico2;
    rt_cell *iso1 = p->iso1;
    rt_cell *iso2 = p->iso2;

    RT_LOGI("-----------------  RUN LEVEL = %d  ------------------\n", 8);

    j = n = p->size;
    while (j-->0)
    {
        if (ico1[j] == iso1[j] && ico2[j] == iso2[j] && !v)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %d\n", j, iar0[j]);

        RT_LOGI("C iarr[%d]+(iarr[%d]<<1) = %d, iarr[%d]+(iarr[%d]>>2) = %d\n",
                j, j, ico1[j], j, j, ico2[j]);

        RT_LOGI("S iarr[%d]+(iarr[%d]<<1) = %d, iarr[%d]+(iarr[%d]>>2) = %d\n",
                j, j, iso1[j], j, j, iso2[j]);

    }

    RT_LOGI("Time C = %d\n", (rt_cell)tC);
    RT_LOGI("Time S = %d\n", (rt_cell)tS);

    RT_LOGI("----------------------------------------------------\n");
}

#endif /* RUN_LEVEL 8 */


#if RUN_LEVEL >= 9

rt_void C_run_level9(rt_INFO *p)
{
    rt_cell i, j, n = p->size;
    rt_cell *iar0 = p->iar0;
    rt_cell *ico1 = p->ico1;
    rt_cell *ico2 = p->ico2;

    i = p->cyc;
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

rt_void S_run_level9(rt_INFO *p)
{
    rt_INFO info = *p;

    ASM_ENTER(info)

        label_ld(cyc_beg) /* load to Reax */
        movxx_st(Reax, Mebp, inf_LABEL)

    LBL(cyc_beg)

        movxx_ld(Recx, Mebp, inf_IAR0)
        movxx_ld(Rebx, Mebp, inf_ISO1)
        movxx_ld(Resi, Mebp, inf_ISO2)
        movxx_ld(Redi, Mebp, inf_SIZE)

    LBL(loc_beg)

        movxx_ld(Reax, Mecx, DP(0x00))
        mulxx_ld(Redx, Reax, Mecx, DP(0x10))
        movxx_st(Reax, Mebx, DP(0x00))
        movxx_ld(Reax, Mecx, DP(0x00))
        movxx_ri(Redx, IB(0))
        divxx_ld(Redx, Reax, Mecx, DP(0x10))
        movxx_st(Reax, Mesi, DP(0x00))

        addxx_ri(Recx, IB(4))
        addxx_ri(Rebx, IB(4))
        addxx_ri(Resi, IB(4))
        subxx_ri(Redi, IB(1))
        cmpxx_ri(Redi, IB(4))
        jgtxx_lb(loc_beg)

        movxx_ld(Redi, Mebp, inf_IAR0)

        movxx_ld(Reax, Mecx, DP(0x00))
        mulxx_ld(Redx, Reax, Medi, DP(0x00))
        movxx_st(Reax, Mebx, DP(0x00))
        movxx_ld(Reax, Mecx, DP(0x00))
        movxx_ri(Redx, IB(0))
        divxx_ld(Redx, Reax, Medi, DP(0x00))
        movxx_st(Reax, Mesi, DP(0x00))

        movxx_ld(Reax, Mecx, DP(0x04))
        mulxx_ld(Redx, Reax, Medi, DP(0x04))
        movxx_st(Reax, Mebx, DP(0x04))
        movxx_ld(Reax, Mecx, DP(0x04))
        movxx_ri(Redx, IB(0))
        divxx_ld(Redx, Reax, Medi, DP(0x04))
        movxx_st(Reax, Mesi, DP(0x04))

        movxx_ld(Reax, Mecx, DP(0x08))
        mulxx_ld(Redx, Reax, Medi, DP(0x08))
        movxx_st(Reax, Mebx, DP(0x08))
        movxx_ld(Reax, Mecx, DP(0x08))
        movxx_ri(Redx, IB(0))
        divxx_ld(Redx, Reax, Medi, DP(0x08))
        movxx_st(Reax, Mesi, DP(0x08))

        movxx_ld(Reax, Mecx, DP(0x0C))
        mulxx_ld(Redx, Reax, Medi, DP(0x0C))
        movxx_st(Reax, Mebx, DP(0x0C))
        movxx_ld(Reax, Mecx, DP(0x0C))
        movxx_ri(Redx, IB(0))
        divxx_ld(Redx, Reax, Medi, DP(0x0C))
        movxx_st(Reax, Mesi, DP(0x0C))

        subxx_mi(Mebp, inf_CYC, IB(1))
        cmpxx_mi(Mebp, inf_CYC, IB(0))
        jeqxx_lb(cyc_end)
        jmpxx_mm(Mebp, inf_LABEL)
        jmpxx_lb(cyc_beg) /* the same jump as above */

    LBL(cyc_end)

    ASM_LEAVE(info)
}

rt_void P_run_level9(rt_INFO *p, rt_long tC, rt_long tS, rt_bool v)
{
    rt_cell j, n;

    rt_cell *iar0 = p->iar0;
    rt_cell *ico1 = p->ico1;
    rt_cell *ico2 = p->ico2;
    rt_cell *iso1 = p->iso1;
    rt_cell *iso2 = p->iso2;

    RT_LOGI("-----------------  RUN LEVEL = %d  ------------------\n", 9);

    j = n = p->size;
    while (j-->0)
    {
        if (ico1[j] == iso1[j] && ico2[j] == iso2[j] && !v)
        {
            continue;
        }

        RT_LOGI("iarr[%d] = %d\n", j, iar0[j]);

        RT_LOGI("C iarr[%d]*iarr[%d] = %d, iarr[%d]/iarr[%d] = %d\n",
                j, (j + 4) % n, ico1[j], j, (j + 4) % n, ico2[j]);

        RT_LOGI("S iarr[%d]*iarr[%d] = %d, iarr[%d]/iarr[%d] = %d\n",
                j, (j + 4) % n, iso1[j], j, (j + 4) % n, iso2[j]);

    }

    RT_LOGI("Time C = %d\n", (rt_cell)tC);
    RT_LOGI("Time S = %d\n", (rt_cell)tS);

    RT_LOGI("----------------------------------------------------\n");
}

#endif /* RUN_LEVEL 9 */


typedef rt_void (*C_run_levelX)(rt_INFO *);
typedef rt_void (*S_run_levelX)(rt_INFO *);
typedef rt_void (*P_run_levelX)(rt_INFO *, rt_long, rt_long, rt_bool);

C_run_levelX Carr[RUN_LEVEL] =
{
#if RUN_LEVEL >= 1
    C_run_level1,
#endif /* RUN_LEVEL 1 */

#if RUN_LEVEL >= 2
    C_run_level2,
#endif /* RUN_LEVEL 2 */

#if RUN_LEVEL >= 3
    C_run_level3,
#endif /* RUN_LEVEL 3 */

#if RUN_LEVEL >= 4
    C_run_level4,
#endif /* RUN_LEVEL 4 */

#if RUN_LEVEL >= 5
    C_run_level5,
#endif /* RUN_LEVEL 5 */

#if RUN_LEVEL >= 6
    C_run_level6,
#endif /* RUN_LEVEL 6 */

#if RUN_LEVEL >= 7
    C_run_level7,
#endif /* RUN_LEVEL 7 */

#if RUN_LEVEL >= 8
    C_run_level8,
#endif /* RUN_LEVEL 8 */

#if RUN_LEVEL >= 9
    C_run_level9,
#endif /* RUN_LEVEL 9 */
};

S_run_levelX Sarr[RUN_LEVEL] =
{
#if RUN_LEVEL >= 1
    S_run_level1,
#endif /* RUN_LEVEL 1 */

#if RUN_LEVEL >= 2
    S_run_level2,
#endif /* RUN_LEVEL 2 */

#if RUN_LEVEL >= 3
    S_run_level3,
#endif /* RUN_LEVEL 3 */

#if RUN_LEVEL >= 4
    S_run_level4,
#endif /* RUN_LEVEL 4 */

#if RUN_LEVEL >= 5
    S_run_level5,
#endif /* RUN_LEVEL 5 */

#if RUN_LEVEL >= 6
    S_run_level6,
#endif /* RUN_LEVEL 6 */

#if RUN_LEVEL >= 7
    S_run_level7,
#endif /* RUN_LEVEL 7 */

#if RUN_LEVEL >= 8
    S_run_level8,
#endif /* RUN_LEVEL 8 */

#if RUN_LEVEL >= 9
    S_run_level9,
#endif /* RUN_LEVEL 9 */
};

P_run_levelX Parr[RUN_LEVEL] =
{
#if RUN_LEVEL >= 1
    P_run_level1,
#endif /* RUN_LEVEL 1 */

#if RUN_LEVEL >= 2
    P_run_level2,
#endif /* RUN_LEVEL 2 */

#if RUN_LEVEL >= 3
    P_run_level3,
#endif /* RUN_LEVEL 3 */

#if RUN_LEVEL >= 4
    P_run_level4,
#endif /* RUN_LEVEL 4 */

#if RUN_LEVEL >= 5
    P_run_level5,
#endif /* RUN_LEVEL 5 */

#if RUN_LEVEL >= 6
    P_run_level6,
#endif /* RUN_LEVEL 6 */

#if RUN_LEVEL >= 7
    P_run_level7,
#endif /* RUN_LEVEL 7 */

#if RUN_LEVEL >= 8
    P_run_level8,
#endif /* RUN_LEVEL 8 */

#if RUN_LEVEL >= 9
    P_run_level9,
#endif /* RUN_LEVEL 9 */
};


rt_long get_time();

/*
 * marr - memory array
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
int main ()
{
    rt_real marr[500] = {0};

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
        7653765.0,
        431874873.,
    };

    rt_real *far0 = (rt_real*)((rt_cell)((rt_char*)&marr[0]   + MASK) & ~MASK);
    rt_real *fco1 = (rt_real*)((rt_cell)((rt_char*)&marr[100] + MASK) & ~MASK);
    rt_real *fco2 = (rt_real*)((rt_cell)((rt_char*)&marr[200] + MASK) & ~MASK);
    rt_real *fso1 = (rt_real*)((rt_cell)((rt_char*)&marr[300] + MASK) & ~MASK);
    rt_real *fso2 = (rt_real*)((rt_cell)((rt_char*)&marr[400] + MASK) & ~MASK);

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
        8764753,
        7665,
        318773,
    };

    rt_cell *iar0 = (rt_cell*)((rt_cell)((rt_char*)&marr[50]  + MASK) & ~MASK);
    rt_cell *ico1 = (rt_cell*)((rt_cell)((rt_char*)&marr[150] + MASK) & ~MASK);
    rt_cell *ico2 = (rt_cell*)((rt_cell)((rt_char*)&marr[250] + MASK) & ~MASK);
    rt_cell *iso1 = (rt_cell*)((rt_cell)((rt_char*)&marr[350] + MASK) & ~MASK);
    rt_cell *iso2 = (rt_cell*)((rt_cell)((rt_char*)&marr[450] + MASK) & ~MASK);

    memcpy(far0, farr, sizeof(farr));
    memcpy(iar0, iarr, sizeof(iarr));

    rt_INFO     info =
    {
        far0,   fco1,   fco2,   fso1,   fso2,
        iar0,   ico1,   ico2,   iso1,   iso2,
        CYC_SIZE, ARR_SIZE
    };

    rt_long time1 = 0;
    rt_long time2 = 0;
    rt_long tC = 0;
    rt_long tS = 0;

    rt_cell i;

    for (i = 0; i < RUN_LEVEL; i++)
    {
        time1 = get_time();

        Carr[i](&info);

        time2 = get_time();
        tC = time2 - time1;

        time1 = get_time();

        Sarr[i](&info);

        time2 = get_time();
        tS = time2 - time1;

        Parr[i](&info, tC, tS, VERBOSE);
    }

#if   defined (WIN32) /* Win32, MSC ----------------------------------------- */

    RT_LOGI("Type any letter and press ENTER to exit:");
    rt_char str[256];
    scanf("%s", str);

#endif /* ------------- OS specific ----------------------------------------- */

    return 0;
}

#if   defined (WIN32) /* Win32, MSC ----------------------------------------- */

#include <windows.h>

rt_long get_time()
{
    LARGE_INTEGER fr;
    QueryPerformanceFrequency(&fr);
    LARGE_INTEGER tm;
    QueryPerformanceCounter(&tm);
    return (rt_long)(tm.QuadPart * 1000 / fr.QuadPart);
}

#elif defined (linux) /* Linux, GCC ----------------------------------------- */

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
