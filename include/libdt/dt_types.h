#ifndef _DT_TYPES_H
#define _DT_TYPES_H

/*
 * Cross-platform date/time handling library for C.
 * Datatypes header file.
 *
 * Authors: Ilya Storozhilov <Ilya_Storozhilov@epam.com>, Andrey Kuznetsov
 * <Andrey_Kuznetsov@epam.com>, Maxim Kot <Maxim_Kot@epam.com>
 * License: Public Domain, http://en.wikipedia.org/wiki/Public_domain
 */

//! Result status values
typedef enum {
    DT_OK,                                  //!< No error
    DT_INVALID_ARGUMENT,                    //!< Invalid argument
    DT_TIMEZONE_NOT_FOUND,                  //!< Timezone not found
    DT_SYSTEM_CALL_ERROR,                   //!< System call error
    DT_MALLOC_ERROR,                        //!< Memory allocate error
    DT_TOO_SMALL_BUFFER,                    //!< Too small buffer has been provided to function
    DT_CONVERT_ERROR,                       //!< Convertations errors
    DT_NO_MORE_ITEMS,                       //!< No more items for enumeration available
    DT_OVERFLOW,                            //!< Operation caused overflow
    DT_UNKNOWN_ERROR                        //!< Unknown error
} dt_status_t;

//! Boolean type
typedef enum {
    DT_FALSE = 0,
    DT_TRUE
} dt_bool_t;

//! Compare result
typedef enum {
    DT_LESSER = -1,
    DT_EQUALS = 0,
    DT_GREATER = 1,
    DT_COMPARSION_FAIL
} dt_compare_result_t;

#define DT_SECONDS_PER_DAY 86400
#define DT_SECONDS_PER_HOUR 3600
#define DT_SECONDS_PER_MINUTE 60

//! Timestamp object, which is opaque to user
/*!
 * This timestamp provides a nano-second precision and has no lower limit as time_t has.
 * Do not access it's fields directly and/or in any way rely on it's structure as it is
 * a subject to change w/o any announcement.
 */
typedef struct dt_timestamp {
    long second;                            //!< Seconds from some particular moment on Time Continuum (platform-specific)
    unsigned long nano_second;              //!< Nano-second part (0UL-999999999UL)
} dt_timestamp_t;

//! Interval object
typedef struct dt_interval {
    unsigned long seconds;                  //!< Seconds part (>=0UL)
    unsigned long nano_seconds;             //!< Nano-seconds part (0UL-999999999UL)
} dt_interval_t;

//! Offset object
typedef struct dt_offset {
    dt_interval_t duration;                 //!< Duration of the offset
    dt_bool_t is_forward;                   //!< Offset direction
} dt_offset_t;

//! Date/time representation object
typedef struct dt_representation {
    int year;                               //!< Year (any, except 0)
    unsigned short month;                   //!< Month (1-12)
    unsigned short day;                     //!< Day (1-28/29/30/31)
    unsigned short hour;                    //!< Hour (0-23)
    unsigned short minute;                  //!< Minute (0-59)
    unsigned short second;                  //!< Second (0-59 or (0-60 if leap second - is not supporting now))
    unsigned long nano_second;              //!< Nano-second (0UL-999999999UL)
} dt_representation_t;

//! Timezone representation
//! @attention it's internal implementation can be changed from version to version
typedef struct dt_timezone {
    //! @cond Doxygen_Suppress
#if defined(__CYGWIN__) || defined(WIN32)
    struct _TIME_DYNAMIC_ZONE_INFORMATION *dtzi;
#else
    const struct state *state;
#endif
    //! @endcond
} dt_timezone_t;

#endif // _DT_TYPES_H
