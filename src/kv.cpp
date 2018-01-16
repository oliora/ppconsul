//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include "ppconsul/kv.h"
#include "s11n.h"


namespace ppconsul { namespace kv {

    void load(const s11n::Json& src, KeyValue& dst)
    {
        using s11n::load;

        std::string value;

        load(src, dst.createIndex, "CreateIndex");
        load(src, dst.modifyIndex, "ModifyIndex");
        load(src, dst.lockIndex, "LockIndex");
        load(src, dst.key, "Key");
        load(src, dst.flags, "Flags");
        load(src, value, "Value");
        dst.value = helpers::decodeBase64(value);
        load(src, dst.session, "Session");
    }

namespace impl {

    std::vector<std::string> parseKeys(const std::string& resp)
    {
        return s11n::parseJson<std::vector<std::string>>(resp);
    }

    std::vector<KeyValue> parseValues(const std::string& resp)
    {
        return s11n::parseJson<std::vector<KeyValue>>(resp);
    }
}
}}
