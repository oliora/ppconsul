//  Copyright (c)  2014-2020 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <ppconsul/http/status.h>
#include <ppconsul/response.h>
#include <tuple>
#include <string>
#include <map>


namespace ppconsul { namespace http {
    const std::string Token_Header_Name("X-Consul-Token");

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

    using RequestHeaders = std::map<std::string, std::string>;

    class HttpClient
    {
    public:
        using GetResponse = std::tuple<Status, ResponseHeaders, std::string>;
        using PutResponse = std::pair<Status, std::string>;
        using DelResponse = std::pair<Status, std::string>;

        // Returns {HttpStatus, headers, body}
        virtual GetResponse get(const std::string& path, const std::string& query,
                                const RequestHeaders & headers = RequestHeaders{}) = 0;

        // Returns {HttpStatus, body}
        virtual PutResponse put(const std::string& path, const std::string& query, const std::string& data,
                                const RequestHeaders & headers = RequestHeaders{}) = 0;

        // Returns {HttpStatus, body}
        virtual DelResponse del(const std::string& path, const std::string& query,
                                const RequestHeaders & headers = RequestHeaders{}) = 0;

        virtual ~HttpClient() {};
    };

}}
