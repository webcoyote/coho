/******************************************************************************
*
*   Debug.cpp
*   
*
*   By Patrick Wyatt - 5/20/2010
*
***/


#include "stdafx.h"
#pragma hdrstop


/******************************************************************************
*
*   Private
*
***/

// Validate Types.h definitions
CCASSERT(sizeof(i8) == 1);
CCASSERT(sizeof(u8) == 1);
CCASSERT(sizeof(i16) == 2);
CCASSERT(sizeof(u16) == 2);
CCASSERT(sizeof(i32) == 4);
CCASSERT(sizeof(u32) == 4);
CCASSERT(sizeof(i64) == 8);
CCASSERT(sizeof(u64) == 8);


// Definitions for DebugSetThreadName
#define MS_VC_EXCEPTION 0x406D1388

#pragma pack(push,8)
typedef struct {
   DWORD    dwType;     // Must be 0x1000.
   LPCSTR   szName;     // Pointer to name (in user addr space).
   DWORD    dwThreadID; // Thread ID (-1=caller thread).
   DWORD    dwFlags;    // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)


/******************************************************************************
*
*   Exports
*
***/

//=============================================================================
void __cdecl DebugMsg (const char fmt[], ...) {
    va_list args;
    va_start(args, fmt);
    DebugMsgV(fmt, args);
    va_end(args);
}

//=============================================================================
void DebugMsgV (const char fmt[], va_list args) {
    char msg[512];
    StrPrintfV(msg, _countof(msg), fmt, args);
    OutputDebugStringA(msg);
}

//=============================================================================
void DebugSetThreadName (const char name[], unsigned threadId) {
    THREADNAME_INFO info;
    info.dwType      = 0x1000;
    info.szName      = name;
    info.dwThreadID  = threadId ? threadId : GetCurrentThreadId();
    info.dwFlags     = 0;

    __try {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(unsigned long *), (unsigned long *) &info);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        // oh well ...
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
