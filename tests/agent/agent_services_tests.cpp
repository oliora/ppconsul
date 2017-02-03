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
    auto const Non_Existing_Service_Name = "DE2F4D40-2664-472D-B0B7-EA0A47D92136";

    inline void sleep(double seconds)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long long>(seconds * 1000.0)));
    }
}

TEST_CASE("agent.service_check_name", "[consul][agent][services]")
{
    REQUIRE(serviceCheckId("bla-bla") == "service:bla-bla");
}

TEST_CASE("agent.service_deregistration", "[consul][agent][services]")
{
    auto consul = create_test_consul();
    Agent agent(consul);

    agent.registerService("service1");

    REQUIRE(agent.services().count("service1"));

    agent.deregisterService("service1");
    REQUIRE(!agent.services().count("service1"));

    CHECK_NOTHROW(agent.deregisterService(Non_Existing_Service_Name));
}

TEST_CASE("agent.service_registration", "[consul][agent][services]")
{
    auto consul = create_test_consul();
    Agent agent(consul);

    agent.deregisterService("service1");
    agent.deregisterService(Unique_Id);

    SECTION("no check very simple")
    {
        agent.registerService(kw::name = "service1");

        const auto services = agent.services();
        REQUIRE(services.count("service1"));
        const auto & s = services.at("service1");

        CHECK(s.id == "service1");
        CHECK(s.name == "service1");
        CHECK(s.address == "");
        CHECK(s.port == 0);
        CHECK(s.tags == ppconsul::Tags());
    }

    SECTION("no check simple")
    {
        agent.registerService(Unique_Id, kw::port = 1357, kw::tags = { "tag1" });

        const auto services = agent.services();
        REQUIRE(services.count(Unique_Id));
        const auto & s = services.at(Unique_Id);

        CHECK(s.id == Unique_Id);
        CHECK(s.name == Unique_Id);
        CHECK(s.address == "");
        CHECK(s.port == 1357);
        CHECK(s.tags == ppconsul::Tags({ "tag1" }));
    }


    SECTION("no check")
    {
        agent.registerService(
            kw::name = "service1",
            kw::port = 9876,
            kw::tags = {"udp", "printer"},
            kw::id = Unique_Id,
            kw::address = "host12"
        );

        const auto services = agent.services();
        REQUIRE(services.count(Unique_Id));
        const auto & s = services.at(Unique_Id);

        CHECK(s.id == Unique_Id);
        CHECK(s.name == "service1");
        CHECK(s.address == "host12");
        CHECK(s.port == 9876);
        CHECK(s.tags == ppconsul::Tags({ "udp", "printer" }));
    }

    SECTION("ttl simple")
    {
        agent.registerService("service1", TtlCheck{std::chrono::minutes(5)});

        const auto services = agent.services();
        REQUIRE(services.count("service1"));
        const auto & s = services.at("service1");

        CHECK(s.id == "service1");
        CHECK(s.name == "service1");
        CHECK(s.address == "");
        CHECK(s.port == 0);
        CHECK(s.tags == ppconsul::Tags());

        const auto checks = agent.checks();
        REQUIRE(checks.count(serviceCheckId("service1")));
        const auto & c = checks.at(serviceCheckId("service1"));

        CHECK(c.id == serviceCheckId("service1"));
        CHECK(c.node == agent.self().second.name);
        CHECK(!c.name.empty());
        CHECK(c.status != CheckStatus::Passing);
        CHECK(c.notes.empty());
        CHECK(c.output.empty());
        CHECK(c.serviceId == "service1");
        CHECK(c.serviceName == "service1");
    }

    SECTION("ttl")
    {
        agent.registerService(
            "service1",

                              TtlCheck{std::chrono::minutes(1)},
            kw::port = 9876,
            kw::tags = { "udp", "printer" },
            kw::id = Unique_Id,
            kw::address = "host25.print"
        );

        const auto services = agent.services();
        REQUIRE(services.count(Unique_Id));
        const auto & s = services.at(Unique_Id);

        CHECK(s.id == Unique_Id);
        CHECK(s.name == "service1");
        CHECK(s.address == "host25.print");
        CHECK(s.port == 9876);
        CHECK(s.tags == ppconsul::Tags({ "udp", "printer" }));

        const auto checks = agent.checks();
        REQUIRE(checks.count(serviceCheckId(Unique_Id)));
        const auto & c = checks.at(serviceCheckId(Unique_Id));

        CHECK(c.id == serviceCheckId(Unique_Id));
        CHECK(c.node == agent.self().second.name);
        CHECK(!c.name.empty());
        CHECK(c.status != CheckStatus::Passing);
        CHECK(c.notes.empty());
        CHECK(c.output.empty());
        CHECK(c.serviceId == Unique_Id);
        CHECK(c.serviceName == "service1");
    }

    SECTION("script simple")
    {
        agent.registerService("service1", ScriptCheck{Non_Existing_Script_Name, std::chrono::minutes(1)});

        const auto services = agent.services();
        REQUIRE(services.count("service1"));
        const auto & s = services.at("service1");

        CHECK(s.id == "service1");
        CHECK(s.name == "service1");
        CHECK(s.address == "");
        CHECK(s.port == 0);
        CHECK(s.tags == ppconsul::Tags());

        sleep(0.5); // To get updated state and output

        const auto checks = agent.checks();
        REQUIRE(checks.count(serviceCheckId("service1")));
        const auto & c = checks.at(serviceCheckId("service1"));

        CHECK(c.id == serviceCheckId("service1"));
        CHECK(c.node == agent.self().second.name);
        CHECK(!c.name.empty());
        CHECK(c.notes.empty());
        CHECK(c.status != CheckStatus::Passing);    // because of Non_Existing_Script_Name
        // CHECK(!c.output.empty());                // different results on different Consul versions on different platforms
        CHECK(c.serviceId == "service1");
        CHECK(c.serviceName == "service1");
    }

    SECTION("script")
    {
        agent.registerService(
            "service1",
            ScriptCheck{Non_Existing_Script_Name, std::chrono::minutes(1)},
            kw::port = 9876,
            kw::tags = { "udp", "printer" },
            kw::id = Unique_Id,
            kw::address = "hooooosttts-adr"
        );

        const auto services = agent.services();
        REQUIRE(services.count(Unique_Id));
        const auto & s = services.at(Unique_Id);

        CHECK(s.id == Unique_Id);
        CHECK(s.name == "service1");
        CHECK(s.address == "hooooosttts-adr");
        CHECK(s.port == 9876);
        CHECK(s.tags == ppconsul::Tags({ "udp", "printer" }));

        sleep(0.5); // To get updated state and output

        const auto checks = agent.checks();
        REQUIRE(checks.count(serviceCheckId(Unique_Id)));
        const auto & c = checks.at(serviceCheckId(Unique_Id));

        CHECK(c.id == serviceCheckId(Unique_Id));
        CHECK(c.node == agent.self().second.name);
        CHECK(!c.name.empty());
        CHECK(c.notes.empty());
        CHECK(c.status != CheckStatus::Passing);    // because of Non_Existing_Script_Name
        // CHECK(!c.output.empty());                // different results on different Consul versions on different platforms
        CHECK(c.serviceId == Unique_Id);
        CHECK(c.serviceName == "service1");
    }
}

