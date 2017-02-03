//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <catch/catch.hpp>

#include "ppconsul/agent.h"
#include "test_consul.h"
#include <thread>
#include <algorithm>


using namespace ppconsul::agent;
using ppconsul::CheckStatus;


/*namespace {
    auto const Non_Existing_Script_Name = "63E7A7B1-FDAC-4D49-9F8F-1479C866815D";
    auto const Unique_Id = "{16CA1AC9-72EE-451D-970E-E520B4EF874A}";
    auto const Non_Existing_Check_Name = "DE2F4D40-2664-472D-B0B7-EA0A47D92136";
}*/

TEST_CASE("agent.self", "[consul][agent][config][self]")
{
    auto consul = create_test_consul();
    Agent agent(consul);

    auto selfMember = agent.self().second;
    CHECK(!selfMember.name.empty());
    CHECK(!selfMember.address.empty());
    CHECK(selfMember.port);
    CHECK(!selfMember.tags.empty());
    CHECK(selfMember.status);
    CHECK(selfMember.protocolMin);
    CHECK(selfMember.protocolMin);
    CHECK(selfMember.protocolMax);
    CHECK(selfMember.protocolCur);
    CHECK(selfMember.delegateMin);
    CHECK(selfMember.delegateMax);
    CHECK(selfMember.delegateCur);
}

TEST_CASE("agent.members", "[consul][agent][config][members]")
{
    using ppconsul::agent::Pool;

    auto consul = create_test_consul();
    Agent agent(consul);

    SECTION("wan")
    {
        const auto members = agent.members(Pool::Wan);
        const auto selfMember = agent.self().second;

        auto it1 = std::find_if(members.begin(), members.end(), [&](const ppconsul::agent::Member& op){
            return op.address == selfMember.address;
        });

        REQUIRE(it1 != members.end());

        const auto& m = *it1;

        CHECK(m.name.find(selfMember.name + ".") == 0);
        CHECK(selfMember.address == m.address);
        CHECK(m.port);
        CHECK(selfMember.tags == m.tags);
        CHECK(selfMember.status == m.status);
        CHECK(selfMember.protocolMin == m.protocolMin);
        CHECK(selfMember.protocolMin == m.protocolMin);
        CHECK(selfMember.protocolMax == m.protocolMax);
        CHECK(selfMember.protocolCur == m.protocolCur);
        CHECK(selfMember.delegateMin == m.delegateMin);
        CHECK(selfMember.delegateMax == m.delegateMax);
        CHECK(selfMember.delegateCur == m.delegateCur);

        for (const auto& m : members)
        {
            CHECK(m.name != "");
            CHECK(m.address != "");
            CHECK(m.port != 0);
            CHECK(!m.tags.empty());
            CHECK(m.status != 0);
            CHECK(m.protocolMin != 0);
            CHECK(m.protocolMax != 0);
            CHECK(m.protocolCur != 0);
            CHECK(m.delegateMin != 0);
            CHECK(m.delegateMax != 0);
            CHECK(m.delegateCur != 0);
        }
    }

    SECTION("lan")
    {
        const auto members = agent.members();
        const auto selfMember = agent.self().second;

        auto it1 = std::find_if(members.begin(), members.end(), [&](const ppconsul::agent::Member& op){
            return op.address == selfMember.address;
        });

        REQUIRE(it1 != members.end());

        const auto& m = *it1;

        CHECK((m.name == selfMember.name
            || m.name.find(selfMember.name + ".") == 0));
        CHECK(selfMember.address == m.address);
        CHECK(m.port);
        CHECK(selfMember.tags == m.tags);
        CHECK(selfMember.status == m.status);
        CHECK(selfMember.protocolMin == m.protocolMin);
        CHECK(selfMember.protocolMin == m.protocolMin);
        CHECK(selfMember.protocolMax == m.protocolMax);
        CHECK(selfMember.protocolCur == m.protocolCur);
        CHECK(selfMember.delegateMin == m.delegateMin);
        CHECK(selfMember.delegateMax == m.delegateMax);
        CHECK(selfMember.delegateCur == m.delegateCur);

        // Useful only if the wan farm present
        CHECK(agent.members().size() == agent.members(Pool::Lan).size());

        for (const auto& m : members)
        {
            CHECK(m.name != "");
            CHECK(m.address != "");
            CHECK(m.port != 0);
            CHECK(!m.tags.empty());
            CHECK(m.status != 0);
            CHECK(m.protocolMin != 0);
            CHECK(m.protocolMax != 0);
            CHECK(m.protocolCur != 0);
            CHECK(m.delegateMin != 0);
            CHECK(m.delegateMax != 0);
            CHECK(m.delegateCur != 0);
        }
    }
}

TEST_CASE("agent.join_and_leave", "[consul][agent][config][join][leave]")
{
    using ppconsul::agent::Pool;

    auto consul = create_test_consul();
    Agent agent(consul);

    CHECK_NOTHROW(agent.forceLeave(agent.self().second.name));
    CHECK_THROWS_AS(agent.join("127.0.0.1:21"), ppconsul::Error);
    CHECK_THROWS_AS(agent.join("127.0.0.1:21", Pool::Wan), ppconsul::Error);
    CHECK_THROWS_AS(agent.join("127.0.0.1:21", Pool::Lan), ppconsul::Error);
    CHECK_NOTHROW(agent.join("127.0.0.1"));
    CHECK_NOTHROW(agent.join("127.0.0.1", Pool::Wan));
    CHECK_NOTHROW(agent.join("127.0.0.1", Pool::Lan));
}
