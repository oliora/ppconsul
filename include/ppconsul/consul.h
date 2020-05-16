//  Copyright (c)  2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <ppconsul/config.h>
#include <ppconsul/error.h>
#include <ppconsul/types.h>
#include <ppconsul/parameters.h>
#include <ppconsul/response.h>
#include <ppconsul/http/status.h>
#include <ppconsul/http/http_client.h>
#include <ppconsul/client_pool.h>
#include <functional>
#include <chrono>
#include <string>
#include <vector>
#include <tuple>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <limits>
#include <iostream>


namespace ppconsul {

    namespace kw {
        PPCONSUL_KEYWORD(dc, std::string)
        PPCONSUL_KEYWORD(token, std::string)
        PPCONSUL_KEYWORD(tag, std::string)
        PPCONSUL_KEYWORD(enable_stop, bool)
        KWARGS_KEYWORD(consistency, Consistency)
        KWARGS_KEYWORD(block_for, BlockForValue)

        namespace tls {
            // Details about TLS options:
            // - https://curl.haxx.se/libcurl/c/curl_easy_setopt.html
            // - https://curl.haxx.se/docs/sslcerts.html

            // Path or name of the client certificate file (CURLOPT_SSLCERT)
            PPCONSUL_KEYWORD(cert, std::string)
            // Type of the client SSL certificate (CURLOPT_SSLCERTTYPE)
            PPCONSUL_KEYWORD(cert_type, std::string)
            // Path or name of the client private key (CURLOPT_SSLKEY)
            PPCONSUL_KEYWORD(key, std::string)
            // Type of the private key file (CURLOPT_SSLKEYTYPE)
            PPCONSUL_KEYWORD(key_type, std::string)
            // Directory holding CA certificates (CURLOPT_CAPATH)
            PPCONSUL_KEYWORD(ca_path, std::string)
            // Path to CA bundle (CURLOPT_CAINFO)
            PPCONSUL_KEYWORD(ca_info, std::string)
            // Verify the peer's SSL certificate (CURLOPT_SSL_VERIFYPEER)
            PPCONSUL_KEYWORD(verify_peer, bool)
            // Verify the certificate's name against host  (CURLOPT_SSL_VERIFYHOST)
            PPCONSUL_KEYWORD(verify_host, bool)
            // Verify the certificate's status (CURLOPT_SSL_VERIFYSTATUS, libcurl >= 7.41.0)
            PPCONSUL_KEYWORD(verify_status, bool)

            // Password for the client's private key or certificate file (CURLOPT_KEYPASSWD)
            // Note that it's a c-str rather than std::string. That's to make it possible
            // to keep the actual password in a specific location like protected or
            // wiped-afer-use memory.
            PPCONSUL_KEYWORD(key_pass, const char *)
        }

        namespace groups {
            KWARGS_KEYWORDS_GROUP(tls, (tls::cert, tls::cert_type,
				        tls::key, tls::key_type, tls::key_pass,
				        tls::ca_path, tls::ca_info,
                                        tls::verify_peer, tls::verify_host, tls::verify_status))
        }

        inline void printParameter(std::ostream& os, const Consistency& v, KWARGS_KW_TAG(consistency))
        {
            switch (v)
            {
                case Consistency::Stale: os << "stale"; break;
                case Consistency::Consistent: os << "consistent"; break;
                default: break;
            }
        }

        inline void printParameter(std::ostream& os, const BlockForValue& v, KWARGS_KW_TAG(block_for))
        {
            os << "wait=" << v.first.count() << "ms&index=" << v.second;
        }
    }

    namespace impl {
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        http::TlsConfig makeTlsConfig(const Params&... params);
    }

    const char Default_Server_Endpoint[] = "http://127.0.0.1:8500";

    using CancellationCallback = std::function<bool()>;

    using HttpClientFactory = std::function<std::unique_ptr<http::HttpClient>(const std::string& endpoint,
                                                                              const ppconsul::http::TlsConfig& tlsConfig,
                                                                              CancellationCallback)>;
    HttpClientFactory makeDefaultHttpClientFactory();


