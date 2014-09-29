#pragma once

#include "http_client.h"
#include <stdint.h>


namespace ppconsul {

    const char Default_Server_Address[] = "localhost:8500";
    const int Protocol_Version = 1;

    namespace detail {
        inline std::string makeBaseUrl(const std::string& addr, int version)
        {
            auto suffix = "/v" + std::to_string(version);

            if (addr.find("://"))
                return addr + suffix;
            else
                return "http://" + addr + suffix;
        }
    }

    enum class Consistency
    {
        Default,
        Consistent,
        Stale
    };

    class Client
    {
    public:
        explicit Client(const std::string& dc = "", const std::string& addr = Default_Server_Address)
        : m_client(detail::makeBaseUrl(addr, Protocol_Version))
        , m_dc(dc)
        {}

    private:
        http::Client m_client;
        std::string m_dc;
    };

}
