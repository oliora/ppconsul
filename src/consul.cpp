//  Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "ppconsul/consul.h"
#include "ppconsul/helpers.h"
#include "http_client_impl_netlib.h"


namespace ppconsul {

    namespace {
        // Creates base URL like "http://<addr>" if 'addr' has no scheme specified
        // or just "<addr>" if 'addr' contains any scheme.
        inline std::string makeAddr(const std::string& addr)
        {
            if (addr.find("://") != std::string::npos)
                return addr;
            else
                return "http://" + addr;
        }
    }

    namespace impl {
        inline std::string makeUrl(const std::string& addr, const std::string& path, const Parameters& parameters)
        {
            auto query = parameters.query();

            std::string r;
            r.reserve(addr.size() + path.size() + query.size() + 1); // 1 is for query prefix '?'

            r += addr;
            r += path;
            if (!query.empty())
            {
                r += '?';
                r += query;
            }

            return r;
        }
    }

    const char *BadStatus::what() const PPCONSUL_NOEXCEPT
    {
        if (m_what.empty())
        {
            if (!m_message.empty())
            {
                m_what = helpers::format("%s [%03d %s]",
                    m_message.c_str(),
                    m_status.code(),
                    m_status.message().c_str());
            }
            else
            {
                m_what = helpers::format("%03d %s",
                    m_status.code(),
                    m_status.message().c_str());
            }
        }

        return m_what.c_str();
    }

    Consul::Consul(const std::string& dataCenter, const std::string& addr)
    : m_addr(makeAddr(addr))
    , m_client(new impl::HttpClient())
    {
        if (!dataCenter.empty())
            m_defaultParams.update("dc", dataCenter);
    }
    
    Consul::~Consul()
    {}

    inline std::string Consul::makeUrl(const std::string& path, const Parameters& params) const
    {
        Parameters p = m_defaultParams;
        p.update(params);

        return impl::makeUrl(m_addr, path, p);
    }

    std::string Consul::get(http::Status& status, const std::string& path, const Parameters& params)
    {
        return m_client->get(status, makeUrl(path, params));
    }

    std::string Consul::put(http::Status& status, const std::string& path, const std::string& body, const Parameters& params)
    {
        return m_client->put(status, makeUrl(path, params), body);
    }

    std::string Consul::del(http::Status& status, const std::string& path, const Parameters& params)
    {
        return m_client->del(status, makeUrl(path, params));
    }

}
