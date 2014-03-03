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

#include "dt_case.h"
#include "libdt/dt.h"
#include <time.h>
#include "libdt/dt_posix.h"
#include <limits>
#include <limits.h>
#include <float.h>

#define MOSCOW_WINDOWS_STANDARD_TZ_NAME "Russian Standard Time"
#define MOSCOW_OLSEN_TZ_NAME  "Europe/Moscow"
#define UNREAL_TIMEZONE_NAME "this time zone is unreal, you cannot find it anywhere, so good luck"

#ifdef _WIN32

#define UTC_TZ_NAME "UTC"
#define MOSCOW_TZ_NAME MOSCOW_WINDOWS_STANDARD_TZ_NAME
#define BERLIN_TZ_NAME "W. Europe Standard Time"
#define GMT_MINUS_5_TZ_NAME "GMT-5"
#define GMT_PLUS_5_TZ_NAME "GMT+5"

#include <windows.h>
#define sleep(x) Sleep(x*1000)

#else

#define UTC_TZ_NAME "UTC"
#define MOSCOW_TZ_NAME MOSCOW_OLSEN_TZ_NAME
#define BERLIN_TZ_NAME "Europe/Berlin"
#define GMT_MINUS_5_TZ_NAME "West Asia Standard Time"
#define GMT_PLUS_5_TZ_NAME "SA Pacific Standard Time"

#endif

#define MAX_NANOSECONDS  999999999UL

static const dt_interval_t invalid_interval = { ULONG_MAX,
                                                MAX_NANOSECONDS + 1
                                              };

static const dt_interval_t overflowable_interval = { ULONG_MAX,
                                                     MAX_NANOSECONDS
                                                   };

static const dt_timestamp_t overflowable_timestamp = { LONG_MAX,
                                                       MAX_NANOSECONDS
                                                     };

static const dt_timestamp_t overflowable_timestamp_negative = { LONG_MIN,
                                                                MAX_NANOSECONDS
                                                              };

static const dt_timestamp_t invalid_timestamp = { LONG_MAX,
                                                  MAX_NANOSECONDS + 1
                                                };

static const dt_offset_t invalid_offset = {invalid_interval, DT_FALSE};
static const dt_offset_t overflawable_offset = {overflowable_interval, DT_FALSE};

DtCase::DtCase() :
    ::testing::Test()
{}

TEST_F(DtCase, is_leap)
{
    EXPECT_EQ(dt_is_leap_year(1579), DT_FALSE);
    EXPECT_EQ(dt_is_leap_year(1580), DT_TRUE);
    EXPECT_EQ(dt_is_leap_year(0), DT_FALSE);
    EXPECT_EQ(dt_is_leap_year(-1), DT_FALSE);
    EXPECT_NE(dt_is_leap_year(2013), DT_TRUE);
    EXPECT_EQ(dt_is_leap_year(2012), DT_TRUE);
    EXPECT_NE(dt_is_leap_year(2100), DT_TRUE);
    EXPECT_EQ(dt_is_leap_year(2400), DT_TRUE);
}

TEST_F(DtCase, strerror)
{
    static const char *no_error_message = "<No error>";
    static const char *invalid_argument_error_message = "Invalid argument";
    static const char *timezone_not_found_error_message = "Timezone not found";
    static const char *system_call_error_message = "System call error";
    static const char *no_more_items_error_message = "No more items in collection";
    static const char *overflow_error_message = "Operation caused owerflow";
    static const char *unknown_error_message = "Unknown error";
    static const char *invalid_status_error_message = "<Invalid result status>";

    EXPECT_EQ(strcmp(dt_strerror(DT_OK), no_error_message), 0);
    EXPECT_EQ(strcmp(dt_strerror(DT_INVALID_ARGUMENT), invalid_argument_error_message), 0);
    EXPECT_EQ(strcmp(dt_strerror(DT_TIMEZONE_NOT_FOUND), timezone_not_found_error_message), 0);
    EXPECT_EQ(strcmp(dt_strerror(DT_SYSTEM_CALL_ERROR), system_call_error_message), 0);
    EXPECT_EQ(strcmp(dt_strerror(DT_NO_MORE_ITEMS), no_more_items_error_message), 0);
    EXPECT_EQ(strcmp(dt_strerror(DT_OVERFLOW), overflow_error_message), 0);
    EXPECT_EQ(strcmp(dt_strerror(DT_UNKNOWN_ERROR), unknown_error_message), 0);
    EXPECT_EQ(strcmp(dt_strerror((dt_status_t) 250), invalid_status_error_message), 0);
}

typedef struct {
    dt_representation_t represenatation;
    dt_bool_t expected;
} validate_representation_test_data_t;

static validate_representation_test_data_t validate_representation_test_data[] = {
    {
        {2012, 12, 21, 8, 30, 45, 123456789},
        DT_TRUE
    },
    {
        {2012, 12, 21, 8, 30, 45, -1},
        DT_FALSE
    },
    {
        {2012, 12, 21, 8, 30, 45, 1000000000},
        DT_FALSE
    },
    {
        {2012, 12, 21, 8, 30, -1, 123456789},
        DT_FALSE
    },
    {
        {2012, 12, 21, 8, 30, 60, 123456789},
        DT_FALSE
    },
    {
        {2012, 12, 21, 8, -1, 45, 123456789},
        DT_FALSE
    },

    {
        {2012, 12, 21, 8, 60, 45, 123456789},
        DT_FALSE
    },
    {
        {2012, 12, 21, -1, 30, 45, 123456789},
        DT_FALSE
    },
    {
        {2012, 12, 21, 60, 30, 45, 123456789},
        DT_FALSE
    },
    {
        {2012, 12, 0, 8, 30, 45, 123456789},
        DT_FALSE
    },
    {
        {2012, 12, 32, 8, 30, 45, 123456789},
        DT_FALSE
    },
    {
        {2012, 0, 21, 8, 30, 45, 123456789},
        DT_FALSE
    },
    {
        {2012, 13, 21, 8, 30, 45, 123456789},
        DT_FALSE
    },
    {
        {0, 12, 21, 8, 30, 45, 123456789},
        DT_FALSE
    },
    // Passage from Julian to Gregorian calendar
    {
        {1582, 10, 10, 8, 30, 45, 123456789},
        DT_FALSE
    },
    {
        {1582, 11, 10, 8, 30, 45, 123456789},
        DT_TRUE
    },
    {
        {1582, 11, 10, 8, 30, 45, 123456789},
        DT_TRUE
    },
    {
        {1582, 10, 2, 8, 30, 45, 123456789},
        DT_TRUE
    },

    {
        {1582, 10, 20, 8, 30, 45, 123456789},
        DT_TRUE
    },

};

class DtCaseValidateRepresentation : public ::testing::TestWithParam<validate_representation_test_data_t>
{

};


TEST_P(DtCaseValidateRepresentation, validate_representation)
{
    dt_representation_t r = GetParam().represenatation;
    EXPECT_EQ(dt_validate_representation(NULL), DT_FALSE);
    EXPECT_EQ(dt_validate_representation(&r), GetParam().expected);
}

//INSTANTIATE_TEST_CASE_P(validate_representation_parametrized, DtCaseValidateRepresentation,
//                        ::testing::ValuesIn(validate_representation_test_data));

