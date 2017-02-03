//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <catch/catch.hpp>

#include "ppconsul/agent.h"
#include "test_consul.h"
#include <thread>

using namespace ppconsul::agent;
using ppconsul::CheckStatus;


namespace {
    auto const Non_Existing_Script_Name = "63E7A7B1-FDAC-4D49-9F8F-1479C866815D";
    auto const Unique_Id = "{16CA1AC9-72EE-451D-970E-E520B4EF874A}";
    auto const Non_Existing_Check_Name = "DE2F4D40-2664-472D-B0B7-EA0A47D92136";

    inline void sleep(double seconds)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long long>(seconds * 1000.0)));
    }
}

TEST_CASE("agent.check_deregistration", "[consul][agent][checks]")
{
    auto consul = create_test_consul();
    Agent agent(consul);

    agent.registerCheck(
        kw::name = "check1",
        kw::check = TtlCheck{std::chrono::seconds(12)}
    );
    REQUIRE(agent.checks().count("check1"));

    agent.deregisterCheck("check1");
    REQUIRE(!agent.checks().count("check1"));

    CHECK_NOTHROW(agent.deregisterCheck(Non_Existing_Check_Name));
}

TEST_CASE("agent.check_registration", "[consul][agent][checks]")
{
    auto consul = create_test_consul();
    Agent agent(consul);

    agent.deregisterCheck("check1");
    agent.deregisterCheck(Unique_Id);

    SECTION("ttl")
    {
        agent.registerCheck("check1", TtlCheck{std::chrono::seconds(180)});

        const auto checks = agent.checks();
        REQUIRE(checks.count("check1"));
        const auto & c = checks.at("check1");

        CHECK(c.id == "check1");
        CHECK(c.node == agent.self().second.name);
        CHECK(c.name == "check1");
        CHECK(c.status != CheckStatus::Passing);
        CHECK(c.notes.empty());
        CHECK(c.output.empty());
        CHECK(c.serviceId.empty());
        CHECK(c.serviceName.empty());
    }

    SECTION("ttl with notes")
    {
        agent.registerCheck("check1", TtlCheck{std::chrono::seconds(180)}, kw::notes = "some notes");

        const auto checks = agent.checks();
        REQUIRE(checks.count("check1"));
        const auto & c = checks.at("check1");

        CHECK(c.id == "check1");
        CHECK(c.node == agent.self().second.name);
        CHECK(c.name == "check1");
        CHECK(c.status != CheckStatus::Passing);
        CHECK(c.notes == "some notes");
        CHECK(c.output.empty());
        CHECK(c.serviceId.empty());
        CHECK(c.serviceName.empty());
    }

    SECTION("ttl with id")
    {
        agent.registerCheck("check1", TtlCheck{std::chrono::seconds(180)}, kw::id = Unique_Id, kw::notes = "other notes");

        const auto checks = agent.checks();
        REQUIRE(checks.count(Unique_Id));
        const auto & c = checks.at(Unique_Id);

        CHECK(c.id == Unique_Id);
        CHECK(c.node == agent.self().second.name);
        CHECK(c.name == "check1");
        CHECK(c.status != CheckStatus::Passing);
        CHECK(c.notes == "other notes");
        CHECK(c.output.empty());
        CHECK(c.serviceId.empty());
        CHECK(c.serviceName.empty());
    }

    SECTION("script")
    {
        agent.registerCheck("check1", ScriptCheck{Non_Existing_Script_Name, std::chrono::seconds(100)});
        sleep(0.5); // To get updated state and output

        const auto checks = agent.checks();
        REQUIRE(checks.count("check1"));
        const auto & c = checks.at("check1");

        CHECK(c.id == "check1");
        CHECK(c.node == agent.self().second.name);
        CHECK(c.name == "check1");
        CHECK(c.notes.empty());
        CHECK(c.status != CheckStatus::Passing);    // because of Non_Existing_Script_Name
        // CHECK(!c.output.empty());                // different results on different Consul versions on different platforms
        CHECK(c.serviceId.empty());
        CHECK(c.serviceName.empty());
    }

    SECTION("script with notes")
    {
        agent.registerCheck(
            kw::name = "check1",
            kw::notes = "the notes",
            kw::check = ScriptCheck{Non_Existing_Script_Name, std::chrono::seconds(100)}
        );
        sleep(0.5); // To get updated state and output

        const auto checks = agent.checks();
        REQUIRE(checks.count("check1"));
        const auto & c = checks.at("check1");

        CHECK(c.id == "check1");
        CHECK(c.node == agent.self().second.name);
        CHECK(c.name == "check1");
        CHECK(c.notes == "the notes");
        CHECK(c.status != CheckStatus::Passing);    // because of Non_Existing_Script_Name
        // CHECK(!c.output.empty());                // different results on different Consul versions on different platforms
        CHECK(c.serviceId.empty());
        CHECK(c.serviceName.empty());
    }

    SECTION("script with id")
    {
        agent.registerCheck("check1", ScriptCheck{Non_Existing_Script_Name, std::chrono::seconds(1)}, kw::notes = "the notes", kw::id = Unique_Id);
        sleep(0.5); // To get updated state and output

        const auto checks = agent.checks();
        REQUIRE(checks.count(Unique_Id));
        const auto & c = checks.at(Unique_Id);

        CHECK(c.id == Unique_Id);
        CHECK(c.node == agent.self().second.name);
        CHECK(c.name == "check1");
        CHECK(c.notes == "the notes");
        CHECK(c.status != CheckStatus::Passing);    // because of Non_Existing_Script_Name
        // CHECK(!c.output.empty());                // different results on different Consul versions on different platforms
        CHECK(c.serviceId.empty());
        CHECK(c.serviceName.empty());
    }
}

