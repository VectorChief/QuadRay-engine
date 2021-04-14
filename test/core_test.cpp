/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "engine.h"
#include "rtimag.h"

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

#define SUB_TEST            18
#define CYC_SIZE            3

#define RT_X_RES            800
#define RT_Y_RES            480

#define CHN(px, sh)         ((px) & (0xFF << (sh)))

#define PEQ(p1, p2)         (RT_ABS32(CHN(p1,24)-CHN(p2,24))<=(t_diff<<24)&&\
                             RT_ABS32(CHN(p1,16)-CHN(p2,16))<=(t_diff<<16)&&\
                             RT_ABS32(CHN(p1, 8)-CHN(p2, 8))<=(t_diff<< 8)&&\
                             RT_ABS32(CHN(p1, 0)-CHN(p2, 0))<=(t_diff<< 0))

#define PDF(p1, p2)         (RT_ABS32(CHN(p1,24)-CHN(p2,24))+               \
                             RT_ABS32(CHN(p1,16)-CHN(p2,16))+               \
                             RT_ABS32(CHN(p1, 8)-CHN(p2, 8))+               \
                             RT_ABS32(CHN(p1, 0)-CHN(p2, 0)))

/******************************************************************************/
/***************************   VARS, FUNCS, TYPES   ***************************/
/******************************************************************************/

rt_si32     x_res       = RT_X_RES;
rt_si32     y_res       = RT_Y_RES;
rt_si32     x_row       = (RT_X_RES+RT_SIMD_WIDTH-1) & ~(RT_SIMD_WIDTH-1);
rt_ui32    *frame       = RT_NULL;

rt_Scene   *scene       = RT_NULL;

rt_si32     n_init      = 0;            /* subtest-init (from command-line) */
rt_si32     n_done      = SUB_TEST-1;   /* subtest-done (from command-line) */
rt_si32     f_num       =-1;        /* number-of-frames (from command-line) */
rt_time     f_time      = 16;       /* frame-delta-(ms) (from command-line) */
rt_si32     n_simd      = 0;        /* SIMD-native-size (from command-line) */
rt_si32     k_size      = 0;        /* SIMD-size-factor (from command-line) */
rt_si32     s_type      = 0;        /* SIMD-sub-variant (from command-line) */
rt_si32     w_size      = 1;        /* Window-rect-size (from command-line) */
rt_si32     t_diff      = 3;          /* diff-threshold (from command-line) */
rt_si32     r_test      =-CYC_SIZE;   /* test-redundant (from command-line) */
rt_bool     v_mode      = RT_FALSE;     /* verbose mode (from command-line) */
rt_bool     p_mode      = RT_FALSE;     /* pixhunt mode (from command-line) */
rt_bool     i_mode      = RT_FALSE;     /* imaging mode (from command-line) */
rt_bool     h_mode      = RT_FALSE;     /* shownum mode (from command-line) */
rt_bool     o_mode      = RT_FALSE;     /* optimal mode (from command-line) */
rt_si32     a_mode      = RT_FSAA_NO;   /* antialiasing (from command-line) */

/*
 * Get system time in milliseconds.
 */
rt_time get_time();

/*
 * Allocate memory from system heap.
 */
rt_pntr sys_alloc(rt_size size);

/*
 * Free memory from system heap.
 */
rt_void sys_free(rt_pntr ptr, rt_size size);

/*
 * Copy frames.
 */
rt_void frame_cpy(rt_ui32 *fd, rt_ui32 *fs)
{
    rt_si32 i;

    /* copy frame */
    for (i = 0; i < y_res * x_row; i++, fd++, fs++)
    {
       *fd = *fs;
    }
}

/*
 * Compare frames.
 */
