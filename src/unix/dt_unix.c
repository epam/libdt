// vim: shiftwidth=4 softtabstop=4
/* Copyright (c) 2013, EPAM Systems. All rights reserved.

Authors:
Ilya Storozhilov <Ilya_Storozhilov@epam.com>,
Andrey Kuznetsov <Andrey_Kuznetsov@epam.com>,
Maxim Kot <Maxim_Kot@epam.com>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <assert.h>

#include <limits.h>
#include <libdt/dt.h>
#include "libtz/tz.h"
#include "libtz/tzfile.h"
#include "../tzmapping.h"

static const time_t WRONG_POSIX_TIME = -1;

dt_status_t dt_now(dt_timestamp_t *result)
{
    if (!result) {
        return DT_INVALID_ARGUMENT;
    }

    struct timespec ts = {0,};
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
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


dt_status_t dt_timestamp_to_representation(const dt_timestamp_t *timestamp, const dt_timezone_t *tz, dt_representation_t *representation)
{
    struct tm tm = {0,};

    if (timestamp == NULL || representation == NULL) {
        return DT_INVALID_ARGUMENT;
    }


    if (tz == NULL) {
        if (localtime_r(&(timestamp->second), &tm) == NULL) {
            return DT_SYSTEM_CALL_ERROR;
        }
        return dt_tm_to_representation(&tm, timestamp->nano_second, representation);
    }

    if (tz != NULL && tz->state == NULL) {
        return DT_INVALID_ARGUMENT;
    }

    if (tz_localtime_r(tz->state, &(timestamp->second), &tm) == NULL) {
        return DT_INVALID_ARGUMENT;
    }

    return dt_tm_to_representation(&tm, timestamp->nano_second, representation);
}

dt_status_t dt_representation_to_timestamp(const dt_representation_t *representation, const dt_timezone_t *timezone,
                                           dt_timestamp_t *first_timestamp, dt_timestamp_t *second_timestamp)
{
    struct tm tm = {0,};
    time_t posix_time = WRONG_POSIX_TIME;

    dt_status_t status = DT_UNKNOWN_ERROR;

    if (!representation || !first_timestamp) {
        return DT_INVALID_ARGUMENT;
    }

    if (DT_OK != (status = dt_representation_to_tm(representation, &tm))) {
        return status;
    }

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

    if (timezone != NULL && timezone->state == NULL) {
        return DT_INVALID_ARGUMENT;
    }


    if (WRONG_POSIX_TIME == (posix_time = tz_mktime(timezone->state, &tm))) {
        return DT_INVALID_ARGUMENT;
    }

    first_timestamp->second = posix_time;
    first_timestamp->nano_second = representation->nano_second;
    return DT_OK;
}

dt_status_t dt_timezone_lookup(const char *timezone_name, dt_timezone_t *timezone)
{
    dt_status_t status = DT_UNKNOWN_ERROR;
    tz_aliases_t *aliases = NULL;
    tz_alias_iterator_t *it = TZMAP_BEGIN;
    tz_alias_t *alias = NULL;
    char path[PATH_MAX] = {0,};

    const struct state *s = NULL;

    if (timezone == NULL || timezone_name == NULL) {
        return DT_INVALID_ARGUMENT;
    }

    if ((status = tzmap_map(timezone_name, &aliases)) != DT_OK) {
        return status;
    }

    while ((status = tzmap_iterate(aliases, &it, &alias)) == DT_OK) {
        if (alias->kind == DT_PREFFERED_TZMAP_TYPE) {
            strcat(path, TZDIR);
            strcat(path, "/");
            strcat(path, alias->name);
            s = tz_alloc(path);
            if (s == NULL) {
                return DT_TIMEZONE_NOT_FOUND;
            }
            timezone->state = s;
            tzmap_free(aliases);
            return DT_OK;
        }
    }

    return status;
}

dt_status_t dt_timezone_cleanup(dt_timezone_t *timezone)
{
    if (timezone == NULL) {
        return DT_INVALID_ARGUMENT;
    }
    tz_free(timezone->state);
    return DT_OK;
}
