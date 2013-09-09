#include "dt_case.h"
#include "libtz/dt_precise.h"

#define MOSCOW_WINDOWS_STANDARD_TZ_NAME "Russian Standard Time"
#define MOSCOW_OLSEN_TZ_NAME  "Europe/Moscow"
#define UNREAL_TIMEZONE_NAME "this time zone is unreal, you can not find it anywhere, so good luck"

#ifdef _WIN32

#define UTC_TZ_NAME "UTC"
#define MOSCOW_TZ_NAME MOSCOW_WINDOWS_STANDARD_TZ_NAME
#define BERLIN_TZ_NAME "W. Europe Standard Time"
#define GMT_MINUS_5_TZ_NAME "GMT-5"
#define GMT_PLUS_5_TZ_NAME "GMT+5"

#include <Windows.h>
#define sleep(x) Sleep(x*1000)

#else

#define UTC_TZ_NAME "UTC"
#define MOSCOW_TZ_NAME MOSCOW_OLSEN_TZ_NAME
#define BERLIN_TZ_NAME "Europe/Berlin"
#define GMT_MINUS_5_TZ_NAME "West Asia Standard Time"
#define GMT_PLUS_5_TZ_NAME "SA Pacific Standard Time"

#endif


DtCase::DtCase() :
    ::testing::Test()
{}

TEST_F(DtCase, is_leap)
{
    EXPECT_FALSE(dt_is_leap_year(2013));
    EXPECT_TRUE(dt_is_leap_year(2012) == DT_TRUE);
    EXPECT_FALSE(dt_is_leap_year(2100) == DT_TRUE);
    EXPECT_TRUE(dt_is_leap_year(2400) == DT_TRUE);
}

TEST_F(DtCase, strerror)
{
    static const char *no_error_message = "<No error>";
    static const char *invalid_argument_error_message = "Invalid argument";
    static const char *timezone_not_found_error_message = "Timezone not found";
    static const char *system_call_error_message = "System call error";
    static const char *unknown_error_message = "Unknown error";
    static const char *invalid_status_error_message = "<Invalid result status>";
    EXPECT_EQ(strcmp(dt_strerror(DT_OK), no_error_message), 0);
    EXPECT_EQ(strcmp(dt_strerror(DT_INVALID_ARGUMENT), invalid_argument_error_message), 0);
    EXPECT_EQ(strcmp(dt_strerror(DT_TIMEZONE_NOT_FOUND), timezone_not_found_error_message), 0);
    EXPECT_EQ(strcmp(dt_strerror(DT_SYSTEM_CALL_ERROR), system_call_error_message), 0);
    EXPECT_EQ(strcmp(dt_strerror(DT_UNKNOWN_ERROR), unknown_error_message), 0);
    EXPECT_EQ(strcmp(dt_strerror((dt_status_t) 250), invalid_status_error_message), 0);
}

TEST_F(DtCase, validate_representation)
{
    EXPECT_EQ(dt_validate_representation(2012, 12, 21, 8, 30, 45, 123456789), DT_TRUE);

    EXPECT_EQ(dt_validate_representation(2012, 12, 21, 8, 30, 45, -1), DT_FALSE);
    EXPECT_EQ(dt_validate_representation(2012, 12, 21, 8, 30, 45, 1000000000), DT_FALSE);
    EXPECT_EQ(dt_validate_representation(2012, 12, 21, 8, 30, -1, 123456789), DT_FALSE);
    EXPECT_EQ(dt_validate_representation(2012, 12, 21, 8, 30, 60, 123456789), DT_FALSE);
    EXPECT_EQ(dt_validate_representation(2012, 12, 21, 8, -1, 45, 123456789), DT_FALSE);
    EXPECT_EQ(dt_validate_representation(2012, 12, 21, 8, 60, 45, 123456789), DT_FALSE);
    EXPECT_EQ(dt_validate_representation(2012, 12, 21, -1, 30, 45, 123456789), DT_FALSE);
    EXPECT_EQ(dt_validate_representation(2012, 12, 21, 60, 30, 45, 123456789), DT_FALSE);
    EXPECT_EQ(dt_validate_representation(2012, 12, 0, 8, 30, 45, 123456789), DT_FALSE);
    EXPECT_EQ(dt_validate_representation(2012, 0, 21, 8, 30, 45, 123456789), DT_FALSE);
    EXPECT_EQ(dt_validate_representation(0, 12, 21, 8, 30, 45, 123456789), DT_FALSE);
    // Passage from Julian to Gregorian calendar
    EXPECT_EQ(dt_validate_representation(1582, 10, 10, 8, 30, 45, 123456789), DT_FALSE);
}

