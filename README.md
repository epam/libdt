# ANSI C cross-platform date/time library

## Introduction

**LibDt** is a cross-platform library which provides a comprehensive set of C-language functions to
operate with date and time.

## Features

* Timestamp to date/time representation conversion using arbitrary time-zone;
* Date/time representation to timestamp conversion using arbitrary time-zone;
* Cross-platform: Windows, Unix-like;
* Platform-independent time-zone names support: Olsen, Windows, Unix;
* Reentrancy;
* Operating system time-zone database support;
* Nanosecond precision (microsecond on Windows platforms);
* ANSI C API;

## Design

**LibDt** operates with following design elements:

* **Timestamp** - an opaque point on the time continuum, see `dt_timestamp_t`;
* **Interval** - a distance between two timestamps expressed in seconds, see `dt_interval_t`;
* **Offset** - a directed **Interval**, see `dt_offset_t`;
* **Representation** - a human-readable representation of the **Timestamp**: {year, month, day,
  hour, minute, second, nano-second}, see `dt_representation_t`;
* **Timezone** - a set of rules how to interprete **Timestamp** as **Representation**, see `dt_timezone_t`;

## Installation

### Runtime requirements

* libc (Unix-like)/CRT (Windows);

### Build requirements

* CMake 2.8;
* Python 2.7;
* GTest (optional for unit-testing);
* lcov (optional for unit testing coverage report);
* astyle (optional for auto code-style application);

### Building and installation

        $ mkdir build && cd build
        $ cmake ../
        $ make
        $ sudo make install

### Making a debug build and running tests

        $ mkdir build && cd build
	$ cmake enable_testing -DCMAKE_BUILD_TYPE=Debug ../
	$ make
	$ make test

### Making a win32 build on Linux

	$ sudo apt-get install mingw-w64 g++-mingw-w64
        $ mkdir build && cd build
	$ cmake -DCMAKE_TOOLCHAIN_FILE=../Toolchain-mingw32-x86_64.cmake ../	# For 64-bit build
	$ cmake -DCMAKE_TOOLCHAIN_FILE=../Toolchain-mingw32-i686.cmake ../	# For 32-bit build
	$ make

## Example of use

Suppose following code example to be in libdt_example.c file:

        #include <stdio.h>
        #include <libdt/dt.h>

        int main(int argc, char *argv[])
        {
                dt_timestamp_t ts;
                dt_status_t s = dt_now(&ts); // Current timestamp fetching
                if (s != DT_OK) {
                        perror(dt_strerror(s));
                        return 1;
                }
                dt_timezone_t tz;
                s = dt_timezone_lookup("Europe/Berlin", &tz); // Timezone object initialization
                if (s != DT_OK) {
                        perror(dt_strerror(s));
                        return 1;
                }
                dt_representation_t rp;
                s = dt_timestamp_to_representation(&ts, &tz, &rp); // Timestamp to representation conversion
                if (s != DT_OK) {
                        dt_timezone_cleanup(&tz);
                        perror(dt_strerror(s));
                        return 1;
                }
                char buf[255];
                s = dt_to_string(&rp, "%Y-%m-%d %H:%M:%S.%f", buf, sizeof(buf)); // Representation to string conversion
                if (s != DT_OK) {
                        dt_timezone_cleanup(&tz);
                        perror(dt_strerror(s));
                        return 1;
                }
                printf("%s\n", buf);
                dt_timezone_cleanup(&tz);
                return 0;
        }

So a compilation command and an output will be:

        $ gcc -o libdt_example libdt_example.c -ldt
        $ ./libdt_example
        2013-10-09 15:54:10.543765895

## TODO list

* Fractional seconds part support in string conversion functions;
* Interval initialization from double seconds value;
* Timezone names mapping API redesign. Timezone lookup without new/delete operators usage;

## License

*TODO: To be clarified.*
