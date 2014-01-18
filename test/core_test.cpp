/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <malloc.h>
#include <string.h>

#include "engine.h"

#define RUN_LEVEL       6
#define VERBOSE         RT_FALSE
#define CYC_SIZE        10

#define RT_X_RES        800
#define RT_Y_RES        480

rt_cell     x_res   = RT_X_RES;
rt_cell     y_res   = RT_Y_RES;
rt_cell     x_row   = RT_X_RES;
rt_word     frame[RT_X_RES * RT_Y_RES];

rt_cell     fsaa    = RT_FSAA_NO;

rt_Scene   *scene   = RT_NULL;

static
rt_void frame_cpy(rt_word *fd, rt_word *fs)
{
    rt_cell i;

    for (i = 0; i < y_res * x_row; i++)
    {
       *fd++ = *fs++;
    }
}

static
rt_cell frame_cmp(rt_word *f1, rt_word *f2)
{
    rt_cell i;

    for (i = 0; i < y_res * x_row; i++)
    {
        if (f1[i] != f2[i])
        {
            RT_LOGI("Frames differ\n");
            return 1;
        }
    }

    if (VERBOSE)
    {
        RT_LOGI("Frames are identical\n");
    }

    return 0;
}


#if RUN_LEVEL >=  1

#include "scn_test01.h"

rt_void test01(rt_cell opts)
{
    scene = new rt_Scene(&scn_test01::sc_root,
                        x_res, y_res, x_row, RT_NULL,
                        malloc, free,
                        RT_NULL, RT_NULL,
                        RT_NULL, RT_NULL);

    scene->set_opts(opts);
}

#endif /* RUN_LEVEL  1 */


#if RUN_LEVEL >=  2

#include "scn_test02.h"

rt_void test02(rt_cell opts)
{
    scene = new rt_Scene(&scn_test02::sc_root,
                        x_res, y_res, x_row, RT_NULL,
                        malloc, free,
                        RT_NULL, RT_NULL,
                        RT_NULL, RT_NULL);

    scene->set_opts(opts);
}

#endif /* RUN_LEVEL  2 */


#if RUN_LEVEL >=  3

#include "scn_test03.h"

rt_void test03(rt_cell opts)
{
    scene = new rt_Scene(&scn_test03::sc_root,
                        x_res, y_res, x_row, RT_NULL,
                        malloc, free,
                        RT_NULL, RT_NULL,
                        RT_NULL, RT_NULL);

    scene->set_opts(opts);
}

#endif /* RUN_LEVEL  3 */


#if RUN_LEVEL >=  4

#include "scn_test04.h"

rt_void test04(rt_cell opts)
{
    scene = new rt_Scene(&scn_test04::sc_root,
                        x_res, y_res, x_row, RT_NULL,
                        malloc, free,
                        RT_NULL, RT_NULL,
                        RT_NULL, RT_NULL);

    scene->set_opts(opts);
}

#endif /* RUN_LEVEL  4 */


#if RUN_LEVEL >=  5

#include "scn_test05.h"

rt_void test05(rt_cell opts)
{
    scene = new rt_Scene(&scn_test05::sc_root,
                        x_res, y_res, x_row, RT_NULL,
                        malloc, free,
                        RT_NULL, RT_NULL,
                        RT_NULL, RT_NULL);

    scene->set_opts(opts);
}

#endif /* RUN_LEVEL  5 */


#if RUN_LEVEL >=  6

#include "scn_test06.h"

rt_void test06(rt_cell opts)
{
    scene = new rt_Scene(&scn_test06::sc_root,
                        x_res, y_res, x_row, RT_NULL,
                        malloc, free,
                        RT_NULL, RT_NULL,
                        RT_NULL, RT_NULL);

    scene->set_opts(opts);
}

#endif /* RUN_LEVEL  6 */


typedef rt_void (*testXX)(rt_cell);

testXX test[RUN_LEVEL] =
{
#if RUN_LEVEL >=  1
    test01,
#endif /* RUN_LEVEL  1 */

#if RUN_LEVEL >=  2
    test02,
#endif /* RUN_LEVEL  2 */

#if RUN_LEVEL >=  3
    test03,
#endif /* RUN_LEVEL  3 */

#if RUN_LEVEL >=  4
    test04,
#endif /* RUN_LEVEL  4 */

#if RUN_LEVEL >=  5
    test05,
#endif /* RUN_LEVEL  5 */

#if RUN_LEVEL >=  6
    test06,
#endif /* RUN_LEVEL  6 */
};


rt_long get_time();

int main ()
{
    rt_long time1 = 0;
    rt_long time2 = 0;
    rt_long tN = 0;
    rt_long tF = 0;

    rt_cell i, j;

    for (i = 0; i < RUN_LEVEL; i++)
    {
        RT_LOGI("-----------------  RUN LEVEL = %2d  -----------------\n", i+1);
        try
        {
            scene = RT_NULL;
            test[i](RT_OPTS_NONE);

            time1 = get_time();

            for (j = 0; j < CYC_SIZE; j++)
            {
                scene->render(j * 16);
            }

            time2 = get_time();
            tN = time2 - time1;
            RT_LOGI("Time N = %d\n", (rt_cell)tN);

            frame_cpy(frame, scene->get_frame());
            delete scene;


            scene = RT_NULL;
            test[i](RT_OPTS_FULL);

            time1 = get_time();

            for (j = 0; j < CYC_SIZE; j++)
            {
                scene->render(j * 16);
            }

            time2 = get_time();
            tF = time2 - time1;
            RT_LOGI("Time F = %d\n", (rt_cell)tF);

            frame_cmp(frame, scene->get_frame());
            delete scene;
        }
        catch (rt_Exception e)
        {
            RT_LOGE("Exception: %s\n", e.err);
        }
        RT_LOGI("----------------------------------------------------\n");
    }

#if   defined (WIN32) /* Win32, MSVC ---------------------------------------- */

    RT_LOGI("Type any letter and press ENTER to exit:");
    rt_char str[256]; /* not secure, do not inherit this practice */
    scanf("%s", str); /* not secure, do not inherit this practice */

#endif /* ------------- OS specific ----------------------------------------- */

    return 0;
}

#if   defined (WIN32) /* Win32, MSVC ---------------------------------------- */

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
