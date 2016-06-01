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

    struct TlsConfig
    {
        TlsConfig() = default;

        std::string cert;
	std::string certType;
        std::string key;
	std::string keyType;
        std::string caPath;
        std::string caInfo;
        bool verifyPeer = true;
        bool verifyHost = true;
        bool verifyStatus = false;

        // Note that keyPass is c-str rather than std::string. That's to make it possible
        // to keep the actual password in a specific location like in protected memory or
        // wiped-afer use memory block and so on.
        const char *keyPass;
    };

    class Client
    {
    public:
        // Returns {status, headers, body}
        virtual std::tuple<Status, ResponseHeaders, std::string> get(const std::string& path, const std::string& query) = 0;

        // Returns {status, body}
        virtual std::pair<Status, std::string> put(const std::string& path, const std::string& query, const std::string& data) = 0;

        // Returns {status, body}
        virtual std::pair<Status, std::string> del(const std::string& path, const std::string& query) = 0;

        virtual ~Client() {};
    };

}}}
