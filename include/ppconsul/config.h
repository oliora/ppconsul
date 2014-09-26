#pragma once

#if defined _MSC_VER && _MSC_VER < 1900  // MS Visual Studio before VS2014
    #define PPCONSUL_NO_CXX11_NOEXCEPT
#endif

#if ! defined PPCONSUL_NO_CXX11_NOEXCEPT
    #define PPCONSUL_NOEXCEPT noexcept
#else
    #define PPCONSUL_NOEXCEPT
#endif
