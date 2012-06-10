/************************************
*
*   Config.cpp
*   
*
*   By Patrick Wyatt - 5/19/2010
*
***/


#include "stdafx.h"
#pragma hdrstop
#include <hash_map>
#include <algorithm>


// http://github.com/etexteditor/e/blob/811bf09e31f26b2c86101448087fb264a26cca74/src/DirWatcher.cpp


/************************************
*
*   Private
*
***/


struct WCmpStrI {
    bool operator()(const wchar * a, const wchar * b) const {
        return _wcsicmp(a, b) < 0;
    }
};


struct ConfigFile {
    ConfigFile (const wchar filename[]);

    u64            m_lastWriteTime;
    wchar *        m_filename;    // points into m_fullpath
    wchar        m_fullpath[MAX_PATH];
};


typedef std::hash_map<
    const wchar *,
    ConfigFile *,
    std::hash_compare<const wchar *, WCmpStrI>
> ConfigFileMap;


typedef std::pair<
    const wchar *,
    ConfigFile *
> ConfigFilePair;


struct DirMonitor : public CTask {
    ConfigFileMap    m_files;
    OVERLAPPED        m_olap;
    HANDLE            m_handle;
    wchar            m_directory[MAX_PATH];
    byte            m_buffer[2*1024];

    DirMonitor (const wchar directory[]);
    virtual ~DirMonitor ();
    void Destroy_CS ();
    void WatchDirectory_CS ();
    void ScanDirectoryForChanges_CS ();
    void CheckReparseFile_CS (const wchar filename[]);

    // From CTask
    void TaskComplete (
        unsigned        bytes,
        OVERLAPPED *    olap
    );
};


typedef std::hash_map<
    const wchar *,
    DirMonitor *,
    std::hash_compare<const wchar *, WCmpStrI>
> DirMonitorMap;


typedef std::pair<
    const wchar *,
    DirMonitor *
> DirMonitorPair;


static DirMonitorMap    s_dirMonitors;
static unsigned            s_dirCount;
static CCritSect        s_critsect;


//===================================
static void ReparseFile (ConfigFile * file) {
    // TODO
    (void) file;
}


//===================================
ConfigFile::ConfigFile (const wchar filename[]) {
    m_lastWriteTime = 0;
    StrCopy(m_fullpath, _countof(m_fullpath), filename);
    m_filename = PathFindFileNameW(m_fullpath);
}


//===================================
struct DeleteConfigFile {
    bool operator()(ConfigFilePair x) const
    {
        delete x.second;
        return true;
    }
};


//===================================
DirMonitor::DirMonitor (const wchar directory[])
:   m_handle(INVALID_HANDLE_VALUE)
{
    StrCopy(m_directory, _countof(m_directory), directory);
    ZERO(m_olap);
    ++s_dirCount;
}


//===================================
DirMonitor::~DirMonitor () {
    ASSERT(m_handle == INVALID_HANDLE_VALUE);
    for_each(m_files.begin(), m_files.end(), DeleteConfigFile());
    m_files.clear();
    --s_dirCount;
}


//===================================
struct DestroyDirMonitor_CS {
    bool operator()(DirMonitorPair x) const
    {
        x.second->Destroy_CS();
        return true;
    }
};


//===================================
void DirMonitor::Destroy_CS () {
    // Closing the handle will initiate an asynchronous callback
    HANDLE handle   = m_handle;
    m_handle        = INVALID_HANDLE_VALUE;
    CloseHandle(handle);
}


//===================================
void DirMonitor::WatchDirectory_CS () {
    if (m_handle == INVALID_HANDLE_VALUE) {
        delete this;
    }
    else if (!ReadDirectoryChangesW(
        m_handle,
        m_buffer,
        sizeof(m_buffer),
        false,  // bWatchSubtree
        FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
        NULL,
        &m_olap,
        NULL
    )) {
        LOG_OS_LAST_ERROR(L"ReadDirectoryChangesW");
        FatalError();
    }
}


//===================================
static bool CheckFileWriteTime (const wchar filename[], u64 * lastWriteTime) {
    // Get the file attributes
    WIN32_FILE_ATTRIBUTE_DATA info;
    if (!GetFileAttributesExW(filename, GetFileExInfoStandard, &info)) {
        LOG_OS_LAST_ERROR(L"GetFileAttributesExW");
        return true;
    }

    // Did the file write time change?
    if (*lastWriteTime >= * (u64 *) &info.ftLastWriteTime)
        return false;

    // Update last write time
    // DebugMsg("%S changed\n", filename);
    *lastWriteTime = * (u64 *) &info.ftLastWriteTime;
    return true;
}


//===================================
void DirMonitor::CheckReparseFile_CS (const wchar filename[]) {
    // Is this a file being watched?
    ConfigFileMap::iterator pair = m_files.find(filename);
    if (pair == m_files.end())
        return;

    // Did the file change?
    ConfigFile * file = pair->second;
    if (!CheckFileWriteTime(file->m_fullpath, &file->m_lastWriteTime))
        return;

    ReparseFile(file);
}


