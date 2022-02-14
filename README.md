# Ppconsul

*Version 0.2*

A C++ client library for [Consul](http://consul.io). Consul is a distributed tool for discovering and configuring services in your infrastructure.

The goal of Ppconsul is to:
* Fully cover version 1 of Consul [HTTP API](http://www.consul.io/docs/agent/http.html). Please check the current [implementation status](status.md).
* Provide simple, modular and effective API based on C++11.
* Support different platforms. At the moment, Linux, Windows and macOS platforms supported.
* Cover all the code with automated tests.

Note that this project is under development and doesn't promise a stable interface.

Library tests are currently running against **Consul v1.11.1**. Library is known to work with Consul starting from version **0.4** (earlier versions might work as well but has never been tested) although some tests fail for older versions because of backward incompatible changes in Consul.

The library is written in C++11 and requires a quite modern compiler. Currently it's compiled with:
* macOS: Clang 11 (Xcode 11.3.1)
* Ubuntu Linux: GCC 7.4 with stdlibc++
* Windows: n/a

Oldest versions of compilers that should work (all with using C++11 standard)
- Clang 5
- GCC 4.8
- Visual Studio 2013

I try to support all modern compilers and platforms but I don't have resources to do extensive testing so from time to time something got broken on some platforms (mostly old GCC or Windows issues). Please create an issue if you discover a compilation error on your platform.

The library depends on:

* [git](https://git-scm.com/) (optional) to deduce correct .so version
* [Boost](http://www.boost.org/) 1.55 or later. Ppconsul needs only headers with one exception: using of GCC 4.8 requires Boost.Regex library because [regular expressions are broken in GCC 4.8](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631).
* [libCURL](http://curl.haxx.se/libcurl/) to do HTTP/HTTPS.

The library includes code of the following 3rd party libraries (check `ext` directory):

* [json11](https://github.com/dropbox/json11) library to deal with JSON.
* [libb64](http://libb64.sourceforge.net/) library for base64 decoding.

For unit tests, the library uses [Catch2](https://github.com/catchorg/Catch2) framework. Many thanks to Phil Nash for this great product.

## Warm Up Examples

### Register, deregister and report the state of your service in Consul:

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

### Determine raft leader (or lack thereof) and raft peers:

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

### Use Key-Value storage:

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

### Blocking query to Key-Value storage:

```cpp
// Get key+value+metadata item
KeyValue item = kv.item("status.last-event-id");

// Wait for the item change for no more than 1 minute:
item = kv.item("status.last-event-id", kw::block_for = {std::chrono::minutes(1), item.modifyIndex});

// If key exists, print it:
if (item)
    std::cout << item.key << "=" << item.value << "\n";
```

### Abort all [blocking] queries:

```cpp
Consul consul(kw::enable_stop = true); // Must be enabled at construction time
Kv kv(consul);

// Issue blocking queries, similarly to example above, on background threads etc.

// Stop all pending requests, e.g. at shutdown. No further requests can be done after this call.
consul.stop();
```

Call to `Consul::stop()` is irreversible: once it's done the `Consul` object is switched to the stopped state forever. This whole feature purpose is to gracefully abort ongoing blocking queries on application/component shutdown.

### Configure connect and request timeouts

By default, connect timeout is set to 5 seconds and request timeout is not set (so it is unlimited). If needed you can override default values as following:

```cpp
#include "ppconsul/consul.h"

using namespace ppconsul;

Consul consul("https://localhost:8080",
              kw::connect_timeout = std::chrono::milliseconds{15000}
              kw::request_timeout = std::chrono::milliseconds{5000});

// Use consul ...
```

If you're using blocking queries then make sure that request timeout is longer than block interval, otherwise request will fail with timeout error.

### Connect to Consul via HTTPS (TLS/SSL, whatever you call it):

```cpp
#include "ppconsul/consul.h"

using namespace ppconsul;

Consul consul("https://localhost:8080",
              kw::tls::cert="path/to/cert",
              kw::tls::key="path/to/private/key",
              kw::tls::ca_info="path/to/ca/cert");

// Use consul ...
```

## Multi-Threaded Usage of Ppconsul

Each <code>Consul</code> object has a pool of HTTP(S) clients to perform network requests. It is safe to call any endpoint (e.g. `Kv`, `Agent` etc) object or <code>Consul</code> object from multiple threads in the same time.

Call to `Consul::stop()` method stops all ongoing requests on that particular `Consul` object.

## Multi-Threaded Usage of Ppconsul and libCURL initialization

libCURL requires that global initialization function [`curl_global_init`](https://curl.se/libcurl/c/curl_global_init.html) is called before any oither libCURL function is called and before any additional thread is started.
Ppconsul calls `curl_global_init` for you at the moment when `makeDefaultHttpClientFactory()` is called for the first time which is usually done when first `Consul` object is created. If this is too late then you need to call to `curl_global_init` and `curl_global_cleanup` at the right moment yourself, similar to this example:

```cpp
int main(int argc, char *argv[])
{
    curl_global_init(CURL_GLOBAL_DEFAULT | CURL_GLOBAL_SSL);

    // Existing code that uses Ppconsul and starts extra threads...

    curl_global_cleanup();

    return 0;
}
```

`CURL_GLOBAL_SSL` flag is only needed if your're using HTTPS (TLS/SSL).

If your application needs to exclusively control libCURL initialization then you m ay want to skip libCURL initialization in Ppconsul's default HttpClientFactory. To do this, create default HttpClientFactory explicitly via `makeDefaultHttpClientFactory(false)` and pass it to `Consul`:

```cpp
Consul consul(makeDefaultHttpClientFactory(false), ...);
```

## Custom `http::HttpClient`

If needed, user can implement `http::HttpClient` interface and pass custom `HttpClientFactory` to `Consul`'s constructor.

## Documentation

TBD

## How To Build

You need C++11 compatible compiler (see above for the list of supported compilers) and [CMake](http://www.cmake.org/) 3.1 or above.

### TLDR

```bash
# Install dependencies
conan install .

# Make workspace directory
mkdir workspace
cd workspace

# Configure:
cmake ..

#Build
cmake --build . --config Release

# Install
cmake --build . --config Release --target install
```

### Get dependencies

If you use [Conan](https://conan.io/) then simply run `conan install .` to install dependencies.

Otherwise:

* Install [git](https://git-scm.com/) (any version should be fine)
* Install [Boost](http://www.boost.org/) 1.55 or later. You need compiled Boost.Regex library if you use GCC 4.8, otherwise you need headers only.
* Install [libCURL](http://curl.haxx.se/libcurl/) (any version should be fine).

### Build

Configure project:

```bash
mkdir workspace
cd workspace
cmake ..
```

You may want to set the following CMake variables on the command line:

To change where CMake looks for Boost, pass `-DBOOST_ROOT=<path_to_boost>` parameter to CMake or set `BOOST_ROOT` environment variable.

To change where CMake looks for libCURL, pass `-DCURL_ROOT=<path_to_curl>` parameter to CMake or set `CURL_ROOT` environment variable.

To change default install location, pass `-DCMAKE_INSTALL_PREFIX=<prefix>` parameter.

To build Ppconsul as static library, pass `-DBUILD_STATIC_LIB=ON` parameter.
Note that in this case you have to link with json11 static library as well (json11 library is build as part of Ppconsul build.)

**Note that on Windows Ppconsul can only be built as static library (that's default mode on Windows), see issue [Allow to build Ppconsul as dynamic library on Windows](https://github.com/oliora/ppconsul/issues/25)**.

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

## How To Run Tests
Install [Consul](http://consul.io) 0.4 or newer. *I recommend 0.7 or newer since it's easier to run it in development mode.*

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

## How to Use Ppconsul in Your Project

Build and install it first as described above.

When installed, the library can be simply used in any CMake-based project as following:

```
find_package(ppconsul)
add_executable(<your_app> ...)
target_link_libraries(<your_app> ppconsul)
```

As an alternative you can clone ppconsul into your project as submodule:

```
git submodule add https://github.com/oliora/ppconsul.git
```

And then include it into your CMake-based project as subdirectory:

```
set(BUILD_TESTS OFF)
set(BUILD_STATIC_LIB ON)
add_subdirectory(ppconsul)
...
target_link_libraries(<your_app> ppconsul)
```


## Found a bug? Got a feature request? Need help with Ppconsul?
Use [issue tracker](https://github.com/oliora/ppconsul/issues) or/and drop an email to [oliora](https://github.com/oliora).

## Contribute
First of all, welcome on board!

To contribute, please fork this repo, make your changes and create a pull request.

## License
The library released under [Boost Software License v1.0](http://www.boost.org/LICENSE_1_0.txt).
