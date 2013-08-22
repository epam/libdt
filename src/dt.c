#include <dt.h>
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

static const char *no_error_message = "<No error>";
static const char *invalid_argument_error_message = "Invalid argument";
static const char *timezone_not_found_error_message = "Timezone not found";
static const char *system_call_error_message = "System call error";
static const char *unknown_error_message = "Unknown error";
static const char *invalid_status_error_message = "<Invalid result status>";

const int month_days[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

dt_bool_t dt_is_leap_year(int year)
{
	if (year < 1582) {
                // No leap year in Julian calendar
		return (abs(year) % 4 == 0);
	} else {
		return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
	}
}

const char * dt_strerror(dt_status_t status)
{
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
                default:
                        return invalid_status_error_message;
        }
}

dt_bool_t dt_validate_representation(int year, int month, int day, int hour, int minute, int second, long nano_second)
{
        // Simple checking for invalid values
        if (year == 0 || month < 1 || month > 12 || day < 1 || hour < 0 || hour > 24 || minute < 0 || minute > 59 || second < 0 ||
                        second > 59 || nano_second < 0L || nano_second > 999999999L) {
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

dt_status_t dt_compare_timestamps(const dt_timestamp_t *lhs, const dt_timestamp_t *rhs, int *result)
{
        if (!lhs || !rhs || !result) {
                return DT_INVALID_ARGUMENT;
        }
        if (lhs->second > rhs->second) {
                *result = 1;
                return DT_OK;
        }
        if (lhs->second < rhs->second) {
                *result = -1;
                return DT_OK;
        }
        if (lhs->nano_second > rhs->nano_second) {
                *result = 1;
                return DT_OK;
        }
        if (lhs->nano_second < rhs->nano_second) {
                *result = -1;
                return DT_OK;
        }
        *result = 0;
        return DT_OK;
}

dt_status_t dt_offset_to(const dt_timestamp_t *lhs, const dt_timestamp_t *rhs, dt_offset_t *result)
{
        if (!lhs || !rhs || !result) {
                return DT_INVALID_ARGUMENT;
        }
        int cr;
        dt_status_t s = dt_compare_timestamps(lhs, rhs, &cr);
        if (s != DT_OK) {
                return s;
        }
        if (cr == 0) {
                // Equal timestamps case
                result->duration.seconds = 0L;
                result->duration.nano_seconds = 0L;
                result->is_forward = DT_TRUE;
                return DT_OK;
        }
        const dt_timestamp_t *lower_timestamp = cr < 0 ? lhs : rhs;
        const dt_timestamp_t *higher_timestamp = cr < 0 ? rhs : lhs;
        dt_bool_t is_forward = cr < 0 ? DT_TRUE : DT_FALSE;
        unsigned long seconds = higher_timestamp->second - lower_timestamp->second;
        long nano_seconds = (long) higher_timestamp->nano_second - (long) lower_timestamp->nano_second;
        if (nano_seconds < 0L) {
                seconds -= 1L;
                nano_seconds += 1000000000L;
        }
        result->duration.seconds = seconds;
        result->duration.nano_seconds = nano_seconds;
        result->is_forward = is_forward;
        return DT_OK;
}

dt_status_t dt_apply_offset(const dt_timestamp_t *lhs, const dt_offset_t *rhs, dt_timestamp_t *result)
{
        if (!lhs || !rhs || !result) {
                return DT_INVALID_ARGUMENT;
        }
        long second;
        long nano_second;
        if (rhs->is_forward) {
                second = lhs->second + rhs->duration.seconds;
                nano_second = (long) lhs->nano_second + (long) rhs->duration.nano_seconds;
                if (nano_second > 999999999L) {
                        second += 1L;
                        nano_second -= 1000000000L;
                }
        } else {
                second = lhs->second - rhs->duration.seconds;
                nano_second = (long) lhs->nano_second - (long) rhs->duration.nano_seconds;
                if (nano_second < 0L) {
                        second -= 1L;
                        nano_second += 1000000000L;
                }
        }
        result->second = second;
        result->nano_second = nano_second;
        return DT_OK;
}

dt_status_t dt_init_interval(unsigned long seconds, unsigned long nano_seconds, dt_interval_t *result)
{
        if (!result) {
                return DT_INVALID_ARGUMENT;
        }
        result->seconds = seconds + nano_seconds / 1000000000L;
        result->nano_seconds = nano_seconds % 1000000000L;
        return DT_OK;
}

dt_status_t dt_compare_intervals(const dt_interval_t *lhs, const dt_interval_t *rhs, int *result)
{
        if (!lhs || !rhs || !result) {
                return DT_INVALID_ARGUMENT;
        }
        if (lhs->seconds > rhs->seconds) {
                *result = 1;
                return DT_OK;
        }
        if (lhs->seconds < rhs->seconds) {
                *result = -1;
                return DT_OK;
        }
        if (lhs->nano_seconds > rhs->nano_seconds) {
                *result = 1;
                return DT_OK;
        }
        if (lhs->nano_seconds < rhs->nano_seconds) {
                *result = -1;
                return DT_OK;
        }
        *result = 0;
        return 0;
}

dt_status_t dt_sum_intervals(const dt_interval_t *lhs, const dt_interval_t *rhs, dt_interval_t *result)
{
        if (!lhs || !rhs || !result) {
                return DT_INVALID_ARGUMENT;
        }
        unsigned long seconds = lhs->seconds + rhs->seconds + (lhs->nano_seconds + rhs->nano_seconds) / 1000000000L;
        unsigned long nano_seconds = (lhs->nano_seconds + rhs->nano_seconds) % 1000000000L;
        result->seconds = seconds;
        result->nano_seconds = nano_seconds;
        return DT_OK;
}

dt_status_t dt_sub_intervals(const dt_interval_t *lhs, const dt_interval_t *rhs, dt_interval_t *result)
{
        if (!lhs || !rhs || !result) {
                return DT_INVALID_ARGUMENT;
        }
        int cr;
        dt_status_t s = dt_compare_intervals(lhs, rhs, &cr);
        if (s != DT_OK) {
                return s;
        }
        if (cr <= 0) {
                result->seconds = 0L;
                result->nano_seconds = 0L;
                return DT_OK;
        }
        unsigned long seconds = lhs->seconds - rhs->seconds - (lhs->nano_seconds < rhs->nano_seconds ? 1L : 0L);
        unsigned long nano_seconds = lhs->nano_seconds < rhs->nano_seconds ?
                1000000000L + lhs->nano_seconds - rhs->nano_seconds :
                lhs->nano_seconds - rhs->nano_seconds;
        result->seconds = seconds;
        result->nano_seconds = nano_seconds;
        return DT_OK;
}

dt_status_t dt_mul_interval(const dt_interval_t *lhs, double rhs, dt_interval_t *result)
{
        if (!lhs || !result) {
                return DT_INVALID_ARGUMENT;
        }
        long double v = (long double) lhs->seconds + (long double) lhs->nano_seconds / 1000000000L;
        long double rv = v * rhs;
        result->seconds = floorl(rv);
        result->nano_seconds = floorl(fmodl(rv, 1.0) * 1000000000L);
        return DT_OK;
}

dt_status_t dt_init_representation(int year, int month, int day, int hour, int minute, int second, int nano_second,
                dt_representation_t *result)
{
        if (!result || !dt_validate_representation(year, month, day, hour, minute, second, nano_second)) {
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

dt_status_t dt_representation_day_of_week(const dt_representation_t *representation, int *dow)
{
        // TODO: Julian calendar support - now only Gregorian is supported
        if (!representation || !dow) {
                return DT_INVALID_ARGUMENT;
        }
        // See http://en.wikipedia.org/wiki/Zeller%27s_congruence
        int year = (representation->month == 1 || representation->month == 2) ? representation->year - 1 : representation->year;
        int month = (representation->month == 1 || representation->month == 2) ? representation->month + 12 : representation->month;
        *dow = (representation->day + (month + 1) * 26 / 10 + year + year / 4 + 6 * year / 100 + year / 400) % 7;
        *dow = *dow == 0 ? 7 : *dow;
        return DT_OK;
}

dt_status_t dt_representation_day_of_year(const dt_representation_t *representation, int *doy)
{
        // TODO: Julian calendar support - now only Gregorian is supported
        if (!representation || !doy) {
                return DT_INVALID_ARGUMENT;
        }
        *doy = 0;
        int i;
        for (i = 1; i < representation->month; ++i) {
                *doy += (i == 2 && dt_is_leap_year(representation->year)) ? 29 : month_days[i];
        }
        *doy += representation->day;
        return DT_OK;
}

dt_status_t dt_representation_to_tm(const dt_representation_t *representation, struct tm *tm)
{
        if (!representation || !tm) {
                return DT_INVALID_ARGUMENT;
        }
        int dow;
        dt_status_t s = dt_representation_day_of_week(representation, &dow);
        if (s != DT_OK) {
                return s;
        }
        int doy;
        s = dt_representation_day_of_year(representation, &doy);
        if (s != DT_OK) {
                return s;
        }
        tm->tm_year = representation->year;
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

dt_status_t dt_representation_to_tm_private(const dt_representation_t *representation, struct tm *tm)
{
        if (!representation || !tm) {
                return DT_INVALID_ARGUMENT;
        }
        tm->tm_year = representation->year;
        tm->tm_mon = representation->month - 1;
        tm->tm_mday = representation->day;
        tm->tm_hour = representation->hour;
        tm->tm_min = representation->minute;
        tm->tm_sec = representation->second;
        tm->tm_wday = 0;
        tm->tm_yday = 0;
        tm->tm_isdst = -1;
        return DT_OK;
}

dt_status_t dt_tm_to_representation(const struct tm *tm, long nano_second, dt_representation_t *representation)
{
        if (!tm || !representation) {
                return DT_INVALID_ARGUMENT;
        }
        return dt_init_representation(tm->tm_year, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, nano_second, representation);
}
