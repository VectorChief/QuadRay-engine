/******************************************************************************/
/* Copyright (c) 2013-2015 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include "RooT.h"

/******************************************************************************/
/****************************   PLATFORM - LINUX   ****************************/
/******************************************************************************/

#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

static Display    *disp;
static Window      win;
static rt_word     depth;

#include <sys/shm.h>
#include <X11/extensions/XShm.h>

static XShmSegmentInfo shminfo;
static XImage     *ximage      = NULL;
static GC          gc;
static XGCValues   gc_values   = {0};

#include <pthread.h>

/******************************************************************************/
/**********************************   MAIN   **********************************/
/******************************************************************************/

rt_cell main_init();
rt_cell main_loop();
rt_cell main_term();

/*
 * Program's main entry point.
 */
rt_cell main()
{
    /* open connection to X server */
    disp = XOpenDisplay(NULL);
    if (disp == NULL)
    {
        RT_LOGE("Cannot open X display\n");
        return 1;
    }

    /* acquire depth */
    rt_cell screen = DefaultScreen(disp);
    depth = DefaultDepth(disp, screen);

    /* create simple window */
    win = XCreateSimpleWindow(disp, RootWindow(disp, screen),
                                    10, 10, x_res, y_res, 1, 0, 0);
    if ((rt_pntr)win == NULL)
    {
        RT_LOGE("Cannot create X window\n");
        XCloseDisplay(disp);
        return 1;
    }

    /* disable window resize (only a hint) */
    XSizeHints sh;
    sh.flags = PMinSize | PMaxSize;
    sh.min_width  = sh.max_width  = x_res;
    sh.min_height = sh.max_height = y_res;
    XSetWMNormalHints(disp, win, &sh);

#if RT_FULLSCREEN == 1

    /* activate fullscreen */
    Atom atom  = XInternAtom(disp, "_NET_WM_STATE", False);
    Atom state = XInternAtom(disp, "_NET_WM_STATE_FULLSCREEN", False);
    XChangeProperty(disp, win, atom, XA_ATOM, 32,
                    PropModeReplace, (rt_byte *)&state, 1);

#endif /* RT_FULLSCREEN */

    /* set title and events */
    XStoreName(disp, win, title);
    XSelectInput(disp, win, ExposureMask | KeyPressMask | KeyReleaseMask);
    /* map (show) window */
    XMapWindow(disp, win);
    XSync(disp, False);

    /* create image,
     * use preconfigured x_res, y_res for rendering,
     * window resizing in runtime is not supported for now */
    ximage = XShmCreateImage(disp,
                             DefaultVisual(disp, screen),
                             depth,
                             ZPixmap,  NULL, &shminfo,
                             x_res, y_res);
    if (ximage == NULL)
    {
        RT_LOGE("Cannot create XShm image\n");
        XDestroyWindow(disp, win);
        XCloseDisplay(disp);
        return 1;
    }

    /* get shared memory */
    shminfo.shmid = shmget(IPC_PRIVATE,
                           ximage->bytes_per_line * ximage->height,
                           IPC_CREAT|0777);
    if (shminfo.shmid < 0)
    {
        RT_LOGE("shmget failed!\n");
        XDestroyImage(ximage);
        XDestroyWindow(disp, win);
        XCloseDisplay(disp);
        return 1;
    }

    /* attach shared memory */
    shminfo.shmaddr = ximage->data = (rt_char *)shmat(shminfo.shmid, 0, 0);
    if (shminfo.shmaddr == (rt_char *)-1)
    {
        RT_LOGE("shmat failed!\n");
        XDestroyImage(ximage);
        XDestroyWindow(disp, win);
        XCloseDisplay(disp);
        return 1;
    }

    shminfo.readOnly = False;
    XShmAttach(disp, &shminfo);
    shmctl(shminfo.shmid, IPC_RMID, 0);
    gc = XCreateGC(disp, win, 0, &gc_values);
    XSync(disp, False);

    /* use true-color target directly */
    if (depth > 16)
    {
        frame = (rt_word *)ximage->data;
        x_row = ximage->bytes_per_line / 4;
    }

    /* run main loop */
    main_init();
    main_loop();
    main_term();

    /* destroy image,
     * detach shared memory */
    XShmDetach(disp, &shminfo);
    XDestroyImage(ximage);
    shmdt(shminfo.shmaddr);
    XFreeGC(disp, gc);

    /* destroy window */
    XDestroyWindow(disp, win);

    /* close connection to X server */
    XCloseDisplay(disp);

    return 0;
}

/******************************************************************************/
/*****************************   MULTI-THREADING   ****************************/
/******************************************************************************/

/* thread's exception variables */
static rt_cell  eout = 0, emax = 0;
static rt_pstr *estr = RT_NULL;

/* platform-specific thread */
struct rt_THREAD
{
    rt_Scene           *scene;
    rt_cell            *cmd;
    rt_cell             index;
    pthread_t           pthr;
    pthread_barrier_t  *barr;
};

/*
 * Worker thread's entry point.
 */
