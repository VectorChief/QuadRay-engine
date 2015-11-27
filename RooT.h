/******************************************************************************/
/* Copyright (c) 2013-2015 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_ROOT_H
#define RT_ROOT_H

#include <string.h>
#include "engine.h"
#include "data/scenes/all_scn.h"

#undef Q /* short name for RT_SIMD_QUADS */
#undef S /* short name for RT_SIMD_WIDTH */
#undef W /* triplet pass-through wrapper */

#define RT_X_RES        800
#define RT_Y_RES        480

rt_astr     title       = "QuadRay engine demo, (C) 2013-2015 VectorChief";

rt_cell     x_res       = RT_X_RES;
rt_cell     y_res       = RT_Y_RES;
rt_cell     x_row       = RT_X_RES;
rt_word    *frame       = RT_NULL;

rt_cell     fsaa        = RT_FSAA_NO; /* no AA */
rt_cell     simd        = 0; /* default SIMD width will be chosen */
rt_cell     type        = 0; /* default SIMD sub-target will be chosen */
rt_cell     hide_num    = 0; /* hide all numbers on the screen if 1 */

rt_SCENE   *sc_rt[]     =
{
    &scn_demo01::sc_root,
    &scn_demo02::sc_root,
};

rt_Scene   *sc[RT_ARR_SIZE(sc_rt)]  = {0};  /* scenes array */
rt_cell     d                       = 1;    /* demo index */

/******************************************************************************/
/********************************   PLATFORM   ********************************/
/******************************************************************************/

/*
 * Initialize platform-specific pool of "thnum" threads.
 */
rt_pntr init_threads(rt_cell thnum, rt_Scene *scn);

/*
 * Terminate platform-specific pool of "thnum" threads.
 */
rt_void term_threads(rt_pntr tdata, rt_cell thnum);

/*
 * Task platform-specific pool of "thnum" threads to update scene,
 * block until finished.
 */
rt_void update_scene(rt_pntr tdata, rt_cell thnum, rt_cell phase);

/*
 * Task platform-specific pool of "thnum" threads to render scene,
 * block until finished.
 */
rt_void render_scene(rt_pntr tdata, rt_cell thnum, rt_cell phase);

/*
 * Get system time in milliseconds.
 */
rt_time get_time();

/*
 * Set current frame to screen.
 */
rt_void frame_to_screen(rt_word *frame);

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
static rt_cell  eout = 0, emax = 0;
static rt_pstr *estr = RT_NULL;

/* time counter variables */
static rt_time init_time = 0;
static rt_time last_time = 0;
static rt_time cur_time = 0;

/* frame counter variables */
static rt_real fps = 0.0f;
static rt_word cnt = 0;
static rt_word scr = 0;

/* virtual keys arrays */
static rt_byte r_to_p[KEY_MASK + 1];
static rt_byte h_keys[KEY_MASK + 1];
static rt_byte t_keys[KEY_MASK + 1];
static rt_byte r_keys[KEY_MASK + 1];

/* hold keys */
#define H_KEYS(k)   (h_keys[r_to_p[(k) & KEY_MASK]])
/* toggle on press */
#define T_KEYS(k)   (t_keys[r_to_p[(k) & KEY_MASK]])
/* toggle on release */
#define R_KEYS(k)   (r_keys[r_to_p[(k) & KEY_MASK]])

/*
 * Event loop's main step.
 */
rt_cell main_step()
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
        if (T_KEYS(RK_F2))
        {
            fsaa = RT_FSAA_4X - fsaa;
            fsaa = sc[d]->set_fsaa(fsaa);
        }
        if (T_KEYS(RK_F3))
        {
            sc[d]->next_cam();
        }
        if (T_KEYS(RK_F4))
        {
            sc[d]->save_frame(scr++);
        }
        if (T_KEYS(RK_F7))
        {
            type = type % 2 + 1;
            type = sc[d]->set_simd(simd | type << 8) >> 8;
        }
        if (T_KEYS(RK_F8))
        {
            simd = simd % 8 + 4;
            simd = sc[d]->set_simd(simd | type << 8) & 0xFF;
        }
        if (T_KEYS(RK_F11))
        {
            d = (d + 1) % RT_ARR_SIZE(sc_rt);
            fsaa = sc[d]->set_fsaa(fsaa);
            simd = sc[d]->set_simd(simd | type << 8);
            type = simd >> 8;
            simd = simd & 0xFF;
        }
        if (T_KEYS(RK_F12))
        {
            hide_num = 1 - hide_num;
        }
        if (T_KEYS(RK_ESCAPE))
        {
            return 0;
        }
        memset(t_keys, 0, sizeof(t_keys));
        memset(r_keys, 0, sizeof(r_keys));

        sc[d]->render(cur_time);

        if (hide_num == 0)
        {
            sc[d]->render_num(x_res-10, 10, -1, 2, (rt_word)fps);
            sc[d]->render_num(      10, 10, +1, 2, (rt_word)simd * 32);
            sc[d]->render_num(x_res-10, 34, -1, 2, (rt_word)fsaa * 4);
            sc[d]->render_num(      10, 34, +1, 2, (rt_word)type);
        }
    }
    catch (rt_Exception e)
    {
        RT_LOGE("Exception: %s\n", e.err);

        return 0;
    }

    if (eout != 0)
    {
        rt_cell i;

        for (i = 0; i < emax; i++)
        {
            if (estr[i] != RT_NULL)
            {            
                RT_LOGE("Exception: thread %d: %s\n", i, estr[i]);
            }
        }

        return 0;
    }

    frame_to_screen(sc[d]->get_frame());

    return 1;
}

/*
 * Initialize event loop.
 */
rt_cell main_init()
{
    rt_cell i, n = RT_ARR_SIZE(sc_rt);

    for (i = 0; i < n; i++)
    {
        try
        {
            sc[i] = new rt_Scene(sc_rt[i],
                                x_res, y_res, x_row, frame,
                                malloc, free,
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

    return 1;
}

/*
 * Terminate event loop.
 */
rt_cell main_term()
{
    rt_cell i, n = RT_ARR_SIZE(sc_rt);

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
