# - Find tz library
# Find the Time zone library includes and library
#
#  TZ_ROOT - where to find header, libraries, etc.
#  TZ_INCLUDE_DIR - where to find dt.h, etc.
#  TZ_LIBRARY   - library when using tzlib.
#  TZ_FOUND       - True if tzlib found.

if (TZ_ROOT)
    set(TZ_LIBRARIES_PATH ${TZ_ROOT}/lib)
    set(TZ_INCLUDES_PATH ${TZ_ROOT}/include)
endif(TZ_ROOT)

FIND_PATH(TZ_INCLUDE_DIR dt.h
    HINTS "${TZ_INCLUDES_PATH}"
    PATH_SUFFIXES libtz)

SET(TZ_NAMES tz)
FIND_LIBRARY(TZ_LIBRARY ${TZ_NAMES}
    HINTS "${TZ_LIBRARIES_PATH}" )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TZ DEFAULT_MSG TZ_LIBRARY TZ_INCLUDE_DIR)

IF(TZ_FOUND)
     SET( TZ_LIBRARIES ${ZLIB_LIBRARY} )
     IF("${TZ_INCLUDE_DIR}" MATCHES "/libtz$")
        # Strip off the trailing "/libtz" in the path.
        GET_FILENAME_COMPONENT(TZ_INCLUDE_DIR ${TZ_INCLUDE_DIR} PATH)
     ENDIF("${TZ_INCLUDE_DIR}" MATCHES "/libtz$")
ELSE(TZ_FOUND)
  SET( TZ_LIBRARIES )
ENDIF(TZ_FOUND)

MARK_AS_ADVANCED( TZ_LIBRARY TZ_INCLUDE_DIR )
