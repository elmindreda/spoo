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

#include "internal.h"

#include <stdlib.h>
#include <string.h>


//========================================================================
// The library initialization state
//========================================================================

static int initialized = SPOO_FALSE;


//========================================================================
// Library global state
//========================================================================

_SPOOlibrary _spooLibrary;


//////////////////////////////////////////////////////////////////////////
//////                   Spoo internal functions                    //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Find pointer to thread with a matching ID
// NOTE: This function is not thread safe and calls to it must be serialized
//========================================================================

_SPOOthread* _spooGetThreadPointer(int threadID)
{
    _SPOOthread* thread;

    for (thread = &_spooLibrary.first;  thread;  thread = thread->next)
    {
        if (thread->ID == threadID)
            break;
    }

    return thread;
}


//========================================================================
// Append thread to thread list
// NOTE: This function is not thread safe and calls to it must be serialized
//========================================================================

void _spooAppendThread(_SPOOthread* thread)
{
    _SPOOthread* last;

    last = &_spooLibrary.first;
    while (last->next)
        last = last->next;

    last->next = thread;
    thread->prev = last;
    thread->next = NULL;
}


//========================================================================
// Remove thread from thread list
// NOTE: This function is not thread safe and calls to it must be serialized
//========================================================================

void _spooRemoveThread(_SPOOthread* thread)
{
    if (thread->prev)
        thread->prev->next = thread->next;

    if (thread->next)
        thread->next->prev = thread->prev;

    free(thread);
}



//////////////////////////////////////////////////////////////////////////
//////                      Spoo user functions                     //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Initialize the library
//========================================================================

int spooInit(void)
{
    if (initialized)
        return SPOO_TRUE;

    memset(&_spooLibrary, 0, sizeof(_SPOOlibrary));

    if (!_spooPlatformInit())
        return SPOO_FALSE;

    atexit(spooTerminate);

    initialized = SPOO_TRUE;
    return SPOO_TRUE;
}


//========================================================================
// Kill all threads and terminate library
//========================================================================

void spooTerminate(void)
{
    if (!initialized)
        return;

    if (!_spooPlatformTerminate())
        return;

    initialized = SPOO_FALSE;
}


//========================================================================
// Return timer value in seconds
//========================================================================

double spooGetTime(void)
{
    if (!initialized)
        return 0.0;

    return _spooPlatformGetTime();
}


//========================================================================
// Set timer value in seconds
//========================================================================

void spooSetTime(double time)
{
    if (!initialized)
        return;

    _spooPlatformSetTime(time);
}


//========================================================================
// Create a new thread
//========================================================================

SPOOthread spooCreateThread(SPOOthreadfun fun, void* arg)
{
    if (!initialized)
        return SPOO_INVALID_THREAD;

    return _spooPlatformCreateThread(fun, arg);
}


//========================================================================
// Put the current thread to sleep for the specified amount of time
//========================================================================

void spooSleep(double time)
{
    if (!initialized)
        return;

    _spooPlatformSleep(time);
}


//========================================================================
// Kill the specified thread
// NOTE: This is a VERY DANGEROUS operation that should NOT BE USED except
// in EXTREME SITUATIONS!
//========================================================================

void spooDestroyThread(SPOOthread threadID)
{
    if (!initialized)
        return;

    // Is it a valid thread? (killing the main thread is not allowed)
    if (threadID < 1)
        return;

    _spooPlatformDestroyThread(threadID);
}


//========================================================================
// Wait for a thread to die
//========================================================================

int spooWaitThread(SPOOthread threadID, int waitmode)
{
    if (!initialized)
        return SPOO_TRUE;

    // Is it a valid thread? (waiting for the main thread is not allowed)
    if (threadID < 1)
        return SPOO_TRUE;

    return _spooPlatformWaitThread(threadID, waitmode);
}


//========================================================================
// Return the thread ID for the current thread
//========================================================================

SPOOthread spooGetThreadID(void)
{
    if (!initialized)
        return (SPOOthread) 0;

    return _spooPlatformGetThreadID();
}


//========================================================================
// Create a mutual exclusion object
//========================================================================

SPOOmutex spooCreateMutex(void)
{
    if (!initialized)
        return (SPOOmutex) 0;

    return _spooPlatformCreateMutex();
}


//========================================================================
// Destroy a mutual exclusion object
//========================================================================

void spooDestroyMutex(SPOOmutex mutex)
{
    if (!initialized || !mutex)
        return;

    _spooPlatformDestroyMutex(mutex);
}


//========================================================================
// Request access to a mutex
//========================================================================

void spooLockMutex(SPOOmutex mutex)
{
    if (!initialized && !mutex)
        return;

    _spooPlatformLockMutex(mutex);
}


//========================================================================
// Release a mutex
//========================================================================

void spooUnlockMutex(SPOOmutex mutex)
{
    if (!initialized && !mutex)
        return;

    _spooPlatformUnlockMutex(mutex);
}


//========================================================================
// Create a new condition variable object
//========================================================================

SPOOcond spooCreateCond(void)
{
    if (!initialized)
        return (SPOOcond) 0;

    return _spooPlatformCreateCond();
}


//========================================================================
// Destroy a condition variable object
//========================================================================

void spooDestroyCond(SPOOcond cond)
{
    if (!initialized || !cond)
        return;

    _spooPlatformDestroyCond(cond);
}


//========================================================================
// Wait for a condition to be raised
//========================================================================

void spooWaitCond(SPOOcond cond, SPOOmutex mutex, double timeout)
{
    if (!initialized || !cond || !mutex)
        return;

    _spooPlatformWaitCond(cond, mutex, timeout);
}


//========================================================================
// Signal a condition to one waiting thread
//========================================================================

void spooSignalCond(SPOOcond cond)
{
    if (!initialized || !cond)
        return;

    _spooPlatformSignalCond(cond);
}


//========================================================================
// Broadcast a condition to all waiting threads
//========================================================================

void spooBroadcastCond(SPOOcond cond)
{
    if (!initialized || !cond)
        return;

    _spooPlatformBroadcastCond(cond);
}


//========================================================================
// Return the number of CPU cores in the system
// This information can be useful for determining the optimal number of
// threads to use for performing certain tasks
//========================================================================

int spooGetCPUCoreCount(void)
{
    if (!initialized)
        return 0;

    return _spooPlatformGetCPUCoreCount();
}

