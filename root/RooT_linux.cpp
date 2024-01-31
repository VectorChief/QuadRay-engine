/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
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

#ifdef __APPLE__

#undef  RT_XSHM /* XShm compiles on macOS with XQuartz, but fails at runtime */
#define RT_XSHM 0

#undef  RT_SETAFFINITY /* setting thread affinity is not present on macOS */
#define RT_SETAFFINITY 0

/*
 * Custom implementation of pthread barriers for macOS found at:
 * http://blog.albertarmea.com/post/47089939939/using-pthreadbarrier-on-mac-os-x
 * copied as is with minor format adjustments.
 */
#include <errno.h>

typedef int pthread_barrierattr_t;
typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int tripCount;
} pthread_barrier_t;

int pthread_barrier_init(pthread_barrier_t *barrier,
                         const pthread_barrierattr_t *attr, unsigned int count)
{
    if(count == 0)
    {
        errno = EINVAL;
        return -1;
    }
    if(pthread_mutex_init(&barrier->mutex, 0) < 0)
    {
        return -1;
    }
    if(pthread_cond_init(&barrier->cond, 0) < 0)
    {
        pthread_mutex_destroy(&barrier->mutex);
        return -1;
    }
    barrier->tripCount = count;
    barrier->count = 0;

    return 0;
}

int pthread_barrier_destroy(pthread_barrier_t *barrier)
{
    pthread_cond_destroy(&barrier->cond);
    pthread_mutex_destroy(&barrier->mutex);
    return 0;
}

int pthread_barrier_wait(pthread_barrier_t *barrier)
{
    pthread_mutex_lock(&barrier->mutex);
    ++(barrier->count);
    if(barrier->count >= barrier->tripCount)
    {
        barrier->count = 0;
        pthread_cond_broadcast(&barrier->cond);
        pthread_mutex_unlock(&barrier->mutex);
        return 1;
    }
    else
    {
        pthread_cond_wait(&barrier->cond, &(barrier->mutex));
        pthread_mutex_unlock(&barrier->mutex);
        return 0;
    }
}

#endif /* __APPLE__ */

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
    /* workaround for out of memory on older Macs with SIMD buffers enabled */
#ifdef __APPLE__
    thnum               = RT_THREADS_NUM / 2; /* limit threads to fix allocs */
