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

#ifndef __spoo_internal_h__
#define __spoo_internal_h__


#include "../include/spoo/spoo.h"

#include "config.h"

#if _SPOO_PTHREADS
 #include "pthreads.h"
#elif _SPOO_WIN32
 #include "win32.h"
#endif


//========================================================================
// Internal types
//========================================================================

//------------------------------------------------------------------------
// Spoo thread state
//------------------------------------------------------------------------

typedef struct _SPOOthread _SPOOthread;

struct _SPOOthread
{
  _SPOOthread*      prev;
  _SPOOthread*      next;
  SPOOthread        ID;
  SPOOthreadfun     function;

  _SPOO_PLATFORM_THREAD_STATE;
};


//------------------------------------------------------------------------
// Spoo library state
//------------------------------------------------------------------------

typedef struct _SPOOlibrary
{
  _SPOOthread       first;
  SPOOthread        nextID;

  _SPOO_PLATFORM_LIBRARY_STATE;
} _SPOOlibrary;


//========================================================================
// Library global state
//========================================================================

extern _SPOOlibrary _spoo;


//========================================================================
// Prototypes for platform-specific functions
//========================================================================

// Init/terminate
int _spooPlatformInit(void);
int _spooPlatformTerminate(void);

// Time
double _spooPlatformGetTime(void);
void _spooPlatformSetTime(double time);
void _spooPlatformSleep(double time);

// Threads
SPOOthread _spooPlatformCreateThread(SPOOthreadfun fun, void* arg);
void _spooPlatformDestroyThread(SPOOthread ID);
int _spooPlatformWaitThread(SPOOthread ID, int waitmode);
SPOOthread _spooPlatformGetThreadID(void);
SPOOmutex _spooPlatformCreateMutex(void);
void _spooPlatformDestroyMutex(SPOOmutex mutex);
void _spooPlatformLockMutex(SPOOmutex mutex);
void _spooPlatformUnlockMutex(SPOOmutex mutex);
SPOOcond _spooPlatformCreateCond(void);
void _spooPlatformDestroyCond(SPOOcond cond);
void _spooPlatformWaitCond(SPOOcond cond, SPOOmutex mutex, double timeout);
void _spooPlatformSignalCond(SPOOcond cond);
void _spooPlatformBroadcastCond(SPOOcond cond);
int _spooPlatformGetCPUCoreCount(void);


//========================================================================
// Prototypes for shared internal functions
//========================================================================

_SPOOthread* _spooGetThreadPointer(int ID);
void _spooAppendThread(_SPOOthread* thread);
void _spooRemoveThread(_SPOOthread* thread);


#endif // __spoo_internal_h__

