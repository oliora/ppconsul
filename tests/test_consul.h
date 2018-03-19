#pragma once

#include "ppconsul/consul.h"
#include "chrono_io.h"
#include <cstdlib>


inline std::string get_test_datacenter()
{
    auto datacenter = std::getenv("PPCONSUL_TEST_DC");
    return datacenter ? datacenter : "ppconsul_test";
}

inline ppconsul::Consul create_test_consul()
{
    auto addr = std::getenv("PPCONSUL_TEST_ADDR");

    return ppconsul::Consul(addr ? addr : ppconsul::Default_Server_Endpoint,
        ppconsul::kw::dc = get_test_datacenter());
}

inline std::string get_test_leader()
{
    auto leader = std::getenv("PPCONSUL_TEST_LEADER_ADDR");
    return leader ? leader : "127.0.0.1:8300";
}