rt_pntr worker_thread(rt_pntr p)
{
    rt_THREAD *thread = (rt_THREAD *)p;

    while (1)
    {
        pthread_barrier_wait(&thread->barr[0]);

        if (thread->scene == RT_NULL)
        {
            break;
        }

        rt_cell cmd = thread->cmd[0];

        /* if one thread throws an exception,
         * other threads are still allowed to proceed
         * in the same run, but not in the next one */
        if (eout == 0)
        try
        {
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
        }
        catch (rt_Exception e)
        {
            estr[thread->index] = e.err;
            eout = 1;
        }

        pthread_barrier_wait(&thread->barr[1]);
    }

    pthread_barrier_wait(&thread->barr[1]);

    return RT_NULL;
}

/* platform-specific pool
 * of "thnum" threads */
struct rt_THREAD_POOL
{
    rt_Scene           *scene;
    rt_cell             cmd;
    rt_cell             thnum;
    rt_THREAD          *thread;
    pthread_barrier_t   barr[2];
};

/*
 * Initialize platform-specific pool of "thnum" threads.
 */
rt_pntr init_threads(rt_cell thnum, rt_Scene *scn)
{
    eout = 0; emax = thnum;
    estr = (rt_pstr *)malloc(sizeof(rt_pstr) * thnum);

    if (estr == RT_NULL)
    {
        throw rt_Exception("out of memory for estr in init_threads");
    }

    memset(estr, 0, sizeof(rt_pstr) * thnum);

    cpu_set_t cpuset_pr, cpuset_th;
    pthread_t pthr = pthread_self();
    pthread_getaffinity_np(pthr, sizeof(cpu_set_t), &cpuset_pr);

    rt_cell i, a;
    rt_THREAD_POOL *tpool = (rt_THREAD_POOL *)malloc(sizeof(rt_THREAD_POOL));

    if (tpool == RT_NULL)
    {
        throw rt_Exception("out of memory for tpool in init_threads");
    }

    tpool->scene = scn;
    tpool->cmd = 0;
    tpool->thnum = thnum;
    tpool->thread = (rt_THREAD *)malloc(sizeof(rt_THREAD) * thnum);

    if (tpool->thread == RT_NULL)
    {
        throw rt_Exception("out of memory for thread data in init_threads");
    }

    pthread_barrier_init(&tpool->barr[0], NULL, thnum + 1);
    pthread_barrier_init(&tpool->barr[1], NULL, thnum + 1);

    for (i = 0, a = 0; i < thnum; i++, a++)
    {
        rt_THREAD *thread = tpool->thread;

        thread[i].scene  = scn;
        thread[i].cmd    = &tpool->cmd;
        thread[i].index  = i;
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

/*
 * Terminate platform-specific pool of "thnum" threads.
 */
rt_void term_threads(rt_pntr tdata, rt_cell thnum)
{
    rt_cell i;
    rt_THREAD_POOL *tpool = (rt_THREAD_POOL *)tdata;

    for (i = 0; i < tpool->thnum; i++)
    {
        rt_THREAD *thread = tpool->thread;

        thread[i].scene = RT_NULL;
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

    free(estr);
    estr = RT_NULL;
    eout = emax = 0;
}

/*
 * Task platform-specific pool of "thnum" threads to update scene,
 * block until finished.
 */
rt_void update_scene(rt_pntr tdata, rt_cell thnum, rt_cell phase)
{
    rt_THREAD_POOL *tpool = (rt_THREAD_POOL *)tdata;

    tpool->cmd = 1 | ((phase & 0xFF) << 2);
    pthread_barrier_wait(&tpool->barr[0]);
    pthread_barrier_wait(&tpool->barr[1]);
}

/*
 * Task platform-specific pool of "thnum" threads to render scene,
 * block until finished.
 */
rt_void render_scene(rt_pntr tdata, rt_cell thnum, rt_cell phase)
{
    rt_THREAD_POOL *tpool = (rt_THREAD_POOL *)tdata;

    tpool->cmd = 2 | ((phase & 0xFF) << 2);
    pthread_barrier_wait(&tpool->barr[0]);
    pthread_barrier_wait(&tpool->barr[1]);
}

/******************************************************************************/
/*******************************   EVENT-LOOP   *******************************/
/******************************************************************************/

/*
 * Allocate memory from system heap.
 */
static
rt_pntr sys_alloc(rt_word size)
{
    return malloc(size);
}

/*
 * Free memory from system heap.
 */
static
rt_void sys_free(rt_pntr ptr)
{
    free(ptr);
}

/*
 * Initialize event loop.
 */
rt_cell main_init()
{
    try
    {
        scene = new rt_Scene(&sc_root,
                            x_res, y_res, x_row, frame,
                            sys_alloc, sys_free,
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

/* time counter variables */
static rt_time init_time = 0;
static rt_time last_time = 0;
static rt_time cur_time = 0;

/* frame counter variables */
static rt_real fps = 0.0f;
static rt_word cnt = 0;
static rt_word scr = 0;

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

/*
 * Event loop's main step.
 */
rt_cell main_step()
{
    if (scene == RT_NULL)
    {
        return 0;
    }

    gettimeofday(&tm, NULL);
    cur_time = (rt_time)(tm.tv_sec * 1000 + tm.tv_usec / 1000);

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
        if (T_KEYS(XK_F3))      scene->next_cam();
        if (T_KEYS(XK_F4))      scene->save_frame(scr++);
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

    XShmPutImage(disp, win, gc, ximage, 0, 0, 0, 0, x_res, y_res, False);
    XSync(disp, False);

    return 1;
}

/*
 * Terminate event loop.
 */
rt_cell main_term()
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

/*
 * Implementation of the event loop.
 */
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
