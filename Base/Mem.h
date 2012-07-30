/******************************************************************************
*
*   Mem.h
*   
*
*   By Patrick Wyatt - 5/7/2010
*
***/


#ifdef MEM_H
#error "Header included more than once"
#endif
#define MEM_H


// Memory clear
#define ZERO(x)     memset(&x, 0, sizeof(x))
#define ZEROPTR(x)  memset(x, 0, sizeof(*x))

// Simplified allocation
#define ALLOC(bytes)        MemAlloc(bytes, __FILE__, __LINE__)
#define REALLOC(ptr, bytes) MemRealloc(ptr, bytes, __FILE__, __LINE__)

// Used to allocate a structure that contains a variable length text
// string at the end. Use StrChars for the "chars" field, not StrLen!
#define SIZEOF_STRUCT_STRING(type, field, chars)  (     \
        sizeof(type)                                    \
        - sizeof(((type *) 0)->field)                   \
        + sizeof(((type *) 0)->field[0]) * chars        \
    )                                                   //

// Memory allocation
void   MemFree (void * ptr);
void * MemAlloc (size_t bytes, const char file[], int line);
void * MemRealloc (void * ptr, size_t bytes, const char file[], int line);


//=============================================================================
inline void * MemAlloc (size_t bytes, const char file[], int line) {
    void * MemAllocHelper (size_t bytes, const char file[], int line);
    void * result = MemAllocHelper(bytes, file, line);
    __assume(result);
    return result;
}

//=============================================================================
// new/delete
inline void * operator new (unsigned bytes) {
    void * MemAllocHelper (size_t bytes, const char file[], int line);
    void * result = MemAllocHelper(bytes, __FILE__, __LINE__);
    __assume(result);
    return result;
}
inline void operator delete (void * p) {
    MemFree(p);
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
