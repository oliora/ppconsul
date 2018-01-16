//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ppconsul/config.h"
#include "ppconsul/error.h"
#include <json11/json11.hpp>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <string>


namespace ppconsul { namespace s11n {

    using json11::Json;

    namespace detail {
        inline Json parse_json(const std::string &s)
        {
            std::string err;
            auto obj = Json::parse(s, err);
            if (!err.empty())
                throw FormatError(std::move(err));
            return obj;
        }
    }

    inline void load(const Json& src, uint16_t& dst)
    {
        dst = static_cast<int>(src.int_value());
    }

    inline void load(const Json& src, bool& dst)
    {
        dst = static_cast<int>(src.bool_value());
    }

    inline void load(const Json& src, int& dst)
    {
        dst = static_cast<int>(src.int_value());
    }

    inline void load(const Json& src, uint64_t& dst)
    {
        // TODO: support full precision of uint64_t in json11
        dst = static_cast<uint64_t>(src.number_value());
    }

    inline void load(const Json& src, std::string& dst)
    {
        dst = src.string_value();
    }

    template<class T>    
    void load(const Json& src, std::vector<T>& dst)
    {
        const auto& arr = src.array_items();
        
        dst.clear();
        dst.reserve(arr.size());

        for (const auto& i : arr)
        {
            T t;
            load(i, t);
            dst.push_back(std::move(t));
        }
    }

    template<class T>
    void load(const Json& src, std::unordered_set<T>& dst)
    {
        const auto& arr = src.array_items();

        dst.clear();
        dst.reserve(arr.size());

        for (const auto& i : arr)
        {
            T t;
            load(i, t);
            dst.insert(std::move(t));
        }
    }

    template<class T>
    void load(const Json& src, std::unordered_map<std::string, T>& dst)
    {
        const auto& obj = src.object_items();

        dst.clear();

        for (const auto& i : obj)
        {
            T t;
            load(i.second, t);
            dst.emplace(i.first, std::move(t));
        }
    }

    template<class T>
    void load(const Json& src, T& dst, const char *name)
    {
        load(src[name], dst);
    }

    template<class T>
    T parseJson(const std::string& jsonStr)
    {
        using namespace s11n;
        
        auto obj = detail::parse_json(jsonStr);
        T t;
        load(obj, t);
        return t;
    }
}}
