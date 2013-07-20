/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include "RooT.h"

/******************************************************************************/
/**************************   PLATFORM - LINUX   ******************************/
/******************************************************************************/

#include <string.h>
#include <sys/time.h>

rt_word    *data = RT_NULL;

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

Display    *disp;
Window      win;

#include <sys/shm.h>
#include <X11/extensions/XShm.h>

XShmSegmentInfo shminfo;
XImage     *ximage      = NULL;
GC          gc;
XGCValues   gc_values   = {0};

/******************************************************************************/
/********************************   MAIN   ************************************/
/******************************************************************************/

rt_cell main_init(rt_word *data, rt_word w, rt_word h);
rt_cell main_loop();
rt_cell main_done();

int main()
{
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
    XSync (disp, False);

    Window win_root;
    rt_cell win_x = 0, win_y = 0;
    rt_word win_b = 0, win_d = 0;

    XGetGeometry(disp, win, &win_root, &win_x, &win_y,
                (rt_word *)&x_res, (rt_word *)&y_res,
                &win_b, &win_d);

    RT_LOGI("XWindow W = %d\n", x_res);
    RT_LOGI("XWindow H = %d\n", y_res);
    RT_LOGI("XWindow B = %d\n", win_b);
    RT_LOGI("XWindow D = %d\n", win_d);

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

    shminfo.shmaddr = ximage->data = (char*)shmat(shminfo.shmid, 0, 0);
    data = (rt_word *)ximage->data;

    if (shminfo.shmaddr == (char *) -1)
    {
        RT_LOGE("shmat failed!\n");
        XDestroyImage(ximage);
        return 1;
    }

    shminfo.readOnly = False;
    XShmAttach(disp, &shminfo);
    XSync(disp, False);

    shmctl(shminfo.shmid, IPC_RMID, 0);

    gc = XCreateGC (disp, win, 0, &gc_values);

    main_init(data, x_res, y_res);
    main_loop();
    main_done();

    /* destroy image */
    XShmDetach(disp, &shminfo);
    XDestroyImage(ximage);
    shmdt(shminfo.shmaddr);
    XFreeGC (disp, gc);

    /* destroy window */
    XDestroyWindow(disp, win);

    /* close connection to Xserver */
    XCloseDisplay(disp);

    return 0;
}

/******************************************************************************/
/******************************   RENDERING   *********************************/
/******************************************************************************/

rt_cell main_init(rt_word *data, rt_word w, rt_word h)
{
    return 1;
}

/* virtual keys array */
static rt_byte x_keys[512];
static rt_char t_keys[512];

#define KEY_MASK    0x01FF
#define X_KEYS(k)   x_keys[(k) & KEY_MASK]
#define T_KEYS(k)   t_keys[(k) & KEY_MASK]
#define R_KEYS(k)   T_KEYS(k) > 0 ? T_KEYS(k) - X_KEYS(k) : -T_KEYS(k)

rt_cell main_step()
{
    /* performance variables */
    static struct timeval tm;

    /* time counter varibales */
    static rt_long init_time = 0;
    static rt_long last_time = 0;
    static rt_long time = 0;
    static rt_word fps = 0;
    static rt_word cnt = 0;

    gettimeofday(&tm, NULL);
    time = tm.tv_sec * 1000 + tm.tv_usec / 1000;

    if (init_time == 0)
    {
        init_time = time;
    }

    time = time - init_time;
    cnt++;

    if (time - last_time >= 500)
    {
        fps = cnt * 1000 / (time - last_time);
        RT_LOGI("FPS = %.1f\n", (rt_real)(cnt * 1000) / (time - last_time));
        cnt = 0;
        last_time = time;
    }

    if (T_KEYS(XK_Escape))
    {
        return 0;
    }
    memset(t_keys, 0, sizeof(t_keys));

    XShmPutImage(disp, win, gc, ximage, 0, 0, 0, 0, x_res, y_res, False);
    XSync(disp, False);

    return 1;
}

rt_cell main_done()
{
    return 1;
}

rt_cell main_loop()
{
    /* event loop */
    while (1)
    {
        XEvent event;
        rt_word key;

        while (XPending(disp))
        {
            XNextEvent(disp, &event);

            /* handle key press */
            if (event.type == KeyPress)
            {
                key = XLookupKeysym((XKeyEvent*)&event, 0) & KEY_MASK;
                /* RT_LOGI("Key press   = %X\n", key); */

                if (key < 512)
                {
                    x_keys[key] = 1;
                    t_keys[key]++;
                }
            }

            /* handle key release */
            if (event.type == KeyRelease)
            {
                key = XLookupKeysym((XKeyEvent*)&event, 0) & KEY_MASK;
                /* RT_LOGI("Key release = %X\n", key); */

                if (key < 512)
                {
                    x_keys[key] = 0;
                    if (t_keys[key] == 0)
                    {
                        t_keys[key]--;
                    }
                }
            }
        }

        if (!main_step())
        {
            break;
        }
    }

    return 0;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
