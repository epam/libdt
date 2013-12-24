// vim: shiftwidth=4 softtabstop=4

#include "performancecase.h"

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

#define MOSCOW_TZ_NAME MOSCOW_WINDOWS_STANDARD_TZ_NAME

#include <windows.h>
#define sleep(x) Sleep(x*1000)

#else

#define MOSCOW_TZ_NAME MOSCOW_OLSEN_TZ_NAME

#endif

#define MAX_NANOSECONDS  999999999UL

PerformanceCase::PerformanceCase()
{
}

TEST_F(PerformanceCase, performance_dt_representation_to_timestamp_test)
{
    dt_representation_t r = {0,};
    dt_timestamp_t t = {0,};
    dt_timezone_t tz_moscow = {0,};

    dt_timestamp_t t_start = {0,};
    dt_timestamp_t t_stop = {0,};
    dt_offset_t t_duration = {0,};
    const long operations_count = 10000;
    //Lookup timezones
    EXPECT_EQ(dt_timezone_lookup(MOSCOW_TZ_NAME, &tz_moscow), DT_OK);

    dt_init_representation(2008, 3, 30, 1, 30, 0, 0, &r);
    // before switch time...
    dt_now(&t_start);

    for (int i = 0; i < operations_count; i++) {
        dt_representation_to_timestamp(&r, &tz_moscow, &t, NULL);
    }

    dt_now(&t_stop);
    dt_offset_between(&t_start, &t_stop, &t_duration);
    double nanosec_per_operation = ((t_duration.duration.seconds * 1000 * 1000 * 1000) + t_duration.duration.nano_seconds);
    nanosec_per_operation /= operations_count;
    std::cout << "duration=" << nanosec_per_operation << std::endl;
    EXPECT_LT(nanosec_per_operation, 100);

    dt_timezone_cleanup(&tz_moscow);
}

TEST_F(PerformanceCase, performance_dt_timestamp_to_representation_test)
{
    dt_representation_t r = {0,};
    dt_timestamp_t t = {0,};
    dt_representation_t result = {0,};
    dt_timezone_t tz_moscow = {0,};

    dt_timestamp_t t_start = {0,};
    dt_timestamp_t t_stop = {0,};
    dt_offset_t t_duration = {0,};
    const long operations_count = 10000;
    //Lookup timezones
    EXPECT_EQ(dt_timezone_lookup(MOSCOW_TZ_NAME, &tz_moscow), DT_OK);

    dt_init_representation(2008, 3, 30, 1, 30, 0, 0, &r);
    // before switch time...
    dt_now(&t_start);

    for (int i = 0; i < operations_count; i++) {
        dt_timestamp_to_representation(&t, &tz_moscow, &result);
    }

    dt_now(&t_stop);
    dt_offset_between(&t_start, &t_stop, &t_duration);
    double nanosec_per_operation = ((t_duration.duration.seconds * 1000 * 1000 * 1000) + t_duration.duration.nano_seconds);
    nanosec_per_operation /= operations_count;
    std::cout << "duration=" << nanosec_per_operation << std::endl;
    EXPECT_LT(nanosec_per_operation, 100);

    dt_timezone_cleanup(&tz_moscow);
}

TEST_F(PerformanceCase, performance_std_mktime_test)
{
    dt_representation_t r = {0,};
    dt_timestamp_t t = {0,};
    dt_timezone_t tz_moscow = {0,};

    dt_timestamp_t t_start = {0,};
    dt_timestamp_t t_stop = {0,};
    dt_offset_t t_duration = {0,};
    const long operations_count = 10000;
    //Lookup timezones
    EXPECT_EQ(dt_timezone_lookup(MOSCOW_TZ_NAME, &tz_moscow), DT_OK);

    dt_init_representation(2008, 3, 30, 1, 30, 0, 0, &r);
    struct tm tm = {0,};
    dt_representation_to_tm(&r, &tm);
    // before switch time...
    dt_now(&t_start);

    for (int i = 0; i < operations_count; i++) {
        mktime(&tm);
    }

    dt_now(&t_stop);
    dt_offset_between(&t_start, &t_stop, &t_duration);
    double nanosec_per_operation = ((t_duration.duration.seconds * 1000 * 1000 * 1000) + t_duration.duration.nano_seconds);
    nanosec_per_operation /= operations_count;
    std::cout << "duration=" << nanosec_per_operation << std::endl;

    dt_timezone_cleanup(&tz_moscow);
}

TEST_F(PerformanceCase, performance_std_gmtime_test)
{
    dt_representation_t r = {0,};
    dt_timezone_t tz_moscow = {0,};

    dt_timestamp_t t_start = {0,};
    dt_timestamp_t t_stop = {0,};
    dt_offset_t t_duration = {0,};
    const long operations_count = 10000;
    struct tm tm = {0,};
    time_t t = time(NULL);

    //Lookup timezones
    EXPECT_EQ(dt_timezone_lookup(MOSCOW_TZ_NAME, &tz_moscow), DT_OK);

    dt_init_representation(2008, 3, 30, 1, 30, 0, 0, &r);
    dt_representation_to_tm(&r, &tm);
    // before switch time...
    dt_now(&t_start);

    for (int i = 0; i < operations_count; i++) {
        gmtime_r(&t, &tm);
    }

    dt_now(&t_stop);
    dt_offset_between(&t_start, &t_stop, &t_duration);
    double nanosec_per_operation = ((t_duration.duration.seconds * 1000 * 1000 * 1000) + t_duration.duration.nano_seconds);
    nanosec_per_operation /= operations_count;
    std::cout << "duration=" << nanosec_per_operation << std::endl;

    dt_timezone_cleanup(&tz_moscow);
}



