/******************************************************************************
*
*   Thread.cpp
*   
*
*   By Patrick Wyatt
*
***/


#include "stdafx.h"


/******************************************************************************
*
*   Private
*
***/

struct Thread {
    LIST_LINK(Thread)   m_link;
    unsigned            m_id;
    HANDLE              m_handle;
    unsigned            m_lastTimeMs;
    char *              m_name;

    Thread (const char name[])
        :m_id(0)
        ,m_handle(NULL)
        ,m_lastTimeMs(GetTickCount())
        ,m_name(StrDupAnsi(name))
    {}

    ~Thread() {
        MemFree(m_name);
    }
};

// Check all threads every minute to ensure they haven't gotten "stuck"
static const unsigned DEADLOCK_CHECK_FREQUENCY_MS = 60 * 1000;

static HANDLE       s_deadlockThread;
static HANDLE       s_deadlockEvent;
static CCritSect    s_critsect;
static LIST_DECLARE(Thread, m_link) s_threads;


//=============================================================================
static void __cdecl Out (const char fmt[], ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

//=============================================================================
static void LogThread_CS (const Thread & t) {
    // TODO: this function is a little... thin. Replace with Matt Pietrek's code:
    // http://www.microsoft.com/msj/0497/hood/hood0497.aspx
    Out("Thread: %u [%s]\n", t.m_id, t.m_name);

    CONTEXT ctx;
    ctx.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
    if (!GetThreadContext(t.m_handle, &ctx)) {
        Out("  ERR: no thread context (%u)\n\n", GetLastError());
    }

    Out("  EIP: %0x\n\n", ctx.Eip);
}

//=============================================================================
static void LogThreads_CS () {
    unsigned threadId = GetCurrentThreadId();
    for (const Thread * t = s_threads.Head(); t; t = t->m_link.Next()) {
        // Suspending the current thread is a bad idea
        if (t->m_id == threadId)
            continue;

        SuspendThread(t->m_handle);
        LogThread_CS(*t);
        ResumeThread(t->m_handle);
    }
}

//=============================================================================
static void CheckForDeadlocks_CS () {
    unsigned timeMs = GetTickCount();
    for (const Thread * t = s_threads.Head(); t; t = t->m_link.Next()) {
        // delta might be less than zero because of an inherent
        // race condition with ThreadMarkAlive; that's okay, but
        // requires that we use a signed comparison below.
        signed delta = (signed) (timeMs - t->m_lastTimeMs);
        if (delta < (signed) DEADLOCK_CHECK_FREQUENCY_MS)
            continue;

        // Deadlock!
        LogThreads_CS();
        DebugBreak();
        * (int *) 0 = 0;
        break;
    }
}

//=============================================================================
static unsigned __stdcall DeadlockThreadProc (void *) {
    for (;;) {
        DWORD result = WaitForSingleObject(
            s_deadlockEvent,
            DEADLOCK_CHECK_FREQUENCY_MS
        );
        if (result != WAIT_TIMEOUT)
            break;

        s_critsect.Enter();
        {
            CheckForDeadlocks_CS();
        }
        s_critsect.Leave();
    }

    return 0;
}


/******************************************************************************
*
*   Public
*
***/

//=============================================================================
void ThreadInit () {
    s_deadlockEvent = CreateEvent(NULL, true, false, NULL);
    ASSERT(s_deadlockEvent);

    unsigned threadId;
    s_deadlockThread = (HANDLE) _beginthreadex(
        NULL,
        0,
        DeadlockThreadProc,
        NULL,
        0,
        &threadId
    );
    ASSERT(s_deadlockThread);
}

//=============================================================================
void ThreadDestroy () {
    ASSERT(!s_threads.Head());

    if (s_deadlockThread) {
        SetEvent(s_deadlockEvent);
        WaitForSingleObject(s_deadlockThread, INFINITE);
        CloseHandle(s_deadlockThread);
        s_deadlockThread = NULL;
    }

    if (s_deadlockEvent) {
        CloseHandle(s_deadlockEvent);
        s_deadlockEvent = NULL;
    }
}

//=============================================================================
Thread * ThreadCreate (
    const char name[],
    unsigned stack_size,
    unsigned (__stdcall * start_address )( void * ),
    void *arglist
) {
    // Create thread suspended so we have a chance to register
    // it before starting it running
    Thread * t = new Thread(name);
    t->m_handle = (HANDLE) _beginthreadex(
        NULL,
        stack_size,
        start_address,
        arglist,
        CREATE_SUSPENDED,
        &t->m_id
    );
    ASSERT(t->m_handle);
    DebugSetThreadName(name, t->m_id);

    // Register thread
    s_critsect.Enter();
    {
        s_threads.InsertTail(t);
    }
    s_critsect.Leave();

    // *Now* start it
    ResumeThread(t->m_handle);
    return t;
}

//=============================================================================
void ThreadDestroy (Thread * t) {
    ASSERT(t->m_handle);

    // Wait for thread to exit and cleanup
    WaitForSingleObject(t->m_handle, INFINITE);
    CloseHandle(t->m_handle);
    t->m_handle = NULL;

    ThreadUnregister(t);
}

//=============================================================================
Thread * ThreadRegister (const char name[]) {
    // Create thread record with empty thread handle
    Thread * t = new Thread(name);
    t->m_id = GetCurrentThreadId();
    DebugSetThreadName(name, t->m_id);

    // Register thread
    s_critsect.Enter();
    {
        s_threads.InsertTail(t);
    }
    s_critsect.Leave();

    return t;
}

//=============================================================================
void ThreadUnregister (Thread * t) {
    ASSERT(!t->m_handle);

    // Safely unlink from thread list
    s_critsect.Enter();
    {
        t->m_link.Unlink();
    }
    s_critsect.Leave();

    delete t;
}    

//=============================================================================
void ThreadMarkAlive (Thread * thread) {
    thread->m_lastTimeMs = GetTickCount();
}

//=============================================================================
void ThreadLogAllThreads () {
    s_critsect.Enter();
    {
        LogThreads_CS();
    }
    s_critsect.Leave();
}


//===================================
// MIT License
//
// Copyright (c) 2012 by Patrick Wyatt
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
