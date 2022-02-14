//  Copyright (c) 2014-2020 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include "ppconsul/coordinate.h"
#include "s11n_types.h"

namespace ppconsul { namespace coordinate {
    void load(const json11::Json& src, ppconsul::coordinate::Node& dst)
    {
        using ppconsul::s11n::load;

        load(src, dst.node, "Node");
        load(src, dst.segment, "Segment");
        load(src, dst.coord, "Coord");
    }

    void load(const json11::Json& src, ppconsul::coordinate::Datacenter& dst)
    {
        using ppconsul::s11n::load;

        load(src, dst.datacenter, "Datacenter");
        load(src, dst.areaId, "AreaID");
        load(src, dst.coordinates, "Coordinates");
    }

namespace impl {

    std::vector<Datacenter> parseDatacenters(const std::string& json)
    {
        return s11n::parseJson<std::vector<Datacenter>>(json);
    }

    std::vector<Node> parseNodes(const std::string& json)
    {
        return s11n::parseJson<std::vector<Node>>(json);
    }

    Node parseNode(const std::string& json)
    {
        return s11n::parseJson<Node>(json);
    }

}

}}
