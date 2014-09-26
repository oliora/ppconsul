#include "http_client.h"
#include <boost/network/protocol/http/client.hpp>


namespace ppconsul { namespace http {

    namespace http = boost::network::http;

    const char *BadStatus::what() const PPCONSUL_NOEXCEPT
    {
        char prefix[] = "HTTP ";

        if (m_what.empty())
        {
            // 3 is for status code + space - NULL of prefix
            m_what.reserve((sizeof(prefix) / sizeof(prefix[0])) + m_status.message().size() + 3);
            m_what = prefix;
            m_what += std::to_string(m_status.code());
            m_what += ' ';
            m_what += m_status.message();
        }

        return m_what.c_str();
    }

    std::string Parameters::query() const
    {
        // TODO: encode parameters?

        if (m_values.empty())
            return {};

        std::string r;

        // 2 is for '=' and '&'. -1 is because first param has no '&'
        auto len = std::accumulate(m_values.begin(), m_values.end(), -1, [](int s, const Parameter& p) {
            return s + p.m_name.size() + p.m_value.size() + 2;
        });

        r.reserve(len - 1);

        auto it = m_values.begin();

        r += it->m_name;
        r += '=';
        r += it->m_value;

        for(;it != m_values.end(); ++it)
        {
            r += '&';
            r += it->m_name;
            r += '=';
            r += it->m_value;
        }

        return r;
    }

    class Client::Impl
    {
    public:
        http::client m_client;
    };

    Client::Client(const char *host)
    : m_impl(new Impl)
    , m_host(host)
    {}

    Client::~Client()
    {}

    std::string Client::get(Status& status, const char *path, const Parameters& parameters)
    {
        http::client::request request(createUri(path, parameters));
        auto response = m_impl->m_client.get(request);
        status = Status(response.status(), response.status_message());
        return response.body();
    }

    void Client::put(Status& status, const char *path, const std::string& body, const Parameters& parameters)
    {
        http::client::request request(createUri(path, parameters));
        // TODO: add content type header?
        request.body(body);
        auto response = m_impl->m_client.delete_(request);
        status = Status(response.status(), response.status_message());
    }

    void Client::del(Status& status, const char *path, const Parameters& parameters)
    {
        http::client::request request(createUri(path, parameters));
        auto response = m_impl->m_client.delete_(request);
        status = Status(response.status(), response.status_message());
    }

}}

