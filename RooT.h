/******************************************************************************/
/* Copyright (c) 2013-2017 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_ROOT_H
#define RT_ROOT_H

#include <stdlib.h>
#include <string.h>
#include "engine.h"
#include "data/scenes/all_scn.h"

#define RT_X_RES        800
#define RT_Y_RES        480

rt_astr     title       = "QuadRay engine demo, (C) 2013-2017 VectorChief";
rt_si32     x_win       = RT_X_RES; /* window-rect (client) x-resolution */
rt_si32     y_win       = RT_Y_RES; /* window-rect (client) y-resolution */
rt_si32     x_res       = RT_X_RES;
rt_si32     y_res       = RT_Y_RES;
rt_si32     x_row       = (RT_X_RES+RT_SIMD_WIDTH-1) & ~(RT_SIMD_WIDTH-1);
rt_ui32    *frame       = RT_NULL;
rt_si32     thnum       = RT_THREADS_NUM;

rt_SCENE   *sc_rt[]     =
{
    &scn_demo01::sc_root,
    &scn_demo02::sc_root,
    &scn_demo03::sc_root,
};

rt_Scene   *sc[RT_ARR_SIZE(sc_rt)]  = {0};                  /* scene array */
rt_si32     d                       = RT_ARR_SIZE(sc_rt)-1; /* demo-scene */
rt_si32     c                       = 0;                    /* camera-idx */

rt_time     b_time      = 0;        /* time-begins-(ms) (from command-line) */
rt_time     e_time      =-1;        /* time-ending-(ms) (from command-line) */
rt_si32     f_num       =-1;        /* number-of-frames (from command-line) */
rt_time     f_time      =-1;        /* frame-delta-(ms) (from command-line) */
rt_si32     k_size      = 0;        /* SIMD-size-factor (from command-line) */
rt_si32     n_simd      = 0;        /* SIMD-native-size (from command-line) */
rt_si32     s_type      = 0;        /* SIMD-sub-variant (from command-line) */
rt_si32     t_pool      = 0;        /* Thread-pool-size (from command-line) */
#if RT_FULLSCREEN == 1
rt_si32     w_size      = 0;        /* Window-rect-size (from command-line) */
#else  /* RT_FULLSCREEN */
rt_si32     w_size      = 1;        /* Window-rect-size (from command-line) */
#endif /* RT_FULLSCREEN */
rt_si32     x_new       = 0;        /* New x-resolution (from command-line) */
rt_si32     y_new       = 0;        /* New y-resolution (from command-line) */

rt_time     img_id      =-1;        /* save-image-index (from command-line) */
rt_time     l_time      = 500;        /* fpslogupd-(ms) (from command-line) */
rt_bool     l_mode      = RT_FALSE;        /* fpslogoff (from command-line) */
rt_bool     h_mode      = RT_FALSE;        /* hide mode (from command-line) */
rt_bool     o_mode      = RT_FALSE;        /* offscreen (from command-line) */
rt_bool     u_mode      = RT_FALSE;        /* updateoff (from command-line) */
rt_bool     a_mode      = RT_FALSE;        /* FSAA mode (from command-line) */

/******************************************************************************/
/********************************   PLATFORM   ********************************/
/******************************************************************************/

#include "rtzero.h"

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
 * Initialize platform-specific pool of "thnum" threads.
 */
rt_pntr init_threads(rt_si32 thnum, rt_Scene *scn);

/*
 * Terminate platform-specific pool of "thnum" threads.
 */
rt_void term_threads(rt_pntr tdata, rt_si32 thnum);

/*
 * Task platform-specific pool of "thnum" threads to update scene,
 * block until finished.
 */
rt_void update_scene(rt_pntr tdata, rt_si32 thnum, rt_si32 phase);

/*
 * Task platform-specific pool of "thnum" threads to render scene,
 * block until finished.
 */
rt_void render_scene(rt_pntr tdata, rt_si32 thnum, rt_si32 phase);

/*
 * Set current frame to screen.
 */
rt_void frame_to_screen(rt_ui32 *frame, rt_si32 x_row);

/******************************************************************************/
/*******************************   EVENT-LOOP   *******************************/
/******************************************************************************/

#define RK_ESCAPE           0

#define RK_F1               1
#define RK_F2               2
#define RK_F3               3
#define RK_F4               4
#define RK_F5               5
#define RK_F6               6
#define RK_F7               7
#define RK_F8               8
#define RK_F9               9
#define RK_F10              10
#define RK_F11              11
#define RK_F12              12

