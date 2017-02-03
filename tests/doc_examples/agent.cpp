#include "ppconsul/agent.h"


void agent_service()
{
    // ### Register, deregister and report the state of your service in Consul:

    using ppconsul::Consul;
    using namespace ppconsul::agent;

    // Create a consul client with using of a default address ("127.0.0.1:8500") and default DC
    Consul consul;
    // We need the 'agent' endpoint for a service registration
    Agent agent(consul);

    // Register a service with associated HTTP check:
    agent.registerService(
        kw::name = "my-service",
        kw::port = 9876,
        kw::tags = {"tcp", "super_server"},
        kw::check = HttpCheck{"http://localhost:80/", std::chrono::seconds(2)}
    );

    // Unregister service
    agent.deregisterService("my-service");

    // ...

    // Register a service with TTL
    agent.registerService(
                          kw::name = "my-service",
                          kw::port = 9876,
                          kw::id = "my-service-1",
                          kw::check = TtlCheck{std::chrono::seconds(5)}
                          );

    // Report service is OK
    agent.servicePass("my-service-1");
    
    // Report service is failed
    agent.serviceFail("my-service-1", "Disk is full");
}
