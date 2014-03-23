/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <malloc.h>
#include <string.h>

#include "engine.h"

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

#define RUN_LEVEL           13
#define CYC_SIZE            3

#define RT_X_RES            800
#define RT_Y_RES            480

#define CHN(px, sh)         ((px) & (0xFF << (sh)))    
#define PEQ(p1, p2)         (RT_ABS(CHN(p1,24)-CHN(p2,24))<=(t_diff<<24) && \
                             RT_ABS(CHN(p1,16)-CHN(p2,16))<=(t_diff<<16) && \
                             RT_ABS(CHN(p1, 8)-CHN(p2, 8))<=(t_diff<< 8) && \
                             RT_ABS(CHN(p1, 0)-CHN(p2, 0))<=(t_diff<< 0))
#define PDF(p1, p2)         (RT_ABS(CHN(p1,24)-CHN(p2,24)) +                \
                             RT_ABS(CHN(p1,16)-CHN(p2,16)) +                \
                             RT_ABS(CHN(p1, 8)-CHN(p2, 8)) +                \
                             RT_ABS(CHN(p1, 0)-CHN(p2, 0)))

/******************************************************************************/
/***************************   VARS, FUNCS, TYPES   ***************************/
/******************************************************************************/

static rt_cell t_diff = 3;
static rt_bool v_mode = RT_FALSE;
static rt_bool i_mode = RT_FALSE;

static rt_cell x_res = RT_X_RES;
static rt_cell y_res = RT_Y_RES;
static rt_cell x_row = RT_X_RES;
static rt_word frame[RT_X_RES * RT_Y_RES];

static rt_cell fsaa = RT_FSAA_NO;
static rt_Scene *scene = RT_NULL;

static
rt_void frame_cpy(rt_word *fd, rt_word *fs)
{
    rt_cell i;

    for (i = 0; i < y_res * x_row; i++, fd++, fs++)
    {
       *fd = *fs;
    }
}

static
rt_cell frame_cmp(rt_word *f1, rt_word *f2)
{
    rt_cell i, ret = 0;

    for (i = 0; i < y_res * x_row; i++)
    {
        if (PEQ(f1[i], f2[i]))
        {
            continue;
        }

        ret = 1;

        RT_LOGI("Frames differ (%06X %06X) at x = %d, y = %d\n",
                    f1[i], f2[i], i % x_row, i / x_row);

        if (!v_mode)
        {
            break;
        }
    }

    if (v_mode && ret == 0)
    {
        RT_LOGI("Frames are identical\n");
    }

    return ret;
}

static
rt_void frame_dff(rt_word *fd, rt_word *fs)
{
    rt_cell i;

    for (i = 0; i < y_res * x_row; i++, fd++, fs++)
    {
       *fd = PDF(*fd, *fs);
    }
}

static
rt_void frame_max(rt_word *fd)
{
    rt_cell i;

    for (i = 0; i < y_res * x_row; i++, fd++)
    {
       *fd = *fd & 0x00FFFFFF ? 0x00FFFFFF : 0x00000000;
    }
}

/******************************************************************************/
/******************************   RUN LEVEL  1   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  1

#include "scn_test01.h"

rt_void o_test01(rt_cell opts)
{
    scene = new rt_Scene(&scn_test01::sc_root,
                        x_res, y_res, x_row, RT_NULL,
                        malloc, free,
                        RT_NULL, RT_NULL,
                        RT_NULL, RT_NULL);

    scene->set_opts(opts);
}

#endif /* RUN_LEVEL  1 */

/******************************************************************************/
/******************************   RUN LEVEL  2   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  2

#include "scn_test02.h"

rt_void o_test02(rt_cell opts)
{
    scene = new rt_Scene(&scn_test02::sc_root,
                        x_res, y_res, x_row, RT_NULL,
                        malloc, free,
                        RT_NULL, RT_NULL,
                        RT_NULL, RT_NULL);

    scene->set_opts(opts);
}

#endif /* RUN_LEVEL  2 */

/******************************************************************************/
/******************************   RUN LEVEL  3   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  3

#include "scn_test03.h"

rt_void o_test03(rt_cell opts)
{
    scene = new rt_Scene(&scn_test03::sc_root,
                        x_res, y_res, x_row, RT_NULL,
                        malloc, free,
                        RT_NULL, RT_NULL,
                        RT_NULL, RT_NULL);

    scene->set_opts(opts);
}

#endif /* RUN_LEVEL  3 */

/******************************************************************************/
/******************************   RUN LEVEL  4   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  4

#include "scn_test04.h"

rt_void o_test04(rt_cell opts)
{
    scene = new rt_Scene(&scn_test04::sc_root,
                        x_res, y_res, x_row, RT_NULL,
                        malloc, free,
                        RT_NULL, RT_NULL,
                        RT_NULL, RT_NULL);

    scene->set_opts(opts);
}

#endif /* RUN_LEVEL  4 */