#define RK_UP               15
#define RK_DOWN             16
#define RK_LEFT             17
#define RK_RIGHT            18

#define RK_W                21
#define RK_S                22
#define RK_A                23
#define RK_D                24

#define KEY_MASK            0xFF

/* thread's exception variables */
rt_si32  eout = 0, emax = 0;
rt_pstr *estr = RT_NULL;

/* time counter variables */
rt_time init_time = 0;
rt_time run_time = 0;
rt_time log_time = 0;
rt_time cur_time = 0;
rt_bool switched = 0;

/* frame counter variables */
rt_si32 cnt = 0;
rt_real fps = 0.0f;
rt_si32 glb = 0;
rt_real avg = 0.0f;
rt_si32 ttl = 0;
rt_si32 scr_id = 0;

/* virtual key arrays */
rt_byte r_to_p[KEY_MASK + 1];
rt_byte h_keys[KEY_MASK + 1];
rt_byte t_keys[KEY_MASK + 1];
rt_byte r_keys[KEY_MASK + 1];

/* hold keys */
#define H_KEYS(k)   (h_keys[r_to_p[(k) & KEY_MASK]])
/* toggle on press */
#define T_KEYS(k)   (t_keys[r_to_p[(k) & KEY_MASK]])
/* toggle on release */
#define R_KEYS(k)   (r_keys[r_to_p[(k) & KEY_MASK]])

/*
 * Event loop's main step.
 */