rt_si32 frame_cmp(rt_ui32 *f1, rt_ui32 *f2)
{
    rt_si32 i, j, ret = 0;

    /* print first or all (verbose) pixel spots above diff-threshold,
     * ignore isolated pixels if pixhunt mode is disabled (default) */
    for (j = 0; j < y_res; j++)
    {
        for (i = 0; i < x_res; i++)
        {
            if (PEQ(f1[j*x_row + i], f2[j*x_row + i]))
            {
                continue;
            }
            else
            if (!p_mode
            &&  j > 0 && j < y_res - 1
            &&  i > 0 && i < x_res - 1 
            &&  PEQ(f1[(j-1)*x_row + (i-1)], f2[(j-1)*x_row + (i-1)])
            &&  PEQ(f1[(j-1)*x_row + (i-0)], f2[(j-1)*x_row + (i-0)])
            &&  PEQ(f1[(j-1)*x_row + (i+1)], f2[(j-1)*x_row + (i+1)])
            &&  PEQ(f1[(j-0)*x_row + (i-1)], f2[(j-0)*x_row + (i-1)])
            &&  PEQ(f1[(j+0)*x_row + (i+1)], f2[(j+0)*x_row + (i+1)])
            &&  PEQ(f1[(j+1)*x_row + (i-1)], f2[(j+1)*x_row + (i-1)])
            &&  PEQ(f1[(j+1)*x_row + (i+0)], f2[(j+1)*x_row + (i+0)])
            &&  PEQ(f1[(j+1)*x_row + (i+1)], f2[(j+1)*x_row + (i+1)]))
            {
                continue;
            }

            ret = 1;

            RT_LOGI("Frames differ (%06X %06X) at x = %d, y = %d\n",
                        f1[j*x_row + i], f2[j*x_row + i], i, j);

            if (!v_mode)
            {
                j = y_res - 1;
                break;
            }
        }
    }

    if (v_mode && ret == 0)
    {
        RT_LOGI("Frames are identical\n");
    }

    return ret;
}

/*
 * Write plain diff between frames.
 */
rt_void frame_dff(rt_ui32 *fd, rt_ui32 *fs)
{
    rt_si32 i;

    /* save diff, max all pixels above diff-threshold
     * if pixhunt mode is enabled */
    for (i = 0; i < y_res * x_row; i++, fd++, fs++)
    {
       *fd = PDF(*fd, *fs);
        if (!p_mode)
        {
            continue;
        }
       *fd = PEQ(*fd, 0x0) ? *fd : 0x00FFFFFF;
    }
}

/*
 * Write maximized diff between frames.
 */
rt_void frame_max(rt_ui32 *fd)
{
    rt_si32 i;

    /* max all pixels with non-zero diff */
    for (i = 0; i < y_res * x_row; i++, fd++)
    {
       *fd = *fd & 0x00FFFFFF ? 0x00FFFFFF : 0x00000000;
    }
}

/*
 * Common instance of platform container.
 */
rt_Platform pfm(sys_alloc, sys_free);

/******************************************************************************/
/*******************************   SUB TEST  1   ******************************/
/******************************************************************************/

#if SUB_TEST >=  1

#include "scn_test01.h"

rt_void o_test01()
{
    scene = new(&pfm) rt_Scene(&scn_test01::sc_root,
                               x_res, y_res, x_row, RT_NULL, &pfm);
}

#endif /* SUB_TEST  1 */

/******************************************************************************/
/*******************************   SUB TEST  2   ******************************/
/******************************************************************************/

#if SUB_TEST >=  2

#include "scn_test02.h"

rt_void o_test02()
{
    scene = new(&pfm) rt_Scene(&scn_test02::sc_root,
                               x_res, y_res, x_row, RT_NULL, &pfm);
}

#endif /* SUB_TEST  2 */

/******************************************************************************/
/*******************************   SUB TEST  3   ******************************/
/******************************************************************************/

#if SUB_TEST >=  3

#include "scn_test03.h"

rt_void o_test03()
{
    scene = new(&pfm) rt_Scene(&scn_test03::sc_root,
                               x_res, y_res, x_row, RT_NULL, &pfm);
}

#endif /* SUB_TEST  3 */

/******************************************************************************/
/*******************************   SUB TEST  4   ******************************/
/******************************************************************************/

#if SUB_TEST >=  4

#include "scn_test04.h"

rt_void o_test04()
{
    scene = new(&pfm) rt_Scene(&scn_test04::sc_root,
                               x_res, y_res, x_row, RT_NULL, &pfm);
}

#endif /* SUB_TEST  4 */

/******************************************************************************/
/*******************************   SUB TEST  5   ******************************/
/******************************************************************************/

#if SUB_TEST >=  5

#include "scn_test05.h"

rt_void o_test05()
{
    scene = new(&pfm) rt_Scene(&scn_test05::sc_root,
                               x_res, y_res, x_row, RT_NULL, &pfm);
}

#endif /* SUB_TEST  5 */

/******************************************************************************/
/*******************************   SUB TEST  6   ******************************/
/******************************************************************************/

#if SUB_TEST >=  6

#include "scn_test06.h"

rt_void o_test06()
{
    scene = new(&pfm) rt_Scene(&scn_test06::sc_root,
                               x_res, y_res, x_row, RT_NULL, &pfm);
}