//===================================
void DirMonitor::ScanDirectoryForChanges_CS () {
    // Search for all files in this directory
    wchar filespec[MAX_PATH];
    PathCombineW(filespec, m_directory, L"*");

    // Start search
    WIN32_FIND_DATAW data;
    HANDLE find = FindFirstFileW(filespec, &data);
    if (find == INVALID_HANDLE_VALUE) {
        LOG_OS_LAST_ERROR(L"FindFirstFileW");
        return;
    }

    // Search for watched files
    do {
        // Ignore directories
        if (data.dwFileAttributes & (FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_DIRECTORY))
            continue;
        
        // Convert to lowercase for hash matching
        wchar * filename = PathFindFileNameW(data.cFileName);
        CharLowerW(filename);

        // Reparse if file changed
        CheckReparseFile_CS(filename);

    } while (FindNextFileW(find, &data));

    // Cleanup
    FindClose(find);
}


//===================================
void DirMonitor::TaskComplete (
    unsigned        bytes,
    OVERLAPPED *
) {
    s_critsect.Enter();
    {
        if (m_handle == INVALID_HANDLE_VALUE) {
            // The monitor is ready to be deleted
        }
        // If no bytes read then m_buffer wasn't large enough to hold all the
        // updates; scan the directory to see which files need to be updated
        else if (!bytes) {
            ScanDirectoryForChanges_CS();
        }
        // Otherwise process the file notifications
        else for (const FILE_NOTIFY_INFORMATION * info = (const FILE_NOTIFY_INFORMATION *) m_buffer;;) {
            // Validate the structure
            // DebugMsg("  %u: %.*S\n", info->Action, info->FileNameLength / sizeof(info->FileName[0]), info->FileName);
            #ifdef ASSERTIONS_ENABLED
            size_t offset = (size_t) ((const byte *) info - (const byte *) m_buffer);
            ASSERT(offset < bytes);
            ASSERT(offset < sizeof_field(DirMonitor, m_buffer));
            #endif

            // Deleting or renaming a file does not cause re-parsing
            if ((info->Action == FILE_ACTION_REMOVED) || (info->Action == FILE_ACTION_RENAMED_OLD_NAME)) {
                // DebugMsg("%.*S deleted\n", info->FileNameLength / sizeof(info->FileName[0]), filename);
            }
            else {
                // Convert to lowercase for hash matching
                wchar filename[MAX_PATH];
                StrCopy(filename, min(_countof(filename), info->FileNameLength / sizeof(info->FileName[0]) + 1), info->FileName);
                CharLowerW(filename);

                // Reparse if file changed
                CheckReparseFile_CS(filename);
            }

            // Move to next entry
            if (!info->NextEntryOffset)
                break;
            info = (const FILE_NOTIFY_INFORMATION *) ((const byte *) info + info->NextEntryOffset);            
        }

        WatchDirectory_CS();
    }
    s_critsect.Leave();
}


//===================================
static DirMonitor * FindOrCreateDirectoryMonitor_CS (const wchar directory[]) {
    // Does this directory already exist in the monitor list?
    DirMonitorMap::iterator pair = s_dirMonitors.find(directory);
    if (pair != s_dirMonitors.end())
        return pair->second;

    // Create the directory monitor
    DirMonitor * dir = new DirMonitor(directory);
    s_dirMonitors.insert(DirMonitorPair(dir->m_directory, dir));

    // Open the directory handle
    if (INVALID_HANDLE_VALUE == (dir->m_handle = CreateFileW(
        directory,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        (LPSECURITY_ATTRIBUTES) NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL
    ))) {
        LOG_OS_LAST_ERROR(L"CreateFileW");
        FatalError();
    }

    // Start watching for notifications
    TaskRegisterHandle(dir, dir->m_handle);
    dir->WatchDirectory_CS();

    return dir;
}


/************************************
*
*   Exports
*
***/


//===================================
void ConfigInitialize () {
    // empty
}


//===================================
void ConfigDestroy () {
    s_critsect.Enter();
    {
        for_each(s_dirMonitors.begin(), s_dirMonitors.end(), DestroyDirMonitor_CS());
        s_dirMonitors.clear();
    }
    s_critsect.Leave();

    // Wait until all directory monitors are deleted asynchronously
    while (s_dirCount)
        Sleep(1);
}


//===================================
void ConfigMonitorFile (const wchar filename[]) {
    // Convert to canonical lowercase form
    wchar fullpath[MAX_PATH];
    if (!GetFullPathNameW(filename, _countof(fullpath), fullpath, NULL)) {
        LOG_OS_LAST_ERROR(L"GetFullPathNameW");
        FatalError();
    }
    CharLowerW(fullpath);

    // Create the directory that contains the file to be watched
    wchar directory[MAX_PATH];
    PathRemoveFileName(directory, _countof(directory), fullpath);
    if (!PathCreateDirectory(directory))
        FatalError();

    // Get the filename part
    filename = PathFindFileNameW(fullpath);

    s_critsect.Enter();
    {
        // Create a monitor for this directory
        DirMonitor * dir = FindOrCreateDirectoryMonitor_CS(directory);

        // Does this file already exist in the monitor's file list?
        ConfigFile * file;
        ConfigFileMap::iterator pair = dir->m_files.find(filename);
        if (pair == dir->m_files.end()) {
            file = new ConfigFile(fullpath);
            dir->m_files.insert(ConfigFilePair(file->m_filename, file));
        }
        else {
            file = pair->second;
        }

        // TODO: signal file for reparsing so the callback gets called.
    }
    s_critsect.Leave();
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
