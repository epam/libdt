#include <libdt/dt.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dt_private.h"

#include <stdio.h>

/*
 * Cross-platform date/time handling library for C.
 * Main source file.
 *
 * Authors: Ilya Storozhilov <Ilya_Storozhilov@epam.com>, Andrey Kuznetsov
 * <Andrey_Kuznetsov@epam.com>, Maxim Kot <Maxim_Kot@epam.com>
 * License: Public Domain, http://en.wikipedia.org/wiki/Public_domain
 */

static const int month_days[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

dt_bool_t dt_is_leap_year(int year)
{
    if (year < 1582) {
        // No leap year in Julian calendar
        return (abs(year) % 4 == 0);
    } else {
        return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
    }
}

const char *dt_strerror(dt_status_t status)
{
    static const char *no_error_message = "<No error>";
    static const char *invalid_argument_error_message = "Invalid argument";
    static const char *timezone_not_found_error_message = "Timezone not found";
    static const char *system_call_error_message = "System call error";
    static const char *unknown_error_message = "Unknown error";
    static const char *malloc_error_message = "malloc returned NULL";
    static const char *no_more_items_error_message = "No more items in collection";
    static const char *invalid_status_error_message = "<Invalid result status>";

    switch (status) {
        case DT_OK:
            return no_error_message;
        case DT_INVALID_ARGUMENT:
            return invalid_argument_error_message;
        case DT_TIMEZONE_NOT_FOUND:
            return timezone_not_found_error_message;
        case DT_SYSTEM_CALL_ERROR:
            return system_call_error_message;
        case DT_UNKNOWN_ERROR:
            return unknown_error_message;
        case DT_MALLOC_ERROR:
            return malloc_error_message;
        case DT_NO_MORE_ITEMS:
            return no_more_items_error_message;
        default:
            return invalid_status_error_message;
    }
}

dt_bool_t dt_validate_representation(int year, unsigned short month, unsigned short day, unsigned short hour, unsigned short minute, unsigned short second, unsigned long nano_second)
{
    // Simple checking for invalid values
    if (year == 0 || month < 1 || month > 12 || day < 1 || hour > 24 || minute > 59 || second > 59 || nano_second > 999999999UL) {
        return DT_FALSE;
    }
    // Passage from Julian to Gregorian calendar
    if (year == 1582 && month == 10 && day > 4 && day < 15) {
        return DT_FALSE;
    }
    // Checking leap year
    if (dt_is_leap_year(year) && month == 2 && day == 29) {
        return DT_TRUE;
    }
    // Checking month days
    return day <= month_days[month] ? DT_TRUE : DT_FALSE;
}

dt_status_t dt_compare_timestamps(const dt_timestamp_t *lhs, const dt_timestamp_t *rhs, dt_compare_resiult_t *result)
{
    if (!lhs || !rhs || !result) {
        return DT_INVALID_ARGUMENT;
    }
    if (lhs->second > rhs->second) {
        *result = MORE;
        return DT_OK;
    }
    if (lhs->second < rhs->second) {
        *result = LESS;
        return DT_OK;
    }
    if (lhs->nano_second > rhs->nano_second) {
        *result = MORE;
        return DT_OK;
    }
    if (lhs->nano_second < rhs->nano_second) {
        *result = LESS;
        return DT_OK;
    }
    *result = EQUAL;
    return DT_OK;
}

dt_status_t dt_offset_between(const dt_timestamp_t *lhs, const dt_timestamp_t *rhs, dt_offset_t *result)
{
    dt_compare_resiult_t cr = EQUAL;
    const dt_timestamp_t *lower_timestamp = NULL;
    const dt_timestamp_t *higher_timestamp = NULL;
    dt_bool_t is_forward = DT_FALSE;
    unsigned long seconds = 0;
    long nano_seconds = 0;
    dt_status_t s = DT_UNKNOWN_ERROR;


    if (!lhs || !rhs || !result) {
        return DT_INVALID_ARGUMENT;
    }

    s = dt_compare_timestamps(lhs, rhs, &cr);
    lower_timestamp = cr < 0 ? lhs : rhs;
    higher_timestamp = cr < 0 ? rhs : lhs;
    seconds = higher_timestamp->second - lower_timestamp->second;
    nano_seconds = (long) higher_timestamp->nano_second - (long) lower_timestamp->nano_second;
    is_forward = cr < 0 ? DT_TRUE : DT_FALSE;

    if (s != DT_OK) {
        return s;
    }
    if (cr == EQUAL) {
        // Equal timestamps case
        result->duration.seconds = 0L;
        result->duration.nano_seconds = 0L;
        result->is_forward = DT_TRUE;
        return DT_OK;
    }

    if (nano_seconds < 0L) {
        seconds -= 1L;
        nano_seconds += 1000000000UL;
    }
    result->duration.seconds = seconds;
    result->duration.nano_seconds = nano_seconds;
    result->is_forward = is_forward;
    return DT_OK;
}

dt_status_t dt_apply_offset(const dt_timestamp_t *lhs, const dt_offset_t *rhs, dt_timestamp_t *result)
{
    long second = 0;
    unsigned long nano_second = 0;
    if (!lhs || !rhs || !result) {
        return DT_INVALID_ARGUMENT;
    }

    if (rhs->is_forward) {
        nano_second = lhs->nano_second + rhs->duration.nano_seconds;
        second = lhs->second + rhs->duration.seconds + nano_second / 1000000000UL;
        nano_second %= 1000000000UL;
    } else {
        if (lhs->nano_second > rhs->duration.nano_seconds) {
            nano_second = lhs->nano_second - rhs->duration.nano_seconds;
            second = lhs->second - rhs->duration.seconds + nano_second / 1000000000UL;
            nano_second %= 1000000000UL;
        } else {
            nano_second = rhs->duration.nano_seconds - lhs->nano_second;
            second = lhs->second - rhs->duration.seconds - nano_second / 1000000000UL - 1;
            nano_second %= 1000000000UL;
            nano_second = 1000000000UL - nano_second;
        }
    }
    result->second = second;
    result->nano_second = nano_second;
    return DT_OK;
}

dt_status_t dt_init_interval(long seconds, unsigned long nano_seconds, dt_interval_t *result)
{
    if (!result) {
        return DT_INVALID_ARGUMENT;
    }
    result->seconds = seconds + nano_seconds / 1000000000UL;
    result->nano_seconds = nano_seconds % 1000000000UL;
    return DT_OK;
}

dt_status_t dt_compare_intervals(const dt_interval_t *lhs, const dt_interval_t *rhs, dt_compare_resiult_t *result)
{
    if (!lhs || !rhs || !result) {
        return DT_INVALID_ARGUMENT;
    }
    if (lhs->seconds > rhs->seconds) {
        *result = MORE;
        return DT_OK;
    }
    if (lhs->seconds < rhs->seconds) {
        *result = LESS;
        return DT_OK;
    }
    if (lhs->nano_seconds > rhs->nano_seconds) {
        *result = MORE;
        return DT_OK;
    }
    if (lhs->nano_seconds < rhs->nano_seconds) {
        *result = LESS;
        return DT_OK;
    }
    *result = EQUAL;
    return 0;
}

dt_status_t dt_sum_intervals(const dt_interval_t *lhs, const dt_interval_t *rhs, dt_interval_t *result)
{
    unsigned long seconds = 0;
    unsigned long nano_seconds = 0;

    if (!lhs || !rhs || !result) {
        return DT_INVALID_ARGUMENT;
    }

    seconds = lhs->seconds + rhs->seconds + (lhs->nano_seconds + rhs->nano_seconds) / 1000000000UL;
    nano_seconds = (lhs->nano_seconds + rhs->nano_seconds) % 1000000000UL;

    result->seconds = seconds;
    result->nano_seconds = nano_seconds;
    return DT_OK;
}

dt_status_t dt_sub_intervals(const dt_interval_t *lhs, const dt_interval_t *rhs, dt_interval_t *result)
{
    dt_compare_resiult_t cr = 0;
    unsigned long seconds = 0;
    unsigned long nano_seconds = 0;
    dt_status_t s = DT_UNKNOWN_ERROR;

    if (!lhs || !rhs || !result) {
        return DT_INVALID_ARGUMENT;
    }

    seconds = lhs->seconds - rhs->seconds - (lhs->nano_seconds < rhs->nano_seconds ? 1L : 0L);
    if (lhs->nano_seconds < rhs->nano_seconds) {
        nano_seconds = 1000000000UL + lhs->nano_seconds - rhs->nano_seconds;
    } else {
        nano_seconds = lhs->nano_seconds - rhs->nano_seconds;
    }
    s = dt_compare_intervals(lhs, rhs, &cr);

    if (s != DT_OK) {
        return s;
    }
    if (cr <= 0) {
        result->seconds = 0L;
        result->nano_seconds = 0L;
        return DT_OK;
    }

    result->seconds = seconds;
    result->nano_seconds = nano_seconds;
    return DT_OK;
}

dt_status_t dt_mul_interval(const dt_interval_t *lhs, double rhs, dt_interval_t *result)
{
    long double v = 0;
    long double rv = 0;

    if (!lhs || !result) {
        return DT_INVALID_ARGUMENT;
    }

    v = (long double) lhs->seconds + (long double) lhs->nano_seconds / 1000000000UL;
    rv = v * rhs;

    result->seconds = (unsigned long)floorl(rv);
    result->nano_seconds = (unsigned long)floorl(fmodl(rv, 1.0) * 1000000000UL);
    return DT_OK;
}

static dt_status_t dt_init_representation_without_check(int year, int month, int day, int hour, int minute, int second, unsigned long nano_second,
                                                        dt_representation_t *result)
{
    if (!result) {
        return DT_INVALID_ARGUMENT;
    }

    memset(result, 0, sizeof(dt_representation_t));
    result->year = year;
    result->month = month;
    result->day = day;
    result->hour = hour;
    result->minute = minute;
    result->second = second;
    result->nano_second = nano_second;
    return DT_OK;
}

dt_status_t dt_init_representation(int year, unsigned short month, unsigned short day, unsigned short hour, unsigned short minute, unsigned short second, unsigned long nano_second,
                                   dt_representation_t *result)
{
    if (!dt_validate_representation(year, month, day, hour, minute, second, nano_second)) {
        return DT_INVALID_ARGUMENT;
    }
    return dt_init_representation_without_check(year, month, day, hour, minute, second, nano_second,
                                                result);
}

dt_status_t dt_representation_day_of_week(const dt_representation_t *representation, int *day_of_week)
{
    int year = 0;
    int month = 0;

    // TODO: Julian calendar support - now only Gregorian is supported
    if (!representation || !day_of_week) {
        return DT_INVALID_ARGUMENT;
    }

    year = (representation->month == 1 || representation->month == 2) ? representation->year - 1 : representation->year;
    month = (representation->month == 1 || representation->month == 2) ? representation->month + 12 : representation->month;
    // See http://en.wikipedia.org/wiki/Zeller%27s_congruence

    *day_of_week = (representation->day + (month + 1) * 26 / 10 + year + year / 4 + 6 * year / 100 + year / 400) % 7;
    *day_of_week = *day_of_week == 0 ? 7 : *day_of_week;
    return DT_OK;
}

dt_status_t dt_representation_day_of_year(const dt_representation_t *representation, int *day_of_year)
{
    unsigned i = 0;
    // TODO: Julian calendar support - now only Gregorian is supported
    if (!representation || !day_of_year) {
        return DT_INVALID_ARGUMENT;
    }
    *day_of_year = 0;

    for (i = 1; i < representation->month; ++i) {
        *day_of_year += (i == 2 && dt_is_leap_year(representation->year)) ? 29 : month_days[i];
    }
    *day_of_year += representation->day;
    return DT_OK;
}

dt_status_t dt_representation_to_tm(const dt_representation_t *representation, struct tm *tm)
{
    int dow = 0;
    int doy = 0;
    dt_status_t s = DT_UNKNOWN_ERROR;


    if (!representation || !tm) {
        return DT_INVALID_ARGUMENT;
    }

    s = dt_representation_day_of_week(representation, &dow);
    if (s != DT_OK) {
        return s;
    }

    s = dt_representation_day_of_year(representation, &doy);
    if (s != DT_OK) {
        return s;
    }
    tm->tm_year = representation->year - 1900;
    tm->tm_mon = representation->month - 1;
    tm->tm_mday = representation->day;
    tm->tm_hour = representation->hour;
    tm->tm_min = representation->minute;
    tm->tm_sec = representation->second;
    tm->tm_wday = dow - 1;
    tm->tm_yday = doy - 1;
    tm->tm_isdst = -1;
    return DT_OK;
}

dt_status_t dt_tm_to_representation(const struct tm *tm, long nano_second, dt_representation_t *representation)
{
    if (!tm || !representation) {
        return DT_INVALID_ARGUMENT;
    }
    return dt_init_representation(1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, nano_second, representation);
}

dt_status_t dt_tm_to_representation_withoutcheck(const struct tm *tm, long nano_second, dt_representation_t *representation)
{
    if (!tm || !representation) {
        return DT_INVALID_ARGUMENT;
    }
    return dt_init_representation_without_check(1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, nano_second, representation);
}

int strftime_tz(const struct tm *representation, const char *tz_name, const char *fmt,
                char *str_buffer, size_t str_buffer_size)
{
    dt_representation_t rep = {0};
    dt_status_t status = DT_UNKNOWN_ERROR;

    if (!representation || !tz_name || !fmt || !str_buffer || str_buffer_size <= 0) {
        return DT_INVALID_ARGUMENT;
    }

    status = dt_tm_to_representation(representation, 0, &rep);
    if (status != DT_OK) {
        return status;
    }

    return dt_to_string(&rep, tz_name, fmt, str_buffer, str_buffer_size);
}

int strptime_tz(const char *str, const char *fmt, struct tm *representation)
{
    dt_representation_t rep = {0};
    dt_status_t status = DT_UNKNOWN_ERROR;

    if (!representation || !fmt || !str) {
        return DT_INVALID_ARGUMENT;
    }

    status = dt_from_string(str, fmt, &rep, NULL, 0);
    if (status != DT_OK) {
        return status;
    }

    return dt_representation_to_tm(&rep, representation);
}

int localtime_tz(const time_t *time, const char *tz_name, struct tm *result)
{
    dt_status_t status = DT_UNKNOWN_ERROR;
    dt_timestamp_t t = {0};
    dt_representation_t rep = {0};
    dt_timezone_t tz = {0,};


    if (!time || !result || !tz_name) {
        return EXIT_FAILURE;
    }

    if (dt_timezone_lookup(tz_name, &tz) != DT_OK) {
        return EXIT_FAILURE;
    }

    status = dt_posix_time_to_timestamp(*time, 0, &t);
    if (status != DT_OK) {
        return EXIT_FAILURE;
    }

    status = dt_timestamp_to_representation(&t, &tz, &rep);
    dt_timezone_cleanup(&tz);
    if (status != DT_OK) {
        return EXIT_FAILURE;
    }

    status = dt_representation_to_tm(&rep, result);
    if (status != DT_OK) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int mktime_tz(const struct tm *tm, const char *tz_name, time_t *result)
{
    dt_status_t status = DT_UNKNOWN_ERROR;
    dt_timestamp_t t = {0};
    dt_representation_t rep = {0};
    dt_timezone_t tz = {0,};
    unsigned long nano = 0;

    if (!tm || !result || !tz_name) {
        return EXIT_FAILURE;
    }
    if (dt_timezone_lookup(tz_name, &tz) != DT_OK) {
        return EXIT_FAILURE;
    }

    status = dt_tm_to_representation(tm, 0, &rep);
    if (status != DT_OK) {
        return EXIT_FAILURE;
    }

    status = dt_representation_to_timestamp(&rep, &tz, &t, NULL);
    dt_timezone_cleanup(&tz);
    if (status != DT_OK) {
        return EXIT_FAILURE;
    }

    status = dt_timestamp_to_posix_time(&t, result, &nano);
    if (status != DT_OK) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
