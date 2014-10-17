//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include "ppconsul/kv.h"
#include "json.h"


namespace ppconsul { namespace kv {
namespace impl {
    std::vector<std::string> parseKeys(const std::string& resp)
    {
        auto obj = json::parse_json(resp);

        std::vector<std::string> r;
        r.reserve(obj.array_items().size());

        for (const auto& i : obj.array_items())
            r.push_back(i.string_value());
        return r;
    }

    std::vector<KeyValue> parseValues(const std::string& resp)
    {
        using namespace json;

        auto obj = parse_json(resp);

        std::vector<KeyValue> r;
        r.reserve(obj.array_items().size());

        for (const auto& i : obj.array_items())
        {
            const auto& o = i.object_items();

            KeyValue kv;
            kv.createIndex = uint64_value(o.at("CreateIndex"));
            kv.modifyIndex = uint64_value(o.at("ModifyIndex"));
            kv.lockIndex = uint64_value(o.at("LockIndex"));
            kv.key = o.at("Key").string_value();
            kv.flags = uint64_value(o.at("Flags"));
            kv.value = helpers::decodeBase64(o.at("Value").string_value());
            kv.session = i["Session"].string_value(); // generalize getting of optional values

            r.push_back(std::move(kv));
        }
        return r;
    }
}
}}
