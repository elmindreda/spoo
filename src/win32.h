//========================================================================
// Spoo - A threading library
//------------------------------------------------------------------------
// This software is based on parts of the GLFW 2.7 library
//
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2011 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#ifndef __spoo_windows_h__
#define __spoo_windows_h__

// We don't need all the fancy stuff
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#include <windows.h>
#include <mmsystem.h>
#include "../../include/GL/glfw.h"


//========================================================================
// Spoo platform-specific macros
//========================================================================

//------------------------------------------------------------------------
// Macros for encapsulating critical code sections (i.e. making parts
// of GLFW thread safe)
//------------------------------------------------------------------------

#define ENTER_THREAD_CRITICAL_SECTION \
        EnterCriticalSection( &_glfwThrd.CriticalSection );
#define LEAVE_THREAD_CRITICAL_SECTION \
        LeaveCriticalSection( &_glfwThrd.CriticalSection );


//========================================================================
// Spoo platform-specific types
//========================================================================

#define _SPOO_PLATFORM_THREAD_STATE  _SPOOthreadWINDOWS windows
#define _SPOO_PLATFORM_LIBRARY_STATE _SPOOlibraryWINDOWS windows

//------------------------------------------------------------------------
// Platform-specific Spoo thread state
//------------------------------------------------------------------------

typedef struct
{
    HANDLE        handle;
    DWORD         ID;

} _SPOOthreadWINDOWS;


//------------------------------------------------------------------------
// Platform-specific Spoo library state
//------------------------------------------------------------------------
typedef struct
{
    int                 hasPerformanceCounter;
    double              timerRes;
    unsigned int        baseTime32;
    __int64             baseTime64;

    CRITICAL_SECTION    criticalSection;

} _SPOOlibraryWINDOWS;


#endif /*__spoo_windows_h__*/

