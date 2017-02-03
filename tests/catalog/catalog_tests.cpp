//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <catch/catch.hpp>

#include "ppconsul/catalog.h"
#include "ppconsul/agent.h"
#include "test_consul.h"
#include <algorithm>
#include <thread>


using namespace ppconsul::catalog;
namespace agent = ppconsul::agent;
using ppconsul::agent::Agent;
using ppconsul::Node;
using ppconsul::Consistency;


namespace {
    const auto Uniq_Name_1 = "B0D8A57F-0A73-4C6A-926A-262088B00B76";
    const auto Uniq_Name_2 = "749E5A49-4202-4995-AD5B-A3F28E19ADC1";
    const auto Uniq_Name_3 = "8d097c82-e9fb-471e-9479-c6b03313ab61";
    const auto Uniq_Name_1_Spec = "\r\nB0D8A57F-0A73-4C6A-926A-262088B00B76{}";
    const auto Uniq_Name_2_Spec = "{}749E5A49-4202-4995-AD5B-A3F28E19ADC1\r\n";
    const auto Tag_Spec = "{}bla\r\n";
    const auto Non_Existing_Service_Name = "D0087276-8F85-4612-AC88-8871DB15B2A7";
    const auto Non_Existing_Tag_Name = Non_Existing_Service_Name;
    const auto Non_Existing_Node_Name = Non_Existing_Service_Name;

    inline void sleep(double seconds)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long long>(seconds * 1000.0)));
    }
}

TEST_CASE("catalog.node_valid", "[consul][catalog][config]")
{
    CHECK_FALSE((Node{}).valid());
    CHECK((Node{ "name", "addr" }).valid());
    CHECK_FALSE((Node{ "", "addr" }).valid());
    CHECK_FALSE((Node{ "name", "" }).valid());
}


TEST_CASE("catalog.datacenters", "[consul][catalog][config]")
{
    auto consul = create_test_consul();
    Catalog catalog(consul);

    auto dcs = catalog.datacenters();

    REQUIRE(dcs.size() > 0);
    CHECK(std::find(dcs.begin(), dcs.end(), get_test_datacenter()) != dcs.end());

    for (const auto& d : dcs)
    {
        CHECK(d != "");
    }
}

TEST_CASE("catalog.nodes", "[consul][catalog][config]")
{
    auto consul = create_test_consul();
    Catalog catalog(consul);

    const auto selfMember = Agent(consul).self().second;

    auto nodes = catalog.nodes();

    REQUIRE(nodes.size());

    auto it1 = std::find_if(nodes.begin(), nodes.end(), [&](const Node& op){
        return op.address == selfMember.address;
    });

    REQUIRE(it1 != nodes.end());
    CHECK((it1->node == selfMember.name
        || it1->node.find(selfMember.name + ".") == 0));

    for (const auto& node : nodes)
    {
        CHECK(node.valid());
    }
}

TEST_CASE("catalog.nodes_blocking", "[consul][catalog][config][blocking]")
{
    auto consul = create_test_consul();
    Catalog catalog(consul);

    const auto selfMember = Agent(consul).self().second;

    auto index1 = catalog.nodes(ppconsul::withHeaders, kw::consistency = Consistency::Consistent).headers().index();

    REQUIRE(index1);

    auto t1 = std::chrono::steady_clock::now();
    auto resp1 = catalog.nodes(ppconsul::withHeaders, kw::block_for = { std::chrono::seconds(5), index1 });
    REQUIRE(index1 == resp1.headers().index()); // otherwise someone else did some change. TODO: make test more stable
    CHECK((std::chrono::steady_clock::now() - t1) >= std::chrono::seconds(5));
    
    CHECK(resp1.data().size());

    auto it1 = std::find_if(resp1.data().begin(), resp1.data().end(), [&](const Node& op){
        return op.address == selfMember.address;
    });

    REQUIRE(it1 != resp1.data().end());
    CHECK((it1->node == selfMember.name
        || it1->node.find(selfMember.name + ".") == 0));

    // Wait for already changed
    auto t2 = std::chrono::steady_clock::now();
    auto resp2 = catalog.nodes(ppconsul::withHeaders, kw::block_for = { std::chrono::seconds(5), 0 });
    CHECK((std::chrono::steady_clock::now() - t2) < std::chrono::seconds(2));
    
    CHECK(resp1.data().size());
}

