#from https://github.com/bilke/cmake-modules/blob/master/CodeCoverage.cmake
# - Enable Code Coverage
#
# 2012-01-31, Lars Bilke
#
# USAGE:
# 1. Copy this file into your cmake modules path
# 2. Add the following line to your CMakeLists.txt:
# INCLUDE(CodeCoverage)
#
# 3. Use the function SETUP_TARGET_FOR_COVERAGE to create a custom make target
# which runs your test executable and produces a lcov code coverage report.
#
# 4. Check is coverage available use CODE_COVERAGE_FOUND
#

# Check prereqs
find_package(Perl)

set(CODE_COVERAGE_FOUND "True")

FIND_PROGRAM( GCOV_PATH gcov )
if(WIN32)
    if(NOT PERL_FOUND)
        message(STATUS "Perl not found! Coverage report will be unavailable")
        unset(CODE_COVERAGE_FOUND)
    endif(NOT PERL_FOUND)

    if(LCOV_ROOT)
        find_file(LCOV_PATH lcov PATHS "${LCOV_ROOT}\\bin\\")
        set(LCOV_PATH_NATIVE ${LCOV_PATH})
        #string(REGEX REPLACE "[/]" "\\\\" LCOV_PATH_NATIVE ${LCOV_PATH})
        find_file(GENHTML_PATH genhtml PATHS "${LCOV_ROOT}\\bin\\")
        set(GENHTML_PATH_NATIVE ${GENHTML_PATH})
       # string(REGEX REPLACE "[/]" "\\\\" GENHTML_PATH_NATIVE ${GENHTML_PATH})
    else(LCOV_ROOT)
        message(STATUS "LCOV_ROOT is not set! Coverage report will be unavailable")
    endif(LCOV_ROOT)
else(WIN32)
    FIND_PROGRAM( LCOV_PATH lcov )
    FIND_PROGRAM( GENHTML_PATH genhtml )
endif(WIN32)

FIND_PROGRAM( GCOVR_PATH gcovr PATHS ${CMAKE_SOURCE_DIR}/tests)

IF(NOT GCOV_PATH)
    MESSAGE(STATUS "gcov not found! Coverage report will be unavailable")
    unset(CODE_COVERAGE_FOUND)
ENDIF() # NOT GCOV_PATH

IF(NOT CMAKE_COMPILER_IS_GNUCXX)
    MESSAGE(STATUS "Compiler is not GNU gcc! Coverage report will be unavailable")
    unset(CODE_COVERAGE_FOUND)
ENDIF() # NOT CMAKE_COMPILER_IS_GNUCXX

IF( NOT CMAKE_BUILD_TYPE STREQUAL "Debug" )
    MESSAGE(STATUS "Code coverage results with an optimised (non-Debug) build may be misleading" )
ENDIF() # NOT CMAKE_BUILD_TYPE STREQUAL "Debug"

IF(CODE_COVERAGE_FOUND)
    # Setup compiler options
    ADD_DEFINITIONS(-fprofile-arcs -ftest-coverage)
    LINK_LIBRARIES(gcov)
ENDIF()


