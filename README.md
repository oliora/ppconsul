# PPConsul

*Version 0.1*

A C++ client library for [Consul](http://consul.io). Consul is a distributed tool for discovering and configuring services in your infrastructure.

The goal of PPConsul is to:
* Fully cover version 1 of Consul [HTTP API](http://www.consul.io/docs/agent/http.html). Please check the current [implementation status](status.md).
* Provide simple but modular and effective API based on modern C++ approaches. This includes to be C++11 aware.
* Support different platforms. At the moment, Linux, Windows and OSX platforms supported.
* Cover all the code with automated tests.
* Have documentation.

Note that this project is just started so it is under active developing, doesn't have a stable interface and was not tested enough.
So if you are looking for something stable and ready to be used in production then it's better to come back later or help me growing this project to the quality level you need.

Library tests are running against **Consul v0.6.3**. Library known to work with versions **0.4** and **0.5** as well although some tests may fail because of differences in Consul behavior.

The library is written in C++11 and requires a quite modern compiler. Currently it has been tested with:
* Windows: Visual Studio 2013 Update 3
* OS X: Clang 7 (Xcode 7), Clang 6 (Xcode 6.1), Clang 5 (Xcode 5.1) all with libc++.
* Linux: GCC 5.3, GCC 4.9, GCC 4.8.2

Please check [PPConsul build status](https://136.243.151.173:4433/project.html?projectId=Ppconsul&guest=1).

The newer versions of specified compilers should work fine but was not tested. Earlier versions of GCC and Clang may work. Visual Studio before 2013 definitely gives up.

The library depends on:
* [Boost](http://www.boost.org/) 1.55 or later [see note](#boost-version-note). PPConsul needs only headers with one exception: using of GCC 4.8 requires Boost.Regex library because [regex is supported only in GCC 4.9](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631).
* [libCURL](http://curl.haxx.se/libcurl/) **or** [C++ Network Library](http://cpp-netlib.org/) (aka cpp-netlib) to deal with HTTP. Note that the latter depends on compiled Boost libraries.

The library includes code of the following 3rd party libraries (look at `ext` directory): 
* Slightly tweaked [version](https://github.com/oliora/json11) of [json11](https://github.com/dropbox/json11) library to deal with JSON.
* [libb64](http://libb64.sourceforge.net/) library for base64 decoding.

For unit tests, the library uses [Catch](https://github.com/philsquared/Catch) framework. Many thanks to Phil Nash for this great product.

##### Boost Version Note:
Boost versions prior to 1.55 can not be used because of [Boost issue #8759](https://svn.boost.org/trac/boost/ticket/8759).

## Warm Up Example

Register, deregister and report the state of your service in Consul:


```cpp
#include "ppconsul/agent.h"

using namespace ppconsul;

// Create a consul client with using of a default address ("127.0.0.1:8500") and default DC
Consul consul;
// We need the 'agent' endpoint for a service registration
agent::Agent agent(consul);

// Register a service with associated HTTP check:
agent.registerService(
    agent::keywords::name = "my-service",
    agent::keywords::port = 9876,
    agent::keywords::tags = {"tcp", "super_server"},
    agent::keywords::check = agent::HttpCheck{"http://localhost:80/", std::chrono::seconds(2)}
);

...

// Unregister service
agent.deregisterService("my-service");
```

## Documentation
TBD

## How To Build

### Get Dependencies
* Get C++11 compatible compiler. See above for the list of supported compilers.
* Install [CMake](http://www.cmake.org/) 2.8.12 or above (earlier version may also be OK on Linux/OSX but not on Windows). Note that CMake 3 not guaranteed to work.
* Install [Boost](http://www.boost.org/) 1.55 or later. You need compiled Boost libraries if you going to use cpp-netlib or GCC < 4.9, otherwise you need Boost headers only.
* Install either [libCURL](http://curl.haxx.se/libcurl/) (any version is OK) or [cpp-netlib](http://cpp-netlib.org/) 0.11 or above.
* Install [Consul](http://consul.io) 0.4.0 or above. It's only needed to run tests.

### Build

Prepare project:  
```
mkdir workspace
cd workspace
cmake ..
```

* To use cpp-netlib instead of libCURL use `-DUSE_CPPNETLIB=1` when invoking CMake.
* To change where the build looks for Boost, pass `-DBOOST_ROOT=<path_to_boost>` parameter to CMake or set `BOOST_ROOT` environment variable.
* To change where the build looks for libCURL, pass `-DCURL_ROOT=<path_to_curl>` parameter to CMake or set `CURL_ROOT` environment variable.
* To change the default install location pass `-DCMAKE_INSTALL_PREFIX=<prefix>` parameter to CMake.

*Note about -G option of CMake to choose you favourite IDE to generate project files for.*

Build on Linux/OSX:  
`make`  

Build on Windows:  
Either open solution file `workspace\ppconsul.sln` or build from the command line:  
`cmake --build . --config Release`.

## How To Run Tests
TBD

## Bug Report
Use [issue tracker](https://github.com/oliora/ppconsul/issues).

## Contribute
First of all, welcome on board! Please fork this repo, commit changes and create a pull request.

## License
The library released under [Boost Software License v1.0](http://www.boost.org/LICENSE_1_0.txt).
