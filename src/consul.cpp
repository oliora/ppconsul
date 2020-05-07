//  Copyright (c)  2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ppconsul/consul.h"
#include "ppconsul/helpers.h"
#include "curl/http_client.h"


namespace ppconsul {

    const char *BadStatus::what() const PPCONSUL_NOEXCEPT
    {
        if (m_what.empty())
        {
            if (!m_message.empty())
            {
                m_what = helpers::format("%s [%03d %s]",
                    m_message.c_str(),
                    m_status.code(),
                    m_status.message().c_str());
            }
            else
            {
                m_what = helpers::format("%03d %s",
                    m_status.code(),
                    m_status.message().c_str());
            }
        }

        return m_what.c_str();
    }

    HttpClientFactory makeDefaultHttpClientFactory()
    {
        return curl::CurlHttpClientFactory{};
    }

    struct Consul::Impl
    {
        Impl(HttpClientFactory clientFactory, std::string endpoint, ppconsul::http::TlsConfig tlsConfig, bool enableStop)
        : m_clientFactory(std::move(clientFactory))
        , m_endpoint(std::move(endpoint))
        , m_tlsConfig(std::move(tlsConfig))
        , m_enableStop(enableStop)
        , m_stopped{false}
        , m_clientPool([this](){ return createClient(); })
        {}

        std::unique_ptr<http::HttpClient> createClient() const
        {
            CancellationCallback cancellationCallback;
            if (m_enableStop)
                cancellationCallback = [this](){ return stopped(); };

            return m_clientFactory(m_endpoint, m_tlsConfig, std::move(cancellationCallback));
        }

        bool stopped() const noexcept
        {
            return m_stopped.load(std::memory_order_relaxed);
        }

        void stop()
        {
            if (!m_enableStop)
                throw std::logic_error("Must enable stop at construction time");
            m_stopped.store(true, std::memory_order_relaxed);
        }

        HttpClientFactory m_clientFactory;
        std::string m_endpoint;
        ppconsul::http::TlsConfig m_tlsConfig;
        bool m_enableStop;
        std::atomic_bool m_stopped;
        ClientPool m_clientPool;
    };

    Consul::Consul(HttpClientFactory clientFactory, std::string defaultToken, std::string dataCenter, std::string endpoint,
                   http::TlsConfig tlsConfig, bool enableStop)
    : m_impl(new Impl{std::move(clientFactory), helpers::ensureScheme(endpoint), std::move(tlsConfig), enableStop})
    , m_dataCenter(std::move(dataCenter))
    , m_defaultToken(std::move(defaultToken))
    {}

    Consul::~Consul() = default;

    Consul::Consul(Consul &&op) PPCONSUL_NOEXCEPT = default;
    Consul& Consul::operator= (Consul &&op) PPCONSUL_NOEXCEPT = default;

    ClientPool::ClientPtr Consul::getClient() const
    {
        return m_impl->m_clientPool();
    }

    bool Consul::stopped() const noexcept
    {
        return m_impl->stopped();
    }

    void Consul::stop()
    {
        m_impl->stop();
    }
}
