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

#ifndef __spoo_pthreads_h__
#define __spoo_pthreads_h__

#include <pthread.h>


//========================================================================
// Spoo platform-specific macros
//========================================================================

//------------------------------------------------------------------------
// Macros for encapsulating critical code sections (i.e. making parts
// of Spoo thread safe)
//------------------------------------------------------------------------

#define ENTER_THREAD_CRITICAL_SECTION \
        pthread_mutex_lock(&_spooLibrary.posix.criticalSection);
#define LEAVE_THREAD_CRITICAL_SECTION \
        pthread_mutex_unlock(&_spooLibrary.posix.criticalSection);


//========================================================================
// Spoo platform-specific types
//========================================================================

#define _SPOO_PLATFORM_THREAD_STATE  _SPOOthreadPOSIX posix
#define _SPOO_PLATFORM_LIBRARY_STATE _SPOOlibraryPOSIX posix

//------------------------------------------------------------------------
// Platform-specific Spoo thread state
//------------------------------------------------------------------------

typedef struct
{
    pthread_t ID;

} _SPOOthreadPOSIX;


//------------------------------------------------------------------------
// Platform-specific Spoo library state
//------------------------------------------------------------------------

typedef struct
{
    pthread_mutex_t     criticalSection;

    long long           timerRes;
    double              baseTime;

} _SPOOlibraryPOSIX;


#endif /*__spoo_pthreads_h__*/

