/************************************
*
*   App.cpp
*   
*
*   By Patrick Wyatt - 5/14/2010
*
***/


#include "stdafx.h"
#pragma hdrstop

#pragma comment(lib, "base.lib")
#pragma comment(lib, "srvlib.lib")


/************************************
*
*   Private
*
***/

class CServiceApp : public CApplication {
private:
    HANDLE  m_shutdownEvt;

public:
    CServiceApp ();
    virtual ~CServiceApp ();

// Application config
    const wchar * Name () const;
    const wchar * Description () const;


// Application run
    void Start (CServiceStatus * status);
    void Run (bool serviceMode);
    void SignalStop ();
    void Stop ();
    int ExitCode () const;
};


static CServiceApp s_app;


//===================================
CServiceApp::CServiceApp ()
:   m_shutdownEvt(NULL)
{}


//===================================
CServiceApp::~CServiceApp () {
    if (m_shutdownEvt)
        CloseHandle(m_shutdownEvt);
}


//===================================
const wchar * CServiceApp::Name () const {
    return L"aSrv";
}


//===================================
const wchar * CServiceApp::Description () const {
    return L"A sample server";
}


//===================================
void CServiceApp::Start (CServiceStatus *) {
    m_shutdownEvt = CreateEvent(NULL, true, false, NULL);
}


//===================================
void CServiceApp::Run (bool serviceMode) {
    LogError("Running as %s\n", serviceMode ? "service" : "application");
    WaitForSingleObject(m_shutdownEvt, INFINITE);
}


//===================================
void CServiceApp::SignalStop () {
    SetEvent(m_shutdownEvt);
}


    //===================================
    // An alternate implementation of the game loop
    // static bool s_shutdown;
    // void CServiceApp::Start (CServiceStatus *) {
    //     s_shutdown = false;
    // }
    // void CServiceApp::Run (bool serviceMode) {
    //     LogError("Running as %s\n", serviceMode ? "service" : "application");
    //     while (!s_shutdown)
    //         DoImportantGameRelatedStuff();
    // }
    // void CServiceApp::SignalStop () {
    //     s_shutdown = true;
    // }


//===================================
void CServiceApp::Stop () {
    // empty
}


//===================================
int CServiceApp::ExitCode () const {
    // Success!
    return 0;
}


/************************************
*
*   Exports
*
***/

//===================================
#ifdef _CONSOLE
int __cdecl _tmain(int, _TCHAR**) {
    return ServiceMain(&s_app);
}
#else
int WINAPI WinMain (HINSTANCE, HINSTANCE, LPSTR, int) {
    return ServiceMain(&s_app);
}
#endif


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
