/******************************************************************************
*
*   Path.cpp
*   
*
*   By Patrick Wyatt - 5/6/2010
*
***/


#include "stdafx.h"
#pragma hdrstop


/******************************************************************************
*
*   Private
*
***/

#pragma comment(lib, "shlwapi")


/******************************************************************************
*
*   Exports
*
***/

//=============================================================================
void PathGetExeFullPath (wchar * path, unsigned chars) {
    unsigned result = GetModuleFileNameW(NULL, path, chars);
    if (result >= chars)
        path[0] = 0;
}

//=============================================================================
void PathGetExeFileName (wchar * path, unsigned chars) {
    PathGetExeFullPath(path, chars);

    wchar * filename = PathFindFileNameW(path);
    if (filename != path)
        memmove(path, filename, StrBytes(filename));
}

//=============================================================================
void PathRemoveFileName (wchar * path, unsigned chars, const wchar src[]) {
    ASSERT(chars >= MAX_PATH);

    if (path != src)
        StrCopy(path, chars, src);
    PathRemoveFileSpecW(path);
}

//=============================================================================
void PathRemovePath (wchar * path, unsigned chars, const wchar src[]) {
    ASSERT(chars >= MAX_PATH);

    if (path != src)
        StrCopy(path, chars, src);
    PathStripPathW(path);
}

//=============================================================================
void PathGetProgramDirectory (wchar * path, unsigned chars) {
    PathGetExeFullPath(path, chars);

    // Trim the filename from path to get the directory
    wchar * filename = PathFindFileNameW(path);
    if (filename != path)
        filename[0] = 0;
}

//=============================================================================
void PathSetProgramDirectory () {
    wchar path[MAX_PATH];
    PathGetProgramDirectory(path, _countof(path));
    if (!SetCurrentDirectoryW(path)) {
        LOG_OS_LAST_ERROR(L"SetCurrentDirectoryW");
        FatalError();
    }
}

//=============================================================================
bool PathCreateDirectory (const wchar directory[]) {
    for (;;) {
        if (CreateDirectoryW(directory, NULL))
            break;

        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            if (PathIsDirectoryW(directory))
                break;
        }

        LOG_OS_LAST_ERROR(L"CreateDirectoryW");
        return false;
    }

    return true;
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
