#  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
#
#  Use, modification and distribution are subject to the
#  Boost Software License, Version 1.0. (See accompanying file
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.1)

# Should be uncommented for external project
#find_package(ppconsul REQUIRED)

project(doc_examples)
add_executable(${PROJECT_NAME}
    agent.cpp
    kv.cpp
    main.cpp
)

target_link_libraries(${PROJECT_NAME} ppconsul)
