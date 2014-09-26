#include <json11.hpp>
#include <iostream>
#include "http_client.h"


int main(int argc, char *argv[])
{
    try
    {
        std::cout << ppconsul::http::Client().get(argv[1], {{"pretty", 1}}) << std::endl;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