TEST_CASE("agent.service_deregistration_with_script", "[consul][agent][services]")
{
    auto consul = create_test_consul();
    Agent agent(consul);

    agent.registerService("service1", TtlCheck{std::chrono::seconds(10)});

    REQUIRE(agent.services().count("service1"));
    REQUIRE(agent.checks().count(serviceCheckId("service1")));

    agent.deregisterService("service1");
    
    REQUIRE(!agent.services().count("service1"));
    REQUIRE(!agent.checks().count(serviceCheckId("service1")));
}

TEST_CASE("agent.service_registration_special_chars", "[consul][agent][service][special chars]")
{
    const auto spec_name = "check{1}\r\n\t\x0";

    auto consul = create_test_consul();
    Agent agent(consul);

    agent.deregisterService(spec_name);
    REQUIRE(!agent.services().count(spec_name));
    REQUIRE(!agent.checks().count(serviceCheckId(spec_name)));

    SECTION("ttl")
    {
        agent.registerService(spec_name, TtlCheck{std::chrono::minutes(5)});

        const auto services = agent.services();
        REQUIRE(services.count(spec_name));
        const auto & s = services.at(spec_name);

        CHECK(s.id == spec_name);
        CHECK(s.name == spec_name);
        CHECK(s.address == "");
        CHECK(s.port == 0);
        CHECK(s.tags == ppconsul::Tags());

        const auto checks = agent.checks();
        REQUIRE(checks.count(serviceCheckId(spec_name)));
        const auto & c = checks.at(serviceCheckId(spec_name));

        CHECK(c.id == serviceCheckId(spec_name));
        CHECK(c.node == agent.self().second.name);
        CHECK(!c.name.empty());
        CHECK(c.status != CheckStatus::Passing);
        CHECK(c.notes.empty());
        CHECK(c.output.empty());
        CHECK(c.serviceId == spec_name);
        CHECK(c.serviceName == spec_name);
    }

    SECTION("script")
    {
        agent.registerService(spec_name, ScriptCheck{Non_Existing_Script_Name, std::chrono::minutes(1)});

        const auto services = agent.services();
        REQUIRE(services.count(spec_name));
        const auto & s = services.at(spec_name);

        CHECK(s.id == spec_name);
        CHECK(s.name == spec_name);
        CHECK(s.address == "");
        CHECK(s.port == 0);
        CHECK(s.tags == ppconsul::Tags());

        sleep(0.5); // To get updated state and output

        const auto checks = agent.checks();
        REQUIRE(checks.count(serviceCheckId(spec_name)));
        const auto & c = checks.at(serviceCheckId(spec_name));

        CHECK(c.id == serviceCheckId(spec_name));
        CHECK(c.node == agent.self().second.name);
        CHECK(!c.name.empty());
        CHECK(c.notes.empty());
        CHECK(c.status != CheckStatus::Passing);    // because of Non_Existing_Script_Name
        // CHECK(!c.output.empty());                // different results on different Consul versions on different platforms
        CHECK(c.serviceId == spec_name);
        CHECK(c.serviceName == spec_name);
    }
}