TEST_CASE("catalog.services_default_attributes", "[consul][catalog][services]")
{
    auto consul = create_test_consul();
    Catalog catalog(consul);
    Agent agent(consul);

    const auto selfMember = Agent(consul).self().second;
    const auto selfNode = Node{ selfMember.name, selfMember.address };

    agent.deregisterService(Uniq_Name_3);
    agent.registerService({ Uniq_Name_3 });

    sleep(2.0); // Give some time to propogate registered services to the catalog

    auto services = catalog.service(Uniq_Name_3);

    REQUIRE(services.size() == 1);

    CHECK(services[0].second.name == Uniq_Name_3);
    CHECK(services[0].second.address == "");
    CHECK(services[0].second.port == 0);
    CHECK(services[0].second.tags == ppconsul::Tags{});
    CHECK(services[0].second.id == Uniq_Name_3);
}

TEST_CASE("catalog.services", "[consul][catalog][services]")
{
    auto consul = create_test_consul();
    Catalog catalog(consul);
    Agent agent(consul);

    const auto selfMember = Agent(consul).self().second;
    const auto selfNode = Node{ selfMember.name, selfMember.address };

    agent.deregisterService("service1");
    agent.deregisterService("service2");
    agent.deregisterService("service3");
    agent.registerService(
        agent::kw::name = Uniq_Name_1,
        agent::kw::port = 1234,
        agent::kw::tags = { "print", "udp" },
        agent::kw::id = "service1",
        agent::kw::address = "hostxxx1"
    );
    agent.registerService(
        agent::kw::name = Uniq_Name_2,
        agent::kw::tags = { "copier", "udp" },
        agent::kw::id = "service2"
    );
    agent.registerService(
        agent::kw::name = Uniq_Name_1,
        agent::kw::port = 3456,
        agent::kw::tags = { "print", "secret" },
        agent::kw::id = "service3",
        agent::kw::address = "hostxxx2"
    );

    sleep(2.0); // Give some time to propogate registered services to the catalog

    SECTION("services")
    {
        auto services = catalog.services();

        REQUIRE(services.count(Uniq_Name_1));
        REQUIRE(services.count(Uniq_Name_2));

        CHECK(services.at(Uniq_Name_1) == ppconsul::Tags({ "print", "secret", "udp" }));
        CHECK(services.at(Uniq_Name_2) == ppconsul::Tags({ "copier", "udp" }));
    }

    SECTION("non existing service")
    {
        CHECK(catalog.service(Non_Existing_Service_Name).empty());
    }

    SECTION("service")
    {
        auto services = catalog.service(Uniq_Name_1);
        
        REQUIRE(services.size() == 2);

        const auto service1Index = services[0].second.id == "service1" ? 0 : 1;

        CHECK(services[service1Index].first == selfNode);
        CHECK(services[service1Index].second.name == Uniq_Name_1);
        CHECK(services[service1Index].second.address == "hostxxx1");
        CHECK(services[service1Index].second.port == 1234);
        CHECK(services[service1Index].second.tags == ppconsul::Tags({ "print", "udp" }));
        CHECK(services[service1Index].second.id == "service1");

        CHECK(services[1 - service1Index].first == selfNode);
        CHECK(services[1 - service1Index].second.name == Uniq_Name_1);
        CHECK(services[1 - service1Index].second.address == "hostxxx2");
        CHECK(services[1 - service1Index].second.port == 3456);
        CHECK(services[1 - service1Index].second.tags == ppconsul::Tags({ "print", "secret" }));
        CHECK(services[1 - service1Index].second.id == "service3");
    }

    SECTION("non existing service tag")
    {
        CHECK(catalog.service(Uniq_Name_1, kw::tag = Non_Existing_Tag_Name).empty());
    }

    SECTION("service with tag")
    {
        auto services1 = catalog.service(Uniq_Name_1, kw::tag = "udp");

        REQUIRE(services1.size() == 1);

        CHECK(services1[0].first == selfNode);
        CHECK(services1[0].second.name == Uniq_Name_1);
        CHECK(services1[0].second.address == "hostxxx1");
        CHECK(services1[0].second.port == 1234);
        CHECK(services1[0].second.tags == ppconsul::Tags({ "print", "udp" }));
        CHECK(services1[0].second.id == "service1");

        auto services2 = catalog.service(Uniq_Name_1, kw::tag = "print");

        REQUIRE(services2.size() == 2);

        const auto service1Index = services2[0].second.id == "service1" ? 0 : 1;
        
        CHECK(services2[service1Index].first == selfNode);
        CHECK(services2[service1Index].second.name == Uniq_Name_1);
        CHECK(services2[service1Index].second.address == "hostxxx1");
        CHECK(services2[service1Index].second.port == 1234);
        CHECK(services2[service1Index].second.tags == ppconsul::Tags({ "print", "udp" }));
        CHECK(services2[service1Index].second.id == "service1");
        
        CHECK(services2[1 - service1Index].first == selfNode);
        CHECK(services2[1 - service1Index].second.name == Uniq_Name_1);
        CHECK(services2[1 - service1Index].second.address == "hostxxx2");
        CHECK(services2[1 - service1Index].second.port == 3456);
        CHECK(services2[1 - service1Index].second.tags == ppconsul::Tags({ "print", "secret" }));
        CHECK(services2[1 - service1Index].second.id == "service3");
    }

    SECTION("node")
    {
        auto node = catalog.node(selfMember.name);

        REQUIRE(node.first.valid());
        CHECK(node.first == selfNode);
        
        REQUIRE(node.second.count("service1"));
        REQUIRE(node.second.count("service2"));
        REQUIRE(node.second.count("service3"));

        const auto& s1 = node.second.at("service1");
        const auto& s2 = node.second.at("service2");
        const auto& s3 = node.second.at("service3");
        
        CHECK(s1.name == Uniq_Name_1);
        CHECK(s1.address == "hostxxx1");
        CHECK(s1.port == 1234);
        CHECK(s1.tags == ppconsul::Tags({ "print", "udp" }));
        CHECK(s1.id == "service1");

        CHECK(s2.name == Uniq_Name_2);
        CHECK(s2.address == "");
        CHECK(s2.port == 0);
        CHECK(s2.tags == ppconsul::Tags({ "copier", "udp" }));
        CHECK(s2.id == "service2");

        CHECK(s3.name == Uniq_Name_1);
        CHECK(s3.address == "hostxxx2");
        CHECK(s3.port == 3456);
        CHECK(s3.tags == ppconsul::Tags({ "print", "secret" }));
        CHECK(s3.id == "service3");
    }

    SECTION("non-existing node")
    {
        auto node = catalog.node(Non_Existing_Node_Name);

        CHECK_FALSE(node.first.valid());
        CHECK(node.second.empty());
    }
}

