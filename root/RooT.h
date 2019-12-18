/******************************************************************************/
/* Copyright (c) 2013-2019 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_ROOT_H
#define RT_ROOT_H

#include <stdlib.h>
#include <string.h>
#include "engine.h"
#include "all_scn.h"

#define RT_TEST_PT      0

#if RT_TEST_PT != 0
#include "../test/scenes/scn_test17.h" /* RaVi - glass cube */
#include "../test/scenes/scn_test18.h" /* smallpt - Cornell box */
#endif /* RT_TEST_PT */

#define RT_X_RES        800
#define RT_Y_RES        480

rt_astr     title       = "QuadRay engine demo, (C) 2013-2019 VectorChief";
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
#if RT_TEST_PT != 0
    &scn_test17::sc_root, /* RaVi - glass cube */
    &scn_test18::sc_root, /* smallpt - Cornell box */
#endif /* RT_TEST_PT */
};

rt_Platform*pfm                     = RT_NULL;              /* platformobj */
rt_Scene   *sc[RT_ARR_SIZE(sc_rt)]  = {0};                  /* scene array */
rt_si32     d                       = RT_ARR_SIZE(sc_rt)-1; /* demo-scene */
rt_si32     c                       = 0;                    /* camera-idx */
rt_si32     tile_w                  = 0;                    /* tile width */

rt_time     b_time      = 0;        /* time-begins-(ms) (from command-line) */
rt_time     e_time      =-1;        /* time-ending-(ms) (from command-line) */
rt_si32     f_num       =-1;        /* number-of-frames (from command-line) */
rt_time     f_time      =-1;        /* frame-delta-(ms) (from command-line) */
rt_si32     n_simd      = 0;        /* SIMD-native-size (from command-line) */
rt_si32     k_size      = 0;        /* SIMD-size-factor (from command-line) */
rt_si32     s_type      = 0;        /* SIMD-sub-variant (from command-line) */
rt_si32     t_pool      = 0;        /* Thread-pool-size (from command-line) */
#if RT_FULLSCREEN == 1
rt_si32     w_size      = 0;        /* Window-rect-size (from command-line) */
#else  /* RT_FULLSCREEN */
rt_si32     w_size      = 1;        /* Window-rect-size (from command-line) */
#endif /* RT_FULLSCREEN */
rt_si32     x_new       = 0;        /* New x-resolution (from command-line) */
rt_si32     y_new       = 0;        /* New y-resolution (from command-line) */

rt_si32     img_id      =-1;        /* save-image-index (from command-line) */
rt_time     l_time      = 500;        /* fpslogupd-(ms) (from command-line) */
rt_bool     l_mode      = RT_FALSE;        /* fpslogoff (from command-line) */
rt_bool     h_mode      = RT_FALSE;        /* hide mode (from command-line) */
rt_bool     p_mode      = RT_FALSE;       /* pause mode (from command-line) */
rt_bool     q_mode      = RT_FALSE;       /* quake mode (from command-line) */
rt_si32     u_mode      = 0; /* update/render threadoff (from command-line) */
rt_bool     o_mode      = RT_FALSE;        /* offscreen (from command-line) */
rt_si32     a_mode      = RT_FSAA_NO;      /* FSAA mode (from command-line) */

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
 * Initialize platform-specific pool of "thnum" threads (< 0 - no feedback).
 */
rt_pntr init_threads(rt_si32 thnum, rt_Platform *pfm);

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

#define RK_X                13
#define RK_C                14

#define RK_UP               15
#define RK_DOWN             16
#define RK_LEFT             17
#define RK_RIGHT            18

#define RK_U                19
#define RK_O                20

#define RK_W                21
#define RK_S                22
#define RK_A                23
#define RK_D                24

#define RK_Q                25
#define RK_E                26

#define RK_I                27
#define RK_L                28
#define RK_P                29

#define RK_T                30
#define RK_Y                31

#define RK_0                50
#define RK_1                51
#define RK_2                52
#define RK_3                53
#define RK_4                54
#define RK_5                55
#define RK_6                56
#define RK_7                57
#define RK_8                58
#define RK_9                59

