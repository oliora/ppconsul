#  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
#
#  Use, modification and distribution are subject to the
#  Boost Software License, Version 1.0. (See accompanying file
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(HEADERS
    agent.h
    catalog.h
    consul.h
    error.h
    health.h
    helpers.h
    http_client.h
    http_status.h
    kv.h
    kwargs.h
    parameters.h
    ppconsul.h
    response.h
    types.h
)

set(SOURCES
    http_helpers.h
    s11n.h
    s11n_types.h
    all_clients.h
    agent.cpp
    catalog.cpp
    consul.cpp
    helpers.cpp
    health.cpp
    kv.cpp
)

if (${USE_CPPNETLIB})
    list(APPEND SOURCES "netlib/http_client.h")
    list(APPEND SOURCES "netlib/http_client.cpp")
else ()
    list(APPEND SOURCES "curl/http_client.h")
    list(APPEND SOURCES "curl/http_client.cpp")
endif ()

foreach(SRC ${HEADERS})
    list(APPEND SOURCES "${HEADERS_DIR}/${SRC}")
endforeach()

include_directories(${Boost_INCLUDE_DIRS})

if (${USE_CPPNETLIB})
    include_directories(${CPPNETLIB_INCLUDE_DIRS})
else ()
    include_directories(${CURL_INCLUDE_DIR})
endif ()

project(ppconsul)

add_library(${PROJECT_NAME} SHARED
    ${SOURCES}
    ${LIBB64_SOURCES}
)

source_group(ppconsul FILES ${SOURCES})
source_group(libb64 FILES ${LIBB64_SOURCES})
