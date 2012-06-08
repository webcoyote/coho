/******************************************************************************
*
*   Timer.cpp
*   
*
*   By Patrick Wyatt - 11/19/2011
*
***/


#include "stdafx.h"
#pragma hdrstop

#if 0


/******************************************************************************
*
*   Private
*
***/

static const unsigned MAX_SLEEP_MS = 5 * 1000;

static const unsigned TIMER_FLAG_QUEUED = 1;


struct Timer : public ITimer {
    ITimerCallback *    m_callback;
    unsigned            m_nextTimeMs;
    unsigned            m_flags;

    virtual void Delete ();
    virtual void Set (__in unsigned sleepMs);

    bool operator < (const Timer & t) const;
};


static HANDLE       s_completionPort;
static HANDLE       s_timerThread;
static CCritSect    s_critsect;

static SOME_PRIORITY_QUEUE_THING<Timer>    s_timerQ;



/******************************************************************************
*
*   Timer object
*
***/

//=============================================================================
void Timer::Delete () {
    s_critsect.Enter();
    if (m_flags & TIMER_FLAG_QUEUED) {
        m_flags &= ~TIMER_FLAG_QUEUED;
        s_timerQ._Get_container().erase(*this);
    }
    s_critsect.Leave();
}

//=============================================================================
void Timer::Set (__in unsigned sleepMs) {
    unsigned nextTimeMs = TimeGetMs() + sleepMs;

    s_critsect.Enter();
    {
        if (m_flags & TIMER_FLAG_QUEUED) {
            m_flags &= ~TIMER_FLAG_QUEUED;
            s_timerQ.remove(*this);
        }
        if (sleepMs != TIMER_INFINITE_MS) {
            m_flags |= TIMER_FLAG_QUEUED;
            s_timerQ.add(*this);
        }
    }
    s_critsect.Leave();
}

//=============================================================================
bool Timer::operator< (const Timer & t) const {
    return (m_nextTimeMs - t.m_nextTimeMs) < 0;
}


/******************************************************************************
*
*   Timer thread
*
***/

//=============================================================================
static unsigned __stdcall TimerThreadProc (void *) {
    DebugSetThreadName("Timer");

    unsigned sleepMs = MAX_SLEEP_MS;
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
            sleepMs
        )) {
            LOG_OS_LAST_ERROR(L"GetQueuedCompletionStatus");
            FatalError();
        }

        // If the task is NULL this is a thread-quit notification
        if (!key)
            break;

    }

    return 0;
}


/******************************************************************************
*
*   Exports
*
***/

//=============================================================================
void TimerInitialize () {
    if (NULL == (s_completionPort = CreateIoCompletionPort(
        INVALID_HANDLE_VALUE,
        NULL,
        NULL,
        0
    ))) {
        LOG_OS_LAST_ERROR(L"CreateIoCompletionPort");
        FatalError();
    }

    unsigned threadId;
    if (NULL == (s_timerThread = (HANDLE) _beginthreadex(
        (LPSECURITY_ATTRIBUTES) NULL,
        0,      // default stack size
        TimerThreadProc,
        NULL,
        0,      // flags
        &threadId
    ))) {
        LOG_OS_LAST_ERROR(L"_beginthreadex");
        FatalError();
    }
}

//=============================================================================
void TimerDestroy () {
    if (s_timerThread) {
        PostQueuedCompletionStatus(s_completionPort, 0, 0, 0);
        WaitForSingleObject(s_timerThread, INFINITE);
        CloseHandle(s_timerThread);
        s_timerThread = NULL;
    }

    if (s_completionPort) {
        CloseHandle(s_completionPort);
        s_completionPort = NULL;
    }
}

//=============================================================================
void TimerCreate (
    __in    ITimerCallback *    callback,
    __in    unsigned            sleepMs,
    __out   ITimer **           timer
) {
    Timer * t       = new Timer;
    t->m_callback   = callback;
    t->m_nextTimeMs = TimeGetMs() + sleepMs;
    t->m_flags      = 0;

    // Set the timer *before* adding it to the queue to avoid a race
    // condition where a callback occurs before the parameter is set.
    *timer = t;

    if (sleepMs == TIMER_INFINITE_MS)
        return;

    s_critsect.Enter();
    {
        t->m_flags |= TIMER_FLAG_QUEUED;
        s_timerQ.add(t);
    }
    s_critsect.Leave();
}

#endif


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
