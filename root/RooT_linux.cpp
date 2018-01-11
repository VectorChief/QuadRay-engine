/******************************************************************************/
/* Copyright (c) 2013-2018 VectorChief (at github, bitbucket, sourceforge)    */
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

Display    *disp;
Window      win;
rt_si32     depth;

#ifndef RT_XSHM
#define RT_XSHM 1
#endif /* RT_XSHM */

#if RT_XSHM
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

XShmSegmentInfo shminfo;
#endif /* RT_XSHM */

rt_bool     xshm        = RT_FALSE;
XImage     *ximage      = NULL;
GC          gc;
XGCValues   gc_values   = {0};

#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/******************************************************************************/
/**********************************   MAIN   **********************************/
/******************************************************************************/

rt_si32 main_init();
rt_si32 main_loop();
rt_si32 main_term();

/*
 * Program's main entry point.
 */
rt_si32 main(rt_si32 argc, rt_char *argv[])
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

    /* init internal variables from command-line args */
    rt_si32 ret;
    ret = args_init(argc, argv);
    if (ret == 0)
    {
        return 1;
    }

    /* open connection to X server */
    disp = XOpenDisplay(NULL);
    if (disp == NULL)
    {
        RT_LOGE("Cannot open X display\n");
        return 1;
    }

    /* acquire screen depth */
    rt_si32 scr_id = DefaultScreen(disp);
    depth = DefaultDepth(disp, scr_id);

    if (w_size == 0)
    {
        /* acquire fullscreen dimensions */
        Screen *screen = ScreenOfDisplay(disp, scr_id);
        x_win = WidthOfScreen(screen);
        y_win = HeightOfScreen(screen);
    }
    else
    {
        /* inherit framebuffer's dimensions */
        x_win = x_res;
        y_win = y_res;
    }

    /* create simple window */
    win = XCreateSimpleWindow(disp, RootWindow(disp, scr_id),
                                    10, 10, x_win, y_win, 1, 0, 0);
    if ((rt_pntr)win == NULL)
    {
        RT_LOGE("Cannot create X window\n");
        XCloseDisplay(disp);
        return 1;
    }

    /* disable window resize (only a hint) */
    XSizeHints sh;
    sh.flags = PMinSize | PMaxSize;
    sh.min_width  = sh.max_width  = x_win;
    sh.min_height = sh.max_height = y_win;
    XSetWMNormalHints(disp, win, &sh);

    if (w_size == 0)
    {
        /* activate fullscreen */
        Atom atom  = XInternAtom(disp, "_NET_WM_STATE", False);
        Atom state = XInternAtom(disp, "_NET_WM_STATE_FULLSCREEN", False);
        XChangeProperty(disp, win, atom, XA_ATOM, 32,
                        PropModeReplace, (rt_byte *)&state, 1);

        RT_LOGI("Window-less mode on! (fullscreen)\n");

        /* create empty cursor from pixmap */
        static rt_char bitmap[8] = {0};
        Pixmap pixmap = XCreateBitmapFromData(disp,  win, bitmap, 8, 8);
        XColor color;   color.red = color.green = color.blue = 0;
        Cursor cursor = XCreatePixmapCursor(disp, pixmap, pixmap,
                                                  &color, &color, 0, 0);

        /* set empty cursor in fullscreen */
        XDefineCursor(disp, win, cursor);
        XFreeCursor(disp, cursor);
        XFreePixmap(disp, pixmap);
    }

    /* set title and events */
    XStoreName(disp, win, title);
    XSelectInput(disp, win, ExposureMask | KeyPressMask | KeyReleaseMask);
    /* map (show) window */
    XMapWindow(disp, win);
    XSync(disp, False);

    if (w_size == 0)
    {
        /* override framebuffer's dimensions in fullscreen mode */
        x_res = x_new != 0 ? x_new : x_win;
        y_res = y_new != 0 ? y_new : y_win;
        x_row = (x_res+RT_SIMD_WIDTH-1) & ~(RT_SIMD_WIDTH-1);
    }

