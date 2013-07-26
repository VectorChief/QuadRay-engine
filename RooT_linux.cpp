/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
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
        RT_LOGI("main(argc = %d)\n", argc);
    }

    for (i = 0; i < argc; i++)
    {
        RT_LOGI("argv[%d] = %s\n", i, argv[i]);
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
    rt_word win_b = 0, win_d = 0;

    XGetGeometry(disp, win, &win_root, &win_x, &win_y,
                (rt_word *)&x_res, (rt_word *)&y_res,
                &win_b, &win_d);
    /*
    RT_LOGI("XWindow W = %d\n", x_res);
    RT_LOGI("XWindow H = %d\n", y_res);
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
    XSync(disp, False);

    shmctl(shminfo.shmid, IPC_RMID, 0);

    gc = XCreateGC(disp, win, 0, &gc_values);

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
/********************************   RENDERING   *******************************/
/******************************************************************************/

rt_cell main_init()
{
    try
    {
        scene = new rt_Scene(&sc_root,
                            x_res, y_res, x_row, frame,
                            malloc, free);
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
static rt_long time = 0;
static rt_word fps = 0;
static rt_word cnt = 0;

/* virtual keys arrays */
static rt_byte x_keys[512];
static rt_char t_keys[512]; /* toggle keys */

#define KEY_MASK    0x01FF
#define X_KEYS(k)   x_keys[(k) & KEY_MASK]
#define T_KEYS(k)   t_keys[(k) & KEY_MASK]
#define R_KEYS(k)   T_KEYS(k) > 0 ? T_KEYS(k) - X_KEYS(k) : -T_KEYS(k)

rt_cell main_step()
{
    if (scene == RT_NULL)
    {
        return 0;
    }

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

    if (X_KEYS(XK_w))       scene->update(time, RT_CAMERA_MOVE_FORWARD);
    if (X_KEYS(XK_s))       scene->update(time, RT_CAMERA_MOVE_BACK);
    if (X_KEYS(XK_a))       scene->update(time, RT_CAMERA_MOVE_LEFT);
    if (X_KEYS(XK_d))       scene->update(time, RT_CAMERA_MOVE_RIGHT);

    if (X_KEYS(XK_Up))      scene->update(time, RT_CAMERA_ROTATE_DOWN);
    if (X_KEYS(XK_Down))    scene->update(time, RT_CAMERA_ROTATE_UP);
    if (X_KEYS(XK_Left))    scene->update(time, RT_CAMERA_ROTATE_LEFT);
    if (X_KEYS(XK_Right))   scene->update(time, RT_CAMERA_ROTATE_RIGHT);

    if (T_KEYS(XK_Escape))
    {
        return 0;
    }
    memset(t_keys, 0, sizeof(t_keys));

    scene->render(time);
    scene->render_fps(x_res - 10, 10, -1, 2, fps);

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

    delete scene;

    return 1;
}

rt_cell main_loop()
{
    rt_cell ret;

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
