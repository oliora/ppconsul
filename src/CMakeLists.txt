include_directories(../include)
include_directories(${JSON11_INCLUDE_DIRS})

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CPPNETLIB_INCLUDE_DIRS})

set(SOURCES
    "${HEADER_DIR}/consul.h"
    "${HEADER_DIR}/http_status.h"
    "${HEADER_DIR}/kv.h"
    "${HEADER_DIR}/parameters.h"
    "${HEADER_DIR}/ppconsul.h"
    consul.cpp
    parameters.cpp
)

project(ppconsul)
add_library(${PROJECT_NAME} ${SOURCES})
