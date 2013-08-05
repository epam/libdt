file(GLOB SRC_LIST "main.c")
if (UNIX)
  file(GLOB SRC_LIST ${SRC_LIST} "*_unix.c")
endif(UNIX)
if (WIN32)
  file(GLOB SRC_LIST ${SRC_LIST} "*_win32.c")
endif(WIN32)

add_library(${PROJECT_NAME} ${SRC_LIST})
