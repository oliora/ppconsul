//  Copyright (c) 2014-2020 Andrey Upadyshev <oliora@gmail.com>
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

    auto self = agent.self();
    CHECK(!self.config.datacenter.empty());
    CHECK(!self.config.nodeName.empty());
    CHECK(!self.config.nodeId.empty());
    CHECK(!self.config.version.empty());
    // seems like self.config.revision can be empty

    CHECK(!self.member.name.empty());
    CHECK(!self.member.address.empty());
    CHECK(self.member.port);
    CHECK(!self.member.tags.empty());
    CHECK(self.member.status);
    CHECK(self.member.protocolMin);
    CHECK(self.member.protocolMin);
    CHECK(self.member.protocolMax);
    CHECK(self.member.protocolCur);
    CHECK(self.member.delegateMin);
    CHECK(self.member.delegateMax);
    CHECK(self.member.delegateCur);

    CHECK(self.config.nodeName == self.member.name);
}

TEST_CASE("agent.members", "[consul][agent][config][members]")
{
    using ppconsul::agent::Pool;

    auto consul = create_test_consul();
    Agent agent(consul);

    SECTION("wan")
    {
        const auto members = agent.members(Pool::Wan);
        auto self = agent.self();

        auto it1 = std::find_if(members.begin(), members.end(), [&](const ppconsul::agent::Member& op){
            return op.address == self.member.address;
        });

        REQUIRE(it1 != members.end());

        const auto& m = *it1;

        CHECK(m.name.find(self.member.name + ".") == 0);
        CHECK(self.member.address == m.address);
        CHECK(m.port);

        // As recently discovered, self have extra tags thus filter them out first
        ppconsul::Metadata filteredTags;
        for (auto& tag: self.member.tags)
            if (m.tags.count(tag.first))
                filteredTags.emplace(tag);
        CHECK(!m.tags.empty());
        CHECK(filteredTags == m.tags);

        CHECK(self.member.status == m.status);
        CHECK(self.member.protocolMin == m.protocolMin);
        CHECK(self.member.protocolMin == m.protocolMin);
        CHECK(self.member.protocolMax == m.protocolMax);
        CHECK(self.member.protocolCur == m.protocolCur);
        CHECK(self.member.delegateMin == m.delegateMin);
        CHECK(self.member.delegateMax == m.delegateMax);
        CHECK(self.member.delegateCur == m.delegateCur);

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
        auto self = agent.self();

        auto it1 = std::find_if(members.begin(), members.end(), [&](const ppconsul::agent::Member& op){
            return op.address == self.member.address;
        });

        REQUIRE(it1 != members.end());

        const auto& m = *it1;

        CHECK((m.name == self.member.name
            || m.name.find(self.member.name + ".") == 0));
        CHECK(self.member.address == m.address);
        CHECK(m.port);
        CHECK(self.member.tags == m.tags);
        CHECK(self.member.status == m.status);
        CHECK(self.member.protocolMin == m.protocolMin);
        CHECK(self.member.protocolMin == m.protocolMin);
        CHECK(self.member.protocolMax == m.protocolMax);
        CHECK(self.member.protocolCur == m.protocolCur);
        CHECK(self.member.delegateMin == m.delegateMin);
        CHECK(self.member.delegateMax == m.delegateMax);
        CHECK(self.member.delegateCur == m.delegateCur);

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

    CHECK_NOTHROW(agent.forceLeave(agent.self().member.name));
    CHECK_THROWS_AS(agent.join("127.0.0.1:21"), ppconsul::Error);
    CHECK_THROWS_AS(agent.join("127.0.0.1:21", Pool::Wan), ppconsul::Error);
    CHECK_THROWS_AS(agent.join("127.0.0.1:21", Pool::Lan), ppconsul::Error);
    CHECK_NOTHROW(agent.join("127.0.0.1"));
    CHECK_NOTHROW(agent.join("127.0.0.1", Pool::Wan));
    CHECK_NOTHROW(agent.join("127.0.0.1", Pool::Lan));
}
