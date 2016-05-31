/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
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
static rt_si32     depth;

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

rt_si32 main_init();
rt_si32 main_loop();
rt_si32 main_term();

/*
 * Program's main entry point.
 */
rt_si32 main()
{
    /* fill in platform's keymap */
    r_to_p[RK_W]        = KEY_MASK & XK_w;
    r_to_p[RK_S]        = KEY_MASK & XK_s;
    r_to_p[RK_A]        = KEY_MASK & XK_a;
    r_to_p[RK_D]        = KEY_MASK & XK_d;

    r_to_p[RK_UP]       = KEY_MASK & XK_Up;   
    r_to_p[RK_DOWN]     = KEY_MASK & XK_Down; 
    r_to_p[RK_LEFT]     = KEY_MASK & XK_Left; 
    r_to_p[RK_RIGHT]    = KEY_MASK & XK_Right;

    r_to_p[RK_F1]       = KEY_MASK & XK_F1;   
    r_to_p[RK_F2]       = KEY_MASK & XK_F2;   
    r_to_p[RK_F3]       = KEY_MASK & XK_F3;   
    r_to_p[RK_F4]       = KEY_MASK & XK_F4;   
    r_to_p[RK_F5]       = KEY_MASK & XK_F5;   
    r_to_p[RK_F6]       = KEY_MASK & XK_F6;   
    r_to_p[RK_F7]       = KEY_MASK & XK_F7;   
    r_to_p[RK_F8]       = KEY_MASK & XK_F8;   
    r_to_p[RK_F9]       = KEY_MASK & XK_F9;   
    r_to_p[RK_F10]      = KEY_MASK & XK_F10;   
    r_to_p[RK_F11]      = KEY_MASK & XK_F11;  
    r_to_p[RK_F12]      = KEY_MASK & XK_F12;  

    r_to_p[RK_ESCAPE]   = KEY_MASK & XK_Escape;

    /* open connection to X server */
    disp = XOpenDisplay(NULL);
    if (disp == NULL)
    {
        RT_LOGE("Cannot open X display\n");
        return 1;
    }

    /* acquire depth */
    rt_si32 screen = DefaultScreen(disp);
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
        frame = (rt_ui32 *)ximage->data;
        x_row = ximage->bytes_per_line / 4;
    }

#if (RT_POINTER - RT_ADDRESS) != 0

    frame = RT_NULL;
    x_row = RT_X_RES;

#endif /* (RT_POINTER - RT_ADDRESS) */

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

/*
 * Get system time in milliseconds.
 */
rt_time get_time()
{
    timeval tm;
    gettimeofday(&tm, NULL);
    return (rt_time)(tm.tv_sec * 1000 + tm.tv_usec / 1000);
}

/*
 * Set current frame to screen.
 */
rt_void frame_to_screen(rt_ui32 *frame)
{
    if (depth == 16 && frame != RT_NULL)
    {
        rt_ui16 *idata = (rt_ui16 *)ximage->data;
        rt_si32 i = x_res * y_res;

        while (i-->0)
        {
            idata[i] = (frame[i] & 0x00F80000) >> 8 |
                       (frame[i] & 0x0000FC00) >> 5 |
                       (frame[i] & 0x000000F8) >> 3;
        }
    }

#if (RT_POINTER - RT_ADDRESS) != 0

    memcpy(ximage->data, frame, x_res * y_res * sizeof(rt_ui32));

#endif /* (RT_POINTER - RT_ADDRESS) */

    XShmPutImage(disp, win, gc, ximage, 0, 0, 0, 0, x_res, y_res, False);
    XSync(disp, False);
}

#if (RT_POINTER - RT_ADDRESS) != 0

#include <sys/mman.h>

#endif /* (RT_POINTER - RT_ADDRESS) */

/*
 * Allocate memory from system heap.
 */
rt_pntr sys_alloc(rt_size size)
{
#if (RT_POINTER - RT_ADDRESS) != 0

    rt_pntr ptr = mmap(NULL, size, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);

#else /* (RT_POINTER - RT_ADDRESS) */

    rt_pntr ptr = malloc(size);

#endif /* (RT_POINTER - RT_ADDRESS) */

#if (RT_POINTER - RT_ADDRESS) != 0 && RT_DEBUG >= 1

    RT_LOGI("ALLOC PTR = %016"RT_PR64"X, size = %ld\n", (rt_full)ptr, size);

#endif /* (RT_POINTER - RT_ADDRESS) && RT_DEBUG */

#if (RT_POINTER - RT_ADDRESS) != 0

    if ((rt_full)ptr > (0xFFFFFFFF - size))
    {
        throw rt_Exception("address exceeded allowed range in sys_alloc");
    }

#endif /* (RT_POINTER - RT_ADDRESS) */

    return ptr;
}

/*
 * Free memory from system heap.
 */
rt_void sys_free(rt_pntr ptr, rt_size size)
{
#if (RT_POINTER - RT_ADDRESS) != 0

    munmap(ptr, size);

#else /* (RT_POINTER - RT_ADDRESS) */

    free(ptr);

#endif /* (RT_POINTER - RT_ADDRESS) */

#if (RT_POINTER - RT_ADDRESS) != 0 && RT_DEBUG >= 1

    RT_LOGI("FREED PTR = %016"RT_PR64"X, size = %ld\n", (rt_full)ptr, size);

#endif /* (RT_POINTER - RT_ADDRESS) && RT_DEBUG */
}

/******************************************************************************/
/*****************************   MULTI-THREADING   ****************************/
/******************************************************************************/

/* platform-specific thread */
struct rt_THREAD
{
    rt_Scene           *scene;
    rt_si32            *cmd;
    rt_si32             index;
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

        rt_si32 cmd = thread->cmd[0];

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
    rt_si32             cmd;
    rt_si32             thnum;
    rt_THREAD          *thread;
    pthread_barrier_t   barr[2];
};

/*
 * Initialize platform-specific pool of "thnum" threads.
 */
rt_pntr init_threads(rt_si32 thnum, rt_Scene *scn)
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

    rt_si32 i, a;
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
rt_void term_threads(rt_pntr tdata, rt_si32 thnum)
{
    rt_si32 i;
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
rt_void update_scene(rt_pntr tdata, rt_si32 thnum, rt_si32 phase)
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
rt_void render_scene(rt_pntr tdata, rt_si32 thnum, rt_si32 phase)
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
 * Implementation of the event loop.
 */
rt_si32 main_loop()
{
    /* event loop */
    while (1)
    {
        rt_si32 ret, key;

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
