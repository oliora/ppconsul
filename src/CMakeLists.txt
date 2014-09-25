project(test-net)

include_directories(../include)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${JSON11_INCLUDE_DIRS})
include_directories(${CPPNETLIB_INCLUDE_DIRS})

set(SOURCES
    ../include/ppconsul/ppconsul.h
    main.cpp
    http_client.h
    http_client.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${CPPNETLIB_LIBRARIES})
target_link_libraries(${PROJECT_NAME} json11)

