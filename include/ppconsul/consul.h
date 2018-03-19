//  Copyright (c)  2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/config.h"
#include "ppconsul/error.h"
#include "ppconsul/types.h"
#include "ppconsul/parameters.h"
#include "ppconsul/http_status.h"
#include "ppconsul/response.h"
#include "ppconsul/http_client.h"
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
        http::impl::TlsConfig makeTlsConfig(const Params&... params);
    }

    const char Default_Server_Endpoint[] = "http://127.0.0.1:8500";

    class Consul
    {
    public:
        // Allowed parameters:
        // - dc - data center to use
        // - token - default token for all client requests (can be overloaded in every specific request)
        // - groups::tls (tls::*) - TLS options
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        explicit Consul(const std::string& endpoint, const Params&... params)
        : Consul(kwargs::get_opt(kw::token, std::string(), params...),
                kwargs::get_opt(kw::dc, std::string(), params...),
                endpoint, impl::makeTlsConfig(params...))
        {
            KWARGS_CHECK_IN_LIST(Params, (kw::dc, kw::token, kw::groups::tls))
        }

        // Same as Consul(Default_Server_Endpoint, keywords...)
        // Allowed parameters:
        // - dc - data center to use
        // - token - default token for all requests
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        explicit Consul(const Params&... params)
            : Consul(Default_Server_Endpoint, params...)
        {}

        Consul(Consul &&op) PPCONSUL_NOEXCEPT
        : m_client(std::move(op.m_client))
        , m_dataCenter(std::move(op.m_dataCenter))
        , m_defaultToken(std::move(op.m_defaultToken))
        {}

        Consul& operator= (Consul &&op) PPCONSUL_NOEXCEPT
        {
            m_client = std::move(op.m_client);
            m_dataCenter = std::move(op.m_dataCenter);
            m_defaultToken = std::move(op.m_defaultToken);

            return *this;
        }

        Consul(const Consul &op) = delete;
        Consul& operator= (const Consul &op) = delete;

        // Throws BadStatus if !response_status.success()
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::string> get(WithHeaders, const std::string& path, const Params&... params);

        // Throws BadStatus if !response_status.success()
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string get(const std::string& path, const Params&... params)
        {
            return std::move(get(withHeaders, path, params...).data());
        }

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        Response<std::string> get(http::Status& status, const std::string& path, const Params&... params)
        {
            return get_impl(status, path, makeQuery(params...));
        }

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string put(http::Status& status, const std::string& path, const std::string& data, const Params&... params)
        {
            return put_impl(status, path, makeQuery(params...), data);
        }

        // Throws BadStatus if !response_status.success()
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string put(const std::string& path, const std::string& data, const Params&... params);

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string del(http::Status& status, const std::string& path, const Params&... params)
        {
            return del_impl(status, path, makeQuery(params...));
        }

        // Throws BadStatus if !response_status.success()
        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string del(const std::string& path, const Params&... params);

    private:
        Consul(const std::string& defaultToken, const std::string& dataCenter, const std::string& endpoint, const http::impl::TlsConfig& tlsConfig);

        template<class... Params, class = kwargs::enable_if_kwargs_t<Params...>>
        std::string makeQuery(const Params&... params) const
        {
            return parameters::makeQuery(kw::dc = m_dataCenter, kw::token = m_defaultToken, params...);
        }

        Response<std::string> get_impl(http::Status& status, const std::string& paty, const std::string& query);
        std::string put_impl(http::Status& status, const std::string& path, const std::string& query, const std::string& data);
        std::string del_impl(http::Status& status, const std::string& path, const std::string& query);

        std::unique_ptr<http::impl::Client> m_client;
        std::string m_dataCenter;

        std::string m_defaultToken;
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
    inline Response<std::string> Consul::get(WithHeaders, const std::string& path, const Params&... params)
    {
        http::Status s;
        auto r = get(s, path, params...);
        if (!s.success())
            throwStatusError(std::move(s), std::move(r.data()));
        return r;
    }

    template<class... Params, class>
    inline std::string Consul::put(const std::string& path, const std::string& data, const Params&... params)
    {
        http::Status s;
        auto r = put(s, path, data, params...);
        if (!s.success())
            throwStatusError(std::move(s), std::move(r));
        return r;
    }

    template<class... Params, class>
    inline std::string Consul::del(const std::string& path, const Params&... params)
    {
        http::Status s;
        auto r = del(s, path, params...);
        if (!s.success())
            throwStatusError(std::move(s), std::move(r));
        return r;
    }

    inline Response<std::string> Consul::get_impl(http::Status& status, const std::string& path, const std::string& query)
    {
        Response<std::string> r;
        std::tie(status, r.headers(), r.data()) = m_client->get(path, query);
        return r;
    }

    inline std::string Consul::put_impl(http::Status& status, const std::string& path, const std::string& query, const std::string& data)
    {
        std::string r;
        std::tie(status, r) = m_client->put(path, query, data);
        return r;
    }

    inline std::string Consul::del_impl(http::Status& status, const std::string& path, const std::string& query)
    {
        std::string r;
        std::tie(status, r) = m_client->del(path, query);
        return r;
    }

    // Implementation
    namespace impl {
        template<class... Params, class>
        inline http::impl::TlsConfig makeTlsConfig(const Params&... params)
        {
            http::impl::TlsConfig res;
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
