#include <dt.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
//#include "tzcode/tzfile.h"
#include "../dt_private.h"

#include <stdio.h>

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
        if (clock_gettime(CLOCK_MONOTONIC, &result->ts) < 0) {
                return DT_SYSTEM_CALL_ERROR;
        }
        return DT_OK;
}

dt_status_t dt_compare_timestamps(const dt_timestamp_t *lhs, const dt_timestamp_t *rhs, int *result)
{
        if (!lhs || !rhs || !result) {
                return DT_INVALID_ARGUMENT;
        }
        if (lhs->ts.tv_sec > rhs->ts.tv_sec) {
                *result = 1;
                return DT_OK;
        }
        if (lhs->ts.tv_sec < rhs->ts.tv_sec) {
                *result = -1;
                return DT_OK;
        }
        if (lhs->ts.tv_nsec > rhs->ts.tv_nsec) {
                *result = 1;
                return DT_OK;
        }
        if (lhs->ts.tv_nsec < rhs->ts.tv_nsec) {
                *result = -1;
                return DT_OK;
        }
        *result = 0;
        return 0;
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
        if (localtime_r(&(timestamp->ts.tv_sec), &tm) == NULL) {
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
                result = dt_init_representation(tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour,
                                tm.tm_min, tm.tm_sec, timestamp->ts.tv_nsec, representation);
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
        tm.tm_year = representation->year;
        tm.tm_mon = representation->month;
        tm.tm_mday = representation->day;
        tm.tm_hour = representation->hour;
        tm.tm_min = representation->minute;
        tm.tm_sec = representation->second;
        tm.tm_isdst = -1;
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
                first_timestamp->ts.tv_sec = posix_time;
                first_timestamp->ts.tv_nsec = representation->nano_second;
        }
        return result;
}

/*dt_status_t dt_representation_day_of_week(const dt_representation_t *representation, int *dow)
{
        if (!representation || !dow) {
                return DT_INVALID_ARGUMENT;
        }

        dt_status_t result = DT_UNKNOWN_ERROR;
        if (pthread_mutex_lock(&mutex)) {
                result = DT_SYSTEM_CALL_ERROR;
                goto return_result;
        }
        struct tm tm;
        tm.tm_year = representation->year;
        tm.tm_mon = representation->month;
        tm.tm_mday = representation->day;
        tm.tm_hour = representation->hour;
        tm.tm_min = representation->minute;
        tm.tm_sec = representation->second;
        tm.tm_isdst = -1;

        printf("%d-%d-%d %d:%d:%d, day of week: %d, day of year: %d\n", tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_wday, tm.tm_yday);

        time_t posix_time = mktime(&tm);

        printf("%d-%d-%d %d:%d:%d, day of week: %d, day of year: %d\n", tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_wday, tm.tm_yday);

        if (posix_time < 0) {
                result = DT_INVALID_ARGUMENT;
                goto return_result;
        }
        if (pthread_mutex_unlock(&mutex)) {
                result = DT_SYSTEM_CALL_ERROR;
        } else {
                result = DT_OK;
                *dow = tm.tm_wday;
        }
return_result:
        return result;
}*/
