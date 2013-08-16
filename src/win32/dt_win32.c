#include <dt.h>
#include "../dt_private.h"

// WinAPI
#include <Windows.h>
#include <WinReg.h>
#include <WinBase.h>
#include <winnt.h>

// Registry timezones database path
static const char REG_TIME_ZONES[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\";

static dt_status_t dt_filetime_to_timestamp(const PFILETIME ft, dt_timestamp_t *ts)
{
        LARGE_INTEGER li;

        if (!ft || !ts) {
                return DT_INVALID_ARGUMENT;
        }
        li.QuadPart = ft->dwHighDateTime;
        li.QuadPart <<= 32;
        li.QuadPart |= ft->dwLowDateTime;
        ts->second = li.QuadPart / 10000000;
        ts->nano_second = li.QuadPart % 10000000 * 100;
        return DT_OK;
}

static dt_status_t dt_timestamp_to_filetime(const dt_timestamp_t *ts, PFILETIME ft)
{
        LARGE_INTEGER li;

        if (!ts || !ft) {
                return DT_INVALID_ARGUMENT;
        }
        li.QuadPart = ts->second * 10000000;
        li.QuadPart += ts->nano_second / 100;
        ft->dwLowDateTime = li.QuadPart;
        li.QuadPart >>= 32;
        ft->dwHighDateTime = li.QuadPart;
        return DT_OK;
}

dt_status_t dt_now(dt_timestamp_t *result)
{
        FILETIME ft = {0};
        LARGE_INTEGER li;
        dt_status_t s;

        if (!result) {
                return DT_INVALID_ARGUMENT;
        }
        GetSystemTimeAsFileTime(&ft);
        s = dt_filetime_to_timestamp(&ft, result);
        return s;
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
        if (!timestamp || !representation) {
                return DT_INVALID_ARGUMENT;
        }
        // TODO: Implementation
        return DT_UNKNOWN_ERROR;
}

dt_status_t dt_representation_to_timestamp(const dt_representation_t *representation, const char *tz_name,
                dt_timestamp_t *first_timestamp, dt_timestamp_t *second_timestamp)
{
        if (!representation || !first_timestamp) {
                return DT_INVALID_ARGUMENT;
        }
        // TODO: Implementation
        return DT_UNKNOWN_ERROR;
}
