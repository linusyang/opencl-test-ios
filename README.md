OpenCL Simple Test
===============

This site contains two simple OpenCL programs: *Transmisson* and *Endianness*.
Both programs could be built on multiple platforms, such as
iOS (clang), Mac OS X (clang) and Windows (MSVC) etc.

There is also a Makefile for iOS to build. And the Darwin ARMv7 binaries are already built here named *Transmisson* and *Endianness*.

##Transmisson
This demo simply transmits data between internal RAM and GPU RAM and calculates the time it costs. It is a four-step progress:
1. Internal RAM to Internal RAM
2. Internal RAM to GPU RAM
3. GPU RAM to GPU RAM
4. GPU RAM to Internal RAM

The data size of the demo is 8 mega bytes.

##Endianness
This demo converts the endianness of unsigned 64-bit integers.

The number of integers are 32768.

##iOS Makefile Usage
This Makefile is not elegantly written. You have to specify your SDK path before using it. The default SDK is Xcode 4.4 with iOS 5.1 SDK.

#Declaration
This site is distributed under BSD license.

Linus Yang