TEST_CASE("catalog.services_special_chars", "[consul][catalog][services][special chars]")
{
    auto consul = create_test_consul();
    Catalog catalog(consul);
    Agent agent(consul);

    const auto selfMember = Agent(consul).self().second;
    const auto selfNode = Node{ selfMember.name, selfMember.address };

    agent.deregisterService("service1");
    agent.deregisterService("service2");
    agent.deregisterService("service3");
    agent.registerService(
        agent::kw::name = Uniq_Name_1_Spec,
        agent::kw::port = 1234,
        agent::kw::tags = { "print", Tag_Spec },
        agent::kw::id = "service1"
    );
    agent.registerService(
        agent::kw::name = Uniq_Name_2_Spec,
        agent::kw::port = 2345,
        agent::kw::tags = { "copier", Tag_Spec },
        agent::kw::id = "service2"
    );
    agent.registerService(
        agent::kw::name = Uniq_Name_1_Spec,
        agent::kw::port = 3456,
        agent::kw::tags = { "print", "secret" },
        agent::kw::id = "service3"
    );

    sleep(2.0); // Give some time to propogate registered services to the catalog

    SECTION("services")
    {
        auto services = catalog.services();

        REQUIRE(services.count(Uniq_Name_1_Spec));
        REQUIRE(services.count(Uniq_Name_2_Spec));

        CHECK(services.at(Uniq_Name_1_Spec) == ppconsul::Tags({ "print", "secret", Tag_Spec }));
        CHECK(services.at(Uniq_Name_2_Spec) == ppconsul::Tags({ "copier", Tag_Spec }));
    }

    SECTION("service")
    {
        auto services = catalog.service(Uniq_Name_1_Spec);

        REQUIRE(services.size() == 2);

        const auto service1Index = services[0].second.id == "service1" ? 0 : 1;

        CHECK(services[service1Index].first == selfNode);
        CHECK(services[service1Index].second.name == Uniq_Name_1_Spec);
        CHECK(services[service1Index].second.port == 1234);
        CHECK(services[service1Index].second.tags == ppconsul::Tags({ "print", Tag_Spec }));
        CHECK(services[service1Index].second.id == "service1");

        CHECK(services[1 - service1Index].first == selfNode);
        CHECK(services[1 - service1Index].second.name == Uniq_Name_1_Spec);
        CHECK(services[1 - service1Index].second.port == 3456);
        CHECK(services[1 - service1Index].second.tags == ppconsul::Tags({ "print", "secret" }));
        CHECK(services[1 - service1Index].second.id == "service3");
    }

    SECTION("service with tag")
    {
        auto services1 = catalog.service(Uniq_Name_1_Spec, kw::tag = Tag_Spec);

        REQUIRE(services1.size() == 1);

        CHECK(services1[0].first == selfNode);
        CHECK(services1[0].second.name == Uniq_Name_1_Spec);
        CHECK(services1[0].second.port == 1234);
        CHECK(services1[0].second.tags == ppconsul::Tags({ "print", Tag_Spec }));
        CHECK(services1[0].second.id == "service1");

        auto services2 = catalog.service(Uniq_Name_1_Spec, kw::tag = "print");

        REQUIRE(services2.size() == 2);

        const auto service1Index = services2[0].second.id == "service1" ? 0 : 1;

        CHECK(services2[service1Index].first == selfNode);
        CHECK(services2[service1Index].second.name == Uniq_Name_1_Spec);
        CHECK(services2[service1Index].second.port == 1234);
        CHECK(services2[service1Index].second.tags == ppconsul::Tags({ "print", Tag_Spec }));
        CHECK(services2[service1Index].second.id == "service1");
        
        CHECK(services2[1 - service1Index].first == selfNode);
        CHECK(services2[1 - service1Index].second.name == Uniq_Name_1_Spec);
        CHECK(services2[1 - service1Index].second.port == 3456);
        CHECK(services2[1 - service1Index].second.tags == ppconsul::Tags({ "print", "secret" }));
        CHECK(services2[1 - service1Index].second.id == "service3");
    }
}

