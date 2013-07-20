/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include "RooT.h"

/******************************************************************************/
/**************************   PLATFORM - WIN32   ******************************/
/******************************************************************************/

rt_word    *data = RT_NULL;

#include <windows.h>

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
    x_res * y_res * sizeof(rt_cell),    /* biSizeImage */
    0,                                  /* biXPelsPerMeter */
    0,                                  /* biYPelsPerMeter */
    0,                                  /* biClrUsed */
    0                                   /* biClrImportant */
};

/******************************************************************************/
/********************************   MAIN   ************************************/
/******************************************************************************/

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

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
/******************************   RENDERING   *********************************/
/******************************************************************************/

rt_cell main_init(rt_word *data, rt_word w, rt_word h)
{
    return 1;
}

rt_cell main_step(rt_long time, rt_word fps)
{
    return 1;
}

rt_cell main_done()
{
    return 1;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    /* virtual keys array */
    static rt_cell v_keys[256];

    /* performance variables */
    static LARGE_INTEGER tm;
    static LARGE_INTEGER fr;

    /* time counter varibales */
    static rt_long init_time = 0;
    static rt_long last_time = 0;
    static rt_long time = 0;
    static rt_word fps = 0;
    static rt_word cnt = 0;

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
                                            (void **)&data, NULL, 0); 

            if (hFrm == NULL || data == NULL)
            {
                return -1;
            }

            hFrmDC = CreateCompatibleDC(hWndDC);

            if (hFrmDC == NULL)
            {
                return -1;
            }

            SelectObject(hFrmDC, hFrm);

            if (main_init(data, x_res, y_res) == 0)
            {
                return -1;
            }

            QueryPerformanceFrequency(&fr);
        }
        break;

        case WM_KEYDOWN:
        {
            if (wParam < 256)
            {
                v_keys[wParam] = 1;
            }

            if (wParam == VK_ESCAPE)
            {
                DestroyWindow(hWnd);
            }
        }
        break;

        case WM_KEYUP:
        {
            if (wParam < 256)
            {
                v_keys[wParam] = 0;
            }
        }
        break;

        case WM_MOUSEMOVE:
        break;

        case WM_PAINT:
        {
            /* acquire perf counter */
            QueryPerformanceCounter(&tm);
            time = (rt_long)(tm.QuadPart * 1000 / fr.QuadPart);

            if (init_time == 0)
            {
                init_time = time;
            }

            time = time - init_time;
            cnt++;

            if (time - last_time >= 500)
            {
                fps = cnt * 1000 / (time - last_time);
                RT_LOGI("FPS = %d\n", fps);
                cnt = 0;
                last_time = time;
            }

            main_step(time, fps);

            SetDIBitsToDevice(hWndDC, 0, 0, x_res, y_res, 0, 0, 0, y_res,
                                            data, &DIBinfo, DIB_RGB_COLORS);
        }
        break;

        case WM_DESTROY:
        {
            main_done();

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
