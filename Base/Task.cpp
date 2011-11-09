/******************************************************************************
*
*   Task.cpp
*   
*
*   By Patrick Wyatt - 5/19/2010
*
***/


#include "stdafx.h"
#pragma hdrstop


/******************************************************************************
*
*   Private
*
***/


static HANDLE   s_completionPort;
static long     s_taskThreads;


//=============================================================================
static unsigned __stdcall TaskThreadProc (void *) {
    DebugSetThreadName("Task");

    for (;;) {
        // Get the next task completion
        DWORD bytes;
        DWORD key;
        OVERLAPPED * olap;
        if (!GetQueuedCompletionStatus(
            s_completionPort,
            &bytes,
            &key,
            &olap,
            INFINITE
        )) {
            LOG_OS_LAST_ERROR(L"GetQueuedCompletionStatus");
            continue;
        }

        // If the task is NULL this is a thread-quit notification
        if (!key)
            break;

        // Dispatch event
        CTask * task = (CTask *) key;
        task->TaskComplete(bytes, olap);
    }

    InterlockedDecrement(&s_taskThreads);
    return 0;
}


/******************************************************************************
*
*   Exports
*
***/

//=============================================================================
void TaskInitialize () {
    if (NULL == (s_completionPort = CreateIoCompletionPort(
        INVALID_HANDLE_VALUE,
        NULL,
        NULL,
        0
    ))) {
        LOG_OS_LAST_ERROR(L"CreateIoCompletionPort");
        FatalError();
    }

    HANDLE thread;
    unsigned threadId;
    if (NULL == (thread = (HANDLE) _beginthreadex(
        (LPSECURITY_ATTRIBUTES) NULL,
        0,      // default stack size
        TaskThreadProc,
        NULL,
        0,      // flags
        &threadId
    ))) {
        LOG_OS_LAST_ERROR(L"_beginthreadex");
        FatalError();
    }
    CloseHandle(thread);    // TODO: Keep in an array and use instead of sleeping on s_taskThreads on exit
    InterlockedIncrement(&s_taskThreads);
}

//=============================================================================
void TaskDestroy () {
    while (volatile long count = s_taskThreads) {
        PostQueuedCompletionStatus(s_completionPort, 0, 0, 0);
        while (count == s_taskThreads)
            Sleep(1);
    }

    if (s_completionPort) {
        CloseHandle(s_completionPort);
        s_completionPort = NULL;
    }
}

//=============================================================================
void TaskRegisterHandle (
    CTask * task,
    HANDLE  handle
) {
    ASSERT(task);
    if (task && !CreateIoCompletionPort(handle, s_completionPort, (DWORD) task, 0)) {
        LOG_OS_LAST_ERROR(L"CreateIoCompletionPort");
        FatalError();
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
