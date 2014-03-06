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

#define LIBDT_EXPORTS
#include <libdt/dt_posix.h>
#include <libdt/dt.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <float.h>
#include <ctype.h>

/*
 * Cross-platform date/time handling library for C.
 * Main source file.
 *
 * Authors: Ilya Storozhilov <Ilya_Storozhilov@epam.com>, Andrey Kuznetsov
 * <Andrey_Kuznetsov@epam.com>, Maxim Kot <Maxim_Kot@epam.com>
 *
 * License: TODO: To be clarified!
 */

static const int month_days[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static const unsigned long MAX_NANOSECONDS = 999999999UL;

static dt_bool_t is_unsigned_long_sum_overflows(unsigned long lhs, unsigned long rhs)
{
    if (rhs > ULONG_MAX - lhs ) {
        return DT_TRUE;
    }
    return DT_FALSE;
}

dt_bool_t dt_is_leap_year(int year)
{
    if (year <= 0) {
        return DT_FALSE;
    }
    if (year < 1582) {
        // Simple leap year calculation in Julian calendar
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
    static const char *no_more_items_error_message = "No more items in collection";
    static const char *overflow_error_message = "Operation caused owerflow";
    static const char *unknown_error_message = "Unknown error";
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
        case DT_NO_MORE_ITEMS:
            return no_more_items_error_message;
        case DT_OVERFLOW:
            return overflow_error_message;
        case DT_UNKNOWN_ERROR:
            return unknown_error_message;
        default:
            return invalid_status_error_message;
    }
}

dt_bool_t dt_validate_timestamp(const dt_timestamp_t *timestamp)
{
    if (timestamp == NULL ||
            timestamp->nano_second > MAX_NANOSECONDS) {
        return DT_FALSE;
    }

    return DT_TRUE;
}

dt_status_t dt_compare_timestamps(const dt_timestamp_t *lhs, const dt_timestamp_t *rhs, dt_compare_result_t *result)
{
    if (dt_validate_timestamp(lhs) != DT_TRUE || dt_validate_timestamp(rhs) != DT_TRUE || !result) {
        return DT_INVALID_ARGUMENT;
    }

    if (lhs->second > rhs->second) {
        *result = DT_GREATER;
        return DT_OK;
    }
    if (lhs->second < rhs->second) {
        *result = DT_LESSER;
        return DT_OK;
    }
    if (lhs->nano_second > rhs->nano_second) {
        *result = DT_GREATER;
        return DT_OK;
    }
    if (lhs->nano_second < rhs->nano_second) {
        *result = DT_LESSER;
        return DT_OK;
    }
    *result = DT_EQUALS;
    return DT_OK;
}

dt_status_t dt_offset_between(const dt_timestamp_t *lhs, const dt_timestamp_t *rhs, dt_offset_t *result)
{
    dt_compare_result_t cr = DT_EQUALS;
    const dt_timestamp_t *lower_timestamp = NULL;
    const dt_timestamp_t *higher_timestamp = NULL;
    dt_bool_t is_forward = DT_FALSE;
    unsigned long seconds = 0;
    long nano_seconds = 0;
    dt_status_t s = DT_UNKNOWN_ERROR;


    if (dt_validate_timestamp(lhs) != DT_TRUE || dt_validate_timestamp(rhs) != DT_TRUE || !result) {
        return DT_INVALID_ARGUMENT;
    }

    s = dt_compare_timestamps(lhs, rhs, &cr);
    if (s != DT_OK) {
        return s;
    }
    lower_timestamp = cr < 0 ? lhs : rhs;
    higher_timestamp = cr < 0 ? rhs : lhs;
    seconds = higher_timestamp->second - lower_timestamp->second;
    nano_seconds = (long) higher_timestamp->nano_second - (long) lower_timestamp->nano_second;
    is_forward = cr < 0 ? DT_TRUE : DT_FALSE;


    if (cr == DT_EQUALS) {
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

dt_bool_t dt_validate_offset(const dt_offset_t *offset)
{
    if (offset == NULL || dt_validate_interval(&offset->duration) != DT_TRUE) {
        return DT_FALSE;
    }
    return DT_TRUE;
}

static dt_status_t dt_apply_interval_forward(const dt_timestamp_t *lhs, const dt_interval_t *duration, dt_timestamp_t *result)
{

    unsigned long duration_seconds_full_part = 0UL;
    unsigned long nanoseconds_second_part = 0UL;
    unsigned long nano_seconds = 0UL;
    unsigned long max = LONG_MAX;

    nano_seconds = lhs->nano_second + duration->nano_seconds;//cannot be owerflowed
    nanoseconds_second_part = nano_seconds / 1000000000UL;
    duration_seconds_full_part = duration->seconds + nanoseconds_second_part;
    if (is_unsigned_long_sum_overflows(duration->seconds, nanoseconds_second_part) == DT_TRUE ||
            (duration_seconds_full_part >  max - lhs->second)) {
        return DT_OVERFLOW;
    }

    result->second = duration_seconds_full_part + lhs->second;
    result->nano_second = nano_seconds % 1000000000UL;
    return DT_OK;
}

static dt_status_t dt_apply_interval_backward(const dt_timestamp_t *lhs, const dt_interval_t *duration, dt_timestamp_t *result)
{

    long min = LONG_MIN;
    if (lhs->nano_second >= duration->nano_seconds) {
        if (duration->seconds > (unsigned long)(-min) + lhs->second) {
            return DT_OVERFLOW;
        }
        result->nano_second = lhs->nano_second - duration->nano_seconds;
        result->second = lhs->second - duration->seconds;
    } else {
        if (lhs->second == LONG_MIN || (duration->seconds  > (unsigned long)(-min) + lhs->second - 1)) {
            return DT_OVERFLOW;
        }
        result->nano_second = duration->nano_seconds - lhs->nano_second;
        result->second = lhs->second - duration->seconds - 1;
        result->nano_second = 1000000000UL - result->nano_second;
    }
    return DT_OK;
}

dt_status_t dt_apply_offset(const dt_timestamp_t *lhs, const dt_offset_t *rhs, dt_timestamp_t *result)
{

    if (dt_validate_timestamp(lhs) != DT_TRUE || dt_validate_offset(rhs) != DT_TRUE || !result) {
        return DT_INVALID_ARGUMENT;
    }


    if (rhs->is_forward) {
        return dt_apply_interval_forward(lhs, &rhs->duration, result);
    } else {
        return dt_apply_interval_backward(lhs, &rhs->duration, result);
    }
    return DT_OK;
}

dt_status_t dt_init_interval(unsigned long seconds, unsigned long nano_seconds, dt_interval_t *result)
{
    unsigned long seconds_delta = 0;
    if (!result) {
        return DT_INVALID_ARGUMENT;
    }

    seconds_delta = nano_seconds / 1000000000UL;

    if (is_unsigned_long_sum_overflows(seconds, seconds_delta) == DT_TRUE) {
        return DT_OVERFLOW;
    }

    result->seconds = seconds + seconds_delta;
    result->nano_seconds = nano_seconds % 1000000000UL;

    return DT_OK;
}

dt_status_t dt_compare_intervals(const dt_interval_t *lhs, const dt_interval_t *rhs, dt_compare_result_t *result)
{
    if (dt_validate_interval(lhs) == DT_FALSE || dt_validate_interval(rhs) == DT_FALSE || !result) {
        return DT_INVALID_ARGUMENT;
    }

    if (lhs->seconds > rhs->seconds) {
        *result = DT_GREATER;
        return DT_OK;
    }
    if (lhs->seconds < rhs->seconds) {
        *result = DT_LESSER;
        return DT_OK;
    }
    if (lhs->nano_seconds > rhs->nano_seconds) {
        *result = DT_GREATER;
        return DT_OK;
    }
    if (lhs->nano_seconds < rhs->nano_seconds) {
        *result = DT_LESSER;
        return DT_OK;
    }
    *result = DT_EQUALS;
    return DT_OK;
}

dt_status_t dt_sum_intervals(const dt_interval_t *lhs, const dt_interval_t *rhs, dt_interval_t *result)
{
    unsigned long seconds = 0;
    unsigned long nano_seconds = 0;
    unsigned long seconds_nanoseconds_part = 0;
    unsigned long full_seconds_part  = 0;

    if (dt_validate_interval(lhs) == DT_FALSE || dt_validate_interval(rhs) == DT_FALSE || !result) {
        return DT_INVALID_ARGUMENT;
    }

    seconds_nanoseconds_part = (lhs->nano_seconds + rhs->nano_seconds) / 1000000000UL;
    full_seconds_part = rhs->seconds + seconds_nanoseconds_part;
    if (is_unsigned_long_sum_overflows(rhs->seconds, seconds_nanoseconds_part) == DT_TRUE ||
            is_unsigned_long_sum_overflows(lhs->seconds, full_seconds_part) == DT_TRUE) {
        return DT_OVERFLOW;
    }
    seconds = lhs->seconds + full_seconds_part;
    nano_seconds = (lhs->nano_seconds + rhs->nano_seconds) % 1000000000UL;

    result->seconds = seconds;
    result->nano_seconds = nano_seconds;
    return DT_OK;
}

dt_status_t dt_sub_intervals(const dt_interval_t *lhs, const dt_interval_t *rhs, dt_interval_t *result)
{
    dt_compare_result_t cr = 0;
    unsigned long seconds = 0;
    unsigned long nano_seconds = 0;
    dt_status_t s = DT_UNKNOWN_ERROR;

    if (!lhs || !rhs || !result) {
        return DT_INVALID_ARGUMENT;
    }

    s = dt_compare_intervals(lhs, rhs, &cr);

    if (s != DT_OK) {
        return s;
    }
    if (cr == DT_LESSER) {
        return DT_OVERFLOW;
    }

    seconds = lhs->seconds - rhs->seconds - (lhs->nano_seconds < rhs->nano_seconds ? 1L : 0L);
    if (lhs->nano_seconds < rhs->nano_seconds) {
        nano_seconds = 1000000000UL + lhs->nano_seconds - rhs->nano_seconds;
    } else {
        nano_seconds = lhs->nano_seconds - rhs->nano_seconds;
    }


    result->seconds = seconds;
    result->nano_seconds = nano_seconds;
    return DT_OK;
}

dt_status_t dt_mul_interval(const dt_interval_t *lhs, double rhs, dt_interval_t *result)
{
    double v = 0;
    double rv = 0;

    if (dt_validate_interval(lhs) == DT_FALSE || !result) {
        return DT_INVALID_ARGUMENT;
    }

    v = (double) lhs->seconds + (double) lhs->nano_seconds / 1000000000.0;
    rv = v * rhs;
#if defined(__CYGWIN__) || defined(WIN32)
    if (!_finite(rv)) {
#else
    if (!isfinite(rv)) {
#endif
        return DT_OVERFLOW;
    }

    result->seconds = (unsigned long)floor(rv);
    if (result->seconds == 0 && v != 0 && rhs != 0) {
        return DT_OVERFLOW;
    }
    result->nano_seconds = (unsigned long)floor(fmod(rv, 1.0) * 1000000000UL);
    return DT_OK;
}

dt_bool_t dt_validate_representation(const dt_representation_t *representation)
{
    if (!representation) {
        return DT_FALSE;
    }
    // Simple checking for invalid values
    if (representation->year == 0 || representation->month < 1 || representation->month > 12 || representation->day < 1 ||
            representation->hour > 24 || representation->minute > 59 || representation->second > 59 || representation->nano_second > 999999999UL) {
        return DT_FALSE;
    }
    // Passage from Julian to Gregorian calendar
    if (representation->year == 1582 && representation->month == 10 && representation->day > 4 && representation->day < 15) {
        return DT_FALSE;
    }
    // Checking leap year
    if (dt_is_leap_year(representation->year) && representation->month == 2 && representation->day == 29) {
        return DT_TRUE;
    }
    // Checking month days
    return representation->day <= month_days[representation->month] ? DT_TRUE : DT_FALSE;
}

dt_status_t dt_init_representation(int year, unsigned short month, unsigned short day, unsigned short hour, unsigned short minute, unsigned short second, unsigned long nano_second,
                                   dt_representation_t *result)
{
    dt_representation_t r = {0,};

    if (!result) {
        return DT_INVALID_ARGUMENT;
    }

    r.year = year;
    r.month = month;
    r.day = day;
    r.hour = hour;
    r.minute = minute;
    r.second = second;
    r.nano_second = nano_second;

    if (!dt_validate_representation(&r)) {
        return DT_INVALID_ARGUMENT;
    }

    *result = r;
    return DT_OK;
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
    dt_status_t status = DT_UNKNOWN_ERROR;

    if (!representation || !tm) {
        return DT_INVALID_ARGUMENT;
    }

    tm->tm_year = representation->year - 1900;
    tm->tm_mon = representation->month - 1;
    tm->tm_mday = representation->day;
    tm->tm_hour = representation->hour;
    tm->tm_min = representation->minute;
    tm->tm_sec = representation->second;
    tm->tm_isdst = -1;
    if (dt_validate_representation(representation) == DT_TRUE) {
        // Setting day of week/year if valid representation has been provided
        status = dt_representation_day_of_week(representation, &dow);
        if (status != DT_OK) {
            return status;
        }
        tm->tm_wday = dow - 1;
        status = dt_representation_day_of_year(representation, &doy);
        if (status != DT_OK) {
            return status;
        }
        tm->tm_yday = doy - 1;
    }
    return DT_OK;
}

dt_status_t dt_tm_to_representation(const struct tm *tm, long nano_second, dt_representation_t *representation)
{
    if (!tm || !representation) {
        return DT_INVALID_ARGUMENT;
    }
    memset(representation, 0, sizeof(dt_representation_t));
    representation->year = 1900 + tm->tm_year;
    representation->month = tm->tm_mon + 1;
    representation->day = tm->tm_mday;
    representation->hour = tm->tm_hour;
    representation->minute = tm->tm_min;
    representation->second = tm->tm_sec;
    representation->nano_second = nano_second;
    return DT_OK;
}

struct tm *localtime_tz(const time_t *time, const char *tz_name, struct tm *result) {
    dt_status_t status = DT_UNKNOWN_ERROR;
    dt_timestamp_t t = {0};
    dt_representation_t rep = {0};
    dt_timezone_t tz = {0,};

    if (!time || !result || !tz_name) {
        return NULL;
    }

    if (*time == -1) {
        return NULL;
    }

    if (dt_timezone_lookup(tz_name, &tz) != DT_OK) {
        return NULL;
    }

    status = dt_posix_time_to_timestamp(*time, 0, &t);
    if (status != DT_OK) {
        dt_timezone_cleanup(&tz);
        return NULL;
    }

    status = dt_timestamp_to_representation(&t, &tz, &rep);
    dt_timezone_cleanup(&tz);
    if (status != DT_OK) {
        return NULL;
    }

    status = dt_representation_to_tm(&rep, result);
    if (status != DT_OK) {
        return NULL;
    }

    return result;
}

time_t mktime_tz(const struct tm *tm, const char *tz_name)
{
    dt_status_t status = DT_UNKNOWN_ERROR;
    dt_timestamp_t t = {0};
    dt_representation_t rep = {0};
    dt_timezone_t tz = {0,};
    time_t result = DT_INVALID_POSIX_TIME;
    unsigned long nano = 0;

    if (!tm || !result || !tz_name) {
        return DT_INVALID_POSIX_TIME;
    }
    if (dt_timezone_lookup(tz_name, &tz) != DT_OK) {
        return DT_INVALID_POSIX_TIME;
    }

    status = dt_tm_to_representation(tm, 0, &rep);
    if (status != DT_OK || dt_validate_representation(&rep) != DT_TRUE) {
        dt_timezone_cleanup(&tz);
        return DT_INVALID_POSIX_TIME;
    }

    status = dt_representation_to_timestamp(&rep, &tz, &t, NULL);
    dt_timezone_cleanup(&tz);
    if (status != DT_OK) {
        return DT_INVALID_POSIX_TIME;
    }

    status = dt_timestamp_to_posix_time(&t, &result, &nano);
    if (status != DT_OK) {
        return DT_INVALID_POSIX_TIME;
    }


    return result;
}

dt_status_t dt_timestamp_to_posix_time(const dt_timestamp_t *timestamp, time_t *time, unsigned long *nano_second)
{
    if (dt_validate_timestamp(timestamp) != DT_TRUE || !time) {
        return DT_INVALID_ARGUMENT;
    }

    *time = timestamp->second;
    *nano_second = timestamp->nano_second;
    return DT_OK;
}

dt_bool_t dt_validate_interval(const dt_interval_t *test)
{
    if (test == NULL) {
        return DT_FALSE;
    }
    if (test->nano_seconds > MAX_NANOSECONDS) {
        return DT_FALSE;
    }

    return DT_TRUE;
}

//! Parses string for fractional seconds format specifier which is "%f" or "%<N>f", where <N> is number from 1 to 9
/*!
 * \param str String to parse
 * \param format_length Format length [OUT]
 * \param precision Precision [OUT]
 * \return Format placeholder start position or -1 if no fractional seconds format encountered
 */
static int dt_parse_fractional_seconds_format(const char *str, size_t *format_length, size_t *precision)
{
    typedef enum {
        DT_FORMAT_PARSER_INITIAL_STATE,
        DT_FORMAT_PARSER_PERCENT_FOUND,
        DT_FORMAT_PARSER_DIGIT_FOUND
    } dt_format_parser_state;

    dt_format_parser_state state = DT_FORMAT_PARSER_INITIAL_STATE;
    size_t pos = 0;
    size_t format_start_pos = 0;
    size_t parsed_precision = 0;

    while (str[pos] != '\0') {
        switch (state) {
            case DT_FORMAT_PARSER_INITIAL_STATE:
                if (str[pos] == '%') {
                    format_start_pos = pos;
                    parsed_precision = 0;
                    state = DT_FORMAT_PARSER_PERCENT_FOUND;
                }
                break;
            case DT_FORMAT_PARSER_PERCENT_FOUND:
                if (str[pos] == 'f') {
                    *format_length = 2;
                    *precision = parsed_precision;
                    return format_start_pos;
                } else if (isdigit(str[pos])) {
                    parsed_precision = str[pos] - '0';
                    state = DT_FORMAT_PARSER_DIGIT_FOUND;
                } else {
                    state = DT_FORMAT_PARSER_INITIAL_STATE;
                }
                break;
            case DT_FORMAT_PARSER_DIGIT_FOUND:
                if (str[pos] == 'f') {
                    *format_length = 3;
                    *precision = parsed_precision;
                    return format_start_pos;
                } else {
                    state = DT_FORMAT_PARSER_INITIAL_STATE;
                }
                break;
            default:
                // Invalid parser state
                return -1;
        }
        ++pos;
    }
    return -1;
}

dt_status_t dt_to_string(const dt_representation_t *representation, const char *fmt,
                         char *str_buffer, size_t str_buffer_size)
{
    struct tm tm = {0};
    dt_status_t status = DT_UNKNOWN_ERROR;
    size_t fmt_len = 0;
    char fmt_buffer[255] = {0};
    int fractional_seconds_format_pos = 0;
    size_t fmt_start_pos = 0;
    size_t fractional_seconds_format_length = 0;
    size_t fractional_seconds_precision = 0;
    size_t str_buffer_eos_pos = 0;
    size_t characters_appended = 0;
    size_t i = 0;

    if (!representation || !fmt || !str_buffer || str_buffer_size <= 0) {
        return DT_INVALID_ARGUMENT;
    }

    // Checking for too long format string
    fmt_len = strlen(fmt);
    if (fmt_len >= sizeof(fmt_buffer)) {
        return DT_OVERFLOW;
    }

    status = dt_representation_to_tm(representation, &tm);
    if (status != DT_OK) {
        return status;
    }

    while (fmt_start_pos < fmt_len) {
        // Searching for fractional format placeholder
        fractional_seconds_format_pos = dt_parse_fractional_seconds_format(fmt + fmt_start_pos,
                                                                           &fractional_seconds_format_length, &fractional_seconds_precision);
        if (fractional_seconds_format_pos < 0) {
            // No fractional seconds placeholder found -> serializing date-time value against the rest of the format string
            characters_appended = strftime(str_buffer + str_buffer_eos_pos, str_buffer_size - str_buffer_eos_pos,
                                           fmt + fmt_start_pos, &tm);

            if ((characters_appended == 0) && (fmt_len > fmt_start_pos) && (fmt_len - fmt_start_pos < str_buffer_size - str_buffer_eos_pos)) {
                // if format string in wrong format, windows std library's function strftime return 0
                str_buffer[str_buffer_eos_pos] = fmt[fmt_start_pos];
                str_buffer_eos_pos++;
                fmt_start_pos++;
                str_buffer[str_buffer_eos_pos] = '\0';
                continue;
            }
            if (characters_appended <= 0) {
                return DT_OVERFLOW;
            }
            str_buffer_eos_pos += characters_appended;
            break;
        } else {
            if (fractional_seconds_format_pos > 0) {
                // Extracting leading data to the buffer and serializing date-time value according this format
                memcpy(fmt_buffer, fmt + fmt_start_pos, fractional_seconds_format_pos);
                fmt_buffer[fractional_seconds_format_pos] = '\0';
                characters_appended = strftime(str_buffer + str_buffer_eos_pos, str_buffer_size - str_buffer_eos_pos,
                                               fmt_buffer, &tm);
                if (characters_appended <= 0) {
                    return DT_OVERFLOW;
                }
                str_buffer_eos_pos += characters_appended;
            }
            fmt_start_pos += (fractional_seconds_format_pos + fractional_seconds_format_length);

            // Serializing fractional seconds
            if (fractional_seconds_precision == 0) {
                fractional_seconds_precision = 9;
            }
            for (i = 0; i < fractional_seconds_precision; ++i) {
                char cur_digit = '\0';
                // Checking for free space in result buffer
                if (str_buffer_eos_pos >= str_buffer_size - 1) {
                    return DT_OVERFLOW;
                }

                cur_digit = (int) floor(representation->nano_second / pow(10, 8 - i)) % 10 + '0';
                str_buffer[str_buffer_eos_pos] = cur_digit;
                ++str_buffer_eos_pos;
            }
            str_buffer[str_buffer_eos_pos] = '\0';
        }
    }
    return DT_OK;
}

dt_status_t dt_from_string(const char *str, const char *fmt, dt_representation_t *representation)
{
    struct tm tm = {0};
    dt_status_t status = DT_UNKNOWN_ERROR;
    size_t fmt_len = 0;
    const char *str_ptr = str;
    char fmt_buffer[255] = {0};
    int fractional_seconds_format_pos = 0;
    size_t fmt_start_pos = 0;
    size_t fractional_seconds_format_length = 0;
    size_t fractional_seconds_precision = 0;
    size_t fractional_digits_parsed = 0;
    unsigned long nano_second = 0;

    if (!representation || !str || !fmt) {
        return DT_INVALID_ARGUMENT;
    }

    // Checking for too long format string
    fmt_len = strlen(fmt);
    if (fmt_len >= sizeof(fmt_buffer)) {
        return DT_OVERFLOW;
    }

    while (1) {

        if (fmt_start_pos < fmt_len) {
            // There is some data in format string
            if (*str_ptr == '\0') {
                // No data to parse
                return DT_INVALID_ARGUMENT;
            }
        } else {
            // Format string has been expired
            if (*str_ptr != '\0') {
                // There is some extra data to parse
                return DT_INVALID_ARGUMENT;
            } else {
                // No data to parse
                break;
            }
        }

        // Searching for fractional format placeholder
        fractional_seconds_format_pos = dt_parse_fractional_seconds_format(fmt + fmt_start_pos,
                                                                           &fractional_seconds_format_length, &fractional_seconds_precision);

        if (fractional_seconds_format_pos < 0) {
            // No fractional seconds placeholder found -> parsing the rest of the string against the rest of format
            str_ptr = strptime(str_ptr, fmt + fmt_start_pos, &tm);   // FIXME: Warning on UNIX platform!
            if (str_ptr == NULL) {
                // Parsing error occured
                return DT_INVALID_ARGUMENT;
            }
            if (*str_ptr != '\0') {
                // There is some extra data to parse
                return DT_INVALID_ARGUMENT;
            }
            break;
        } else {
            if (fractional_seconds_format_pos > 0) {
                // Extracting leading format data to the buffer and parsing date-time value according this format
                memcpy(fmt_buffer, fmt + fmt_start_pos, fractional_seconds_format_pos);
                fmt_buffer[fractional_seconds_format_pos] = '\0';
                str_ptr = strptime(str_ptr, fmt_buffer, &tm);   // FIXME: Warning on UNIX platform!
                if (str_ptr == NULL) {
                    // Parsing error occured
                    return DT_INVALID_ARGUMENT;
                }
            }
            fmt_start_pos += (fractional_seconds_format_pos + fractional_seconds_format_length);

            // Parsing fractional seconds
            if (fractional_seconds_precision == 0) {
                fractional_seconds_precision = 9;
            }
            nano_second = 0;
            fractional_digits_parsed = 0;
            while (fractional_digits_parsed < fractional_seconds_precision) {
                if (!isdigit(*str_ptr)) {
                    if (fractional_digits_parsed == 0) {
                        // Parsing error occured
                        return DT_INVALID_ARGUMENT;
                    }
                    break;
                }
                nano_second *= 10UL;
                nano_second += (unsigned long) * str_ptr - '0';
                ++str_ptr;
                ++fractional_digits_parsed;
            }
            nano_second *= (unsigned long)pow(10, 9 - fractional_digits_parsed);
        }
    }

    status = dt_tm_to_representation(&tm, nano_second, representation);
    if (status != DT_OK) {
        return status;
    }

    return DT_OK;
}
