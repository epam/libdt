#include "posixcase.h"
#include <libdt/dt_posix.h>

static const char *unrealTimezone = "notreal/timezone/where/no/light";
static const char *testMoscowTimeZone =
#ifdef _WIN32
    "Russian Standard Time"
#else
    "Europe/Moscow"
#endif
    ;
static const char *testUTCTimeZone =
#ifdef _WIN32
    "UTC"
#else
    "Etc/UTC"
#endif
    ;
static const char *testGMTNeg5TimeZone =
#ifdef _WIN32
    "West Asia Standard Time"
#else
    "Etc/GMT-5"
#endif
    ;
static const char *testGMT5TimeZone =
#ifdef _WIN32
    "SA Pacific Standard Time"
#else
    "Etc/GMT+5"
#endif
    ;
static const char *testBerlinTimeZone =
#ifdef _WIN32
    "W. Europe Standard Time"
#else
    "Europe/Berlin"
#endif
    ;
static const char *testTimeZoneWithoutHistoricalLawInfo =
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

inline void test_tm(const struct tm *tm, const char *tz_name, const struct tm *tmExpected);

PosixCase::PosixCase()
{
}

TEST_F(PosixCase, historic_tz_check)
{
    struct tm tm = {0,};
    struct tm tmUtc = {0,};

    //Most old law rule
    fillTmRepresentationOnly(&tm, 2009, 7, 15, 8, 0, 0);
    fillTmRepresentationOnly(&tmUtc, 2009, 7, 15, 4, 0, 0);
    test_tm(&tm, testMoscowTimeZone, &tmUtc);
    fillTmRepresentationOnly(&tm, 2009, 1, 15, 8, 0, 0);
    fillTmRepresentationOnly(&tmUtc, 2009, 1, 15, 5, 0, 0);
    test_tm(&tm, testMoscowTimeZone, &tmUtc);

    //Law rule in the middle of list
    fillTmRepresentationOnly(&tm, 2011, 7, 15, 8, 0, 0);
    fillTmRepresentationOnly(&tmUtc, 2009, 7, 15, 4, 0, 0);
    test_tm(&tm, testMoscowTimeZone, &tmUtc);
    fillTmRepresentationOnly(&tm, 2011, 1, 15, 8, 0, 0);
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

    fillTmRepresentationOnly(&tm, 2009, 7, 15, 8, 0, 0);
    fillTmRepresentationOnly(&tmUtc, 2009, 7, 15, 6, 0, 0);
    test_tm(&tm, testTimeZoneWithoutHistoricalLawInfo, &tmUtc);
    fillTmRepresentationOnly(&tm, 2009, 1, 15, 8, 0, 0);
    fillTmRepresentationOnly(&tmUtc, 2009, 1, 15, 7, 0, 0);
    test_tm(&tm, testTimeZoneWithoutHistoricalLawInfo, &tmUtc);
    fillTmRepresentationOnly(&tm, 2013, 7, 15, 8, 0, 0);
    fillTmRepresentationOnly(&tmUtc, 2013, 7, 15, 6, 0, 0);
    test_tm(&tm, testTimeZoneWithoutHistoricalLawInfo, &tmUtc);
    fillTmRepresentationOnly(&tm, 2013, 1, 15, 8, 0, 0);
    fillTmRepresentationOnly(&tmUtc, 2013, 1, 15, 7, 0, 0);
    test_tm(&tm, testTimeZoneWithoutHistoricalLawInfo, &tmUtc);
}

TEST_F(PosixCase, localtime_tz)
{
    time_t testTime = 0;
    //Wrong arguments :
    struct tm tm = {0,};
    memset(&tm, 0, sizeof (struct tm));
    EXPECT_EQ(localtime_tz(NULL, testMoscowTimeZone, &tm), (struct tm *)NULL);
    memset(&tm, 0, sizeof (struct tm));
    EXPECT_EQ(localtime_tz(&testTime, NULL, &tm), (struct tm *)NULL);
    EXPECT_EQ(localtime_tz(&testTime, testMoscowTimeZone, NULL), (struct tm *)NULL);
    memset(&tm, 0, sizeof (struct tm));
    EXPECT_EQ(localtime_tz(&testTime, unrealTimezone, &tm), (struct tm *)NULL);
    memset(&tm, 0, sizeof (struct tm));
    testTime = 0;

    //Useful Behaivor :
    //standard timezone encoding behaivor
    struct tm utcTime = {0,};
    struct tm gmt5Time = {0,};
    struct tm gmtNeg5Time = {0,};
    memset(&utcTime, 0, sizeof (struct tm));
    memset(&gmt5Time, 0, sizeof (struct tm));
    memset(&gmtNeg5Time, 0, sizeof (struct tm));
    testTime = 0;
    EXPECT_NE(localtime_tz(&testTime, testUTCTimeZone, &utcTime), (struct tm *)NULL);
    EXPECT_NE(localtime_tz(&testTime, testGMT5TimeZone, &gmt5Time), (struct tm *)NULL);
    EXPECT_NE(localtime_tz(&testTime, testGMTNeg5TimeZone, &gmtNeg5Time), (struct tm *)NULL);
    EXPECT_TRUE(gmt5Time.tm_hour == 19);
    EXPECT_TRUE(gmtNeg5Time.tm_hour == 5);

}

