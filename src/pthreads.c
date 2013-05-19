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

#if defined(_SPOO_HAS_SCHED_YIELD)
#include <sched.h>
#endif /*_SPOO_HAS_SCHED_YIELD*/

#if defined(_SPOO_HAS_SYSCONF)

#include <unistd.h>

// Use a single constant for querying number of online processors using
// the sysconf function (e.g. SGI defines _SC_NPROC_ONLN instead of
// _SC_NPROCESSORS_ONLN)
//
#ifndef _SC_NPROCESSORS_ONLN
 #ifdef  _SC_NPROC_ONLN
  #define _SC_NPROCESSORS_ONLN _SC_NPROC_ONLN
 #else
  #error POSIX constant _SC_NPROCESSORS_ONLN not defined!
 #endif
#endif

#endif /*_SPOO_HAS_SYSCONF*/

#if defined(_SPOO_HAS_SYSCTL)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif /*_SPOO_HAS_SYSCTL*/

#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>

// Macros for encapsulating critical code sections (i.e. making parts
// of Spoo thread safe)
//
#define ENTER_THREAD_CRITICAL_SECTION \
        pthread_mutex_lock(&_spoo.posix.criticalSection)
#define LEAVE_THREAD_CRITICAL_SECTION \
        pthread_mutex_unlock(&_spoo.posix.criticalSection)


//////////////////////////////////////////////////////////////////////////
//////                   Spoo internal functions                    //////
//////////////////////////////////////////////////////////////////////////

// Set up the Spoo thread, run the user function and clean up
//
static void* runThread(void* arg)
{
    SPOOthreadfun threadfun;
    _SPOOthread* thread;

    // Get pointer to thread information for current thread
    thread = _spooGetThreadPointer(spooGetThreadID());
    if (!thread)
        return NULL;

    // Call the user thread function
    thread->function(arg);

    // Remove thread from thread list
    ENTER_THREAD_CRITICAL_SECTION;
    _spooRemoveThread(thread);
    LEAVE_THREAD_CRITICAL_SECTION;

    // When this function returns, the thread dies
    return NULL;
}

// Set up a timespec struct to a time duration seconds after now
//
static void makeWaitTime(struct timespec* result, double duration)
{
    struct timeval tv;
    long dt_sec, dt_usec;

    gettimeofday(&tv, NULL);
    dt_sec  = (long) duration;
    dt_usec = (long) ((duration - (double) dt_sec) * 1000000.0);

    result->tv_nsec = (tv.tv_usec + dt_usec) * 1000L;
    if (result->tv_nsec > 1000000000L)
    {
        result->tv_nsec -= 1000000000L;
        dt_sec++;
    }

    result->tv_sec = tv.tv_sec + dt_sec;
}

// Returns the current raw time
//
long long getCurrentRawTime(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (long long) tv.tv_sec * (long long) 1000000 +
           (long long) tv.tv_usec;
}


//////////////////////////////////////////////////////////////////////////
//////                   Spoo platform functions                    //////
//////////////////////////////////////////////////////////////////////////

// Initialize library
//
int _spooPlatformInit(void)
{
    // "Resolution" is 1 us
    _spoo.posix.timerRes = 1e-6;

    // Set start time for timer
    _spoo.posix.baseTime = getCurrentRawTime();

    pthread_mutex_init(&_spoo.posix.criticalSection, NULL);

    // The first thread (the main thread) has ID 0
    _spoo.nextID = 0;

    // Fill out information about the main thread (this thread)
    _spoo.first.ID       = _spoo.nextID++;
    _spoo.first.function = NULL;
    _spoo.first.prev     = NULL;
    _spoo.first.next     = NULL;
    _spoo.first.posix.ID = pthread_self();

    return SPOO_TRUE;
}

