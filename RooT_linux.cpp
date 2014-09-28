/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include "RooT.h"

/******************************************************************************/
/****************************   PLATFORM - LINUX   ****************************/
/******************************************************************************/

#include <malloc.h>
#include <string.h>
#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

Display    *disp;
Window      win;
rt_word     depth;

#include <sys/shm.h>
#include <X11/extensions/XShm.h>

XShmSegmentInfo shminfo;
XImage     *ximage      = NULL;
GC          gc;
XGCValues   gc_values   = {0};

#include <pthread.h>

/******************************************************************************/
/**********************************   MAIN   **********************************/
/******************************************************************************/

rt_cell main_init();
rt_cell main_loop();
rt_cell main_done();

rt_cell main(rt_cell argc, rt_char *argv[])
{
    rt_cell i;

    if (argc > 1)
    {
        RT_LOGI("\n");
        RT_LOGI("argc = %d\n", argc);
        for (i = 0; i < argc; i++)
        {
            RT_LOGI("argv[%d] = %s\n", i, argv[i]);
        }
    }

    if (argc >= 3 && strcmp(argv[1], "-t") == 0)
    {
        RT_LOGI("Converting textures\n");
        for (i = 2; i < argc; i++)
        {
            convert_texture(argv[i]);
        }
        RT_LOGI("\nDone!\n");
        return 0;
    }

    /* open connection to Xserver */
    disp = XOpenDisplay(NULL);
    /* RT_LOGI("Display = %p\n", disp); */
    if (disp == NULL)
    {
        RT_LOGE("Cannot open display\n");
        return 1;
    }

    rt_cell screen = DefaultScreen(disp);
    /* RT_LOGI("Screen = %X\n", screen); */

    XVisualInfo vi;
    XMatchVisualInfo(disp, screen, 24, TrueColor, &vi);
    /* RT_LOGI("Visual = %p\n", vi.visual); */

    /* create simple window */
    win = XCreateSimpleWindow(disp, RootWindow(disp, screen),
                                    10, 10, x_res, y_res, 1, 0, 0);
    /* RT_LOGI("Window = %p\n", (rt_pntr)win); */
    if ((rt_pntr)win == NULL)
    {
        RT_LOGE("Cannot create window\n");
        return 1;
    }

#if RT_FULLSCREEN == 1

    Atom atom  = XInternAtom(disp, "_NET_WM_STATE", False);
    Atom state = XInternAtom(disp, "_NET_WM_STATE_FULLSCREEN", False);
    XChangeProperty(disp, win, atom,
                    XA_ATOM, 32, PropModeReplace,
                    (rt_byte *)&state, 1);

#endif /* RT_FULLSCREEN */

    /* set title */
    XStoreName(disp, win, title);
    /* select kind of events we are interested in */
    XSelectInput(disp, win, ExposureMask | KeyPressMask | KeyReleaseMask);
    /* map (show) the window */
    XMapWindow(disp, win);
    XSync(disp, False);

    Window win_root;
    rt_cell win_x = 0, win_y = 0;
    rt_word win_w = 0, win_h = 0;
    rt_word win_b = 0, win_d = 0;

    /* Not reliable query mechanism to determine fullscreen window size.
     * It sporadically returns either original size or fullscreen size,
     * therefore always use original x_res, y_res for rendering. */
    XGetGeometry(disp, win, &win_root,
                &win_x, &win_y,
                &win_w, &win_h,
                &win_b, &win_d);
    /*
    RT_LOGI("XWindow W = %d\n", win_w);
    RT_LOGI("XWindow H = %d\n", win_h);
    RT_LOGI("XWindow B = %d\n", win_b);
    RT_LOGI("XWindow D = %d\n", win_d);
    */
    depth = win_d;

    /* create image */
    ximage = XShmCreateImage(disp,
                            DefaultVisual(disp, screen),
                            DefaultDepth(disp, screen),
                            ZPixmap,  NULL, &shminfo,
                            x_res, y_res);

    if (ximage == NULL)
    {
        RT_LOGE("XShmCreateImage failed!\n");
        return 1;
    }

    shminfo.shmid = shmget(IPC_PRIVATE,
                    ximage->bytes_per_line * ximage->height, IPC_CREAT|0777);

    if (shminfo.shmid < 0)
    {
        RT_LOGE("shmget failed!\n");
        XDestroyImage(ximage);
        return 1;
    }

    shminfo.shmaddr = ximage->data = (rt_char *)shmat(shminfo.shmid, 0, 0);

    if (shminfo.shmaddr == (rt_char *)-1)
    {
        RT_LOGE("shmat failed!\n");
        XDestroyImage(ximage);
        return 1;
    }

    shminfo.readOnly = False;
    XShmAttach(disp, &shminfo);

    shmctl(shminfo.shmid, IPC_RMID, 0);

    gc = XCreateGC(disp, win, 0, &gc_values);
    XSync(disp, False);

    if (depth > 16)
    {
        frame = (rt_word *)ximage->data;
        x_row = ximage->bytes_per_line / 4;
    }

    rt_cell ret;

    ret = main_init();
    ret = main_loop();
    ret = main_done();

    /* destroy image */
    XShmDetach(disp, &shminfo);
    XDestroyImage(ximage);
    shmdt(shminfo.shmaddr);
    XFreeGC(disp, gc);

    /* destroy window */
    XDestroyWindow(disp, win);

    /* close connection to Xserver */
    XCloseDisplay(disp);

    return 0;
}

