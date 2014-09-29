#include <json11.hpp>
#include <iostream>
#include "ppconsul/ppconsul.h"


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " url-to-fetch" << std::endl;
        return 2;
    }
        
    try
    {
        std::cout << ppconsul::Consul().get(argv[1]) << std::endl;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
