/************************************
*
*   SrvLib.cpp
*   
*
*   By Patrick Wyatt - 5/6/2010
*
***/


#include "stdafx.h"
#pragma hdrstop


/************************************
*
*   Private
*
***/


static CApplication * s_app;


/************************************
*
*   Command line parser
*
***/


enum EAppMode {
    APP_RUN_APPLICATION,
    APP_RUN_SERVICE,
    APP_INSTALL_SERVICE,
    APP_USAGE_SHOW,
    APP_FAILED_ERROR,
};


struct CmdLineOptions {
private:
    bool m_error;

    void ParseOpt (const wchar arg[]);
    void ParseArg (const wchar arg[]);
    void __cdecl CmdLineError (const char fmt[], ...);

public:
    EAppMode    m_appMode;
    wchar       m_username[64];
    wchar       m_password[64];

    CmdLineOptions (const wchar cmdLine[]);
};


//===================================
static void Usage () {
    wchar message[512];
    StrPrintf(
        message,
        _countof(message),
        L"Usage:\n"
        L"    %s [options]\n"
        L"\n"
        L"Options:\n"
        L"   /?\t\t\tOutput this help\n"
        L"   /install\t\t\tInstall as service\n"
        L"   /username:<user>\tInstallation username\n"
        L"   /password:<pass>\tInstallation password\n"
        L"\n"
        ,   // end of format string
        s_app->Name()
    );

    #ifdef _CONSOLE
        fputws(message, stderr);
    #else
        wchar caption[MAX_PATH];
        PathGetExeFileName(caption, _countof(caption));
        MessageBoxW(NULL, message, caption, MB_OK);
    #endif
}


//===================================
CmdLineOptions::CmdLineOptions (const wchar cmdLine[]) {
    // Initialize options
    m_error         = false;
    m_appMode       = APP_RUN_APPLICATION;
    m_username[0]   = 0;
    m_password[0]   = 0;

    // Tokenize the command line
    int argc = 0;
    wchar ** argv = CommandLineToArgvW(cmdLine, &argc);
    if (!argv) {
        LOG_OS_LAST_ERROR(L"CommandLineToArgvW");
        m_appMode = APP_FAILED_ERROR;
        return;
    }

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        wchar first = argv[i][0];
        if (first == '/' || first == '-')
            ParseOpt(argv[i] + 1);
        else
            ParseArg(argv[i]);
        if (m_error)
            break;
    }

    if (m_error)
        m_appMode = APP_FAILED_ERROR;
    LocalFree(argv);
}
    

//===================================
void CmdLineOptions::ParseOpt (const wchar arg[]) {
    static const wchar ARG_USERNAME[] = L"username:";
    static const wchar ARG_PASSWORD[] = L"password:";

    // NOTE: the "/service" argument isn't listed in the command-line usage
    // because it should only be used by the service control manager
    if (!wcscmp(arg, L"service"))
        m_appMode = APP_RUN_SERVICE;
    else if (!wcscmp(arg, L"install"))
        m_appMode = APP_INSTALL_SERVICE;
    else if (!wcscmp(arg, L"?"))
        m_appMode = APP_USAGE_SHOW;
    else if (!wcsncmp(arg, ARG_USERNAME, _countof(ARG_USERNAME) - 1))
        StrCopy(m_username, _countof(m_username), arg + _countof(ARG_USERNAME) - 1);
    else if (!wcsncmp(arg, ARG_PASSWORD, _countof(ARG_PASSWORD) - 1))
        StrCopy(m_password, _countof(m_password), arg + _countof(ARG_PASSWORD) - 1);
    else
        CmdLineError("ERROR: unknown command-line option -'%S'\n", arg);
}


//===================================
void CmdLineOptions::ParseArg (const wchar arg[]) {
    CmdLineError("ERROR: unknown command-line argument '%S'\n", arg);
}


