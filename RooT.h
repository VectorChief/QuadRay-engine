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
rt_si32     x_res       = RT_X_RES;
rt_si32     y_res       = RT_Y_RES;
rt_si32     x_row       = (RT_X_RES+RT_SIMD_WIDTH-1) & ~(RT_SIMD_WIDTH-1);
rt_ui32    *frame       = RT_NULL;
rt_si32     thnum       = RT_THREADS_NUM;

rt_si32     q_simd      = 0; /* SIMD quad-factor from command-line */
rt_si32     s_type      = 0; /* SIMD sub-variant from command-line */
rt_si32     t_pool      = 0; /* Thread-pool size from command-line */
#if RT_FULLSCREEN == 1
rt_si32     w_size      = 0; /* Window-rect size from command-line */
#else  /* RT_FULLSCREEN */
rt_si32     w_size      = 1; /* Window-rect size from command-line */
#endif /* RT_FULLSCREEN */
rt_si32     x_new       = 0; /* New x-resolution from command-line */
rt_si32     y_new       = 0; /* New y-resolution from command-line */

rt_bool     u_mode      = RT_FALSE; /* updateoff from command-line */
rt_bool     o_mode      = RT_FALSE; /* offscreen from command-line */
rt_bool     a_mode      = RT_FALSE; /* FSAA mode from command-line */

rt_si32     fsaa        = RT_FSAA_NO; /* no FSAA by default, -a enables */
rt_si32     simd        = 0; /* default SIMD width (q*4) will be chosen */
rt_si32     type        = 0; /* default SIMD sub-variant will be chosen */
rt_si32     hide        = 0; /* hide all numbers on the screen if set 1 */

rt_SCENE   *sc_rt[]     =
{
    &scn_demo01::sc_root,
    &scn_demo02::sc_root,
    &scn_demo03::sc_root,
};

rt_Scene   *sc[RT_ARR_SIZE(sc_rt)]  = {0};                  /* scene array */
rt_si32     d                       = RT_ARR_SIZE(sc_rt)-1; /* demo-index */
rt_si32     c                       = 0;                    /* camera-num */

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
rt_time last_time = 0;
rt_time cur_time = 0;

/* frame counter variables */
rt_real fps = 0.0f;
rt_si32 cnt = 0;
rt_si32 scr = 0;

/* virtual keys arrays */
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

    cur_time = get_time();

    if (init_time == 0)
    {
        init_time = cur_time;
    }

    cur_time = cur_time - init_time;
    cnt++;

    if (cur_time - last_time >= 500)
    {
        fps = (rt_real)cnt * 1000 / (cur_time - last_time);
        RT_LOGI("FPS = %.1f\n", fps);
        cnt = 0;
        last_time = cur_time;
    }

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
            sc[d]->next_cam();
        }
        if (T_KEYS(RK_F11))
        {
            d = (d + 1) % RT_ARR_SIZE(sc_rt);
            fsaa = sc[d]->set_fsaa(fsaa);
            simd = sc[d]->set_simd(simd | type << 8);
            type = simd >> 8;
            simd = simd & 0xFF;
        }

#if RT_OPTS_STATIC != 0
        } /* --<----<-- skip update0 --<----<-- */
