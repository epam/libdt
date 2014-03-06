project(dt)
enable_testing()
cmake_minimum_required(VERSION 2.8)
set(BUILD_SHARED_LIBS true)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include_directories(${CMAKE_SOURCE_DIR}/include/)
find_package(Doxygen)
include(AstyleFormatSource)
if(DOXYGEN_FOUND)
    message(STATUS "Doxygen was found you can build docs with 'doc' target - for user documentation and 'doc-develop' for development documentation and review them in ${CMAKE_SOURCE_DIR}/doc[-develop] directory")
    add_custom_target(doc COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    add_custom_target(doc-devel COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile.devel WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

else()
    message(WARNING "Doxygen not found documentation will be unavailable")
endif()


subdirs(src test include resources scripts)
