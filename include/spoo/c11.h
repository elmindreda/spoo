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

#ifndef spoo_c11_h
#define spoo_c11_h

#ifdef __cplusplus
extern "C" {
#endif


/* Thread */
typedef int thrd_t;

/* Condition variable */
typedef void* cnd_t;

/* Mutex */
typedef void* mtx_t;

/* Thread constructor */
typedef void (*thrd_start_t)(void*);

/* Thread destructor */
typedef void (*thrd_dtor_t)(void*);

/* Time */
struct xtime
{
    time_t sec;
    long nsec;
};


#ifdef __cplusplus
}
#endif

#endif /* spoo_h */

