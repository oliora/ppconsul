//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ppconsul/http_status.h"
#include <curl/curl.h>
#include <algorithm>
#include <cstdlib>
#include <tuple>
#include <cassert>
#include <regex>


namespace ppconsul { namespace impl {

    namespace {
        struct CurlInitializer
        {
            CurlInitializer()
            {
                m_initialized = 0 == curl_global_init(CURL_GLOBAL_DEFAULT & ~CURL_GLOBAL_SSL);
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

        const CurlInitializer g_initialized;

        const std::regex g_statusLineRegex(R"***(HTTP\/1\.1 +(\d\d\d) +(.*)\r\n)***");

        inline bool parseStatus(http::Status& status, const char *buf, size_t size)
        {
            std::cmatch match;
            if (!std::regex_match(buf, buf + size, match, g_statusLineRegex))
                return false;
            status = http::Status(std::atol(match[1].str().c_str()), match[2].str());
            return true;
        }
    }

    class HttpClient
    {
        enum { Buffer_Size = 16384 };

        typedef std::pair<const std::string *, size_t> ReadContext;

    public:
        HttpClient()
        : m_handle(nullptr)
        {
            if (!g_initialized)
                throw std::runtime_error("CURL was not successfully initialized");

            m_handle = curl_easy_init();
            if (!m_handle)
                throw std::runtime_error("CURL handle creation failed");

            // TODO: check return codes
            // TODO: CURLOPT_NOSIGNAL?
            curl_easy_setopt(m_handle, CURLOPT_NOPROGRESS, 1l);
            curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, &HttpClient::writeCallback);
            curl_easy_setopt(m_handle, CURLOPT_READFUNCTION, &HttpClient::readCallback);
            curl_easy_setopt(m_handle, CURLOPT_HEADERFUNCTION, &HttpClient::headerCallback);
        }

        ~HttpClient()
        {
            if (m_handle)
                curl_easy_cleanup(m_handle);
        }

        std::string get(http::Status& status, const std::string& url)
        {
            std::string r;
            r.reserve(Buffer_Size);

            // TODO: check return codes
            curl_easy_setopt(m_handle, CURLOPT_CUSTOMREQUEST, nullptr);
            curl_easy_setopt(m_handle, CURLOPT_URL, url.c_str());
            curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, &r);
            curl_easy_setopt(m_handle, CURLOPT_HEADERDATA, &status);
            curl_easy_setopt(m_handle, CURLOPT_HTTPGET, 1l);
            curl_easy_perform(m_handle);

            return r;
        }

        std::string put(http::Status& status, const std::string& url, const std::string& body)
        {
            ReadContext ctx(&body, 0u);
            
            std::string r;
            r.reserve(Buffer_Size);

            // TODO: check return codes
            curl_easy_setopt(m_handle, CURLOPT_CUSTOMREQUEST, nullptr);
            curl_easy_setopt(m_handle, CURLOPT_URL, url.c_str());
            curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, &r);
            curl_easy_setopt(m_handle, CURLOPT_HEADERDATA, &status);
            curl_easy_setopt(m_handle, CURLOPT_UPLOAD, 1l);
            curl_easy_setopt(m_handle, CURLOPT_PUT, 1l);
            curl_easy_setopt(m_handle, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(body.size()));
            curl_easy_setopt(m_handle, CURLOPT_READDATA, &ctx);
            curl_easy_perform(m_handle);

            return r;
        }

        std::string del(http::Status& status, const std::string& url)
        {
            std::string r;
            r.reserve(Buffer_Size);

            // TODO: check return codes
            curl_easy_setopt(m_handle, CURLOPT_URL, url.c_str());
            curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, &r);
            curl_easy_setopt(m_handle, CURLOPT_HEADERDATA, &status);
            curl_easy_setopt(m_handle, CURLOPT_HTTPGET, 1l);
            curl_easy_setopt(m_handle, CURLOPT_CUSTOMREQUEST, "DELETE");
            curl_easy_perform(m_handle);

            return r;
        }

    private:
        static size_t headerCallback(char *ptr, size_t size_, size_t nitems, void *outputStatus)
        {
            const auto size = size_ * nitems;
            parseStatus(*static_cast<http::Status *>(outputStatus), ptr, size);
            return size;
        }

        static size_t writeCallback(char *ptr, size_t size_, size_t nitems, void *outputStr)
        {
            const auto size = size_ * nitems;
            static_cast<std::string *>(outputStr)->append(ptr, size);
            return size;
        }

        static size_t readCallback(char *buffer, size_t size_, size_t nitems, void *readContext)
        {
            const auto ctx = static_cast<ReadContext *>(readContext);
            
            const auto remainingSize = ctx->first->size() - ctx->second;
            if (!remainingSize)
                return 0;

            auto size = (std::min)(size_ * nitems, remainingSize);
            memcpy(buffer, ctx->first->data() + ctx->second, size);
            ctx->second += size;
            return size;
        }

        CURL *m_handle;
    };

}}
