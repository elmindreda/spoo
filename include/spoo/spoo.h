/************************************************************************
 * Spoo - A threading library
 *------------------------------------------------------------------------
 * This software is based on parts of the GLFW 2.7 library
 *
 * Copyright (c) 2002-2006 Marcus Geelnard
 * Copyright (c) 2006-2011 Camilla Berglund
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would
 *    be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 *************************************************************************/

#ifndef __spoo_h__
#define __spoo_h__

#ifdef __cplusplus
extern "C" {
#endif


/*************************************************************************
 * Global definitions
 *************************************************************************/

/* We need a NULL pointer from time to time */
#ifndef NULL
 #ifdef __cplusplus
  #define NULL 0
 #else
  #define NULL ((void*) 0)
 #endif
#endif /* NULL */


/*************************************************************************
 * API constants
 *************************************************************************/

/* Library version */
#define SPOO_VERSION_MAJOR    0
#define SPOO_VERSION_MINOR    1
#define SPOO_VERSION_REVISION 0

/* Boolean values */
#define SPOO_TRUE                 1
#define SPOO_FALSE                0

/* spooWaitThread wait modes */
#define SPOO_WAIT                 0x00040001
#define SPOO_NOWAIT               0x00040002

/* Time spans longer than this (seconds) are considered to be infinity */
#define SPOO_INFINITY 100000.0

/* The official (but not only) invalid thread ID */
#define SPOO_INVALID_THREAD       (-1)


/*************************************************************************
 * Typedefs
 *************************************************************************/

/* Thread ID */
typedef int SPOOthread;

/* Mutex object */
typedef void* SPOOmutex;

/* Condition variable object */
typedef void* SPOOcond;

/* Function pointer types */
typedef void (*SPOOthreadfun)(void*);


/*************************************************************************
 * Prototypes
 *************************************************************************/

/* Initialization and termination */
int  spooInit(void);
void spooTerminate(void);

/* Time */
double spooGetTime(void);
void spooSetTime(double time);
void spooSleep(double time);

/* Threading support */
SPOOthread spooCreateThread(SPOOthreadfun fun, void* arg);
void spooDestroyThread(SPOOthread ID);
int  spooWaitThread(SPOOthread ID, int waitmode);
SPOOthread spooGetThreadID(void);
SPOOmutex spooCreateMutex(void);
void spooDestroyMutex(SPOOmutex mutex);
void spooLockMutex(SPOOmutex mutex);
void spooUnlockMutex(SPOOmutex mutex);
SPOOcond spooCreateCond(void);
void spooDestroyCond(SPOOcond cond);
void spooWaitCond(SPOOcond cond, SPOOmutex mutex, double timeout);
void spooSignalCond(SPOOcond cond);
void spooBroadcastCond(SPOOcond cond);
int  spooGetCPUCoreCount(void);


#ifdef __cplusplus
}
#endif

#endif /* __spoo_h__ */

