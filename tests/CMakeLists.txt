#  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
#
#  Use, modification and distribution are subject to the
#  Boost Software License, Version 1.0. (See accompanying file
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

macro(link_test_libs project_name)
    target_link_libraries(${project_name} ppconsul)
    target_link_libraries(${project_name} json11)
    target_link_libraries(${project_name} ${Boost_LIBRARIES})

    if (${USE_CPPNETLIB})
        target_link_libraries(${project_name} ${CPPNETLIB_LIBRARIES})
    else ()
        target_link_libraries(${project_name} ${CURL_LIBRARIES})
    endif ()
endmacro()

macro(add_catch_test name)
    if (TEST_REPORT_FORMAT)
        add_test(NAME ${name} COMMAND ${name} -r ${TEST_REPORT_FORMAT} -o "${name}.test_out.xml")
    else()
        add_test(NAME ${name} COMMAND ${name})
    endif()
endmacro()

include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${Boost_INCLUDE_DIRS})

if (${USE_CPPNETLIB})
    include_directories(${CPPNETLIB_INCLUDE_DIRS})
else ()
    include_directories(${CURL_INCLUDE_DIR})
endif ()

add_subdirectory(unittests) # to run unittests before consul ones

add_subdirectory(agent)
add_subdirectory(catalog)
add_subdirectory(kv)

add_subdirectory(consul-get)
add_subdirectory(doc_examples)