# Param _targetname The name of new the custom make target
# Param _testrunner The name of the target which runs the tests
# Param _outputname lcov output is generated as _outputname.info
# HTML report is generated in _outputname/index.html
# Optional fourth parameter is passed as arguments to _testrunner
# Pass them in list form, e.g.: "-j;2" for -j 2
FUNCTION(SETUP_TARGET_FOR_COVERAGE _targetname _testrunner _outputname)

    IF(NOT LCOV_PATH)
        MESSAGE(WARNING "lcov not found! Coverage report will be unavailable")
    ENDIF() # NOT LCOV_PATH

    IF(NOT GENHTML_PATH)
        MESSAGE(WARNING "genhtml not found! Coverage report will be unavailable")
    ENDIF() # NOT GENHTML_PATH

    IF(WIN32)
        set(LCOV_COMMAND ${PERL_EXECUTABLE})
        set(GENHTML_COMMAND ${PERL_EXECUTABLE})
        set(LCOV_COMMAND_ARGS ${LCOV_PATH_NATIVE})
        set(GENHTML_COMMAND_ARGS ${GENHTML_PATH_NATIVE})
    ELSE(WIN32)
        set(LCOV_COMMAND ${LCOV_PATH})
        set(GENHTML_COMMAND ${GENHTML_PATH})
    ENDIF(WIN32)
    message(STATUS "LCOV_COMMAND is ${LCOV_COMMAND}")
    # Setup target
    ADD_CUSTOM_TARGET(${_targetname}

    # Cleanup lcov
    ${LCOV_COMMAND} ${LCOV_COMMAND_ARGS} --directory . --zerocounters

    # Run tests
    COMMAND ${_testrunner} ${ARGV3}

    # Capturing lcov counters and generating report
    COMMAND ${LCOV_COMMAND} ${LCOV_COMMAND_ARGS} --directory . --capture --output-file ${_outputname}.info
    COMMAND ${LCOV_COMMAND} ${LCOV_COMMAND_ARGS} --remove ${_outputname}.info 'tests\\*'  --output-file ${_outputname}.info.cleaned
    COMMAND ${GENHTML_COMMAND} ${GENHTML_COMMAND_ARGS} -o ${_outputname} ${_outputname}.info.cleaned
    COMMAND ${CMAKE_COMMAND} -E remove ${_outputname}.info ${_outputname}.info.cleaned

    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Resetting code coverage counters to zero.\nProcessing code coverage counters and generating report."
    )

    # Show info where to find the report
    ADD_CUSTOM_COMMAND(TARGET ${_targetname} POST_BUILD
        COMMAND ;
        COMMENT "Open ./${_outputname}/index.html in your browser to view the coverage report."
    )

ENDFUNCTION() # SETUP_TARGET_FOR_COVERAGE

# Param _targetname The name of new the custom make target
# Param _testrunner The name of the target which runs the tests
# Param _outputname cobertura output is generated as _outputname.xml
# Optional fourth parameter is passed as arguments to _testrunner
# Pass them in list form, e.g.: "-j;2" for -j 2
FUNCTION(SETUP_TARGET_FOR_COVERAGE_COBERTURA _targetname _testrunner _outputname)

IF(NOT PYTHON_EXECUTABLE)
MESSAGE(WARNING "Python not found! Coverage report will be unavailable")
ENDIF() # NOT PYTHON_EXECUTABLE

IF(NOT GCOVR_PATH)
MESSAGE(WARNING "gcovr not found! Coverage report will be unavailable")
ENDIF() # NOT GCOVR_PATH

    ADD_CUSTOM_TARGET(${_targetname}

    # Run tests
    ${_testrunner} ${ARGV3}

# Running gcovr
COMMAND ${PYTHON_EXECUTABLE} ${GCOVR_PATH} -r ${CMAKE_SOURCE_DIR} -e '${CMAKE_SOURCE_DIR}/tests/'
COMMAND ${PYTHON_EXECUTABLE} ${GCOVR_PATH} --html  -r ${CMAKE_SOURCE_DIR} -e '${CMAKE_SOURCE_DIR}/tests/' -o ${_outputname}.html
COMMAND ${PYTHON_EXECUTABLE} ${GCOVR_PATH} --xml   -r ${CMAKE_SOURCE_DIR} -e '${CMAKE_SOURCE_DIR}/tests/' -o ${_outputname}.xml
WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
COMMENT "Running gcovr to produce Cobertura code coverage report."
)

    # Show info where to find the report
    ADD_CUSTOM_COMMAND(TARGET ${_targetname} POST_BUILD
        COMMAND ;
        COMMENT "Cobertura code coverage report saved in '${_outputname}.html' and '${_outputname}.xml'"
    )

ENDFUNCTION() # SETUP_TARGET_FOR_COVERAGE_COBERTURA

