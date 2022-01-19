#  Copyright (c) 2014-2020 Andrey Upadyshev <oliora@gmail.com>
#
#  Use, modification and distribution are subject to the
#  Boost Software License, Version 1.0. (See accompanying file
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

macro(link_test_libs project_name)
    target_include_directories(${project_name} PRIVATE
        ${Boost_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/ext
        ${CMAKE_SOURCE_DIR}/tests
    )

    target_link_libraries(${project_name} ppconsul)
endmacro()

macro(add_catch_test name)
    if (TEST_REPORT_FORMAT)
        add_test(NAME ${name} COMMAND ${name} -r ${TEST_REPORT_FORMAT} -o "${name}.test_out.xml")
    else()
        add_test(NAME ${name} COMMAND ${name})
    endif()
endmacro()

configure_file(consul.json.in consul.json)

add_subdirectory(unittests) # to run unittests before consul ones

add_subdirectory(consul)
add_subdirectory(agent)
add_subdirectory(catalog)
add_subdirectory(coordinate)
add_subdirectory(kv)
add_subdirectory(sessions)
add_subdirectory(status)

add_subdirectory(consul-get)
add_subdirectory(doc_examples)
