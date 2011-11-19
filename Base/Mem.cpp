/******************************************************************************
*
*   Mem.cpp
*   
*
*   By Patrick Wyatt - 5/7/2010
*
***/


#include "stdafx.h"
#pragma hdrstop


#define USE_MALLOC


/******************************************************************************
*
*   Private
*
***/

#ifndef USE_MALLOC
static HANDLE s_heap;
#endif

//=============================================================================
static void OutOfMemory () {
    Fatal("Out of memory");
}


/******************************************************************************
*
*   Exports
*
***/

//=============================================================================
void MemFree (void * ptr) {
#ifdef USE_MALLOC
    _free_dbg(ptr, _NORMAL_BLOCK);
#else
    HeapFree(s_heap, 0, ptr);
#endif
}

//=============================================================================
void * MemAllocHelper (size_t bytes, const char file[], int line) {
#ifdef USE_MALLOC
    if (void * result = _malloc_dbg(bytes, _NORMAL_BLOCK, file, line))
        return result;
#else
    REF(file);
    REF(line);
    if (!s_heap)
        s_heap = GetProcessHeap();

    if (void * result = HeapAlloc(s_heap, 0, bytes))
        return result;
#endif

    OutOfMemory();
    return NULL;
}

//=============================================================================
void *  MemRealloc (void * ptr, size_t bytes, const char file[], int line) {
#ifdef USE_MALLOC
    if (void * result = _realloc_dbg(ptr, bytes, _NORMAL_BLOCK, file, line))
        return result;
#else
    REF(file);
    REF(line);
    if (!s_heap)
        s_heap = GetProcessHeap();

    if (void * result = HeapReAlloc(s_heap, 0, ptr, bytes))
        return result;
#endif
    OutOfMemory();
    return NULL;
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
