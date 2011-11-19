/******************************************************************************
*
*   Str.cpp
*   
*
*   By Patrick Wyatt - 5/6/2010
*
***/


#include "stdafx.h"
#pragma hdrstop


/******************************************************************************
*
*   Exports
*
***/

//=============================================================================
void __cdecl StrPrintf (
    char *      dstBuf,
    size_t      dstChars,
    const char  format[],
    ...
) {
    va_list args;
    va_start(args, format);
    StrPrintfV(dstBuf, dstChars, format, args);
    va_end(args);
}

//=============================================================================
void __cdecl StrPrintf (
    wchar *     dstBuf,
    size_t      dstChars,
    const wchar format[],
    ...
) {
    va_list args;
    va_start(args, format);
    StrPrintfV(dstBuf, dstChars, format, args);
    va_end(args);
}

//=============================================================================
void StrPrintfV (
    char *      dstBuf,
    size_t      dstChars,
    const char  format[],
    va_list     args
) {
#pragma warning(disable:4996)    // unsafe function
    int result = _vsnprintf(
        dstBuf,
        dstChars,
        format,
        args
    );
    // Properly NULL-terminate the string
    if (dstChars && (unsigned) result >= dstChars)
        dstBuf[dstChars - 1] = 0;        
#pragma warning(default:4996)    // unsafe function
}

//=============================================================================
void StrPrintfV (
    wchar *     dstBuf,
    size_t      dstChars,
    const wchar format[],
    va_list     args
) {
#pragma warning(disable:4996)    // unsafe function
    int result = _vsnwprintf(
        dstBuf,
        dstChars,
        format,
        args
    );
    // Properly NULL-terminate the string
    if (dstChars && (unsigned) result >= dstChars)
        dstBuf[dstChars - 1] = 0;        
#pragma warning(default:4996)    // unsafe function
}

//=============================================================================
void StrCopy (
    char *      dstBuf,
    size_t      dstChars,
    const char  srcBuf[]
) {
    if (!dstChars)
        return;

    while (--dstChars) {
        if (0 == (*dstBuf = *srcBuf))
            return;
        ++dstBuf;
        ++srcBuf;
    }

    // Ensure string is always terminated
    *dstBuf = 0;
}

//=============================================================================
void StrCopy (
    wchar *     dstBuf,
    size_t      dstChars,
    const wchar srcBuf[]
) {
    if (!dstChars)
        return;

    while (--dstChars) {
        if (0 == (*dstBuf = *srcBuf))
            return;
        ++dstBuf;
        ++srcBuf;
    }

    // Ensure string is always terminated
    *dstBuf = 0;
}

//=============================================================================
size_t StrCopyLen (char * dstBuf, size_t dstChars, const char srcBuf[]) {
    if (!dstChars)
        return 0;
    
    char * dstBase = dstBuf;
    while (--dstChars) {
        if (0 == (*dstBuf = *srcBuf))
            return (size_t) (dstBuf - dstBase);
        ++dstBuf;
        ++srcBuf;
    }

    // Ensure string is always terminated
    *dstBuf = 0;
    return (size_t) (dstBuf - dstBase);
}

//=============================================================================
size_t StrCopyLen (wchar * dstBuf, size_t dstChars, const wchar srcBuf[]) {
    if (!dstChars)
        return 0;
    
    wchar * dstBase = dstBuf;
    while (--dstChars) {
        if (0 == (*dstBuf = *srcBuf))
            return (size_t) (dstBuf - dstBase);
        ++dstBuf;
        ++srcBuf;
    }

    // Ensure string is always terminated
    *dstBuf = 0;
    return (size_t) (dstBuf - dstBase);
}

//=============================================================================
size_t StrLen (const char str[]) {
    return (size_t) lstrlenA(str);
}
size_t StrLen (const wchar str[]) {
    return (size_t) lstrlenW(str);
}

//=============================================================================
size_t StrChars (const char str[]) {
    return (size_t) lstrlenA(str) + 1;
}
size_t StrChars (const wchar str[]) {
    return (size_t) lstrlenW(str) + 1;
}

//=============================================================================
size_t StrBytes (const char str[]) {
    return StrChars(str) * sizeof(str[0]);
}
size_t StrBytes (const wchar str[]) {
    return StrChars(str) * sizeof(str[0]);
}

//=============================================================================
void StrUnicodeToAnsi (char * dstBuf, size_t dstChars, const wchar srcBuf[]) {
    if (!WideCharToMultiByte(
        CP_ACP,
        0,
        srcBuf,
        -1,
        dstBuf,
        (int) (dstChars * sizeof(dstBuf[0])),
        NULL,
        NULL
    )) {
        LOG_OS_LAST_ERROR(L"WideCharToMultiByte");
        if (dstChars)
            dstBuf[0] = 0;
    }
}

//=============================================================================
void StrAnsiToUnicode (wchar * dstBuf, size_t dstChars, const char srcBuf[]) {
    if (!MultiByteToWideChar(
        CP_ACP,
        MB_PRECOMPOSED,
        srcBuf,
        -1,
        dstBuf,
        (int) dstChars
    )) {
        LOG_OS_LAST_ERROR(L"MultiByteToWideChar");
        if (dstChars)
            dstBuf[0] = 0;
    }
}

//=============================================================================
char * StrDupAnsi (const  char str[]) {
    unsigned bytes = StrBytes(str);
    char * dup = (char *) ALLOC(bytes);
    memcpy(dup, str, bytes);
    return dup;
}
wchar * StrDupWide (const wchar str[]) {
    unsigned bytes = StrBytes(str);
    wchar * dup = (wchar *) ALLOC(bytes);
    memcpy(dup, str, bytes);
    return dup;
}

//=============================================================================
wchar * StrDupAnsiToWide (const  char str[]) {
    unsigned chars = StrChars(str);
    wchar * dup = (wchar *) ALLOC(chars * sizeof(dup[0]));
    StrAnsiToUnicode(dup, chars, str);
    return dup;
}
char  * StrDupWideToAnsi (const wchar str[]) {
    unsigned chars = StrChars(str);
    char * dup = (char *) ALLOC(chars * sizeof(dup[0]));
    StrUnicodeToAnsi(dup, chars, str);
    return dup;
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
