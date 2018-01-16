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

set(PPCONSUL_MAJOR_VERSION 0)
set(PPCONSUL_MINOR_VERSION 1)

project(ppconsul
VERSION ${PPCONSUL_MAJOR_VERSION}.${PPCONSUL_MINOR_VERSION}
)
add_library(${PROJECT_NAME} STATIC
    ${SOURCES}
    ${LIBB64_SOURCES}
)

source_group(${PROJECT_NAME} FILES ${SOURCES})
source_group(libb64 FILES ${LIBB64_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PROJECT_NAME})

if (NOT WINDOWS OR CYGWIN)
    set(ppconsul_libs -lppconsul)

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/ppconsul.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/ppconsul.pc
        )

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/ppconsul.pc
        DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig"
        )
endif()

install(
    TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )



