/************************************
*
*   MainWnd.cpp
*   
*
*   By Patrick Wyatt - 5/17/2010
*
***/


#include "stdafx.h"
#pragma hdrstop


/************************************
*
*   Private
*
***/


struct MainWndParam {
    HANDLE  shutdownEvt;
    wchar   wndCaption[64];
};


static HANDLE   s_msgThread;
static HANDLE   s_shutdownEvt;


//===================================
static LRESULT CALLBACK MainWndProc (
    HWND    wnd,
    UINT    msg,
    WPARAM  wp,
    LPARAM  lp
) {
    switch (msg) {
        case WM_CLOSE:
            DestroyWindow(wnd);
        break;

        case WM_DESTROY:
            ServiceSignalStop();
        break;

        default:
        return DefWindowProc(wnd, msg, wp, lp);
    }

    return 0;
}


//===================================
static void MainWndCreate (const wchar name[]) {
    // Register window class
    WNDCLASSW wc;
    ZERO(wc);
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = MainWndProc;
    wc.hInstance        = GetModuleHandle(NULL);
    wc.hbrBackground    = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszClassName    = name;
    if (!RegisterClassW(&wc)) {
        LOG_OS_LAST_ERROR(L"RegisterClassW");
        FatalError();
    }

    // Create window
    HWND wnd;
    if (NULL == (wnd = CreateWindowW(
        name,
        name,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
        (HWND) NULL,
        (HMENU) NULL,
        wc.hInstance,
        NULL
    ))) {
        LOG_OS_LAST_ERROR(L"CreateWindowW");
        FatalError();
    }

    ShowWindow(wnd, SW_SHOWNORMAL);
}


//===================================
static unsigned __stdcall MainWndThreadProc (void * p) {
    DebugSetThreadName("MainWnd");

    // Initialize window from parameter
    HANDLE shutdownEvt;
    {
        const MainWndParam & param  = * (MainWndParam *) p;
        shutdownEvt                 = param.shutdownEvt;
        MainWndCreate(param.wndCaption);
        MemFree(p);
        p = NULL;
    }

    for (;;) {
        // Wait for a message or shutdown-signal
        switch (MsgWaitForMultipleObjects(
            1,
            &shutdownEvt,
            false,
            INFINITE,
            QS_ALLINPUT
        )) {
            case WAIT_FAILED:
                LOG_OS_LAST_ERROR(L"MsgWaitForMultipleObjects");
            return 0;

            case WAIT_OBJECT_0:
                // shutdown event fired
            return 0;

            default:
            break;
        }

        // Process all messages in the queue
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}


/************************************
*
*   Exports
*
***/


//===================================
void MainWndInitialize (const wchar name[]) {
    // Allocate window parameter
    unsigned chars = StrChars(name);
    MainWndParam * param = (MainWndParam *) ALLOC(sizeof(*param));
    StrCopy(param->wndCaption, _countof(param->wndCaption), name);

    // Create an event to signal shutdown
    s_shutdownEvt = param->shutdownEvt = CreateEvent(
        (LPSECURITY_ATTRIBUTES) NULL,
        true,   // manual reset event
        false,  // initial state
        NULL    // name
    );
    if (!param->shutdownEvt) {
        LOG_OS_LAST_ERROR(L"CreateEvent");
        FatalError();
    }

    // Create event pump thread
    unsigned threadId;
    if (NULL == (s_msgThread = (HANDLE) _beginthreadex(
        (LPSECURITY_ATTRIBUTES) NULL,
        0,      // default stack size
        MainWndThreadProc,
        param,
        0,      // flags
        &threadId
    ))) {
        LOG_OS_LAST_ERROR(L"_beginthreadex");
        FatalError();
    }
}


//===================================
void MainWndDestroy () {
    // Signal the message thread to shut down
    if (s_shutdownEvt) {
        SetEvent(s_shutdownEvt);
        s_shutdownEvt = NULL;
    }

    // Wait for message thread
    if (s_msgThread) {
        WaitForSingleObject(s_msgThread, INFINITE);
        CloseHandle(s_msgThread);
        s_msgThread = NULL;
    }
}


//===================================
// MIT License
//
// Copyright (c) 2010 by Patrick Wyatt
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//===================================
