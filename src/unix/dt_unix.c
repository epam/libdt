
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <assert.h>

#include "../dt_private.h"
#include "libtz/dt.h"

#include <libtz/tzmapping.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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

dt_status_t dt_timestamp_to_representation(const dt_timestamp_t *timestamp, const char *tz_name, dt_representation_t *representation)
{
    // FIXME: System timezone database usage!

    if (!timestamp || !representation)
        return DT_INVALID_ARGUMENT;

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
        if (fakeenv == NULL || (fakeenv[0] = malloc(longest + 4)) == NULL)
            return DT_MALLOC_ERROR;

        int to = 0;
        strcpy(fakeenv[to++], "TZ=");
        int from = 0;
        for (from = 0; environ[from] != NULL; ++from) {
            if (strncmp(environ[from], "TZ=", 3) != 0)
                fakeenv[to++] = environ[from];
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
    if (pthread_mutex_unlock(&mutex))
        result = DT_SYSTEM_CALL_ERROR;

cleanup_fakeenv:
    if (tz_name)
        free(fakeenv);

    if (result == DT_OK)
        result = dt_tm_to_representation(&tm, timestamp->nano_second, representation);

    return result;
}

dt_status_t dt_representation_to_timestamp(const dt_representation_t *representation, const char *tz_name,
                                           dt_timestamp_t *first_timestamp, dt_timestamp_t *second_timestamp)
{
    // FIXME: System timezone database usage!

    if (!representation || !first_timestamp)
        return DT_INVALID_ARGUMENT;

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
        if (fakeenv == NULL || (fakeenv[0] = malloc(longest + 4)) == NULL)
            return DT_MALLOC_ERROR;

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
    dt_representation_to_tm(representation, &tm);

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
    if (pthread_mutex_unlock(&mutex))
        result = DT_SYSTEM_CALL_ERROR;

cleanup_fakeenv:
    if (tz_name)
        free(fakeenv);

    if (result == DT_OK) {
        first_timestamp->second = posix_time;
        first_timestamp->nano_second = representation->nano_second;
    }
    return result;
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

const char * findTimeZoneByName(const char *tz_name)
{
    dt_status_t status = DT_UNKNOWN_ERROR;
    tz_aliases_t* aliases = NULL;
    tz_alias_iterator_t* it = TZMAP_BEGIN;
    tz_alias_t * alias = NULL;

    if (tzmap_map(tz_name, &aliases) != DT_OK) {
        return 0;
    }

    while((status = tzmap_iterate(aliases, &it, &alias)) == DT_OK) {
        if (alias->kind == TZMAP_OLSEN_NAME) {
            tzmap_free(aliases);
            return alias->name;
        }
    }


    return 0;
}