    class Consul
    {
    public:
        // Allowed parameters:
        // - dc - data center to use
        // - token - default token for all client requests (can be overloaded in every specific request)
        // - groups::tls (tls::*) - TLS options
        // - enable_stop - configures client such that stop() may be used
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        explicit Consul(std::string endpoint, const Params&... params)
        : Consul(makeDefaultHttpClientFactory(),
                 kwargs::get_opt(kw::token, std::string(), params...),
                 kwargs::get_opt(kw::dc, std::string(), params...),
                 std::move(endpoint),
                 impl::makeTlsConfig(params...),
                 kwargs::get_opt(kw::enable_stop, false, params...))
        {
            KWARGS_CHECK_IN_LIST(Params, (kw::dc, kw::token, kw::groups::tls, kw::enable_stop))
        }

        // Allowed parameters:
        // - dc - data center to use
        // - token - default token for all client requests (can be overloaded in every specific request)
        // - groups::tls (tls::*) - TLS options
        // - enable_stop - configures client such that stop() may be used
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        explicit Consul(HttpClientFactory clientFactory, std::string endpoint, const Params&... params)
        : Consul(std::move(clientFactory),
                 kwargs::get_opt(kw::token, std::string(), params...),
                 kwargs::get_opt(kw::dc, std::string(), params...),
                 std::move(endpoint),
                 impl::makeTlsConfig(params...),
                 kwargs::get_opt(kw::enable_stop, false, params...))
        {
            KWARGS_CHECK_IN_LIST(Params, (kw::dc, kw::token, kw::groups::tls, kw::enable_stop))
        }

        // Allowed parameters:
        // - dc - data center to use
        // - token - default token for all client requests (can be overloaded in every specific request)
        // - groups::tls (tls::*) - TLS options
        // - enable_stop - configures client such that stop() may be used
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        explicit Consul(HttpClientFactory clientFactory, const Params&... params)
        : Consul(std::move(clientFactory),
                 kwargs::get_opt(kw::token, std::string(), params...),
                 kwargs::get_opt(kw::dc, std::string(), params...),
                 Default_Server_Endpoint,
                 impl::makeTlsConfig(params...),
                 kwargs::get_opt(kw::enable_stop, false, params...))
        {
            KWARGS_CHECK_IN_LIST(Params, (kw::dc, kw::token, kw::groups::tls, kw::enable_stop))
        }

        // Same as Consul(Default_Server_Endpoint, keywords...)
        // Allowed parameters:
        // - dc - data center to use
        // - token - default token for all requests
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        explicit Consul(const Params&... params)
            : Consul(Default_Server_Endpoint, params...)
        {}

        ~Consul();

        Consul(Consul &&op) PPCONSUL_NOEXCEPT;
        Consul& operator= (Consul &&op) PPCONSUL_NOEXCEPT;

        Consul(const Consul &op) = delete;
        Consul& operator= (const Consul &op) = delete;

        // Throws BadStatus if !response_status.success()
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::string> get(WithHeaders, const std::string& path, const Params&... params) const;

        // Throws BadStatus if !response_status.success()
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string get(const std::string& path, const Params&... params) const
        {
            return std::move(get(withHeaders, path, params...).data());
        }

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::string> get(http::Status& status, const std::string& path, const Params&... params) const
        {
            return get_impl(status, path, makeQuery(params...));
        }

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string put(http::Status& status, const std::string& path, const std::string& data, const Params&... params) const
        {
            return put_impl(status, path, makeQuery(params...), data);
        }

        // Throws BadStatus if !response_status.success()
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string put(const std::string& path, const std::string& data, const Params&... params) const;

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string del(http::Status& status, const std::string& path, const Params&... params) const
        {
            return del_impl(status, path, makeQuery(params...));
        }

        // Throws BadStatus if !response_status.success()
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string del(const std::string& path, const Params&... params) const;

