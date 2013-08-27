#include "basiccase.h"
#include "libtz/timezone.h"

/*
static const char* unrealTimezone = "notreal/timezone/where/no/light";
static const char* testMoscowTimeZone =
        #ifdef _WIN32
        "Russian Standard Time"
        #else
        "Europe/Moscow"
        #endif
        ;
static const char* testUTCTimeZone =
        #ifdef _WIN32
        "UTC"
        #else
        "Etc/UTC"
        #endif
        ;
static const char* testGMTNeg5TimeZone =
        #ifdef _WIN32
        "West Asia Standard Time"
        #else
        "GMT-5"
        #endif
        ;
static const char* testGMT5TimeZone =
        #ifdef _WIN32
        "SA Pacific Standard Time"
        #else
        "GMT+5"
        #endif
        ;
static const char* testBerlinTimeZone =
        #ifdef _WIN32
        "W. Europe Standard Time"
        #else
        "Europe/Berlin"
        #endif
        ;

// Fills tm structure
static void fillTm(struct tm *output, int tm_gmtoff,
                   int tm_hour, int tm_isdst, int tm_mday,
                   int tm_min, int tm_mon, int tm_sec,
                   int tm_wday, int tm_yday, int tm_year,
                   char *tm_zone);


// Fills only representation fields in tm structure
static void fillTmRepresentationOnly(struct tm *output, int tm_year, int tm_mon, int tm_mday,
                int tm_hour, int tm_min, int tm_sec);

static void dump_tm(const struct tm *tm);

void test_tm(const struct tm *tm, const char *tz_name, const struct tm *tmExpected);

BasicCase::BasicCase()
{
}

TEST_F(BasicCase, historic_tz_check)
{
        struct tm tm;
        struct tm tmUtc;

        fillTmRepresentationOnly(&tm, 2009, 7, 15, 8, 0, 0);
        fillTmRepresentationOnly(&tmUtc, 2009, 7, 15, 4, 0, 0);
        test_tm(&tm, testMoscowTimeZone, &tmUtc);
        fillTmRepresentationOnly(&tm, 2009, 1, 15, 8, 0, 0);
        fillTmRepresentationOnly(&tmUtc, 2009, 1, 15, 5, 0, 0);
        test_tm(&tm, testMoscowTimeZone, &tmUtc);
        fillTmRepresentationOnly(&tm, 2013, 7, 15, 8, 0, 0);
        fillTmRepresentationOnly(&tmUtc, 2013, 7, 15, 4, 0, 0);
        test_tm(&tm, testMoscowTimeZone, &tmUtc);
        fillTmRepresentationOnly(&tm, 2013, 1, 15, 8, 0, 0);
        fillTmRepresentationOnly(&tmUtc, 2013, 1, 15, 4, 0, 0);
        test_tm(&tm, testMoscowTimeZone, &tmUtc);


        fillTmRepresentationOnly(&tm, 2009, 7, 15, 8, 0, 0);
        fillTmRepresentationOnly(&tmUtc, 2009, 7, 15, 6, 0, 0);
        test_tm(&tm, testBerlinTimeZone, &tmUtc);
        fillTmRepresentationOnly(&tm, 2009, 1, 15, 8, 0, 0);
        fillTmRepresentationOnly(&tmUtc, 2009, 1, 15, 7, 0, 0);
        test_tm(&tm, testBerlinTimeZone, &tmUtc);
        fillTmRepresentationOnly(&tm, 2013, 7, 15, 8, 0, 0);
        fillTmRepresentationOnly(&tmUtc, 2013, 7, 15, 6, 0, 0);
        test_tm(&tm, testBerlinTimeZone, &tmUtc);
        fillTmRepresentationOnly(&tm, 2013, 1, 15, 8, 0, 0);
        fillTmRepresentationOnly(&tmUtc, 2013, 1, 15, 7, 0, 0);
        test_tm(&tm, testBerlinTimeZone, &tmUtc);
}

TEST_F(BasicCase, localtime_tz)
{
    time_t testTime = 0;
    //Wrong arguments :
    struct tm tm;
    memset(&tm, 0, sizeof (struct tm));
    EXPECT_TRUE(localtime_tz(NULL, testMoscowTimeZone, &tm) == EXIT_FAILURE);
    memset(&tm, 0, sizeof (struct tm));
    EXPECT_TRUE(localtime_tz(&testTime, NULL, &tm) == EXIT_FAILURE);
    EXPECT_TRUE(localtime_tz(&testTime, testMoscowTimeZone, NULL) == EXIT_FAILURE);
    memset(&tm, 0, sizeof (struct tm));
    EXPECT_TRUE(localtime_tz(&testTime, unrealTimezone, &tm) == EXIT_FAILURE);
    memset(&tm, 0, sizeof (struct tm));
    testTime = 0;

    //Useful Behaivor :
    //standard timezone encoding behaivor
    struct tm utcTime;
    struct tm gmt5Time;
    struct tm gmtNeg5Time;
    memset(&utcTime, 0, sizeof (struct tm));
    memset(&gmt5Time, 0, sizeof (struct tm));
    memset(&gmtNeg5Time, 0, sizeof (struct tm));
    testTime = 0;
    EXPECT_TRUE(localtime_tz(&testTime, testUTCTimeZone, &utcTime) == EXIT_SUCCESS);
    EXPECT_TRUE(localtime_tz(&testTime, testGMT5TimeZone, &gmt5Time) == EXIT_SUCCESS);
    EXPECT_TRUE(localtime_tz(&testTime, testGMTNeg5TimeZone, &gmtNeg5Time) == EXIT_SUCCESS);
    EXPECT_TRUE(gmt5Time.tm_hour == 19);
    EXPECT_TRUE(gmtNeg5Time.tm_hour == 5);
    //TODO: add law daylight based test

}

TEST_F(BasicCase, mktime_tz)
{

    time_t testTime = 0;
    //Wrong arguments :
    struct tm tm;
    memset(&tm, 0, sizeof (struct tm));
    EXPECT_TRUE(mktime_tz(NULL, testMoscowTimeZone, &testTime) == EXIT_FAILURE);
    memset(&tm, 0, sizeof (struct tm));
    EXPECT_TRUE(mktime_tz(&tm, NULL, &testTime) == EXIT_FAILURE);
    EXPECT_TRUE(mktime_tz(NULL, testMoscowTimeZone, &testTime) == EXIT_FAILURE);
    memset(&tm, 0, sizeof (struct tm));
    EXPECT_TRUE(mktime_tz(&tm, unrealTimezone, &testTime) == EXIT_FAILURE);

    //Useful behaivor :
    struct tm utcTime;
    fillTm(&utcTime, 0, 0, 0, 1, 0, 0, 0, 4, 0, 1970, 0);// 1 january of 1970 (UTC)
    struct tm gmt5Time;
    fillTm(&gmt5Time, -18000, 19, 0, 31, 0, 11, 0, 3, 364, 1969, 0);// 31 december of 1969
    struct tm gmtNeg5Time;
    fillTm(&gmtNeg5Time, 18000, 5, 0, 1, 0, 0, 0, 4, 0, 1970, 0);// 1 january of 1970 (GMT-5)

    testTime = 666; // not a 0
    EXPECT_TRUE(mktime_tz(&utcTime, testUTCTimeZone, &testTime) == EXIT_SUCCESS);
    EXPECT_TRUE (testTime == 0);

    testTime = 666; // not a 0
    EXPECT_TRUE(mktime_tz(&gmt5Time, testGMT5TimeZone, &testTime) == EXIT_SUCCESS);
    EXPECT_TRUE (testTime == 0);

    testTime = 666; // not a 0
    EXPECT_TRUE(mktime_tz(&gmtNeg5Time, testGMTNeg5TimeZone, &testTime) == EXIT_SUCCESS);
    EXPECT_TRUE (testTime == 0);
    //TODO: add law daylight based test


}


static void fillTm(struct tm *output, int tm_gmtoff,
                   int tm_hour, int tm_isdst, int tm_mday,
                   int tm_min, int tm_mon, int tm_sec,
                   int tm_wday, int tm_yday, int tm_year,
                   char *tm_zone) {
    if (output == NULL) return;
    memset(output, 0, sizeof(struct tm));
#ifdef __USE_BSD
    output->tm_gmtoff = tm_gmtoff;
    output->tm_zone = tm_zone;
#endif
    output->tm_hour = tm_hour;
    output->tm_isdst = tm_isdst;
    output->tm_mday = tm_mday;
    output->tm_min = tm_min;
    output->tm_mon = tm_mon;
    output->tm_sec = tm_sec;
    output->tm_wday = tm_wday;
    output->tm_yday = tm_yday;
    output->tm_year = tm_year - 1900;

}

static void fillTmRepresentationOnly(struct tm *output, int tm_year, int tm_mon, int tm_mday,
                int tm_hour, int tm_min, int tm_sec)
{
        if (output == NULL) {
                return;
        }
        memset(output, 0, sizeof(struct tm));
        output->tm_hour = tm_hour;
        output->tm_mday = tm_mday;
        output->tm_min = tm_min;
        output->tm_mon = tm_mon;
        output->tm_sec = tm_sec;
        output->tm_year = tm_year - 1900;

}

static void dump_tm(const struct tm *tm)
{
        printf("%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
}

void test_tm(const struct tm *tm, const char *tz_name, const struct tm *tmUtc)
{
        time_t tm_ts;
        struct tm tm_ts_tm;
        struct tm tmUtcToEquasion = {0};
        EXPECT_TRUE(mktime_tz(tm, tz_name, &tm_ts) == 0);
        EXPECT_TRUE(localtime_tz(&tm_ts, tz_name, &tm_ts_tm) == 0);
        EXPECT_TRUE(localtime_tz(&tm_ts, testUTCTimeZone, &tmUtcToEquasion) == 0);

        EXPECT_EQ(tmUtc->tm_hour, tmUtcToEquasion.tm_hour);

        printf("Datetime representation '");
        dump_tm(tm);
        printf("' in '%s' timezone is an %ld timestamp, round-up converted representation is '", tz_name, tm_ts);
        dump_tm(&tm_ts_tm);
        printf("', UTC representation is '");
        dump_tm(tmUtc);
        printf("'\n");
}
*/
