//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
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

            void operator() (const GrpcCheck& c) const
            {
                dst()["grpc"] = c.url;
                dst()["interval"] = to_json(c.interval);
                if (c.timeout != decltype(c.timeout)::zero())
                    dst()["timeout"] = to_json(c.timeout);
                dst ()["DeregisterCriticalServiceAfter"] = to_json(c.deregisterCriticalServiceAfter);
                dst()["ServiceID"] = c.serviceID;
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

    std::pair<Config, Member> parseSelf(const std::string& json)
    {
        return s11n::parseJson<std::pair<Config, Member>>(json);
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
                { "Notes", service.check->notes }
            };

            save(check_o, service.check->params);

            o["Check"] = check_o;
        }

        return Json(std::move(o)).dump();
    }
}
}}