//===================================
void __cdecl CmdLineOptions::CmdLineError (const char fmt[], ...) {
    va_list args;
    va_start(args, fmt);
    #ifdef _CONSOLE
        vfprintf(stderr, fmt, args);
    #else
        LogErrorV(fmt, args);
    #endif
    va_end(args);
    m_error = true;
}


/************************************
*
*   Service run
*
***/


struct ServiceParam : public CServiceStatus {
    SERVICE_STATUS_HANDLE   handle;
    SERVICE_STATUS          status;

    void SetState (DWORD state);

    // From CServiceStatus
    virtual void UpdateStatus (
        unsigned checkPoint,
        unsigned waitHintMs
    );
};


//===================================
void ServiceParam::SetState (DWORD state) {
    status.dwCurrentState = state;
    if (handle && !SetServiceStatus(handle, &status))
        LOG_OS_LAST_ERROR(L"SetServiceStatus");
}


//===================================
void ServiceParam::UpdateStatus (
    unsigned checkPoint,
    unsigned waitHintMs
) {
    if (handle) {
        status.dwCheckPoint = checkPoint;
        status.dwWaitHint   = waitHintMs;
        SetServiceStatus(handle, &status);
    }
}


//===================================
static DWORD WINAPI ServiceHandlerEx (
    DWORD   dwControl,
    DWORD   , // dwEventType,
    LPVOID  , // lpEventData,
    LPVOID  lpContext
) {
    ServiceParam & sp = * (ServiceParam *) lpContext;
    switch (dwControl) {
        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
            sp.SetState(SERVICE_STOP_PENDING);
            s_app->SignalStop();
        return NO_ERROR;

        case SERVICE_CONTROL_INTERROGATE:
            SetServiceStatus(sp.handle, &sp.status);
        return NO_ERROR;
    }

    return ERROR_CALL_NOT_IMPLEMENTED;
}


//===================================
// This section is only necessary because Visual Studio 6 doesn't have bindings
// for RegisterServiceCtrlHandlerExW so I have to manually load the library
// that contains the function. Time to upgrade to VS10!
static SERVICE_STATUS_HANDLE RegisterService (ServiceParam * sp) {
    HMODULE lib                     = NULL;
    SERVICE_STATUS_HANDLE handle    = NULL;
    for (;;) {
        // Load service library
        lib = LoadLibraryW(L"AdvApi32.dll");
        if (!lib) {
            LOG_OS_LAST_ERROR(L"LoadLibraryW");
            break;
        }

        // Bind to RegisterServiceCtrlHandlerExW
        typedef DWORD (WINAPI * LPHANDLER_FUNCTION_EX)(DWORD,DWORD,LPVOID,LPVOID);
        typedef SERVICE_STATUS_HANDLE (WINAPI * FRegisterServiceCtrlHandlerExW)(
            LPCWSTR lpServiceName,
            LPHANDLER_FUNCTION_EX lpHandlerProc,
            LPVOID lpContext
        );
        FRegisterServiceCtrlHandlerExW RegisterServiceCtrlHandlerExW;
        * (FARPROC *) &RegisterServiceCtrlHandlerExW = GetProcAddress(lib, "RegisterServiceCtrlHandlerExW");
        if (!RegisterServiceCtrlHandlerExW) {
            LOG_OS_LAST_ERROR(L"GetProcAddress");
            break;
        }
            
        // Register service handler
        handle = RegisterServiceCtrlHandlerExW(s_app->Name(), ServiceHandlerEx, sp);
        if (!handle) {
            LOG_OS_LAST_ERROR(L"RegisterServiceCtrlHandlerExW");
            break;
        }

        // Success!
        break;
    }
    if (lib)
        FreeLibrary(lib);
    return handle;
}


