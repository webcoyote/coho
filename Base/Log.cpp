/******************************************************************************
*
*   Log.cpp
*   
*
*   By Patrick Wyatt - 5/11/2010
*
***/


#include "stdafx.h"
#pragma hdrstop


/******************************************************************************
*
*   Private
*
***/

static FILE * s_log;


/******************************************************************************
*
*   Logging
*
***/

//=============================================================================
void LogInitialize (const wchar appName[], const wchar subDir[]) {
    wchar path[MAX_PATH];
    PathGetProgramDirectory(path, _countof(path));
    PathAppendW(path, subDir);
    if (!PathCreateDirectory(path))
        FatalError();

    PathAppendW(path, L"Error.log");
    _wfopen_s(&s_log, path, L"a+");
    LogError(
        "Log opened, %S, %d\n",
        appName,
        GetCurrentProcessId()
    );
}

//=============================================================================
void LogDestroy () {
    if (s_log) {    
        fclose(s_log);
        s_log = NULL;
    }
}

//=============================================================================
void FatalError () {
    if (_set_error_mode(_REPORT_ERRMODE) == _OUT_TO_MSGBOX) {
        wchar path[MAX_PATH];
        PathGetExeFileName(path, _countof(path));
        if (IDCANCEL == MessageBoxW(
            NULL,
            L"Fatal error",
            path,
            MB_OKCANCEL | MB_ICONHAND | MB_TASKMODAL))
            DebugBreak();
    }

    LogDestroy();
    ExitProcess(1);
}

//=============================================================================
void __cdecl Fatal (const char fmt[], ...) {
    va_list args;
    va_start(args, fmt);
    LogErrorV(fmt, args);
    va_end(args);
    FatalError();
}

//=============================================================================
void FatalAssert (const char msg[], const char file[], int line) {
    Fatal("Assert failed (%s:%d): %s\n", file, line, msg);
}

//=============================================================================
void LogErrorV (const char fmt[], va_list args) {
    if (!s_log)
        return;
    
    SYSTEMTIME time;
    GetSystemTime(&time);
    fprintf(
        s_log,
        "%02u:%02u:%02u ",
        time.wHour,
        time.wMinute,
        time.wSecond
    );
    vfprintf(
        s_log,
        fmt,
        args
    );
}

//=============================================================================
void __cdecl LogError (const char fmt[], ...) {
    va_list args;
    va_start(args, fmt);
    LogErrorV(fmt, args);
    va_end(args);
}

//=============================================================================
void LogOsError (const wchar msg[], unsigned error, const char file[], int line) {
    LogError("Err, %S, %u, %s, %d\n", msg, error, file, line);
}

//=============================================================================
void LogOsLastError (const wchar msg[], const char file[], int line) {
    LogOsError(msg, GetLastError(), file, line);
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
