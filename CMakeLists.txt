project(libtz)
enable_testing()
cmake_minimum_required(VERSION 2.8)

include_directories(${CMAKE_SOURCE_DIR}/include)
subdirs(src test include)