/******************************************************************************/
/******************************   RUN LEVEL  5   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  5

#include "scn_test05.h"

rt_void o_test05(rt_cell opts)
{
    scene = new rt_Scene(&scn_test05::sc_root,
                        x_res, y_res, x_row, RT_NULL,
                        malloc, free,
                        RT_NULL, RT_NULL,
                        RT_NULL, RT_NULL);

    scene->set_opts(opts);
}

#endif /* RUN_LEVEL  5 */

/******************************************************************************/
/******************************   RUN LEVEL  6   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  6

#include "scn_test06.h"

rt_void o_test06(rt_cell opts)
{
    scene = new rt_Scene(&scn_test06::sc_root,
                        x_res, y_res, x_row, RT_NULL,
                        malloc, free,
                        RT_NULL, RT_NULL,
                        RT_NULL, RT_NULL);

    scene->set_opts(opts);
}

#endif /* RUN_LEVEL  6 */

/******************************************************************************/
/******************************   RUN LEVEL  7   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  7

#include "scn_test07.h"

rt_void o_test07(rt_cell opts)
{
    scene = new rt_Scene(&scn_test07::sc_root,
                        x_res, y_res, x_row, RT_NULL,
                        malloc, free,
                        RT_NULL, RT_NULL,
                        RT_NULL, RT_NULL);

    scene->set_opts(opts);
}

#endif /* RUN_LEVEL  7 */

/******************************************************************************/
/******************************   RUN LEVEL  8   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  8

#include "scn_test08.h"

rt_void o_test08(rt_cell opts)
{
    scene = new rt_Scene(&scn_test08::sc_root,
                        x_res, y_res, x_row, RT_NULL,
                        malloc, free,
                        RT_NULL, RT_NULL,
                        RT_NULL, RT_NULL);

    scene->set_opts(opts);
}

#endif /* RUN_LEVEL  8 */

/******************************************************************************/
/******************************   RUN LEVEL  9   ******************************/
/******************************************************************************/

#if RUN_LEVEL >=  9

#include "scn_test09.h"

rt_void o_test09(rt_cell opts)
{
    scene = new rt_Scene(&scn_test09::sc_root,
                        x_res, y_res, x_row, RT_NULL,
                        malloc, free,
                        RT_NULL, RT_NULL,
                        RT_NULL, RT_NULL);

    scene->set_opts(opts);
}

#endif /* RUN_LEVEL  9 */

/******************************************************************************/
/******************************   RUN LEVEL 10   ******************************/
/******************************************************************************/

#if RUN_LEVEL >= 10

#include "scn_test10.h"

rt_void o_test10(rt_cell opts)
{
    scene = new rt_Scene(&scn_test10::sc_root,
                        x_res, y_res, x_row, RT_NULL,
                        malloc, free,
                        RT_NULL, RT_NULL,
                        RT_NULL, RT_NULL);

    scene->set_opts(opts);
}

#endif /* RUN_LEVEL 10 */

/******************************************************************************/
/******************************   RUN LEVEL 11   ******************************/
/******************************************************************************/

#if RUN_LEVEL >= 11

#include "scn_test11.h"

rt_void o_test11(rt_cell opts)
{
    scene = new rt_Scene(&scn_test11::sc_root,
                        x_res, y_res, x_row, RT_NULL,
                        malloc, free,
                        RT_NULL, RT_NULL,
                        RT_NULL, RT_NULL);

    scene->set_opts(opts);
}

#endif /* RUN_LEVEL 11 */

/******************************************************************************/
/******************************   RUN LEVEL 12   ******************************/
/******************************************************************************/

#if RUN_LEVEL >= 12

#include "scn_test12.h"

rt_void o_test12(rt_cell opts)
{
    scene = new rt_Scene(&scn_test12::sc_root,
                        x_res, y_res, x_row, RT_NULL,
                        malloc, free,
                        RT_NULL, RT_NULL,
                        RT_NULL, RT_NULL);

    scene->set_opts(opts);
}

#endif /* RUN_LEVEL 12 */

/******************************************************************************/
/******************************   RUN LEVEL 13   ******************************/
/******************************************************************************/

#if RUN_LEVEL >= 13

#include "scn_test13.h"

rt_void o_test13(rt_cell opts)
{
    scene = new rt_Scene(&scn_test13::sc_root,
                        x_res, y_res, x_row, RT_NULL,
                        malloc, free,
                        RT_NULL, RT_NULL,
                        RT_NULL, RT_NULL);

    scene->set_opts(opts);
}

#endif /* RUN_LEVEL 13 */

