//  Copyright (c)  2014-2020 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ppconsul/consul.h"
#include "ppconsul/helpers.h"
#include "curl/http_client.h"


namespace ppconsul {

    const char *BadStatus::what() const noexcept
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

    HttpClientFactory makeDefaultHttpClientFactory(bool initCurl)
    {
        if (initCurl) {
            static const curl::CurlInitializer g_initialized;

            if (!g_initialized)
                throw std::runtime_error("CURL was not successfully initialized");
        }

        return curl::CurlHttpClientFactory{};
    }

    struct Consul::Impl
    {
        Impl(HttpClientFactory clientFactory, std::string dataCenter, std::string defaultToken, std::string endpoint, ppconsul::http::HttpClientConfig clientConfig, bool enableStop)
        : m_clientFactory(std::move(clientFactory))
        , m_dataCenter(std::move(dataCenter))
        , m_defaultToken(std::move(defaultToken))
        , m_endpoint(std::move(endpoint))
        , m_clientConfig(std::move(clientConfig))
        , m_enableStop(enableStop)
        , m_stopped{false}
        , m_clientPool([this](){ return createClient(); })
        {}

        std::unique_ptr<http::HttpClient> createClient() const
        {
            CancellationCallback cancellationCallback;
            if (m_enableStop)
                cancellationCallback = [this](){ return stopped(); };

            return m_clientFactory(m_endpoint, m_clientConfig, std::move(cancellationCallback));
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
        std::string m_dataCenter;
        std::string m_defaultToken;
        std::string m_endpoint;
        ppconsul::http::HttpClientConfig m_clientConfig;
        bool m_enableStop;
        std::atomic_bool m_stopped;
        ClientPool m_clientPool;
    };

    Consul::Consul(HttpClientFactory clientFactory, std::string defaultToken, std::string dataCenter, std::string endpoint,
                   http::HttpClientConfig clientConfig, bool enableStop)
    : m_impl(new Impl{std::move(clientFactory), std::move(dataCenter), std::move(defaultToken),
                      helpers::ensureScheme(endpoint), std::move(clientConfig), enableStop})
    {}

    Consul::~Consul() = default;

    Consul::Consul(Consul &&op) noexcept = default;
    Consul& Consul::operator= (Consul &&op) noexcept = default;

    ClientPool::ClientPtr Consul::getClient() const
    {
        return m_impl->m_clientPool();
    }

    const std::string& Consul::dataCenter() const noexcept
    {
        return m_impl->m_dataCenter;
    }

    const std::string& Consul::defaultToken() const noexcept
    {
        return m_impl->m_defaultToken;
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
