//========================================================================
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
// This is a small test application for Spoo
// It prints the string "Hello world!" using two threads
//========================================================================

#include <spoo/spoo.h>

#include <stdio.h>
#include <stdlib.h>


void hello_function(void* arg)
{
    // Print the first part of the message
    printf("Hello ");
}

int main(void)
{
    SPOOthread thread;

    if (!spooInit())
    {
        fprintf(stderr, "Failed to initialize Spoo\n");
        exit(EXIT_FAILURE);
    }

    thread = spooCreateThread(hello_function, NULL);

    // Wait for the thread to terminate
    spooWaitThread(thread, SPOO_WAIT);

    // Print the rest of the message
    printf("world!\n");

    spooTerminate();
    exit(EXIT_SUCCESS);
}

