/******************************************************************************/
/* Copyright (c) 2013-2018 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include "RooT.h"

/******************************************************************************/
/****************************   PLATFORM - WINXX   ****************************/
/******************************************************************************/

#include <windows.h>

HINSTANCE   hInst;
HWND        hWnd;
HDC         hWndDC;
DEVMODE     DevMode;

HBITMAP     hFrm;
HDC         hFrmDC;
BITMAPINFO  DIBinfo =
{
    sizeof(BITMAPINFOHEADER),           /* biSize */
   +x_row,                              /* biWidth */
   -y_res,                              /* biHeight */
    1,                                  /* biPlanes */
    32,                                 /* biBitCount */
    BI_RGB,                             /* biCompression */
    x_row * y_res * sizeof(rt_ui32),    /* biSizeImage */
    0,                                  /* biXPelsPerMeter */
    0,                                  /* biYPelsPerMeter */
    0,                                  /* biClrUsed */
    0                                   /* biClrImportant */
};

CRITICAL_SECTION critSec;

#if   (defined RT_WIN32)
/* thread-group size, maximum 32 threads per group (Win32 limitation) */
#define TG 32
#elif (defined RT_WIN64)
/* thread-group size, maximum 64 threads per group (Win64 limitation) */
#define TG 64
#endif /* defined RT_WIN64 */

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
    /* fill in platform's keymap */
    r_to_p[RK_W]        = KEY_MASK & 'W';
    r_to_p[RK_S]        = KEY_MASK & 'S';
    r_to_p[RK_A]        = KEY_MASK & 'A';
    r_to_p[RK_D]        = KEY_MASK & 'D';

    r_to_p[RK_UP]       = KEY_MASK & VK_UP;
    r_to_p[RK_DOWN]     = KEY_MASK & VK_DOWN;
    r_to_p[RK_LEFT]     = KEY_MASK & VK_LEFT;
    r_to_p[RK_RIGHT]    = KEY_MASK & VK_RIGHT;

    r_to_p[RK_F1]       = KEY_MASK & VK_F1;
    r_to_p[RK_F2]       = KEY_MASK & VK_F2;
    r_to_p[RK_F3]       = KEY_MASK & VK_F3;
    r_to_p[RK_F4]       = KEY_MASK & VK_F4;
    r_to_p[RK_F5]       = KEY_MASK & VK_F5;
    r_to_p[RK_F6]       = KEY_MASK & VK_F6;
    r_to_p[RK_F7]       = KEY_MASK & VK_F7;
    r_to_p[RK_F8]       = KEY_MASK & VK_F8;
    r_to_p[RK_F9]       = KEY_MASK & VK_F9;
    r_to_p[RK_F10]      = KEY_MASK & VK_F10;
    r_to_p[RK_F11]      = KEY_MASK & VK_F11;
    r_to_p[RK_F12]      = KEY_MASK & VK_F12;

    r_to_p[RK_ESCAPE]   = KEY_MASK & VK_ESCAPE;

    /* init internal variables from command-line args */
    rt_si32 ret;

    ret = args_init(__argc, __argv);

    if (ret == 0)
    {
        return FALSE;
    }

    /* register window class */
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

    /* acquire screen settings */
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &DevMode);

    if (w_size == 0)
    {
        /* acquire fullscreen dimensions */
        x_win = DevMode.dmPelsWidth;
        y_win = DevMode.dmPelsHeight;

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

    /* init framebuffer's dimensions */
    DIBinfo.bmiHeader.biWidth     = +x_row;
    DIBinfo.bmiHeader.biHeight    = -y_res;
    DIBinfo.bmiHeader.biSizeImage = (x_row * y_res * sizeof(rt_ui32));

    /* create window */
    if (w_size == 0)
    {
        /* create fullscreen window */
        hWnd = CreateWindow(wnd_class, title,
                    WS_POPUP,
                    CW_USEDEFAULT, CW_USEDEFAULT, x_win, y_win,
                    NULL, NULL, hInst, NULL);

        if (!hWnd)
        {
            RT_LOGE("Couldn't create fullscreen window\n");
            return FALSE;
        }
    }
    else
    {
        /* create regular window */
        hWnd = CreateWindow(wnd_class, title,
                    WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,
                    CW_USEDEFAULT, CW_USEDEFAULT, x_win, y_win,
                    NULL, NULL, hInst, NULL);

        if (!hWnd)
        {
            RT_LOGE("Couldn't create regular window\n");
            return FALSE;
        }

        RECT cRect;

        /* move and resize window */
        GetClientRect(hWnd, &cRect);
        MoveWindow(hWnd, (DevMode.dmPelsWidth  - 2 * x_win + cRect.right)  / 2,
                         (DevMode.dmPelsHeight - 2 * y_win + cRect.bottom) / 2,
                         2 * x_win - cRect.right,
                         2 * y_win - cRect.bottom, FALSE);
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    if (w_size == 0)
    {
        POINT point;

        /* hide cursor */
        ShowCursor(FALSE);
        GetCursorPos(&point);
        SetCursorPos(point.x, point.y);
    }

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

/*
 * Get system time in milliseconds.
 */
rt_time get_time()
{
    LARGE_INTEGER fr;
    QueryPerformanceFrequency(&fr);
    LARGE_INTEGER tm;
    QueryPerformanceCounter(&tm);
    return (rt_time)(tm.QuadPart * 1000 / fr.QuadPart);
}


#if RT_POINTER == 64
#if RT_ADDRESS == 32

#define RT_ADDRESS_MIN      ((rt_byte *)0x0000000040000000)
#define RT_ADDRESS_MAX      ((rt_byte *)0x0000000080000000)

#else /* RT_ADDRESS == 64 */

#define RT_ADDRESS_MIN      ((rt_byte *)0x0000000140000000)
#define RT_ADDRESS_MAX      ((rt_byte *)0x0000080000000000)

#endif /* RT_ADDRESS */

rt_byte *s_ptr = RT_ADDRESS_MIN;

#endif /* RT_POINTER */


DWORD s_step = 0;

SYSTEM_INFO s_sys = {0};

/*
 * Allocate memory from system heap.
 */
rt_pntr sys_alloc(rt_size size)
{
    EnterCriticalSection(&critSec);

#if RT_POINTER == 64

    /* loop around RT_ADDRESS_MAX boundary */
    if (s_ptr >= RT_ADDRESS_MAX - size)
    {
        s_ptr  = RT_ADDRESS_MIN;
    }

    if (s_step == 0)
    {
        GetSystemInfo(&s_sys);
        s_step = s_sys.dwAllocationGranularity;
    }

    rt_pntr ptr = VirtualAlloc(s_ptr, size, MEM_COMMIT | MEM_RESERVE,
                  PAGE_READWRITE);

    /* advance with allocation granularity */
    s_ptr = (rt_byte *)ptr + ((size + s_step - 1) / s_step) * s_step;

#else /* RT_POINTER == 32 */

    rt_pntr ptr = malloc(size);

#endif /* RT_POINTER */

#if RT_DEBUG >= 2

    RT_LOGI("ALLOC PTR = %016" PR_Z "X, size = %ld\n", (rt_full)ptr, size);

#endif /* RT_DEBUG */

    LeaveCriticalSection(&critSec);

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
    EnterCriticalSection(&critSec);

#if RT_POINTER == 64

    VirtualFree(ptr, 0, MEM_RELEASE);

#else /* RT_POINTER == 32 */

    free(ptr);

#endif /* RT_POINTER */

#if RT_DEBUG >= 2

    RT_LOGI("FREED PTR = %016" PR_Z "X, size = %ld\n", (rt_full)ptr, size);

#endif /* RT_DEBUG */

    LeaveCriticalSection(&critSec);
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
    HANDLE             *pevent; /* per-thr events */
    rt_si32             windex;
    HANDLE              wevent[2]; /* wrkr-events */
    HANDLE              cevent[TG]; /* ctl-events */
};

/* platform-specific thread */
struct rt_THREAD
{
    rt_THREAD_POOL     *tpool;
    rt_si32             index;
    HANDLE              pthr;
};

/*
 * Worker thread's entry point.
 */
DWORD WINAPI worker_thread(rt_pntr p)
{
    rt_THREAD *thread = (rt_THREAD *)p;
    rt_si32 wi = 0, ti = thread->index;

    while (1)
    {
        /* every worker-thread waits on current worker-event */
        WaitForSingleObject(thread->tpool->wevent[wi], INFINITE);

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

        /* swap worker-event for all worker-threads to wait on */
        wi = 1 - wi;

        /* every worker-thread signals per-thread event when done */
        SetEvent(thread->tpool->pevent[ti]);

        /* pick one worker-thread in a group as a control-thread,
         * which waits for other worker-threads in that group and
         * signals its respective control-event for the main thread */
        if ((ti % TG) == 0)
        {
            WaitForMultipleObjects(RT_MIN(TG, thread->tpool->thnum - ti),
                                   thread->tpool->pevent + (ti / TG) * TG,
                                   TRUE, INFINITE);

            SetEvent(thread->tpool->cevent[ti / TG]);
        }
    }

    /* every worker-thread signals per-thread event when done */
    SetEvent(thread->tpool->pevent[ti]);

    /* pick one worker-thread in a group as a control-thread,
     * which waits for other worker-threads in that group and
     * signals its respective control-event for the main thread */
    if ((ti % TG) == 0)
    {
        WaitForMultipleObjects(RT_MIN(TG, thread->tpool->thnum - ti),
                               thread->tpool->pevent + (ti / TG) * TG,
                               TRUE, INFINITE);

        SetEvent(thread->tpool->cevent[ti / TG]);
    }

    return 1;
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
#if (_WIN32_WINNT < 0x0601) /* Windows XP, Vista */

    rt_uptr pam, sam;
    HANDLE process = GetCurrentProcess();
    GetProcessAffinityMask(process, &pam, &sam);
#if RT_DEBUG >= 1
    RT_LOGI("ProcessAffinityMask: %016" PR_Z "X\n", (rt_full)pam);
#endif /* RT_DEBUG */

#else /* Windows 7 or newer */

    USHORT j, gcnt = TG, garr[TG] = {0};
    HANDLE process = GetCurrentProcess();
    GetProcessGroupAffinity(process, &gcnt, garr);
#if RT_DEBUG >= 1
    RT_LOGI("InitProcessGroupAffinity: %d - {", gcnt);
    for (j = 0; j < gcnt; j++)
    {
        if (j > 0)
        {
            RT_LOGI(",");
        }
        RT_LOGI("%d", garr[j]);
    }
    RT_LOGI("}\n");
#endif /* RT_DEBUG */

#endif /* Windows 7 or newer */
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
    tpool->pevent = (HANDLE *)malloc(sizeof(HANDLE) * thnum);

    if (tpool->thread == RT_NULL
    ||  tpool->pevent == RT_NULL)
    {
        throw rt_Exception("out of memory for thread data in init_threads");
    }

    tpool->windex = 0;
    tpool->wevent[0] = CreateEvent(NULL, TRUE, FALSE, NULL);
    tpool->wevent[1] = CreateEvent(NULL, TRUE, FALSE, NULL);

    rt_si32 i, k = 0;
    rt_si32 a = k, g = 0;

    rt_THREAD *thread = tpool->thread;

    /* create thread 0 outside of the loop
     * for multi-group affinity probing */
    thread[0].tpool  = tpool;
    thread[0].index  = 0;
    thread[0].pthr   = CreateThread(NULL, 0, worker_thread,
                                                   &thread[0], 0, NULL);
    for (i = 0; i < thnum; i++)
    {
#if RT_SETAFFINITY
#if (_WIN32_WINNT < 0x0601) /* Windows XP, Vista */

        if (a < TG
        && (pam & (rt_uptr)(ULL(1) << a)) != 0)
        {
            g = a;
            a += 2; /* <- allocate to physical cores first */
        }
        else
        {
            k = 1 - k; /* <- allocate to logical cores second */
            a = k;
            if ((pam & (rt_uptr)(ULL(1) << a)) != 0)
            {
                g = a;
                a += 2;
            }
            else
            {
                k = 0;
                a = k;
                g = a;
                a += 2;
            }
            if (feedback && k == 0)
            {
                thnum = i;
                break;
            }
        }

#else /* Windows 7 or newer */

        GROUP_AFFINITY ga;
        ga.Mask = ULL(1) << a;
        ga.Group = g;
        ga.Reserved[0] = ga.Reserved[1] = ga.Reserved[2] = 0;
        if (a < TG
        &&  SetThreadGroupAffinity(thread[0].pthr, &ga, NULL) != FALSE)
        {
            ga.Mask = ULL(1) << a;
            ga.Group = g;
            a += 2; /* <- allocate to physical cores first */
        }
        else
        {
            a = k;
            g++; /* <- move onto the next thread-group */
            ga.Mask = ULL(1) << a;
            ga.Group = g;
            ga.Reserved[0] = ga.Reserved[1] = ga.Reserved[2] = 0;
            if (SetThreadGroupAffinity(thread[0].pthr, &ga, NULL) != FALSE)
            {
                ga.Mask = ULL(1) << a;
                ga.Group = g;
                a += 2;
            }
            else
            {
                k = 1 - k; /* <- allocate to logical cores second */
                a = k;
                g = 0;
                ga.Mask = ULL(1) << a;
                ga.Group = g;
                ga.Reserved[0] = ga.Reserved[1] = ga.Reserved[2] = 0;
                if (SetThreadGroupAffinity(thread[0].pthr, &ga, NULL) != FALSE)
                {
                    ga.Mask = ULL(1) << a;
                    ga.Group = g;
                    a += 2;
                }
                else
                {
                    k = 0;
                    a = k;
                    g = 0;
                    ga.Mask = ULL(1) << a;
                    ga.Group = g;
                    a += 2;
                }
                if (feedback && k == 0)
                {
                    thnum = i;
                    break;
                }
            }
        }

#endif /* Windows 7 or newer */
#endif /* RT_SETAFFINITY */

        /* thread 0 created outside of the loop
         * for multi-group affinity probing */
        if (i > 0)
        {
            thread[i].tpool  = tpool;
            thread[i].index  = i;
            thread[i].pthr   = CreateThread(NULL, 0, worker_thread,
                                                           &thread[i], 0, NULL);
        }

        tpool->pevent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);

        if ((i % TG) == 0)
        {
            tpool->cevent[i / TG] = CreateEvent(NULL, FALSE, FALSE, NULL);
        }

#if RT_SETAFFINITY
#if (_WIN32_WINNT < 0x0601) /* Windows XP, Vista */

        SetThreadAffinityMask(thread[i].pthr, (rt_uptr)(ULL(1) << g));
#if RT_DEBUG >= 2
        RT_LOGI("ThreadAffinityMask: %016" PR_Z "X\n",
                                              (rt_full)(ULL(1) << g));
#endif /* RT_DEBUG */

#else /* Windows 7 or newer */

        ga.Reserved[0] = ga.Reserved[1] = ga.Reserved[2] = 0;
        SetThreadGroupAffinity(thread[i].pthr, &ga, NULL);
#if RT_DEBUG >= 2
        RT_LOGI("ThreadGroupAffinity: Mask = %016" PR_Z "X, Group = %d\n",
                                              (rt_full)ga.Mask, ga.Group);
#endif /* RT_DEBUG */

#endif /* Windows 7 or newer */
#endif /* RT_SETAFFINITY */
    }

#if RT_SETAFFINITY
#if (_WIN32_WINNT >= 0x0601) /* Windows 7 or newer */

    /* restore affinity of thread 0 outside of the loop
     * after multi-group affinity probing */
    GROUP_AFFINITY ga;
    ga.Mask = ULL(1) << 0;
    ga.Group = 0;
    ga.Reserved[0] = ga.Reserved[1] = ga.Reserved[2] = 0;
    SetThreadGroupAffinity(thread[0].pthr, &ga, NULL);

    /* query group affinity for the whole process */
    gcnt = TG; memset(garr, 0, sizeof(USHORT)*TG);
    GetProcessGroupAffinity(process, &gcnt, garr);
#if RT_DEBUG >= 1
    RT_LOGI("DoneProcessGroupAffinity: %d - {", gcnt);
    for (j = 0; j < gcnt; j++)
    {
        if (j > 0)
        {
            RT_LOGI(",");
        }
        RT_LOGI("%d", garr[j]);
    }
    RT_LOGI("}\n");
#endif /* RT_DEBUG */

#endif /* Windows 7 or newer */
#endif /* RT_SETAFFINITY */

    if (feedback)
    {
        pfm->set_thnum(thnum);
    }
    tpool->thnum = thnum;

    return tpool;
}

/*
 * Terminate platform-specific pool of "thnum" threads.
 */
rt_void term_threads(rt_pntr tdata, rt_si32 thnum)
{
    rt_si32 i;
    rt_THREAD_POOL *tpool = (rt_THREAD_POOL *)tdata;

    /* signal worker-event for all worker-threads to terminate */
    tpool->cmd = 0;
    tpool->pfm = RT_NULL;
    SetEvent(tpool->wevent[tpool->windex]);
    /* wait for control-threads to signal control-events for their groups */
    WaitForMultipleObjects((thnum + TG-1) / TG, tpool->cevent, TRUE, INFINITE);

    CloseHandle(tpool->wevent[0]);
    CloseHandle(tpool->wevent[1]);

    for (i = 0; i < tpool->thnum; i++)
    {
        CloseHandle(tpool->thread[i].pthr);
        CloseHandle(tpool->pevent[i]);

        if ((i % TG) == 0)
        {
            CloseHandle(tpool->cevent[i / TG]);
        }
    }

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
rt_void update_scene(rt_pntr tdata, rt_si32 thnum, rt_si32 phase)
{
    rt_THREAD_POOL *tpool = (rt_THREAD_POOL *)tdata;

    /* signal worker-event for all worker-threads to update scene */
    tpool->cmd = 1 | ((phase & 0xFF) << 2);
    SetEvent(tpool->wevent[tpool->windex]);
    /* wait for control-threads to signal control-events for their groups */
    WaitForMultipleObjects((thnum + TG-1) / TG, tpool->cevent, TRUE, INFINITE);
    /* manually reset current worker-event */
    ResetEvent(tpool->wevent[tpool->windex]);
    /* swap worker-event for the main thread to signal */
    tpool->windex = 1 - tpool->windex;
}

/*
 * Task platform-specific pool of "thnum" threads to render scene,
 * block until finished.
 */
rt_void render_scene(rt_pntr tdata, rt_si32 thnum, rt_si32 phase)
{
    rt_THREAD_POOL *tpool = (rt_THREAD_POOL *)tdata;

    /* signal worker-event for all worker-threads to render scene */
    tpool->cmd = 2 | ((phase & 0xFF) << 2);
    SetEvent(tpool->wevent[tpool->windex]);
    /* wait for control-threads to signal control-events for their groups */
    WaitForMultipleObjects((thnum + TG-1) / TG, tpool->cevent, TRUE, INFINITE);
    /* manually reset current worker-event */
    ResetEvent(tpool->wevent[tpool->windex]);
    /* swap worker-event for the main thread to signal */
    tpool->windex = 1 - tpool->windex;
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

    if (frame != ::frame)
    {
        rt_si32 i;

        for (i = 0; i < y_res; i++)
        {
            rt_ui32 *idata = ::frame + i * ::x_row;

            memcpy(idata, frame + i * x_row, x_res * sizeof(rt_ui32));
        }
    }

    SetDIBitsToDevice(hWndDC, 0, 0, x_res, y_res, 0, 0, 0, y_res,
                                  ::frame, &DIBinfo, DIB_RGB_COLORS);
}

/*
 * Implementation of the event loop.
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    rt_si32 ret, key;

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

            /* init sys_alloc's mutex */
            InitializeCriticalSection(&critSec);

            ret = main_init();

            if (ret == 0)
            {
                return -1;
            }
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

            /* destroy sys_alloc's mutex */
            DeleteCriticalSection(&critSec);

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
