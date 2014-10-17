//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include "ppconsul/agent.h"
#include "json.h"


namespace ppconsul { namespace agent {
namespace impl {

    void parseJson(std::vector<Member>& out, const std::string& json)
    {}

    void parseJson(std::pair<Config, Member>& out, const std::string& json)
    {}

    void parseJson(std::map<std::string, Check>& out, const std::string& json)
    {}

    void parseJson(std::map<std::string, Service>& out, const std::string& json)
    {}

    std::string checkRegistrationJson(const std::string& name, const std::chrono::seconds& ttl, const std::string& notes, const std::string& id)
    {
        return{};
    }

    std::string checkRegistrationJson(const std::string& name, const std::string& script, const std::chrono::seconds& interval, const std::string& notes, const std::string& id)
    {
        return{};
    }

    std::string serviceRegistrationJson(const Service& service)
    {
        return{};
    }

    std::string serviceRegistrationJson(const Service& service, const std::chrono::seconds& ttl)
    {
        return{};
    }

    std::string serviceRegistrationJson(const Service& service, const std::string& script, const std::chrono::seconds& interval)
    {
        return{};
    }

}
}}
