//  Copyright (c)  2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <ppconsul/http/http_client.h>
#include "http_helpers.h"
#include <curl/curl.h>
#include <memory>
#include <atomic>


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

    class CurlHttpClient: public ppconsul::http::HttpClient
    {
    public:
        using GetResponse = std::tuple<http::Status, ResponseHeaders, std::string>;

        CurlHttpClient(const std::string& endpoint,
                       const ppconsul::http::TlsConfig& tlsConfig,
                       const std::function<bool()>& cancellationCallback);

        virtual ~CurlHttpClient() override;

        GetResponse get(const std::string& path, const std::string& query) override;
        std::pair<http::Status, std::string> put(const std::string& path, const std::string& query, const std::string& data) override;
        std::pair<http::Status, std::string> del(const std::string& path, const std::string& query) override;

        bool stopped() const noexcept { return m_cancellationCallback(); }

        CurlHttpClient(const CurlHttpClient&) = delete;
        CurlHttpClient(CurlHttpClient&&) = delete;
        CurlHttpClient& operator= (const CurlHttpClient&) = delete;
        CurlHttpClient& operator= (CurlHttpClient&&) = delete;

    private:
        void setupTls(const ppconsul::http::TlsConfig& tlsConfig);

        template<class Opt, class T>
        void setopt(Opt opt, const T& t);

        std::string makeUrl(const std::string& path, const std::string& query) const { return ppconsul::http::impl::makeUrl(m_endpoint, path, query); }

        CURL *handle() const PPCONSUL_NOEXCEPT { return m_handle.get(); }

        void perform();

        std::function<bool()> m_cancellationCallback;

        std::string m_endpoint;
        std::unique_ptr<CURL, detail::CurlEasyDeleter> m_handle;
        char m_errBuffer[CURL_ERROR_SIZE]; // Replace with unique_ptr<std::array<char, CURL_ERROR_SIZE>> if moving is needed
    };

    struct CurlHttpClientFactory
    {
        CurlHttpClientFactory();

        std::unique_ptr<CurlHttpClient> operator() (const std::string& endpoint,
                                                    const ppconsul::http::TlsConfig& tlsConfig,
                                                    std::function<bool()> cancellationCallback) const;
    };
}}
