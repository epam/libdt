#include "basiccase.h"
#include "libtz/localtime.h"
static const char* notRealtimezone = "notreal/timezone/where/no/light";
static const char* testMoscowTimeZone = "Europe/Moscow";
static const char* testUTCTimeZone = "Etc/UTC";
static const char* testGMTNeg5TimeZone = "GMT-5";
static const char* testGMT5TimeZone = "GMT+5";
static  time_t testTime = 0;
BasicCase::BasicCase()
{
}

TEST_F(BasicCase, localtime_tz)
{
    //Wrong arguments :
    struct tm tm;
    memset(&tm, 0, sizeof (struct tm));
    EXPECT_TRUE(localtime_tz(NULL, testMoscowTimeZone, &tm) == EXIT_FAILURE);
    memset(&tm, 0, sizeof (struct tm));
    EXPECT_TRUE(localtime_tz(&testTime, notRealtimezone, &tm) == EXIT_FAILURE);
    memset(&tm, 0, sizeof (struct tm));
    EXPECT_TRUE(localtime_tz(&testTime, NULL, &tm) == EXIT_FAILURE);
    EXPECT_TRUE(localtime_tz(&testTime, testMoscowTimeZone, NULL) == EXIT_FAILURE);

    memset(&tm, 0, sizeof (struct tm));

    //Useful Behaivor :
    //standard timezone encoding behaivor
    struct tm utcTime;
    struct tm gmt5Time;
    struct tm gmtNeg5Time;
    memset(&utcTime, 0, sizeof (struct tm));
    memset(&gmt5Time, 0, sizeof (struct tm));
    memset(&gmtNeg5Time, 0, sizeof (struct tm));
    //testTime = time(NULL);
    EXPECT_TRUE(localtime_tz(&testTime, testUTCTimeZone, &utcTime) == EXIT_SUCCES);
    EXPECT_TRUE(localtime_tz(&testTime, testGMT5TimeZone, &gmt5Time) == EXIT_SUCCES);
    EXPECT_TRUE(localtime_tz(&testTime, testGMTNeg5TimeZone, &gmtNeg5Time) == EXIT_SUCCES);
    EXPECT_TRUE(gmt5Time.tm_hour == 19);
    EXPECT_TRUE(gmtNeg5Time.tm_hour == 5);

}
