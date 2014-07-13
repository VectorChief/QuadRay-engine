/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include "RooT.h"

/******************************************************************************/
/****************************   PLATFORM - WIN32   ****************************/
/******************************************************************************/

#include <malloc.h>
#include <windows.h>

static HINSTANCE   hInst;
static HWND        hWnd;
static HDC         hWndDC;

static HBITMAP     hFrm;
static HDC         hFrmDC;
static BITMAPINFO  DIBinfo = 
{
    sizeof(BITMAPINFOHEADER),           /* biSize */
   +x_res,                              /* biWidth */
   -y_res,                              /* biHeight */
    1,                                  /* biPlanes */
    32,                                 /* biBitCount */
    BI_RGB,                             /* biCompression */
    x_res * y_res * sizeof(rt_word),    /* biSizeImage */
    0,                                  /* biXPelsPerMeter */
    0,                                  /* biYPelsPerMeter */
    0,                                  /* biClrUsed */
    0                                   /* biClrImportant */
};

/******************************************************************************/
/**********************************   MAIN   **********************************/
/******************************************************************************/

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

/*
 * Program's main entry point.
 */
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    MSG msg;
    hInst = hInstance;

    TCHAR wnd_class[] = "RooT";

    WNDCLASSEX wcex;

    wcex.cbSize         = sizeof(WNDCLASSEX); 
    wcex.lpszClassName  = wnd_class;
    wcex.style          = CS_OWNDC;
    wcex.lpfnWndProc    = (WNDPROC)WndProc;
    wcex.hInstance      = hInst;
    wcex.hIcon          = NULL;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = NULL;
    wcex.lpszMenuName   = NULL;
    wcex.hIconSm        = NULL;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;

    if (!RegisterClassEx(&wcex))
    {
        RT_LOGE("Couldn't register class\n");
        return FALSE;
    }


    RECT wRect, cRect;

    hWnd = CreateWindow(wnd_class, title,
                WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,
                CW_USEDEFAULT, CW_USEDEFAULT, x_res, y_res,
                NULL, NULL, hInst, NULL);

    if (!hWnd)
    {
        RT_LOGE("Couldn't create window\n");
        return FALSE;
    }

    GetWindowRect(hWnd, &wRect);
    GetClientRect(hWnd, &cRect);
    MoveWindow(hWnd, 100, 50, 2 * x_res - cRect.right,
                              2 * y_res - cRect.bottom, FALSE);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
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
    HANDLE              pthr;
    HANDLE              pevent;
    HANDLE             *cevent;
};

/*
 * Worker thread's entry point.
 */
DWORD WINAPI worker_thread(rt_pntr p)
{
    rt_THREAD *thread = (rt_THREAD *)p;
    rt_cell i = 0;

    while (1)
    {
        WaitForSingleObject(thread->cevent[i], INFINITE);

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

        SetEvent(thread->pevent);
        i = 1 - i;
    }

    SetEvent(thread->pevent);
    return 1;
}

/* platform-specific pool
 * of "thnum" threads */
struct rt_THREAD_POOL
{
    rt_Scene           *scene;
    rt_cell             cmd;
    rt_cell             thnum;
    rt_THREAD          *thread;
    HANDLE             *pevent;
    HANDLE              cevent[2];
    rt_cell             cindex;
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

    DWORD  pam, sam;
    HANDLE process = GetCurrentProcess();
    GetProcessAffinityMask(process, &pam, &sam);

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
    tpool->pevent = (HANDLE *)malloc(sizeof(HANDLE) * thnum);

    if (tpool->thread == RT_NULL
    ||  tpool->pevent == RT_NULL)
    {
        throw rt_Exception("out of memory for thread data in init_threads");
    }

    tpool->cevent[0] = CreateEvent(NULL, TRUE, FALSE, NULL);
    tpool->cevent[1] = CreateEvent(NULL, TRUE, FALSE, NULL);
    tpool->cindex = 0;

    for (i = 0, a = 0; i < thnum; i++, a++)
    {
        rt_THREAD *thread = tpool->thread;

        thread[i].scene  = scn;
        thread[i].cmd    = &tpool->cmd;
        thread[i].index  = i;
        thread[i].pevent = CreateEvent(NULL, FALSE, FALSE, NULL);
        thread[i].cevent = tpool->cevent;
        tpool->pevent[i] = thread[i].pevent;

        thread[i].pthr   = CreateThread(NULL, 0, worker_thread,
                                        &thread[i], 0, NULL);

        while (((pam & (1 << a)) == 0))
        {
            a++;
            if (a == 32) a = 0;
        }
        SetThreadAffinityMask(thread[i].pthr, 1 << a);
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

    SetEvent(tpool->cevent[tpool->cindex]);
    WaitForMultipleObjects(tpool->thnum, tpool->pevent, TRUE, INFINITE);

    for (i = 0; i < tpool->thnum; i++)
    {
        rt_THREAD *thread = tpool->thread;

        CloseHandle(thread[i].pthr);
        CloseHandle(thread[i].pevent);
    }

    CloseHandle(tpool->cevent[0]);
    CloseHandle(tpool->cevent[1]);

    free(tpool->thread);
    free(tpool->pevent);
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
    SetEvent(tpool->cevent[tpool->cindex]);
    WaitForMultipleObjects(tpool->thnum, tpool->pevent, TRUE, INFINITE);
    ResetEvent(tpool->cevent[tpool->cindex]);
    tpool->cindex = 1 - tpool->cindex;
}

/*
 * Task platform-specific pool of "thnum" threads to render scene,
 * block until finished.
 */
rt_void render_scene(rt_pntr tdata, rt_cell thnum, rt_cell phase)
{
    rt_THREAD_POOL *tpool = (rt_THREAD_POOL *)tdata;

    tpool->cmd = 2 | ((phase & 0xFF) << 2);
    SetEvent(tpool->cevent[tpool->cindex]);
    WaitForMultipleObjects(tpool->thnum, tpool->pevent, TRUE, INFINITE);
    ResetEvent(tpool->cevent[tpool->cindex]);
    tpool->cindex = 1 - tpool->cindex;
}

/******************************************************************************/
/*******************************   EVENT-LOOP   *******************************/
/******************************************************************************/

/*
 * Initialize event loop.
 */
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
static LARGE_INTEGER tm;
static LARGE_INTEGER fr;

/* time counter variables */
static rt_time init_time = 0;
static rt_time last_time = 0;
static rt_time cur_time = 0;

/* frame counter variables */
static rt_real fps = 0.0f;
static rt_word cnt = 0;
static rt_word scr = 0;

/* virtual keys array */
#define KEY_MASK    0x00FF
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

