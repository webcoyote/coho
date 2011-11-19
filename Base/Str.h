/******************************************************************************
*
*   Str.h
*   
*
*   By Patrick Wyatt - 5/6/2010
*
***/


#ifdef MYSTR_H
#error "Header included more than once"
#endif
#define MYSTR_H


void __cdecl StrPrintf (
    char *      dstBuf,
    size_t      dstChars,
    const char  format[],
    ...
);
void __cdecl StrPrintf (
    wchar *     dstBuf,
    size_t      dstChars,
    const wchar format[],
    ...
);


void StrPrintfV (
    char *      dstBuf,
    size_t      dstChars,
    const char  format[],
    va_list     args
);
void StrPrintfV (
    wchar *     dstBuf,
    size_t      dstChars,
    const wchar format[],
    va_list     args
);


// String copy
void StrCopy ( char * dstBuf, size_t dstChars, const  char srcBuf[]);
void StrCopy (wchar * dstBuf, size_t dstChars, const wchar srcBuf[]);

size_t StrCopyLen ( char * dstBuf, size_t dstChars, const  char srcBuf[]);
size_t StrCopyLen (wchar * dstBuf, size_t dstChars, const wchar srcBuf[]);


// Number of characters of string *EXCLUDING* the terminating NULL: strlen()
size_t StrLen (const  char str[]);
size_t StrLen (const wchar str[]);

// Number of characters of string *INCLUDING* the terminating NULL: strlen() + 1
size_t StrChars (const  char str[]);
size_t StrChars (const wchar str[]);

// Number of characters of string *INCLUDING* the terminating NULL: (strlen() + 1) * sizeof(str[0])
size_t StrBytes (const  char str[]);
size_t StrBytes (const wchar str[]);


// Unicode <-> Ansi conversion
void StrUnicodeToAnsi (char * dstBuf, size_t dstChars, const wchar srcBuf[]);
void StrAnsiToUnicode (wchar * dstBuf, size_t dstChars, const char srcBuf[]);


// String duplication functions
char  * StrDupAnsi (const  char str[]); // use MemFree to relase
wchar * StrDupWide (const wchar str[]); // use MemFree to relase

wchar * StrDupAnsiToWide (const  char str[]); // use MemFree to relase
char  * StrDupWideToAnsi (const wchar str[]); // use MemFree to relase


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
