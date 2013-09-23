#ifndef _DT_POSIX_H
#define _DT_POSIX_H

/*
 * Cross-platform date/time handling library for C.
 * Main header file.
 * Extend STD ANSI C Library
 *
 * Authors: Ilya Storozhilov <Ilya_Storozhilov@epam.com>, Andrey Kuznetsov
 * <Andrey_Kuznetsov@epam.com>, Maxim Kot <Maxim_Kot@epam.com>
 * License: Public Domain, http://en.wikipedia.org/wiki/Public_domain
 */

#include <time.h>
#define DT_POSIX_WRONG_TIME -1//! < When you use standard library you this time stamp is reserved for reporting errors purposes


#ifdef __cplusplus
extern "C" {
#endif

    /*!
     * \defgroup STDExtendFunctons Functions, which is extending STD ANSI C Library
     * @{
     */

    //! Converts time_t to local time in specific Time Zone. See man localtime.
    /*!
     * @param time - time to format
     * @param tz_name - name of time zone. Must be in format <Area>/<Place, such as Europe/Moscow or Asia/Oral.
     * @param result - variable for result. Value will be set to local time representation
     * @return pointer to result tm instance, or NULL in error case.
     */
    struct tm *localtime_tz(const time_t *time, const char *tz_name, struct tm *result);

    //! Converts local time in specific Time Zone to time_t. See man mktime.
    /*!
     * @param tm - tm instance which will be used to make timestamp
     * @param tz_name - name of time zone. Must be in format <Area>/<Place, such as Europe/Moscow or Asia/Oral.
     * @return converted time returned or DT_POSIX_WRONG_TIME in error case.
     */
    time_t mktime_tz(const struct tm *tm, const char *tz_name);


#if defined(__CYGWIN__) || defined(WIN32)
#ifndef strptime
#define strptime libdt_strptime
    /*!
     * \brief libdt_strptime Converts string to representation. See man strptime.
     * Because strptime there is not implemented in windows standard library, we provide there
     * our implementation
     * @attention Please pay attention - this function is provided only for win32 platform, in other platforms it must be a part of libc
     * \param buf string to convert
     * \param fmt format which define how to convert buf to tm
     * \param tm output tm instance
     * \return NULL on failure, otherwise pointer to last not interpretated symbol from buf
     */
    char *strptime(const char *buf, const char *fmt, struct tm *tm);
#endif
#endif

    /*! @}*/

#ifdef __cplusplus
}
#endif

#endif // _DT_POSIX_H
