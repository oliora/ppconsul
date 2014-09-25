#include <boost/network/protocol/http/client.hpp>
#include <json11.hpp>
#include <iostream>

namespace ppconsul {

    class Client
    {
    public:
        explicit Client(const char *url = "http://localhost:8500")
        : m_url(url)
        {}

    private:
        std::string m_url;
    };
}


int main(int argc, char *argv[])
{
    using namespace boost::network;

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " [url]" << std::endl;
        return 1;
    }

    http::client client;
    http::client::request request(argv[1]);
    request << header("Connection", "close");
    http::client::response response = client.get(request);
    std::cout << body(response) << std::endl;

    return 0;
}
