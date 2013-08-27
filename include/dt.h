#ifndef _DT_H
#define _DT_H

/*
 * Cross-platform date/time handling library for C.
 * Main header file.
 *
 * Authors: Ilya Storozhilov <Ilya_Storozhilov@epam.com>, Andrey Kuznetsov
 * <Andrey_Kuznetsov@epam.com>, Maxim Kot <Maxim_Kot@epam.com>
 * License: Public Domain, http://en.wikipedia.org/wiki/Public_domain
 */

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup GeneralPurpose General purpose functions
 * @{
 */

//! Represents a timestamp using a timezone none
/*!
 * \param timestamp Timestamp to represent
 * \param tz_name Timezone name or NULL if local tiezone is considered
 * \param tm Timestamp representation [OUT]
 * \return Result status of the operation. 0 on success and <0 in other cases
 */
int dt_time_t_to_tm(const time_t timestamp, const char *tz_name, struct tm *tm);

//! Returns a timestamps for a representation in timezone by it's name
/*!
 * It is possible for the representation to have two timestamps, e.g. when a time is "going back" for
 * an hour.
 * \param representation Representation to fetch a timestamp of
 * \param tz_name Timezone name or NULL if local tiezone is considered
 * \param first_timestamp First representation's timestamp [OUT]
 * \param second_timestamp Optional second representation's timestamp (can be NULL, not supported at the moment) [OUT]
 * \return Result status of the operation. 0 on success and <0 in other cases
 */
int dt_tm_to_time_t(const struct tm *representation, const char *tz_name,
                time_t *first_timestamp, time_t *second_timestamp);

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
 * \return Result status of the operation. 0 on success and <0 in other cases
 */
int dt_tm_to_string(const struct tm *representation, const char *tz_name, const char *fmt,
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
int dt_tm_from_string(const char *str, const char *fmt, struct tm *representation,
                char *tz_name_buffer, size_t tz_name_buffer_size);

/*!@}*/

#ifdef __cplusplus
}
#endif

#endif // _DT_H
