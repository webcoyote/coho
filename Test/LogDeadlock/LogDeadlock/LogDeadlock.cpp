

#include "stdafx.h"


//=============================================================================
static unsigned __stdcall ThreadProc (void * param) {
    bool * shutdown = (bool *) param;
    while (!*shutdown)
        Sleep(1);
    return 0;
}

//=============================================================================
int _tmain(int argc, _TCHAR* argv[]) {
    bool shutdown = false;
    ThreadInit();
    Thread * mainThread = ThreadRegister("main");
    Thread * childThread = ThreadCreate("test", 0, ThreadProc, &shutdown);

    ThreadLogAllThreads();

    shutdown = true;
    ThreadDestroy(childThread); childThread = NULL;
    ThreadUnregister(mainThread); mainThread = NULL;
    ThreadDestroy();

    return 0;
}
