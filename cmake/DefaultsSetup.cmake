#macro setDefault
#checks was VARNAME setted if, no tryes to obtain it's value from enviroment
#if it failed, just setups to it DEFAULT_VALUE
macro (setDefault VARNAME DEFAULT_VALUE)
    if(NOT ${VARNAME})
        message(STATUS "${VARNAME} not defined")
        if("$ENV{${VARNAME}}" STREQUAL "")
            message(STATUS "There was setup default value for ${VARNAME}=${DEFAULT_VALUE} ")
            set(${VARNAME} ${DEFAULT_VALUE})
        else()
            message(STATUS "There was setup value from envieroment for ${VARNAME}=$ENV{${VARNAME}}")
            set(${VARNAME} $ENV{${VARNAME}})
        endif()
    endif()
endmacro(setDefault)
