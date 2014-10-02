#pragma once

#include "ppconsul/consul.h"
#include <cstdlib>

inline ppconsul::Consul create_test_consul()
{
    auto datacenter = std::getenv("PPCONSUL_TEST_DC");
    auto addr = std::getenv("PPCONSUL_TEST_ADDR");

    return ppconsul::Consul(datacenter ? datacenter : "ppconsul_test",
        addr ? addr : ppconsul::Default_Server_Address);
}
