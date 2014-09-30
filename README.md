# PPCONSUL

*Version 0.1*

A C++ client library for [Consul](http://consul.io). Consul is a distributed tool for discovering and configuring services in your infrastructure.

The main goal is to fully cover version 1 of [Consul HTTP API](http://www.consul.io/docs/agent/http.html).

**Note that this project is just started so it is under active developing, don't have a stable interface and was not tested well.
So if you are looking for something stable and ready to be used in production then it's better to come back later or help me growing this project to the
quality level you need.**

The library uses [C++ Network Library](http://cpp-netlib.org/) to deal with HTTP and [modified version](https://github.com/oliora/json11) of [json11](https://github.com/dropbox/json11) to deal with JSON.
It uses [Catch unit test framework](https://github.com/philsquared/Catch) for testing.

There is a vague plan to switch from C++ Network Library to [libCURL](http://curl.haxx.se/libcurl/) because the former depends on [Boost](http://www.boost.org/) and this may prevents
some persons from using the library.

## Documentation
TBD

## Requirements
1. **C++11 compatible** compiler. Was tested with Visual Studio 2013 and Xcode 6.0 (Clang 3.5)
2. [CMake](http://www.cmake.org/) 2.8 or above on Linux/OSX, 2.8.12 or above on Windows. **Note that CMake 3 not guaranteed to work.**
3. [C++ Network Library](http://cpp-netlib.org/) 0.11 or above. Note that this library depends on [Boost](http://www.boost.org/).
4. [Consul](http://consul.io) 0.4.0 or above. It's optional and needed only to run some of the tests.

## How To Build 
* Install all the requirements specified
* Build C++ Network Library as described in documentation.
* Checkout this repository into *project_dir*
* Go into *project_dir*
* `mkdir workspace`
* `cd workspace`
* `cmake ..`
   or generate project for your preferable IDE with `cmake -G <generator> ..`
* `make` (Linux/OSX)
  `cmake --build . --config release` (Windows)
   or open generated project files in IDE (check `workspace` directory for them)

## How To Run Tests
TBD

## License
Copyright (c) 2014 Andrey Upadyshev <oliora@gmail.com>
The library published under Boost Software License, Version 1.0. (See accompanying [license file](LICENSE_1_0.txt) or copy at http://www.boost.org/LICENSE_1_0.txt

## Bug Report
Use [issue tracker](https://github.com/oliora/ppconsul/issues).

## Contribute
Fork it, commit changes and create a pull request. Thank you!
