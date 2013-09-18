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
    DT_UNKNOWN_ERROR                        //!< Unknown error
} dt_status_t;

//! Boolean type
typedef enum {
    DT_FALSE = 0,
    DT_TRUE
} dt_bool_t;

//! Compare result
typedef enum {
    LESS = -1,
    EQUAL = 0,
    MORE = 1
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
    unsigned long nano_second;              //!< Nano-second part (0L-999999999L)
} dt_timestamp_t;

//! Interval object
typedef struct dt_interval {
    long seconds;                           //!< Seconds part (>=0L)
    unsigned long nano_seconds;             //!< Nano-seconds part (0L-999999999L)
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
    unsigned short second;                  //!< Second (0-59 or 0-60 if leap second)
    unsigned long nano_second;              //!< Nano-second (0L-999999999L)
} dt_representation_t;

//! Timezone representation
//! @note it's internal implementation can be changed from version to version
typedef struct dt_timezone {
#ifdef WIN32
    const char *time_zone_name;
#else
    const struct state *state;
#endif
} dt_timezone_t;

#endif // _DT_TYPES_H