/******************************************************************************/
/********************************   THREADING   *******************************/
/******************************************************************************/

struct rt_THREAD
{
    rt_cell            *cmd;
    rt_cell             index;
    rt_Scene           *scene;
    pthread_barrier_t  *barr;
    pthread_t           pthr;
};

rt_pntr worker_thread(rt_pntr p)
{
    rt_THREAD *thread = (rt_THREAD *)p;

    while (1)
    {
        pthread_barrier_wait(&thread->barr[0]);

        if (!thread->scene)
        {
            break;
        }

        rt_cell cmd = thread->cmd[0];

        switch (cmd & 0x3)
        {
            case 1:
            thread->scene->update_slice(thread->index, (cmd >> 2) & 0xFF);
            break;

            case 2:
            thread->scene->render_slice(thread->index, (cmd >> 2) & 0xFF);
            break;

            default:
            break;
        };

        pthread_barrier_wait(&thread->barr[1]);
    }

    pthread_barrier_wait(&thread->barr[1]);

    return NULL;
}

struct rt_THREAD_POOL
{
    rt_cell             thnum;
    rt_THREAD          *thread;
    rt_Scene           *scene;
    pthread_barrier_t   barr[2];
    rt_cell             cmd;
};

rt_pntr init_threads(rt_cell thnum, rt_Scene *scn)
{
    cpu_set_t cpuset_pr, cpuset_th;
    pthread_t pthr = pthread_self();
    pthread_getaffinity_np(pthr, sizeof(cpu_set_t), &cpuset_pr);

    rt_cell i, a;
    rt_THREAD_POOL *tpool = (rt_THREAD_POOL *)malloc(sizeof(rt_THREAD_POOL));

    tpool->scene = scn;
    tpool->thread = (rt_THREAD *)malloc(sizeof(rt_THREAD) * thnum);

    pthread_barrier_init(&tpool->barr[0], NULL, thnum + 1);
    pthread_barrier_init(&tpool->barr[1], NULL, thnum + 1);

    tpool->cmd = 0;
    tpool->thnum = thnum;

    for (i = 0, a = 0; i < thnum; i++, a++)
    {
        rt_THREAD *thread = tpool->thread;

        thread[i].cmd    = &tpool->cmd;
        thread[i].index  = i;
        thread[i].scene  = scn;
        thread[i].barr   = tpool->barr;
        pthread_create(&thread[i].pthr, NULL, worker_thread, &thread[i]);

        while (!CPU_ISSET(a, &cpuset_pr))
        {
            a++;
            if (a == CPU_SETSIZE) a = 0;
        }
        CPU_ZERO(&cpuset_th);
        CPU_SET(a, &cpuset_th);
        pthread_setaffinity_np(thread[i].pthr, sizeof(cpu_set_t), &cpuset_th);
    }

    return tpool;
}

rt_void term_threads(rt_pntr tdata, rt_cell thnum)
{
    rt_cell i;
    rt_THREAD_POOL *tpool = (rt_THREAD_POOL *)tdata;

    for (i = 0; i < tpool->thnum; i++)
    {
        rt_THREAD *thread = tpool->thread;

        thread[i].scene = NULL;
    }

    pthread_barrier_wait(&tpool->barr[0]);
    pthread_barrier_wait(&tpool->barr[1]);

    for (i = 0; i < tpool->thnum; i++)
    {
        rt_THREAD *thread = tpool->thread;

        pthread_join(thread[i].pthr, NULL);

        thread[i].barr = NULL;
    }

    pthread_barrier_destroy(&tpool->barr[0]);
    pthread_barrier_destroy(&tpool->barr[1]);

    free(tpool->thread);
    free(tpool);
}

rt_void update_scene(rt_pntr tdata, rt_cell thnum, rt_cell phase)
{
    rt_THREAD_POOL *tpool = (rt_THREAD_POOL *)tdata;

    tpool->cmd = 1 | ((phase & 0xFF) << 2);
    pthread_barrier_wait(&tpool->barr[0]);
    pthread_barrier_wait(&tpool->barr[1]);
}

rt_void render_scene(rt_pntr tdata, rt_cell thnum, rt_cell phase)
{
    rt_THREAD_POOL *tpool = (rt_THREAD_POOL *)tdata;

    tpool->cmd = 2 | ((phase & 0xFF) << 2);
    pthread_barrier_wait(&tpool->barr[0]);
    pthread_barrier_wait(&tpool->barr[1]);
}

/******************************************************************************/
/********************************   RENDERING   *******************************/
/******************************************************************************/

