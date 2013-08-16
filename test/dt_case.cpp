#include "dt_case.h"
#include "dt.h"

#ifdef _WIN32

#define UTC_TZ_NAME "Etc/UTC"
#define MOSCOW_TZ_NAME "Russian Standard Time"
#define BERLIN_TZ_NAME "W. Europe Standard Time"
#define GMT_MINUS_5_TZ_NAME "GMT-5"
#define GMT_PLUS_5_TZ_NAME "GMT+5"

#else

#define UTC_TZ_NAME "UTC"
#define MOSCOW_TZ_NAME "Europe/Moscow"
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
        EXPECT_TRUE(dt_is_leap_year(2012));
        EXPECT_FALSE(dt_is_leap_year(2100));
        EXPECT_TRUE(dt_is_leap_year(2400));
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


TEST_F(DtCase, timestamps_offset)
{
        dt_timestamp_t ts_01;
        dt_timestamp_t ts_02;
        dt_offset_t o;

        EXPECT_EQ(dt_timestamps_offset(NULL, &ts_02, &o), DT_INVALID_ARGUMENT);
        EXPECT_EQ(dt_timestamps_offset(&ts_01, NULL, &o), DT_INVALID_ARGUMENT);
        EXPECT_EQ(dt_timestamps_offset(&ts_01, &ts_02, NULL), DT_INVALID_ARGUMENT);

        dt_representation_t r;
        assert(dt_init_representation(2012, 12, 21, 8, 30, 45, 123456789L, &r) == DT_OK);
        assert(dt_representation_to_timestamp(&r, MOSCOW_TZ_NAME, &ts_01, NULL) == DT_OK);
        EXPECT_EQ(dt_timestamps_offset(&ts_01, &ts_01, &o), DT_OK);
        EXPECT_EQ(o.is_forward, DT_TRUE);
        EXPECT_EQ(o.duration.seconds, 0L);
        EXPECT_EQ(o.duration.nano_seconds, 0L);
        
        assert(dt_init_representation(2012, 12, 22, 8, 30, 45, 123456889L, &r) == DT_OK);
        assert(dt_representation_to_timestamp(&r, MOSCOW_TZ_NAME, &ts_02, NULL) == DT_OK);
        EXPECT_EQ(dt_timestamps_offset(&ts_01, &ts_02, &o), DT_OK);
        EXPECT_EQ(o.is_forward, DT_TRUE);
        EXPECT_EQ(o.duration.seconds, DT_SECONDS_PER_DAY);
        EXPECT_EQ(o.duration.nano_seconds, 100L);
        EXPECT_EQ(dt_timestamps_offset(&ts_02, &ts_01, &o), DT_OK);
        EXPECT_EQ(o.is_forward, DT_FALSE);
        EXPECT_EQ(o.duration.seconds, DT_SECONDS_PER_DAY);
        EXPECT_EQ(o.duration.nano_seconds, 100L);

        assert(dt_init_representation(2012, 12, 22, 8, 30, 45, 100L, &r) == DT_OK);
        assert(dt_representation_to_timestamp(&r, MOSCOW_TZ_NAME, &ts_02, NULL) == DT_OK);
        EXPECT_EQ(dt_timestamps_offset(&ts_01, &ts_02, &o), DT_OK);
        EXPECT_EQ(o.is_forward, DT_TRUE);
        EXPECT_EQ(o.duration.seconds, DT_SECONDS_PER_DAY - 1);
        EXPECT_EQ(o.duration.nano_seconds, 876543311L);
        EXPECT_EQ(dt_timestamps_offset(&ts_02, &ts_01, &o), DT_OK);
        EXPECT_EQ(o.is_forward, DT_FALSE);
        EXPECT_EQ(o.duration.seconds, DT_SECONDS_PER_DAY - 1);
        EXPECT_EQ(o.duration.nano_seconds, 876543311L);
}

