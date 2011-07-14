The Spoo threading library
==========================

Spoo is a simple threading library based on the deprecated threading API from
GLFW 2.x.  It was created as a service to people whose projects depend on this
API, as a way to ease their transition to GLFW 3.

You should probably use a library such as TinyThread++ or TinyCThread instead
of Spoo, as they emulate the behavior of the upcoming C++0x and C1X threading
facilities.  That said, Spoo is still a useful library.

Spoo has been verified to work on:

  GCC and Cygwin pthreads
  VS 2008 and Win32 threads
  MinGW and Win32 threads
  GCC and Snow Leopard pthreads
  GCC and Linux pthreads