        void stop();
        bool stopped() const noexcept;

    private:
        struct Impl;

        Consul(HttpClientFactory clientFactory, std::string defaultToken, std::string dataCenter, std::string endpoint,
               http::TlsConfig tlsConfig, bool enableStop);

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string makeQuery(const Params&... params) const
        {
            return parameters::makeQuery(kw::dc = dataCenter(), kw::token = defaultToken(), params...);
        }

        Response<std::string> get_impl(http::Status& status, const std::string& paty, const std::string& query) const;
        std::string put_impl(http::Status& status, const std::string& path, const std::string& query, const std::string& data) const;
        std::string del_impl(http::Status& status, const std::string& path, const std::string& query) const;

        const std::string& dataCenter() const PPCONSUL_NOEXCEPT;
        const std::string& defaultToken() const PPCONSUL_NOEXCEPT;

        ClientPool::ClientPtr getClient() const;

        std::unique_ptr<Impl> m_impl;
    };

    // Implementation

    inline void throwStatusError(http::Status status, std::string data)
    {
        if (NotFoundError::Code == status.code())
        {
            throw NotFoundError{};
            //throw NotFoundError(std::move(status), std::move(data));
        }
        else
        {
            throw BadStatus(std::move(status), std::move(data));
        }
    }

    template<class... Params, class>
    inline Response<std::string> Consul::get(WithHeaders, const std::string& path, const Params&... params) const
    {
        http::Status s;
        auto r = get(s, path, params...);
        if (!s.success())
            throwStatusError(std::move(s), std::move(r.data()));
        return r;
    }

    template<class... Params, class>
    inline std::string Consul::put(const std::string& path, const std::string& data, const Params&... params) const
    {
        http::Status s;
        auto r = put(s, path, data, params...);
        if (!s.success())
            throwStatusError(std::move(s), std::move(r));
        return r;
    }

    template<class... Params, class>
    inline std::string Consul::del(const std::string& path, const Params&... params) const
    {
        http::Status s;
        auto r = del(s, path, params...);
        if (!s.success())
            throwStatusError(std::move(s), std::move(r));
        return r;
    }

    inline Response<std::string> Consul::get_impl(http::Status& status, const std::string& path, const std::string& query) const
    {
        Response<std::string> r;
        std::tie(status, r.headers(), r.data()) = getClient()->get(path, query);
        return r;
    }

    inline std::string Consul::put_impl(http::Status& status, const std::string& path, const std::string& query, const std::string& data) const
    {
        std::string r;
        std::tie(status, r) = getClient()->put(path, query, data);
        return r;
    }

    inline std::string Consul::del_impl(http::Status& status, const std::string& path, const std::string& query) const
    {
        std::string r;
        std::tie(status, r) = getClient()->del(path, query);
        return r;
    }

    // Implementation
    namespace impl {
        template<class... Params, class>
        inline http::TlsConfig makeTlsConfig(const Params&... params)
        {
            http::TlsConfig res;
            res.cert = kwargs::get_opt(kw::tls::cert, std::string(), params...);
            res.certType = kwargs::get_opt(kw::tls::cert_type, std::string(), params...);
            res.key = kwargs::get_opt(kw::tls::key, std::string(), params...);
            res.keyType = kwargs::get_opt(kw::tls::key_type, std::string(), params...);
            res.caPath = kwargs::get_opt(kw::tls::ca_path, std::string(), params...);
            res.caInfo = kwargs::get_opt(kw::tls::ca_info, std::string(), params...);
            res.verifyPeer = kwargs::get_opt(kw::tls::verify_peer, true, params...);
            res.verifyHost = kwargs::get_opt(kw::tls::verify_host, true, params...);
            res.verifyStatus = kwargs::get_opt(kw::tls::verify_status, false, params...);
            res.keyPass = kwargs::get_opt(kw::tls::key_pass, nullptr, params...);
            return res;
        }
    }
}
