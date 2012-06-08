/******************************************************************************
*
*   Macros.h
*   
*
*   By Patrick Wyatt - 5/16/2010
*
***/


#ifdef MACROS_H
#error "Header included more than once"
#endif
#define MACROS_H


/******************************************************************************
*
*   Defines
*
***/

#ifdef _DEBUG
#define ASSERTIONS_ENABLED
#endif


/******************************************************************************
*
*   Utility macros
*
***/

// number of elements in an array
// _countof(array) -- defined in MSVC headers

// size of field in a structure
#define sizeof_field(type, field) sizeof(((type *) 0)->field)


// Do nothing
#define NULL_STMT ((void) 0)


// shorter version of UNREFERENCED_PARAMETER
#define REF(arg)    (arg)


// type definition for API classes with no implementation
#define APICLASS struct __declspec(novtable)


// Run-time assertion
#ifdef ASSERTIONS_ENABLED
#define ASSERT(x) ((x) ? 0 : FatalAssert(#x, __FILE__, __LINE__))
#define ASSERTFAST(x) ASSERT(x)
#else
#define ASSERT(x) ((void) 0)
#define ASSERTFAST(x) __assume(x)
#endif


// Compile-time assertion 
#define CCASSERT(predicate) _x_CCASSERT_LINE(predicate, __LINE__)
#define _x_CCASSERT_LINE(predicate, line) typedef char constraint_violated_on_line_##line[2*((predicate)!=0)-1];


// Output WARNINGS and NOTES during compilation
#define STRING2_(x) #x
#define STRING_(x) STRING2_(x)
#define NOTE(msg) message (__FILE__ "[" STRING_(__LINE__) "] : note: " msg)
#define WARN(msg) message (__FILE__ "[" STRING_(__LINE__) "] : warning C0000: " msg)


// Swap two values
template <typename T>
inline void swap (T & a, T & b) {
    T temp(a);
    a = b;
    b = temp;
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