#if RT_XSHM
    do
    {
        /* create image,
         * use preconfigured x_res, y_res for rendering,
         * window resizing in runtime is not supported for now */
        ximage = XShmCreateImage(disp,
                                 DefaultVisual(disp, scr_id),
                                 depth,
                                 ZPixmap, NULL, &shminfo,
                                 x_row, y_res);
        if (ximage == NULL)
        {
            RT_LOGE("Cannot create XShm image\n");
            RT_LOGE("defaulting to (slower) non-XShm fallback\n");
            break;
        }

        /* get shared memory */
        shminfo.shmid = shmget(IPC_PRIVATE,
                               ximage->bytes_per_line * ximage->height,
                               IPC_CREAT|0777);
        if (shminfo.shmid < 0)
        {
            RT_LOGE("shmget failed with size = %d bytes\n",
                                 ximage->bytes_per_line * ximage->height);
            RT_LOGE("defaulting to (slower) non-XShm fallback\n");
            XDestroyImage(ximage);
            break;
        }

        /* attach shared memory */
        shminfo.shmaddr = ximage->data = (rt_char *)shmat(shminfo.shmid, 0, 0);
        if (shminfo.shmaddr == (rt_char *)-1)
        {
            RT_LOGE("shmat failed\n");
            RT_LOGE("defaulting to (slower) non-XShm fallback\n");
            XDestroyImage(ximage);
            break;
        }

        shminfo.readOnly = False;
        XShmAttach(disp, &shminfo);
        shmctl(shminfo.shmid, IPC_RMID, 0);

        xshm = RT_TRUE;
    }
    while (0);
#endif /* RT_XSHM */

    if (xshm == RT_FALSE)
    {
        rt_si32 pixel = depth > 16 ? 32 : 16;
        /* only malloc fits XCreateImage as XDestroyImage calls free */
        rt_pntr f_ptr = malloc(x_row * y_res * pixel / 8);

        /* create image,
         * use preconfigured x_res, y_res for rendering,
         * window resizing in runtime is not supported for now */
        ximage = XCreateImage(disp,
                              DefaultVisual(disp, scr_id),
                              depth,
                              ZPixmap, 0, (rt_char *)f_ptr,
                              x_res, y_res, pixel, x_row * pixel / 8);
        if (ximage == NULL)
        {
            RT_LOGE("Cannot create X image\n");
            XDestroyWindow(disp, win);
            XCloseDisplay(disp);
            return 1;
        }
    }

    gc = XCreateGC(disp, win, 0, &gc_values);
    XSync(disp, False);

    /* use true-color target directly */
    if (depth > 16)
    {
        frame = (rt_ui32 *)ximage->data;
        x_row = ximage->bytes_per_line / 4;
    }

    /* init sys_alloc's mutex */
    pthread_mutex_init(&mutex, NULL);

    /* run main loop */
    ret = main_init();
    if (ret == 0)
    {
        return 1;
    }
    ret = main_loop();
    ret = main_term();

    /* destroy sys_alloc's mutex */
    pthread_mutex_destroy(&mutex);

    if (w_size == 0)
    {
        /* restore original cursor */
        XDefineCursor(disp, win, None);
    }

#if RT_XSHM
    if (xshm == RT_TRUE)
    {
        /* destroy image,
         * detach shared memory */
        XShmDetach(disp, &shminfo);
        XDestroyImage(ximage);
        shmdt(shminfo.shmaddr);
    }
#endif /* RT_XSHM */

    if (xshm == RT_FALSE)
    {
        /* destroy image */
        XDestroyImage(ximage);
    }

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


#if RT_POINTER == 64
#if RT_ADDRESS == 32

#define RT_ADDRESS_MIN      ((rt_byte *)0x0000000040000000)
#define RT_ADDRESS_MAX      ((rt_byte *)0x0000000080000000)

#else /* RT_ADDRESS == 64 */

#define RT_ADDRESS_MIN      ((rt_byte *)0x0000000140000000)
#define RT_ADDRESS_MAX      ((rt_byte *)0x0000080000000000)

#endif /* RT_ADDRESS */

static
rt_byte *s_ptr = RT_ADDRESS_MIN;

#endif /* RT_POINTER */


#if RT_POINTER == 64

#include <sys/mman.h>

#endif /* RT_POINTER */

/*
 * Allocate memory from system heap.
 */
rt_pntr sys_alloc(rt_size size)
{
    pthread_mutex_lock(&mutex);

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

#if RT_DEBUG >= 1

    RT_LOGI("ALLOC PTR = %016" PR_Z "X, size = %ld\n", (rt_full)ptr, size);

#endif /* RT_DEBUG */

    pthread_mutex_unlock(&mutex);

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
    pthread_mutex_lock(&mutex);

#if RT_POINTER == 64

    munmap(ptr, size);

#else /* RT_POINTER == 32 */

    free(ptr);

#endif /* RT_POINTER */

#if RT_DEBUG >= 1

    RT_LOGI("FREED PTR = %016" PR_Z "X, size = %ld\n", (rt_full)ptr, size);

#endif /* RT_DEBUG */

    pthread_mutex_unlock(&mutex);
}

/******************************************************************************/
/*****************************   MULTI-THREADING   ****************************/
/******************************************************************************/