#endif /* SUB_TEST  6 */

/******************************************************************************/
/*******************************   SUB TEST  7   ******************************/
/******************************************************************************/

#if SUB_TEST >=  7

#include "scn_test07.h"

rt_void o_test07()
{
    scene = new(&pfm) rt_Scene(&scn_test07::sc_root,
                               x_res, y_res, x_row, RT_NULL, &pfm);
}

#endif /* SUB_TEST  7 */

/******************************************************************************/
/*******************************   SUB TEST  8   ******************************/
/******************************************************************************/

#if SUB_TEST >=  8

#include "scn_test08.h"

rt_void o_test08()
{
    scene = new(&pfm) rt_Scene(&scn_test08::sc_root,
                               x_res, y_res, x_row, RT_NULL, &pfm);
}

#endif /* SUB_TEST  8 */

/******************************************************************************/
/*******************************   SUB TEST  9   ******************************/
/******************************************************************************/

#if SUB_TEST >=  9

#include "scn_test09.h"

rt_void o_test09()
{
    scene = new(&pfm) rt_Scene(&scn_test09::sc_root,
                               x_res, y_res, x_row, RT_NULL, &pfm);
}

#endif /* SUB_TEST  9 */

/******************************************************************************/
/*******************************   SUB TEST 10   ******************************/
/******************************************************************************/

#if SUB_TEST >= 10

#include "scn_test10.h"

rt_void o_test10()
{
    scene = new(&pfm) rt_Scene(&scn_test10::sc_root,
                               x_res, y_res, x_row, RT_NULL, &pfm);
}

#endif /* SUB_TEST 10 */

/******************************************************************************/
/*******************************   SUB TEST 11   ******************************/
/******************************************************************************/

#if SUB_TEST >= 11

#include "scn_test11.h"

rt_void o_test11()
{
    scene = new(&pfm) rt_Scene(&scn_test11::sc_root,
                               x_res, y_res, x_row, RT_NULL, &pfm);
}

#endif /* SUB_TEST 11 */

/******************************************************************************/
/*******************************   SUB TEST 12   ******************************/
/******************************************************************************/

#if SUB_TEST >= 12

#include "scn_test12.h"

rt_void o_test12()
{
    scene = new(&pfm) rt_Scene(&scn_test12::sc_root,
                               x_res, y_res, x_row, RT_NULL, &pfm);
}

#endif /* SUB_TEST 12 */

/******************************************************************************/
/*******************************   SUB TEST 13   ******************************/
/******************************************************************************/

#if SUB_TEST >= 13

#include "scn_test13.h"

rt_void o_test13()
{
    scene = new(&pfm) rt_Scene(&scn_test13::sc_root,
                               x_res, y_res, x_row, RT_NULL, &pfm);
}

#endif /* SUB_TEST 13 */

/******************************************************************************/
/*******************************   SUB TEST 14   ******************************/
/******************************************************************************/

#if SUB_TEST >= 14

#include "scn_test14.h"

rt_void o_test14()
{
    scene = new(&pfm) rt_Scene(&scn_test14::sc_root,
                               x_res, y_res, x_row, RT_NULL, &pfm);
}

#endif /* SUB_TEST 14 */

/******************************************************************************/
/*******************************   SUB TEST 15   ******************************/
/******************************************************************************/

#if SUB_TEST >= 15

#include "scn_test15.h"

rt_void o_test15()
{
    scene = new(&pfm) rt_Scene(&scn_test15::sc_root,
                               x_res, y_res, x_row, RT_NULL, &pfm);
}

#endif /* SUB_TEST 15 */

/******************************************************************************/
/*******************************   SUB TEST 16   ******************************/
/******************************************************************************/

#if SUB_TEST >= 16

#include "scn_test16.h"

rt_void o_test16()
{
    scene = new(&pfm) rt_Scene(&scn_test16::sc_root,
                               x_res, y_res, x_row, RT_NULL, &pfm);
}

#endif /* SUB_TEST 16 */

/******************************************************************************/
/*******************************   SUB TEST 17   ******************************/
/******************************************************************************/

#if SUB_TEST >= 17

#include "scn_test17.h"

rt_void o_test17()
{
    scene = new(&pfm) rt_Scene(&scn_test17::sc_root,
                               x_res, y_res, x_row, RT_NULL, &pfm);
}

#endif /* SUB_TEST 17 */

/******************************************************************************/
/*******************************   SUB TEST 18   ******************************/
/******************************************************************************/

