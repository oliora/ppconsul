#  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
#
#  Use, modification and distribution are subject to the
#  Boost Software License, Version 1.0. (See accompanying file
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

project(json11
	VERSION 0.1)

add_library(${PROJECT_NAME} STATIC json11.hpp json11.cpp)

# Library installation directory
if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR lib)
endif(NOT DEFINED CMAKE_INSTALL_LIBDIR)
set(libdir ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})

set_target_properties(json11 PROPERTIES DEBUG_POSTFIX -d FOLDER json11)
set(install_libs json11)
set_property(TARGET json11 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

if (NOT WINDOWS OR CYGWIN)
    set(ppconsul_libs -ljson11)

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/json11.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/json11.pc
        )

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/json11.pc
        DESTINATION "${libdir}/pkgconfig"
        )
endif()

install(
    TARGETS ${install_libs}
    LIBRARY DESTINATION "${libdir}"
    ARCHIVE DESTINATION "${libdir}"
    RUNTIME DESTINATION "bin"
    )
