#  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
#
#  Use, modification and distribution are subject to the
#  Boost Software License, Version 1.0. (See accompanying file
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

project(ppconsul)

if (MSVC)
    cmake_minimum_required(VERSION 2.8.12)
else ()
    cmake_minimum_required(VERSION 2.8)
endif ()


if (${CMAKE_CXX_COMPILER_ID} MATCHES GNU)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -ftemplate-depth=256 -std=c++11 -stdlib=libc++")
elseif (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")

    # Turn off MS specific warnings that shown for standard compatible code (mostly for Boost)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)

    # Build for Windows Vista / Windows Server 2008
    add_definitions(-D_WIN32_WINNT=0x0600)

    set(Boost_USE_STATIC_LIBS ON)
    #set(Boost_USE_STATIC_RUNTIME OFF)
endif()

enable_testing()

find_package(Boost 1.55 REQUIRED COMPONENTS thread filesystem system date_time chrono regex)

find_package(cppnetlib 0.11.0 REQUIRED)

set(JSON11_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/ext/json11")
set(CATCH_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/ext/catch")
set(HEADERS_DIR "${CMAKE_SOURCE_DIR}/include/ppconsul")

add_subdirectory(ext/json11)
add_subdirectory(src)
add_subdirectory(tests)
