/******************************************************************************
*
*   Log.h
*   
*
*   By Patrick Wyatt - 5/11/2010
*
***/


#ifdef LOG_H
#error "Header included more than once"
#endif
#define LOG_H


// Module init
void LogInitialize (const wchar appName[], const wchar subDir[] = L"");
void LogDestroy ();


// Write the error message and exit application
void FatalError ();
void __cdecl Fatal (const char fmt[], ...);
void FatalAssert (const char msg[], const char file[], int line);


// Write error message
void __cdecl LogError (const char fmt[], ...);
void LogErrorV (const char fmt[], va_list args);


// Write operating system error
#define LOG_OS_ERROR(msg, error) LogOsError(msg, error, __FILE__, __LINE__)
void LogOsError (
    const wchar msg[],
    unsigned    error,
    const char  file[],
    int         line
);

#define LOG_OS_LAST_ERROR(msg) LogOsLastError(msg, __FILE__, __LINE__)
void LogOsLastError (
    const wchar msg[],
    const char  file[],
    int         line
);


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