//===================================
static int ApplicationRun (bool serviceMode) {
    // Initialize service parameters
    ServiceParam sp;
    sp.handle = NULL;
    memset(&sp.status, 0, sizeof(sp.status));
    sp.status.dwServiceType         = SERVICE_WIN32_OWN_PROCESS;
    sp.status.dwControlsAccepted    = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

    // Register service application
    if (serviceMode) {
        if (NULL == (sp.handle = RegisterService(&sp)))
            FatalError();
    }

    // Start service
    sp.SetState(SERVICE_START_PENDING);
    if (!serviceMode)
        MainWndInitialize(s_app->Name());
    TaskInitialize();
    ConfigInitialize();
    ConfigMonitorFile(L"Config\\Srv.ini");
    s_app->Start(&sp);

    // Service is running
    sp.SetState(SERVICE_RUNNING);
    s_app->Run(serviceMode);

    // Stop service
    sp.SetState(SERVICE_STOP_PENDING);
    s_app->Stop();
    ConfigDestroy();
    TaskDestroy();
    MainWndDestroy();

    // Service is stopped
    int exitCode = s_app->ExitCode();
    sp.status.dwWin32ExitCode = (DWORD) exitCode;
    sp.SetState(SERVICE_STOPPED);
    return exitCode;
}


//===================================
static void WINAPI ServiceMain (DWORD, LPWSTR *) {
    (void) ApplicationRun(true);
}


//===================================
static int ServiceRun () {
    // Get the application name in a non-const buffer
    const wchar * name  = s_app->Name();
    unsigned chars      = StrChars(name);
    wchar * nameBuf     = (wchar *) _alloca(chars * sizeof(nameBuf[0]));
    StrCopy(nameBuf, chars, name);

    // Start the application
    SERVICE_TABLE_ENTRYW services[] = {
        { nameBuf, ServiceMain },
        { NULL, NULL }
    };

    // If this function fails with error 1063 it means that a user is attempting to start the
    // service from the command line using the "/service" argument; this doesn't work. Services
    // must be started using the service control manager; for example: sc start <ServiceName>
    // (after the service has been installed using <AppName> /install).
    if (!StartServiceCtrlDispatcherW(services)) {
        LOG_OS_LAST_ERROR(L"StartServiceCtrlDispatcherW");
        return 1;
    }

    // Success!
    return s_app->ExitCode();
}


/************************************
*
*   Service install
*
***/


//===================================
static bool ValidateInstallationDrive (const wchar path[]) {
    // Get the root directory of the path including trailing slash
    wchar root[MAX_PATH];
    StrCopy(root, _countof(root), path);
    PathStripToRootW(root);

    // Validate the drive type; services can only run from "FIXED" drives
    UINT type = GetDriveTypeW(root);
    if (type != DRIVE_FIXED) {
        wchar err[128];
        StrPrintf(
            err,
            _countof(err),
            L"Cannot install service on drive type %u",
            type
        );
        LOG_OS_ERROR(err, ERROR_INVALID_DRIVE);
        return false;
    }

    // QueryDosDeviceW doesn't like trailing backslash
    wchar * slash = PathRemoveBackslashW(root);
    if (slash && *slash == '\\')
        *slash = 0;

    // Is this a SUBST drive?
    wchar drive[MAX_PATH];
    if (!QueryDosDeviceW(root, drive, _countof(drive))) {
        LOG_OS_LAST_ERROR(L"QueryDosDeviceW");
        return false;
    }

    // FRAGILE: surely there is a better way to detect a SUBST!
    if (drive[0] == '\\'
    &&  drive[1] == '?'
    &&  drive[2] == '?'
    &&  drive[3] == '\\'
    &&  drive[4] != 0
    &&  drive[5] == ':'
    ) {
        LOG_OS_ERROR(L"Cannot install service on subst drive", ERROR_INVALID_DRIVE);
        return false;
    }

    return true;
}


