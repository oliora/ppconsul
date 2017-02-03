#include "ppconsul/kv.h"


void kv()
{

    // ### Use Key-Value storage:

    using ppconsul::Consul;
    using ppconsul::Consistency;
    using namespace ppconsul::kv;

    // Create a consul client with using of a default address ("127.0.0.1:8500") and default DC
    Consul consul;

    // We need the 'kv' endpoint
    Kv kv(consul);

    // Read the value of a key from the storage
    std::string something = kv.get("settings.something", "default-value");

    // Read the value of a key from the storage with consistency mode specified
    something = kv.get("settings.something", "default-value", kw::consistency = Consistency::Consistent);

    // Erase a key from the storage
    kv.erase("settings.something-else");

    // Set the value of a key
    kv.set("settings.something", "new-value");
}

void kv_blocking_query()
{
    // ### Blocking query to Key-Value storage:

    using ppconsul::Consul;
    using namespace ppconsul::kv;

    // Create a consul client with using of a default address ("127.0.0.1:8500") and default DC
    Consul consul;

    // We need the 'kv' endpoint
    Kv kv(consul);
    
    // Get key+value+metadata item
    KeyValue item = kv.item("status.last-event-id");

    // Wait for the item change for no more than 1 minute:
    item = kv.item("status.last-event-id", kw::block_for = {std::chrono::minutes(1), item.modifyIndex});

    // If key exists, print it:
    if (item)
        std::cout << item.key << "=" << item.value << "\n";
}