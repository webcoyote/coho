// List.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma hdrstop


/******************************************************************************
*
*   List tests
*
***/

namespace ListTest {

//=============================================================================
struct Data {
    LIST_LINK(Data) forward;
    LIST_LINK(Data) reverse;
    unsigned        value;
};

//=============================================================================
static void TestList () {
    LIST_DECLARE(Data, forward) forward;
    LIST_DECLARE(Data, reverse) reverse;

    for (unsigned j = 0; j < 100; ++j) {

        const unsigned COUNT = 10;
        ASSERT(forward.Empty());
        ASSERT(reverse.Empty());
        Data * last = NULL;
        for (unsigned i = 0; i < COUNT; ++i){
            Data * data = new Data;
            data->value = i;
            forward.InsertTail(data);
            reverse.InsertHead(data);
            ASSERT(forward.Prev(data) == last);
            ASSERT(reverse.Next(data) == last);
            last = data;
        }
        ASSERT(!forward.Empty());
        ASSERT(!reverse.Empty());

        Data * f = forward.Head();
        Data * r = reverse.Head();
        for (unsigned i = 0; i < COUNT; ++i) {
            ASSERT(f->value == i);
            ASSERT(r->value == COUNT - 1 - i);
            ASSERT(f->forward.Next() == forward.Next(f));
            ASSERT(r->reverse.Next() == reverse.Next(r));
            f = f->forward.Next();
            r = r->reverse.Next();
        }
        ASSERT(!f);
        ASSERT(!r);

        f = forward.Tail();
        r = reverse.Tail();
        for (unsigned i = 0; i < COUNT; ++i) {
            ASSERT(f->value == COUNT - 1 - i);
            ASSERT(r->value == i);
            ASSERT(f->forward.Prev() == forward.Prev(f));
            ASSERT(r->reverse.Prev() == reverse.Prev(r));
            f = f->forward.Prev();
            r = r->reverse.Prev();
        }
        ASSERT(!f);
        ASSERT(!r);

        forward.UnlinkAll();
        ASSERT(forward.Empty());

        ASSERT(!reverse.Empty());
        reverse.DeleteAll();
        ASSERT(reverse.Empty());
    }
}

//=============================================================================
static void InsertIntoBefore (LIST_PTR(Data) list, Data * data) {
    Data * before = list->Head();
    while (before) {
        if (before->value >= data->value)
            break;
        before = list->Next(before);
    }
    list->InsertBefore(data, before);
}

//=============================================================================
static void InsertIntoAfter (LIST_PTR(Data) list, Data * data) {
    Data * after = list->Tail();
    while (after) {
        if (after->value <= data->value)
            break;
        after = list->Prev(after);
    }
    list->InsertAfter(data, after);
}

//=============================================================================
static void TestRandom () {
    srand(GetTickCount());
    for (unsigned j = 0; j < 10000; ++j){
        LIST_DECLARE(Data, forward) forward;
        LIST_DECLARE(Data, reverse) reverse;

        // Insert random items sequentially
        for (unsigned i = 0; i < 20; ++i){
            Data * data = new Data;
            data->value = (unsigned) rand();
            InsertIntoBefore(&forward, data);
            InsertIntoAfter(&reverse, data);
        }

        // Ensure all items were inserted in order
        for (const Data * f = forward.Head(); const Data * next = forward.Next(f); f = next)
            ASSERT(f->value <= next->value);
        for (const Data * r = reverse.Tail(); const Data * prev = reverse.Prev(r); r = prev)
            ASSERT(r->value >= prev->value);

        // Cleanup
        forward.DeleteAll();
        ASSERT(reverse.Empty());
    }
}

}   // namespace ListTest


/******************************************************************************
*
*   Hash tests
*
***/

