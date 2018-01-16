#  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
#
#  Use, modification and distribution are subject to the
#  Boost Software License, Version 1.0. (See accompanying file
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

project(json11
    VERSION 0.1
    )

add_library(${PROJECT_NAME} STATIC json11.hpp json11.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PROJECT_NAME})

if (NOT WINDOWS OR CYGWIN)
    set(ppconsul_libs -ljson11)

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/json11.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/json11.pc
        )

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/json11.pc
        DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig"
        )
endif()

install(
    TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )
