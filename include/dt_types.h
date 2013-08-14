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
        DT_UNKNOWN_ERROR                        //!< Unknown error
} dt_status_t;

#define DT_FALSE 0
#define DT_TRUE 1

//! Boolean type
typedef char dt_bool_t;

#define DT_SECONDS_PER_DAY 86400
#define DT_SECONDS_PER_HOUR 3600
#define DT_SECONDS_PER_MINUTE 60

//! Timestamp object
typedef struct dt_timestamp {
#if defined(_WIN32)
        // TODO
#else
        struct timespec ts;                     //!< Timespec structure to store timestamp on Unix platform
#endif
} dt_timestamp_t;

//! Interval object
typedef struct dt_interval {
        unsigned long seconds;                  //!< Seconds part
        unsigned long nano_seconds;             //!< Nano-seconds part
} dt_interval_t;

//! Offset object
typedef struct dt_offset {
        dt_interval_t duration;                 //!< Duration of the offset
        dt_bool_t is_forward;                   //!< Offset direction
} dt_offset_t;

//! Date/time representation object
typedef struct dt_representation {
        int year;                               //!< Year (any, except 0)
        int month;                              //!< Month (1-12)
        int day;                                //!< Day (1-28/29/30/31)
        int hour;                               //!< Hour (0-23)
        int minute;                             //!< Minute (0-59)
        int second;                             //!< Second (0-59 or 0-60 if leap second)
        long nano_second;                       //!< Nano-second (0L-999999999L)
} dt_representation_t;

#endif // _DT_TYPES_H
