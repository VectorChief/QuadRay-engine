/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include "RooT.h"

/******************************************************************************/
/****************************   PLATFORM - WIN32   ****************************/
/******************************************************************************/

#include <malloc.h>
#include <windows.h>
#include <tchar.h>

HINSTANCE   hInst;
HWND        hWnd;
HDC         hWndDC;

HBITMAP     hFrm;
HDC         hFrmDC;
BITMAPINFO  DIBinfo = 
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

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    MSG msg;
    hInst = hInstance;

    TCHAR wnd_class[] = _T("RooT");

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

    hWnd = CreateWindow(wnd_class, _T(RT_TITLE),
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
/********************************   THREADING   *******************************/
/******************************************************************************/

struct rt_THREAD
{
    rt_cell    *cmd;
    rt_cell     index;
    rt_Scene   *scene;
    HANDLE      pevent;
    HANDLE     *cevent;
    HANDLE      pthr;
};

DWORD WINAPI worker_thread(void *p)
{
    rt_THREAD *thread = (rt_THREAD *)p;
    rt_cell i = 0;

    while (1)
    {
        WaitForSingleObject(thread->cevent[i], INFINITE);

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

        SetEvent(thread->pevent);
        i = 1 - i;
    }

    SetEvent(thread->pevent);
    return 1;
}

struct rt_THREAD_POOL
{
    rt_Scene   *scene;
    rt_THREAD  *thread;
    HANDLE     *edata;
    HANDLE      cevent[2];
    rt_cell     cindex;
    rt_cell     cmd;
    rt_cell     thnum;
};

rt_pntr init_threads(rt_cell thnum, rt_Scene *scn)
{
    DWORD  pam, sam;
    HANDLE process = GetCurrentProcess();
    GetProcessAffinityMask(process, &pam, &sam);

    rt_cell i, a;
    rt_THREAD_POOL *tpool = (rt_THREAD_POOL *)malloc(sizeof(rt_THREAD_POOL));

    tpool->scene = scn;
    tpool->thread = (rt_THREAD *)malloc(sizeof(rt_THREAD) * thnum);
    tpool->edata = (HANDLE *)malloc(sizeof(HANDLE) * thnum);

    tpool->cevent[0] = CreateEvent(NULL, TRUE, FALSE, NULL);
    tpool->cevent[1] = CreateEvent(NULL, TRUE, FALSE, NULL);

    tpool->cindex = 0;
    tpool->cmd = 0;
    tpool->thnum = thnum;

    for (i = 0, a = 0; i < thnum; i++, a++)
    {
        rt_THREAD *thread = tpool->thread;

        thread[i].cmd    = &tpool->cmd;
        thread[i].index  = i;
        thread[i].scene  = scn;
        thread[i].pevent = CreateEvent(NULL, FALSE, FALSE, NULL);
        thread[i].cevent = tpool->cevent;
        thread[i].pthr = CreateThread(NULL, 0, worker_thread,
                                &thread[i], 0, NULL);

        tpool->edata[i] = thread[i].pevent;

        while (((pam & (1 << a)) == 0))
        {
            a++;
            if (a == 32) a = 0;
        }
        SetThreadAffinityMask(thread[i].pthr, 1 << a);
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

    SetEvent(tpool->cevent[tpool->cindex]);
    WaitForMultipleObjects(tpool->thnum, tpool->edata, TRUE, INFINITE);

    for (i = 0; i < tpool->thnum; i++)
    {
        rt_THREAD *thread = tpool->thread;

        CloseHandle(thread[i].pthr);
        CloseHandle(thread[i].pevent);
    }

    CloseHandle(tpool->cevent[0]);
    CloseHandle(tpool->cevent[1]);

    free(tpool->thread);
    free(tpool->edata);
    free(tpool);
}

rt_void update_scene(rt_pntr tdata, rt_cell thnum, rt_cell phase)
{
    rt_THREAD_POOL *tpool = (rt_THREAD_POOL *)tdata;

    tpool->cmd = 1 | ((phase & 0xFF) << 2);
    SetEvent(tpool->cevent[tpool->cindex]);
    WaitForMultipleObjects(tpool->thnum, tpool->edata, TRUE, INFINITE);
    ResetEvent(tpool->cevent[tpool->cindex]);
    tpool->cindex = 1 - tpool->cindex;
}

rt_void render_scene(rt_pntr tdata, rt_cell thnum, rt_cell phase)
{
    rt_THREAD_POOL *tpool = (rt_THREAD_POOL *)tdata;

    tpool->cmd = 2 | ((phase & 0xFF) << 2);
    SetEvent(tpool->cevent[tpool->cindex]);
    WaitForMultipleObjects(tpool->thnum, tpool->edata, TRUE, INFINITE);
    ResetEvent(tpool->cevent[tpool->cindex]);
    tpool->cindex = 1 - tpool->cindex;
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
static LARGE_INTEGER tm;
static LARGE_INTEGER fr;

/* time counter varibales */
static rt_long init_time = 0;
static rt_long last_time = 0;
static rt_long cur_time = 0;
static rt_real fps = 0.0f;
static rt_word cnt = 0;

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

rt_cell main_step()
{
    if (scene == RT_NULL)
    {
        return 0;
    }

    QueryPerformanceCounter(&tm);
    cur_time = (rt_long)(tm.QuadPart * 1000 / fr.QuadPart);

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

    SetDIBitsToDevice(hWndDC, 0, 0, x_res, y_res, 0, 0, 0, y_res,
                                    scene->get_frame(), &DIBinfo, DIB_RGB_COLORS);
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
                                            (void **)&frame, NULL, 0); 

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
            ret = main_done();

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
