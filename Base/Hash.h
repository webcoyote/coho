/******************************************************************************
*
*   Hash.h
*   
*
*   By Patrick Wyatt - 5/16/2010
*
***/


// WARNING: A proper hash table should be able to grow the row-count, but this
// version doesn't yet have that functionality yet.


/******************************************************************************
*
*   WHAT IT IS
*
*   This module defines a hash-table implementation that uses "embedded"
*   links rather than separately allocated link-nodes as does STL and
*   more or less all other hash-table implementations.
*
*   Why is this cool:
*       1. No additional memory allocations (malloc) required to link
*           an object into a hash table.
*       2. Not necessary to traverse an additional pointer references
*           to get to the object being dereferenced.
*       3. Probably most importantly, when objects get deleted, they
*           automatically unlink themselves from the tables they're
*           linked to, eliminating many common types of bugs.
*
*   HOW TO USE IT
*
*   Declare a structure that will be contained in one or more hash tables:
*
*       class CFooDataKey {
*           unsigned GetHashValue () const;
*           bool operator== (const class CFoo &) const;
*           unsigned m_data;
*       };
*
*       class CFooTypeKey {
*           unsigned GetHashValue () const;
*           bool operator== (const class CFoo &) const;
*           unsigned m_type;
*       };
*
*       class CFoo {
*           HASH_LINK(CFoo) m_hashByData;
*           HASH_LINK(CFoo) m_hashByType;
*           CFooDataKey     m_data;
*           CFooTypeKey     m_type;
*           ...
*       };
*
*   Declare hash variables:
*       HASH_DECLARE(CFoo, CFooDataKey, m_linkByData) hashByData(1024);
*       HASH_DECLARE(CFoo, CFooTypeKey, m_linkByType) hashByType(32);
*       HASH_PTR(CFoo, CFooTypeKey) hashPtr = foo ? &hashByData : &hashByType;
*
*   Insert into table
*       hashByData.Add(someVar, someVar->m_data.GetHashValue());
*       hashByType.Add(someVar, someVar->m_type.GetHashValue());
*
*   Operations on links:
*       T * Prev ();
*       T * Next ();
*       void Unlink ();
*       bool IsLinked () const;
*
*   Operations on hashes:
*       bool AllRowsEmpty () const;
*       void UnlinkAll ();
*       void DeleteAll ();
*
*       T * Prev (T * node);
*       T * Next (T * node);
*       const T * Prev (const T * node) const;
*       const T * Next (const T * node) const;
*
*       void Add (T * node);
*       void Add (T * node, unsigned hashVal);
*
*       T * Find (const TKey & key);
*       const T * Find (const TKey & key) const;
*
*       T * FindNext (const TKey & key, const T * node);
*       const T * FindNext (const TKey & key, const T * node) const;
*
*   NOTES
*
*   Limitations:
*       All nodes must be allocated on (at least) two-byte boundaries
*       because the low bit is used as a sentinel to signal end-of-list.
*       
*   Thanks to:
*       Something like this code was originally implemented by Mike
*       O'Brien in storm.dll for Diablo in 1995, and again at ArenaNet
*       for Guild Wars.
*
***/


#ifdef HASH_H
#error "Header included more than once"
#endif
#define HASH_H


/******************************************************************************
*
*   Hash definition macros
*
***/

// Define a hash table:
// T    = type of object being hashed
// link = member within object which is the link field
#define HASH_DECLARE(TObj, TKey, link) THashDeclare<TObj, TKey, offsetof(TObj, link)>

// Define a field within a structure that will be used to link it into a table
#define HASH_LINK(T) TLink<T>

// Define a pointer to a list
#define HASH_PTR(T,TKey) THash<T, TKey> *


/******************************************************************************
*
*   THash
*
***/

//=============================================================================
template<class T, class TKey>
class THash {
public:
    ~THash ();
    THash (unsigned rows);

    bool AllRowsEmpty () const;
    void UnlinkAll ();
    void DeleteAll ();

    void Add (T * node);
    void Add (T * node, unsigned hashVal);

    T * Find (const TKey & key);
    const T * Find (const TKey & key) const;

    T * FindNext (const TKey & key, const T * node);
    const T * FindNext (const TKey & key, const T * node) const;

private:
    unsigned    m_rows;
    size_t      m_offset;
    TLink<T> *  m_buckets;

