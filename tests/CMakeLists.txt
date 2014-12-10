#  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
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

include_directories(${Boost_INCLUDE_DIRS})

if (${USE_CPPNETLIB})
    include_directories(${CPPNETLIB_INCLUDE_DIRS})
else ()
    include_directories(${CURL_INCLUDE_DIR})
endif ()

add_subdirectory(unittests) # to run unittests before consul ones
add_subdirectory(consul-get)
add_subdirectory(consul-tests)
