#  Copyright (c) 2014-2020 Andrey Upadyshev <oliora@gmail.com>
#
#  Use, modification and distribution are subject to the
#  Boost Software License, Version 1.0. (See accompanying file
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

project(ppconsul)

set(HEADERS
    agent.h
    catalog.h
    consul.h
    coordinate.h
    error.h
    health.h
    helpers.h
    client_pool.h
    kv.h
    kwargs.h
    parameters.h
    ppconsul.h
    response.h
    sessions.h
    types.h
    http/http_client.h
    http/status.h
)

set(SOURCES
    http_helpers.h
    s11n.h
    s11n_types.h
    agent.cpp
    catalog.cpp
    consul.cpp
    coordinate.cpp
    helpers.cpp
    health.cpp
    client_pool.cpp
    kv.cpp
    sessions.cpp
    status.cpp
)

list(APPEND SOURCES "curl/http_client.h")
list(APPEND SOURCES "curl/http_client.cpp")

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
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<BUILD_INTERFACE:${Boost_INCLUDE_DIRS}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/../ext
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        json11
        ${Boost_LIBRARIES}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${CURL_LIBRARIES})

source_group(${PROJECT_NAME} FILES ${SOURCES})
source_group(libb64 FILES ${LIBB64_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${Ppconsul_VERSION}
    SOVERSION "${Ppconsul_VERSION_MAJOR}.${Ppconsul_VERSION_MINOR}"
    COMPILE_PDB_NAME ${PROJECT_NAME}
    COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    FOLDER ${PROJECT_NAME}
)

install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${CMAKE_PROJECT_NAME}
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
if(WIN32)
    install(
        FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.pdb
        DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        OPTIONAL
    )
endif()

