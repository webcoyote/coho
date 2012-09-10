/******************************************************************************
*
*   List.h
*   
*
*   By Patrick Wyatt - 5/16/2010
*
***/


/******************************************************************************
*
*   WHAT IT IS
*
*   This module defines a linked-list implementation that uses "embedded"
*   links rather than separately allocated link-nodes as does STL and
*   more or less all other linked-list implementations.
*
*   Why is this cool:
*       1. No additional memory allocations (malloc) required to link
*           an object into a linked list.
*       2. Not necessary to traverse an additional pointer references
*           to get to the object being dereferenced.
*       3. Probably most importantly, when objects get deleted, they
*           automatically unlink themselves from the lists they're
*           linked to, eliminating many common types of bugs.
*
*   HOW TO USE IT
*
*   Declare a structure that will be contained in one or more linked lists:
*       class CFoo {
*           LIST_LINK(CFoo) m_linkByData;
*           LIST_LINK(CFoo) m_linkByType;
*           int             m_data;
*           int             m_type;
*           ...
*       };
*
*   Declare list variables:
*       LIST_DECLARE(CFoo, m_linkByData) listByData;
*       LIST_DECLARE(CFoo, m_linkByType) listByType;
*       LIST_PTR(CFoo) m_listPtr = foo ? &listByData : &listByType;
*
*   Operations on links:
*       T * Prev ();
*       T * Next ();
*       void Unlink ();
*       bool IsLinked () const;
*
*   Operations on lists:
*       bool Empty () const;
*       void UnlinkAll ();
*       void DeleteAll ();
*
*       T * Head (); 
*       T * Tail ();
*       T * Prev (T * node);
*       T * Next (T * node);
*
*       void InsertHead (T * node);
*       void InsertTail (T * node);
*       void InsertBefore (T * node, T * before);
*       void InsertAfter (T * node, T * after);
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


#ifdef LIST_H
#error "Header included more than once"
#endif
#define LIST_H


/******************************************************************************
*
*   List definition macros
*
***/

// Define a linked list:
// T    = type of object being linked
// link = member within object which is the link field
#define LIST_DECLARE(T, link) TListDeclare<T, offsetof(T, link)>

// Define a field within a structure that will be used to link it into a list
#define LIST_LINK(T) TLink<T>

// Define a pointer to a list
#define LIST_PTR(T) TList<T> *


/******************************************************************************
*
*   TLink
*
***/

//=============================================================================
template<class T>
class TLink {
public:
    ~TLink ();
    TLink ();

    bool IsLinked () const;
    void Unlink ();

    T * Prev ();
    T * Next ();
    const T * Prev () const;
    const T * Next () const;

    // For use by list-type classes, not user code;
    // the alternative is to friend TList<T>, THash<T>,
    // and (eventually) many other structures.
    TLink (size_t offset);
    void SetOffset (size_t offset);
    TLink<T> * NextLink ();
    TLink<T> * PrevLink ();
    void InsertBefore (T * node, TLink<T> * nextLink);
    void InsertAfter (T * node, TLink<T> * prevLink);

private:
    T *         m_nextNode; // pointer to the next >object<
    TLink<T> *  m_prevLink; // pointer to the previous >link field<
    void RemoveFromList ();

    // Hide copy-constructor and assignment operator
    TLink (const TLink &);
    TLink & operator= (const TLink &);
};

//=============================================================================
template<class T>
TLink<T>::~TLink () {
    RemoveFromList();
}

//=============================================================================
template<class T>
TLink<T>::TLink () {
    // Mark this node as the end of the list, with no link offset
    m_nextNode = (T *) ((size_t) this + 1 - 0);
    m_prevLink = this;
}

//=============================================================================
template<class T>
TLink<T>::TLink (size_t offset) {
    // Mark this node as the end of the list, with the link offset set
    m_nextNode = (T *) ((size_t) this + 1 - offset);
    m_prevLink = this;
}

//=============================================================================
template<class T>
void TLink<T>::SetOffset (size_t offset) {
    // Mark this node as the end of the list, with the link offset set
    m_nextNode = (T *) ((size_t) this + 1 - offset);
    m_prevLink = this;
}

//=============================================================================
template<class T>
TLink<T> * TLink<T>::NextLink () {
    // Calculate the offset from a node pointer to a link structure
    size_t offset = (size_t) this - ((size_t) m_prevLink->m_nextNode & ~1);

    // Get the link field for the next node
    return (TLink<T> *) (((size_t) m_nextNode & ~1) + offset);
}

//=============================================================================
template<class T>
void TLink<T>::RemoveFromList () {
    NextLink()->m_prevLink = m_prevLink;
    m_prevLink->m_nextNode = m_nextNode;
}

//=============================================================================
template<class T>
void TLink<T>::InsertBefore (T * node, TLink<T> * nextLink) {
    RemoveFromList();

    m_prevLink = nextLink->m_prevLink;
    m_nextNode = m_prevLink->m_nextNode;

    nextLink->m_prevLink->m_nextNode = node;
    nextLink->m_prevLink = this;
}

//=============================================================================
template<class T>
void TLink<T>::InsertAfter (T * node, TLink<T> * prevLink) {
    RemoveFromList();

    m_prevLink = prevLink;
    m_nextNode = prevLink->m_nextNode;

    prevLink->NextLink()->m_prevLink = this;
    prevLink->m_nextNode = node;
}