rt_si32 main_step()
{
    if (sc[d] == RT_NULL)
    {
        return 0;
    }

    /* update time variables */
    cur_time = get_time();
    if (init_time == 0)
    {
        init_time = cur_time - b_time;
        log_time = run_time = b_time;
    }
    cur_time = cur_time - init_time;

    if (cur_time - log_time >= l_time)
    {
        fps = (rt_real)cnt * 1000 / (cur_time - log_time);

        glb += cnt;
        cnt = 0;
        log_time = cur_time;

        if (!l_mode)
        {
            RT_LOGI("FPS = %.2f\n", fps);
        }
    }
    if (e_time >= 0 && cur_time >= e_time)
    {
        return 0;
    }
    if (f_num >= 0 && ttl >= f_num)
    {
        return 0;
    }

    rt_si32 g = d; /* current scene for save_frame at the end of each run */

    try
    {
#if RT_OPTS_STATIC != 0
        if (!u_mode)
        { /* -->---->-- skip update0 -->---->-- */
#endif /* RT_OPTS_STATIC */

        if (H_KEYS(RK_W))       sc[d]->update(cur_time, RT_CAMERA_MOVE_FORWARD);
        if (H_KEYS(RK_S))       sc[d]->update(cur_time, RT_CAMERA_MOVE_BACK);
        if (H_KEYS(RK_A))       sc[d]->update(cur_time, RT_CAMERA_MOVE_LEFT);
        if (H_KEYS(RK_D))       sc[d]->update(cur_time, RT_CAMERA_MOVE_RIGHT);

        if (H_KEYS(RK_UP))      sc[d]->update(cur_time, RT_CAMERA_ROTATE_DOWN);
        if (H_KEYS(RK_DOWN))    sc[d]->update(cur_time, RT_CAMERA_ROTATE_UP);
        if (H_KEYS(RK_LEFT))    sc[d]->update(cur_time, RT_CAMERA_ROTATE_LEFT);
        if (H_KEYS(RK_RIGHT))   sc[d]->update(cur_time, RT_CAMERA_ROTATE_RIGHT);

        if (T_KEYS(RK_F1))
        {
            sc[d]->print_state();
        }
        if (T_KEYS(RK_F3))
        {
            rt_si32 cold = c;
            c = sc[d]->next_cam();
            switched = cold != c ? 1 : switched;
        }
        if (T_KEYS(RK_F6))
        {
            rt_si32 kold = k_size;
            rt_si32 size, type, simd;
            do
            {
                k_size = k_size % 4 + k_size % 3; /* 1, 2, 4 */
                simd = sc[d]->set_simd(simd_init(n_simd, s_type, k_size));
                simd = from_simd(simd);
                size = (simd >> 16) & 0xFF;
                type = (simd >> 8) & 0xFF;
                simd = simd & 0xFF;
                if (simd != n_simd || type != s_type)
                {
                    size = 0;
                }
            }
            while (size != k_size);
            switched = kold != k_size ? 1 : switched;
        }
        if (T_KEYS(RK_F7))
        {
            rt_si32 told = s_type;
            rt_si32 size, type, simd;
            do
            {
                s_type = s_type % 8 + s_type % 7; /* 1, 2, 4, 8 */
                simd = sc[d]->set_simd(simd_init(n_simd, s_type, k_size));
                simd = from_simd(simd);
                size = (simd >> 16) & 0xFF;
                type = (simd >> 8) & 0xFF;
                simd = simd & 0xFF;
                if (simd != n_simd || size != k_size)
                {
                    type = 0;
                }
            }
            while (type != s_type);
            switched = told != s_type ? 1 : switched;
        }
        if (T_KEYS(RK_F8))
        {
            rt_si32 nold = n_simd;
            rt_si32 size, type, simd;
            do
            {
                n_simd = n_simd % 4 + n_simd % 3; /* 1, 2, 4 */
                simd = sc[d]->set_simd(simd_init(n_simd, s_type, k_size));
                simd = from_simd(simd);
                size = (simd >> 16) & 0xFF;
                type = (simd >> 8) & 0xFF;
                simd = simd & 0xFF;
                if (simd != n_simd)
                {
                    simd = sc[d]->set_simd(simd_init(n_simd, 0, 0));
                    simd = from_simd(simd);
                    size = (simd >> 16) & 0xFF;
                    type = (simd >> 8) & 0xFF;
                    simd = simd & 0xFF;
                }
                if (simd == n_simd)
                {
                    k_size = size;
                    s_type = type;
                }
            }
            while (simd != n_simd);
            switched = nold != n_simd ? 1 : switched;
        }
        if (T_KEYS(RK_F11))
        {
            rt_si32 dold = d;
            d = (d + 1) % RT_ARR_SIZE(sc_rt);
            c = sc[d]->get_cam_idx();
            sc[d]->set_fsaa(a_mode ? RT_FSAA_4X : RT_FSAA_NO);
            sc[d]->set_simd(simd_init(n_simd, s_type, k_size));
            switched = dold != d ? 1 : switched;
        }

#if RT_OPTS_STATIC != 0
        } /* --<----<-- skip update0 --<----<-- */
#endif /* RT_OPTS_STATIC */

        if (T_KEYS(RK_F2))
        {
            a_mode = !a_mode;
            sc[d]->set_fsaa(a_mode ? RT_FSAA_4X : RT_FSAA_NO);
            switched = 1;
        }
        if (T_KEYS(RK_F4))
        {
            sc[d]->save_frame(scr_id++);
            switched = 1;
        }
        if (T_KEYS(RK_F5))
        {
            l_mode = !l_mode;
            switched = 1;
        }
        if (T_KEYS(RK_F9))
        {
            o_mode = !o_mode;
            switched = 1;
        }
        if (T_KEYS(RK_F10))
        {
            rt_si32 opts = sc[d]->get_opts();
            u_mode = !u_mode;
            if (u_mode)
            {
                opts |= +RT_OPTS_STATIC;
            }
            else
            {
                opts &= ~RT_OPTS_STATIC;
            }
            sc[d]->set_opts(opts);
            switched = 1;
        }
        if (T_KEYS(RK_F12))
        {
            h_mode = 1 - h_mode;
            switched = 1;
        }
        if (T_KEYS(RK_ESCAPE))
        {
            return 0;
        }
        memset(t_keys, 0, sizeof(t_keys));
        memset(r_keys, 0, sizeof(r_keys));

        if (switched && img_id >= 0 && img_id <= 999)
        {
            sc[g]->save_frame(img_id++);
        }

        sc[d]->render(f_time >= 0 ? b_time + f_time * ttl : cur_time);

        if (!h_mode)
        {
            sc[d]->render_num(x_res-30, 10, -1, 2, (rt_si32)fps);
            sc[d]->render_num(x_res-10, 10, -1, 2, (rt_si32)
                                               sc[d]->get_tile_w() / 8);
            sc[d]->render_num(x_res-10, 34, -1, 2, (rt_si32)a_mode * 4
                                                     / (RT_ELEMENT / 32));
            sc[d]->render_num(      30, 10, +1, 2, (rt_si32)n_simd * 128);
            sc[d]->render_num(      10, 10, +1, 2, (rt_si32)k_size);
            sc[d]->render_num(      10, 34, +1, 2, (rt_si32)s_type);
        }
    }
    catch (rt_Exception e)
    {
        RT_LOGE("Exception: %s\n", e.err);
        return 0;
    }

    if (eout != 0)
    {
        rt_si32 i;

        for (i = 0; i < emax; i++)
        {
            if (estr[i] != RT_NULL)
            {            
                RT_LOGE("Exception: thread %d: %s\n", i, estr[i]);
            }
        }

        return 0;
    }

    if (!o_mode)
    {
        frame_to_screen(sc[d]->get_frame(), sc[d]->get_x_row());
    }

    if (switched)
    {
        switched = 0;

        RT_LOGI("----------------------  FPS AVG  -----------------------\n");
        if (cur_time - run_time)
        {
            avg = (rt_real)(glb + cnt) * 1000 / (cur_time - run_time);
        }
        else
        {
            avg = (rt_real)0;
        }
        RT_LOGI("AVG = %.2f\n", avg);

        RT_LOGI("-------------------  TARGET CONFIG  --------------------\n");
        RT_LOGI("Window-rect X-res = %5d, Y-res = %4d, d%2d, c%2d\n",
                                                    x_win, y_win, d+1, c+1);
        RT_LOGI("SIMD factor*width = %dx%3d, logoff = %d, numoff = %d\n",
                                      k_size, n_simd * 128, l_mode, h_mode);
        RT_LOGI("SIMD type/variant = %d,     reserved %d,   tile = %d\n",
                                        s_type, 0, sc[d]->get_tile_w() / 8);
        RT_LOGI("Framebuffer X-res = %5d, Y-res = %4d, FSAA = %d\n",
                              x_res, y_res, a_mode * 4 / (RT_ELEMENT / 32));
        RT_LOGI("Framebuffer X-row = %5d, ptr = %016"PR_Z"X\n",
                           sc[d]->get_x_row(), (rt_full)sc[d]->get_frame());
        RT_LOGI("Number-of-threads = %5d, offscr = %d, updoff = %d\n",
                                                     thnum, o_mode, u_mode);

        RT_LOGI("----------------------  FPS LOG  -----------------------\n");

        glb = 0;
        run_time = cur_time;

        cnt = 0;
        log_time = cur_time;
    }

    /* update frame counters */
    cnt++;
    ttl++;

    return 1;
}

