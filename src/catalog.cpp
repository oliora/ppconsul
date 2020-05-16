//  Copyright (c) 2014-2020 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include "ppconsul/catalog.h"
#include "s11n_types.h"


namespace json11 {
    void load(const json11::Json& src, ppconsul::catalog::NodeService& dst)
    {
        using ppconsul::s11n::load;

        load(src, dst.first);
        load(src, dst.second.id, "ServiceID");
        load(src, dst.second.name, "ServiceName");
        load(src, dst.second.address, "ServiceAddress");
        load(src, dst.second.port, "ServicePort");
        load(src, dst.second.tags, "ServiceTags");
        load(src, dst.second.meta, "ServiceMeta");
    }

    void load(const json11::Json& src, ppconsul::catalog::NodeServices& dst)
    {
        using ppconsul::s11n::load;

        load(src, dst.first, "Node");
        load(src, dst.second, "Services");
    }
}

namespace ppconsul { namespace catalog {
    
namespace impl {

    StringList parseDatacenters(const std::string& json)
    {
        return s11n::parseJson<StringList>(json);
    }

    std::vector<Node> parseNodes(const std::string& json)
    {
        return s11n::parseJson<std::vector<Node>>(json);
    }

    NodeServices parseNode(const std::string& json)
    {
        return s11n::parseJson<NodeServices>(json);
    }

    std::map<std::string, Tags> parseServices(const std::string& json)
    {
        return s11n::parseJson<std::map<std::string, Tags>>(json);
    }

    std::vector<NodeService> parseService(const std::string& json)
    {
        return s11n::parseJson<std::vector<NodeService>>(json);
    }

}
}}
