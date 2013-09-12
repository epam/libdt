#include "basiccase.h"
#include "libtz/dt.h"

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
        "Etc/GMT-5"
        #endif
        ;
static const char* testGMT5TimeZone =
        #ifdef _WIN32
        "SA Pacific Standard Time"
        #else
        "Etc/GMT+5"
        #endif
        ;
static const char* testBerlinTimeZone =
        #ifdef _WIN32
        "W. Europe Standard Time"
        #else
        "Europe/Berlin"
        #endif
        ;
static const char* testTimeZoneWithoutHistoricalLawInfo =
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

inline void test_tm(const struct tm *tm, const char *tz_name, const struct tm *tmExpected);

BasicCase::BasicCase()
{
}

TEST_F(BasicCase, historic_tz_check)
{
    struct tm tm;
    struct tm tmUtc;

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
    EXPECT_NE(localtime_tz(&testTime, unrealTimezone, &tm), EXIT_SUCCESS);
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

TEST_F(BasicCase, wrongStringConvert)
{
    const char *timeOnly = "08:31";
    const char *timeOnlyWrong = "08:31a";
    const char *timeOnlyFormat = "%H:%M";
    const char *timeOnlyFormatWrong = "H:M";

    struct tm tr = {0,};
    size_t buf_size = 1024;
    char buf[1024] = {0,};

    tr.tm_hour = 8;
    tr.tm_min = 612;
    EXPECT_NE(strptime_tz(NULL, timeOnlyFormat, &tr), EXIT_SUCCESS);
    EXPECT_NE(strptime_tz(timeOnlyWrong, NULL, NULL), EXIT_SUCCESS);
    EXPECT_NE(strptime_tz(timeOnlyWrong, timeOnlyFormat, &tr), EXIT_SUCCESS);

    printf("%d\n", __LINE__);
    return;
    EXPECT_NE(strptime_tz(timeOnly, timeOnlyFormatWrong, &tr), EXIT_SUCCESS);

    printf("%d\n", __LINE__);

    EXPECT_NE(strftime_tz(NULL, testUTCTimeZone, timeOnlyFormatWrong, buf, buf_size), EXIT_SUCCESS);
    EXPECT_NE(strftime_tz(&tr, NULL, timeOnlyFormatWrong, buf, buf_size), EXIT_SUCCESS);
    EXPECT_NE(strftime_tz(&tr, testUTCTimeZone, NULL, buf, buf_size), EXIT_SUCCESS);
    EXPECT_NE(strftime_tz(&tr, testUTCTimeZone, timeOnlyFormatWrong, NULL, buf_size), EXIT_SUCCESS);
    EXPECT_NE(strftime_tz(&tr, testUTCTimeZone, timeOnlyFormatWrong, buf, 0), EXIT_SUCCESS);
}

TEST_F(BasicCase, toStringConvert)
{
    const char *timeOnly1 = "08:31";
    const char *timeOnlyFormat1 = "%H:%M";
    const char *timeOnly2 = "11/09/2001 16:54:12";
    const char *timeOnlyFormat2 = "%d/%m/%Y %H:%M:%S";

    struct tm tr1 = {0,};
    struct tm tr2 = {0,};
    size_t buf_size = 1024;
    char buf[1024] = {0,};

    tr1.tm_hour = 8;
    tr1.tm_min = 31;
    tr1.tm_mday = 1;

    EXPECT_EQ(strftime_tz(&tr1, testUTCTimeZone, timeOnlyFormat1, buf, buf_size), EXIT_SUCCESS);
    EXPECT_STREQ(timeOnly1, buf);

    tr2.tm_mday = 11;
    tr2.tm_mon = 8;
    tr2.tm_year = 2001 - 1900;
    tr2.tm_min = 54;
    tr2.tm_hour = 16;
    tr2.tm_sec = 12;

    EXPECT_EQ(strftime_tz(&tr2, testUTCTimeZone, timeOnlyFormat2, buf, buf_size), EXIT_SUCCESS);
    EXPECT_STREQ(timeOnly2, buf);
}

TEST_F(BasicCase, fromStringConvert)
{
    const char *timeOnly1 = "08:31";
    const char *timeOnlyFormat1 = "%H:%M";
    const char *timeOnly2 = "11/09/2001 16:54:12";
    const char *timeOnlyFormat2 = "%d/%m/%Y %H:%M:%S";

    struct tm tr1 = {0,};
    struct tm tr2 = {0,};


    EXPECT_EQ(strptime_tz(timeOnly1, timeOnlyFormat1, &tr1), EXIT_SUCCESS);
    EXPECT_EQ(tr1.tm_hour, 8);
    EXPECT_EQ(tr1.tm_min, 31);


    EXPECT_EQ(strptime_tz(timeOnly2, timeOnlyFormat2, &tr2), EXIT_SUCCESS);
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

inline void test_tm(const struct tm *tm, const char *tz_name, const struct tm *tmUtc)
{
    time_t tm_ts;
    struct tm tm_ts_tm;
    struct tm tmUtcToEquasion = {0};
    EXPECT_TRUE(mktime_tz(tm, tz_name, &tm_ts) == 0);
    EXPECT_TRUE(localtime_tz(&tm_ts, tz_name, &tm_ts_tm) == 0);
    EXPECT_TRUE(localtime_tz(&tm_ts, testUTCTimeZone, &tmUtcToEquasion) == 0);

    EXPECT_EQ(tmUtc->tm_hour, tmUtcToEquasion.tm_hour);
}

TEST_F(BasicCase, utc_convert_mktime_tz)
{
    struct tm currentRepresentation = {0,};
    const char *timeZone = "UTC";
    time_t resultTime = 0;

    time_t currentTime = 0;
    time(&currentTime);
    localtime_tz(&currentTime, timeZone, &currentRepresentation);
    
    mktime_tz(&currentRepresentation, timeZone, &resultTime);

    EXPECT_NE(resultTime, 0);
}
