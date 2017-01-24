//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <catch/catch.hpp>

#include "ppconsul/health.h"
#include "ppconsul/agent.h"
#include "test_consul.h"
#include <thread>
#include <algorithm>
#include <map>


using ppconsul::health::Health;
using ppconsul::agent::Agent;
using ppconsul::CheckStatus;
using ppconsul::Consistency;


namespace {
    std::map<std::string, ppconsul::CheckInfo> make_checks_map(std::vector<ppconsul::CheckInfo> checks)
    {
        std::map<std::string, ppconsul::CheckInfo> res;
        for(auto& i: checks)
            res.emplace(i.name, std::move(i));
        return res;
    }
}

TEST_CASE("agent.health", "[consul][health]")
{
    auto consul = create_test_consul();
    Health health(consul);
    Agent agent(consul);

    agent.deregisterCheck("check1");
    agent.deregisterService("service1");

    const auto selfMember = Agent(consul).self().second;

    agent.registerCheck({ "check1" }, std::chrono::seconds(12));
    agent.registerService({ "service1" }, std::chrono::minutes(5));

    auto nodeChecks = health.node(selfMember.name);
    CHECK(nodeChecks.size() >= 3);

    std::map<std::string, 
}
