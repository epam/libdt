project(datetime)
cmake_minimum_required(VERSION 2.8)

file(GLOB SRC_LIST "main.c")
if (UNIX)
  file(GLOB SRC_LIST ${SRC_LIST} "*_unix.c")
endif(UNIX)
if (WIN32)
  file(GLOB SRC_LIST ${SRC_LIST} "*_win32.c")
endif(WIN32)

#aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})