/*
 * Initialize internal variables from command-line arguments.
 */
rt_si32 args_init(rt_si32 argc, rt_char *argv[])
{
    rt_si32 k, l, r, t;

    if (argc >= 2)
    {
        RT_LOGI("--------------------------------------------------------\n");
        RT_LOGI("Usage options are given below:\n");
        RT_LOGI(" -d n, specify default demo-scene, where 1 <= n <= d_num\n");
        RT_LOGI(" -c n, specify default camera-idx, where 1 <= n <= c_num\n");
        RT_LOGI(" -b n, specify time (ms) at which testing begins, n >= 0\n");
        RT_LOGI(" -e n, specify time (ms) at which testing ends, n >= min\n");
        RT_LOGI(" -f n, specify # of consecutive frames to render, n >= 0\n");
        RT_LOGI(" -g n, specify delta (ms) for consecutive frames, n >= 0\n");
        RT_LOGI(" -k n, override SIMD-size-factor, where new size is 1..4\n");
        RT_LOGI(" -n n, override SIMD-native-size, where new simd is 1..4\n");
        RT_LOGI(" -s n, override SIMD-sub-variant, where new type is 1..8\n");
        RT_LOGI(" -t n, override thread-pool-size, where new size <= 1000\n");
        RT_LOGI(" -w n, override window-rect-size, where new size is 0..9\n");
        RT_LOGI(" -w 0, activate window-less mode, full native resolution\n");
        RT_LOGI(" -x n, override x-resolution, where new x-value <= 65535\n");
        RT_LOGI(" -y n, override y-resolution, where new y-value <= 65535\n");
        RT_LOGI(" -i n, save image at the end of each run, n is image-idx\n");
        RT_LOGI(" -r n, fps-logging update rate, where n is interval (ms)\n");
        RT_LOGI(" -l, fps-logging-off mode, turns off fps-logging updates\n");
        RT_LOGI(" -h, hide-screen-num mode, turns off info-number drawing\n");
        RT_LOGI(" -o, offscreen-frame mode, turns off window-rect updates\n");
        RT_LOGI(" -u, multi-threaded scene updates are turned off, static\n");
        RT_LOGI(" -a, enable antialiasing, 4x for fp32, 2x for fp64 pipes\n");
        RT_LOGI("options -d n, -c n, ... , ... , ... , -a can be combined\n");
        RT_LOGI("--------------------------------------------------------\n");
    }

    for (k = 1; k < argc; k++)
    {
        if (k < argc && strcmp(argv[k], "-d") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t >= 1 && t <= RT_ARR_SIZE(sc_rt))
            {
                RT_LOGI("Demo-scene overridden: %d\n", t);
                d = t-1;
            }
            else
            {
                RT_LOGI("Demo-scene value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-c") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t >= 1 && t <= 65535)
            {
                RT_LOGI("Camera-idx overridden: %d\n", t);
                c = t-1;
            }
            else
            {
                RT_LOGI("Camera-idx value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-b") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t >= 0)
            {
                RT_LOGI("Initial-test-time (ms): %d\n", t);
                b_time = t;
            }
            else
            {
                RT_LOGI("Initial-test-time value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-e") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t >= 1)
            {
                RT_LOGI("Closing-test-time (ms): %d\n", t);
                e_time = t;
            }
            else
            {
                RT_LOGI("Closing-test-time value out of range\n");
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
        if (k < argc && strcmp(argv[k], "-k") == 0 && ++k < argc)
        {
            t = argv[k][0] - '0';
            if (strlen(argv[k]) == 1
            && (t == 1 || t == 2 || t == 4))
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
        if (k < argc && strcmp(argv[k], "-n") == 0 && ++k < argc)
        {
            t = argv[k][0] - '0';
            if (strlen(argv[k]) == 1
            && (t == 1 || t == 2 || t == 4))
            {
                RT_LOGI("SIMD-native-size overridden: %d\n", t);
                n_simd = t;
            }
            else
            {
                RT_LOGI("SIMD-native-size value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-s") == 0 && ++k < argc)
        {
            t = argv[k][0] - '0';
            if (strlen(argv[k]) == 1
            && (t == 1 || t == 2 || t == 4 || t == 8))
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
        if (k < argc && strcmp(argv[k], "-t") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t >= 0 && t <= 1000)
            {
                RT_LOGI("Thread-pool-size overridden: %d\n", t);
                t_pool = t;
            }
            else
            {
                RT_LOGI("Thread-pool-size value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-w") == 0 && ++k < argc)
        {
            t = argv[k][0] - '0';
            if (strlen(argv[k]) == 1 && t >= 0 && t <= 9)
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
                x_res = x_new = t;
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
                y_res = y_new = t;
            }
            else
            {
                RT_LOGI("Y-resolution value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-i") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t >= 0 && t <= 999)
            {
                RT_LOGI("Save-image-index: %d\n", t);
                img_id = t;
            }
            else
            {
                RT_LOGI("Save-image-index value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-r") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1, t = 0; l > 0; l--, r *= 10)
            {
                t += (argv[k][l-1] - '0') * r;
            }
            if (t >= 0)
            {
                RT_LOGI("FPS-logging-interval (ms): %d\n", t);
                l_time = t;
            }
            else
            {
                RT_LOGI("FPS-logging-interval value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-l") == 0 && !l_mode)
        {
            l_mode = RT_TRUE;
            RT_LOGI("FPS-logging-off mode\n");
        }
        if (k < argc && strcmp(argv[k], "-h") == 0 && !h_mode)
        {
            h_mode = RT_TRUE;
            RT_LOGI("Hide-screen-num mode\n");
        }
        if (k < argc && strcmp(argv[k], "-o") == 0 && !o_mode)
        {
            o_mode = RT_TRUE;
            RT_LOGI("Offscreen-frame mode\n");
        }
        if (k < argc && strcmp(argv[k], "-u") == 0 && !u_mode)
        {
            u_mode = RT_TRUE;
            RT_LOGI("Threaded-updates-off\n");
        }
        if (k < argc && strcmp(argv[k], "-a") == 0 && !a_mode)
        {
            a_mode = RT_TRUE;
            RT_LOGI("Antialiasing enabled\n");
        }
    }

    x_res = x_res * (w_size != 0 ? w_size : 1);
    y_res = y_res * (w_size != 0 ? w_size : 1);
    x_row = (x_res+RT_SIMD_WIDTH-1) & ~(RT_SIMD_WIDTH-1);

    thnum = t_pool != 0 ? t_pool : thnum;

    return 1;
}

/*
 * Initialize event loop.
 */
rt_si32 main_init()
{
    rt_si32 size, type, simd = 0;
    rt_si32 i, n = RT_ARR_SIZE(sc_rt);

#if RT_OPTS_STATIC != 0
    if (u_mode)
    {
        i = d;
        n = i + 1;
    }
#endif /* RT_OPTS_STATIC */

    for (i = 0; i < n; i++)
    {
        try
        {
            sc[i] = new rt_Scene(sc_rt[i],
                                x_res, y_res, x_row, frame,
                                sys_alloc, sys_free, thnum,
                                init_threads, term_threads,
                                update_scene, render_scene);

            sc[i]->set_fsaa(a_mode ? RT_FSAA_4X : RT_FSAA_NO);
            simd = sc[i]->set_simd(simd_init(n_simd, s_type, k_size));
        }
        catch (rt_Exception e)
        {
            RT_LOGE("Exception in scene %d: %s\n", i+1, e.err);
            return 0;
        }
    }

    /* test SIMD variables in original command-line format */
    if ((s_type != 0 && s_type != ((simd >> 8) & 0x0F) && k_size == 0)
    ||  (n_simd != 0 && n_simd != ((simd >> 2) & 0x0F) && k_size == 0))
    {
        RT_LOGI("Chosen SIMD target not supported, check -n/-k/-s options\n");
        return 0;
    }

    /* temporarily convert internal SIMD variables to new command-line format */
    simd = from_simd(simd);
    size = (simd >> 16) & 0xFF;
    type = (simd >> 8) & 0xFF;
    simd = simd & 0xFF;

    /* test converted internal SIMD variables against new command-line format */
    if ((k_size != 0 && k_size != size)
    ||  (s_type != 0 && s_type != type && k_size != 0)
    ||  (n_simd != 0 && n_simd != simd && k_size != 0))
    {
        RT_LOGI("Chosen SIMD target not supported, check -n/-k/-s options\n");
        return 0;
    }

    /* update state-tracking SIMD variables from currently chosen SIMD target */
    k_size = size;
    s_type = type;
    n_simd = simd;

    for (; c > 0; c--)
    {
        sc[d]->next_cam();
    }

    c = sc[d]->get_cam_idx();

#if RT_OPTS_STATIC != 0
    if (u_mode)
    {
        rt_si32 opts = sc[d]->get_opts();
        opts |= RT_OPTS_STATIC;
        sc[d]->set_opts(opts);
    }
#endif /* RT_OPTS_STATIC */

    RT_LOGI("-------------------  TARGET CONFIG  --------------------\n");
    RT_LOGI("Window-rect X-res = %5d, Y-res = %4d, d%2d, c%2d\n",
                                                x_win, y_win, d+1, c+1);
    RT_LOGI("SIMD factor*width = %dx%3d, logoff = %d, numoff = %d\n",
                                  k_size, n_simd * 128, l_mode, h_mode);
    RT_LOGI("SIMD type/variant = %d,     reserved %d,   tile = %d\n",
                                    s_type, 0, sc[d]->get_tile_w() / 8);
    RT_LOGI("Framebuffer X-res = %5d, Y-res = %4d, FSAA = %d\n",
                          x_res, y_res, a_mode * 4 / (RT_ELEMENT / 32));
    RT_LOGI("Framebuffer X-row = %5d, ptr = %016"PR_Z"X\n",
                       sc[d]->get_x_row(), (rt_full)sc[d]->get_frame());
    RT_LOGI("Number-of-threads = %5d, offscr = %d, updoff = %d\n",
                                                 thnum, o_mode, u_mode);

    RT_LOGI("----------------------  FPS LOG  -----------------------\n");

    return 1;
}

/*
 * Terminate event loop.
 */
rt_si32 main_term()
{
    if (img_id >= 0 && img_id <= 999)
    {
        sc[d]->save_frame(img_id++);
    }

    RT_LOGI("----------------------  FPS AVG  -----------------------\n");
    if (cur_time - run_time)
    {
        avg = (rt_real)(glb + cnt) * 1000 / (cur_time - run_time);
    }
    else
    {
        avg = (rt_real)0;
    }
    RT_LOGI("AVG = %.2f\n", avg);

    rt_si32 i, n = RT_ARR_SIZE(sc_rt);

    for (i = 0; i < n; i++)
    {
        if (sc[i] == RT_NULL)
        {
            continue;
        }
        try
        {
            delete sc[i];
        }
        catch (rt_Exception e)
        {
            RT_LOGE("Exception in scene %d: %s\n", i+1, e.err);
            return 0;
        }
    }

    return 1;
}

#endif /* RT_ROOT_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
