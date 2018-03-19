//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include "ppconsul/status.h"
#include "s11n_types.h"

namespace ppconsul { namespace status {

namespace impl {

    std::string parseLeader(const std::string& json)
    {
        return s11n::parseJson<std::string>(json);
    }

    std::vector<std::string> parsePeers(const std::string& json)
    {
        return s11n::parseJson<std::vector<std::string>>(json);
    }

}
}}
