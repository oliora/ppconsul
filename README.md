# PPConsul

*Version 0.1*

A C++ client library for [Consul](http://consul.io). Consul is a distributed tool for discovering and configuring services in your infrastructure.

The goal of PPConsul is to fully cover version 1 of Consul [HTTP API](http://www.consul.io/docs/agent/http.html).

**Note that this project is just started so it is under active developing, doesn't have a stable interface and was not tested enough.
So if you are looking for something stable and ready to be used in production then it's better to come back later or help me growing this project to the quality level you need.**

The library is written in C++11 so it is not compatible with old compilers.
The library uses [C++ Network Library](http://cpp-netlib.org/) (aka cpp-netlib) to deal with HTTP and
[my own version](https://github.com/oliora/json11) of [json11](https://github.com/dropbox/json11) library to deal with JSON.
It uses [Catch unit test framework](https://github.com/philsquared/Catch) for testing.

There is a vague plan to use [libCURL](http://curl.haxx.se/libcurl/) instead of cpp-netlib because the latter depends on (quite huge) Boost library and need to be build separately so this may prevent some persons from using PPConsul.

## API Status

The implementation status of HTTP API endpoints coverage:

In progress:
* **kv**

To do:
* **agent**
* **catalog**
* **health**
* **session**
* **acl** - 
* **event**
* **status**

## Documentation
TBD

## How To Build 

### Install Prerequirements:
* Get C++11 compatible compiler. Was tested with Visual Studio 2013 and Xcode 6.0 (Clang 3.5)
* Install [Git](http://git-scm.com/) client
* Install [CMake](http://www.cmake.org/) 2.8 or above on Linux/OSX, 2.8.12 or above on Windows. **Note that CMake 3 not guaranteed to work.**
* Download and build [cpp-netlib](http://cpp-netlib.org/) 0.11 or above. Note that this library depends on [Boost](http://www.boost.org/).
* Install [Consul](http://consul.io) 0.4.0 or above. It's optional and needed to run some of the tests only.

### Prepare Project
Execute the following commands:
`git clone git@github.com:oliora/ppconsul.git`  
`cd ppconsul`  
`mkdir workspace`  
`cd workspace`  
`cmake ..`  

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
