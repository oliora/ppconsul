//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ppconsul/http_status.h"
#include "ppconsul/response.h"
#include <curl/curl.h>
#include <algorithm>
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
        const std::regex g_consulHeaderLineRegex(R"***(([^:]+): +(.+)\r\n)***");

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
        typedef std::tuple<http::Status, ResponseHeaders, std::string> GetResponse;

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
        }

        ~HttpClient()
        {
            if (m_handle)
                curl_easy_cleanup(m_handle);
        }

        GetResponse get(const std::string& url)
        {
            GetResponse r;
            std::get<2>(r).reserve(Buffer_Size);

            // TODO: check return codes
            curl_easy_setopt(m_handle, CURLOPT_HEADERFUNCTION, &HttpClient::headerCallback);
            curl_easy_setopt(m_handle, CURLOPT_CUSTOMREQUEST, nullptr);
            curl_easy_setopt(m_handle, CURLOPT_URL, url.c_str());
            curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, &std::get<2>(r));
            curl_easy_setopt(m_handle, CURLOPT_HEADERDATA, &r);
            curl_easy_setopt(m_handle, CURLOPT_HTTPGET, 1l);
            curl_easy_perform(m_handle);

            return r;
        }

        std::pair<http::Status, std::string> put(const std::string& url, const std::string& body)
        {
            ReadContext ctx(&body, 0u);
            
            std::pair<http::Status, std::string> r;
            r.second.reserve(Buffer_Size);

            // TODO: check return codes
            curl_easy_setopt(m_handle, CURLOPT_HEADERFUNCTION, &HttpClient::headerStatusCallback);
            curl_easy_setopt(m_handle, CURLOPT_CUSTOMREQUEST, nullptr);
            curl_easy_setopt(m_handle, CURLOPT_URL, url.c_str());
            curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, &r.second);
            curl_easy_setopt(m_handle, CURLOPT_HEADERDATA, &r.first);
            curl_easy_setopt(m_handle, CURLOPT_UPLOAD, 1l);
            curl_easy_setopt(m_handle, CURLOPT_PUT, 1l);
            curl_easy_setopt(m_handle, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(body.size()));
            curl_easy_setopt(m_handle, CURLOPT_READDATA, &ctx);
            curl_easy_perform(m_handle);

            return r;
        }

        std::pair<http::Status, std::string> del(const std::string& url)
        {
            std::pair<http::Status, std::string> r;
            r.second.reserve(Buffer_Size);

            // TODO: check return codes
            curl_easy_setopt(m_handle, CURLOPT_HEADERFUNCTION, &HttpClient::headerStatusCallback);
            curl_easy_setopt(m_handle, CURLOPT_URL, url.c_str());
            curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, &r.second);
            curl_easy_setopt(m_handle, CURLOPT_HEADERDATA, &r.first);
            curl_easy_setopt(m_handle, CURLOPT_HTTPGET, 1l);
            curl_easy_setopt(m_handle, CURLOPT_CUSTOMREQUEST, "DELETE");
            curl_easy_perform(m_handle);

            return r;
        }

    private:
        static size_t headerStatusCallback(char *ptr, size_t size_, size_t nitems, void *outputStatus)
        {
            const auto size = size_ * nitems;
            parseStatus(*static_cast<http::Status *>(outputStatus), ptr, size);
            return size;
        }

        static size_t headerCallback(char *ptr, size_t size_, size_t nitems, void *outputResponse_)
        {
            const auto size = size_ * nitems;
            auto outputResponse = reinterpret_cast<GetResponse *>(outputResponse_);

            if (parseStatus(std::get<0>(*outputResponse), ptr, size))
                return size;

            // Parse headers
            std::cmatch match;
            if (!std::regex_match(const_cast<const char *>(ptr), const_cast<const char *>(ptr) +size, match, g_consulHeaderLineRegex))
                return size;

            ResponseHeaders& headers = std::get<1>(*outputResponse);

            if (0 == match[1].compare(Index_Header_Name))
                headers.m_index = uint64_headerValue(match[2].str().c_str());
            else if (0 == match[1].compare(LastContact_Headers_Name))
                headers.m_lastContact = std::chrono::milliseconds(uint64_headerValue(match[2].str().c_str()));
            else if (0 == match[1].compare(KnownLeader_Header_Name))
                headers.m_knownLeader = bool_headerValue(match[2].str().c_str());

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
