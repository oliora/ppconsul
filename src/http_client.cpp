#include "http_client.h"


namespace ppconsul { namespace http {

    class Client::Impl {};

    Client::Client(const char *host)
    {}

    std::string Client::get(Status& status, const char *path, const Options& options)
    {
        return {};
    }

    void Client::put(Status& status, const char *path, const std::string& body, const Options& options)
    {}

    void Client::del(Status& status, const char *path, const Options& options)
    {}

}}

