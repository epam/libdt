#ifndef _DT_PRIVATE_H
#define _DT_PRIVATE_H

/*
 * Cross-platform date/time handling library for C.
 * Private header file.
 *
 * Authors: Ilya Storozhilov <Ilya_Storozhilov@epam.com>, Andrey Kuznetsov
 * <Andrey_Kuznetsov@epam.com>, Maxim Kot <Maxim_Kot@epam.com>
 * License: Public Domain, http://en.wikipedia.org/wiki/Public_domain
 */

#include <libtz/dt_precise.h>

#ifdef __cplusplus
extern "C" {
#endif

//! Converts a POSIX breakdown time structure to localized representation
/*!
 * This function does not set tm.tm_wday and tm.tm_yday fields. It not check validate of result
 * \param tm POSIX breakdown time structure for the representation
 * \param nano_second nano seconds
 * \param representation Representation object [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_tm_to_representation_withoutcheck(const struct tm *tm, long nano_second, dt_representation_t *representation);

//! Converts a time zone name in form, valid for current system ("Etc/GMT" will be convert in "Etc/UTC" for UNIX and in "UTC" for Windows 7, for example)
/*!
 * This function does not set tm.tm_wday and tm.tm_yday fields. It not check validate of result
 * \param tz_name some representation of time zone
 * \return link to static variable or NULL, if time zone didn't found
 */
const char *findTimeZoneByName(const char *tz_name);

#ifdef __cplusplus
}
#endif

#endif // _DT_PRIVATE_H
