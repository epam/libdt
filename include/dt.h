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
#ifndef EXIT_SUCCESS
    #define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
    #define EXIT_FAILURE -1
#endif

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
int dt_tm2string(const struct tm *representation, const char *tz_name, const char *fmt,
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
int dt_string2tm(const char *str, const char *fmt, struct tm *representation,
                char *tz_name_buffer, size_t tz_name_buffer_size);

/*!@}*/

#ifdef __cplusplus
}
#endif

#endif // _DT_H
