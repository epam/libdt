#This is inline version of libtz by Ilya Novoselov (https://github.com/nullie/libtz)
#if(UNIX)
#if(NOT CYGWIN)
#    add_library(tz-inline libtz.c)
#endif(NOT CYGWIN)
#endif(UNIX)
