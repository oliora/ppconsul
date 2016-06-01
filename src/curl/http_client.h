//  Copyright (c)  2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <ppconsul/http_client.h>
#include "http_helpers.h"
#include <curl/curl.h>
#include <memory>


namespace ppconsul { namespace curl {

    namespace detail
    {
        struct CurlEasyDeleter
        {
            void operator() (CURL *handle) const PPCONSUL_NOEXCEPT
            {
                curl_easy_cleanup(handle);
            }
        };
    }

    class HttpClient: public ppconsul::http::impl::Client
    {
    public:
        using GetResponse = std::tuple<http::Status, ResponseHeaders, std::string>;

        HttpClient(const std::string& endpoint);
        HttpClient(const std::string& endpoint, const ppconsul::http::impl::TlsConfig& tlsConfig);

        virtual ~HttpClient() override;

        GetResponse get(const std::string& path, const std::string& query) override;
        std::pair<http::Status, std::string> put(const std::string& path, const std::string& query, const std::string& data) override;
        std::pair<http::Status, std::string> del(const std::string& path, const std::string& query) override;

        HttpClient(const HttpClient&) = delete;
        HttpClient(HttpClient&&) = delete;
        HttpClient& operator= (const HttpClient&) = delete;
        HttpClient& operator= (HttpClient&&) = delete;

    private:
        template<class Opt, class T>
        void setopt(Opt opt, const T& t);

        std::string makeUrl(const std::string& path, const std::string& query) const { return ppconsul::http::impl::makeUrl(m_endpoint, path, query); }

        CURL *handle() const PPCONSUL_NOEXCEPT { return m_handle.get(); }

        void perform();

        std::string m_endpoint;
        std::unique_ptr<CURL, detail::CurlEasyDeleter> m_handle;
        char m_errBuffer[CURL_ERROR_SIZE]; // Replace with unique_ptr<std::array<char, CURL_ERROR_SIZE>> if moving is needed
    };

}}
