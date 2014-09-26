project(tests)

include_directories(../include)
include_directories(${JSON11_INCLUDE_DIRS})
include_directories(${CATCH_INCLUDE_DIRS})

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CPPNETLIB_INCLUDE_DIRS})

set(SOURCES
    main.cpp
    http_client.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ppconsul)
target_link_libraries(${PROJECT_NAME} json11)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${CPPNETLIB_LIBRARIES})


add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
