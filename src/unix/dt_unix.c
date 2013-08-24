#include <libtz/dt.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
//#include "tzcode/tzfile.h"
#include "../dt_private.h"

#include <stdio.h>
#include <assert.h>

/*
 * Cross-platform date/time handling library for C.
 * Unix platform source file.
 *
 * Authors: Ilya Storozhilov <Ilya_Storozhilov@epam.com>, Andrey Kuznetsov
 * <Andrey_Kuznetsov@epam.com>, Maxim Kot <Maxim_Kot@epam.com>
 * License: Public Domain, http://en.wikipedia.org/wiki/Public_domain
 */

//! Returns an offset from UTC of to represent timestamp in timezone
/*!
 * \param timestamp Timestamp to represent
 * \param timezone Timezone
 * \param offset Offset from UTC [OUT]
 * \return Result status of the operation
 */
//static dt_status_t dt_timestamp_utc_offset(const dt_timestamp_t *timestamp, const dt_timezone_t *timezone, dt_offset_t *offset);

//! Returns two offsets from UTC of the local timestamp representation
/*!
 * It is possible for the representation to have two UTC-offsets, e.g. when a time is going back for
 * an hour.
 * \param representation Representation to get offset of
 * \param timezone Timezone
 * \param first_offset First offset from UTC [OUT]
 * \param secons_offset First offset from UTC [OUT]
 * \return Result status of the operation
 */
//static dt_status_t dt_representation_utc_offset(const dt_representation_t *representation, const dt_timezone_t *timezone,
//                dt_offset_t *first_offset, dt_offset_t *second_offset);

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

dt_status_t dt_now(dt_timestamp_t *result)
{
        if (!result) {
                return DT_INVALID_ARGUMENT;
        }
        struct timespec ts;
        if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) {
                return DT_SYSTEM_CALL_ERROR;
        }
        result->second = ts.tv_sec;
        result->nano_second = ts.tv_nsec;
        return DT_OK;
}

dt_status_t dt_posix_time_to_timestamp(time_t time, unsigned long nano_second, dt_timestamp_t *result)
{
        if (time < 0 || !result) {
                return DT_INVALID_ARGUMENT;
        }
        result->second = time;
        result->nano_second = nano_second;
        return DT_OK;
}

dt_status_t dt_timestamp_to_posix_time(const dt_timestamp_t *timestamp, time_t *time, unsigned long *nano_second)
{
        if (!timestamp || !time || timestamp->second < 0) {
                return DT_INVALID_ARGUMENT;
        }
        *time = timestamp->second;
        *nano_second = timestamp->nano_second;
        return DT_OK;
}

dt_status_t dt_timestamp_to_representation(const dt_timestamp_t *timestamp, const char *tz_name, dt_representation_t *representation)
{
        // TODO: System timezone database usage!
 
        if (!timestamp || !representation) {
                return DT_INVALID_ARGUMENT;
        }

        dt_status_t result = DT_UNKNOWN_ERROR;

        extern char **environ;

        char **fakeenv = NULL;
        char **oldenv = NULL;

        if (tz_name) {
                // Preparing fake environment if TZ has been defined
                int i = 0;
                for (i = 0; environ[i] != NULL; ++i) {
                        continue;
                }
                fakeenv = malloc((i + 2) * sizeof *fakeenv);
                size_t longest = 1024;
                if (fakeenv == NULL || (fakeenv[0] = malloc(longest + 4)) == NULL) {
                        return -1;
                }
                int to = 0;
                strcpy(fakeenv[to++], "TZ=");
                int from = 0;
                for (from = 0; environ[from] != NULL; ++from) {
                        if (strncmp(environ[from], "TZ=", 3) != 0) {
                                fakeenv[to++] = environ[from];
                        }
                }
                fakeenv[to] = NULL;
        }

        if (pthread_mutex_lock(&mutex)) {
                result = DT_SYSTEM_CALL_ERROR;
                goto cleanup_fakeenv;
        }

        if (tz_name) {
                // Substituting environment if TZ has been defined
                oldenv = environ;
                environ = fakeenv;
                // Setting timezone
                strcpy(&fakeenv[0][3], tz_name);
                tzset();
        }

        struct tm tm;
        if (localtime_r(&(timestamp->second), &tm) == NULL) {
                result = DT_INVALID_ARGUMENT;
        } else {
                result = DT_OK;
        }
        if (tz_name) {
                // Restoring environment
                environ = oldenv;
                // Restoring timezone
                tzset();
        }
unlock_mutex:
        if (pthread_mutex_unlock(&mutex)) {
                result = DT_SYSTEM_CALL_ERROR;
        }
cleanup_fakeenv:
        if (tz_name) {
                free(fakeenv);
        }
        if (result == DT_OK) {
                result = dt_tm_to_representation(&tm, timestamp->nano_second, representation);
        }
        return result;
}

dt_status_t dt_representation_to_timestamp(const dt_representation_t *representation, const char *tz_name,
                dt_timestamp_t *first_timestamp, dt_timestamp_t *second_timestamp)
{
        // TODO: System timezone database usage!
 
        if (!representation || !first_timestamp) {
                return DT_INVALID_ARGUMENT;
        }

        dt_status_t result = DT_UNKNOWN_ERROR;

        extern char **environ;

        char **fakeenv = NULL;
        char **oldenv = NULL;

        if (tz_name) {
                // Preparing fake environment if TZ has been defined
                int i = 0;
                for (i = 0; environ[i] != NULL; ++i) {
                        continue;
                }
                fakeenv = malloc((i + 2) * sizeof *fakeenv);
                size_t longest = 1024;
                if (fakeenv == NULL || (fakeenv[0] = malloc(longest + 4)) == NULL) {
                        return -1;
                }
                int to = 0;
                strcpy(fakeenv[to++], "TZ=");
                int from = 0;
                for (from = 0; environ[from] != NULL; ++from) {
                        if (strncmp(environ[from], "TZ=", 3) != 0) {
                                fakeenv[to++] = environ[from];
                        }
                }
                fakeenv[to] = NULL;
        }

        if (pthread_mutex_lock(&mutex)) {
                result = DT_SYSTEM_CALL_ERROR;
                goto cleanup_fakeenv;
        }

        if (tz_name) {
                // Substituting environment if TZ has been defined
                oldenv = environ;
                environ = fakeenv;
                // Setting timezone
                strcpy(&fakeenv[0][3], tz_name);
                tzset();
        }

        struct tm tm;
        dt_representation_to_tm_private(representation, &tm);

        time_t posix_time = mktime(&tm);
        if (posix_time < 0) {
                result = DT_INVALID_ARGUMENT;
        } else {
                result = DT_OK;
        }

        if (tz_name) {
                // Restoring environment
                environ = oldenv;
                // Restoring timezone
                tzset();
        }
unlock_mutex:
        if (pthread_mutex_unlock(&mutex)) {
                result = DT_SYSTEM_CALL_ERROR;
        }
cleanup_fakeenv:
        if (tz_name) {
                free(fakeenv);
        }
        if (result == DT_OK) {
                first_timestamp->second = posix_time;
                first_timestamp->nano_second = representation->nano_second;
        }
        return result;
}
