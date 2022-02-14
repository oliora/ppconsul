//  Copyright (c) 2014-2020 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include "ppconsul/agent.h"
#include "s11n_types.h"

namespace ppconsul { namespace agent {
    using s11n::load;

    inline void load(const json11::Json& src, SelfInfo& dst)
    {
        load(src, dst.config, "Config");
        load(src, dst.member, "Member");
        load(src, dst.coord, "Coord");
    }

    void load(const s11n::Json& src, Config& dst)
    {
        load(src, dst.datacenter, "Datacenter");
        load(src, dst.nodeName, "NodeName");
        load(src, dst.nodeId, "NodeID");
        load(src, dst.server, "Server");
        load(src, dst.revision, "Revision");
        load(src, dst.version, "Version");
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


namespace impl {

    namespace {
        using s11n::to_json;

        s11n::Json::array to_json(const StringList& strings)
        {
            return s11n::Json::array(strings.begin(), strings.end());
        }

        s11n::Json::array to_json(const Tags& tags)
        {
            return s11n::Json::array(tags.begin(), tags.end());
        }

        s11n::Json::object to_json(const Metadata& meta)
        {
            return s11n::Json::object(meta.begin(), meta.end());
        }

        struct CheckConfigSaver: boost::static_visitor<>
        {
            CheckConfigSaver(s11n::Json::object& dst): dst_(&dst) {}

            void operator() (const TtlCheck& c) const
            {
                dst()["ttl"] = to_json(c.ttl);
            }

            void operator() (const ScriptCheck& c) const
            {
                dst()["script"] = c.script;
                dst()["interval"] = to_json(c.interval);
            }

            void operator() (const CommandCheck& c) const
            {
                dst()["args"] = to_json(c.args);
                dst()["interval"] = to_json(c.interval);
            }

            void operator() (const HttpCheck& c) const
            {
                dst()["http"] = c.url;
                dst()["interval"] = to_json(c.interval);
                if (c.timeout != decltype(c.timeout)::zero())
                    dst()["timeout"] = to_json(c.timeout);
            }

            void operator() (const TcpCheck& c) const
            {
                dst()["tcp"] = c.address;
                dst()["interval"] = to_json(c.interval);
                if (c.timeout != decltype(c.timeout)::zero())
                    dst()["timeout"] = to_json(c.timeout);
            }

            void operator() (const DockerScriptCheck& c) const
            {
                dst()["docker_container_id"] = c.containerId;
                dst()["shell"] = c.shell;
                dst()["script"] = c.script;
                dst()["interval"] = to_json(c.interval);
            }

            void operator() (const DockerCommandCheck& c) const
            {
                dst()["docker_container_id"] = c.containerId;
                dst()["shell"] = c.shell;
                dst()["args"] = to_json(c.args);
                dst()["interval"] = to_json(c.interval);
            }

            s11n::Json::object& dst() const { return *dst_; }

        private:
            s11n::Json::object* dst_;
        };


        void save(s11n::Json::object& dst, const CheckParams& c)
        {
            boost::apply_visitor(CheckConfigSaver(dst), c);
        }
    }
    
    std::vector<Member> parseMembers(const std::string& json)
    {
        return s11n::parseJson<std::vector<Member>>(json);
    }

    SelfInfo parseSelf(const std::string& json)
    {
        return s11n::parseJson<SelfInfo>(json);
    }

    std::map<std::string, CheckInfo> parseChecks(const std::string& json)
    {
        return s11n::parseJson<std::map<std::string, CheckInfo>>(json);
    }

    std::map<std::string, ServiceInfo> parseServices(const std::string& json)
    {
        return s11n::parseJson<std::map<std::string, ServiceInfo>>(json);
    }

    std::string checkRegistrationJson(const CheckRegistrationData& check)
    {
        using s11n::Json;

        Json::object o {
            { "ID", check.id },
            { "Name", check.name },
            { "Notes", check.notes }
        };
        if (check.deregisterCriticalServiceAfter != decltype(check.deregisterCriticalServiceAfter)::zero()) {
            o["DeregisterCriticalServiceAfter"] =  to_json(check.deregisterCriticalServiceAfter);
        }

        save(o, check.params);

        return Json(std::move(o)).dump();
    }

    std::string serviceRegistrationJson(const ServiceRegistrationData& service)
    {
        using s11n::Json;

        Json::object o {
            { "ID", service.id },
            { "Name", service.name },
            { "Tags", to_json(service.tags) },
            { "Meta", to_json(service.meta) },
            { "Address", service.address },
            { "Port", service.port }
        };

        if (service.check)
        {
            Json::object check_o {
                { "Notes", service.check->notes },
            };

            if (service.check->deregisterCriticalServiceAfter != decltype(service.check->deregisterCriticalServiceAfter)::zero()) {
            check_o["DeregisterCriticalServiceAfter"] =  to_json(service.check->deregisterCriticalServiceAfter);
        }
            save(check_o, service.check->params);

            o["Check"] = check_o;
        }

        return Json(std::move(o)).dump();
    }
}
}}
