#  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
#
#  Use, modification and distribution are subject to the
#  Boost Software License, Version 1.0. (See accompanying file
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

project(json11)

add_library(${PROJECT_NAME} STATIC json11.hpp json11.cpp)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_auto_type cxx_decltype cxx_static_assert cxx_rvalue_references)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PROJECT_NAME})

if (BUILD_STATIC_LIB)
    install(
        TARGETS ${PROJECT_NAME}
        EXPORT ${CMAKE_PROJECT_NAME}
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )
endif()
