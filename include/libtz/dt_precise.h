#ifndef _DT_PRECISE_H
#define _DT_PRECISE_H

/*
 * Cross-platform date/time handling library for C.
 * Main header file.
 *
 * Authors: Ilya Storozhilov <Ilya_Storozhilov@epam.com>, Andrey Kuznetsov
 * <Andrey_Kuznetsov@epam.com>, Maxim Kot <Maxim_Kot@epam.com>
 * License: Public Domain, http://en.wikipedia.org/wiki/Public_domain
 */

#include <time.h>
#include <libtz/dt_types.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// General purpose functions
//------------------------------------------------------------------------------

/*!
 * \defgroup GeneralPurpose General purpose functions
 * @{
 */

//! Checks for leap year
/*!
 * \param year Year to check
 * \return Check result
 */
dt_bool_t dt_is_leap_year(int year);

//! Returns string represenatation of the datetime error code
const char * dt_strerror(dt_status_t status);

//! Validates a date/time representation values set
/*!
 * \param year Year
 * \param month Month
 * \param day Day
 * \param hour Hour
 * \param minute Minute
 * \param second Second
 * \param nano_second Nano-second
 * \return Validation result
 *
 * TODO: Leap seconds support!
 */
dt_bool_t dt_validate_representation(int year, int month, int day, int hour, int minute, int second,
                unsigned long nano_second);

/*!@}*/

//------------------------------------------------------------------------------
// Timestamp functions
//------------------------------------------------------------------------------

/*!
 * \defgroup Timestamp Timestamp functions
 * @{
 */

//! Returns a current timestamp
/*!
 * \param result Current timestamp [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_now(dt_timestamp_t *result);

//! Returns a current timestamp
/*!
 * \return Current timestamp [OUT]
 */
//dt_timestamp_t dt_now();

//! Compares two timestamps
/*!
 * \param lhs First timestamp to compare
 * \param rhs Second timestamp to compare
 * \param result Zero if timestamps are equal, less than zero if first timestamp is before second and more than zero otherwise [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_compare_timestamps(const dt_timestamp_t *lhs, const dt_timestamp_t *rhs, int *result);

//! Returns an offset between timestamps
/*!
 * \param lhs First timestamp
 * \param rhs Second timestamp
 * \param result Offset b/w timestamps [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_offset_to(const dt_timestamp_t *lhs, const dt_timestamp_t *rhs, dt_offset_t *result);

//! Applies offset on the timestamp
/*!
 * \param lhs Timestamp to apply the offset on
 * \param rhs Offset to apply on timestamp
 * \param result Result offset [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_apply_offset(const dt_timestamp_t *lhs, const dt_offset_t *rhs, dt_timestamp_t *result);

//! Loads timestamp from POSIX time
/*!
 * \note POSIX time (time_t) supports greater than zero values only
 * \param time POSIX time value
 * \param result Timestamp for the POSIX time value [OUT]
 * \param nano_second Nano-seconds part of the timestamp
 * \return Result status of the operation
 */
dt_status_t dt_posix_time_to_timestamp(time_t time, unsigned long nano_second, dt_timestamp_t *result);

//! Converts timestamp to POSIX time value
/*!
 * Function returns DT_INVALID_ARGUMENT if a timestamp could not be represented by POSIX time.
 * \note POSIX time (time_t) supports greater than zero values only
 * \param timestamp Timestamp to convert
 * \param time POSIX time for the timestamp [OUT]
 * \param nano_second Optional nano-seconds part of the timestamp (0L-999999999L), could be NULL if not used [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_timestamp_to_posix_time(const dt_timestamp_t *timestamp, time_t *time, unsigned long *nano_second);

/*!@}*/

//------------------------------------------------------------------------------
// Interval functions
//------------------------------------------------------------------------------

/*!
 * \defgroup Interval Interval functions
 * @{
 */

//! Initializes time interval
/*!
 * Seconds part is to be subtracted from nano-seconds and appended to seconds
 * \param seconds Seconds
 * \param nano_seconds Nano-seconds
 * \param result interval to initialize [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_init_interval(long seconds, unsigned long nano_seconds, dt_interval_t *result);

//! Compares two intervals
/*!
 * \param lhs First interval to compare
 * \param rhs Second interval to compare
 * \param result Zero if intervals are equal, less than zero if first intervals is less than second and more than zero otherwise [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_compare_intervals(const dt_interval_t *lhs, const dt_interval_t *rhs, int *result);

//! Summarizes two intervals
/*!
 * \param lhs First interval to summarize
 * \param rhs Second interval to summarize
 * \param result Result interval [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_sum_intervals(const dt_interval_t *lhs, const dt_interval_t *rhs, dt_interval_t *result);

//! Subtracts two intervals
/*!
 * \note Zero interval is returned if the greater is subtracting from the lesser one
 * \param lhs First interval to subtract
 * \param rhs Second interval to subtract
 * \param result Result interval [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_sub_intervals(const dt_interval_t *lhs, const dt_interval_t *rhs, dt_interval_t *result);

//! Multiplies an interval
/*!
 * \param lhs First interval to multiply
 * \param rhs Multiplier
 * \param result Result interval [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_mul_interval(const dt_interval_t *lhs, double rhs, dt_interval_t *result);

#if defined(_WIN32)
struct timespec {
        time_t tv_sec;                          //!< Seconds
        long tv_nsec;                           //!< Nano-seconds
};
#endif

//! Converts interval to timespec structure
/*!
 * \param interval Interval to convert to timespec structure
 * \param result Timespec structure [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_interval_to_timespec(const dt_interval_t *interval, struct timespec *result);

//! Converts timespec structure to interval
/*!
 * \param ts Timespec structure
 * \param result Interval [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_timespec_to_interval(const struct timespec *ts, dt_interval_t *result);

/*!@}*/

//------------------------------------------------------------------------------
// Representation functions
//------------------------------------------------------------------------------

/*!
 * \defgroup Representation Representation functions
 * @{
 */

//! Initializes representetion with error checking
/*!
 * \param year Year
 * \param month Month
 * \param day Day
 * \param hour Hour
 * \param minute Minute
 * \param second Second
 * \param nano_second Nano-second
 * \param result Representation to init [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_init_representation(int year, int month, int day, int hour, int minute, int second, unsigned long nano_second,
                dt_representation_t *result);

//! Represents a timestamp using a timezone none
/*!
 * \param timestamp Timestamp to represent
 * \param tz_name Timezone name or NULL if local tiezone is considered
 * \param representation Timestamp representation [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_timestamp_to_representation(const dt_timestamp_t *timestamp, const char *tz_name, dt_representation_t *representation);

//! Returns a timestamps for a representation in timezone by it's name
/*!
 * It is possible for the representation to have two timestamps, e.g. when a time is "going back" for
 * an hour.
 * \param representation Representation to fetch a timestamp of
 * \param tz_name Timezone name or NULL if local tiezone is considered
 * \param first_timestamp First representation's timestamp [OUT]
 * \param second_timestamp Optional second representation's timestamp (can be NULL, not supported at the moment) [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_representation_to_timestamp(const dt_representation_t *representation, const char *tz_name,
                dt_timestamp_t *first_timestamp, dt_timestamp_t *second_timestamp);

//! Returns representation's week day number
/*!
 * \param representation Representation object
 * \param dow Representation's day of week (1-7, 1 is Sunday) [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_representation_day_of_week(const dt_representation_t *representation, int *dow);

//! Returns representation's day of year
/*!
 * \param representation Representation object
 * \param doy Representation's day of year (1-365/366) [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_representation_day_of_year(const dt_representation_t *representation, int *doy);

//! Converts a localized representation to POSIX breakdown time structure
/*!
 * \param representation Representation object
 * \param tm POSIX breakdown time structure for the representation [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_representation_to_tm(const dt_representation_t *representation, struct tm *tm);

//! Converts a POSIX breakdown time structure to representation
/*!
 * \param tm POSIX breakdown time structure
 * \param nano_second Nano-second part of the representation
 * \param representation Representation of POSIX breakdown time structure [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_tm_to_representation(const struct tm *tm, long nano_second, dt_representation_t *representation);

/*!@}*/

//------------------------------------------------------------------------------
// String conversion functions
//------------------------------------------------------------------------------

/*!
 * \defgroup StringConversion String conversion functions
 * @{
 */

//! Converts representation to string
/*!
 * \param representation Representation to convert
 * \param tz_name Optional timezone name, could be NULL if local timezone is considered
 * \param fmt Format string, see strptime()/strftime() plus "%f" for nano-seconds
 * \param str_buffer Buffer to fill [OUT]
 * \param str_buffer_size A size of the buffer to fill
 * \return Result status of the operation
 */
dt_status_t dt_to_string(const dt_representation_t *representation, const char *tz_name, const char *fmt,
                char *str_buffer, size_t str_buffer_size);

//! Converts string to representation
/*!
 * \param str A NULL-terminated string to parse
 * \param fmt Format string, see strptime()/strftime() plus "%f" for nano-seconds
 * \param representation Representation object to fill [OUT]
 * \param tz_name_buffer Optional buffer to fill in with timezone name, could be NULL [OUT]
 * \param tz_name_buffer_size Timezone name buffer size, ignored if tz_name_buffer == NULL
 * \return Result status of the operation
 */
dt_status_t dt_from_string(const char *str, const char *fmt, dt_representation_t *representation,
                char *tz_name_buffer, size_t tz_name_buffer_size);

/*!@}*/

#ifdef __cplusplus
}
#endif

#endif // _DT_PRECISE_H