TEST_F(DtCase, validate_timestamps)
{
    dt_timestamp_t timestamp = invalid_timestamp;
    EXPECT_EQ(dt_validate_timestamp(NULL), DT_FALSE);
    EXPECT_EQ(dt_validate_timestamp(&timestamp), DT_FALSE);
    timestamp = overflowable_timestamp;
    EXPECT_EQ(dt_validate_timestamp(&timestamp), DT_TRUE);
}

TEST_F(DtCase, now_compare_timestamps)
{
    dt_timestamp_t ts_01 = {0,};
    EXPECT_EQ(dt_now(NULL), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_now(&ts_01), DT_OK);
    sleep(1);
    dt_timestamp_t ts_02 = {0,};
    EXPECT_EQ(dt_now(&ts_02), DT_OK);
    dt_compare_result_t cr = DT_EQUALS;
    EXPECT_EQ(dt_compare_timestamps(NULL, &ts_02, &cr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_compare_timestamps(&ts_01, NULL, &cr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_compare_timestamps(&ts_01, &ts_02, NULL), DT_INVALID_ARGUMENT);

    EXPECT_EQ(dt_compare_timestamps(&invalid_timestamp, &ts_01, &cr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_compare_timestamps(&ts_01, &invalid_timestamp, &cr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_compare_timestamps(&invalid_timestamp, &ts_01, &cr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_compare_timestamps(&ts_01, &invalid_timestamp, &cr), DT_INVALID_ARGUMENT);

    EXPECT_EQ(dt_compare_timestamps(&ts_01, &ts_02, &cr), DT_OK);
    EXPECT_EQ(cr, DT_LESSER);
    EXPECT_EQ(dt_compare_timestamps(&ts_02, &ts_01, &cr), DT_OK);
    EXPECT_EQ(cr, DT_GREATER);
    EXPECT_EQ(dt_compare_timestamps(&ts_01, &ts_01, &cr), DT_OK);
    EXPECT_EQ(cr, DT_EQUALS);

    ts_02 = ts_01;
    if (ts_01.nano_second == 0UL) {
        ts_02.nano_second++;
    } else {
        ts_01.nano_second--;
    }
    EXPECT_EQ(dt_compare_timestamps(&ts_01, &ts_02, &cr), DT_OK);
    EXPECT_EQ(cr, DT_LESSER);
    EXPECT_EQ(dt_compare_timestamps(&ts_02, &ts_01, &cr), DT_OK);
    EXPECT_EQ(cr, DT_GREATER);
}


TEST_F(DtCase, offset_between)
{
    dt_timestamp_t ts_01 = {0,};
    dt_timestamp_t ts_02 = {0,};
    dt_offset_t o = {0,};
    dt_timezone_t tz_moscow = {0,};
    dt_representation_t r = {0,};

    //init timestamps
    EXPECT_EQ(dt_timezone_lookup(MOSCOW_TZ_NAME, &tz_moscow), DT_OK);
    EXPECT_EQ(dt_init_representation(2012, 12, 21, 8, 30, 45, 123456789L, &r), DT_OK);
    EXPECT_EQ(dt_representation_to_timestamp(&r, &tz_moscow, &ts_01, NULL), DT_OK);
    EXPECT_EQ(dt_init_representation(2012, 12, 22, 8, 30, 45, 123456889L, &r), DT_OK);
    EXPECT_EQ(dt_representation_to_timestamp(&r, &tz_moscow, &ts_02, NULL), DT_OK);

    //invalid arguments
    EXPECT_EQ(dt_offset_between(NULL, &ts_02, &o), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_offset_between(&ts_01, NULL, &o), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_offset_between(&ts_01, &ts_02, NULL), DT_INVALID_ARGUMENT);

    EXPECT_EQ(dt_offset_between(&invalid_timestamp, &ts_01, &o), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_offset_between(&ts_01, &invalid_timestamp, &o), DT_INVALID_ARGUMENT);

    //buisness
    EXPECT_EQ(dt_offset_between(&ts_01, &ts_01, &o), DT_OK);
    EXPECT_EQ(o.is_forward, DT_TRUE);
    EXPECT_EQ(o.duration.seconds, 0L);
    EXPECT_EQ(o.duration.nano_seconds, 0L);

    EXPECT_EQ(dt_offset_between(&ts_01, &ts_02, &o), DT_OK);
    EXPECT_EQ(o.is_forward, DT_TRUE);
    EXPECT_EQ(o.duration.seconds, DT_SECONDS_PER_DAY);
    EXPECT_EQ(o.duration.nano_seconds, 100L);
    EXPECT_EQ(dt_offset_between(&ts_02, &ts_01, &o), DT_OK);
    EXPECT_EQ(o.is_forward, DT_FALSE);
    EXPECT_EQ(o.duration.seconds, DT_SECONDS_PER_DAY);
    EXPECT_EQ(o.duration.nano_seconds, 100L);

    EXPECT_TRUE(dt_init_representation(2012, 12, 22, 8, 30, 45, 100L, &r) == DT_OK);
    EXPECT_TRUE(dt_representation_to_timestamp(&r, &tz_moscow, &ts_02, NULL) == DT_OK);
    EXPECT_EQ(dt_offset_between(&ts_01, &ts_02, &o), DT_OK);
    EXPECT_EQ(o.is_forward, DT_TRUE);
    EXPECT_EQ(o.duration.seconds, DT_SECONDS_PER_DAY - 1);
    EXPECT_EQ(o.duration.nano_seconds, 876543311L);
    EXPECT_EQ(dt_offset_between(&ts_02, &ts_01, &o), DT_OK);
    EXPECT_EQ(o.is_forward, DT_FALSE);
    EXPECT_EQ(o.duration.seconds, DT_SECONDS_PER_DAY - 1);
    EXPECT_EQ(o.duration.nano_seconds, 876543311L);

    EXPECT_EQ(dt_timezone_cleanup(&tz_moscow), DT_OK);
}

TEST_F(DtCase, validate_offset)
{
    dt_offset_t offset = {0,};
    EXPECT_EQ(dt_validate_offset(NULL), DT_FALSE);
    EXPECT_EQ(dt_validate_offset(&invalid_offset), DT_FALSE);
    EXPECT_EQ(dt_validate_offset(&offset), DT_TRUE);
}

TEST_F(DtCase, apply_offset_overflow)
{
    dt_timestamp_t result = {1L, 1UL};

    // Forward case
    dt_timestamp_t maximum_overflowable_forward_timestamp = {LONG_MAX , 0};
    dt_timestamp_t minimum_overflowable_forward_timestamp = {LONG_MIN + 1, 0};
    dt_timestamp_t maximum_overflowable_forward_timestamp_for_max_nanoseconds = {LONG_MAX - 1, MAX_NANOSECONDS};
    dt_timestamp_t minimum_overflowable_forward_timestamp_for_max_nanoseconds = {LONG_MIN + 1 + 1, MAX_NANOSECONDS};
    dt_offset_t maximum_offset_forward = {{ULONG_MAX, 0}, DT_TRUE};
    dt_offset_t maximum_offset_forward_max_nanoseconds = {{ULONG_MAX, MAX_NANOSECONDS}, DT_TRUE};

    memset(&result, 0, sizeof(result));
    EXPECT_EQ(dt_apply_offset(&maximum_overflowable_forward_timestamp, &maximum_offset_forward, &result), DT_OVERFLOW);
    memset(&result, 0, sizeof(result));
    EXPECT_EQ(dt_apply_offset(&minimum_overflowable_forward_timestamp, &maximum_offset_forward, &result), DT_OVERFLOW);

    memset(&result, 0, sizeof(result));
    EXPECT_EQ(dt_apply_offset(&maximum_overflowable_forward_timestamp_for_max_nanoseconds,
                              &maximum_offset_forward_max_nanoseconds, &result), DT_OVERFLOW);
    memset(&result, 0, sizeof(result));
    EXPECT_EQ(dt_apply_offset(&minimum_overflowable_forward_timestamp_for_max_nanoseconds,
                              &maximum_offset_forward_max_nanoseconds, &result), DT_OVERFLOW);

    // Backward case
    dt_timestamp_t maximum_overflowable_backward_timestamp = {LONG_MAX - 1, 0};
    dt_timestamp_t minimum_overflowable_backward_timestamp = {LONG_MIN, 0};
    dt_timestamp_t maximum_overflowable_backward_timestamp_for_max_nanoseconds = {LONG_MAX - 1 - 1, 0};
    dt_timestamp_t minimum_overflowable_backward_timestamp_for_max_nanoseconds = {LONG_MIN + 1 , 0};
    dt_offset_t maximum_offset_backward = {{ULONG_MAX, 0}, DT_FALSE};
    dt_offset_t minimum_offset_backward = {{1, 0}, DT_FALSE};
    dt_offset_t maximum_offset_backward_max_nanoseconds = {{ULONG_MAX, MAX_NANOSECONDS}, DT_FALSE};
    dt_offset_t minimum_offset_backward_max_nanoseconds = {{1, MAX_NANOSECONDS}, DT_FALSE};

    memset(&result, 0, sizeof(result));
    EXPECT_EQ(dt_apply_offset(&maximum_overflowable_backward_timestamp, &maximum_offset_backward, &result), DT_OVERFLOW);
    memset(&result, 0, sizeof(result));
    EXPECT_EQ(dt_apply_offset(&minimum_overflowable_backward_timestamp, &minimum_offset_backward, &result), DT_OVERFLOW);

    memset(&result, 0, sizeof(result));
    EXPECT_EQ(dt_apply_offset(&maximum_overflowable_backward_timestamp_for_max_nanoseconds,
                              &maximum_offset_backward_max_nanoseconds, &result), DT_OVERFLOW);
    memset(&result, 0, sizeof(result));
    EXPECT_EQ(dt_apply_offset(&minimum_overflowable_backward_timestamp_for_max_nanoseconds,
                              &minimum_offset_backward_max_nanoseconds, &result), DT_OVERFLOW);
}

TEST_F(DtCase, apply_offset)
{
    dt_timestamp_t ts_01 = {1, 1};
    dt_timestamp_t ts_02 = {1, 1};
    dt_offset_t o = {{1, 0}, DT_TRUE};

    dt_timezone_t tz_moscow = {0,};

    EXPECT_EQ(dt_timezone_lookup(MOSCOW_TZ_NAME, &tz_moscow), DT_OK);

    EXPECT_EQ(dt_apply_offset(NULL, &o, &ts_02), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_apply_offset(&ts_01, NULL, &ts_02), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_apply_offset(&ts_01, &o, NULL), DT_INVALID_ARGUMENT);

    EXPECT_EQ(dt_apply_offset(&ts_01, &invalid_offset, &ts_02), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_apply_offset(&invalid_timestamp, &o, &ts_02), DT_INVALID_ARGUMENT);

    dt_representation_t r = {0,};
    EXPECT_TRUE(dt_init_representation(2012, 12, 21, 8, 30, 45, 123456789UL, &r) == DT_OK);
    EXPECT_TRUE(dt_representation_to_timestamp(&r, &tz_moscow, &ts_01, NULL) == DT_OK);
    dt_representation_t rr = {0,};

    EXPECT_TRUE(dt_init_interval(DT_SECONDS_PER_DAY, 100UL, &o.duration) == DT_OK);
    o.is_forward = DT_TRUE;

    EXPECT_EQ(dt_apply_offset(&ts_01, &o, &ts_02), DT_OK);
    EXPECT_TRUE(dt_timestamp_to_representation(&ts_02, &tz_moscow, &rr) == DT_OK);
    EXPECT_EQ(rr.year, 2012);
    EXPECT_EQ(rr.month, 12);
    EXPECT_EQ(rr.day, 22);
    EXPECT_EQ(rr.hour, 8);
    EXPECT_EQ(rr.minute, 30);
    EXPECT_EQ(rr.second, 45);
    EXPECT_EQ(rr.nano_second, 123456889L);

    EXPECT_TRUE(dt_init_interval(DT_SECONDS_PER_DAY, 900000000UL, &o.duration) == DT_OK);
    o.is_forward = DT_TRUE;
    EXPECT_EQ(dt_apply_offset(&ts_01, &o, &ts_02), DT_OK);
    EXPECT_TRUE(dt_timestamp_to_representation(&ts_02, &tz_moscow, &rr) == DT_OK);
    EXPECT_EQ(rr.year, 2012);
    EXPECT_EQ(rr.month, 12);
    EXPECT_EQ(rr.day, 22);
    EXPECT_EQ(rr.hour, 8);
    EXPECT_EQ(rr.minute, 30);
    EXPECT_EQ(rr.second, 46);
    EXPECT_EQ(rr.nano_second, 23456789L);

    EXPECT_TRUE(dt_init_interval(DT_SECONDS_PER_DAY, 100UL, &o.duration) == DT_OK);
    o.is_forward = DT_FALSE;
    EXPECT_EQ(dt_apply_offset(&ts_01, &o, &ts_02), DT_OK);
    EXPECT_TRUE(dt_timestamp_to_representation(&ts_02, &tz_moscow, &rr) == DT_OK);
    EXPECT_EQ(rr.year, 2012);
    EXPECT_EQ(rr.month, 12);
    EXPECT_EQ(rr.day, 20);
    EXPECT_EQ(rr.hour, 8);
    EXPECT_EQ(rr.minute, 30);
    EXPECT_EQ(rr.second, 45);
    EXPECT_EQ(rr.nano_second, 123456689L);

    EXPECT_TRUE(dt_init_interval(DT_SECONDS_PER_DAY, 900000000UL, &o.duration) == DT_OK);
    o.is_forward = DT_FALSE;
    EXPECT_EQ(dt_apply_offset(&ts_01, &o, &ts_02), DT_OK);
    EXPECT_TRUE(dt_timestamp_to_representation(&ts_02, &tz_moscow, &rr) == DT_OK);
    EXPECT_EQ(rr.year, 2012);
    EXPECT_EQ(rr.month, 12);
    EXPECT_EQ(rr.day, 20);
    EXPECT_EQ(rr.hour, 8);
    EXPECT_EQ(rr.minute, 30);
    EXPECT_EQ(rr.second, 44);
    EXPECT_EQ(rr.nano_second, 223456789UL);

    EXPECT_EQ(dt_timezone_cleanup(&tz_moscow), DT_OK);
}

TEST_F(DtCase, posix_time_to_and_from_timestamp)
{
    time_t rtm = DT_INVALID_POSIX_TIME;
    time_t now = time(NULL);
    dt_timestamp_t ts = {0,};
    unsigned long nano_second = 0;
    dt_offset_t o = {0,};

    EXPECT_EQ(dt_posix_time_to_timestamp(-1, 123456789L, &ts), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_posix_time_to_timestamp(0, 123456789L, NULL), DT_INVALID_ARGUMENT);

    EXPECT_EQ(dt_timestamp_to_posix_time(NULL, &rtm, NULL), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_timestamp_to_posix_time(&ts, NULL, NULL), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_timestamp_to_posix_time(&invalid_timestamp, &rtm, &nano_second), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_timestamp_to_posix_time(&invalid_timestamp, &rtm, &nano_second), DT_INVALID_ARGUMENT);

    EXPECT_EQ(dt_posix_time_to_timestamp(0, 123456789L, &ts), DT_OK);

    EXPECT_TRUE(dt_init_interval(DT_SECONDS_PER_DAY, 0, &o.duration) == DT_OK);
    o.is_forward = DT_FALSE;
    dt_timestamp_t nts = {0,};
    EXPECT_TRUE(dt_apply_offset(&ts, &o, &nts) == DT_OK);
    EXPECT_EQ(dt_timestamp_to_posix_time(&ts, &rtm, &nano_second), DT_OK);
    EXPECT_EQ(rtm, 0);
    EXPECT_EQ(nano_second, 123456789L);

    EXPECT_EQ(dt_posix_time_to_timestamp(now, 123456789L, &ts), DT_OK);
    EXPECT_EQ(dt_timestamp_to_posix_time(&ts, &rtm, &nano_second), DT_OK);
    EXPECT_EQ(rtm, now);
    EXPECT_EQ(nano_second, 123456789L);
}

TEST_F(DtCase, validate_interval)
{
    dt_interval_t interval = invalid_interval;


    EXPECT_EQ(dt_validate_interval(NULL), DT_FALSE);
    EXPECT_EQ(dt_validate_interval(&interval), DT_FALSE);

    interval.nano_seconds = MAX_NANOSECONDS;
    EXPECT_EQ(dt_validate_interval(&interval), DT_TRUE);

}

TEST_F(DtCase, init_interval)
{
    dt_interval_t i = {0,};
    EXPECT_EQ(dt_init_interval(1, 1000100500L, NULL), DT_INVALID_ARGUMENT);

    EXPECT_EQ(dt_init_interval(1, 100500L, &i), DT_OK);
    EXPECT_EQ(i.seconds, 1);
    EXPECT_EQ(i.nano_seconds, 100500L);

    EXPECT_EQ(dt_init_interval(1, 1000100500L, &i), DT_OK);
    EXPECT_EQ(i.seconds, 2);
    EXPECT_EQ(i.nano_seconds, 100500L);

    EXPECT_EQ(dt_init_interval(ULONG_MAX, ULONG_MAX, &i), DT_OVERFLOW);
    EXPECT_EQ(dt_init_interval(ULONG_MAX, MAX_NANOSECONDS + 1 , &i), DT_OVERFLOW);
    EXPECT_EQ(dt_init_interval(ULONG_MAX, 123UL, &i), DT_OK);
}

TEST_F(DtCase, compare_interval)
{
    dt_interval_t i_01 = {0,};
    EXPECT_TRUE(dt_init_interval(1L, 123456789L, &i_01) == DT_OK);
    dt_interval_t i_02 = {0,};
    EXPECT_TRUE(dt_init_interval(2L, 123456789L, &i_02) == DT_OK);
    dt_compare_result_t cr = DT_EQUALS;

    EXPECT_EQ(dt_compare_intervals(NULL, &i_02, &cr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_compare_intervals(&i_01, NULL, &cr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_compare_intervals(&i_01, &i_02, NULL), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_compare_intervals(&invalid_interval, &i_02, &cr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_compare_intervals(&i_01, &invalid_interval, &cr), DT_INVALID_ARGUMENT);


    EXPECT_EQ(dt_compare_intervals(&i_01, &i_02, &cr), DT_OK);
    EXPECT_LT(cr, 0L);
    EXPECT_TRUE(dt_init_interval(0L, 123456789L, &i_02) == DT_OK);
    EXPECT_EQ(dt_compare_intervals(&i_01, &i_02, &cr), DT_OK);
    EXPECT_GT(cr, 0L);
    EXPECT_TRUE(dt_init_interval(1L, 123456790L, &i_02) == DT_OK);
    EXPECT_EQ(dt_compare_intervals(&i_01, &i_02, &cr), DT_OK);
    EXPECT_LT(cr, 0L);
    EXPECT_TRUE(dt_init_interval(1L, 123456788L, &i_02) == DT_OK);
    EXPECT_EQ(dt_compare_intervals(&i_01, &i_02, &cr), DT_OK);
    EXPECT_GT(cr, 0L);
    EXPECT_EQ(dt_compare_intervals(&i_01, &i_01, &cr), DT_OK);
    EXPECT_EQ(cr, 0L);
}

TEST_F(DtCase, sum_intervals)
{
    dt_interval_t i_01 = {0,};
    EXPECT_TRUE(dt_init_interval(1L, 123456789L, &i_01) == DT_OK);
    dt_interval_t i_02 = {0,};
    EXPECT_TRUE(dt_init_interval(2L, 123456789L, &i_02) == DT_OK);

    dt_interval_t ri = {0,};
    EXPECT_EQ(dt_sum_intervals(NULL, &i_02, &ri), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_sum_intervals(&i_01, NULL, &ri), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_sum_intervals(&i_01, &i_02, NULL), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_sum_intervals(&invalid_interval, &i_02, &ri), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_sum_intervals(&i_01, &invalid_interval, &ri), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_sum_intervals(&overflowable_interval, &overflowable_interval, &ri), DT_OVERFLOW);

    EXPECT_EQ(dt_sum_intervals(&i_01, &i_02, &ri), DT_OK);
    EXPECT_EQ(ri.seconds, 3L);
    EXPECT_EQ(ri.nano_seconds, 246913578L);
    EXPECT_TRUE(dt_init_interval(2L, 900000000L, &i_02) == DT_OK);
    EXPECT_EQ(dt_sum_intervals(&i_01, &i_02, &ri), DT_OK);
    EXPECT_EQ(ri.seconds, 4L);
    EXPECT_EQ(ri.nano_seconds, 23456789L);
}

TEST_F(DtCase, sub_intervals)
{
    dt_interval_t i_01 = {0,};
    EXPECT_TRUE(dt_init_interval(2L, 123456789L, &i_01) == DT_OK);
    dt_interval_t i_02 = {0,};
    EXPECT_TRUE(dt_init_interval(3L, 123456789L, &i_02) == DT_OK);
    dt_interval_t ri = {0,};

    EXPECT_EQ(dt_sub_intervals(NULL, &i_02, &ri), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_sub_intervals(&i_01, NULL, &ri), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_sub_intervals(&i_01, &i_02, NULL), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_sub_intervals(&invalid_interval, &i_02, &ri), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_sub_intervals(&i_01, &invalid_interval, &ri), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_sub_intervals(&i_01, &overflowable_interval, &ri), DT_OVERFLOW);

    EXPECT_EQ(dt_sub_intervals(&i_01, &i_02, &ri), DT_OVERFLOW);
    EXPECT_TRUE(dt_init_interval(1L, 900000000L, &i_02) == DT_OK);
    EXPECT_EQ(dt_sub_intervals(&i_01, &i_02, &ri), DT_OK);
    EXPECT_EQ(ri.seconds, 0L);
    EXPECT_EQ(ri.nano_seconds, 223456789L);

    EXPECT_TRUE(dt_init_interval(3L, 200000000L, &i_01) == DT_OK);
    EXPECT_TRUE(dt_init_interval(1L, 100000000L, &i_02) == DT_OK);
    EXPECT_EQ(dt_sub_intervals(&i_01, &i_02, &ri), DT_OK);
    EXPECT_EQ(ri.seconds, 2L);
    EXPECT_EQ(ri.nano_seconds, 100000000L);

    EXPECT_TRUE(dt_init_interval(3L, 100000000L, &i_01) == DT_OK);
    EXPECT_TRUE(dt_init_interval(1L, 200000000L, &i_02) == DT_OK);
    EXPECT_EQ(dt_sub_intervals(&i_01, &i_02, &ri), DT_OK);
    EXPECT_EQ(ri.seconds, 1L);
    EXPECT_EQ(ri.nano_seconds, 900000000L);
}

TEST_F(DtCase, miltiply_interval)
{
    dt_interval_t i = {0,};
    dt_interval_t ri = {0,};
    dt_interval_t dt_mul_overflawable_interval = overflowable_interval;
    dt_mul_overflawable_interval.seconds = invalid_interval.seconds;

    EXPECT_EQ(dt_mul_interval(&i, DBL_MAX, &ri), DT_OK);
    EXPECT_EQ(ri.seconds, 0);
    EXPECT_EQ(ri.nano_seconds, 0);

    EXPECT_TRUE(dt_init_interval(3UL, 500000000UL, &i) == DT_OK);
    EXPECT_EQ(i.seconds, 3UL);
    EXPECT_EQ(i.nano_seconds, 500000000UL);

    EXPECT_EQ(dt_mul_interval(NULL, 2.5, &ri), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_mul_interval(&i, 2.5, NULL), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_mul_interval(&invalid_interval, 2.5, &ri), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_mul_interval(&i, DBL_MAX, &ri), DT_OVERFLOW);

    EXPECT_EQ(dt_mul_interval(&overflowable_interval, MAX_NANOSECONDS, &ri), DT_OVERFLOW);

    EXPECT_EQ(dt_mul_interval(&i, 2.5, &ri), DT_OK);
    EXPECT_EQ(ri.seconds, 8UL);
    EXPECT_EQ(ri.nano_seconds, 750000000UL);
    EXPECT_EQ(dt_mul_interval(&i, 0.75, &ri), DT_OK);
    EXPECT_EQ(ri.seconds, 2UL);
    EXPECT_EQ(ri.nano_seconds, 625000000UL);
    EXPECT_EQ(dt_mul_interval(&i, 0.0, &ri), DT_OK);
    EXPECT_EQ(ri.seconds, 0UL);
    EXPECT_EQ(ri.nano_seconds, 0UL);
}

TEST_F(DtCase, init_representation)
{
    dt_representation_t r = {0,};
    EXPECT_EQ(dt_init_representation(2012, 12, 21, 8, 30, 45, 123456789, NULL), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_init_representation(2012, 12, 21, 8, 30, 45, -1, &r), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_init_representation(2012, 12, 21, 8, 30, 45, 1000000000, &r), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_init_representation(2012, 12, 21, 8, 30, -1, 123456789, &r), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_init_representation(2012, 12, 21, 8, 30, 60, 123456789, &r), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_init_representation(2012, 12, 21, 8, -1, 45, 123456789, &r), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_init_representation(2012, 12, 21, 8, 60, 45, 123456789, &r), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_init_representation(2012, 12, 21, -1, 30, 45, 123456789, &r), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_init_representation(2012, 12, 21, 60, 30, 45, 123456789, &r), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_init_representation(2012, 12, 0, 8, 30, 45, 123456789, &r), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_init_representation(2012, 0, 21, 8, 30, 45, 123456789, &r), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_init_representation(0, 12, 21, 8, 30, 45, 123456789, &r), DT_INVALID_ARGUMENT);
    // Passage from Julian to Gregorian calendar
    EXPECT_EQ(dt_init_representation(1582, 10, 10, 8, 30, 45, 123456789, &r), DT_INVALID_ARGUMENT);

    EXPECT_EQ(dt_init_representation(2012, 12, 21, 8, 30, 45, 123456789, &r), DT_OK);
    EXPECT_EQ(r.year, 2012);
    EXPECT_EQ(r.month, 12);
    EXPECT_EQ(r.day, 21);
    EXPECT_EQ(r.hour, 8);
    EXPECT_EQ(r.minute, 30);
    EXPECT_EQ(r.second, 45);
    EXPECT_EQ(r.nano_second, 123456789);
}

TEST_F(DtCase, representation_timestamp_conversion)
{
    dt_representation_t r = {0,};
    dt_timestamp_t t = {0,};
    dt_representation_t rr = {0,};
    dt_representation_t urr = {0,};
    dt_timezone_t tz_moscow = {0,};
    dt_timezone_t tz_berlin = {0,};
    dt_timezone_t tz_utc = {0,};
    //Lookup timezones
    EXPECT_EQ(dt_timezone_lookup(MOSCOW_TZ_NAME, &tz_moscow), DT_OK);
    EXPECT_EQ(dt_timezone_lookup(BERLIN_TZ_NAME, &tz_berlin), DT_OK);
    EXPECT_EQ(dt_timezone_lookup(UTC_TZ_NAME, &tz_utc), DT_OK);

    //Local timezone
    EXPECT_TRUE(dt_init_representation(2009, 1, 15, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_to_timestamp(&r, NULL, &t, NULL), DT_OK);
    EXPECT_EQ(dt_timestamp_to_representation(&t, NULL, &rr), DT_OK);

    // Europe/Moscow
    EXPECT_TRUE(dt_init_representation(2009, 1, 15, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_to_timestamp(&r, &tz_moscow, &t, NULL), DT_OK);
    EXPECT_EQ(dt_timestamp_to_representation(&t, &tz_moscow, &rr), DT_OK);
    EXPECT_EQ(memcmp(&r, &rr, sizeof(dt_representation_t)), 0);
    EXPECT_EQ(dt_timestamp_to_representation(&t, &tz_utc, &urr), DT_OK);
    EXPECT_EQ(urr.hour, 5);

    EXPECT_TRUE(dt_init_representation(2013, 1, 15, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_to_timestamp(&r, &tz_moscow, &t, NULL), DT_OK);
    EXPECT_EQ(dt_timestamp_to_representation(&t, &tz_moscow, &rr), DT_OK);
    EXPECT_EQ(memcmp(&r, &rr, sizeof(dt_representation_t)), 0);
    EXPECT_EQ(dt_timestamp_to_representation(&t, &tz_utc, &urr), DT_OK);
    EXPECT_EQ(urr.hour, 4);

    EXPECT_TRUE(dt_init_representation(2009, 7, 15, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_to_timestamp(&r, &tz_moscow, &t, NULL), DT_OK);
    EXPECT_EQ(dt_timestamp_to_representation(&t, &tz_moscow, &rr), DT_OK);
    EXPECT_EQ(memcmp(&r, &rr, sizeof(dt_representation_t)), 0);
    EXPECT_EQ(dt_timestamp_to_representation(&t, &tz_utc, &urr), DT_OK);
    EXPECT_EQ(urr.hour, 4);

    EXPECT_TRUE(dt_init_representation(2013, 7, 15, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_to_timestamp(&r, &tz_moscow, &t, NULL), DT_OK);
    EXPECT_EQ(dt_timestamp_to_representation(&t, &tz_moscow, &rr), DT_OK);
    EXPECT_EQ(memcmp(&r, &rr, sizeof(dt_representation_t)), 0);
    EXPECT_EQ(dt_timestamp_to_representation(&t, &tz_utc, &urr), DT_OK);
    EXPECT_EQ(urr.hour, 4);

    // Europe/Berlin
    EXPECT_TRUE(dt_init_representation(2009, 1, 15, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_to_timestamp(&r, &tz_berlin, &t, NULL), DT_OK);
    EXPECT_EQ(dt_timestamp_to_representation(&t, &tz_berlin, &rr), DT_OK);
    EXPECT_EQ(memcmp(&r, &rr, sizeof(dt_representation_t)), 0);
    EXPECT_EQ(dt_timestamp_to_representation(&t, &tz_utc, &urr), DT_OK);
    EXPECT_EQ(urr.hour, 7);

    EXPECT_TRUE(dt_init_representation(2013, 1, 15, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_to_timestamp(&r, &tz_berlin, &t, NULL), DT_OK);
    EXPECT_EQ(dt_timestamp_to_representation(&t, &tz_berlin, &rr), DT_OK);
    EXPECT_EQ(memcmp(&r, &rr, sizeof(dt_representation_t)), 0);
    EXPECT_EQ(dt_timestamp_to_representation(&t, &tz_utc, &urr), DT_OK);
    EXPECT_EQ(urr.hour, 7);

    EXPECT_TRUE(dt_init_representation(2009, 7, 15, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_to_timestamp(&r, &tz_berlin, &t, NULL), DT_OK);
    EXPECT_EQ(dt_timestamp_to_representation(&t, &tz_berlin, &rr), DT_OK);
    EXPECT_EQ(memcmp(&r, &rr, sizeof(dt_representation_t)), 0);
    EXPECT_EQ(dt_timestamp_to_representation(&t, &tz_utc, &urr), DT_OK);
    EXPECT_EQ(urr.hour, 6);

    EXPECT_TRUE(dt_init_representation(2013, 7, 15, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_to_timestamp(&r, &tz_berlin, &t, NULL), DT_OK);
    EXPECT_EQ(dt_timestamp_to_representation(&t, &tz_berlin, &rr), DT_OK);
    EXPECT_EQ(memcmp(&r, &rr, sizeof(dt_representation_t)), 0);
    EXPECT_EQ(dt_timestamp_to_representation(&t, &tz_utc, &urr), DT_OK);
    EXPECT_EQ(urr.hour, 6);

    EXPECT_EQ(dt_timezone_cleanup(&tz_moscow), DT_OK);
    EXPECT_EQ(dt_timezone_cleanup(&tz_berlin), DT_OK);
    EXPECT_EQ(dt_timezone_cleanup(&tz_utc), DT_OK);

    // TODO: Case for current time zone!
}

TEST_F(DtCase, representation_day_of_week)
{
    dt_representation_t r = {0,};
    int dow = 0;
    EXPECT_TRUE(dt_init_representation(2013, 8, 11, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_day_of_week(NULL, &dow), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_representation_day_of_week(&r, NULL), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_representation_day_of_week(&r, &dow), DT_OK);
    EXPECT_EQ(dow, 1);
    EXPECT_TRUE(dt_init_representation(2013, 8, 12, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_day_of_week(&r, &dow), DT_OK);
    EXPECT_EQ(dow, 2);
    EXPECT_TRUE(dt_init_representation(2013, 8, 13, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_day_of_week(&r, &dow), DT_OK);
    EXPECT_EQ(dow, 3);
    EXPECT_TRUE(dt_init_representation(2013, 8, 14, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_day_of_week(&r, &dow), DT_OK);
    EXPECT_EQ(dow, 4);
    EXPECT_TRUE(dt_init_representation(2013, 8, 15, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_day_of_week(&r, &dow), DT_OK);
    EXPECT_EQ(dow, 5);
    EXPECT_TRUE(dt_init_representation(2013, 8, 16, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_day_of_week(&r, &dow), DT_OK);
    EXPECT_EQ(dow, 6);
    EXPECT_TRUE(dt_init_representation(2013, 8, 17, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_day_of_week(&r, &dow), DT_OK);
    EXPECT_EQ(dow, 7);
    EXPECT_TRUE(dt_init_representation(2013, 8, 18, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_day_of_week(&r, &dow), DT_OK);
    EXPECT_EQ(dow, 1);
}

TEST_F(DtCase, representation_day_of_year)
{
    dt_representation_t r = {0,};
    int doy = 0;

    EXPECT_EQ(dt_representation_day_of_year(NULL, &doy), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_representation_day_of_year(&r, NULL), DT_INVALID_ARGUMENT);

    EXPECT_TRUE(dt_init_representation(2013, 1, 1, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_day_of_year(&r, &doy), DT_OK);
    EXPECT_EQ(doy, 1);
    EXPECT_TRUE(dt_init_representation(2013, 8, 11, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_day_of_year(&r, &doy), DT_OK);
    EXPECT_EQ(doy, 223);
    EXPECT_TRUE(dt_init_representation(2013, 12, 31, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_day_of_year(&r, &doy), DT_OK);
    EXPECT_EQ(doy, 365);
    EXPECT_TRUE(dt_init_representation(2012, 12, 31, 8, 0, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_day_of_year(&r, &doy), DT_OK);
    EXPECT_EQ(doy, 366);
}

TEST_F(DtCase, representation_to_tm)
{
    // TODO: Check for dow/doy on invalid representation
    dt_representation_t r = {0,};
    struct tm tm = {0,};

    EXPECT_EQ(dt_representation_to_tm(NULL, &tm), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_representation_to_tm(&r, NULL), DT_INVALID_ARGUMENT);

    EXPECT_TRUE(dt_init_representation(2012, 12, 21, 8, 30, 45, 123456789, &r) == DT_OK);
    EXPECT_EQ(dt_representation_to_tm(&r, &tm), DT_OK);
    EXPECT_EQ(tm.tm_year, 112);
    EXPECT_EQ(tm.tm_mon, 11);
    EXPECT_EQ(tm.tm_mday, 21);
    EXPECT_EQ(tm.tm_hour, 8);
    EXPECT_EQ(tm.tm_min, 30);
    EXPECT_EQ(tm.tm_sec, 45);
    EXPECT_EQ(tm.tm_wday, 5);
    EXPECT_EQ(tm.tm_yday, 355);
}

TEST_F(DtCase, tm_to_representation)
{
    struct tm tm = {0,};
    tm.tm_year = 112;
    tm.tm_mon = 11;
    tm.tm_mday = 21;
    tm.tm_hour = 8;
    tm.tm_min = 30;
    tm.tm_sec = -1;
    dt_representation_t r = {0,};

    EXPECT_EQ(dt_tm_to_representation(NULL, 123456789, &r), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_tm_to_representation(&tm, 0, NULL), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_tm_to_representation(&tm, 123456789, &r), DT_OK);  // Invalid POSIX breakdown structure is allowed
    tm.tm_sec = 45;
    EXPECT_EQ(dt_tm_to_representation(&tm, 123456789, &r), DT_OK);
    EXPECT_EQ(r.year, 2012);
    EXPECT_EQ(r.month, 12);
    EXPECT_EQ(r.day, 21);
    EXPECT_EQ(r.hour, 8);
    EXPECT_EQ(r.minute, 30);
    EXPECT_EQ(r.second, 45);
    EXPECT_EQ(r.nano_second, 123456789);
}

TEST_F(DtCase, to_string)
{
    char overflowableFormat[1024] = {0};
    memset(overflowableFormat, '0', sizeof(overflowableFormat) - 1);

    const char *timeOnly = "08:31";
    const char *timeOnlyFormat = "%H:%M";
    const char *dateTime1 = "11/09/2001 16:54:12";
    const char *dateTimeFormat1 = "%d/%m/%Y %H:%M:%S";
    const char *dateTime2 = "11/09/2001 16:54:12.987654321 987";
    const char *dateTimeFormat2 = "%d/%m/%Y %H:%M:%S.%f %3f";
    const char *dateTime3 = "11/09/2001 16:54:12.987654321 9876 11";
    const char *dateTimeFormat3 = "%d/%m/%Y %H:%M:%S.%f %4f %d";
    const char *dateTime4 = "11/09/2001 16:54:12.987654321 987698765 11";
    const char *dateTimeFormat4 = "%d/%m/%Y %H:%M:%S.%f %4f%5f %d";
    const char *dateTime5 = "11/09/2001 16:54:12.987654321 987698765%1_ 11";
    const char *dateTimeFormat5 = "%d/%m/%Y %H:%M:%S.%f %4f%5f%1_ %d";

    dt_representation_t tr1 = {0,};
    dt_representation_t tr2 = {0,};
    char buf[1024] = {0,};
    const size_t buf_size = sizeof(buf);

    tr1.hour = 8;
    tr1.minute = 31;

    EXPECT_EQ(dt_to_string(NULL, overflowableFormat, buf, buf_size), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_to_string(&tr1, NULL, buf, buf_size), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_to_string(&tr1, overflowableFormat, NULL, buf_size), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_to_string(&tr1, overflowableFormat, buf, 0), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_to_string(&tr1, overflowableFormat, buf, buf_size), DT_OVERFLOW);

    EXPECT_EQ(dt_to_string(&tr1, timeOnlyFormat, buf, buf_size), DT_OK);
    EXPECT_STREQ(timeOnly, buf);

    tr2.year = 2001;
    tr2.month = 9;
    tr2.day = 11;
    tr2.hour = 16;
    tr2.minute = 54;
    tr2.second = 12;
    tr2.nano_second = 987654321;

    // Checking for overflow
    EXPECT_EQ(dt_to_string(&tr2, dateTimeFormat1, buf, 10), DT_OVERFLOW);
    EXPECT_EQ(dt_to_string(&tr2, dateTimeFormat2, buf, 10), DT_OVERFLOW);
    EXPECT_EQ(dt_to_string(&tr2, dateTimeFormat2, buf, 25), DT_OVERFLOW);

    EXPECT_EQ(dt_to_string(&tr2, dateTimeFormat1, buf, buf_size), DT_OK);
    EXPECT_STREQ(dateTime1, buf);

    EXPECT_EQ(dt_to_string(&tr2, dateTimeFormat2, buf, buf_size), DT_OK);
    EXPECT_STREQ(buf, dateTime2);

    EXPECT_EQ(dt_to_string(&tr2, dateTimeFormat3, buf, buf_size), DT_OK);
    EXPECT_STREQ(buf, dateTime3);

    EXPECT_EQ(dt_to_string(&tr2, dateTimeFormat4, buf, buf_size), DT_OK);
    EXPECT_STREQ(buf, dateTime4);

    EXPECT_EQ(dt_to_string(&tr2, dateTimeFormat5, buf, buf_size), DT_OK);
    EXPECT_STREQ(buf, dateTime5);
}

TEST_F(DtCase, from_string)
{
    char overflowableFormat[1024] = {0};
    memset(overflowableFormat, '0', sizeof(overflowableFormat) - 1);

    const char *timeOnly = "08:31";
    const char *timeOnlyFormat = "%H:%M";
    const char *fractionalOnly = "123 456";
    const char *fractionalOnlyFormat = "%f %f";
    const char *dateTime1 = "11/09/2001 16:54:12";
    const char *dateTimeFormat1 = "%d/%m/%Y %H:%M:%S";
    const char *dateTime2 = "11/09/2001 16:54:12.123456789";
    const char *dateTimeFormat2 = "%d/%m/%Y %H:%M:%S.%f";
    const char *dateTime3 = "11/09/2001 16:54:12.123";
    const char *dateTimeFormat3 = "%d/%m/%Y %H:%M:%S.%f";
    const char *dateTime5 = "123 11/09/2001 16:54:12";
    const char *dateTimeFormat5 = "%6f %d/%m/%Y %H:%M:%S";
    const char *dateTime6 = "123 11/09/2001 456 16:54:12 789";
    const char *dateTimeFormat6 = "%6f %d/%m/%Y %f %H:%M:%S %f";

    const char *invalidTimeOnly = "08:31a";
    const char *invalidFractionalOnly = "123";
    const char *invalidDateTime1 = "11/09/2001 16:pp:12";
    const char *invalidDateTime2 = "11/09/2001 16:54:";
    const char *invalidDateTime3 = "11/09/2001 16:54:12.";
    const char *invalidDateTime4 = "11/09/2001 16:54:12.a";
    const char *invalidDateTime5 = "123 11/aa/2001 456 16:54:12 789";
    const char *invalidDateTime6 = "123 11/09/2001 456 16:54:12 abc";
    const char *invalidDateTime7 = "123 11/09/2001 456 16:54:12 789a";

    dt_representation_t tr = {0,};

    EXPECT_EQ(dt_from_string(NULL, timeOnlyFormat, &tr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_from_string(timeOnly, NULL, &tr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_from_string(timeOnly, timeOnlyFormat, NULL), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_from_string(timeOnly, overflowableFormat, &tr), DT_OVERFLOW);

    EXPECT_EQ(dt_from_string(invalidTimeOnly, timeOnlyFormat, &tr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_from_string(invalidFractionalOnly, fractionalOnlyFormat, &tr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_from_string(invalidDateTime1, dateTimeFormat1, &tr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_from_string(invalidDateTime2, dateTimeFormat1, &tr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_from_string(invalidDateTime3, dateTimeFormat2, &tr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_from_string(invalidDateTime4, dateTimeFormat2, &tr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_from_string(invalidDateTime5, dateTimeFormat6, &tr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_from_string(invalidDateTime6, dateTimeFormat6, &tr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_from_string(invalidDateTime7, dateTimeFormat6, &tr), DT_INVALID_ARGUMENT);

    EXPECT_EQ(dt_from_string(timeOnly, timeOnlyFormat, &tr), DT_OK);
    EXPECT_EQ(tr.hour, 8);
    EXPECT_EQ(tr.minute, 31);

    memset(&tr, 0, sizeof(dt_representation_t));
    EXPECT_EQ(dt_from_string(fractionalOnly, fractionalOnlyFormat, &tr), DT_OK);
    EXPECT_EQ(tr.nano_second, 456000000);

    memset(&tr, 0, sizeof(dt_representation_t));
    EXPECT_EQ(dt_from_string(dateTime1, dateTimeFormat1, &tr), DT_OK);
    EXPECT_EQ(tr.day, 11);
    EXPECT_EQ(tr.month, 9);
    EXPECT_EQ(tr.year, 2001);
    EXPECT_EQ(tr.minute, 54);
    EXPECT_EQ(tr.hour, 16);
    EXPECT_EQ(tr.second, 12);
    EXPECT_EQ(tr.nano_second, 0);

    memset(&tr, 0, sizeof(dt_representation_t));
    EXPECT_EQ(dt_from_string(dateTime2, dateTimeFormat2, &tr), DT_OK);
    EXPECT_EQ(tr.day, 11);
    EXPECT_EQ(tr.month, 9);
    EXPECT_EQ(tr.year, 2001);
    EXPECT_EQ(tr.minute, 54);
    EXPECT_EQ(tr.hour, 16);
    EXPECT_EQ(tr.second, 12);
    EXPECT_EQ(tr.nano_second, 123456789);

    memset(&tr, 0, sizeof(dt_representation_t));
    EXPECT_EQ(dt_from_string(dateTime3, dateTimeFormat3, &tr), DT_OK);
    EXPECT_EQ(tr.day, 11);
    EXPECT_EQ(tr.month, 9);
    EXPECT_EQ(tr.year, 2001);
    EXPECT_EQ(tr.minute, 54);
    EXPECT_EQ(tr.hour, 16);
    EXPECT_EQ(tr.second, 12);
    EXPECT_EQ(tr.nano_second, 123000000);

    memset(&tr, 0, sizeof(dt_representation_t));
    EXPECT_EQ(dt_from_string(dateTime5, dateTimeFormat5, &tr), DT_OK);
    EXPECT_EQ(tr.day, 11);
    EXPECT_EQ(tr.month, 9);
    EXPECT_EQ(tr.year, 2001);
    EXPECT_EQ(tr.minute, 54);
    EXPECT_EQ(tr.hour, 16);
    EXPECT_EQ(tr.second, 12);
    EXPECT_EQ(tr.nano_second, 123000000);

    memset(&tr, 0, sizeof(dt_representation_t));
    EXPECT_EQ(dt_from_string(dateTime6, dateTimeFormat6, &tr), DT_OK);
    EXPECT_EQ(tr.day, 11);
    EXPECT_EQ(tr.month, 9);
    EXPECT_EQ(tr.year, 2001);
    EXPECT_EQ(tr.minute, 54);
    EXPECT_EQ(tr.hour, 16);
    EXPECT_EQ(tr.second, 12);
    EXPECT_EQ(tr.nano_second, 789000000);
}

TEST_F(DtCase, lookup_free_timezone)
{
    dt_timezone_t tz_moscow_standard = {0,};
    dt_timezone_t tz_moscow_olsen = {0,};
    dt_timezone_t tz_unreal_timezone = {0,};

    EXPECT_EQ(dt_timezone_lookup(MOSCOW_WINDOWS_STANDARD_TZ_NAME, NULL), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_timezone_lookup(NULL, &tz_unreal_timezone), DT_INVALID_ARGUMENT);

    EXPECT_EQ(dt_timezone_lookup(UNREAL_TIMEZONE_NAME, &tz_unreal_timezone), DT_TIMEZONE_NOT_FOUND);
    EXPECT_EQ(dt_timezone_lookup(MOSCOW_WINDOWS_STANDARD_TZ_NAME, &tz_moscow_standard), DT_OK);
    EXPECT_EQ(dt_timezone_lookup(MOSCOW_OLSEN_TZ_NAME, &tz_moscow_olsen), DT_OK);

    EXPECT_EQ(dt_timezone_cleanup(NULL), DT_INVALID_ARGUMENT);

    EXPECT_EQ(dt_timezone_cleanup(&tz_moscow_olsen), DT_OK);
    EXPECT_EQ(dt_timezone_cleanup(&tz_moscow_standard), DT_OK);


}

TEST_F(DtCase, edge_of_time_switch)
{
    dt_representation_t r = {0,};
    dt_timestamp_t t = {0,};
    dt_representation_t result = {0,};
    dt_timezone_t tz_moscow = {0,};
    dt_timezone_t tz_utc = {0,};
    //Lookup timezones
    EXPECT_EQ(dt_timezone_lookup(MOSCOW_TZ_NAME, &tz_moscow), DT_OK);
    EXPECT_EQ(dt_timezone_lookup(UTC_TZ_NAME, &tz_utc), DT_OK);

    // before switch time...
    EXPECT_TRUE(dt_init_representation(2008, 3, 30, 1, 30, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_to_timestamp(&r, &tz_moscow, &t, NULL), DT_OK);

    EXPECT_EQ(dt_timestamp_to_representation(&t, &tz_utc, &result), DT_OK);
    EXPECT_EQ(result.hour, 22);
    EXPECT_EQ(result.minute, 30);
    EXPECT_EQ(result.day, 29);

    // non exists time 2:30 30.03.2008 in Moscow
    EXPECT_TRUE(dt_init_representation(2008, 3, 30, 2, 30, 0, 0, &r) == DT_OK);
    dt_representation_to_timestamp(&r, &tz_moscow, &t, NULL);//FIXME: must return error for non exist time

    EXPECT_EQ(dt_timestamp_to_representation(&t, &tz_utc, &result), DT_OK);
    EXPECT_EQ(result.hour, 22);
    EXPECT_EQ(result.minute, 30);
    EXPECT_EQ(result.day, 29);

    // undefined double time
    EXPECT_TRUE(dt_init_representation(2008, 10, 26, 2, 30, 0, 0, &r) == DT_OK);
    EXPECT_EQ(dt_representation_to_timestamp(&r, &tz_moscow, &t, NULL), DT_OK);//TODO: must return both timestamps

    EXPECT_EQ(dt_timestamp_to_representation(&t, &tz_utc, &result), DT_OK);
    EXPECT_EQ(result.hour, 22);
    EXPECT_EQ(result.minute, 30);
    EXPECT_EQ(result.day, 25);

}