TEST_F(DtCase, apply_offset)
{
        dt_timestamp_t ts_01;
        dt_timestamp_t ts_02;
        dt_offset_t o;

        EXPECT_EQ(dt_apply_offset(NULL, &o, &ts_02), DT_INVALID_ARGUMENT);
        EXPECT_EQ(dt_apply_offset(&ts_01, NULL, &ts_02), DT_INVALID_ARGUMENT);
        EXPECT_EQ(dt_apply_offset(&ts_01, &o, NULL), DT_INVALID_ARGUMENT);

        dt_representation_t r;
        assert(dt_init_representation(2012, 12, 21, 8, 30, 45, 123456789L, &r) == DT_OK);
        assert(dt_representation_to_timestamp(&r, MOSCOW_TZ_NAME, &ts_01, NULL) == DT_OK);
        dt_representation_t rr;

        assert(dt_init_interval(DT_SECONDS_PER_DAY, 100L, &o.duration) == DT_OK);
        o.is_forward = DT_TRUE;
        EXPECT_EQ(dt_apply_offset(&ts_01, &o, &ts_02), DT_OK);
        assert(dt_timestamp_to_representation(&ts_02, MOSCOW_TZ_NAME, &rr) == DT_OK);
        EXPECT_EQ(rr.year, 2012);
        EXPECT_EQ(rr.month, 12);
        EXPECT_EQ(rr.day, 22);
        EXPECT_EQ(rr.hour, 8);
        EXPECT_EQ(rr.minute, 30);
        EXPECT_EQ(rr.second, 45);
        EXPECT_EQ(rr.nano_second, 123456889L);

        assert(dt_init_interval(DT_SECONDS_PER_DAY, 900000000L, &o.duration) == DT_OK);
        o.is_forward = DT_TRUE;
        EXPECT_EQ(dt_apply_offset(&ts_01, &o, &ts_02), DT_OK);
        assert(dt_timestamp_to_representation(&ts_02, MOSCOW_TZ_NAME, &rr) == DT_OK);
        EXPECT_EQ(rr.year, 2012);
        EXPECT_EQ(rr.month, 12);
        EXPECT_EQ(rr.day, 22);
        EXPECT_EQ(rr.hour, 8);
        EXPECT_EQ(rr.minute, 30);
        EXPECT_EQ(rr.second, 46);
        EXPECT_EQ(rr.nano_second, 23456789L);

        assert(dt_init_interval(DT_SECONDS_PER_DAY, 100L, &o.duration) == DT_OK);
        o.is_forward = DT_FALSE;
        EXPECT_EQ(dt_apply_offset(&ts_01, &o, &ts_02), DT_OK);
        assert(dt_timestamp_to_representation(&ts_02, MOSCOW_TZ_NAME, &rr) == DT_OK);
        EXPECT_EQ(rr.year, 2012);
        EXPECT_EQ(rr.month, 12);
        EXPECT_EQ(rr.day, 20);
        EXPECT_EQ(rr.hour, 8);
        EXPECT_EQ(rr.minute, 30);
        EXPECT_EQ(rr.second, 45);
        EXPECT_EQ(rr.nano_second, 123456689L);

        assert(dt_init_interval(DT_SECONDS_PER_DAY, 900000000L, &o.duration) == DT_OK);
        o.is_forward = DT_FALSE;
        EXPECT_EQ(dt_apply_offset(&ts_01, &o, &ts_02), DT_OK);
        assert(dt_timestamp_to_representation(&ts_02, MOSCOW_TZ_NAME, &rr) == DT_OK);
        EXPECT_EQ(rr.year, 2012);
        EXPECT_EQ(rr.month, 12);
        EXPECT_EQ(rr.day, 20);
        EXPECT_EQ(rr.hour, 8);
        EXPECT_EQ(rr.minute, 30);
        EXPECT_EQ(rr.second, 44);
        EXPECT_EQ(rr.nano_second, 223456789L);
}

