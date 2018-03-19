//  Copyright (c)  2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "http_client.h"
#include "../http_helpers.h"
#include <ppconsul/helpers.h>
#include <algorithm>
#include <tuple>
#include <cassert>
#include <cstdlib>
#include <stdexcept>

#if !defined PPCONSUL_USE_BOOST_REGEX
#include <regex>
#else
#include <boost/regex.hpp>
#endif

#if (LIBCURL_VERSION_MAJOR < 7)
#error "Where did you get such an ancient libcurl?"
#endif

// CURLOPT_SSL_VERIFYSTATUS was added in libcurl 7.41.0
// https://curl.haxx.se/libcurl/c/CURLOPT_SSL_VERIFYSTATUS.html
#if (LIBCURL_VERSION_MAJOR == 7 && LIBCURL_VERSION_MINOR < 41)
#define PPCONSUL_DISABLE_SSL_VERIFYSTATUS
#endif


namespace ppconsul { namespace curl {

    using namespace ppconsul::http::impl;

    namespace {
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

    HttpClient::HttpClient(const std::string& endpoint)
    : m_endpoint(endpoint)
    {
        static const CurlInitializer g_initialized;

        if (!g_initialized)
            throw std::runtime_error("CURL was not successfully initialized");

        m_handle.reset(curl_easy_init());
        if (!m_handle)
            throw std::runtime_error("CURL handle creation failed");

        if (auto err = curl_easy_setopt(handle(), CURLOPT_ERRORBUFFER, m_errBuffer))
            throwCurlError(err, "");

        // TODO: CURLOPT_NOSIGNAL?
        setopt(CURLOPT_NOPROGRESS, 1l);
        setopt(CURLOPT_WRITEFUNCTION, &writeCallback);
        setopt(CURLOPT_READFUNCTION, &readCallback);
    }

    HttpClient::HttpClient(const std::string& endpoint, const TlsConfig& tlsConfig)
    : HttpClient(endpoint)
    {
        if (!tlsConfig.cert.empty())
            setopt(CURLOPT_SSLCERT, tlsConfig.cert.c_str());
        if (!tlsConfig.certType.empty())
            setopt(CURLOPT_CAPATH, tlsConfig.certType.c_str());
        if (!tlsConfig.key.empty())
            setopt(CURLOPT_SSLKEY, tlsConfig.key.c_str());
        if (!tlsConfig.keyType.empty())
            setopt(CURLOPT_CAPATH, tlsConfig.certType.c_str());
        if (!tlsConfig.caPath.empty())
            setopt(CURLOPT_CAPATH, tlsConfig.caPath.c_str());
        if (!tlsConfig.caInfo.empty())
            setopt(CURLOPT_CAINFO, tlsConfig.caInfo.c_str());

        if (tlsConfig.keyPass && *tlsConfig.keyPass)
            setopt(CURLOPT_KEYPASSWD, tlsConfig.keyPass);

        setopt(CURLOPT_SSL_VERIFYPEER, tlsConfig.verifyPeer ? 1 : 0);
        setopt(CURLOPT_SSL_VERIFYHOST, tlsConfig.verifyHost ? 2 : 0);

        if (tlsConfig.verifyStatus)
        {
#ifdef PPCONSUL_DISABLE_SSL_VERIFYSTATUS
            throw std::runtime_error("Ppconsul was built without support for CURLOPT_SSL_VERIFYSTATUS");
#else
            setopt(CURLOPT_SSL_VERIFYSTATUS, 1);
#endif
        }
    }

    HttpClient::~HttpClient() = default;

    HttpClient::GetResponse HttpClient::get(const std::string& path, const std::string& query)
    {
        GetResponse r;
        std::get<2>(r).reserve(Buffer_Size);

        setopt(CURLOPT_HEADERFUNCTION, &headerCallback);
        setopt(CURLOPT_CUSTOMREQUEST, nullptr);
        setopt(CURLOPT_URL, makeUrl(path, query).c_str());
        setopt(CURLOPT_WRITEDATA, &std::get<2>(r));
        setopt(CURLOPT_HEADERDATA, &r);
        setopt(CURLOPT_HTTPGET, 1l);
        perform();

        return r;
    }

    std::pair<http::Status, std::string> HttpClient::put(const std::string& path, const std::string& query, const std::string& data)
    {
        ReadContext ctx(&data, 0u);
        
        std::pair<http::Status, std::string> r;
        r.second.reserve(Buffer_Size);

        setopt(CURLOPT_HEADERFUNCTION, &headerStatusCallback);
        setopt(CURLOPT_CUSTOMREQUEST, nullptr);
        setopt(CURLOPT_URL, makeUrl(path, query).c_str());
        setopt(CURLOPT_WRITEDATA, &r.second);
        setopt(CURLOPT_HEADERDATA, &r.first);
        setopt(CURLOPT_UPLOAD, 1l);
        setopt(CURLOPT_PUT, 1l);
        setopt(CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(data.size()));
        setopt(CURLOPT_READDATA, &ctx);
        perform();

        return r;
    }

    std::pair<http::Status, std::string> HttpClient::del(const std::string& path, const std::string& query)
    {
        std::pair<http::Status, std::string> r;
        r.second.reserve(Buffer_Size);

        setopt(CURLOPT_HEADERFUNCTION, &headerStatusCallback);
        setopt(CURLOPT_URL, makeUrl(path, query).c_str());
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
        const auto err = curl_easy_setopt(handle(), opt, t);
        if (err)
            throwCurlError(err, m_errBuffer);
    }


    inline void HttpClient::perform()
    {
        const auto err = curl_easy_perform(handle());
        if (err)
            throwCurlError(err, m_errBuffer);
    }

}}
