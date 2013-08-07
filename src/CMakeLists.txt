file(GLOB SRC_LIST "main.c")
if (UNIX)
  file(GLOB SRC_LIST ${SRC_LIST} "unix/*.c")
  include_directories(. unix)
endif(UNIX)
if (WIN32)
  include_directories(. win32)
  file(GLOB SRC_LIST ${SRC_LIST} "*win32/*.c")
endif(WIN32)


add_library(${PROJECT_NAME} ${SRC_LIST} )
add_dependencies(${PROJECT_NAME} timezones-map)