TEST_F(DtCase, posix_time_to_and_from_timestamp)
{
        time_t tm;
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
        assert(dt_init_interval(DT_SECONDS_PER_DAY, 0, &o.duration) == DT_OK);
        o.is_forward = DT_FALSE;
        dt_timestamp_t nts;
        assert(dt_apply_offset(&ts, &o, &nts) == DT_OK);
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
        assert(dt_init_interval(1L, 123456789L, &i_01) == DT_OK);
        dt_interval_t i_02;
        assert(dt_init_interval(2L, 123456789L, &i_02) == DT_OK);
        int cr;
        EXPECT_EQ(dt_compare_intervals(NULL, &i_02, &cr), DT_INVALID_ARGUMENT);
        EXPECT_EQ(dt_compare_intervals(&i_01, NULL, &cr), DT_INVALID_ARGUMENT);
        EXPECT_EQ(dt_compare_intervals(&i_01, &i_02, NULL), DT_INVALID_ARGUMENT);
        EXPECT_EQ(dt_compare_intervals(&i_01, &i_02, &cr), DT_OK);
        EXPECT_LT(cr, 0L);
        assert(dt_init_interval(0L, 123456789L, &i_02) == DT_OK);
        EXPECT_EQ(dt_compare_intervals(&i_01, &i_02, &cr), DT_OK);
        EXPECT_GT(cr, 0L);
        assert(dt_init_interval(1L, 123456790L, &i_02) == DT_OK);
        EXPECT_EQ(dt_compare_intervals(&i_01, &i_02, &cr), DT_OK);
        EXPECT_LT(cr, 0L);
        assert(dt_init_interval(1L, 123456788L, &i_02) == DT_OK);
        EXPECT_EQ(dt_compare_intervals(&i_01, &i_02, &cr), DT_OK);
        EXPECT_GT(cr, 0L);
        EXPECT_EQ(dt_compare_intervals(&i_01, &i_01, &cr), DT_OK);
        EXPECT_EQ(cr, 0L);
}

TEST_F(DtCase, sum_intervals)
{
        dt_interval_t i_01;
        assert(dt_init_interval(1L, 123456789L, &i_01) == DT_OK);
        dt_interval_t i_02;
        assert(dt_init_interval(2L, 123456789L, &i_02) == DT_OK);
        dt_interval_t ri;
        EXPECT_EQ(dt_sum_intervals(NULL, &i_02, &ri), DT_INVALID_ARGUMENT);
        EXPECT_EQ(dt_sum_intervals(&i_01, NULL, &ri), DT_INVALID_ARGUMENT);
        EXPECT_EQ(dt_sum_intervals(&i_01, &i_02, NULL), DT_INVALID_ARGUMENT);
        EXPECT_EQ(dt_sum_intervals(&i_01, &i_02, &ri), DT_OK);
        EXPECT_EQ(ri.seconds, 3L);
        EXPECT_EQ(ri.nano_seconds, 246913578L);
        assert(dt_init_interval(2L, 900000000L, &i_02) == DT_OK);
        EXPECT_EQ(dt_sum_intervals(&i_01, &i_02, &ri), DT_OK);
        EXPECT_EQ(ri.seconds, 4L);
        EXPECT_EQ(ri.nano_seconds, 23456789L);
}

TEST_F(DtCase, sub_intervals)
{
        dt_interval_t i_01;
        assert(dt_init_interval(2L, 123456789L, &i_01) == DT_OK);
        dt_interval_t i_02;
        assert(dt_init_interval(3L, 123456789L, &i_02) == DT_OK);
        dt_interval_t ri;
        EXPECT_EQ(dt_sub_intervals(NULL, &i_02, &ri), DT_INVALID_ARGUMENT);
        EXPECT_EQ(dt_sub_intervals(&i_01, NULL, &ri), DT_INVALID_ARGUMENT);
        EXPECT_EQ(dt_sub_intervals(&i_01, &i_02, NULL), DT_INVALID_ARGUMENT);
        EXPECT_EQ(dt_sub_intervals(&i_01, &i_02, &ri), DT_OK);
        EXPECT_EQ(ri.seconds, 0L);
        EXPECT_EQ(ri.nano_seconds, 0L);
        assert(dt_init_interval(1L, 900000000L, &i_02) == DT_OK);
        EXPECT_EQ(dt_sub_intervals(&i_01, &i_02, &ri), DT_OK);
        EXPECT_EQ(ri.seconds, 0L);
        EXPECT_EQ(ri.nano_seconds, 223456789L);
}