#define KEY_MASK            0xFF

/* thread's exception variables */
rt_si32  eout = 0, emax = 0;
rt_pstr *estr = RT_NULL;

/* state tracking variables */
rt_si32 d_prev = -1;        /* prev demo-scene */
rt_si32 c_prev = -1;        /* prev camera-idx */
rt_si32 n_prev = -1;        /* prev SIMD-native-size */
rt_si32 k_prev = -1;        /* prev SIMD-size-factor */
rt_si32 s_prev = -1;        /* prev SIMD-sub-variant */
rt_si32 p_prev = -1;        /* prev pause mode */
rt_si32 q_prev = -1;        /* prev quake mode */

/* time counter variables */
rt_time init_time = 0;
rt_time anim_time = 0;
rt_time prev_time = 0;
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
 * Print average fps.
 */
rt_void print_avgfps()
{
    RT_LOGI("---%s%s--------------  FPS AVG  ----- simd = %4dx%dv%d -\n",
      q_prev == 2 ? " P " : p_prev ? " p " : "---", q_prev ? "q " : "--",
                                            n_prev * 128, k_prev, s_prev);
    if (cur_time - run_time > 0)
    {
        avg = (rt_real)(glb + cnt) * 1000 / (cur_time - run_time);
    }
    else
    {
        avg = (rt_real)0;
    }
    RT_LOGI("AVG = %.2f\n", avg);
}

/*
 * Print current target config.
 */
rt_void print_target()
{
    RT_LOGI("-------------------  TARGET CONFIG  --------------------\n");
    RT_LOGI("SIMD size/type = %4dx%dv%d, tile_W = %dxW, FSAA = %d %s\n",
                               n_simd * 128, k_size, s_type, tile_w / 8,
                               1 << a_mode, a_mode ? "(spp)" : "(off)");
    RT_LOGI("Framebuffer X-row = %5d, ptr = %016" PR_Z "X\n",
                       sc[d]->get_x_row(), (rt_full)sc[d]->get_frame());
    RT_LOGI("Framebuffer X-res = %5d, Y-res = %4d, l %d, h %d  %s %s\n",
                                          x_res, y_res, l_mode, h_mode,
            q_mode == 2 ? "P" : p_mode ? "p" : " ", q_mode ? "q" : " ");
    RT_LOGI("Window-rect X-res = %5d, Y-res = %4d, u %d, o %d\n",
                                          x_win, y_win, u_mode, o_mode);
    RT_LOGI("Threads/affinity = %4d/%d, reserved = %d, d%2d, c%2d\n",
                         pfm->get_thnum(), RT_SETAFFINITY, 0, d+1, c+1);

    RT_LOGI("---%s%s--------------  FPS LOG  ----- ptr/fp = %d%s%d --\n",
      q_mode == 2 ? " P " : p_mode ? " p " : "---", q_mode ? "q " : "--",
                    RT_POINTER, RT_ADDRESS == 32 ? "_" : "f", RT_ELEMENT);
}

/*
 * Event loop's main step.
 */
