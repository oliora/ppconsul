//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#if defined _MSC_VER && _MSC_VER < 1900  // MS Visual Studio before VS2014
    #define PPCONSUL_NO_CXX11_NOEXCEPT

    #if ! defined PPCONSUL_SNPRINTF_DEFINED && ! defined snprintf
        #define PPCONSUL_SNPRINTF_DEFINED
        #define snprintf _snprintf
    #endif
#endif

#if ! defined PPCONSUL_NO_CXX11_NOEXCEPT
    #define PPCONSUL_NOEXCEPT noexcept
#else
    #define PPCONSUL_NOEXCEPT
#endif
