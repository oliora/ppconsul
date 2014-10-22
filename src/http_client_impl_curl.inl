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

#if !defined PPCONSUL_USE_BOOST_REGEX
#include <regex>
#else
#include <boost/regex.hpp>
#endif

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

#if !defined PPCONSUL_USE_BOOST_REGEX
        using std::regex;
        using std::regex_match;
        using std::cmatch;
#else
        using boost::regex;
        using boost::regex_match;
        using boost::cmatch;
#endif

        const regex g_statusLineRegex(R"***(HTTP\/1\.1 +(\d\d\d) +(.*)\r\n)***");
        const regex g_consulHeaderLineRegex(R"***(([^:]+): +(.+)\r\n)***");

        inline bool parseStatus(http::Status& status, const char *buf, size_t size)
        {
            cmatch match;
            if (!regex_match(buf, buf + size, match, g_statusLineRegex))
                return false;
            status = http::Status(std::atol(match[1].str().c_str()), match[2].str());
            return true;
        }

        void throwCurlError(CURLcode code, const char *err)
        {
            throw std::runtime_error(std::string(err) + " (" + std::to_string(code) + ")");
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

            if (auto err = curl_easy_setopt(m_handle, CURLOPT_ERRORBUFFER, m_errBuffer))
                throwCurlError(err, "");

            // TODO: CURLOPT_NOSIGNAL?
            setopt(CURLOPT_NOPROGRESS, 1l);
            setopt(CURLOPT_WRITEFUNCTION, &HttpClient::writeCallback);
            setopt(CURLOPT_READFUNCTION, &HttpClient::readCallback);
        }

        ~HttpClient()
        {
            if (m_handle)
                curl_easy_cleanup(m_handle);
        }

        HttpClient(const HttpClient&) = delete;
        HttpClient(HttpClient&&) = delete;
        HttpClient& operator= (const HttpClient&) = delete;
        HttpClient& operator= (HttpClient&&) = delete;

        GetResponse get(const std::string& url)
        {
            GetResponse r;
            std::get<2>(r).reserve(Buffer_Size);

            setopt(CURLOPT_HEADERFUNCTION, &HttpClient::headerCallback);
            setopt(CURLOPT_CUSTOMREQUEST, nullptr);
            setopt(CURLOPT_URL, url.c_str());
            setopt(CURLOPT_WRITEDATA, &std::get<2>(r));
            setopt(CURLOPT_HEADERDATA, &r);
            setopt(CURLOPT_HTTPGET, 1l);
            perform();

            return r;
        }

        std::pair<http::Status, std::string> put(const std::string& url, const std::string& data)
        {
            ReadContext ctx(&data, 0u);
            
            std::pair<http::Status, std::string> r;
            r.second.reserve(Buffer_Size);

            setopt(CURLOPT_HEADERFUNCTION, &HttpClient::headerStatusCallback);
            setopt(CURLOPT_CUSTOMREQUEST, nullptr);
            setopt(CURLOPT_URL, url.c_str());
            setopt(CURLOPT_WRITEDATA, &r.second);
            setopt(CURLOPT_HEADERDATA, &r.first);
            setopt(CURLOPT_UPLOAD, 1l);
            setopt(CURLOPT_PUT, 1l);
            setopt(CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(data.size()));
            setopt(CURLOPT_READDATA, &ctx);
            perform();

            return r;
        }

        std::pair<http::Status, std::string> del(const std::string& url)
        {
            std::pair<http::Status, std::string> r;
            r.second.reserve(Buffer_Size);

            setopt(CURLOPT_HEADERFUNCTION, &HttpClient::headerStatusCallback);
            setopt(CURLOPT_URL, url.c_str());
            setopt(CURLOPT_WRITEDATA, &r.second);
            setopt(CURLOPT_HEADERDATA, &r.first);
            setopt(CURLOPT_HTTPGET, 1l);
            setopt(CURLOPT_CUSTOMREQUEST, "DELETE");
            perform();

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
            cmatch match;
            if (!regex_match(const_cast<const char *>(ptr), const_cast<const char *>(ptr) +size, match, g_consulHeaderLineRegex))
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

        template<class T>
        void setopt(CURLoption opt, T&& t)
        {
            const auto err = curl_easy_setopt(m_handle, opt, std::forward<T>(t));
            if (err)
                throwCurlError(err, m_errBuffer);
        }

        void perform()
        {
            const auto err = curl_easy_perform(m_handle);
            if (err)
                throwCurlError(err, m_errBuffer);
        }

        CURL *m_handle;
        char m_errBuffer[CURL_ERROR_SIZE];
    };

}}