TEST_CASE("catalog.services_blocking", "[consul][catalog][services][blocking]")
{
    auto consul = create_test_consul();
    Catalog catalog(consul);
    Agent agent(consul);

    const auto selfMember = Agent(consul).self().second;
    const auto selfNode = Node{ selfMember.name, selfMember.address };

    agent.deregisterService("service1");
    agent.deregisterService("service2");
    agent.deregisterService("service3");
    agent.registerService(
        agent::kw::name = Uniq_Name_1,
        agent::kw::port = 1234,
        agent::kw::tags = { "print", "udp" },
        agent::kw::id = "service1"
    );
    agent.registerService(
        agent::kw::name = Uniq_Name_2,
        agent::kw::port = 2345,
        agent::kw::tags = { "copier", "udp" },
        agent::kw::id = "service2"
    );

    sleep(2.0); // Give some time to propogate registered services to the catalog

    SECTION("services")
    {
        auto index1 = catalog.services(ppconsul::withHeaders, kw::consistency = Consistency::Consistent).headers().index();

        REQUIRE(index1);

        auto t1 = std::chrono::steady_clock::now();
        auto resp1 = catalog.services(ppconsul::withHeaders, kw::block_for = { std::chrono::seconds(5), index1 });
        REQUIRE(index1 == resp1.headers().index()); // otherwise someone else did some change. TODO: make test more stable
        CHECK((std::chrono::steady_clock::now() - t1) >= std::chrono::seconds(5));

        CHECK(resp1.data().at(Uniq_Name_1) == ppconsul::Tags({ "print", "udp" }));
        CHECK(resp1.data().at(Uniq_Name_2) == ppconsul::Tags({ "copier", "udp" }));

        agent.registerService(
            agent::kw::name = Uniq_Name_1,
            agent::kw::port = 3456,
            agent::kw::tags = { "print", "secret" },
            agent::kw::id = "service3"
        );

        sleep(1.0); // Give some time to propogate to the catalog

        auto t2 = std::chrono::steady_clock::now();
        auto resp2 = catalog.services(ppconsul::withHeaders, kw::block_for = { std::chrono::seconds(5), index1 });
        CHECK((std::chrono::steady_clock::now() - t2) < std::chrono::seconds(2));
        CHECK(resp2.headers().index() >= index1);

        CHECK(resp2.data().at(Uniq_Name_1) == ppconsul::Tags({ "print", "secret", "udp" }));
        CHECK(resp2.data().at(Uniq_Name_2) == ppconsul::Tags({ "copier", "udp" }));
    }

    SECTION("service")
    {
        auto index1 = catalog.service(ppconsul::withHeaders, Uniq_Name_1, kw::consistency = Consistency::Consistent).headers().index();

        REQUIRE(index1);

        auto t1 = std::chrono::steady_clock::now();
        auto resp1 = catalog.service(ppconsul::withHeaders, Uniq_Name_1, kw::block_for = { std::chrono::seconds(5), index1 });
        REQUIRE(index1 == resp1.headers().index()); // otherwise someone else did some change. TODO: make test more stable
        CHECK((std::chrono::steady_clock::now() - t1) >= std::chrono::seconds(5));


        REQUIRE(resp1.data().size() == 1);
        CHECK(resp1.data()[0].second.id == "service1");

        agent.registerService(
            agent::kw::name = Uniq_Name_1,
            agent::kw::port = 3456,
            agent::kw::tags = { "print", "secret" },
            agent::kw::id = "service3"
        );

        sleep(2.0); // Give some time to propogate to the catalog

        auto t2 = std::chrono::steady_clock::now();
        auto resp2 = catalog.service(ppconsul::withHeaders, Uniq_Name_1, kw::block_for = { std::chrono::seconds(5), index1 });
        CHECK((std::chrono::steady_clock::now() - t2) < std::chrono::seconds(2));
        CHECK(index1 != resp2.headers().index());

        REQUIRE(resp2.data().size() == 2);
        const auto service1Index = resp2.data()[0].second.id == "service1" ? 0 : 1;
        CHECK(resp2.data()[service1Index].second.id == "service1");
        CHECK(resp2.data()[1 - service1Index].second.id == "service3");
    }
}