rt_si32 main_step()
{
    if (sc[d] == RT_NULL)
    {
        return 0;
    }

    rt_si32 g = d; /* current scene for save_frame at the end of each run */
    rt_pstr str = "--------------------------------------------------------";

    try
    {
        if (T_KEYS(RK_F4) || T_KEYS(RK_4))
        {
            sc[d]->save_frame(scr_id++);
            switched = 1;
        }
        if (T_KEYS(RK_F5) || T_KEYS(RK_L))
        {
            l_mode = !l_mode;
            switched = 1;
        }
        if (T_KEYS(RK_P))
        {
            if (q_mode)
            {
                p_mode = q_mode - 1;
                p_mode = !p_mode;
                q_mode = 1 + p_mode;
                p_mode = RT_TRUE;
                q_prev = q_mode; /* <-- update here as switched is 0 */
                RT_LOGI("%s\n", str);
                RT_LOGI("---%s%s----- changing inherited pause mode --%s\n",
                            q_mode == 2 ? " P " : " p ", "q ", "----------");
                RT_LOGI("%s\n", str);
            }
            else
            {
                p_prev = p_mode;
                p_mode = !p_mode;
                switched = 1;
            }
        }
        if (T_KEYS(RK_Q))
        {
            p_prev = p_mode;
            q_prev = q_mode;
            if (q_mode)
            {
                p_mode = q_mode - 1;
                q_mode = 0;
            }
            else
            {
                q_mode = 1 + p_mode;
                p_mode = RT_TRUE;
            }

            rt_si32 q_test = sc[d]->set_pton(q_mode != 0);
            if (q_test != (q_mode != 0))
            {
                p_mode = p_prev;
                q_mode = q_prev;
                /* to enable path-tracer in a particular scene
                 * add RT_OPTS_PT to the list of optimizations
                 * to be turned off in scene definition struct */
                RT_LOGI("%s\n", str);
                RT_LOGI("Quasi-realistic mode: %d (off), %s\n", q_mode,
                                            "add RT_OPTS_PT per scene");
                RT_LOGI("%s\n", str);
            }
            else
            {
                switched = 1;
            }
        }
        if (T_KEYS(RK_9) || T_KEYS(RK_U))
        {
            rt_si32 opts = sc[d]->get_opts();
            u_mode = (u_mode + 1) % 7;
            opts &= ~RT_OPTS_UPDATE_EXT0 & ~RT_OPTS_UPDATE_EXT1 &
                    ~RT_OPTS_UPDATE_EXT2 & ~RT_OPTS_UPDATE_EXT3 &
                    ~RT_OPTS_RENDER_EXT0 & ~RT_OPTS_RENDER_EXT1;
            switch (u_mode)
            {
                case 6:
                opts |= RT_OPTS_RENDER_EXT0;
                case 5:
                opts |= RT_OPTS_UPDATE_EXT0;
                break;

                case 4:
                opts |= RT_OPTS_RENDER_EXT1;
                case 3:
                opts |= RT_OPTS_UPDATE_EXT3;
                case 2:
                opts |= RT_OPTS_UPDATE_EXT2;
                case 1:
                opts |= RT_OPTS_UPDATE_EXT1;
                break;

                default:
                break;
            }
            sc[d]->set_opts(opts);
            switched = 1;
        }
        if (T_KEYS(RK_0) || T_KEYS(RK_O))
        {
            o_mode = !o_mode;
            switched = 1;
        }
        if (T_KEYS(RK_F12) || T_KEYS(RK_5))
        {
            h_mode = !h_mode;
            switched = 1;
        }
        if (T_KEYS(RK_ESCAPE))
        {
            return 0;
        }

        /* update time variables */
        cur_time = get_time();
        if (init_time == 0)
        {
            init_time = cur_time - b_time;
            anim_time = run_time = log_time = prev_time = b_time;
        }
        cur_time = cur_time - init_time;

        if (!p_mode)
        {
            anim_time += (cur_time - prev_time);
        }
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
        if (e_time >= 0 && anim_time >= e_time)
        {
            return 0;
        }
        if (f_num >= 0 && ttl >= f_num)
        {
            return 0;
        }

#if RT_OPTS_UPDATE_EXT0 != 0
        if (u_mode <= 4 && !p_mode)
        { /* -->---->-- skip update0 -->---->-- */
#endif /* RT_OPTS_UPDATE_EXT0 */

        if (H_KEYS(RK_W))     sc[d]->update(anim_time, RT_CAMERA_MOVE_FORWARD);
        if (H_KEYS(RK_S))     sc[d]->update(anim_time, RT_CAMERA_MOVE_BACK);
        if (H_KEYS(RK_A))     sc[d]->update(anim_time, RT_CAMERA_MOVE_LEFT);
        if (H_KEYS(RK_D))     sc[d]->update(anim_time, RT_CAMERA_MOVE_RIGHT);

        if (H_KEYS(RK_UP))    sc[d]->update(anim_time, RT_CAMERA_ROTATE_DOWN);
        if (H_KEYS(RK_DOWN))  sc[d]->update(anim_time, RT_CAMERA_ROTATE_UP);
        if (H_KEYS(RK_LEFT))  sc[d]->update(anim_time, RT_CAMERA_ROTATE_LEFT);
        if (H_KEYS(RK_RIGHT)) sc[d]->update(anim_time, RT_CAMERA_ROTATE_RIGHT);

        if (T_KEYS(RK_F1) || T_KEYS(RK_I))
        {
            sc[d]->print_state();
        }
        if (T_KEYS(RK_F2) || T_KEYS(RK_2))
        {
            a_mode = (a_mode + 1) % (pfm->get_fsaa_max() + 1);
            a_mode = pfm->set_fsaa(a_mode);
            switched = 1;
        }
        if (T_KEYS(RK_F3) || T_KEYS(RK_3))
        {
            c_prev = c;
            c = sc[d]->next_cam();
            switched = c_prev != c ? 1 : switched;
        }
        if (T_KEYS(RK_F6) || T_KEYS(RK_6))
        {
            k_prev = k_size;
            rt_si32 size, type, simd;
            do
            {
                k_size = k_size % 4 + k_size % 3; /* 1, 2, 4 */
                simd = pfm->set_simd(simd_init(n_simd, s_type, k_size));
                size = (simd >> 16) & 0xFF;
                type = (simd >> 8) & 0xFF;
                simd = simd & 0xFF;
                if (size != k_size)
                {
                    simd = pfm->set_simd(simd_init(n_simd, 0, k_size));
                    size = (simd >> 16) & 0xFF;
                    type = (simd >> 8) & 0xFF;
                    simd = simd & 0xFF;
                }
                if (size == k_size && simd == n_simd)
                {
                    s_type = type;
                }
                if (simd != n_simd)
                {
                    size = 0;
                }
            }
            while (size != k_size);
            a_mode = pfm->get_fsaa();
            switched = k_prev != k_size ? 1 : switched;
        }
        if (T_KEYS(RK_F7) || T_KEYS(RK_7))
        {
            s_prev = s_type;
            rt_si32 size, type, simd;
            do
            {
                s_type = s_type % 32 + s_type % 31; /* 1, 2, 4, 8, 16, 32 */
                simd = pfm->set_simd(simd_init(n_simd, s_type, k_size));
                size = (simd >> 16) & 0xFF;
                type = (simd >> 8) & 0xFF;
                simd = simd & 0xFF;
                if (simd != n_simd || size != k_size)
                {
                    type = 0;
                }
            }
            while (type != s_type);
            a_mode = pfm->get_fsaa();
            switched = s_prev != s_type ? 1 : switched;
        }
        if (T_KEYS(RK_F8) || T_KEYS(RK_8))
        {
            n_prev = n_simd;
            rt_si32 size, type, simd;
            do
            {
                n_simd = n_simd % 16 + n_simd % 15; /* 1, 2, 4, 8, 16 */
                simd = pfm->set_simd(simd_init(n_simd, s_type, k_size));
                size = (simd >> 16) & 0xFF;
                type = (simd >> 8) & 0xFF;
                simd = simd & 0xFF;
                if (simd != n_simd)
                {
                    simd = pfm->set_simd(simd_init(n_simd, 0, k_size));
                    size = (simd >> 16) & 0xFF;
                    type = (simd >> 8) & 0xFF;
                    simd = simd & 0xFF;
                }
                if (simd != n_simd)
                {
                    simd = pfm->set_simd(simd_init(n_simd, 0, 0));
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
            a_mode = pfm->get_fsaa();
            switched = n_prev != n_simd ? 1 : switched;
        }
        if (T_KEYS(RK_F11) || T_KEYS(RK_1))
        {
            d_prev = d;
            d = (d + 1) % RT_ARR_SIZE(sc_rt);
            c = sc[d]->get_cam_idx();
            pfm->set_cur_scene(sc[d]);
            switched = d_prev != d ? 1 : switched;
        }

#if RT_OPTS_UPDATE_EXT0 != 0
        } /* --<----<-- skip update0 --<----<-- */
#endif /* RT_OPTS_UPDATE_EXT0 */

        memset(t_keys, 0, sizeof(t_keys));
        memset(r_keys, 0, sizeof(r_keys));

        if (switched && img_id >= 0 && img_id <= 999)
        {
            sc[g]->save_frame(img_id++);
        }

        if (switched)
        {
            switched = 0;

            print_avgfps();
            print_target();

            d_prev = d;
            c_prev = c;
            n_prev = n_simd;
            k_prev = k_size;
            s_prev = s_type;
            p_prev = p_mode;
            q_prev = q_mode;

            glb = 0;
            run_time = cur_time;

            cnt = 0;
            log_time = cur_time;
        }

        prev_time = cur_time;

        /* update frame counters */
        cnt++;
        ttl++;

        sc[d]->render(f_time >= 0 ? b_time + f_time * ttl : anim_time);

        if (!h_mode)
        {
            sc[d]->render_num(x_res-30, 10, -1, 2, (rt_si32)fps);
            sc[d]->render_num(x_res-10, 10, -1, 2, tile_w / 8);
            sc[d]->render_num(x_res-10, 34, -1, 2, 1 << a_mode);
            sc[d]->render_num(      30, 10, +1, 2, n_simd * 128);
            sc[d]->render_num(      10, 10, +1, 2, k_size);
            sc[d]->render_num(      10, 34, +1, 2, s_type);
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
        RT_LOGI(" -n n, override SIMD-native-size, where new simd is 1.16\n");
        RT_LOGI(" -k n, override SIMD-size-factor, where new size is 1..4\n");
        RT_LOGI(" -s n, override SIMD-sub-variant, where new type is 1.32\n");
        RT_LOGI(" -t n, override thread-pool-size, where new size <= 1000\n");
        RT_LOGI(" -w n, override window-rect-size, where new size is 1..9\n");
        RT_LOGI(" -w 0, activate window-less mode, full native resolution\n");
        RT_LOGI(" -x n, override x-resolution, where new x-value <= 65535\n");
        RT_LOGI(" -y n, override y-resolution, where new y-value <= 65535\n");
        RT_LOGI(" -i n, save image at the end of each run, n is image-idx\n");
        RT_LOGI(" -r n, fps-logging update rate, where n is interval (ms)\n");
        RT_LOGI(" -l, fps-logging-off mode, turns off fps-logging updates\n");
        RT_LOGI(" -h, hide-screen-num mode, turns off info-number drawing\n");
        RT_LOGI(" -p, pause mode, stops animation from starting time (ms)\n");
        RT_LOGI(" -q, quake mode, enables path-tracing for quality lights\n");
        RT_LOGI(" -u n, 1-3/4 serial update/render, 5/6 update/render off\n");
        RT_LOGI(" -o, offscreen-frame mode, turns off window-rect updates\n");
        RT_LOGI(" -a n, enable antialiasing, 2 for 2x, 4 for 4x, 8 for 8x\n");
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
        if (k < argc && strcmp(argv[k], "-i") == 0)
        {
            l = 0;
            if (++k < argc)
            {
                if (argv[k][0] != '-')
                {
                    l = strlen(argv[k]);
                }
                else
                {
                    k--;
                }
            }
            for (r = 1, t = 0; l > 0; l--, r *= 10)
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
                RT_LOGI("FPS-logging-interval (ms) overridden: %d\n", t);
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
            RT_LOGI("FPS-logging-off mode: %d\n", l_mode);
        }
        if (k < argc && strcmp(argv[k], "-h") == 0 && !h_mode)
        {
            h_mode = RT_TRUE;
            RT_LOGI("Hide-screen-num mode: %d\n", h_mode);
        }
        if (k < argc && strcmp(argv[k], "-p") == 0 && !p_mode)
        {
            p_mode = RT_TRUE;
            RT_LOGI("Pause-animation mode: %d\n", p_mode);
        }
        if (k < argc && strcmp(argv[k], "-q") == 0 && !q_mode)
        {
            q_mode = RT_TRUE;
            RT_LOGI("Quasi-realistic mode: %d\n", q_mode);
        }
        if (k < argc && strcmp(argv[k], "-u") == 0 && ++k < argc)
        {
            t = argv[k][0] - '0';
            if (strlen(argv[k]) == 1 && t >= 0 && t <= 6)
            {
                RT_LOGI("Threaded-updates-off phases-up-to: %d\n", t);
                u_mode = t;
            }
            else
            {
                RT_LOGI("Threaded-updates-off value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-o") == 0 && !o_mode)
        {
            o_mode = RT_TRUE;
            RT_LOGI("Offscreen-frame mode: %d\n", o_mode);
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

    x_res = x_res * (w_size != 0 ? w_size : 1);
    y_res = y_res * (w_size != 0 ? w_size : 1);
    x_row = (x_res+RT_SIMD_WIDTH-1) & ~(RT_SIMD_WIDTH-1);

    thnum = t_pool != 0 ? -t_pool : thnum; /* no feedback (< 0) if overridden */

    return 1;
}

/*
 * Initialize event loop.
 */
rt_si32 main_init()
{
    rt_si32 size, type, simd = 0;
    rt_si32 i, n = RT_ARR_SIZE(sc_rt);

    try
    {
        i = -1;
        pfm = new rt_Platform(sys_alloc, sys_free, thnum,
                              init_threads, term_threads,
                              update_scene, render_scene);

        simd = pfm->set_simd(simd_init(n_simd, s_type, k_size));
        a_mode = pfm->set_fsaa(a_mode);
        tile_w = pfm->get_tile_w();

        for (i = 0; i < n; i++)
        {
            sc[i] = new(pfm) rt_Scene(sc_rt[i],
                                      x_res, y_res, x_row, frame, pfm);
        }

        pfm->set_cur_scene(sc[d]);
    }
    catch (rt_Exception e)
    {
        RT_LOGE("Exception in main_init, %s %d: %s\n",
                i+1 ? "scene" : "platform", i+1, e.err);
        return 0;
    }

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

    for (; c > 0; c--)
    {
        sc[d]->next_cam();
    }

    c = sc[d]->get_cam_idx();

    rt_si32 opts = sc[d]->get_opts();
    switch (u_mode)
    {
        case 6:
        opts |= RT_OPTS_RENDER_EXT0;
        case 5:
        opts |= RT_OPTS_UPDATE_EXT0;
        break;

        case 4:
        opts |= RT_OPTS_RENDER_EXT1;
        case 3:
        opts |= RT_OPTS_UPDATE_EXT3;
        case 2:
        opts |= RT_OPTS_UPDATE_EXT2;
        case 1:
        opts |= RT_OPTS_UPDATE_EXT1;
        break;

        default:
        break;
    }
    sc[d]->set_opts(opts);

    rt_si32 q_test = sc[d]->set_pton(q_mode != 0);
    if (q_test != (q_mode != 0))
    {
        q_mode = RT_FALSE;
        /* to enable path-tracer in a particular scene
         * add RT_OPTS_PT to the list of optimizations
         * to be turned off in scene definition struct */
        RT_LOGI("Quasi-realistic mode: %d (off), %s\n", q_mode,
                                    "add RT_OPTS_PT per scene");
    }
    if (q_mode)
    {
        q_mode += p_mode;
        p_mode = RT_TRUE;
    }

    d_prev = d;
    c_prev = c;
    n_prev = n_simd;
    k_prev = k_size;
    s_prev = s_type;
    p_prev = p_mode;
    q_prev = q_mode;

    print_target();

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

    print_avgfps();

    rt_si32 i, n = RT_ARR_SIZE(sc_rt);

    try
    {
        for (i = 0; i < n; i++)
        {
            if (sc[i] == RT_NULL)
            {
                continue;
            }
            delete sc[i];
        }

        i = -1;
        delete pfm;
    }
    catch (rt_Exception e)
    {
        RT_LOGE("Exception in main_term, %s %d: %s\n",
                i+1 ? "scene" : "platform", i+1, e.err);
        return 0;
    }

    return 1;
}

#endif /* RT_ROOT_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
