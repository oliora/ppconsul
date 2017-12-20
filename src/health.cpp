//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ppconsul/health.h"
#include "s11n_types.h"


namespace json11 {
    inline void load(const json11::Json& src, ppconsul::health::NodeServiceChecks& dst)
    {
        using ppconsul::s11n::load;

        load(src, std::get<0>(dst), "Node");
        load(src, std::get<1>(dst), "Service");
        load(src, std::get<2>(dst), "Checks");
    }
}

namespace ppconsul { namespace health {
    namespace impl {
        std::vector<CheckInfo> parseCheckInfos(const std::string& json)
        {
            return s11n::parseJson<std::vector<CheckInfo>>(json);
        }

        std::vector<NodeServiceChecks> parseService(const std::string& json)
        {
            return s11n::parseJson<std::vector<NodeServiceChecks>>(json);
        }
    }
}}

