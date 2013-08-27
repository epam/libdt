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

//! Timezone object. TODO: Do we need it at all?
typedef struct dt_timezone {
        const char * name;              //!< Timezone name
#if defined(_WIN32)
        // TODO
#else
        // TODO
#endif
} dt_timezone_t;

#ifdef __cplusplus
extern "C" {
#endif

//! Converts a localized representation to POSIX breakdown time structure
/*!
 * This function does not set tm.tm_wday and tm.tm_yday fields.
 * \param representation Representation object
 * \param tm POSIX breakdown time structure for the representation [OUT]
 * \return Result status of the operation
 */
dt_status_t dt_representation_to_tm_private(const dt_representation_t *representation, struct tm *tm);

//! Initializes timezone with data
/*!
 * \note dt_cleanup_timezone() sould be called on timezone object at the end of it's use on successful operation
 * \param name Time name
 * \param timezone Pointer to timezone data structure to fill [OUT]
 * \return Result status of the operation
 */
//dt_status_t dt_init_timezone(const char *name, dt_timezone_t *timezone);

//! Cleans up timezone structure
/*!
 * \param timezone Pointer to timezone to clean up
 * \return Result status of the operation
 */
//dt_status_t dt_cleanup_timezone(dt_timezone_t *timezone);

//! Represents a timestamp using a timezone object
/*!
 * \param timestamp Timestamp to represent
 * \param timezone Timezone object or NULL if local tiezone is considered
 * \param result Timestamp representation [OUT]
 * \return Result status of the operation
 */
//dt_status_t dt_timestamp_to_representation_tz(const dt_timestamp_t *timestamp, const dt_timezone_t *timezone, dt_representation_t *result);

//! Returns a timestamps for a representation in timezone by it's object
/*!
 * It is possible for the representation to have two timestamps, e.g. when a time is "going back" for
 * an hour.
 * \param representation Representation to fetch a timestamp of
 * \param timezone Timezone object or NULL if local tiezone is considered
 * \param first_timestamp First representation's timestamp [OUT]
 * \param second_timestamp Optional second representation's timestamp (can be NULL) [OUT]
 * \return Result status of the operation
 */
//dt_status_t dt_representation_to_timestamp_tz(const dt_representation_t *representation, const dt_timezone_t *timezone,
//                dt_timestamp_t *first_timestamp, dt_timestamp_t *second_timestamp);

#ifdef __cplusplus
}
#endif

#endif // _DT_PRIVATE_H
