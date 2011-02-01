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


//************************************************************************
// This is an implementation of POSIX "compatible" condition variables for
// Win32, as described by Douglas C. Schmidt and Irfan Pyarali:
// http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
//************************************************************************

enum
{
    _SPOO_COND_SIGNAL     = 0,
    _SPOO_COND_BROADCAST  = 1
};

typedef struct
{
    // Signal and broadcast event HANDLEs
    HANDLE events[2];

    // Count of the number of waiters
    unsigned int waiterCount;

    // Serialize access to waiterCount
    CRITICAL_SECTION waiterCountLock;

} _SPOOcond;



//////////////////////////////////////////////////////////////////////////
//////                   SPOO internal functions                    //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Set up the Spoo thread, run the user function and clean up
//========================================================================

static DWORD WINAPI runThread(LPVOID lpParam)
{
    _SPOOthread* thread;

    // Get pointer to thread information for current thread
    thread = _spooGetThreadPointer(_spooPlatformGetThreadID());
    if (!thread)
        return 0;

    // Call the user thread function
    thread->function(lpParam);

    // Remove thread from thread list
    ENTER_THREAD_CRITICAL_SECTION
    _spooRemoveThread(thread);
    LEAVE_THREAD_CRITICAL_SECTION

    // When this function returns, the thread dies
    return 0;
}


//////////////////////////////////////////////////////////////////////////
//////                   SPOO platform functions                    //////
//////////////////////////////////////////////////////////////////////////

//========================================================================
// Create a new thread
//========================================================================

SPOOthread _spooPlatformCreateThread(SPOOthreadfun fun, void* arg)
{
    _SPOOthread* thread;
    HANDLE hThread;
    DWORD dwThreadId;

    ENTER_THREAD_CRITICAL_SECTION

    thread = (_SPOOthread*) malloc(sizeof(_SPOOthread));
    if (!thread)
    {
        LEAVE_THREAD_CRITICAL_SECTION
        return SPOO_INVALID_THREAD;
    }

    // Store thread information
    thread->ID = _spooLibrary.nextID++;
    thread->function = fun;

    hThread = CreateThread(NULL,         // Default security attributes
                           0,            // Default stack size (1 MB)
                           runThread,    // Internal thread function
                           (LPVOID) arg, // Argument to thread user function
                           0,            // Default creation flags
                           &dwThreadId); // Returned Windows thread ID

    // Did the thread creation fail?
    if (!hThread)
    {
        free(thread);
        LEAVE_THREAD_CRITICAL_SECTION
        return SPOO_INVALID_THREAD;
    }

    // Store more thread information in the thread list
    thread->windows.handle = hThread;
    thread->windows.ID     = dwThreadId;

    _spooAppendThread(thread);

    LEAVE_THREAD_CRITICAL_SECTION

    return thread->ID;
}


//========================================================================
// Kill a running thread
// NOTE: This is a VERY DANGEROUS operation that should NOT BE USED except
// in EXTREME SITUATIONS!
//========================================================================

void _spooPlatformDestroyThread(SPOOthread ID)
{
    _SPOOthread* thread;

    ENTER_THREAD_CRITICAL_SECTION

    thread = _spooGetThreadPointer(ID);
    if (!thread)
    {
        LEAVE_THREAD_CRITICAL_SECTION
        return;
    }

    // Simply murder the process
    if (TerminateThread(thread->windows.handle, 0))
    {
        CloseHandle(thread->windows.handle);
        _spooRemoveThread(thread);
    }

    LEAVE_THREAD_CRITICAL_SECTION
}


//========================================================================
// Wait for a thread to die
//========================================================================

int _spooPlatformWaitThread(SPOOthread ID, int waitmode)
{
    DWORD result;
    HANDLE hThread;
    _SPOOthread* thread;

    ENTER_THREAD_CRITICAL_SECTION

    thread = _spooGetThreadPointer(ID);

    // Is the thread already dead?
    if (!thread)
    {
        LEAVE_THREAD_CRITICAL_SECTION
        return SPOO_TRUE;
    }

    // Get thread handle
    hThread = thread->handle;

    LEAVE_THREAD_CRITICAL_SECTION

    // Wait for thread to die
    if (waitmode == SPOO_WAIT)
        result = WaitForSingleObject(hThread, INFINITE);
    else if (waitmode == SPOO_NOWAIT)
        result = WaitForSingleObject(hThread, 0);
    else
        return SPOO_FALSE;

    // Did we have a time-out?
    if (result == WAIT_TIMEOUT)
        return SPOO_FALSE;

    return SPOO_TRUE;
}


//========================================================================
// Return the thread ID for the current thread
//========================================================================

SPOOthread _spooPlatformGetThreadID(void)
{
    _SPOOthread* thread
    SPOOthread threadID = SPOO_INVALID_THREAD;
    DWORD windowsID;

    // Get Windows thread ID
    windowsID = GetCurrentThreadId();

    // Seralize access to thread list
    ENTER_THREAD_CRITICAL_SECTION

    for (thread = &_spooLibrary.first;  thread;  thread = thread->next)
    {
        if (thread->windows.ID == windowsID)
        {
            threadID = thread->ID;
            break;
        }
    }

    LEAVE_THREAD_CRITICAL_SECTION

    return threadID;
}


//========================================================================
// Create a mutual exclusion object
//========================================================================

