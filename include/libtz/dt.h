#ifndef _DT_H
#define _DT_H

/*
 * Cross-platform date/time handling library for C.
 * Main header file.
 * Expand STD ANSI C Library
 *
 * Authors: Ilya Storozhilov <Ilya_Storozhilov@epam.com>, Andrey Kuznetsov
 * <Andrey_Kuznetsov@epam.com>, Maxim Kot <Maxim_Kot@epam.com>
 * License: Public Domain, http://en.wikipedia.org/wiki/Public_domain
 */

#include <time.h>


#ifdef __cplusplus
extern "C" {
#endif

/** Converts time_t to local time in specific Time Zone.
 * @param time - time to format
 * @param tzName - name of time zone. Must be in format <Area>/<Place, such as Europe/Moscow or Asia/Oral.
 * @param result - variable for result. Value will be set to local time representation
 * @return on success, EXIT_SUCCESS is returned or EXIT_FAILURE in error case.
 */
int localtime_tz(const time_t *time, const char *tzName, struct tm *result);

/** Converts local time in specific Time Zone to time_t.
 * @param time - time to format
 * @param tzName - name of time zone. Must be in format <Area>/<Place, such as Europe/Moscow or Asia/Oral.
 * @param result - variable for result. Value will be set to local time representation
 * @return on success, EXIT_SUCCESS is returned or EXIT_FAILURE in error case.
 */
int mktime_tz(const struct tm *tm, const char *tzName, time_t *result);

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
int strftime_tz(const struct tm *representation, const char *tz_name, const char *fmt,
                char *str_buffer, size_t str_buffer_size);

//! Converts string to representation
/*!
 * \param str A NULL-terminated string to parse
 * \param fmt Format string, see strptime()/strftime() plus "%f" for nano-seconds
 * \param representation Representation object to fill [OUT]
 * \return Result status of the operation
 */
int strptime_tz(const char *str, const char *fmt, struct tm *representation);

/*!@}*/

#ifdef __cplusplus
}
#endif

#endif // _DT_H
