//  Copyright (c)  2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/http_status.h"
#include "ppconsul/response.h"
#include <tuple>
#include <string>


namespace ppconsul { namespace http { namespace impl {

    class Client
    {
    public:
        // Returns {status, headers, body}
        virtual std::tuple<Status, ResponseHeaders, std::string> get(const std::string& url) = 0;

        // Returns {status, body}
        virtual std::pair<Status, std::string> put(const std::string& url, const std::string& data) = 0;

        // Returns {status, body}
        virtual std::pair<Status, std::string> del(const std::string& url) = 0;

        virtual ~Client() {};
    };

}}}