TEST_CASE("agent.service_check_update", "[consul][agent][service][health]")
{
    auto consul = create_test_consul();
    Agent agent(consul);

    agent.deregisterService("service1");

    agent.registerService("service1", TtlCheck{std::chrono::minutes(5)});
    ppconsul::CheckInfo c = agent.checks().at(serviceCheckId("service1"));
    REQUIRE(c.status != CheckStatus::Passing);
    REQUIRE(c.output == "");

    agent.serviceFail("service1", "it's failed :(");
    c = agent.checks().at(serviceCheckId("service1"));
    REQUIRE(c.status == CheckStatus::Critical);
    REQUIRE(c.output == "it's failed :(");

    agent.servicePass("service1", "status:\neverything passing!!!\n");
    c = agent.checks().at(serviceCheckId("service1"));
    REQUIRE(c.status == CheckStatus::Passing);
    REQUIRE(c.output == "status:\neverything passing!!!\n");

    agent.servicePass("service1");
    c = agent.checks().at(serviceCheckId("service1"));
    REQUIRE(c.status == CheckStatus::Passing);
    REQUIRE(c.output == "");

    agent.serviceWarn("service1");
    c = agent.checks().at(serviceCheckId("service1"));
    REQUIRE(c.status != CheckStatus::Passing);
    REQUIRE(c.output == "");

    //CHECK_THROWS_AS(agent.updateServiceCheck("service1", CheckStatus::Unknown), std::logic_error);
}
