//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include "ppconsul/agent.h"
#include "s11n_types.h"


namespace json11 {
    inline void load(const json11::Json& src, std::pair<ppconsul::agent::Config, ppconsul::agent::Member>& dst)
    {
        using ppconsul::s11n::load;

        load(src, dst.first, "Config");
        load(src, dst.second, "Member");
    }
}


namespace ppconsul { namespace agent {
    using s11n::load;

    void load(const s11n::Json& src, CheckStatus& dst)
    {
        const auto& s = src.string_value();

        if (s == "passing")
            dst = CheckStatus::Passing;
        else if (s == "warning")
            dst = CheckStatus::Warning;
        else if (s == "critical")
            dst = CheckStatus::Failed;
        else
            dst = CheckStatus::Unknown;
    }

    void load(const s11n::Json& src, Config& dst)
    {
        // TODO
    }

    void load(const s11n::Json& src, Member& dst)
    {
        load(src, dst.name, "Name");
        load(src, dst.address, "Addr");
        load(src, dst.port, "Port");
        load(src, dst.tags, "Tags");
        load(src, dst.status, "Status");
        load(src, dst.protocolMin, "ProtocolMin");
        load(src, dst.protocolMax, "ProtocolMax");
        load(src, dst.protocolCur, "ProtocolCur");
        load(src, dst.delegateMin, "DelegateMin");
        load(src, dst.delegateMax, "DelegateMax");
        load(src, dst.delegateCur, "DelegateCur");
    }

    void load(const s11n::Json& src, CheckInfo& dst)
    {
        load(src, dst.id, "CheckID");
        load(src, dst.node, "Node");
        load(src, dst.name, "Name");
        load(src, dst.status, "Status");
        load(src, dst.notes, "Notes");
        load(src, dst.output, "Output");
        load(src, dst.serviceId, "ServiceID");
        load(src, dst.serviceName, "ServiceName");
    }

namespace impl {

    namespace {
        std::string to_json(const std::chrono::seconds& seconds)
        {
            std::ostringstream os;
            os << seconds.count() << "s";
            return os.str();
        }

        s11n::Json::array to_json(const Tags& tags)
        {
            return s11n::Json::array(tags.begin(), tags.end());
        }
    }
    
    std::vector<Member> parseMembers(const std::string& json)
    {
        return s11n::parseJson<std::vector<Member>>(json);
    }

    std::pair<Config, Member> parseSelf(const std::string& json)
    {
        return s11n::parseJson<std::pair<Config, Member>>(json);
    }

    std::unordered_map<std::string, CheckInfo> parseChecks(const std::string& json)
    {
        return s11n::parseJson<std::unordered_map<std::string, CheckInfo>>(json);
    }

    std::unordered_map<std::string, Service> parseServices(const std::string& json)
    {
        return s11n::parseJson<std::unordered_map<std::string, Service>>(json);
    }

    std::string checkRegistrationJson(const Check& check, const std::chrono::seconds& ttl)
    {
        using s11n::Json;

        return Json(Json::object{
            { "ID", check.id },
            { "Name", check.name },
            { "Notes", check.notes },
            { "TTL", to_json(ttl) }
        }).dump();
    }

    std::string checkRegistrationJson(const Check& check, const std::string& script, const std::chrono::seconds& interval)
    {
        using s11n::Json;

        return Json(Json::object{
            { "ID", check.id },
            { "Name", check.name },
            { "Notes", check.notes },
            { "Script", script },
            { "Interval", to_json(interval) }
        }).dump();
    }

    std::string serviceRegistrationJson(const Service& service)
    {
        using s11n::Json;

        return Json(Json::object{
            { "ID", service.id },
            { "Name", service.name },
            { "Tags", to_json(service.tags) },
            { "Port", service.port }
        }).dump();
    }

    std::string serviceRegistrationJson(const Service& service, const std::chrono::seconds& ttl)
    {
        using s11n::Json;

        return Json(Json::object{
            { "ID", service.id },
            { "Name", service.name },
            { "Tags", to_json(service.tags) },
            { "Port", service.port },
            { "Check", Json::object{
                { "TTL", to_json(ttl) }
            }}
        }).dump();
    }

    std::string serviceRegistrationJson(const Service& service, const std::string& script, const std::chrono::seconds& interval)
    {
        using s11n::Json;

        return Json(Json::object{
            { "ID", service.id },
            { "Name", service.name },
            { "Tags", to_json(service.tags) },
            { "Port", service.port },
            { "Check", Json::object{
                { "Script", script },
                { "Interval", to_json(interval) }
            } }
        }).dump();
    }

}
}}
