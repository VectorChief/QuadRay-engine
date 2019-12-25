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

#define RUN_LEVEL       1
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


typedef rt_void (*C_run_levelX)(rt_INFO *);
typedef rt_void (*S_run_levelX)(rt_INFO *);
typedef rt_void (*P_run_levelX)(rt_INFO *, rt_long, rt_long, rt_bool);

C_run_levelX Carr[RUN_LEVEL] =
{
#if RUN_LEVEL >= 1
    C_run_level1,
#endif /* RUN_LEVEL 1 */
};

S_run_levelX Sarr[RUN_LEVEL] =
{
#if RUN_LEVEL >= 1
    S_run_level1,
#endif /* RUN_LEVEL 1 */
};

P_run_levelX Parr[RUN_LEVEL] =
{
#if RUN_LEVEL >= 1
    P_run_level1,
#endif /* RUN_LEVEL 1 */
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
        87647534,
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

#if   defined (RT_WIN32) /* Win32, MSC -------------------------------------- */

    RT_LOGI("Type any letter and press ENTER to exit:");
    rt_char str[256];
    scanf("%s", str);

#endif /* ----------------- OS specific ------------------------------------- */

    return 0;
}

#if   defined (RT_WIN32) /* Win32, MSC -------------------------------------- */

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

#endif /* ----------------- OS specific ------------------------------------- */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