/******************************************************************************/
/*********************************   TABLES   *********************************/
/******************************************************************************/

typedef rt_void (*testXX)(rt_cell);

testXX o_test[RUN_LEVEL] =
{
#if RUN_LEVEL >=  1
    o_test01,
#endif /* RUN_LEVEL  1 */

#if RUN_LEVEL >=  2
    o_test02,
#endif /* RUN_LEVEL  2 */

#if RUN_LEVEL >=  3
    o_test03,
#endif /* RUN_LEVEL  3 */

#if RUN_LEVEL >=  4
    o_test04,
#endif /* RUN_LEVEL  4 */

#if RUN_LEVEL >=  5
    o_test05,
#endif /* RUN_LEVEL  5 */

#if RUN_LEVEL >=  6
    o_test06,
#endif /* RUN_LEVEL  6 */

#if RUN_LEVEL >=  7
    o_test07,
#endif /* RUN_LEVEL  7 */

#if RUN_LEVEL >=  8
    o_test08,
#endif /* RUN_LEVEL  8 */

#if RUN_LEVEL >=  9
    o_test09,
#endif /* RUN_LEVEL  9 */

#if RUN_LEVEL >= 10
    o_test10,
#endif /* RUN_LEVEL 10 */

#if RUN_LEVEL >= 11
    o_test11,
#endif /* RUN_LEVEL 11 */

#if RUN_LEVEL >= 12
    o_test12,
#endif /* RUN_LEVEL 12 */

#if RUN_LEVEL >= 13
    o_test13,
#endif /* RUN_LEVEL 13 */
};

/******************************************************************************/
/**********************************   MAIN   **********************************/
/******************************************************************************/

rt_long get_time();

rt_cell main(rt_cell argc, rt_char *argv[])
{
    rt_cell k;

    if (argc >= 2)
    {
        RT_LOGI("argc = %d\n", argc);
        for (k = 0; k < argc; k++)
        {
            RT_LOGI("argv[%d] = %s\n", k, argv[k]);
        }
    }

    for (k = 1; k < argc; k++)
    {
        if (strcmp(argv[k], "-d") == 0 && ++k < argc)
        {
            t_diff = argv[k][0] - '0';
            if (strlen(argv[k]) == 1 && t_diff >= 0 && t_diff <= 9)
            {
                RT_LOGI("Diff threshold overriden: %d\n", t_diff);
            }
            else
            {
                RT_LOGI("Diff threshold value out of range\n");
                return 0;
            }
        }
        if (strcmp(argv[k], "-v") == 0 && !v_mode)
        {
            v_mode = RT_TRUE;
            RT_LOGI("Verbose mode enabled\n");
        }
        if (strcmp(argv[k], "-i") == 0 && !i_mode)
        {
            i_mode = RT_TRUE;
            RT_LOGI("Imaging mode enabled\n");
        }
    }

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
            o_test[i](RT_OPTS_NONE);

            time1 = get_time();

            for (j = 0; j < CYC_SIZE; j++)
            {
                scene->render(j * 16);
            }

            time2 = get_time();
            tN = time2 - time1;
            RT_LOGI("Time N = %d\n", (rt_cell)tN);

            if (i_mode)
            {
                scene->save_frame((i+1) * 10 + 0);
            }
            frame_cpy(frame, scene->get_frame());

            delete scene;

            /* --------------------------------- */

            scene = RT_NULL;
            o_test[i](RT_OPTS_FULL);

            time1 = get_time();

            for (j = 0; j < CYC_SIZE; j++)
            {
                scene->render(j * 16);
            }

            time2 = get_time();
            tF = time2 - time1;
            RT_LOGI("Time F = %d\n", (rt_cell)tF);

            if (i_mode)
            {
                scene->save_frame((i+1) * 10 + 1);
            }
            frame_cmp(frame, scene->get_frame());

            /* --------------------------------- */

            frame_dff(scene->get_frame(), frame);
            if (i_mode)
            {
                scene->save_frame((i+1) * 10 + 2);
            }

            frame_max(scene->get_frame());
            if (i_mode)
            {
                scene->save_frame((i+1) * 10 + 3);
            }

            delete scene;
        }
        catch (rt_Exception e)
        {
            RT_LOGE("Exception: %s\n", e.err);
        }
        RT_LOGI("----------------------------------------------------\n");
    }

#if   defined (RT_WIN32) /* Win32, MSVC ------------------------------------ */

    RT_LOGI("Type any letter and press ENTER to exit:");
    rt_char str[256]; /* not secure, do not inherit this practice */
    scanf("%s", str); /* not secure, do not inherit this practice */

#endif /* ------------- OS specific ----------------------------------------- */

    return 0;
}

/******************************************************************************/
/**********************************   UTILS   *********************************/
/******************************************************************************/

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
