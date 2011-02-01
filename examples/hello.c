//========================================================================
// This is a small test application for Spoo
// The program prints "Hello world!", using two threads.
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

