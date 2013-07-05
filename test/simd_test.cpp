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

#define RUN_LEVEL       0
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
