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

The library is written in C++11 and requires a quite modern compiler. Currently it has been tested with:
* Windows: Visual Studio 2013
* OS X: Clang 6 + libc++ (Xcode 6.1), Clang 5.1 + libc++ (Xcode 5.1)
* Linux: GCC 4.9, GCC 4.8.2

[PPConsul build status](https://78.46.71.115/project.html?projectId=Ppconsul).

The newer versions of specified compilers should work fine but was not tested. Earlier versions of GCC and Clang may work. Visual Studio before 2013 definitely gives up.

The library depends on:
* [libCURL](http://curl.haxx.se/libcurl/) **or** [C++ Network Library](http://cpp-netlib.org/) (aka cpp-netlib) to deal with HTTP.
* [Boost](http://www.boost.org/) library. PPConsul needs only headers with one exception: using of GCC 4.8 requires Boost.Regex library because [regex is supported only in GCC 4.9](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631).

The library includes code of the following 3rd party libraries (look at `ext` directory): 
* [my own version](https://github.com/oliora/json11) of [json11](https://github.com/dropbox/json11) library to deal with JSON.
* [libb64](http://libb64.sourceforge.net/) library for base64 decoding.

For unit tests, the library uses [Catch](https://github.com/philsquared/Catch) framework. Many thanks to Phil Nash for this great product.

## Documentation
TBD

## How To Build

### Get Dependencies
* Get C++11 compatible compiler. See above for the list of supported compilers.
* Install [Git](http://git-scm.com/) client.
* Install [CMake](http://www.cmake.org/) 2.8.12 or above (earlier version may also be OK on Linux/OSX but not on Windows). Note that CMake 3 not guaranteed to work.
* Download [Boost](http://www.boost.org/) library. Build it if you going to use cpp-netlib instead of libCURL or GCC 4.8.
* If you prefer to use libCURL then install libCURL (any version should be OK). Note that on Windows it is included in [CURL installer](http://curl.haxx.se/download.html).
Otherwise download and build [cpp-netlib](http://cpp-netlib.org/) 0.11 or above. Note that the latter depends on compiled Boost libraries.
* Install [Consul](http://consul.io) 0.4.0 or above. It's optional and needed to run some of the tests only.

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
open solution file `workspace\ppconsul.sln` in MSVS or build the project from the command line with  
`cmake --build . --config Release`.

## How To Run Tests
TBD

## Bug Report
Use [issue tracker](https://github.com/oliora/ppconsul/issues).

## Contribute
First of all, welcome on board! Please fork this repo, commit changes and create a pull request.

## License
The library released under [Boost Software License](http://www.boost.org/LICENSE_1_0.txt).