    T * Next (T * node);
    const T * Next (const T * node) const;
    const T * FindInternal (const TKey & key, const T * node) const;

    THash (unsigned rows, size_t offset);
    TLink<T> * GetLinkFromNode (const T * node) const;
    template<class T, class TKey, size_t offset> friend class THashDeclare;

    // Hide copy-constructor and assignment operator
    THash (const THash &);
    THash & operator= (const THash &);
};

//=============================================================================
template<class T, class TKey>
THash<T, TKey>::~THash () {
    delete [] m_buckets;
}

//=============================================================================
template<class T, class TKey>
THash<T, TKey>::THash (unsigned rows) : THash<T, TKey>(rows, 0)
{}

//=============================================================================
template<class T, class TKey>
THash<T, TKey>::THash (unsigned rows, size_t offset) :
    m_rows(rows),
    m_offset(offset)
{
    ASSERT(rows);
    m_buckets = new TLink<T>[rows];
    for (unsigned i = 0; i < m_rows; ++i)
        m_buckets[i].SetOffset(offset);
}

//=============================================================================
template<class T, class TKey>
bool THash<T, TKey>::AllRowsEmpty () const {
    for (unsigned i = 0; i < m_rows; ++i) {
        if (m_buckets[i].Next() != NULL)
            return false;
    }
    return true;
}

//=============================================================================
template<class T, class TKey>
void THash<T, TKey>::UnlinkAll () {
    for (unsigned i = 0; i < m_rows; ++i) {
        for (;;) {
            TLink<T> * link = m_buckets[i].PrevLink();
            if (link == &m_buckets[i])
                break;
            link->Unlink();
        }
    }
}

//=============================================================================
template<class T, class TKey>
void THash<T, TKey>::DeleteAll () {
    for (unsigned i = 0; i < m_rows; ++i) {
        while (T * node = m_buckets[i].Next())
            delete node;
    }
}

//=============================================================================
template<class T, class TKey>
void THash<T, TKey>::Add (T * node) {
    Add(node, node->GetHashVal());
}

//=============================================================================
template<class T, class TKey>
void THash<T, TKey>::Add (T * node, unsigned hashVal) {
    ASSERT(!((size_t) node & 1));
    GetLinkFromNode(node)->InsertBefore(
        node,
        &m_buckets[hashVal % m_rows]
    );
}

//=============================================================================
template<class T, class TKey>
T * THash<T, TKey>::Find (const TKey & key) {
    TLink<T> & link = m_buckets[key.GetHashValue() % m_rows];
    return (T *) FindInternal(key, link.Next());
}

//=============================================================================
template<class T, class TKey>
const T * THash<T, TKey>::Find (const TKey & key) const {
    const TLink<T> & link = m_buckets[key.GetHashValue() % m_rows];
    return FindInternal(key, link.Next());
}

//=============================================================================
template<class T, class TKey>
T * THash<T, TKey>::FindNext (const TKey & key, const T * node) {
    return (T *) FindInternal(key, Next(node));
}

//=============================================================================
template<class T, class TKey>
const T * THash<T, TKey>::FindNext (const TKey & key, const T * node) const {
    return FindInternal(key, Next(node));
}

//=============================================================================
template<class T, class TKey>
const T * THash<T, TKey>::FindInternal (const TKey & key, const T * node) const {
    while (node) {
        if (key == *node)
            return node;
        node = Next(node);
    }
    return NULL;
}

//=============================================================================
template<class T, class TKey>
T * THash<T, TKey>::Next (T * node) {
    return GetLinkFromNode(node)->Next();
}

//=============================================================================
template<class T, class TKey>
const T * THash<T, TKey>::Next (const T * node) const {
    return GetLinkFromNode(node)->Next();
}

//=============================================================================
template<class T, class TKey>
TLink<T> * THash<T, TKey>::GetLinkFromNode (const T * node) const {
    ASSERT(m_offset != (size_t) -1);
    return (TLink<T> *) ((size_t) node + m_offset);
}


/******************************************************************************
*
*   THashDeclare - declare a list with a known link offset
*
***/

//=============================================================================
template<class T, class TKey, size_t offset>
class THashDeclare : public THash<T, TKey> {
public:
    THashDeclare (unsigned rows);
};

//=============================================================================
template<class T, class TKey, size_t offset>
THashDeclare<T, TKey, offset>::THashDeclare (unsigned rows) :
    THash<T, TKey>(rows, offset)
{}

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