TEST_CASE("agent.check_registration_special_chars", "[consul][agent][checks][special chars]")
{
    const auto spec_name = "check{1}\r\n\t\x0";

    auto consul = create_test_consul();
    Agent agent(consul);

    agent.deregisterCheck(spec_name);
    REQUIRE(!agent.checks().count(spec_name));

    SECTION("ttl")
    {
        agent.registerCheck(spec_name, TtlCheck{std::chrono::seconds(100)}, kw::notes = "note\n1\n2\n3\nsummary: bla bla");

        REQUIRE(agent.checks().count(spec_name));
        auto c = agent.checks().at(spec_name);

        CHECK(c.id == spec_name);
        CHECK(c.name == spec_name);
        CHECK(c.status != CheckStatus::Passing);
        CHECK(c.notes == "note\n1\n2\n3\nsummary: bla bla");
    }

    SECTION("script")
    {
        agent.registerCheck(spec_name,  ScriptCheck{Non_Existing_Script_Name, std::chrono::seconds(100)});
        sleep(0.5); // To get updated state and output

        REQUIRE(agent.checks().count(spec_name));
        auto c = agent.checks().at(spec_name);

        CHECK(c.id == spec_name);
        CHECK(c.name == spec_name);
        CHECK(c.status != CheckStatus::Passing);    // because of Non_Existing_Script_Name
        // CHECK(!c.output.empty());                // different results on different Consul versions on different platforms
        CHECK(c.notes == "");
    }
}

TEST_CASE("agent.http_check_registration", "[consul][agent][checks][http_check]")
{
    auto consul = create_test_consul();
    Agent agent(consul);

    agent.deregisterCheck("check1");
    agent.deregisterCheck(Unique_Id);

    SECTION("default timeout")
    {
        REQUIRE_NOTHROW(agent.registerCheck("check1", HttpCheck{ "invalid.", std::chrono::minutes(5) }));
    }
    SECTION("custom timeout")
    {
        REQUIRE_NOTHROW(agent.registerCheck("check1", HttpCheck{ "invalid.", std::chrono::minutes(5), std::chrono::milliseconds(500) }));
    }
}

TEST_CASE("agent.tcp_check_registration", "[consul][agent][checks][tcp_check]")
{
    auto consul = create_test_consul();
    Agent agent(consul);

    agent.deregisterCheck("check1");
    agent.deregisterCheck(Unique_Id);

    SECTION("string address")
    {
        SECTION("default timeout")
        {
            REQUIRE_NOTHROW(agent.registerCheck("check1", TcpCheck{ "127.0.0.1:0", std::chrono::minutes(5) }));
        }

        SECTION("custom timeout")
        {
            REQUIRE_NOTHROW(agent.registerCheck("check1", TcpCheck{ "127.0.0.1:0", std::chrono::minutes(5), std::chrono::milliseconds(500) }));
        }
    }

    SECTION("host and port")
    {
        SECTION("default timeout")
        {
            REQUIRE_NOTHROW(agent.registerCheck("check1", TcpCheck{ "127.0.0.1", 0, std::chrono::minutes(5) }));
        }

        SECTION("custom timeout")
        {
            REQUIRE_NOTHROW(agent.registerCheck("check1", TcpCheck{ "127.0.0.1", 0, std::chrono::minutes(5), std::chrono::milliseconds(500) }));
        }
    }
}

TEST_CASE("agent.docker_check_registration", "[consul][agent][checks][docker_check]")
{
    auto consul = create_test_consul();
    Agent agent(consul);

    agent.deregisterCheck("check1");
    agent.deregisterCheck(Unique_Id);

    SECTION("default shell")
    {
        REQUIRE_NOTHROW(agent.registerCheck("check1", DockerCheck{ "example.com-docker-unexisting", Non_Existing_Script_Name, std::chrono::minutes(5) }));
    }

    SECTION("custom shell")
    {
        REQUIRE_NOTHROW(agent.registerCheck("check1", DockerCheck{ "example.com-docker-unexisting", Non_Existing_Script_Name, std::chrono::minutes(5), "/usr/bin/cpp_shell_of_the_future_3016" }));
    }
}

