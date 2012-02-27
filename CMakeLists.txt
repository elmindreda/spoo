########################################################################
# Spoo - A threading library
#-----------------------------------------------------------------------
# This software is based on parts of the GLFW 2.7 library
#
# Copyright (c) 2002-2006 Marcus Geelnard
# Copyright (c) 2006-2011 Camilla Berglund
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would
#    be appreciated but is not required.
#
# 2. Altered source versions must be plainly marked as such, and must not
#    be misrepresented as being the original software.
#
# 3. This notice may not be removed or altered from any source
#    distribution.
#
########################################################################

project(spoo C)

cmake_minimum_required(VERSION 2.6)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_THREAD_PREFER_PTHREADS 1)
find_package(Threads REQUIRED)

set(spoo_SOURCES ${spoo_SOURCE_DIR}/include/spoo/spoo.h
                 ${spoo_SOURCE_DIR}/src/common.c)

if (CMAKE_USE_WIN32_THREADS_INIT)

  message(STATUS "Building spoo for Win32 threads") 
  set(_SPOO_WIN32 1)

  list(APPEND spoo_SOURCES ${spoo_SOURCE_DIR}/src/win32.c)
  list(APPEND spoo_LIBRARIES winmm)

elseif (CMAKE_USE_PTHREADS_INIT)

  message(STATUS "Building spoo for pthreads") 
  set(_SPOO_PTHREADS 1)

  list(APPEND spoo_SOURCES ${spoo_SOURCE_DIR}/src/pthreads.c)

  if (CMAKE_HAVE_THREADS_LIBRARY)
    list(APPEND spoo_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
  endif (CMAKE_HAVE_THREADS_LIBRARY)

  include(CheckFunctionExists)

  check_function_exists(sched_yield _SPOO_HAS_SCHED_YIELD)
  check_function_exists(sysctl _SPOO_HAS_SYSCTL)
  check_function_exists(sysconf _SPOO_HAS_SYSCONF)

endif (CMAKE_USE_WIN32_THREADS_INIT)

set(SPOO_LIBRARIES ${spoo_LIBRARIES} CACHE STRING "Depdendencies of the Spoo library")
set(SPOO_INCLUDE_DIR ${spoo_SOURCE_DIR}/include CACHE STRING "Public include directory of the Spoo library")

configure_file(${spoo_SOURCE_DIR}/src/config.h.in 
               ${spoo_BINARY_DIR}/src/config.h @ONLY)

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(tests)