#if SUB_TEST >= 18

#include "scn_test18.h"

rt_void o_test18()
{
    scene = new(&pfm) rt_Scene(&scn_test18::sc_root,
                               x_res, y_res, x_row, RT_NULL, &pfm);
}

#endif /* SUB_TEST 18 */

/******************************************************************************/
/*********************************   TABLES   *********************************/
/******************************************************************************/

typedef rt_void (*testXX)();

testXX o_test[SUB_TEST] =
{
#if SUB_TEST >=  1
    o_test01,
#endif /* SUB_TEST  1 */

#if SUB_TEST >=  2
    o_test02,
#endif /* SUB_TEST  2 */

#if SUB_TEST >=  3
    o_test03,
#endif /* SUB_TEST  3 */

#if SUB_TEST >=  4
    o_test04,
#endif /* SUB_TEST  4 */

#if SUB_TEST >=  5
    o_test05,
#endif /* SUB_TEST  5 */

#if SUB_TEST >=  6
    o_test06,
#endif /* SUB_TEST  6 */

#if SUB_TEST >=  7
    o_test07,
#endif /* SUB_TEST  7 */

#if SUB_TEST >=  8
    o_test08,
#endif /* SUB_TEST  8 */

#if SUB_TEST >=  9
    o_test09,
#endif /* SUB_TEST  9 */

#if SUB_TEST >= 10
    o_test10,
#endif /* SUB_TEST 10 */

#if SUB_TEST >= 11
    o_test11,
#endif /* SUB_TEST 11 */

#if SUB_TEST >= 12
    o_test12,
#endif /* SUB_TEST 12 */

#if SUB_TEST >= 13
    o_test13,
#endif /* SUB_TEST 13 */

#if SUB_TEST >= 14
    o_test14,
#endif /* SUB_TEST 14 */

#if SUB_TEST >= 15
    o_test15,
#endif /* SUB_TEST 15 */

#if SUB_TEST >= 16
    o_test16,
#endif /* SUB_TEST 16 */

#if SUB_TEST >= 17
    o_test17,
#endif /* SUB_TEST 17 */

#if SUB_TEST >= 18
    o_test18,
#endif /* SUB_TEST 18 */
};

/******************************************************************************/
/**********************************   MAIN   **********************************/
/******************************************************************************/

