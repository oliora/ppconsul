//  Copyright (c) 2014-2020 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <catch/catch.hpp>

#include "ppconsul/coordinate.h"
#include "ppconsul/agent.h"
#include "test_consul.h"

#include <algorithm>

using namespace ppconsul::coordinate;
using ppconsul::agent::Agent;

namespace {
    const auto Non_Existing_Node_Name = "D0087276-8F85-4612-AC88-8871DB15B2A7";

    // Verifies that all nodes have the same dimensionality
    bool sameDim(const std::vector<Node>& nodes)
    {
        if (nodes.empty())
        {
            return true;
        }

        auto dim = nodes.front().coord.vec.size();
        
        return std::all_of(
            nodes.begin(), nodes.end(),
            [dim](const Node& node)
            {
                return (node.coord.vec.size() == dim);
            }
        );
    }

}

TEST_CASE("coordinate.datacenters", "[consul][coordinate]")
{
    auto consul = create_test_consul();
    Coordinate coordinate(consul);

    auto dcs = coordinate.datacenters();

    REQUIRE_FALSE(dcs.empty());
    auto it = std::find_if(dcs.begin(), dcs.end(), [&](const Datacenter& op){
        return op.datacenter== get_test_datacenter();
    });
    CHECK(it != dcs.end());

    for (const auto& d : dcs)
    {
        CHECK_FALSE(d.datacenter.empty());
        CHECK_FALSE(d.areaId.empty());

        REQUIRE_FALSE(d.coordinates.empty());
        for (const auto& node : d.coordinates)
        {
            CHECK_FALSE(node.node.empty());
            CHECK_FALSE(node.coord.vec.empty());
        }
        CHECK(sameDim(d.coordinates));
    }
}

TEST_CASE("coordinate.nodes", "[consul][coordinate]")
{
    auto consul = create_test_consul();
    Coordinate coordinate(consul);

    auto self = Agent(consul).self();

    auto nodes = coordinate.nodes();

    REQUIRE_FALSE(nodes.empty());

    auto it = std::find_if(nodes.begin(), nodes.end(), [&](const Node& op){
        return op.node == self.member.name;
    });
    REQUIRE(it != nodes.end());

    for (const auto& node : nodes)
    {
        CHECK_FALSE(node.node.empty());
        CHECK_FALSE(node.coord.vec.empty());
    }
    CHECK(sameDim(nodes));
}

TEST_CASE("coordinate.node", "[consul][coordinate]")
{
    auto consul = create_test_consul();
    Coordinate coordinate(consul);

    auto self = Agent(consul).self();

    auto node = coordinate.node(self.member.name);

    REQUIRE(node.size() >= 1);
    for (const auto& nodeRec : node)
    {
        CHECK(nodeRec.node == self.member.name);
        CHECK_FALSE(nodeRec.coord.vec.empty());
    }
    CHECK(sameDim(node));

    CHECK(coordinate.node(Non_Existing_Node_Name).empty());
}

TEST_CASE("coordinate.node_float_parse", "[consul][coordinate][parse]")
{
    const std::string json("{"
        "\"Node\": \"core-1\","
        "\"Segment\": \"segment\","
        "\"Coord\": {"
            "\"Vec\": ["
                "-0.0030908182083252632,"
                "-0.0034924296469496137,"
                "0.004027661974926579,"
                "0.009441736044369261,"
                "0.009120584954036386,"
                "0.005945711502389089,"
                "-0.005375781324440643,"
                "0.002691224249211683"
            "],"
            "\"Error\": 0.33682855812450196,"
            "\"Adjustment\": -8.957602083707684e-05,"
            "\"Height\": 0.0005342546178363255"
        "}"
    "}");

    auto node = impl::parseNode(json);

    CHECK(node.node == "core-1");
    CHECK(node.segment == "segment");

    REQUIRE(node.coord.vec.size() == 8);
    CHECK(node.coord.vec[0] == Approx(-0.0030908182083252632));
    CHECK(node.coord.vec[1] == Approx(-0.0034924296469496137));
    CHECK(node.coord.vec[2] == Approx(0.004027661974926579));
    CHECK(node.coord.vec[3] == Approx(0.009441736044369261));
    CHECK(node.coord.vec[4] == Approx(0.009120584954036386));
    CHECK(node.coord.vec[5] == Approx(0.005945711502389089));
    CHECK(node.coord.vec[6] == Approx(-0.005375781324440643));
    CHECK(node.coord.vec[7] == Approx(0.002691224249211683));

    CHECK(node.coord.error == Approx(0.33682855812450196));
    CHECK(node.coord.adjustment == Approx(-0.00008957602083707684));
    CHECK(node.coord.height == Approx(0.0005342546178363255));
}
