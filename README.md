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

The library is written in C++11 and requires a quite modern compiler. Compilers officially supported at the moment:
* Windows: Visual Studio 2013
* OSX: Clang 6, libc++ (Xcode 6)
* Linux: GCC 4.8.2

The newer versions of specified compilers should work fine but was not tested. Earlier versions of GCC and Clang may work. Visual Studio 2012 or earlier will give up.

The library uses
* [libCURL](http://curl.haxx.se/libcurl/) **or** [C++ Network Library](http://cpp-netlib.org/) (aka cpp-netlib) to deal with HTTP.
* [my own version](https://github.com/oliora/json11) of [json11](https://github.com/dropbox/json11) library to deal with JSON.
* [Catch](https://github.com/philsquared/Catch) unit test framework.
* [libb64](http://libb64.sourceforge.net/) library for base64 decoding.

## Documentation
TBD

## How To Build

### Dependencies
* Get C++11 compatible compiler. See above for the list of supported compilers.
* Install [Git](http://git-scm.com/) client.
* Install [CMake](http://www.cmake.org/) 2.8 or above on Linux/OSX, 2.8.12 or above on Windows. Note that CMake 3 not guaranteed to work.
* If you prefer to use libCURL then install libCURL (On Windows it's included in [CURL](http://curl.haxx.se/) installer).
Otherwise download and build [cpp-netlib](http://cpp-netlib.org/) 0.11 or above. Note that the latter depends on [Boost](http://www.boost.org/).
* Install [Consul](http://consul.io) 0.4.0 or above. It's optional and needed to run some of the tests only.

### Prepare Project

Execute the following commands:  
`git clone git@github.com:oliora/ppconsul.git`  
`cd ppconsul`  
`mkdir workspace`  
`cd workspace`  
If you want to use libCURL:  
`cmake ..`  
Otherwise:  
`cmake .. -DUSE_CPPNETLIB=1`

If you are building on Windows you *probably* need to set up additional variables:
* Path to libCURL headers and library.
  Set environment variable `set CURL_ROOT=<path_to_curl>` or pass it to CMake as `cmake .. -DCURL_ROOT=<path_to_curl>`.
* Path to Boost headers and library.
  Set environment variable `set BOOST_ROOT=<path_to_boost>` or pass it to CMake as for libCURL.

*Note about -G option of CMake to choose you favourite IDE to generate project files for.*

### Build

Linux/OSX:  
`make`  

Windows:  
open `workspace\ppconsul.sln` file in MSVS and build it there or build from command line with  
`cmake --build . --config release`.

## How To Run Tests
TBD

## Bug Report
Use [issue tracker](https://github.com/oliora/ppconsul/issues).

## Contribute
Fork this repo, commit changes and create a pull request. Welcome on board!

## License
The library released under [Boost Software License](http://www.boost.org/LICENSE_1_0.txt).