//=============================================================================
template<class T>
bool TLink<T>::IsLinked () const {
    return m_prevLink != this;
}

//=============================================================================
template<class T>
void TLink<T>::Unlink () {
    RemoveFromList();

    // Mark this node as the end of the list with no link offset
    m_nextNode = (T *) ((size_t) this + 1);
    m_prevLink = this;
}

//=============================================================================
template<class T>
TLink<T> * TLink<T>::PrevLink () {
    return m_prevLink;
}

//=============================================================================
template<class T>
T * TLink<T>::Prev () {
    T * prevNode = m_prevLink->m_prevLink->m_nextNode;
    if ((size_t) prevNode & 1)
        return NULL;
    return prevNode;
}

//=============================================================================
template<class T>
const T * TLink<T>::Prev () const {
    const T * prevNode = m_prevLink->m_prevLink->m_nextNode;
    if ((size_t) prevNode & 1)
        return NULL;
    return prevNode;
}

//=============================================================================
template<class T>
T * TLink<T>::Next () {
    if ((size_t) m_nextNode & 1)
        return NULL;
    return m_nextNode;
}

//=============================================================================
template<class T>
const T * TLink<T>::Next () const {
    if ((size_t) m_nextNode & 1)
        return NULL;
    return m_nextNode;
}


/******************************************************************************
*
*   TList
*
***/

//=============================================================================
template<class T>
class TList {
public:
    ~TList ();
    TList ();

    bool Empty () const;
    void UnlinkAll ();
    void DeleteAll ();

    T * Head (); 
    T * Tail ();
    const T * Head () const;
    const T * Tail () const;

    T * Prev (T * node);
    T * Next (T * node);
    const T * Prev (const T * node) const;
    const T * Next (const T * node) const;

    void InsertHead (T * node);
    void InsertTail (T * node);
    void InsertBefore (T * node, T * before);
    void InsertAfter (T * node, T * after);

private:
    TLink<T>    m_link;
    size_t      m_offset;

    TList (size_t offset);
    TLink<T> * GetLinkFromNode (const T * node) const;
    template<class T, size_t offset> friend class TListDeclare;

    // Hide copy-constructor and assignment operator
    TList (const TList &);
    TList & operator= (const TList &);
};

//=============================================================================
template<class T>
TList<T>::~TList () {
    UnlinkAll();
}

//=============================================================================
template<class T>
TList<T>::TList () :
    m_link(),
    m_offset((size_t) -1)
{}

//=============================================================================
template<class T>
TList<T>::TList (size_t offset) :
    m_link(offset),
    m_offset(offset)
{}

//=============================================================================
template<class T>
bool TList<T>::Empty () const {
    return m_link.Next() == NULL;
}

//=============================================================================
template<class T>
void TList<T>::UnlinkAll () {
    for (;;) {
        TLink<T> * link = m_link.PrevLink();
        if (link == &m_link)
            break;
        link->Unlink();
    }
}

//=============================================================================
template<class T>
void TList<T>::DeleteAll () {
    while (T * node = m_link.Next())
        delete node;
}

//=============================================================================
template<class T>
T * TList<T>::Head () {
    return m_link.Next();
}

//=============================================================================
template<class T>
T * TList<T>::Tail () {
    return m_link.Prev();
}

//=============================================================================
template<class T>
const T * TList<T>::Head () const {
    return m_link.Next();
}

//=============================================================================
template<class T>
const T * TList<T>::Tail () const {
    return m_link.Prev();
}

//=============================================================================
template<class T>
T * TList<T>::Prev (T * node) {
    return GetLinkFromNode(node)->Prev();
}

//=============================================================================
template<class T>
const T * TList<T>::Prev (const T * node) const {
    return GetLinkFromNode(node)->Prev();
}

//=============================================================================
template<class T>
T * TList<T>::Next (T * node) {
    return GetLinkFromNode(node)->Next();
}

//=============================================================================
template<class T>
const T * TList<T>::Next (const T * node) const {
    return GetLinkFromNode(node)->Next();
}

//=============================================================================
template<class T>
void TList<T>::InsertHead (T * node) {
    InsertAfter(node, NULL);
}

//=============================================================================
template<class T>
void TList<T>::InsertTail (T * node) {
    InsertBefore(node, NULL);
}

//=============================================================================
template<class T>
void TList<T>::InsertBefore (T * node, T * before) {
    ASSERT(!((size_t) node & 1));
    GetLinkFromNode(node)->InsertBefore(
        node,
        before ? GetLinkFromNode(before) : &m_link
    );
}

//=============================================================================
template<class T>
void TList<T>::InsertAfter (T * node, T * after) {
    ASSERT(!((size_t) node & 1));
    GetLinkFromNode(node)->InsertAfter(
        node,
        after ? GetLinkFromNode(after) : &m_link
    );
}

//=============================================================================
template<class T>
TLink<T> * TList<T>::GetLinkFromNode (const T * node) const {
    ASSERT(m_offset != (size_t) -1);
    return (TLink<T> *) ((size_t) node + m_offset);
}

/******************************************************************************
*
*   TListDeclare - declare a list with a known link offset
*
***/

//=============================================================================
template<class T, size_t offset>
class TListDeclare : public TList<T> {
public:
    TListDeclare ();
};

//=============================================================================
template<class T, size_t offset>
TListDeclare<T, offset>::TListDeclare () : TList<T>(offset)
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