#endif /* __APPLE__ */

    /* fill in platform's keymap */
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

    r_to_p[RK_UP]       = KEY_MASK & XK_Up;
    r_to_p[RK_DOWN]     = KEY_MASK & XK_Down;
    r_to_p[RK_LEFT]     = KEY_MASK & XK_Left;
    r_to_p[RK_RIGHT]    = KEY_MASK & XK_Right;

    r_to_p[RK_U]        = KEY_MASK & XK_u;
    r_to_p[RK_O]        = KEY_MASK & XK_o;

    r_to_p[RK_W]        = KEY_MASK & XK_w;
    r_to_p[RK_S]        = KEY_MASK & XK_s;
    r_to_p[RK_A]        = KEY_MASK & XK_a;
    r_to_p[RK_D]        = KEY_MASK & XK_d;

    r_to_p[RK_TAB]      = KEY_MASK & XK_Tab;
    r_to_p[RK_Q]        = KEY_MASK & XK_q;
    r_to_p[RK_E]        = KEY_MASK & XK_e;
    r_to_p[RK_R]        = KEY_MASK & XK_r;

    r_to_p[RK_I]        = KEY_MASK & XK_i;
    r_to_p[RK_L]        = KEY_MASK & XK_l;
    r_to_p[RK_P]        = KEY_MASK & XK_p;

    r_to_p[RK_T]        = KEY_MASK & XK_t;
    r_to_p[RK_Y]        = KEY_MASK & XK_y;

    r_to_p[RK_X]        = KEY_MASK & XK_x;
    r_to_p[RK_C]        = KEY_MASK & XK_c;

    r_to_p[RK_0]        = KEY_MASK & XK_0;
    r_to_p[RK_1]        = KEY_MASK & XK_1;
    r_to_p[RK_2]        = KEY_MASK & XK_2;
    r_to_p[RK_3]        = KEY_MASK & XK_3;
    r_to_p[RK_4]        = KEY_MASK & XK_4;
    r_to_p[RK_5]        = KEY_MASK & XK_5;
    r_to_p[RK_6]        = KEY_MASK & XK_6;
    r_to_p[RK_7]        = KEY_MASK & XK_7;
    r_to_p[RK_8]        = KEY_MASK & XK_8;
    r_to_p[RK_9]        = KEY_MASK & XK_9;

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
        RT_LOGE("Couldn't open X display\n");
        return 1;
    }

    /* acquire screen depth */
    rt_si32 scr_id = DefaultScreen(disp);
    depth = DefaultDepth(disp, scr_id);
    Screen *screen = ScreenOfDisplay(disp, scr_id);

    while (w_size > 0)
    {
        if (x_res > WidthOfScreen(screen)
        ||  y_res > HeightOfScreen(screen))
        {
            /*
            RT_LOGI("Window size exceeds screen, try -w 0 for fullscreen\n");
            return 1;
            */
            w_size--;
            x_res = (x_res / (w_size + 1)) * w_size;
            y_res = (y_res / (w_size + 1)) * w_size;
            x_row = (x_res+RT_SIMD_WIDTH-1) & ~(RT_SIMD_WIDTH-1);
            RT_LOGI("Window size exceeds screen, resizing: -w %d\n", w_size);
        }
        else
        {
            break;
        }
    };

    if (w_size == 0)
    {
        /* acquire fullscreen dimensions */
        x_win = WidthOfScreen(screen);
        y_win = HeightOfScreen(screen);

        /* override framebuffer's dimensions in fullscreen mode */
        x_res = x_new != 0 ? x_new : x_win;
        y_res = y_new != 0 ? y_new : y_win;
        x_row = (x_res+RT_SIMD_WIDTH-1) & ~(RT_SIMD_WIDTH-1);
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
        RT_LOGE("Couldn't create X window\n");
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

        /* RT_LOGI("Window-less mode on! (fullscreen)\n"); */

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
            RT_LOGE("Couldn't create XShm image\n");
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
            RT_LOGE("Couldn't create X image\n");
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


#if (RT_POINTER - RT_ADDRESS) != 0

#include <sys/mman.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON  /* workaround for macOS compilation */
#endif /* macOS still cannot allocate with mmap within 32-bit range */

#endif /* (RT_POINTER - RT_ADDRESS) */

/*
 * Allocate memory from system heap.
 */
rt_pntr sys_alloc(rt_size size)
{
    pthread_mutex_lock(&mutex);

#if (RT_POINTER - RT_ADDRESS) != 0

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

#else /* (RT_POINTER - RT_ADDRESS) */

    rt_pntr ptr = malloc(size);

#endif /* (RT_POINTER - RT_ADDRESS) */

#if RT_DEBUG >= 2

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

#if (RT_POINTER - RT_ADDRESS) != 0

    munmap(ptr, size);

#else /* (RT_POINTER - RT_ADDRESS) */

    free(ptr);

#endif /* (RT_POINTER - RT_ADDRESS) */

#if RT_DEBUG >= 2

    RT_LOGI("FREED PTR = %016" PR_Z "X, size = %ld\n", (rt_full)ptr, size);

#endif /* RT_DEBUG */

    pthread_mutex_unlock(&mutex);
}

/******************************************************************************/
/*****************************   MULTI-THREADING   ****************************/
/******************************************************************************/

struct rt_THREAD;

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

/* platform-specific thread */
struct rt_THREAD
{
    rt_THREAD_POOL     *tpool;
    rt_si32             index;
    pthread_t           pthr;
};

/*
 * Worker thread's entry point.
 */
rt_pntr worker_thread(rt_pntr p)
{
    rt_THREAD *thread = (rt_THREAD *)p;
    rt_si32 ti = thread->index;

    while (thread->tpool->cmd < 0) /* <- wait for barriers */
    {
        sched_yield();
    }

    while (1)
    {
        /* every worker-thread waits signal from main thread */
        pthread_barrier_wait(&thread->tpool->barr[0]);

        rt_Platform *pfm = thread->tpool->pfm;

        if (pfm == RT_NULL)
        {
            break;
        }

        rt_si32 cmd = thread->tpool->cmd;

        /* if one thread throws an exception,
         * other threads are still allowed to proceed
         * in the same run, but not in the next one */
        if (eout == 0)
        try
        {
            rt_Scene *scene = pfm->get_cur_scene();

            switch (cmd & 0x3)
            {
                case 1:
                scene->update_slice(ti, (cmd >> 2) & 0xFF);
                break;

                case 2:
                scene->render_slice(ti, (cmd >> 2) & 0xFF);
                break;

                default:
                break;
            };
        }
        catch (rt_Exception e)
        {
            estr[ti] = e.err;
            eout = 1;
        }

        /* every worker-thread signals to main thread when done */
        pthread_barrier_wait(&thread->tpool->barr[1]);
    }

    /* every worker-thread signals to main thread when done */
    pthread_barrier_wait(&thread->tpool->barr[1]);

    return RT_NULL;
}

/*
 * Initialize platform-specific pool of "thnum" threads (< 0 - no feedback).
 */
rt_pntr init_threads(rt_si32 thnum, rt_Platform *pfm)
{
    rt_bool feedback = thnum < 0 ? RT_FALSE : RT_TRUE;
    thnum = thnum < 0 ? -thnum : thnum;

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
    tpool->cmd = -1;
    tpool->thnum = thnum;
    tpool->thread = (rt_THREAD *)malloc(sizeof(rt_THREAD) * thnum);

    if (tpool->thread == RT_NULL)
    {
        throw rt_Exception("out of memory for thread data in init_threads");
    }

    rt_si32 i, a;

    for (i = 0, a = 0; i < thnum; i++, a++)
    {
#if RT_SETAFFINITY

        while (!CPU_ISSET(a, &cpuset_pr))
        {
            a++;
            if (a == CPU_SETSIZE)
            {
                if (feedback)
                {
                    thnum = i;
                    break;
                }
                else
                {
                    a = 0;
                }
            }
        }
        if (thnum == i)
        {
            break;
        }

#endif /* RT_SETAFFINITY */

        rt_THREAD *thread = tpool->thread;

        thread[i].tpool = tpool;
        thread[i].index = i;
        pthread_create(&thread[i].pthr, NULL, worker_thread, &thread[i]);

#if RT_SETAFFINITY

        CPU_ZERO(&cpuset_th);
        CPU_SET(a, &cpuset_th);
        pthread_setaffinity_np(thread[i].pthr, sizeof(cpu_set_t), &cpuset_th);

#endif /* RT_SETAFFINITY */
    }

    pthread_barrier_init(&tpool->barr[0], NULL, thnum + 1);
    pthread_barrier_init(&tpool->barr[1], NULL, thnum + 1);

    if (feedback)
    {
        pfm->set_thnum(thnum);
    }
    tpool->thnum = thnum;
    tpool->cmd = 0;

    return tpool;
}

/*
 * Terminate platform-specific pool of "thnum" threads.
 */
rt_void term_threads(rt_pntr tdata, rt_si32 thnum)
{
    rt_si32 i;
    rt_THREAD_POOL *tpool = (rt_THREAD_POOL *)tdata;

    /* signal all worker-threads to terminate */
    tpool->cmd = 0;
    tpool->pfm = RT_NULL;
    pthread_barrier_wait(&tpool->barr[0]);
    /* wait for all worker-threads to finish */
    pthread_barrier_wait(&tpool->barr[1]);

    for (i = 0; i < tpool->thnum; i++)
    {
        rt_THREAD *thread = tpool->thread;

        pthread_join(thread[i].pthr, NULL);
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

    /* signal all worker-threads to update scene */
    tpool->cmd = 1 | ((phase & 0xFF) << 2);
    pthread_barrier_wait(&tpool->barr[0]);
    /* wait for all worker-threads to finish */
    pthread_barrier_wait(&tpool->barr[1]);
}

/*
 * Task platform-specific pool of "thnum" threads to render scene,
 * block until finished.
 */
rt_void render_scene(rt_pntr tdata, rt_si32 thnum, rt_si32 phase)
{
    rt_THREAD_POOL *tpool = (rt_THREAD_POOL *)tdata;

    /* signal all worker-threads to render scene */
    tpool->cmd = 2 | ((phase & 0xFF) << 2);
    pthread_barrier_wait(&tpool->barr[0]);
    /* wait for all worker-threads to finish */
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
