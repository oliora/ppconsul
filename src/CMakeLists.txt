#  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
#
#  Use, modification and distribution are subject to the
#  Boost Software License, Version 1.0. (See accompanying file
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

include_directories(../include)
include_directories(${JSON11_INCLUDE_DIRS})

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CPPNETLIB_INCLUDE_DIRS})

set(HEADERS
    consul.h
    error.h
    helpers.h
    http_status.h
    json.h
    kv.h
    parameters.h
    ppconsul.h
)

set(SOURCES
    http_client_impl_netlib.h
    consul.cpp
    helpers.cpp
    parameters.cpp
)

foreach(SRC ${HEADERS})
    list(APPEND SOURCES "${HEADERS_DIR}/${SRC}")
endforeach()

project(ppconsul)
add_library(${PROJECT_NAME} ${SOURCES})