SPOOmutex _spooPlatformCreateMutex(void)
{
    CRITICAL_SECTION* mutex;

    mutex = (CRITICAL_SECTION*) malloc(sizeof(CRITICAL_SECTION));
    if (!mutex)
        return NULL;

    InitializeCriticalSection(mutex);

    return (SPOOmutex) mutex;
}


//========================================================================
// Destroy a mutual exclusion object
//========================================================================

void _spooPlatformDestroyMutex(SPOOmutex mutex)
{
    DeleteCriticalSection((CRITICAL_SECTION*) mutex);
    free(mutex);
}


//========================================================================
// Request access to a mutex
//========================================================================

void _spooPlatformLockMutex(SPOOmutex mutex)
{
    // Wait for mutex to be released
    EnterCriticalSection((CRITICAL_SECTION*) mutex);
}


//========================================================================
// Release a mutex
//========================================================================

void _spooPlatformUnlockMutex(SPOOmutex mutex)
{
    // Release mutex
    LeaveCriticalSection((CRITICAL_SECTION*) mutex);
}


//========================================================================
// Create a new condition variable object
//========================================================================

SPOOcond _spooPlatformCreateCond(void)
{
    _SPOOcond* cond;

    cond = (_SPOOcond *) malloc(sizeof(_SPOOcond));
    if (!cond)
        return NULL;

    // Initialize condition variable
    cond->waiterCount = 0;
    cond->events[_SPOO_COND_SIGNAL] = CreateEvent(NULL, FALSE, FALSE, NULL);
    cond->events[_SPOO_COND_BROADCAST] = CreateEvent(NULL, TRUE, FALSE, NULL);
    InitializeCriticalSection(&cond->waiterCountLock);

    return (SPOOcond) cond;
}


//========================================================================
// Destroy a condition variable object
//========================================================================

void _spooPlatformDestroyCond(SPOOcond handle)
{
    _SPOOcond* cond = (_SPOOcond*) handle;

    // Close the condition variable handles
    CloseHandle(cond->events[_SPOO_COND_SIGNAL]);
    CloseHandle(cond->events[_SPOO_COND_BROADCAST]);

    // Delete critical section
    DeleteCriticalSection(&cond->waiterCountLock);

    // Free memory for condition variable
    free(cond);
}


//========================================================================
// Wait for a condition to be raised
//========================================================================

void _spooPlatformWaitCond(SPOOcond handle, SPOOmutex mutex, double timeout)
{
    _SPOOcond* cond = (_SPOOcond*) handle;
    int result, lastWaiter;
    DWORD timeoutMS;

    // Avoid race conditions
    EnterCriticalSection(&cond->waiters_count_lock);
    cond->waiterCount++;
    LeaveCriticalSection(&cond->waiters_count_lock);

    // It's ok to release the mutex here since Win32 manual-reset events
    // maintain state when used with SetEvent()
    LeaveCriticalSection((CRITICAL_SECTION*) mutex);

    // Translate timeout into milliseconds
    if (timeout >= SPOO_INFINITY)
        timeoutMS = INFINITE;
    else
    {
        timeoutMS = (DWORD) (1000.0 * timeout + 0.5);
        if (timeoutMS <= 0)
            timeoutMS = 1;
    }

    // Wait for either event to become signaled due to spooSignalCond or
    // spooBroadcastCond being called
    result = WaitForMultipleObjects(2, cond->events, FALSE, timeoutMS);

    // Check if we are the last waiter
    EnterCriticalSection(&cv->waiterCountLock);
    cond->waiters_count--;
    last_waiter = (result == WAIT_OBJECT_0 + _SPOO_COND_BROADCAST) &&
                  (cond->waiterCount == 0);
    LeaveCriticalSection(&cond->waiterCountLock);

    // Some thread called spooBroadcastCond
    if (lastWaiter)
    {
        // We're the last waiter to be notified or to stop waiting, so
        // reset the manual event
        ResetEvent(cond->events[_SPOO_COND_BROADCAST]);
    }

    // Reacquire the mutex
    EnterCriticalSection((CRITICAL_SECTION*) mutex);
}


//========================================================================
// Signal a condition to one waiting thread
//========================================================================

void _spooPlatformSignalCond(SPOOcond handle)
{
    int haveWaiters;
    _SPOOcond* cond = (_SPOOcond*) handle;

    // Avoid race conditions
    EnterCriticalSection(&cond->waiterCountLock);
    haveWaiters = cond->waiterCount > 0;
    LeaveCriticalSection(&cond->waiterCountLock);

    if (haveWaiters)
        SetEvent(cond->events[_SPOO_COND_SIGNAL]);
}


//========================================================================
// Broadcast a condition to all waiting threads
//========================================================================

void _spooPlatformBroadcastCond(SPOOcond handle)
{
    int haveWaiters;
    _SPOOcond* cond = (_SPOOcond*) handle;

    // Avoid race conditions
    EnterCriticalSection(&cond->waiterCountLock);
    haveWaiters = cond->waiterCount > 0;
    LeaveCriticalSection(&cond->waiterCountLock);

    if (haveWaiters)
        SetEvent(cond->events[_SPOO_COND_BROADCAST]);
}


//========================================================================
// Return the number of processors in the system
//========================================================================

int _spooPlatformGetCPUCoreCount(void)
{
    SYSTEM_INFO si;

    // Get hardware system information
    GetSystemInfo(&si);

    return (int) si.dwNumberOfProcessors;
}