namespace HashTest {

//=============================================================================
struct DataKey {
    DataKey (unsigned value);
    unsigned GetHashValue () const;
    bool operator== (const struct Obj & obj) const;
    unsigned m_data;
};

struct TypeKey {
    TypeKey (unsigned value);
    unsigned GetHashValue () const;
    bool operator== (const struct Obj & obj) const;
    unsigned m_type;
};

struct Obj {
    Obj (unsigned value) : data(value), type(value) {}
    HASH_LINK(Obj)  byData;
    HASH_LINK(Obj)  byType;
    DataKey            data;
    TypeKey            type;
};


DataKey::DataKey (unsigned value) : m_data(value) {}
unsigned DataKey::GetHashValue () const { return m_data; }
bool DataKey::operator== (const Obj & obj) const { return obj.data.m_data == m_data; }

TypeKey::TypeKey (unsigned value) : m_type(value) {}
unsigned TypeKey::GetHashValue () const { return m_type; }
bool TypeKey::operator== (const Obj & obj) const { return obj.type.m_type == m_type; }


//=============================================================================
static void TestHash () {
    HASH_DECLARE(Obj, DataKey, byData) dataHash(32);
    HASH_DECLARE(Obj, TypeKey, byType) typeHash(17);

    for (unsigned j = 0; j < 100; ++j) {
        const unsigned COUNT = 10;
        ASSERT(dataHash.AllRowsEmpty());
        ASSERT(typeHash.AllRowsEmpty());
        for (unsigned i = 0; i < COUNT; ++i) {
            Obj * obj = new Obj(i);
            dataHash.Add(obj, obj->data.GetHashValue());
            typeHash.Add(obj, obj->type.GetHashValue());
        }
        ASSERT(!dataHash.AllRowsEmpty());
        ASSERT(!typeHash.AllRowsEmpty());

        for (unsigned i = 0; i < COUNT; ++i) {
            const Obj * obj;

            DataKey data(i);
            obj = dataHash.Find(data);
            ASSERT(obj);
            ASSERT(!dataHash.FindNext(data, obj));

            TypeKey type(i);
            obj = typeHash.Find(type);
            ASSERT(obj);
            ASSERT(!typeHash.FindNext(type, obj));
        }

        dataHash.UnlinkAll();
        ASSERT(dataHash.AllRowsEmpty());

        ASSERT(!typeHash.AllRowsEmpty());
        typeHash.DeleteAll();
        ASSERT(typeHash.AllRowsEmpty());
    }
}

//=============================================================================
static void TestRandom () {
    HASH_DECLARE(Obj, DataKey, byData) dataHash(16 * 1024);

    int seed = GetTickCount();
    srand(seed);
    for (unsigned j = 0; j < 10000; ++j) {
        Obj * obj = new Obj((unsigned) rand());
        dataHash.Add(obj, obj->data.GetHashValue());
    }

    srand(seed);
    for (unsigned j = 0; j < 10000; ++j) {
        unsigned value = (unsigned) rand();
        DataKey data(value);
        Obj * obj = dataHash.Find(data);
        delete obj;
    }

    ASSERT(dataHash.AllRowsEmpty());
}


}   // namespace HashTest


//=============================================================================
static void SetErrMode () {
    // Report to message box
    _set_error_mode(_OUT_TO_MSGBOX);

    // Send all errors to stdout
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE |  _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);
    _CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE |  _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);
    _CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE |  _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW);
    _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDOUT);
}


/******************************************************************************
*
*   Main
*
***/

//=============================================================================
int _tmain(int argc, _TCHAR* argv[]) {
    SetErrMode();

    // Test lists
    {
        #ifdef _DEBUG
        _CrtMemState before, after, delta;
        #endif
        _CrtMemCheckpoint(&before);
        ListTest::TestList();
        ListTest::TestRandom();
        _CrtMemCheckpoint(&after);
        if (_CrtMemDifference(&delta, &before, &after)) {
            printf("\n\nMemory leak in lists!\n\n");
            _CrtMemDumpStatistics(&delta);
            DebugBreak();
            return 1;
        }
    }

    // Test hashes
    {
        #ifdef _DEBUG
        _CrtMemState before, after, delta;
        #endif
        _CrtMemCheckpoint(&before);
        HashTest::TestHash();
        HashTest::TestRandom();
        _CrtMemCheckpoint(&after);
        if (_CrtMemDifference(&delta, &before, &after)) {
            printf("\n\nMemory leak in hashes!\n\n");
            _CrtMemDumpStatistics(&delta);
            DebugBreak();
            return 1;
        }
    }

    return 0;
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
