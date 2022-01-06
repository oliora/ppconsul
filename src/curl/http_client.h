//  Copyright (c)  2014-2020 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <ppconsul/http/http_client.h>
#include "http_helpers.h"
#include <curl/curl.h>
#include <functional>
#include <memory>
#include <atomic>


namespace ppconsul { namespace curl {

    namespace detail
    {
        struct CurlEasyDeleter
        {
            void operator() (CURL *handle) const noexcept
            {
                curl_easy_cleanup(handle);
            }
        };

        struct CurlSListDeleter
        {
            void operator() (curl_slist * slist) const noexcept
            {
                curl_slist_free_all(slist);
            }
        };
    }

    using CurlHeaderList = std::unique_ptr<curl_slist, detail::CurlSListDeleter>;

    struct CurlInitializer
    {
        CurlInitializer()
        {
            m_initialized = 0 == curl_global_init(CURL_GLOBAL_DEFAULT | CURL_GLOBAL_SSL);
        }

        ~CurlInitializer()
        {
            curl_global_cleanup();
        }

        CurlInitializer(const CurlInitializer&) = delete;
        CurlInitializer& operator= (const CurlInitializer&) = delete;

        explicit operator bool() const { return m_initialized; }

    private:
        bool m_initialized;
    };

    class CurlHttpClient: public ppconsul::http::HttpClient
    {
    public:
        CurlHttpClient(const std::string& endpoint,
                       const ppconsul::http::HttpClientConfig& config,
                       const std::function<bool()>& cancellationCallback);

        virtual ~CurlHttpClient() override;

        GetResponse get(const std::string& path, const std::string& query,
                        const http::RequestHeaders & headers = http::RequestHeaders{}) override;
        PutResponse put(const std::string& path, const std::string& query, const std::string& data,
                        const http::RequestHeaders & headers = http::RequestHeaders{}) override;
        DelResponse del(const std::string& path, const std::string& query,
                        const http::RequestHeaders & headers = http::RequestHeaders{}) override;

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

        CURL *handle() const noexcept { return m_handle.get(); }

        void perform();

        void setHeaders(const http::RequestHeaders & headers);

        std::function<bool()> m_cancellationCallback;

        std::string m_endpoint;
        std::unique_ptr<CURL, detail::CurlEasyDeleter> m_handle;
        CurlHeaderList m_headers;
        char m_errBuffer[CURL_ERROR_SIZE]; // Replace with unique_ptr<std::array<char, CURL_ERROR_SIZE>> if moving is needed
    };

    struct CurlHttpClientFactory
    {
        std::unique_ptr<CurlHttpClient> operator() (const std::string& endpoint,
                                                    const ppconsul::http::HttpClientConfig& config,
                                                    std::function<bool()> cancellationCallback) const;
    };
}}
