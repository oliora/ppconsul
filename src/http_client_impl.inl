//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#if defined PPCONSUL_USE_CPPNETLIB
    #include "http_client_impl_netlib.inl"
#else
    #include "http_client_impl_curl.inl"
#endif
