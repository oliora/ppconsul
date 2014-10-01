#pragma once

#include "ppconsul/consul.h"


inline ppconsul::Consul create_test_consul()
{
    return ppconsul::Consul("ppconsul_test");
}