rt_si32 main(rt_si32 argc, rt_char *argv[])
{
    rt_si32 k, l, r, t;

    if (argc >= 2)
    {
        RT_LOGI("--------------------------------------------------------\n");
        RT_LOGI("Usage options are given below:\n");
        RT_LOGI(" -b n, specify subtest # at which testing begins, n >= 1\n");
        RT_LOGI(" -e n, specify subtest # at which testing ends, n <= max\n");
        RT_LOGI(" -f n, specify # of consecutive frames to render, n >= 0\n");
        RT_LOGI(" -g n, specify delta (ms) for consecutive frames, n >= 0\n");
        RT_LOGI(" -n n, override SIMD-native-size, where new simd is 1.16\n");
        RT_LOGI(" -k n, override SIMD-size-factor, where new size is 1..4\n");
        RT_LOGI(" -s n, override SIMD-sub-variant, where new type is 1.32\n");
        RT_LOGI(" -w n, override window-rect-size, where new size is 1..9\n");
        RT_LOGI(" -x n, override x-resolution, where new x-value <= 65535\n");
        RT_LOGI(" -y n, override y-resolution, where new y-value <= 65535\n");
        RT_LOGI(" -d n, override diff-threshold for qualification, n >= 0\n");
        RT_LOGI(" -c n, override counter of redundant test cycles, n >= 1\n");
        RT_LOGI(" -v, enable verbose mode, print all pixel spots (> diff)\n");
        RT_LOGI(" -p, enable pixhunt mode, print isolated pixels (> diff)\n");
        RT_LOGI(" -i, enable imaging mode, save images before-after-diffs\n");
        RT_LOGI(" -h, enable shownum mode, activate screen-number drawing\n");
        RT_LOGI(" -o, enable optimal mode, omit unoptimized rendering run\n");
        RT_LOGI(" -a n, enable antialiasing, 2 for 2x, 4 for 4x, 8 for 8x\n");
        RT_LOGI(" -t tex1 tex2 texn, convert images in data/textures/tex*\n");
        RT_LOGI(" -z, plot Fresnel/Gamma functions & antialiasing samples\n");
        RT_LOGI("options -b, .., -a can be combined, -t/-z are standalone\n");
        RT_LOGI("--------------------------------------------------------\n");
    }

    if (argc >= 2 && strcmp(argv[1], "-z") == 0)
    {
        RT_LOGI("Plotting samples/functions: ");
        o_test[0]();
        scene->plot_frags();
        scene->plot_funcs();
        delete scene;
        scene = RT_NULL;
        RT_LOGI("Done!\n");
        return 0;
    }

    if (argc >= 3 && strcmp(argv[1], "-t") == 0)
    {
        RT_LOGI("Converting textures:\n[");
        rt_Heap *hp = new rt_Heap(sys_alloc, sys_free);
        for (k = 2; k < argc; k++)
        {
            r = convert_image(hp, argv[k]);
            if (r == 0)
            {
                RT_LOGI("x");
            }
            else
            {
                RT_LOGI(".");
            }
        }
        delete hp;
        RT_LOGI("]\nDone!\n");
        return 0;
    }

    for (k = 1; k < argc; k++)
    {
        if (k < argc && strcmp(argv[k], "-b") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t >= 1 && t <= SUB_TEST)
            {
                RT_LOGI("Subtest-index-init overridden: %d\n", t);
                n_init = t-1;
            }
            else
            {
                RT_LOGI("Subtest-index-init value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-e") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t >= 1 && t <= SUB_TEST)
            {
                RT_LOGI("Subtest-index-done overridden: %d\n", t);
                n_done = t-1;
            }
            else
            {
                RT_LOGI("Subtest-index-done value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-f") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t >= 0)
            {
                RT_LOGI("Number-of-frames: %d\n", t);
                f_num = t;
            }
            else
            {
                RT_LOGI("Number-of-frames value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-g") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t >= 0)
            {
                RT_LOGI("Frame-delta (ms): %d\n", t);
                f_time = t;
            }
            else
            {
                RT_LOGI("Frame-delta (ms) value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-n") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t == 1   || t == 2   || t == 4   || t == 8    || t == 16
            ||  t == 128 || t == 256 || t == 512 || t == 1024 || t == 2048)
            {
                RT_LOGI("SIMD-native-size overridden: %d\n", t);
                n_simd = t >= 128 ? t / 128 : t;
            }
            else
            {
                RT_LOGI("SIMD-native-size value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-k") == 0 && ++k < argc)
        {
            t = argv[k][0] - '0';
            if (strlen(argv[k]) == 1 && (t == 1 || t == 2 || t == 4))
            {
                RT_LOGI("SIMD-size-factor overridden: %d\n", t);
                k_size = t;
            }
            else
            {
                RT_LOGI("SIMD-size-factor value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-s") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t == 1 || t == 2 || t == 4 || t == 8 || t == 16 || t == 32)
            {
                RT_LOGI("SIMD-sub-variant overridden: %d\n", t);
                s_type = t;
            }
            else
            {
                RT_LOGI("SIMD-sub-variant value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-w") == 0 && ++k < argc)
        {
            t = argv[k][0] - '0';
            if (strlen(argv[k]) == 1 && t >= 1 && t <= 9)
            {
                RT_LOGI("Window-rect-size overridden: %d\n", t);
                w_size = t;
            }
            else
            {
                RT_LOGI("Window-rect-size value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-x") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t >= 1 && t <= 65535)
            {
                RT_LOGI("X-resolution overridden: %d\n", t);
                x_res = t;
            }
            else
            {
                RT_LOGI("X-resolution value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-y") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t >= 1 && t <= 65535)
            {
                RT_LOGI("Y-resolution overridden: %d\n", t);
                y_res = t;
            }
            else
            {
                RT_LOGI("Y-resolution value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-d") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t >= 0)
            {
                RT_LOGI("Diff-threshold overridden: %d\n", t);
                t_diff = t;
            }
            else
            {
                RT_LOGI("Diff-threshold value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-c") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t >= 1)
            {
                RT_LOGI("Test-redundant overridden: %d\n", t);
                r_test = t;
            }
            else
            {
                RT_LOGI("Test-redundant value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-v") == 0 && !v_mode)
        {
            v_mode = RT_TRUE;
            RT_LOGI("Verbose mode enabled: %d\n", v_mode);
        }
        if (k < argc && strcmp(argv[k], "-p") == 0 && !p_mode)
        {
            p_mode = RT_TRUE;
            RT_LOGI("Pixhunt mode enabled: %d\n", p_mode);
        }
        if (k < argc && strcmp(argv[k], "-i") == 0 && !i_mode)
        {
            i_mode = RT_TRUE;
            RT_LOGI("Imaging mode enabled: %d\n", i_mode);
        }
        if (k < argc && strcmp(argv[k], "-h") == 0 && !h_mode)
        {
            h_mode = RT_TRUE;
            RT_LOGI("Shownum mode enabled: %d\n", h_mode);
        }
        if (k < argc && strcmp(argv[k], "-o") == 0 && !o_mode)
        {
            o_mode = RT_TRUE;
            RT_LOGI("Optimal mode enabled: %d\n", o_mode);
        }
        if (k < argc && strcmp(argv[k], "-a") == 0)
        {
            rt_si32 aa_map[10] =
            {
                RT_FSAA_NO, RT_FSAA_NO, RT_FSAA_2X, RT_FSAA_2X,
                RT_FSAA_4X, RT_FSAA_4X, RT_FSAA_4X, RT_FSAA_4X,
                RT_FSAA_8X, RT_FSAA_8X
            };
            a_mode = RT_FSAA_4X;
            if (++k < argc)
            {
                t = argv[k][0] - '0';
                if (strlen(argv[k]) == 1 && t >= 0 && t <= 9)
                {
                    a_mode = aa_map[t];
                }
                else
                {
                    k--;
                }
            }            
            RT_LOGI("Antialiasing request: %d\n", 1 << a_mode);
        }
    }

    if (r_test != -CYC_SIZE && f_num != -1)
    {
        RT_LOGI("Test-redundant overridden: %d (-f)\n", f_num);
    }
    if (f_num != -1)
    {
        r_test = f_num;
    }
    r_test = RT_ABS32(r_test);

    rt_time time1 = 0;
    rt_time time2 = 0;
    rt_time tN = 0;
    rt_time tF = 0;

    x_res = x_res * (w_size != 0 ? w_size : 1);
    y_res = y_res * (w_size != 0 ? w_size : 1);
    x_row = (x_res+RT_SIMD_WIDTH-1) & ~(RT_SIMD_WIDTH-1);

    rt_si32 tile_w = 0;
    rt_si32 size, type, simd = 0;

    simd = (&pfm)->set_simd(simd_init(n_simd, s_type, k_size));
    if (a_mode != (&pfm)->set_fsaa(a_mode))
    {
        RT_LOGI("Requested antialiasing mode not supported, check options\n");
        return 0;
    }
    tile_w = (&pfm)->get_tile_w();

    size = (simd >> 16) & 0xFF;
    type = (simd >> 8) & 0xFF;
    simd = simd & 0xFF;

    /* test converted internal SIMD variables against new command-line format */
    if ((k_size != 0 && k_size != size)
    ||  (s_type != 0 && s_type != type)
    ||  (n_simd != 0 && n_simd != simd && n_simd != simd * size))
    {
        RT_LOGI("Chosen SIMD target not supported, check -n/-k/-s options\n");
        return 0;
    }

    /* update state-tracking SIMD variables from currently chosen SIMD target */
    k_size = size;
    s_type = type;
    n_simd = simd;

    frame = (rt_ui32 *)sys_alloc(x_row * y_res * sizeof(rt_ui32));

    RT_LOGI("------------------  TARGET CONFIG  ---------------------\n");
    RT_LOGI("SIMD size/type = %4dx%dv%d, tile_W = %dxW, FSAA = %d %s\n",
                               n_simd * 128, k_size, s_type, tile_w / 8,
                               1 << a_mode, a_mode ? "(spp)" : "(off)");
    RT_LOGI("Framebuffer X-row = %5d, ptr = %016" PR_Z "X\n",
                                                 x_row, (rt_full)frame);
    RT_LOGI("Framebuffer X-res = %5d, Y-res = %4d, l %d, h %d  %s\n",
                                              x_res, y_res, 0, !h_mode,
                                                    o_mode ? "o" : " ");

    rt_si32 i, j;

    for (i = n_init; i <= n_done; i++)
    {
        RT_LOGI("--------------------  SUB TEST = %2d  - ptr/fp = %d%s%d --\n",
                    i+1, RT_POINTER, RT_ADDRESS == 32 ? "_" : "f", RT_ELEMENT);
        try
        {
            (&pfm)->set_simd(simd_init(n_simd, s_type, k_size));
            a_mode = (&pfm)->get_fsaa();

            if (!o_mode)
            { /* -->---->-- skip run0 -->---->-- */

            /* ------------ test run0 ---------- */

            o_test[i]();

            scene->set_opts(RT_OPTS_NONE);

            time1 = get_time();

            for (j = 0; j < r_test; j++)
            {
                scene->render(j * f_time);
            }

            time2 = get_time();
            tN = time2 - time1;
            RT_LOGI("Time N = %d\n", (rt_si32)tN);

            if (h_mode)
            {
                scene->render_num(x_res-30, 10, -1, 2, 0);
                scene->render_num(x_res-10, 10, -1, 2, tile_w / 8);
                scene->render_num(x_res-10, 34, -1, 2, 1 << a_mode);
                scene->render_num(      30, 10, +1, 2, n_simd * 128);
                scene->render_num(      10, 10, +1, 2, k_size);
                scene->render_num(      10, 34, +1, 2, s_type);
            }

            if (i_mode)
            {
                scene->save_frame((i+1) * 10 + 0);
            }

            frame_cpy(frame, scene->get_frame());

            delete scene;
            scene = RT_NULL;

            } /* --<----<-- skip run0 --<----<-- */

            /* ------------ test run1 ---------- */

            o_test[i]();

            scene->set_opts(RT_OPTS_FULL);

            time1 = get_time();

            for (j = 0; j < r_test; j++)
            {
                scene->render(j * f_time);
            }

            time2 = get_time();
            tF = time2 - time1;
            RT_LOGI("Time F = %d\n", (rt_si32)tF);

            if (h_mode)
            {
                scene->render_num(x_res-30, 10, -1, 2, 0);
                scene->render_num(x_res-10, 10, -1, 2, tile_w / 8);
                scene->render_num(x_res-10, 34, -1, 2, 1 << a_mode);
                scene->render_num(      30, 10, +1, 2, n_simd * 128);
                scene->render_num(      10, 10, +1, 2, k_size);
                scene->render_num(      10, 34, +1, 2, s_type);
            }

            if (i_mode)
            {
                scene->save_frame((i+1) * 10 + 1);
            }

            if (!o_mode)
            { /* -->---->-- skip diff -->---->-- */

            frame_cmp(frame, scene->get_frame());

            /* ------------ test diff ---------- */

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

            } /* --<----<-- skip diff --<----<-- */

            delete scene;
            scene = RT_NULL;
        }
        catch (rt_Exception e)
        {
            RT_LOGE("Exception in test %d: %s\n", i+1, e.err);
        }
        RT_LOGI("--%s--------------------------------- simd = %4dx%dv%d -\n",
                        o_mode ? " o " : "---", n_simd * 128, k_size, s_type);
    }

    sys_free(frame, x_row * y_res * sizeof(rt_ui32));

#if (defined RT_WIN32) || (defined RT_WIN64) /* Win32, MSVC -- Win64, GCC --- */

    RT_LOGI("Type any letter and press ENTER to exit:");
    rt_char str[80];
    scanf("%79s", str);

#endif /* ------------- OS specific ----------------------------------------- */

    return 0;
}

/******************************************************************************/
/**********************************   UTILS   *********************************/
/******************************************************************************/

#include "rtzero.h"

#if RT_POINTER == 64
#if RT_ADDRESS == 32

#define RT_ADDRESS_MIN      ((rt_byte *)0x0000000040000000)
#define RT_ADDRESS_MAX      ((rt_byte *)0x0000000080000000)

#else /* RT_ADDRESS == 64 */

#define RT_ADDRESS_MIN      ((rt_byte *)0x0000000140000000)
#define RT_ADDRESS_MAX      ((rt_byte *)0x0000080000000000)

#endif /* RT_ADDRESS */

rt_byte *s_ptr = RT_ADDRESS_MIN;

#endif /* RT_POINTER */


#if (defined RT_WIN32) || (defined RT_WIN64) /* Win32, MSVC -- Win64, GCC --- */

#include <windows.h>

/*
 * Get system time in milliseconds.
 */
rt_time get_time()
{
    LARGE_INTEGER fr;
    QueryPerformanceFrequency(&fr);
    LARGE_INTEGER tm;
    QueryPerformanceCounter(&tm);
    return (rt_time)(tm.QuadPart * 1000 / fr.QuadPart);
}

DWORD s_step = 0;

SYSTEM_INFO s_sys = {0};

/*
 * Allocate memory from system heap.
 * Not thread-safe due to common static ptr.
 */
rt_pntr sys_alloc(rt_size size)
{
#if RT_POINTER == 64

    /* loop around RT_ADDRESS_MAX boundary */
    if (s_ptr >= RT_ADDRESS_MAX - size)
    {
        s_ptr  = RT_ADDRESS_MIN;
    }

    if (s_step == 0)
    {
        GetSystemInfo(&s_sys);
        s_step = s_sys.dwAllocationGranularity;
    }

    rt_pntr ptr = VirtualAlloc(s_ptr, size, MEM_COMMIT | MEM_RESERVE,
                  PAGE_READWRITE);

    /* advance with allocation granularity */
    s_ptr = (rt_byte *)ptr + ((size + s_step - 1) / s_step) * s_step;

#else /* RT_POINTER == 32 */

    rt_pntr ptr = malloc(size);

#endif /* RT_POINTER */

#if RT_DEBUG >= 2

    RT_LOGI("ALLOC PTR = %016" PR_Z "X, size = %ld\n", (rt_full)ptr, size);

#endif /* RT_DEBUG */

#if (RT_POINTER - RT_ADDRESS) != 0

    if ((rt_byte *)ptr >= RT_ADDRESS_MAX - size)
    {
        throw rt_Exception("address exceeded allowed range in sys_alloc");
    }

#endif /* (RT_POINTER - RT_ADDRESS) */

    if (ptr == RT_NULL)
    {
        throw rt_Exception("alloc failed with NULL address in sys_alloc");
    }

    return ptr;
}

/*
 * Free memory from system heap.
 */
rt_void sys_free(rt_pntr ptr, rt_size size)
{
#if RT_POINTER == 64

    VirtualFree(ptr, 0, MEM_RELEASE);

#else /* RT_POINTER == 32 */

    free(ptr);

#endif /* RT_POINTER */

#if RT_DEBUG >= 2

    RT_LOGI("FREED PTR = %016" PR_Z "X, size = %ld\n", (rt_full)ptr, size);

#endif /* RT_DEBUG */
}

#elif (defined RT_LINUX) /* Linux, GCC -------------------------------------- */

#include <sys/time.h>

/*
 * Get system time in milliseconds.
 */
rt_time get_time()
{
    timeval tm;
    gettimeofday(&tm, NULL);
    return (rt_time)(tm.tv_sec * 1000 + tm.tv_usec / 1000);
}

#if RT_POINTER == 64

#include <sys/mman.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON  /* workaround for macOS compilation */
#endif /* macOS still cannot allocate with mmap within 32-bit range */

#endif /* RT_POINTER */

/*
 * Allocate memory from system heap.
 * Not thread-safe due to common static ptr.
 */
rt_pntr sys_alloc(rt_size size)
{
#if RT_POINTER == 64

    /* loop around RT_ADDRESS_MAX boundary */
    /* in 64/32-bit hybrid mode addresses can't have sign bit
     * as MIPS64 sign-extends all 32-bit mem-loads by default */
    if (s_ptr >= RT_ADDRESS_MAX - size)
    {
        s_ptr  = RT_ADDRESS_MIN;
    }

    rt_pntr ptr = mmap(s_ptr, size, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    /* advance with allocation granularity */
    /* in case when page-size differs from default 4096 bytes
     * mmap should round toward closest correct page boundary */
    s_ptr = (rt_byte *)ptr + ((size + 4095) / 4096) * 4096;

#else /* RT_POINTER == 32 */

    rt_pntr ptr = malloc(size);

#endif /* RT_POINTER */

#if RT_DEBUG >= 2

    RT_LOGI("ALLOC PTR = %016" PR_Z "X, size = %ld\n", (rt_full)ptr, size);

#endif /* RT_DEBUG */

#if (RT_POINTER - RT_ADDRESS) != 0

    if ((rt_byte *)ptr >= RT_ADDRESS_MAX - size)
    {
        throw rt_Exception("address exceeded allowed range in sys_alloc");
    }

#endif /* (RT_POINTER - RT_ADDRESS) */

    if (ptr == RT_NULL)
    {
        throw rt_Exception("alloc failed with NULL address in sys_alloc");
    }

    return ptr;
}

/*
 * Free memory from system heap.
 */
rt_void sys_free(rt_pntr ptr, rt_size size)
{
#if RT_POINTER == 64

    munmap(ptr, size);

#else /* RT_POINTER == 32 */

    free(ptr);

#endif /* RT_POINTER */

#if RT_DEBUG >= 2

    RT_LOGI("FREED PTR = %016" PR_Z "X, size = %ld\n", (rt_full)ptr, size);

#endif /* RT_DEBUG */
}

#endif /* ------------- OS specific ----------------------------------------- */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
