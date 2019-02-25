#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>

#include "../include/ppconsul/ppconsul.h"



void kv_query()
{
    // ### Blocking query to Key-Value storage:

    using ppconsul::Consul;
    using namespace ppconsul::kv;

    // Create a consul client that uses default local endpoint `http://127.0.0.1:8500` and default data center
    Consul consul;

    // We need the 'kv' endpoint
    Kv kv(consul);
    kv.set("backend/test1", "1");
    kv.set("backend/test2", "2");
    // Get key+value+metadata item
    std::string resultTest1 = kv.get("backend/test1", "default-value-test1");
    std::cout << "resultTest1: " << resultTest1 << std::endl;
}

void agent_service()
{
    // ### Register, deregister and report the state of your service in Consul:

    using ppconsul::Consul;
    using namespace ppconsul::agent;

    // Create a consul client that uses default local endpoint `http://127.0.0.1:8500` and default data center
    std::shared_ptr<Consul> consul = std::make_shared<Consul>();
    // We need the 'agent' endpoint for a service registration
    Agent agent(*consul.get());
    std::cout << "Before reg servcie" << std::endl;
    // Register a service with associated HTTP check:
    agent.registerService(
            kw::name = "aaaa-pe-service",
            kw::port = 6666,
            kw::tags = {"test", "aaaa-pe"},
            kw::check = GrpcCheck{"192.168.20.168:50051", std::chrono::seconds(2)}//!!!!change to your host IP
    );

//    agent.registerCheck("aa-register-check", GrpcCheck{"192.168.20.168:50051", std::chrono::seconds(2)});

    kv_query();

    std::this_thread::sleep_for(std::chrono::milliseconds(20000));
    // Unregister service
    agent.deregisterService("aaaa-pe-service");
}



int main() {
    agent_service();
    return 0;
}