TEST_CASE("agent.check_update", "[consul][agent][checks][health]")
{
    auto consul = create_test_consul();
    Agent agent(consul);

    agent.deregisterCheck("check1");
    REQUIRE(!agent.checks().count("check1"));

    agent.registerCheck("check1", TtlCheck{std::chrono::minutes(5)}, kw::notes = "the check");

    ppconsul::CheckInfo c = agent.checks().at("check1");
    REQUIRE(c.status != CheckStatus::Passing);
    REQUIRE(c.notes == "the check");
    REQUIRE(c.output == "");
    
    agent.fail("check1", "it's failed :(");
    c = agent.checks().at("check1");
    REQUIRE(c.status == CheckStatus::Critical);
    REQUIRE(c.notes == "the check");
    REQUIRE(c.output == "it's failed :(");

    agent.pass("check1", "status:\neverything passing!!!\n");
    c = agent.checks().at("check1");
    REQUIRE(c.status == CheckStatus::Passing);
    REQUIRE(c.notes == "the check");
    REQUIRE(c.output == "status:\neverything passing!!!\n");

    agent.pass("check1");
    c = agent.checks().at("check1");
    REQUIRE(c.status == CheckStatus::Passing);
    REQUIRE(c.notes == "the check");
    REQUIRE(c.output == "");

    agent.warn("check1");
    c = agent.checks().at("check1");
    REQUIRE(c.status != CheckStatus::Passing);
    REQUIRE(c.notes == "the check");
    REQUIRE(c.output == "");

    //CHECK_THROWS_AS(agent.updateCheck("check1", CheckStatus::Unknown), std::logic_error);
}

TEST_CASE("agent.check_update_incorrect", "[consul][agent][checks][health]")
{
    auto consul = create_test_consul();
    Agent agent(consul);

    agent.deregisterCheck("check1");
    REQUIRE(!agent.checks().count("check1"));

    CHECK_THROWS_AS(agent.pass(Non_Existing_Check_Name), ppconsul::Error);

    SECTION("script")
    {
        agent.registerCheck("check1" , ScriptCheck{Non_Existing_Script_Name, std::chrono::minutes(5)});

        CHECK_THROWS_AS(agent.pass("check1"), ppconsul::Error);
    }
    SECTION("http")
    {
        agent.registerCheck("check1", HttpCheck{"invalid.", std::chrono::minutes(5)});

        CHECK_THROWS_AS(agent.pass("check1"), ppconsul::Error);
    }
    SECTION("tcp")
    {
        agent.registerCheck("check1" , TcpCheck{"127.0.0.1:0", std::chrono::minutes(5)});

        CHECK_THROWS_AS(agent.pass("check1"), ppconsul::Error);
    }
    SECTION("docker")
    {
        agent.registerCheck("check1", DockerCheck{"example.com-docker-unexisting", Non_Existing_Script_Name, std::chrono::minutes(5)});

        CHECK_THROWS_AS(agent.pass("check1"), ppconsul::Error);
    }
}

TEST_CASE("agent.check_expired", "[consul][agent][checks][health]")
{
    auto consul = create_test_consul();
    Agent agent(consul);

    agent.deregisterCheck("check1");
    REQUIRE(!agent.checks().count("check1"));

    agent.registerCheck("check1", TtlCheck{std::chrono::seconds(1)});
    REQUIRE(agent.checks().count("check1"));

    sleep(1.5);

    auto c = agent.checks().at("check1");
    CHECK(c.status == CheckStatus::Critical);
    CHECK(!c.output.empty());
}

TEST_CASE("agent.check_update_special_chars", "[consul][agent][checks][health][special chars]")
{
    const auto spec_name = "check{1}\r\n\t\x0";

    auto consul = create_test_consul();
    Agent agent(consul);

    agent.deregisterCheck(spec_name);
    REQUIRE(!agent.checks().count(spec_name));

    agent.registerCheck("check1", TtlCheck{std::chrono::minutes(5)}, kw::notes = "the check");

    ppconsul::CheckInfo c = agent.checks().at("check1");
    REQUIRE(c.status != CheckStatus::Passing);
    REQUIRE(c.notes == "the check");
    REQUIRE(c.output == "");

    agent.pass("check1", "status:\neverything passing!!!\n");
    c = agent.checks().at("check1");
    REQUIRE(c.status == CheckStatus::Passing);
    REQUIRE(c.notes == "the check");
    REQUIRE(c.output == "status:\neverything passing!!!\n");
}
