#pragma once

#include <ppconsul/http/http_client.h>
#include <mutex>
#include <vector>
#include <functional>
#include <utility>
#include <memory>
#include <cassert>

namespace ppconsul {

    class ClientPool
    {
    public:
        struct ReleaseToClientPool
        {
            ReleaseToClientPool(ClientPool& pool) noexcept : m_pool(&pool) {}

            void operator() (http::HttpClient *client) const noexcept { m_pool->release(client); }

        private:
            ClientPool *m_pool;
        };

        using ClientPtr = std::unique_ptr<http::HttpClient, ReleaseToClientPool>;
        using Factory = std::function<std::unique_ptr<http::HttpClient>()>;

        explicit ClientPool(Factory factory)
        : m_factory(std::move(factory)) {}

        ClientPool(ClientPool&&) = delete;
        ClientPool(const ClientPool&) = delete;
        ClientPool& operator=(const ClientPool&) = delete;
        ClientPool& operator=(ClientPool&&) = delete;

        ClientPtr operator() () { return get(); }
        ClientPtr get() { return ClientPtr{acquire().release(), ReleaseToClientPool{*this}}; }

    private:
        std::unique_ptr<http::HttpClient> acquire();
        void release(http::HttpClient *client) noexcept;

        const Factory m_factory;

        std::mutex m_mutex;
        std::vector<std::unique_ptr<http::HttpClient>> m_pool;
    };
}

