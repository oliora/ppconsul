//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ppconsul/http_status.h"
#include <boost/network/protocol/http/client.hpp>
#include <tuple>


namespace ppconsul { namespace impl {
    
    namespace {
        inline const char *header(const boost::network::http::client::response::headers_container_type& h, const std::string& name)
        {
            auto it = h.find(name);
            return h.end() == it ? "" : it->second.c_str();
        }
    
        inline ResponseHeaders getHeaders(const boost::network::http::client::response& rsp)
        {
            ResponseHeaders r;
            r.m_index = uint64_headerValue(header(rsp.headers(), Index_Header_Name));
            r.m_knownLeader = bool_headerValue(header(rsp.headers(), KnownLeader_Header_Name));
            r.m_lastContact = std::chrono::milliseconds(uint64_headerValue(header(rsp.headers(), LastContact_Headers_Name)));
            return r;
        }
    }


    class HttpClient
    {
    public:
        std::tuple<http::Status, ResponseHeaders, std::string> get(const std::string& url)
        {
            boost::network::http::client::request request(url);
            auto response = m_client.get(request);
            return std::make_tuple(http::Status(response.status(), response.status_message()),
                getHeaders(response), response.data());
        }

        std::pair<http::Status, std::string> put(const std::string& url, const std::string& data)
        {
            boost::network::http::client::request request(url);
            auto response = m_client.put(request, data);
            return std::make_pair(http::Status(response.status(), response.status_message()), response.data());
        }

        std::pair<http::Status, std::string> del(const std::string& url)
        {
            boost::network::http::client::request request(url);
            auto response = m_client.delete_(request);
            return std::make_pair(http::Status(response.status(), response.status_message()), response.data());
        }

    private:
        boost::network::http::client m_client;
    };

}}
