//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include "ppconsul/catalog.h"
#include "s11n_types.h"

namespace ppconsul { namespace s11n {
    void load(const s11n::Json& src, ppconsul::catalog::NodeService& dst)
    {
        load(src, dst.first);
        load(src, dst.second.name, "ServiceName");
        load(src, dst.second.port, "ServicePort");
        load(src, dst.second.tags, "ServiceTags");
        load(src, dst.second.id, "ServiceID");
    }

    void load(const s11n::Json& src, ppconsul::catalog::NodeServices& dst)
    {
        load(src, dst.first, "Node");
        load(src, dst.second, "Services");
    }
}}

namespace ppconsul { namespace catalog {
    
namespace impl {

    std::vector<std::string> parseDatacenters(const std::string& json)
    {
        return s11n::parseJson<std::vector<std::string>>(json);
    }

    std::vector<Node> parseNodes(const std::string& json)
    {
        return s11n::parseJson<std::vector<Node>>(json);
    }

    NodeServices parseNode(const std::string& json)
    {
        return s11n::parseJson<NodeServices>(json);
    }

    std::unordered_map<std::string, Tags> parseServices(const std::string& json)
    {
        return s11n::parseJson<std::unordered_map<std::string, Tags>>(json);
    }

    std::vector<NodeService> parseService(const std::string& json)
    {
        return s11n::parseJson<std::vector<NodeService>>(json);
    }

}
}}