    QueryPerformanceCounter(&tm);
    cur_time = (rt_time)(tm.QuadPart * 1000 / fr.QuadPart);

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
        if (H_KEYS('W'))        scene->update(cur_time, RT_CAMERA_MOVE_FORWARD);
        if (H_KEYS('S'))        scene->update(cur_time, RT_CAMERA_MOVE_BACK);
        if (H_KEYS('A'))        scene->update(cur_time, RT_CAMERA_MOVE_LEFT);
        if (H_KEYS('D'))        scene->update(cur_time, RT_CAMERA_MOVE_RIGHT);

        if (H_KEYS(VK_UP))      scene->update(cur_time, RT_CAMERA_ROTATE_DOWN);
        if (H_KEYS(VK_DOWN))    scene->update(cur_time, RT_CAMERA_ROTATE_UP);
        if (H_KEYS(VK_LEFT))    scene->update(cur_time, RT_CAMERA_ROTATE_LEFT);
        if (H_KEYS(VK_RIGHT))   scene->update(cur_time, RT_CAMERA_ROTATE_RIGHT);

        if (T_KEYS(VK_F1))      scene->print_state();
        if (T_KEYS(VK_F2))      fsaa = RT_FSAA_4X - fsaa;
        if (T_KEYS(VK_F3))      scene->next_cam();
        if (T_KEYS(VK_F4))      scene->save_frame(scr++);
        if (T_KEYS(VK_ESCAPE))
        {
            return 0;
        }
        memset(t_keys, 0, sizeof(t_keys));
        memset(r_keys, 0, sizeof(r_keys));

        scene->set_fsaa(fsaa);
        scene->render(cur_time);
        scene->render_fps(x_res - 10, 10, -1, 2, (rt_word)fps);
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

    SetDIBitsToDevice(hWndDC, 0, 0, x_res, y_res, 0, 0, 0, y_res,
                                    scene->get_frame(), &DIBinfo, DIB_RGB_COLORS);
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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    rt_cell ret, key;

    switch (message) 
    {
        case WM_CREATE:
        {
            /* RT_LOGI("Window = %p\n", hWnd); */

            hWndDC = GetDC(hWnd);

            if (hWndDC == NULL)
            {
                return -1;
            }

            hFrm = CreateDIBSection(NULL, &DIBinfo, DIB_RGB_COLORS,
                                            (rt_pntr *)&frame, NULL, 0); 

            if (hFrm == NULL || frame == NULL)
            {
                return -1;
            }

            hFrmDC = CreateCompatibleDC(hWndDC);

            if (hFrmDC == NULL)
            {
                return -1;
            }

            SelectObject(hFrmDC, hFrm);

            ret = main_init();

            if (ret == 0)
            {
                return -1;
            }

            QueryPerformanceFrequency(&fr);
        }
        break;

        case WM_KEYDOWN:
        {
            key = wParam;
            /* RT_LOGI("Key press   = %X\n", key); */

            key &= KEY_MASK;
            if (h_keys[key] == 0)
            {
                t_keys[key] = 1;
            }
            h_keys[key] = 1;
        }
        break;

        case WM_KEYUP:
        {
            key = wParam;
            /* RT_LOGI("Key release = %X\n", key); */

            key &= KEY_MASK;
            h_keys[key] = 0;
            r_keys[key] = 1;
        }
        break;

        case WM_MOUSEMOVE:
        break;

        case WM_PAINT:
        {
            ret = main_step();

            if (ret == 0)
            {
                DestroyWindow(hWnd);
            }
        }
        break;

        case WM_DESTROY:
        {
            ret = main_term();

            if (hFrmDC != NULL)
            {
                DeleteDC(hFrmDC);
                hFrmDC  = NULL;
            }

            if (hFrm   != NULL)
            {
                DeleteObject(hFrm);
                hFrm    = NULL;
            }

            if (hWndDC != NULL)
            {
                ReleaseDC(hWnd, hWndDC);
                hWndDC  = NULL;
            }

            PostQuitMessage(0);
        }
        break;

        default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
