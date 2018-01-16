#pragma once

#if defined PPCONSUL_USE_CPPNETLIB
#include "netlib/http_client.h"
#else
#include "curl/http_client.h"
#endif

namespace ppconsul { namespace impl {
#if defined PPCONSUL_USE_CPPNETLIB
    using netlib::HttpClient;
#else
    using curl::HttpClient;
#endif
}}