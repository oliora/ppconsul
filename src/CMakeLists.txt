include_directories(../include)
include_directories(${JSON11_INCLUDE_DIRS})

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CPPNETLIB_INCLUDE_DIRS})

set(HEADERS
    consul.h
    helpers.h
    http_status.h
    kv.h
    parameters.h
    ppconsul.h
)

set(SOURCES
    http_client_impl_netlib.h
    consul.cpp
    parameters.cpp
)

foreach(SRC ${HEADERS})
    list(APPEND SOURCES "${HEADERS_DIR}/${SRC}")
endforeach()

project(ppconsul)
add_library(${PROJECT_NAME} ${SOURCES})
