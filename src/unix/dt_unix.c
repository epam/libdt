#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <assert.h>

#include "../dt_private.h"
#include <limits.h>
#include <libtz/tz.h>
#include <libtz/tzfile.h>
#include <libdt/dt.h>
#include <dt-private/tzmapping.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static const time_t WRONG_POSIX_TIME = -1;

dt_status_t dt_now(dt_timestamp_t *result)
{
    if (!result)
        return DT_INVALID_ARGUMENT;

    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
        return DT_SYSTEM_CALL_ERROR;

    result->second = ts.tv_sec;
    result->nano_second = ts.tv_nsec;
    return DT_OK;
}

dt_status_t dt_posix_time_to_timestamp(time_t time, unsigned long nano_second, dt_timestamp_t *result)
{
    if (time < 0 || !result)
        return DT_INVALID_ARGUMENT;

    result->second = time;
    result->nano_second = nano_second;
    return DT_OK;
}

dt_status_t dt_timestamp_to_posix_time(const dt_timestamp_t *timestamp, time_t *time, unsigned long *nano_second)
{
    if (!timestamp || !time || timestamp->second < 0)
        return DT_INVALID_ARGUMENT;

    *time = timestamp->second;
    *nano_second = timestamp->nano_second;
    return DT_OK;
}

dt_status_t dt_timestamp_to_representation(const dt_timestamp_t *timestamp, const dt_timezone_t* tz, dt_representation_t *representation)
{

    const struct state *s = NULL;
    struct tm tm = {0,};

    if (timestamp == NULL || representation == NULL)
        return DT_INVALID_ARGUMENT;


    if (tz == NULL) {
        if (localtime_r(&(timestamp->second), &tm) == NULL)
            return DT_SYSTEM_CALL_ERROR;
        return dt_tm_to_representation(&tm, timestamp->nano_second, representation);
    }

    if (tz != NULL && tz->state == NULL)
        return DT_INVALID_ARGUMENT;

    if (tz_localtime_r(tz->state, &(timestamp->second), &tm) == NULL) {
        return DT_INVALID_ARGUMENT;
    }

    tz_free(s);
    return dt_tm_to_representation(&tm, timestamp->nano_second, representation);
}

dt_status_t dt_representation_to_timestamp(const dt_representation_t *representation, const dt_timezone_t *timezone,
                                           dt_timestamp_t *first_timestamp, dt_timestamp_t *second_timestamp)
{
    const struct state *s = NULL;
    char path[PATH_MAX + sizeof(char) + 1] = {0,};
    struct tm tm = {0,};
    time_t posix_time = WRONG_POSIX_TIME;

    dt_status_t status = DT_UNKNOWN_ERROR;

    if (!representation || !first_timestamp)
        return DT_INVALID_ARGUMENT;

    if (DT_OK != (status = dt_representation_to_tm(representation, &tm)))
        return status;

    if (timezone == NULL) {
        posix_time = mktime(&tm);
        if (posix_time != WRONG_POSIX_TIME) {
            first_timestamp->second = posix_time;
            first_timestamp->nano_second = representation->nano_second;
            return DT_OK;
        } else {
            return DT_SYSTEM_CALL_ERROR;
        }

    }

    if (timezone != NULL && timezone->state == NULL)
        return DT_INVALID_ARGUMENT;


    if (WRONG_POSIX_TIME == (posix_time = tz_mktime(timezone->state, &tm))) {
        tz_free(s);
        return DT_INVALID_ARGUMENT;
    }

    tz_free(s);
    first_timestamp->second = posix_time;
    first_timestamp->nano_second = representation->nano_second;
    return DT_OK;
}

dt_status_t dt_to_string(const dt_representation_t *representation, const char *tz_name, const char *fmt,
                         char *str_buffer, size_t str_buffer_size)
{
    dt_status_t status = DT_UNKNOWN_ERROR;
    struct tm tm = {0};

    if (!representation || !tz_name || !fmt || !str_buffer || str_buffer_size <= 0)
        return DT_INVALID_ARGUMENT;

    status = dt_representation_to_tm(representation, &tm);
    if (status != DT_OK)
        return status;

    size_t size = strftime(str_buffer, str_buffer_size, fmt, &tm);
    if (size > 0)
        return DT_OK;
    return status;

}

dt_status_t dt_from_string(const char *str, const char *fmt, dt_representation_t *representation,
                           char *tz_name_buffer, size_t tz_name_buffer_size)
{
    char *result = NULL;
    struct tm tm = {0};
    dt_status_t status = DT_SYSTEM_CALL_ERROR;

    if (!representation || !str || !fmt)
        return DT_INVALID_ARGUMENT;

    result = strptime(str, fmt, &tm);//FIXME: Why warning?
    if (result == NULL)
        return status;
    if (*result != '\0')// end of string
        return status;

    status = dt_tm_to_representation_withoutcheck(&tm, 0, representation);
    if (status != DT_OK)
        return status;

    return DT_OK;
}

dt_status_t dt_timezone_lookup(const char* timezone_name, dt_timezone_t *timezone)
{
    dt_status_t status = DT_UNKNOWN_ERROR;
    tz_aliases_t *aliases = NULL;
    tz_alias_iterator_t *it = TZMAP_BEGIN;
    tz_alias_t *alias = NULL;
    char path[PATH_MAX + sizeof(char) + 1] = {0,};

    const struct state* s = NULL;

    if (timezone == NULL || timezone_name == NULL) {
        return DT_INVALID_ARGUMENT;
    }

    if ((status = tzmap_map(timezone_name, &aliases)) != DT_OK) {
        return status;
    }

    while ((status = tzmap_iterate(aliases, &it, &alias)) == DT_OK) {
        if (alias->kind == PREFERED_TZMAP_TYPE) {
            strcat(path, TZDIR);
            strcat(path, "/");
            strcat(path, alias->name);
            s = tz_alloc(path);
            if (s == NULL)
                return DT_TIMEZONE_NOT_FOUND;
            timezone->state = s;
            tzmap_free(aliases);
            return DT_OK;
        }
    }

    return status;
}

dt_status_t dt_timezone_cleanup(dt_timezone_t *timezone)
{
    if (timezone == NULL)
        return DT_INVALID_ARGUMENT;
    tz_free(timezone->state);
    return DT_OK;
}