//===================================
static int ServiceInstall (
    const wchar username[],
    const wchar password[]
) {
    int exitCode        = 1;
    SC_HANDLE hManager  = NULL;
    SC_HANDLE hService  = NULL;
    for (;;) {
        // Find the path to the executable
        wchar path[MAX_PATH];
        PathGetExeFullPath(path, _countof(path));

        // If username is specified, password must be specified
        // If password is specified, username must be specified
        if (username && !username[0])
            username = NULL;
        if (password && !password[0])
            password = NULL;
        if (!username != !password) {
            LOG_OS_ERROR(L"Incorrect username/password usage", ERROR_ACCESS_DENIED);
            break;
        } 

        // Is this a valid installation drive for a service application?
        if (!ValidateInstallationDrive(path))
            break;

        // Open the service control manager
        if (NULL == (hManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE))) {
            LOG_OS_LAST_ERROR(L"OpenSCManager");
            break;
        }

        // Create the service
        wchar cmdLine[MAX_PATH + 100];
        StrPrintf(cmdLine, _countof(cmdLine), L"\"%s\" /service", path);
        const wchar * name = s_app->Name();
        if (NULL == (hService = CreateServiceW(
            hManager,
            name,
            name,
            SERVICE_ALL_ACCESS,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_AUTO_START,
            SERVICE_ERROR_NORMAL,
            cmdLine,
            NULL, // no load ordering group 
            NULL, // no tag identifier 
            NULL, // no dependencies
            username,
            password
        ))) {
            LOG_OS_LAST_ERROR(L"CreateServiceW");
            break;
        }

        // Set service restart time after crash
        SC_ACTION actions[3] = {
            { SC_ACTION_RESTART,  5 * 1000 },
            { SC_ACTION_RESTART, 15 * 1000 },
            { SC_ACTION_RESTART, 30 * 1000 },
        };
        SERVICE_FAILURE_ACTIONSW failureActions;
        failureActions.dwResetPeriod   = 1 * 60 * 60; // 1 hour in seconds
        failureActions.lpRebootMsg     = L"";
        failureActions.lpCommand       = L"";
        failureActions.cActions        = _countof(actions);
        failureActions.lpsaActions     = actions;
        if (!ChangeServiceConfig2W(
            hService,
            SERVICE_CONFIG_FAILURE_ACTIONS,
            &failureActions
        )) {
            LOG_OS_LAST_ERROR(L"ChangeServiceConfig2W");
            break;
        }

        // Set service description
        SERVICE_DESCRIPTIONW descrip;
        descrip.lpDescription = const_cast<LPWSTR>(s_app->Description());
        if (!ChangeServiceConfig2W(
            hService,
            SERVICE_CONFIG_DESCRIPTION,
            &descrip
        )) {
            LOG_OS_LAST_ERROR(L"ChangeServiceConfig2W");
            break;
        }

        // Success!
        exitCode = 0;
        break;
    }
    if (hService)
        CloseServiceHandle(hService);
    if (hManager)
        CloseServiceHandle(hManager);
    return exitCode;
}


/************************************
*
*   Exports
*
***/


//===================================
void ServiceSignalStop () {
    s_app->SignalStop();
}


//===================================
int ServiceMain (CApplication * app) {
    // Service applications run out of system directory;
    // reset to application directory to normalize behavior
    // running as a service and as an application
    s_app = app;
    PathSetProgramDirectory();
    LogInitialize(s_app->Name());
    DebugSetThreadName("Main");

    // Parse the command line
    CmdLineOptions opt(GetCommandLineW());

    // Run the application
    int exitCode = 1;
    switch (opt.m_appMode) {
        case APP_RUN_APPLICATION:
            exitCode = ApplicationRun(false);
        break;

        case APP_RUN_SERVICE:
            exitCode = ServiceRun();
        break;

        case APP_INSTALL_SERVICE:
            exitCode = ServiceInstall(opt.m_username, opt.m_password);
        break;

        case APP_USAGE_SHOW:
            Usage();
        break;

        case APP_FAILED_ERROR:
        break;
    }

    // Complete!
    LogError("Exit code %u\n", exitCode);
    LogDestroy();
    return exitCode;
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