TEST_F(DtCase, miltiply_interval)
{
        dt_interval_t i;
        assert(dt_init_interval(3L, 500000000L, &i) == DT_OK);
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

        // Europe/Moscow
        assert(dt_init_representation(2009, 1, 15, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_to_timestamp(&r, MOSCOW_TZ_NAME, &t, NULL), DT_OK);
        EXPECT_EQ(dt_timestamp_to_representation(&t, MOSCOW_TZ_NAME, &rr), DT_OK);
        EXPECT_EQ(memcmp(&r, &rr, sizeof(dt_representation_t)), 0);
        EXPECT_EQ(dt_timestamp_to_representation(&t, UTC_TZ_NAME, &urr), DT_OK);
        EXPECT_EQ(urr.hour, 4);         // TODO: Remove it after history support enabled
        //EXPECT_EQ(urr.hour, 3);         // TODO: Uncomment it - here is a history check

        assert(dt_init_representation(2013, 1, 15, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_to_timestamp(&r, MOSCOW_TZ_NAME, &t, NULL), DT_OK);
        EXPECT_EQ(dt_timestamp_to_representation(&t, MOSCOW_TZ_NAME, &rr), DT_OK);
        EXPECT_EQ(memcmp(&r, &rr, sizeof(dt_representation_t)), 0);
        EXPECT_EQ(dt_timestamp_to_representation(&t, UTC_TZ_NAME, &urr), DT_OK);
        EXPECT_EQ(urr.hour, 4);

        assert(dt_init_representation(2009, 7, 15, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_to_timestamp(&r, MOSCOW_TZ_NAME, &t, NULL), DT_OK);
        EXPECT_EQ(dt_timestamp_to_representation(&t, MOSCOW_TZ_NAME, &rr), DT_OK);
        EXPECT_EQ(memcmp(&r, &rr, sizeof(dt_representation_t)), 0);
        EXPECT_EQ(dt_timestamp_to_representation(&t, UTC_TZ_NAME, &urr), DT_OK);
        EXPECT_EQ(urr.hour, 4);

        assert(dt_init_representation(2013, 7, 15, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_to_timestamp(&r, MOSCOW_TZ_NAME, &t, NULL), DT_OK);
        EXPECT_EQ(dt_timestamp_to_representation(&t, MOSCOW_TZ_NAME, &rr), DT_OK);
        EXPECT_EQ(memcmp(&r, &rr, sizeof(dt_representation_t)), 0);
        EXPECT_EQ(dt_timestamp_to_representation(&t, UTC_TZ_NAME, &urr), DT_OK);
        EXPECT_EQ(urr.hour, 4);

        // Europe/Berlin
        assert(dt_init_representation(2009, 1, 15, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_to_timestamp(&r, BERLIN_TZ_NAME, &t, NULL), DT_OK);
        EXPECT_EQ(dt_timestamp_to_representation(&t, BERLIN_TZ_NAME, &rr), DT_OK);
        EXPECT_EQ(memcmp(&r, &rr, sizeof(dt_representation_t)), 0);
        EXPECT_EQ(dt_timestamp_to_representation(&t, UTC_TZ_NAME, &urr), DT_OK);
        EXPECT_EQ(urr.hour, 7);

        assert(dt_init_representation(2013, 1, 15, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_to_timestamp(&r, BERLIN_TZ_NAME, &t, NULL), DT_OK);
        EXPECT_EQ(dt_timestamp_to_representation(&t, BERLIN_TZ_NAME, &rr), DT_OK);
        EXPECT_EQ(memcmp(&r, &rr, sizeof(dt_representation_t)), 0);
        EXPECT_EQ(dt_timestamp_to_representation(&t, UTC_TZ_NAME, &urr), DT_OK);
        EXPECT_EQ(urr.hour, 7);

        assert(dt_init_representation(2009, 7, 15, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_to_timestamp(&r, BERLIN_TZ_NAME, &t, NULL), DT_OK);
        EXPECT_EQ(dt_timestamp_to_representation(&t, BERLIN_TZ_NAME, &rr), DT_OK);
        EXPECT_EQ(memcmp(&r, &rr, sizeof(dt_representation_t)), 0);
        EXPECT_EQ(dt_timestamp_to_representation(&t, UTC_TZ_NAME, &urr), DT_OK);
        EXPECT_EQ(urr.hour, 6);

        assert(dt_init_representation(2013, 7, 15, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_to_timestamp(&r, BERLIN_TZ_NAME, &t, NULL), DT_OK);
        EXPECT_EQ(dt_timestamp_to_representation(&t, BERLIN_TZ_NAME, &rr), DT_OK);
        EXPECT_EQ(memcmp(&r, &rr, sizeof(dt_representation_t)), 0);
        EXPECT_EQ(dt_timestamp_to_representation(&t, UTC_TZ_NAME, &urr), DT_OK);
        EXPECT_EQ(urr.hour, 6);

        // TODO: Case for current time zone!
}

TEST_F(DtCase, representation_day_of_week)
{
        dt_representation_t r;
        int dow;
        assert(dt_init_representation(2013, 8, 11, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_day_of_week(NULL, &dow), DT_INVALID_ARGUMENT);
        EXPECT_EQ(dt_representation_day_of_week(&r, NULL), DT_INVALID_ARGUMENT);
        EXPECT_EQ(dt_representation_day_of_week(&r, &dow), DT_OK);
        EXPECT_EQ(dow, 1);
        assert(dt_init_representation(2013, 8, 12, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_day_of_week(&r, &dow), DT_OK);
        EXPECT_EQ(dow, 2);
        assert(dt_init_representation(2013, 8, 13, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_day_of_week(&r, &dow), DT_OK);
        EXPECT_EQ(dow, 3);
        assert(dt_init_representation(2013, 8, 14, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_day_of_week(&r, &dow), DT_OK);
        EXPECT_EQ(dow, 4);
        assert(dt_init_representation(2013, 8, 15, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_day_of_week(&r, &dow), DT_OK);
        EXPECT_EQ(dow, 5);
        assert(dt_init_representation(2013, 8, 16, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_day_of_week(&r, &dow), DT_OK);
        EXPECT_EQ(dow, 6);
        assert(dt_init_representation(2013, 8, 17, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_day_of_week(&r, &dow), DT_OK);
        EXPECT_EQ(dow, 7);
        assert(dt_init_representation(2013, 8, 18, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_day_of_week(&r, &dow), DT_OK);
        EXPECT_EQ(dow, 1);
}

TEST_F(DtCase, representation_day_of_year)
{
        dt_representation_t r;
        int doy;

        EXPECT_EQ(dt_representation_day_of_year(NULL, &doy), DT_INVALID_ARGUMENT);
        EXPECT_EQ(dt_representation_day_of_year(&r, NULL), DT_INVALID_ARGUMENT);

        assert(dt_init_representation(2013, 1, 1, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_day_of_year(&r, &doy), DT_OK);
        EXPECT_EQ(doy, 1);
        assert(dt_init_representation(2013, 8, 11, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_day_of_year(&r, &doy), DT_OK);
        EXPECT_EQ(doy, 223);
        assert(dt_init_representation(2013, 12, 31, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_day_of_year(&r, &doy), DT_OK);
        EXPECT_EQ(doy, 365);
        assert(dt_init_representation(2012, 12, 31, 8, 0, 0, 0, &r) == DT_OK);
        EXPECT_EQ(dt_representation_day_of_year(&r, &doy), DT_OK);
        EXPECT_EQ(doy, 366);
}

TEST_F(DtCase, representation_to_tm)
{
        dt_representation_t r;
        struct tm tm;

        EXPECT_EQ(dt_representation_to_tm(NULL, &tm), DT_INVALID_ARGUMENT);
        EXPECT_EQ(dt_representation_to_tm(&r, NULL), DT_INVALID_ARGUMENT);

        assert(dt_init_representation(2012, 12, 21, 8, 30, 45, 123456789, &r) == DT_OK);
        EXPECT_EQ(dt_representation_to_tm(&r, &tm), DT_OK);
        EXPECT_EQ(tm.tm_year, 2012);
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
        tm.tm_year = 2012;
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
