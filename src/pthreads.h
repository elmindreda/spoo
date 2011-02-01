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

// We support two different ways for getting the number of processors in
// the system: sysconf (POSIX) and sysctl (BSD?)
#if defined(_SPOO_HAS_SYSCONF)

 // Use a single constant for querying number of online processors using
 // the sysconf function (e.g. SGI defines _SC_NPROC_ONLN instead of
 // _SC_NPROCESSORS_ONLN)
 #ifndef _SC_NPROCESSORS_ONLN
  #ifdef  _SC_NPROC_ONLN
   #define _SC_NPROCESSORS_ONLN _SC_NPROC_ONLN
  #else
   #error POSIX constant _SC_NPROCESSORS_ONLN not defined!
  #endif
 #endif

 // Macro for querying the number of processors
 #define _spoo_numprocessors(n) n=(int)sysconf(_SC_NPROCESSORS_ONLN)

#elif defined(_SPOO_HAS_SYSCTL)

 #include <sys/types.h>
 #include <sys/sysctl.h>

 // Macro for querying the number of processors
 #define _spoo_numprocessors(n) { \
    int mib[2], ncpu; \
    size_t len = 1; \
    mib[0] = CTL_HW; \
    mib[1] = HW_NCPU; \
    n      = 1; \
    if( sysctl( mib, 2, &ncpu, &len, NULL, 0 ) != -1 ) \
    { \
        if( len > 0 ) \
        { \
            n = ncpu; \
        } \
    } \
 }

#else

 // If neither sysconf nor sysctl is supported, assume single processor
 // system
 #define _spoo_numprocessors(n) n=1

#endif

// Critical section guard macros
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

