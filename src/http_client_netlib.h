//  Copyright (c) 2014-2016 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/http_client.h"
#include <boost/network/protocol/http/client.hpp>


namespace ppconsul { namespace http { namespace impl {
    
    class NetlibHttpClient: public Client
    {
    public:
        virtual std::tuple<http::Status, ResponseHeaders, std::string> get(const std::string& url) override;
        virtual std::pair<http::Status, std::string> put(const std::string& url, const std::string& data) override;
        virtual std::pair<http::Status, std::string> del(const std::string& url) override;

    private:
        boost::network::http::client m_client;
    };

    
    using HttpClient = NetlibHttpClient;

}}}
