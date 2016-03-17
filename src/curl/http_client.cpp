//  Copyright (c) 2014-2016 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "http_client.h"
#include "../http_helpers.h"
#include <algorithm>
#include <tuple>
#include <cassert>
#include <cstdlib>

#if !defined PPCONSUL_USE_BOOST_REGEX
#include <regex>
#else
#include <boost/regex.hpp>
#endif

namespace ppconsul { namespace curl {

    using namespace ppconsul::http::impl;

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

        enum { Buffer_Size = 16384 };

        typedef std::pair<const std::string *, size_t> ReadContext;

        size_t headerStatusCallback(char *ptr, size_t size_, size_t nitems, void *outputStatus)
        {
            const auto size = size_ * nitems;
            parseStatus(*static_cast<http::Status *>(outputStatus), ptr, size);
            return size;
        }

        size_t headerCallback(char *ptr, size_t size_, size_t nitems, void *outputResponse_)
        {
            const auto size = size_ * nitems;
            auto outputResponse = reinterpret_cast<HttpClient::GetResponse *>(outputResponse_);

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

        size_t writeCallback(char *ptr, size_t size_, size_t nitems, void *outputStr)
        {
            const auto size = size_ * nitems;
            static_cast<std::string *>(outputStr)->append(ptr, size);
            return size;
        }

        size_t readCallback(char *buffer, size_t size_, size_t nitems, void *readContext)
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
    }

    HttpClient::HttpClient()
    : m_handle(nullptr)
    {
        static const CurlInitializer g_initialized;

        if (!g_initialized)
            throw std::runtime_error("CURL was not successfully initialized");

        m_handle = curl_easy_init();
        if (!m_handle)
            throw std::runtime_error("CURL handle creation failed");

        if (auto err = curl_easy_setopt(m_handle, CURLOPT_ERRORBUFFER, m_errBuffer))
            throwCurlError(err, "");

        // TODO: CURLOPT_NOSIGNAL?
        setopt(CURLOPT_NOPROGRESS, 1l);
        setopt(CURLOPT_WRITEFUNCTION, &writeCallback);
        setopt(CURLOPT_READFUNCTION, &readCallback);
    }

    HttpClient::~HttpClient()
    {
        if (m_handle)
            curl_easy_cleanup(m_handle);
    }

    HttpClient::GetResponse HttpClient::get(const std::string& url)
    {
        GetResponse r;
        std::get<2>(r).reserve(Buffer_Size);

        setopt(CURLOPT_HEADERFUNCTION, &headerCallback);
        setopt(CURLOPT_CUSTOMREQUEST, nullptr);
        setopt(CURLOPT_URL, url.c_str());
        setopt(CURLOPT_WRITEDATA, &std::get<2>(r));
        setopt(CURLOPT_HEADERDATA, &r);
        setopt(CURLOPT_HTTPGET, 1l);
        perform();

        return r;
    }

    std::pair<http::Status, std::string> HttpClient::put(const std::string& url, const std::string& data)
    {
        ReadContext ctx(&data, 0u);
        
        std::pair<http::Status, std::string> r;
        r.second.reserve(Buffer_Size);

        setopt(CURLOPT_HEADERFUNCTION, &headerStatusCallback);
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

    std::pair<http::Status, std::string> HttpClient::del(const std::string& url)
    {
        std::pair<http::Status, std::string> r;
        r.second.reserve(Buffer_Size);

        setopt(CURLOPT_HEADERFUNCTION, &headerStatusCallback);
        setopt(CURLOPT_URL, url.c_str());
        setopt(CURLOPT_WRITEDATA, &r.second);
        setopt(CURLOPT_HEADERDATA, &r.first);
        setopt(CURLOPT_HTTPGET, 1l);
        setopt(CURLOPT_CUSTOMREQUEST, "DELETE");
        perform();

        return r;
    }

    template<class Opt, class T>
    inline void HttpClient::setopt(Opt opt, const T& t)
    {
        const auto err = curl_easy_setopt(m_handle, opt, t);
        if (err)
            throwCurlError(err, m_errBuffer);
    }


    inline void HttpClient::perform()
    {
        const auto err = curl_easy_perform(m_handle);
        if (err)
            throwCurlError(err, m_errBuffer);
    }

}}