TEST_F(DtCase, now_compare_timestamps)
{
    dt_timestamp_t ts_01;
    EXPECT_EQ(dt_now(NULL), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_now(&ts_01), DT_OK);
    sleep(1);
    dt_timestamp_t ts_02;
    EXPECT_EQ(dt_now(&ts_02), DT_OK);
    int cr;
    EXPECT_EQ(dt_compare_timestamps(NULL, &ts_02, &cr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_compare_timestamps(&ts_01, NULL, &cr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_compare_timestamps(&ts_01, &ts_02, NULL), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_compare_timestamps(&ts_01, &ts_02, &cr), DT_OK);
    EXPECT_LT(cr, 0);
    EXPECT_EQ(dt_compare_timestamps(&ts_02, &ts_01, &cr), DT_OK);
    EXPECT_GT(cr, 0);
    EXPECT_EQ(dt_compare_timestamps(&ts_01, &ts_01, &cr), DT_OK);
    EXPECT_EQ(cr, 0);
}


TEST_F(DtCase, offset_to)
{
    dt_timestamp_t ts_01;
    dt_timestamp_t ts_02;
    dt_offset_t o;
    dt_timezone_t tz_moscow = {0,};

    EXPECT_EQ(dt_timezone_lookup(&tz_moscow, MOSCOW_TZ_NAME), DT_OK);

    EXPECT_EQ(dt_offset_between(NULL, &ts_02, &o), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_offset_between(&ts_01, NULL, &o), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_offset_between(&ts_01, &ts_02, NULL), DT_INVALID_ARGUMENT);

    dt_representation_t r;
    EXPECT_TRUE(dt_init_representation(2012, 12, 21, 8, 30, 45, 123456789L, &r) == DT_OK);
    EXPECT_TRUE(dt_representation_to_timestamp(&r, &tz_moscow, &ts_01, NULL) == DT_OK);
    EXPECT_EQ(dt_offset_between(&ts_01, &ts_01, &o), DT_OK);
    EXPECT_EQ(o.is_forward, DT_TRUE);
    EXPECT_EQ(o.duration.seconds, 0L);
    EXPECT_EQ(o.duration.nano_seconds, 0L);

    EXPECT_TRUE(dt_init_representation(2012, 12, 22, 8, 30, 45, 123456889L, &r) == DT_OK);
    EXPECT_TRUE(dt_representation_to_timestamp(&r, &tz_moscow, &ts_02, NULL) == DT_OK);
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

TEST_F(DtCase, apply_offset)
{
    dt_timestamp_t ts_01;
    dt_timestamp_t ts_02;
    dt_offset_t o;
    dt_timezone_t tz_moscow = {0,};

    EXPECT_EQ(dt_timezone_lookup(&tz_moscow, MOSCOW_TZ_NAME), DT_OK);

    EXPECT_EQ(dt_apply_offset(NULL, &o, &ts_02), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_apply_offset(&ts_01, NULL, &ts_02), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_apply_offset(&ts_01, &o, NULL), DT_INVALID_ARGUMENT);

    dt_representation_t r;
    EXPECT_TRUE(dt_init_representation(2012, 12, 21, 8, 30, 45, 123456789UL, &r) == DT_OK);
    EXPECT_TRUE(dt_representation_to_timestamp(&r, &tz_moscow, &ts_01, NULL) == DT_OK);
    dt_representation_t rr;

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
    time_t rtm;
    time_t now = time(NULL);
    dt_timestamp_t ts;
    unsigned long nano_second;

    EXPECT_EQ(dt_posix_time_to_timestamp(-1, 123456789L, &ts), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_posix_time_to_timestamp(0, 123456789L, NULL), DT_INVALID_ARGUMENT);

    EXPECT_EQ(dt_timestamp_to_posix_time(NULL, &rtm, NULL), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_timestamp_to_posix_time(&ts, NULL, NULL), DT_INVALID_ARGUMENT);

    EXPECT_EQ(dt_posix_time_to_timestamp(0, 123456789L, &ts), DT_OK);
    dt_offset_t o;
    EXPECT_TRUE(dt_init_interval(DT_SECONDS_PER_DAY, 0, &o.duration) == DT_OK);
    o.is_forward = DT_FALSE;
    dt_timestamp_t nts;
    EXPECT_TRUE(dt_apply_offset(&ts, &o, &nts) == DT_OK);
    EXPECT_EQ(dt_timestamp_to_posix_time(&nts, &rtm, &nano_second), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_timestamp_to_posix_time(&ts, &rtm, &nano_second), DT_OK);
    EXPECT_EQ(rtm, 0);
    EXPECT_EQ(nano_second, 123456789L);

    EXPECT_EQ(dt_posix_time_to_timestamp(now, 123456789L, &ts), DT_OK);
    EXPECT_EQ(dt_timestamp_to_posix_time(&ts, &rtm, &nano_second), DT_OK);
    EXPECT_EQ(rtm, now);
    EXPECT_EQ(nano_second, 123456789L);
}

TEST_F(DtCase, init_interval)
{
    dt_interval_t i;
    EXPECT_EQ(dt_init_interval(1, 1000100500L, NULL), DT_INVALID_ARGUMENT);

    EXPECT_EQ(dt_init_interval(1, 100500L, &i), DT_OK);
    EXPECT_EQ(i.seconds, 1);
    EXPECT_EQ(i.nano_seconds, 100500L);

    EXPECT_EQ(dt_init_interval(1, 1000100500L, &i), DT_OK);
    EXPECT_EQ(i.seconds, 2);
    EXPECT_EQ(i.nano_seconds, 100500L);
}

TEST_F(DtCase, compare_interval)
{
    dt_interval_t i_01;
    EXPECT_TRUE(dt_init_interval(1L, 123456789L, &i_01) == DT_OK);
    dt_interval_t i_02;
    EXPECT_TRUE(dt_init_interval(2L, 123456789L, &i_02) == DT_OK);
    int cr;
    EXPECT_EQ(dt_compare_intervals(NULL, &i_02, &cr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_compare_intervals(&i_01, NULL, &cr), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_compare_intervals(&i_01, &i_02, NULL), DT_INVALID_ARGUMENT);
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
    dt_interval_t i_01;
    EXPECT_TRUE(dt_init_interval(1L, 123456789L, &i_01) == DT_OK);
    dt_interval_t i_02;
    EXPECT_TRUE(dt_init_interval(2L, 123456789L, &i_02) == DT_OK);
    dt_interval_t ri;
    EXPECT_EQ(dt_sum_intervals(NULL, &i_02, &ri), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_sum_intervals(&i_01, NULL, &ri), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_sum_intervals(&i_01, &i_02, NULL), DT_INVALID_ARGUMENT);
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
    dt_interval_t i_01;
    EXPECT_TRUE(dt_init_interval(2L, 123456789L, &i_01) == DT_OK);
    dt_interval_t i_02;
    EXPECT_TRUE(dt_init_interval(3L, 123456789L, &i_02) == DT_OK);
    dt_interval_t ri;
    EXPECT_EQ(dt_sub_intervals(NULL, &i_02, &ri), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_sub_intervals(&i_01, NULL, &ri), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_sub_intervals(&i_01, &i_02, NULL), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_sub_intervals(&i_01, &i_02, &ri), DT_OK);
    EXPECT_EQ(ri.seconds, 0L);
    EXPECT_EQ(ri.nano_seconds, 0L);
    EXPECT_TRUE(dt_init_interval(1L, 900000000L, &i_02) == DT_OK);
    EXPECT_EQ(dt_sub_intervals(&i_01, &i_02, &ri), DT_OK);
    EXPECT_EQ(ri.seconds, 0L);
    EXPECT_EQ(ri.nano_seconds, 223456789L);
}

TEST_F(DtCase, miltiply_interval)
{
    dt_interval_t i;
    EXPECT_TRUE(dt_init_interval(3L, 500000000L, &i) == DT_OK);
    dt_interval_t ri;
    EXPECT_EQ(dt_mul_interval(NULL, 2.5, &ri), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_mul_interval(&i, 2.5, NULL), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_mul_interval(&i, 2.5, &ri), DT_OK);
    EXPECT_EQ(ri.seconds, 8L);
    EXPECT_EQ(ri.nano_seconds, 750000000L);
    EXPECT_EQ(dt_mul_interval(&i, 0.75, &ri), DT_OK);
    EXPECT_EQ(ri.seconds, 2L);
    EXPECT_EQ(ri.nano_seconds, 625000000L);
}

TEST_F(DtCase, init_representation)
{
    dt_representation_t r;
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
    dt_representation_t r;
    dt_timestamp_t t;
    dt_representation_t rr;
    dt_representation_t urr;
    dt_timezone_t tz_moscow = {0,};
    dt_timezone_t tz_berlin = {0,};
    dt_timezone_t tz_utc= {0,};
    //Lookup timezones
    EXPECT_EQ(dt_timezone_lookup(&tz_moscow, MOSCOW_TZ_NAME), DT_OK);
    EXPECT_EQ(dt_timezone_lookup(&tz_berlin, BERLIN_TZ_NAME), DT_OK);
    EXPECT_EQ(dt_timezone_lookup(&tz_utc, UTC_TZ_NAME), DT_OK);

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
    dt_representation_t r;
    int dow;
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
    dt_representation_t r;
    int doy;

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
    dt_representation_t r;
    struct tm tm;

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
    struct tm tm;
    tm.tm_year = 112;
    tm.tm_mon = 11;
    tm.tm_mday = 21;
    tm.tm_hour = 8;
    tm.tm_min = 30;
    tm.tm_sec = -1;
    dt_representation_t r;

    EXPECT_EQ(dt_tm_to_representation(NULL, 123456789, &r), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_tm_to_representation(&tm, 0, NULL), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_tm_to_representation(&tm, 123456789, &r), DT_INVALID_ARGUMENT);
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

TEST_F(DtCase, wrongStringConvert)
{
    const char *timeOnly = "08:31";
    const char *timeOnlyWrong = "08:31a";
    const char *timeOnlyFormat = "%H:%M";
    const char *timeOnlyFormatWrong = "H:M";

    dt_representation_t tr = {0,};
    size_t buf_size = 1024;
    char buf[1024] = {0,};

    tr.hour = 8;
    tr.minute = 612;

    EXPECT_NE(dt_from_string(NULL, timeOnlyFormat, &tr, NULL, 0), DT_OK);
    EXPECT_NE(dt_from_string(timeOnlyWrong, NULL, NULL, NULL, 0), DT_OK);
    EXPECT_NE(dt_from_string(timeOnlyWrong, timeOnlyFormat, &tr, NULL, 0), DT_OK);
    EXPECT_NE(dt_from_string(timeOnly, timeOnlyFormatWrong, &tr, NULL, 0), DT_OK);

    EXPECT_NE(dt_to_string(NULL, UTC_TZ_NAME, timeOnlyFormatWrong, buf, buf_size), DT_OK);
    EXPECT_NE(dt_to_string(&tr, NULL, timeOnlyFormatWrong, buf, buf_size), DT_OK);
    EXPECT_NE(dt_to_string(&tr, UTC_TZ_NAME, NULL, buf, buf_size), DT_OK);
    EXPECT_NE(dt_to_string(&tr, UTC_TZ_NAME, timeOnlyFormatWrong, NULL, buf_size), DT_OK);
    EXPECT_NE(dt_to_string(&tr, UTC_TZ_NAME, timeOnlyFormatWrong, buf, 0), DT_OK);
}

TEST_F(DtCase, toStringConvert)
{
    const char *timeOnly1 = "08:31";
    const char *timeOnlyFormat1 = "%H:%M";
    const char *timeOnly2 = "11/09/2001 16:54:12";
    const char *timeOnlyFormat2 = "%d/%m/%Y %H:%M:%S";

    dt_representation_t tr1 = {0,};
    dt_representation_t tr2 = {0,};
    size_t buf_size = 1024;
    char buf[1024] = {0,};

    tr1.hour = 8;
    tr1.minute = 31;

    EXPECT_EQ(dt_to_string(&tr1, UTC_TZ_NAME, timeOnlyFormat1, buf, buf_size), EXIT_SUCCESS);
    EXPECT_STREQ(timeOnly1, buf);

    tr2.day = 11;
    tr2.month = 9;
    tr2.year = 2001;
    tr2.minute = 54;
    tr2.hour = 16;
    tr2.second = 12;

    EXPECT_EQ(dt_to_string(&tr2, UTC_TZ_NAME, timeOnlyFormat2, buf, buf_size), EXIT_SUCCESS);
    EXPECT_STREQ(timeOnly2, buf);
}

TEST_F(DtCase, fromStringConvert)
{
    const char *timeOnly1 = "08:31";
    const char *timeOnlyFormat1 = "%H:%M";
    const char *timeOnly2 = "11/09/2001 16:54:12";
    const char *timeOnlyFormat2 = "%d/%m/%Y %H:%M:%S";

    dt_representation_t tr1 = {0,};
    dt_representation_t tr2 = {0,};


    EXPECT_EQ(dt_from_string(timeOnly1, timeOnlyFormat1, &tr1, NULL, 0), DT_OK);
    EXPECT_EQ(tr1.hour, 8);
    EXPECT_EQ(tr1.minute, 31);


    EXPECT_EQ(dt_from_string(timeOnly2, timeOnlyFormat2, &tr2, NULL, 0), EXIT_SUCCESS);
    EXPECT_EQ(tr2.day, 11);
    EXPECT_EQ(tr2.month, 9);
    EXPECT_EQ(tr2.year, 2001);
    EXPECT_EQ(tr2.minute, 54);
    EXPECT_EQ(tr2.hour, 16);
    EXPECT_EQ(tr2.second, 12);
}

TEST_F(DtCase, lookup_free_timezone)
{
    dt_timezone_t tz_moscow_standard = {0,};
    dt_timezone_t tz_moscow_olsen = {0,};
    dt_timezone_t tz_unreal_timezone = {0,};

    EXPECT_EQ(dt_timezone_lookup(NULL, MOSCOW_WINDOWS_STANDARD_TZ_NAME), DT_INVALID_ARGUMENT);
    EXPECT_EQ(dt_timezone_lookup(&tz_unreal_timezone, NULL), DT_INVALID_ARGUMENT);

    EXPECT_EQ(dt_timezone_lookup(&tz_unreal_timezone, UNREAL_TIMEZONE_NAME), DT_TIMEZONE_NOT_FOUND);
    EXPECT_EQ(dt_timezone_lookup(&tz_moscow_standard, MOSCOW_WINDOWS_STANDARD_TZ_NAME), DT_OK);
    EXPECT_EQ(dt_timezone_lookup(&tz_moscow_olsen, MOSCOW_OLSEN_TZ_NAME), DT_OK);

    EXPECT_EQ(dt_timezone_cleanup(NULL), DT_INVALID_ARGUMENT);

    EXPECT_EQ(dt_timezone_cleanup(&tz_moscow_olsen), DT_OK);
    EXPECT_EQ(dt_timezone_cleanup(&tz_moscow_standard), DT_OK);


}
