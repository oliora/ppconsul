project(test-net)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${JSON11_INCLUDE_DIRS})
include_directories(${CPPNETLIB_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${CPPNETLIB_LIBRARIES})
target_link_libraries(${PROJECT_NAME} json11)
