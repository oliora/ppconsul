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

    return ppconsul::Consul(addr ? addr : ppconsul::Default_Server_Address,
        ppconsul::kw::dc = get_test_datacenter());
}