#endif /* RT_OPTS_STATIC */

        if (T_KEYS(RK_F2))
        {
            fsaa = RT_FSAA_4X - fsaa;
            fsaa = sc[d]->set_fsaa(fsaa);
        }
        if (T_KEYS(RK_F4))
        {
            sc[d]->save_frame(scr++);
        }
        if (T_KEYS(RK_F7))
        {
            rt_si32 tnew;
            do
            {
                type = type % 8 + type % 7; /* 1, 2, 4, 8 */
                tnew = sc[d]->set_simd(simd | type << 8) >> 8;
            }
            while (type != tnew);
        }
        if (T_KEYS(RK_F8))
        {
            rt_si32 snew;
            do
            {
                simd = simd % 64 + simd % 60; /* 4, 8, 16, 32, 64 */
                snew = sc[d]->set_simd(simd | type << 8) & 0xFF;
                if (simd != snew)
                {
                    rt_si32 tnew = 0;
                    snew = sc[d]->set_simd(simd | tnew << 8);
                    tnew = snew >> 8;
                    snew = snew & 0xFF;
                    if (simd == snew)
                    {
                        type = tnew;
                    }
                }
            }
            while (simd != snew);
        }
        if (T_KEYS(RK_F12))
        {
            hide = 1 - hide;
        }
        if (T_KEYS(RK_ESCAPE))
        {
            return 0;
        }
        memset(t_keys, 0, sizeof(t_keys));
        memset(r_keys, 0, sizeof(r_keys));

        sc[d]->render(cur_time);

        if (hide == 0)
        {
            sc[d]->render_num(x_res-10, 10, -1, 2, (rt_si32)fps);
            sc[d]->render_num(x_res-10, 34, -1, 2, (rt_si32)fsaa * 4
                                                   / (RT_ELEMENT / 32));
            sc[d]->render_num(      10, 10, +1, 2, (rt_si32)simd * 32);
            sc[d]->render_num(      10, 34, +1, 2, (rt_si32)type);
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
        RT_LOGI("---------------------------------------------------------\n");
        RT_LOGI("Usage options are given below:\n");
        RT_LOGI(" -d n, specify default demo scene, where 1 <= n <= limit\n");
        RT_LOGI(" -c n, specify camera in current scene, where n <= 65535\n");
        RT_LOGI(" -q n, override SIMD quad-factor, where new quad is 1..8\n");
        RT_LOGI(" -s n, override SIMD sub-variant, where new type is 1..8\n");
        RT_LOGI(" -t n, override thread-pool size, where new size <= 1000\n");
        RT_LOGI(" -w n, override window-rect size, where new size is 0..9\n");
        RT_LOGI(" -w 0, activate window-less mode, full native resolution\n");
        RT_LOGI(" -x n, override x-resolution, where new x-value <= 65535\n");
        RT_LOGI(" -y n, override y-resolution, where new y-value <= 65535\n");
        RT_LOGI(" -u, multi-threaded scene updates are turned off, static\n");
        RT_LOGI(" -o, offscreen frame mode, turns off window-rect updates\n");
        RT_LOGI(" -a, enable antialiasing, 4x for fp32, 2x for fp64 pipes\n");
        RT_LOGI("options -d n, -c n, -q n, -s n, ... , -a can be combined\n");
        RT_LOGI("---------------------------------------------------------\n");
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
                RT_LOGI("Demo-index overridden: %d\n", t);
                d = t-1;
            }
            else
            {
                RT_LOGI("Demo-index value out of range\n");
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
                RT_LOGI("Camera-num overridden: %d\n", t);
                c = t;
            }
            else
            {
                RT_LOGI("Camera-num value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-q") == 0 && ++k < argc)
        {
            q_simd = argv[k][0] - '0';
            if (strlen(argv[k]) == 1
            && (q_simd == 1 || q_simd == 2 || q_simd == 4 || q_simd == 8))
            {
                RT_LOGI("SIMD quad-factor overridden: %d\n", q_simd);
            }
            else
            {
                RT_LOGI("SIMD quad-factor value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-s") == 0 && ++k < argc)
        {
            s_type = argv[k][0] - '0';
            if (strlen(argv[k]) == 1
            && (s_type == 1 || s_type == 2 || s_type == 4 || s_type == 8))
            {
                RT_LOGI("SIMD sub-variant overridden: %d\n", s_type);
            }
            else
            {
                RT_LOGI("SIMD sub-variant value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-t") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1; l > 0; l--, r *= 10)
            {
                t_pool += (argv[k][l-1] - '0') * r;
            }
            if (t_pool >= 0 && t_pool <= 1000)
            {
                RT_LOGI("Thread-pool size overridden: %d\n", t_pool);
            }
            else
            {
                RT_LOGI("Thread-pool size value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-w") == 0 && ++k < argc)
        {
            w_size = argv[k][0] - '0';
            if (strlen(argv[k]) == 1 && w_size >= 0 && w_size <= 9)
            {
                RT_LOGI("Window-rect size overridden: %d\n", w_size);
            }
            else
            {
                RT_LOGI("Window-rect size value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-x") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1; l > 0; l--, r *= 10)
            {
                x_new += (argv[k][l-1] - '0') * r;
            }
            if (x_new >= 1 && x_new <= 65535)
            {
                RT_LOGI("X-resolution overridden: %d\n", x_new);
                x_res = x_new;
            }
            else
            {
                RT_LOGI("X-resolution value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-y") == 0 && ++k < argc)
        {
            for (l = strlen(argv[k]), r = 1; l > 0; l--, r *= 10)
            {
                y_new += (argv[k][l-1] - '0') * r;
            }
            if (y_new >= 1 && y_new <= 65535)
            {
                RT_LOGI("Y-resolution overridden: %d\n", y_new);
                y_res = y_new;
            }
            else
            {
                RT_LOGI("Y-resolution value out of range\n");
                return 0;
            }
        }
        if (k < argc && strcmp(argv[k], "-u") == 0 && !u_mode)
        {
            u_mode = RT_TRUE;
            RT_LOGI("Threaded updates off\n");
        }
        if (k < argc && strcmp(argv[k], "-o") == 0 && !o_mode)
        {
            o_mode = RT_TRUE;
            RT_LOGI("Offscreen frame mode\n");
        }
        if (k < argc && strcmp(argv[k], "-a") == 0 && !a_mode)
        {
            a_mode = RT_TRUE;
            RT_LOGI("Antialiasing enabled\n");
        }
    }

    simd = q_simd * 4;
    type = s_type * 1;
    fsaa = a_mode ? RT_FSAA_4X : RT_FSAA_NO;

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

            fsaa = sc[i]->set_fsaa(fsaa);
            simd = sc[i]->set_simd(simd | type << 8);
            type = simd >> 8;
            simd = simd & 0xFF;
        }
        catch (rt_Exception e)
        {
            RT_LOGE("Exception in scene %d: %s\n", i+1, e.err);
            return 0;
        }
    }

    if ((s_type != 0 && s_type != ((type / 1) & 0x0F))
    ||  (q_simd != 0 && q_simd != ((simd / 4) & 0x0F)))
    {
        RT_LOGI("Chosen SIMD target is not supported, check -q/-s options\n");
        return 0;
    }

    for (; c > 0; c--)
    {
        sc[d]->next_cam();
    }

#if RT_OPTS_STATIC != 0
    if (u_mode)
    {
        sc[d]->add_opts(RT_OPTS_STATIC);
    }
#endif /* RT_OPTS_STATIC */

    /* always draw empty frame before rendering any scenes */
    frame_to_screen(sc[d]->get_frame(), sc[d]->get_x_row());

    return 1;
}

/*
 * Terminate event loop.
 */
rt_si32 main_term()
{
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
