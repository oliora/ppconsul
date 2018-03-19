#  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
#
#  Use, modification and distribution are subject to the
#  Boost Software License, Version 1.0. (See accompanying file
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

project(ppconsul)

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
    status.cpp
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

if (BUILD_STATIC_LIB)
    add_library(${PROJECT_NAME} STATIC
        ${SOURCES}
        ${LIBB64_SOURCES}
    )
else()
    add_library(${PROJECT_NAME} SHARED
        ${SOURCES}
        ${LIBB64_SOURCES}
    )
endif()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_auto_type cxx_decltype cxx_static_assert cxx_rvalue_references)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${Boost_INCLUDE_DIRS}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/ext
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        json11
        ${Boost_LIBRARIES}
)

if (${USE_CPPNETLIB})
    target_include_directories(${PROJECT_NAME} PRIVATE ${CPPNETLIB_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CPPNETLIB_LIBRARIES})
else ()
    target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CURL_LIBRARIES})
endif ()

source_group(${PROJECT_NAME} FILES ${SOURCES})
source_group(libb64 FILES ${LIBB64_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${Ppconsul_VERSION}
    SOVERSION ${Ppconsul_VERSION}
    FOLDER ${PROJECT_NAME}
)

install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${CMAKE_PROJECT_NAME}
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