rt_cell main_init()
{
    try
    {
        scene = new rt_Scene(&sc_root,
                            x_res, y_res, x_row, frame,
                            malloc, free,
                            init_threads, term_threads,
                            update_scene, render_scene);
    }
    catch (rt_Exception e)
    {
        RT_LOGE("Exception: %s\n", e.err);

        return 0;
    }

    return 1;
}

/* performance variables */
static struct timeval tm;

/* time counter varibales */
static rt_long init_time = 0;
static rt_long last_time = 0;
static rt_long cur_time = 0;
static rt_real fps = 0.0f;
static rt_word cnt = 0;

/* virtual keys arrays */
#define KEY_MASK    0x01FF
static rt_byte h_keys[KEY_MASK + 1];
static rt_byte t_keys[KEY_MASK + 1];
static rt_byte r_keys[KEY_MASK + 1];

/* hold keys */
#define H_KEYS(k)   (h_keys[(k) & KEY_MASK])
/* toggle on press */
#define T_KEYS(k)   (t_keys[(k) & KEY_MASK])
/* toggle on release */
#define R_KEYS(k)   (r_keys[(k) & KEY_MASK])

rt_cell main_step()
{
    if (scene == RT_NULL)
    {
        return 0;
    }

    gettimeofday(&tm, NULL);
    cur_time = tm.tv_sec * 1000 + tm.tv_usec / 1000;

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
        if (H_KEYS(XK_w))       scene->update(cur_time, RT_CAMERA_MOVE_FORWARD);
        if (H_KEYS(XK_s))       scene->update(cur_time, RT_CAMERA_MOVE_BACK);
        if (H_KEYS(XK_a))       scene->update(cur_time, RT_CAMERA_MOVE_LEFT);
        if (H_KEYS(XK_d))       scene->update(cur_time, RT_CAMERA_MOVE_RIGHT);

        if (H_KEYS(XK_Up))      scene->update(cur_time, RT_CAMERA_ROTATE_DOWN);
        if (H_KEYS(XK_Down))    scene->update(cur_time, RT_CAMERA_ROTATE_UP);
        if (H_KEYS(XK_Left))    scene->update(cur_time, RT_CAMERA_ROTATE_LEFT);
        if (H_KEYS(XK_Right))   scene->update(cur_time, RT_CAMERA_ROTATE_RIGHT);

        if (T_KEYS(XK_F1))      scene->print_state();
        if (T_KEYS(XK_F2))      fsaa = RT_FSAA_4X - fsaa;
        if (T_KEYS(XK_Escape))
        {
            return 0;
        }
        memset(t_keys, 0, sizeof(t_keys));
        memset(r_keys, 0, sizeof(r_keys));

        scene->set_fsaa(fsaa);
        scene->render(cur_time);
        scene->render_fps(x_res - 10, 10, -1, 2, (rt_word)fps);

        if (depth == 16)
        {
            frame = scene->get_frame();
            rt_half *idata = (rt_half *)ximage->data;
            rt_cell i = x_res * y_res;

            while (i-->0)
            {
                idata[i] = (frame[i] & 0x00F80000) >> 8 |
                           (frame[i] & 0x0000FC00) >> 5 |
                           (frame[i] & 0x000000F8) >> 3;
            }
        }
    }
    catch (rt_Exception e)
    {
        RT_LOGE("Exception: %s\n", e.err);

        return 0;
    }

    XShmPutImage(disp, win, gc, ximage, 0, 0, 0, 0, x_res, y_res, False);
    XSync(disp, False);

    return 1;
}

rt_cell main_done()
{
    if (scene == RT_NULL)
    {
        return 0;
    }

    try
    {
        delete scene;
    }
    catch (rt_Exception e)
    {
        RT_LOGE("Exception: %s\n", e.err);

        return 0;
    }

    return 1;
}

rt_cell main_loop()
{
    /* event loop */
    while (1)
    {
        rt_cell ret, key;

        while (XPending(disp))
        {
            XEvent event;

            XNextEvent(disp, &event);

            if (event.type == KeyPress)
            {
                key = XLookupKeysym((XKeyEvent *)&event, 0);
                /* RT_LOGI("Key press   = %X\n", key); */

                key &= KEY_MASK;
                if (h_keys[key] == 0)
                {
                    t_keys[key] = 1;
                }
                h_keys[key] = 1;
            }

            if (event.type == KeyRelease)
            {
                key = XLookupKeysym((XKeyEvent *)&event, 0);
                /* RT_LOGI("Key release = %X\n", key); */

                /* eat key release from repetitions */
                if (XPending(disp) && XPeekEvent(disp, &event)
                &&  event.type == KeyPress
                &&  XLookupKeysym((XKeyEvent *)&event, 0) == key)
                {
                    continue;
                }

                key &= KEY_MASK;
                h_keys[key] = 0;
                r_keys[key] = 1;
            }
        }

        ret = main_step();

        if (ret == 0)
        {
            break;
        }
    }

    return 0;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
