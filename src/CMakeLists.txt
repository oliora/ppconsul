include_directories(../include)
include_directories(${JSON11_INCLUDE_DIRS})

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CPPNETLIB_INCLUDE_DIRS})

set(SOURCES
    ../include/ppconsul/ppconsul.h
    ../include/ppconsul/http_client.h
    http_client.cpp
)

project(ppconsul)
add_library(${PROJECT_NAME} ${SOURCES})

project(test-net)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} ppconsul)
target_link_libraries(${PROJECT_NAME} json11)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${CPPNETLIB_LIBRARIES})


