/******************************************************************************
*
*   Timer.h
*   
*
*   By Patrick Wyatt - 11/19/2011
*
***/


#ifdef TIMER_H
#error "Header included more than once"
#endif
#define TIMER_H


/******************************************************************************
*
*   Exports
*
***/

// Used for TimerCreate() and Timer::Set()
const unsigned TIMER_INFINITE_MS = (unsigned) -1;

// Your class should derive from this class to receive a callback
APICLASS ITimerCallback {
    virtual unsigned OnTimer () = 0;
};

// When you create a timer you get this timer management object
APICLASS ITimer {
    virtual void Delete () = 0;
    virtual void Set (__in unsigned sleepMs) = 0;
};

void TimerCreate (
    __in    ITimerCallback *    callback,
    __in    unsigned            sleepMs,
    __out   ITimer **           timer
);

// Module creation/destruction
void TimerInitialize ();
void TimerDestroy ();


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
