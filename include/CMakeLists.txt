file(GLOB INCLUDE_FILES ${INCLUDE_FILES} "libdt/*.h")
file(GLOB INCLUDE_FILES ${INCLUDE_FILES} "dt-private/*.h")
add_custom_target(includes SOURCES ${INCLUDE_FILES})
