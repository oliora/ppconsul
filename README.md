# Ppconsul

*Version 0.1*

A C++ client library for [Consul](http://consul.io). Consul is a distributed tool for discovering and configuring services in your infrastructure.

The goal of Ppconsul is to:
* Fully cover version 1 of Consul [HTTP API](http://www.consul.io/docs/agent/http.html). Please check the current [implementation status](status.md).
* Provide simple, modular and effective API based on C++11.
* Support different platforms. At the moment, Linux, Windows and macOS platforms supported.
* Cover all the code with automated tests.

Note that this project is under development and doesn't promise a stable interface.

Library tests are currently running against **Consul v1.0.6**. Library is known to work with Consul starting from version **0.4** (earlier versions might work as well but has never been tested) although some tests fail for older versions because of backward incompatible changes in Consul.

The library is written in C++11 and requires a quite modern compiler. Currently it's compiled with:
* macOS: Clang 9 (Xcode 9.2)
* Ubuntu Linux: GCC 5.3, GCC 4.9, GCC 4.8.2 all with stdlibc++
* Windows: Visual Studio 2013 Update 3

Newer versions of specified compilers should work fine.
Older versions of Clang should work fine (at least Clang 5 and newer).
Versions of GCC prior to 4.8 and Visual Studio prior to 2013 are known to fail.

The library depends on:
* [Boost](http://www.boost.org/) 1.55 or later. Ppconsul needs only headers with one exception: using of GCC 4.8 requires Boost.Regex library because [regular expressions are broken in GCC 4.8](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631).
* [libCURL](http://curl.haxx.se/libcurl/) ~~or [C++ Network Library](http://cpp-netlib.org/) (aka cpp-netlib)~~ to do HTTP/HTTPS. **Note that C++ Network Library support is removed. If you care, please share your thoughts in [Keep support for C++ Network Library](https://github.com/oliora/ppconsul/issues/11).**

The library includes code of the following 3rd party libraries (check `ext` directory):
* [json11](https://github.com/dropbox/json11) library to deal with JSON.
* [libb64](http://libb64.sourceforge.net/) library for base64 decoding.

For unit tests, the library uses [Catch](https://github.com/philsquared/Catch) framework. Many thanks to Phil Nash for this great product.

## Warm Up Examples

Register, deregister and report the state of your service in Consul:


```cpp
#include "ppconsul/agent.h"

using ppconsul::Consul;
using namespace ppconsul::agent;

// Create a consul client that uses default local endpoint `http://127.0.0.1:8500` and default data center
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

...

// Unregister service
agent.deregisterService("my-service");

...

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
```

Determine raft leader (or lack thereof) and raft peers:

```cpp
#include "ppconsul/status.h"

using ppconsul::Consul;
using namespace ppconsul::status;

// Create a consul client that uses default local endpoint `http://127.0.0.1:8500` and default data center
Consul consul;

// We need the status endpoint
Status status(consul);

// Determine whether a leader has been elected
bool isLeaderElected = status.isLeaderElected();

// Determine the actual raft leader
auto leader = status.leader();

// Determine the raft peers
auto peers = status.peers();
```

Use Key-Value storage:

```cpp
#include "ppconsul/kv.h"

using ppconsul::Consul;
using ppconsul::Consistency;
using namespace ppconsul::kv;

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
```

Blocking query to Key-Value storage:

```cpp
// Get key+value+metadata item
KeyValue item = kv.item("status.last-event-id");

// Wait for the item change for no more than 1 minute:
item = kv.item("status.last-event-id", kw::block_for = {std::chrono::minutes(1), item.modifyIndex});

// If key exists, print it:
if (item)
    std::cout << item.key << "=" << item.value << "\n";
```

Connect to Consul via HTTPS (TLS/SSL, whatever you call it):

```cpp
#include "ppconsul/consul.h"

using namespace ppconsul;

Consul consul("https://localhost:8080",
              kw::tls::cert="path/to/cert",
              kw::tls::key="path/to/private/key",
              kw::tls::ca_info="path/to/ca/cert");

// Use consul ...
```

## Documentation
TBD

## How To Build

### Get Dependencies
* Get C++11 compatible compiler. See above for the list of supported compilers.
* Install [CMake](http://www.cmake.org/) 3.1 or above.
* Install [Boost](http://www.boost.org/) 1.55 or later. You need compiled Boost libraries if you going to use cpp-netlib or GCC 4.8, otherwise you need Boost headers only.
* Install [libCURL](http://curl.haxx.se/libcurl/) (any version should be fine).
* If you want to run Ppconsul tests then install [Consul](http://consul.io) 0.4 or newer. *I recommend 0.7 or newer since it's easier to run them in development mode.*

### Build

Prepare project:
```bash
mkdir workspace
cd workspace
cmake ..
```

* To change where the build looks for Boost, pass `-DBOOST_ROOT=<path_to_boost>` parameter to CMake or set `BOOST_ROOT` environment variable.
* To change where the build looks for libCURL, pass `-DCURL_ROOT=<path_to_curl>` parameter to CMake or set `CURL_ROOT` environment variable.
* To change the default install location, pass `-DCMAKE_INSTALL_PREFIX=<prefix>` parameter.
* To build Ppconsul as static library, pass `-DBUILD_STATIC_LIB=ON` parameter.
  Note that in this case you have to link with json11 static library as well (json11 library is build as part of Ppconsul build.)

**Note that on Windows Ppconsul can be built as static library only (and that's the default build mode on Windows), see issue [Allow to build Ppconsul as dynamic library on Windows](https://github.com/oliora/ppconsul/issues/25)**.

*Note about -G option of CMake to choose you favourite IDE to generate project files for.*

Build:
```bash
cmake --build . --config Release
```

If Makefile generator was used then you can also do:
```bash
make
```

## How to Install

Build it first as described above then run
```bash
cmake --build . --config Release --target install
```

If Makefile generator was used then you can also do:
```bash
make install
```
## How to Use

Build and install it first as described above.

When installed, the library can be simply used in any CMake-based project as following:

```
find_package(ppconsul)
add_executable(<your_app> ...)
target_link_libraries(<your_app> ppconsul)
```

## How To Run Tests

### Run Consul

For Consul 0.9 and above:
```bash
consul agent -dev -datacenter=ppconsul_test -enable-script-checks=true
```

For Consul 0.7 and 0.8:
```bash
consul agent -dev -datacenter=ppconsul_test
```

For earlier version of Consul follow its documentation on how to run it with `ppconsul_test` datacenter.

### Run Tests

Build it first as described above then run
```bash
ctest -C Release
```

If Makefile generator was used then you can also do:
```bash
make test
```

There are the following environment variable to configure tests:

|Name|Default Value|Description|
|----|-------------|-----------|
|`PPCONSUL_TEST_ADDR`|"127.0.0.1:8500"|The Consul network address|
|`PPCONSUL_TEST_DC`|"ppconsul_test"|The Consul datacenter|
|`PPCONSUL_TEST_LEADER_ADDR`|"127.0.0.1:8300"|The Consul raft leader address|

**Never set `PPCONSUL_TEST_DC` into a datacenter that you can't throw away because Ppconsul tests will screw it up in many different ways.**

### Known Problems

Sometimes catalog tests failed on assertion `REQUIRE(index1 == resp1.headers().index());`. In this case, just rerun the tests.
The reason for the failure is Consul's internal idempotent write which cause a spurious wakeup of waiting blocking query. Check the critical note under the blocking queries documentation at https://www.consul.io/docs/agent/http.html.

## Found a bag? Got a feature request? Need help with Ppconsul?
Use [issue tracker](https://github.com/oliora/ppconsul/issues) or/and drop an email to [oliora](https://github.com/oliora).

## Contribute
First of all, welcome on board!

To contribute, please fork this repo, make your changes and create a pull request.

## License
The library released under [Boost Software License v1.0](http://www.boost.org/LICENSE_1_0.txt).
