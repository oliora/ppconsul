//  Copyright (c) 2014-2016 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "http_client.h"
#include "../http_helpers.h"


namespace ppconsul { namespace netlib {

    using namespace ppconsul::http::impl;
    
    namespace {
        inline const char *header(const boost::network::http::client::response::headers_container_type& h, const std::string& name)
        {
            auto it = h.find(name);
            return h.end() == it ? "" : it->second.c_str();
        }
    
        inline ResponseHeaders getHeaders(const boost::network::http::client::response& rsp)
        {
            ResponseHeaders r;
            auto&& hs = headers(rsp);
            r.m_index = uint64_headerValue(header(hs, Index_Header_Name));
            r.m_knownLeader = bool_headerValue(header(hs, KnownLeader_Header_Name));
            r.m_lastContact = std::chrono::milliseconds(uint64_headerValue(header(hs, LastContact_Headers_Name)));
            return r;
        }

        inline http::Status getStatus(const boost::network::http::client::response& rsp)
        {
            return http::Status(status(rsp), status_message(rsp));
        }
    }

    std::tuple<http::Status, ResponseHeaders, std::string> HttpClient::get(const std::string& url)
    {
        boost::network::http::client::request request(url);
        auto response = m_client.get(request);
        return std::make_tuple(getStatus(response), getHeaders(response), body(response));
    }

    std::pair<http::Status, std::string> HttpClient::put(const std::string& url, const std::string& data)
    {
        boost::network::http::client::request request(url);
        auto response = m_client.put(request, data);
        return std::make_pair(getStatus(response), body(response));
    }

    std::pair<http::Status, std::string> HttpClient::del(const std::string& url)
    {
        boost::network::http::client::request request(url);
        auto response = m_client.delete_(request);
        return std::make_pair(getStatus(response), body(response));
    }

}}
