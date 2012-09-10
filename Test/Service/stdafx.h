// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Disable errors
#pragma warning(disable:4514)   // unreferenced inline function has been removed
#pragma warning(disable:4530)   // C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
#pragma warning(disable:4710)   // function not inlined
#pragma warning(disable:4820)   // padding added after member
#pragma warning(disable:4986)   // exception specification does not match previous declaration


#include "targetver.h"

// System includes
#pragma warning(push, 1)
#define STRICT
#include <windows.h>
#include <shlwapi.h>
#include <process.h>
#include <stdio.h>
#include <stddef.h>
#include <malloc.h>
#include <stdlib.h>
#include <winbase.h>
#include <tchar.h>
#pragma warning(pop)


// Project includes
#include "../../Base/Base.h"
#include "../../Lib/SrvLib/SrvLib.h"