TEST_F(PosixCase, mktime_tz)
{

    time_t testTime = 0;
    //Wrong arguments :
    struct tm tm = {0,};
    memset(&tm, 0, sizeof (struct tm));
    EXPECT_EQ((testTime = mktime_tz(NULL, testMoscowTimeZone)), DT_POSIX_WRONG_TIME);
    memset(&tm, 0, sizeof (struct tm));
    EXPECT_EQ((testTime = mktime_tz(&tm, NULL)), DT_POSIX_WRONG_TIME);
    EXPECT_EQ((testTime = mktime_tz(NULL, testMoscowTimeZone)), DT_POSIX_WRONG_TIME);
    memset(&tm, 0, sizeof (struct tm));
    EXPECT_EQ((testTime = mktime_tz(&tm, unrealTimezone)), DT_POSIX_WRONG_TIME);

    //Useful behaivor :
    struct tm utcTime = {0,};
    fillTm(&utcTime, 0, 0, 0, 1, 0, 0, 0, 4, 0, 1970, 0);// 1 january of 1970 (UTC)
    struct tm gmt5Time = {0,};
    fillTm(&gmt5Time, -18000, 19, 0, 31, 0, 11, 0, 3, 364, 1969, 0);// 31 december of 1969
    struct tm gmtNeg5Time = {0,};
    fillTm(&gmtNeg5Time, 18000, 5, 0, 1, 0, 0, 0, 4, 0, 1970, 0);// 1 january of 1970 (GMT-5)

    testTime = 666; // not a 0
    EXPECT_NE((testTime = mktime_tz(&utcTime, testUTCTimeZone)), DT_POSIX_WRONG_TIME);
    EXPECT_TRUE (testTime == 0);

    testTime = 666; // not a 0
    EXPECT_NE((testTime = mktime_tz(&gmt5Time, testGMT5TimeZone)), DT_POSIX_WRONG_TIME);
    EXPECT_TRUE (testTime == 0);

    testTime = 666; // not a 0
    EXPECT_NE((testTime = mktime_tz(&gmtNeg5Time, testGMTNeg5TimeZone)), DT_POSIX_WRONG_TIME);
    EXPECT_TRUE (testTime == 0);


}

TEST_F(PosixCase, wrongStringConvert)
{
    const char *timeOnly = "08:31";
    const char *timeOnlyWrong = "08:31a";
    const char *timeOnlyFormat = "%H:%M";
    const char *timeOnlyFormatWrong = "H:M";

    struct tm tr = {0,};

    tr.tm_hour = 8;
    tr.tm_min = 612;

    EXPECT_EQ(strptime(timeOnlyWrong, timeOnlyFormat, &tr), timeOnlyWrong + 5);
    EXPECT_EQ(strptime(timeOnly, timeOnlyFormatWrong, &tr), (char *)NULL);


}

TEST_F(PosixCase, fromStringConvert)
{
    const char *timeOnly1 = "08:31";
    const char *timeOnlyFormat1 = "%H:%M";
    const char *timeOnly2 = "11/09/2001 16:54:12";
    const char *timeOnlyFormat2 = "%d/%m/%Y %H:%M:%S";

    struct tm tr1 = {0,};
    struct tm tr2 = {0,};


    EXPECT_NE(strptime(timeOnly1, timeOnlyFormat1, &tr1), (char *)NULL);
    EXPECT_EQ(tr1.tm_hour, 8);
    EXPECT_EQ(tr1.tm_min, 31);


    EXPECT_NE(strptime(timeOnly2, timeOnlyFormat2, &tr2), (char *)NULL);
    EXPECT_EQ(tr2.tm_mday, 11);
    EXPECT_EQ(tr2.tm_mon, 8);
    EXPECT_EQ(tr2.tm_year, 2001 - 1900);
    EXPECT_EQ(tr2.tm_min, 54);
    EXPECT_EQ(tr2.tm_hour, 16);
    EXPECT_EQ(tr2.tm_sec, 12);
}

static void fillTm(struct tm *output, int tm_gmtoff,
                   int tm_hour, int tm_isdst, int tm_mday,
                   int tm_min, int tm_mon, int tm_sec,
                   int tm_wday, int tm_yday, int tm_year,
                   char *tm_zone)
{
    if (output == NULL) {
        return;
    }
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

inline void test_tm(const struct tm *tm, const char *tz_name, const struct tm *tmUtc)
{
    time_t tm_ts;
    struct tm tm_ts_tm;
    struct tm tmUtcToEquasion = {0};
    EXPECT_NE((tm_ts = mktime_tz(tm, tz_name)), DT_POSIX_WRONG_TIME);
    EXPECT_NE(localtime_tz(&tm_ts, tz_name, &tm_ts_tm), (struct tm *)NULL);
    EXPECT_NE(localtime_tz(&tm_ts, testUTCTimeZone, &tmUtcToEquasion), (struct tm *)NULL);

    EXPECT_EQ(tmUtc->tm_hour, tmUtcToEquasion.tm_hour);
}

TEST_F(PosixCase, utc_convert_mktime_tz)
{
    struct tm currentRepresentation = {0,};
    const char *timeZone = "UTC";
    time_t resultTime = 0;

    time_t currentTime = 0;
    time(&currentTime);
    localtime_tz(&currentTime, timeZone, &currentRepresentation);

    resultTime = mktime_tz(&currentRepresentation, timeZone);

    EXPECT_NE(resultTime, 0);
    EXPECT_NE(resultTime, DT_POSIX_WRONG_TIME);
}
