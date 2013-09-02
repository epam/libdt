file(GLOB SRC_LIST "dt.c")
if (UNIX)
  file(GLOB SRC_LIST ${SRC_LIST} "unix/*.c")
  include_directories(. unix)
endif(UNIX)
if (WIN32)
  include_directories(. win32)
  file(GLOB SRC_LIST ${SRC_LIST} "*win32/*.c")
file(GLOB SRC_LIST ${SRC_LIST} "*win32/gnutimeutils/*.c")
endif(WIN32)

add_library(${PROJECT_NAME} ${SRC_LIST} )
add_dependencies(${PROJECT_NAME} timezones-map)

if (UNIX)
  target_link_libraries(${PROJECT_NAME} rt m)
endif(UNIX)

include(../cmake/TZPack.cmake)