// Kill all threads and terminate library
//
int _spooPlatformTerminate(void)
{
    _SPOOthread* thread;

    // Only the main thread is allowed to do this
    if (pthread_self() != _spoo.first.posix.ID)
        return SPOO_FALSE;

    ENTER_THREAD_CRITICAL_SECTION;

    // Kill all remaining threads created by Spoo
    // NOTE: The user should wait for all threads to die BEFORE calling
    // spooTerminate.  Any work we need to do here is really an error.
    while (thread = _spoo.first.next)
        _spooPlatformDestroyThread(thread->ID);

    LEAVE_THREAD_CRITICAL_SECTION;

    // Delete critical section handle
    pthread_mutex_destroy(&_spoo.posix.criticalSection);

    return SPOO_TRUE;
}

// Return timer value in seconds
//
double _spooPlatformGetTime(void)
{
    long long time = getCurrentRawTime() - _spoo.posix.baseTime;

    return (double) time * _spoo.posix.timerRes;
}

// Set timer value in seconds
//
void _spooPlatformSetTime(double time)
{
    long long offset = (long long) (time / _spoo.posix.timerRes);

    _spoo.posix.baseTime = getCurrentRawTime() - offset;
}

// Put the current thread to sleep for the specified amount of time
//
void _spooPlatformSleep(double time)
{
    struct timespec wait;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    if (time == 0.0)
    {
#ifdef _SPOO_HAS_SCHED_YIELD
        sched_yield();
#endif
        return;
    }

    // Not all pthread implementations have a pthread_sleep() function. We
    // do it the portable way, using a timed wait for a condition that we
    // will never signal. NOTE: The unistd functions sleep/usleep suspends
    // the entire PROCESS, not a signle thread, which is why we can not
    // use them to implement spooSleep.

    // Set timeout time, relative to current time
    makeWaitTime(&wait, time);

    // Initialize condition and mutex objects
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    // Do a timed wait
    pthread_mutex_lock(&mutex);
    pthread_cond_timedwait(&cond, &mutex, &wait);
    pthread_mutex_unlock(&mutex);

    // Destroy condition and mutex objects
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

// Create a new thread
//
SPOOthread _spooPlatformCreateThread(SPOOthreadfun fun, void* arg)
{
    _SPOOthread* thread;

    ENTER_THREAD_CRITICAL_SECTION;

    // Create a new thread information memory area
    thread = (_SPOOthread*) malloc(sizeof(_SPOOthread));
    if (!thread)
    {
        LEAVE_THREAD_CRITICAL_SECTION;
        return SPOO_INVALID_THREAD;
    }

    // Store thread information
    thread->ID = _spoo.nextID++;
    thread->function = fun;

    // Did the thread creation fail?
    if (pthread_create(&thread->posix.ID, // POSIX thread handle
                       NULL,              // Default thread attributes
                       runThread,         // Internal thread function
                       arg) != 0)         // Argument to thread user function
    {
        free(thread);
        LEAVE_THREAD_CRITICAL_SECTION;
        return SPOO_INVALID_THREAD;
    }

    _spooAppendThread(thread);

    LEAVE_THREAD_CRITICAL_SECTION;

    return thread->ID;
}

// Kill a running thread
// NOTE: This is a VERY DANGEROUS operation that should NOT BE USED except
// in EXTREME SITUATIONS!
//
void _spooPlatformDestroyThread(SPOOthread ID)
{
    _SPOOthread* thread;

    ENTER_THREAD_CRITICAL_SECTION;

    thread = _spooGetThreadPointer(ID);
    if (!thread)
    {
        LEAVE_THREAD_CRITICAL_SECTION;
        return;
    }

    // Simply murder the process
    pthread_kill(thread->posix.ID, SIGKILL);

    // Remove thread from thread list
    _spooRemoveThread(thread);

    LEAVE_THREAD_CRITICAL_SECTION;
}

// Wait for a thread to die
//
int _spooPlatformWaitThread(SPOOthread ID, int waitmode)
{
    _SPOOthread* thread;

    ENTER_THREAD_CRITICAL_SECTION;

    thread = _spooGetThreadPointer(ID);

    // Is the thread already dead?
    if (!thread)
    {
        LEAVE_THREAD_CRITICAL_SECTION;
        return SPOO_TRUE;
    }

    // If got this far, the thread is alive => polling returns FALSE
    if (waitmode == SPOO_NOWAIT)
    {
        LEAVE_THREAD_CRITICAL_SECTION;
        return SPOO_FALSE;
    }

    LEAVE_THREAD_CRITICAL_SECTION;

    // Wait for thread to die
    pthread_join(thread->posix.ID, NULL);

    return SPOO_TRUE;
}

// Return the thread ID for the current thread
//
SPOOthread _spooPlatformGetThreadID(void)
{
    _SPOOthread* thread;
    SPOOthread threadID = SPOO_INVALID_THREAD;
    pthread_t posixID;

    // Get current POSIX thread ID
    posixID = pthread_self();

    ENTER_THREAD_CRITICAL_SECTION;

    // Loop through entire list of threads to find the matching POSIX
    // thread ID
    for (thread = &_spoo.first;  thread;  thread = thread->next)
    {
        if (thread->posix.ID == posixID)
        {
            threadID = thread->ID;
            break;
        }
    }

    LEAVE_THREAD_CRITICAL_SECTION;

    return threadID;
}

// Create a mutual exclusion object
//
SPOOmutex _spooPlatformCreateMutex(void)
{
    pthread_mutex_t* mutex;

    mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    if (!mutex)
        return NULL;

    pthread_mutex_init(mutex, NULL);

    return (SPOOmutex) mutex;
}

// Destroy a mutual exclusion object
//
void _spooPlatformDestroyMutex(SPOOmutex mutex)
{
    pthread_mutex_destroy((pthread_mutex_t*) mutex);

    free(mutex);
}

// Request access to a mutex
//
void _spooPlatformLockMutex(SPOOmutex mutex)
{
    pthread_mutex_lock((pthread_mutex_t*) mutex);
}

// Release a mutex
//
void _spooPlatformUnlockMutex(SPOOmutex mutex)
{
    pthread_mutex_unlock((pthread_mutex_t*) mutex);
}

// Create a new condition variable object
//
SPOOcond _spooPlatformCreateCond(void)
{
    pthread_cond_t* cond;

    cond = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
    if (!cond)
        return NULL;

    pthread_cond_init(cond, NULL);

    return (SPOOcond) cond;
}

// Destroy a condition variable object
//
void _spooPlatformDestroyCond(SPOOcond cond)
{
    pthread_cond_destroy((pthread_cond_t*) cond);

    free(cond);
}

// Wait for a condition to be raised
//
void _spooPlatformWaitCond(SPOOcond cond, SPOOmutex mutex, double timeout)
{
    struct timespec wait;

    // Select infinite or timed wait
    if (timeout >= SPOO_INFINITY)
    {
        // Wait indefinitely for condition
        pthread_cond_wait((pthread_cond_t*) cond, (pthread_mutex_t*) mutex);
    }
    else
    {
        // Set timeout time, relative to current time
        makeWaitTime(&wait, timeout);

        // Wait for condition, with timeout
        pthread_cond_timedwait((pthread_cond_t*) cond,
                               (pthread_mutex_t*) mutex,
                               &wait);
    }
}

// Signal a condition to one waiting thread
//
void _spooPlatformSignalCond(SPOOcond cond)
{
    pthread_cond_signal((pthread_cond_t*) cond);
}

// Broadcast a condition to all waiting threads
//
void _spooPlatformBroadcastCond(SPOOcond cond)
{
    pthread_cond_broadcast((pthread_cond_t*) cond);
}

// Return the number of processors in the system
//
int _spooPlatformGetCPUCoreCount(void)
{
    int count;
#if defined(_SPOO_HAS_SYSCTL)
    int result, mib[2];
    size_t length;
#endif /*_SPOO_HAS_SYSCTL*/

#if defined(_SPOO_HAS_SYSCONF)
    count = (int) sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(_SPOO_HAS_SYSCTL)
    mib[0] = CTL_HW;
    mib[1] = HW_NCPU;

    count = 1;
    length = 1;

    if (sysctl(mib, 2, &result, &length, NULL, 0) != -1)
    {
        if (length > 0 )
            count = result;
    }
#else /*_SPOO_HAS_SYSCONF*/
    count = 1;
#endif /*_SPOO_HAS_SYSCONF*/

    return count;
}