/* platform-specific thread */
struct rt_THREAD
{
    rt_Platform        *pfm;
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

        if (thread->pfm == RT_NULL)
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
            rt_Scene *scene = thread->pfm->get_cur_scene();

            switch (cmd & 0x3)
            {
                case 1:
                scene->update_slice(thread->index, (cmd >> 2) & 0xFF);
                break;

                case 2:
                scene->render_slice(thread->index, (cmd >> 2) & 0xFF);
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
    rt_Platform        *pfm;
    rt_si32             cmd;
    rt_si32             thnum;
    rt_THREAD          *thread;
    pthread_barrier_t   barr[2];
};

/*
 * Initialize platform-specific pool of "thnum" threads.
 */
rt_pntr init_threads(rt_si32 thnum, rt_Platform *pfm)
{
    eout = 0; emax = thnum;
    estr = (rt_pstr *)malloc(sizeof(rt_pstr) * thnum);

    if (estr == RT_NULL)
    {
        throw rt_Exception("out of memory for estr in init_threads");
    }

    memset(estr, 0, sizeof(rt_pstr) * thnum);

#if RT_SETAFFINITY

    cpu_set_t cpuset_pr, cpuset_th;
    pthread_t pthr = pthread_self();
    pthread_getaffinity_np(pthr, sizeof(cpu_set_t), &cpuset_pr);

#endif /* RT_SETAFFINITY */

    rt_THREAD_POOL *tpool = (rt_THREAD_POOL *)malloc(sizeof(rt_THREAD_POOL));

    if (tpool == RT_NULL)
    {
        throw rt_Exception("out of memory for tpool in init_threads");
    }

    tpool->pfm = pfm;
    tpool->cmd = 0;
    tpool->thnum = thnum;
    tpool->thread = (rt_THREAD *)malloc(sizeof(rt_THREAD) * thnum);

    if (tpool->thread == RT_NULL)
    {
        throw rt_Exception("out of memory for thread data in init_threads");
    }

    pthread_barrier_init(&tpool->barr[0], NULL, thnum + 1);
    pthread_barrier_init(&tpool->barr[1], NULL, thnum + 1);

    rt_si32 i, a;

    for (i = 0, a = 0; i < thnum; i++, a++)
    {
        rt_THREAD *thread = tpool->thread;

        thread[i].pfm    = pfm;
        thread[i].cmd    = &tpool->cmd;
        thread[i].index  = i;
        thread[i].barr   = tpool->barr;

        pthread_create(&thread[i].pthr, NULL, worker_thread, &thread[i]);

#if RT_SETAFFINITY

        while (!CPU_ISSET(a, &cpuset_pr))
        {
            a++;
            if (a == CPU_SETSIZE) a = 0;
        }
        CPU_ZERO(&cpuset_th);
        CPU_SET(a, &cpuset_th);
        pthread_setaffinity_np(thread[i].pthr, sizeof(cpu_set_t), &cpuset_th);

#endif /* RT_SETAFFINITY */
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

        thread[i].pfm = RT_NULL;
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
 * Set current frame to screen.
 */
rt_void frame_to_screen(rt_ui32 *frame, rt_si32 x_row)
{
    if (frame == RT_NULL)
    {
        return;
    }

    if (depth == 16)
    {
        rt_si32 i, j;

        for (i = 0; i < y_res; i++)
        {
            rt_ui16 *idata = (rt_ui16 *)ximage->data +
                             i * (ximage->bytes_per_line / 2);

            for (j = 0; j < x_res; j++)
            {
                idata[j] = (frame[i * x_row + j] & 0x00F80000) >> 8 |
                           (frame[i * x_row + j] & 0x0000FC00) >> 5 |
                           (frame[i * x_row + j] & 0x000000F8) >> 3;
            }
        }
    }
    else
    if (frame != ::frame)
    {
        rt_si32 i;

        for (i = 0; i < y_res; i++)
        {
            rt_ui32 *idata = (rt_ui32 *)ximage->data +
                             i * (ximage->bytes_per_line / 4);

            memcpy(idata, frame + i * x_row, x_res * sizeof(rt_ui32));
        }
    }

#if RT_XSHM
    if (xshm == RT_TRUE)
    {
        /* put XShm image to the screen */
        XShmPutImage(disp, win, gc, ximage, 0, 0, 0, 0, x_res, y_res, False);
    }
#endif /* RT_XSHM */

    if (xshm == RT_FALSE)
    {
        /* put X image to the screen */
        XPutImage(disp, win, gc, ximage, 0, 0, 0, 0, x_res, y_res);
    }

    XSync(disp, False);
}

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
