#include <stdio.h>  /* for stdout, stderr, perror */
#include <time.h>   /* for struct tm */
#include <stdlib.h> /* for exit, malloc, atoi */
#include <string.h> /* for strlen, strcpy_s, strcat_s*/
#include <memory.h> /* for malloc, free*/

//for winapi
#include <Windows.h>
#include <WinReg.h>
#include <WinBase.h>
#include <winnt.h>


#define UTC_TZ_NAME "UTC"
#define MOSCOW_TZ_NAME "Russian Standard Time"
#define BERLIN_TZ_NAME "W. Europe Standard Time"
#ifndef EXIT_SUCCESS
    #define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
    #define EXIT_FAILURE -1
#endif

//Registry timezones database path
static const char REG_TIME_ZONES[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\";

//Helper functions prototypes
int GetTimeZoneInformationByName(TIME_ZONE_INFORMATION *ptzi, const char szStandardName[]);
int TmFromSystemTime(const SYSTEMTIME * pTime, struct tm *tm);
int SystemTimeFromTm(SYSTEMTIME *pTime, const struct tm *tm);
int UnixTimeToSystemTime(const time_t *t, LPSYSTEMTIME pst);
int SystemTimeToUnixTime(SYSTEMTIME *systemTime, time_t *dosTime);

typedef struct _REG_TZI_FORMAT {
    LONG Bias;
    LONG StandardBias;
    LONG DaylightBias;
    SYSTEMTIME StandardDate;
    SYSTEMTIME DaylightDate;
} REG_TZI_FORMAT;

int localtime_tz(const time_t *time, const char *tzName, struct tm *result)
{

    DWORD dwError;
    SYSTEMTIME tLocalTime;
    SYSTEMTIME tUniversalTime;
    TIME_ZONE_INFORMATION tzi;

    if (time == NULL || tzName == NULL || result == NULL) {
        return EXIT_FAILURE;
    }

    dwError = GetTimeZoneInformationByName(&tzi, tzName);
    if (dwError != 0) {
        return EXIT_FAILURE;
    }

    if (UnixTimeToSystemTime(time, &tUniversalTime) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    if (SystemTimeToTzSpecificLocalTime(&tzi, &tUniversalTime, &tLocalTime) == FALSE) {
        return EXIT_FAILURE;
    }

    return TmFromSystemTime(&tLocalTime, result);
}

int mktime_tz(const struct tm *tm, const char *tzname, time_t *result)
{
    DWORD dwError;
    TIME_ZONE_INFORMATION tzi;
    SYSTEMTIME tUniversalTime = {0};
    SYSTEMTIME tLocalTime = {0};


    if (tm == NULL || tzname == NULL || result == NULL) {
        return EXIT_FAILURE;
    }
    if (SystemTimeFromTm(&tLocalTime, tm) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    if (strcmp(tzname, "UTC") == 0) {
        return SystemTimeToUnixTime(&tLocalTime, result);
    } else {
        dwError = GetTimeZoneInformationByName(&tzi, tzname);
        if (dwError != 0) {
            return EXIT_FAILURE;
        }

        if (TzSpecificLocalTimeToSystemTime(&tzi, &tLocalTime, &tUniversalTime) == FALSE) {
            return EXIT_FAILURE;
        }

    }

    return SystemTimeToUnixTime(&tUniversalTime, result);
}

int GetTimeZoneInformationByName(TIME_ZONE_INFORMATION *ptzi, const char szStandardName[])
{
    int rc;
    HKEY hkey_tz;
    DWORD dw;
    REG_TZI_FORMAT regtzi;
    size_t subKeySize = strlen(REG_TIME_ZONES) + strlen(szStandardName) + 1;
    char* tszSubkey = (char *)malloc(subKeySize);
    memset(tszSubkey, (int)NULL, subKeySize );
    sprintf_s(tszSubkey, subKeySize, "%s%s", REG_TIME_ZONES, szStandardName);

    if (ERROR_SUCCESS != (dw = RegOpenKeyA(HKEY_LOCAL_MACHINE, tszSubkey, &hkey_tz))) {
        rc = -1;
        goto ennd;
    }

    rc = 0;
    #define X(param, type, var) \
        do if ((dw = sizeof(var)), (ERROR_SUCCESS != (dw = RegGetValueW(hkey_tz, NULL, param, type, NULL, &var, &dw)))) { \
            rc = -1; \
            goto ennd; \
        } while(0)
    X(L"TZI", RRF_RT_REG_BINARY, regtzi);
    X(L"Std", RRF_RT_REG_SZ, ptzi->StandardName);
    X(L"Dlt", RRF_RT_REG_SZ, ptzi->DaylightName);
    #undef X
    ptzi->Bias = regtzi.Bias;
    ptzi->DaylightBias = regtzi.DaylightBias;
    ptzi->DaylightDate = regtzi.DaylightDate;
    ptzi->StandardBias = regtzi.StandardBias;
    ptzi->StandardDate = regtzi.StandardDate;
ennd:
    RegCloseKey(hkey_tz);
    free(tszSubkey);
    return rc;
}

int TmFromSystemTime(const SYSTEMTIME * pTime, struct tm *tm)
{
    if (tm == NULL || pTime == NULL) return EXIT_FAILURE;

    memset(tm, 0, sizeof(tm));

    tm->tm_year = pTime->wYear;
    tm->tm_mon = pTime->wMonth - 1;
    tm->tm_mday = pTime->wDay;
    tm->tm_wday = pTime->wDayOfWeek;

    tm->tm_hour = pTime->wHour;
    tm->tm_min = pTime->wMinute;
    tm->tm_sec = pTime->wSecond;

    return EXIT_SUCCESS;
}

//was gotten from microsoft support
void UnixTimeToFileTime(const time_t *t, LPFILETIME pft) {
    // Note that LONGLONG is a 64-bit value
    UINT64 ll;
    UINT64 time = *t;
    ll = (time * 10000000) + 116444736000000000;
    pft->dwLowDateTime = (DWORD)ll;
    pft->dwHighDateTime = ll >> 32;
}

//was gotten from microsoft support
int UnixTimeToSystemTime(const time_t *t, LPSYSTEMTIME pst) {
    FILETIME ft;

    UnixTimeToFileTime(t, &ft);
    if (FileTimeToSystemTime(&ft, pst) == FALSE) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

//was gotten from microsoft support
int SystemTimeToUnixTime(SYSTEMTIME *systemTime, time_t *dosTime)

{
    LARGE_INTEGER jan1970FT = {0};
    LARGE_INTEGER utcFT = {0};
    UINT64 utcDosTime = 0;
    jan1970FT.QuadPart = 116444736000000000I64; // january 1st 1970


    if (SystemTimeToFileTime(systemTime, (FILETIME*)&utcFT) == FALSE) {
        return EXIT_FAILURE;
    }
     utcDosTime = (utcFT.QuadPart - jan1970FT.QuadPart)/10000000;
    *dosTime = (time_t)utcDosTime;
     return EXIT_SUCCESS;
}


int SystemTimeFromTm(SYSTEMTIME *pTime, const struct tm *tm)
{
    if (tm == NULL || pTime == NULL) return EXIT_FAILURE;

    memset(pTime, 0, sizeof(SYSTEMTIME));

    pTime->wYear = 1900 + tm->tm_year;
    pTime->wMonth = tm->tm_mon + 1;
    pTime->wDay = tm->tm_mday;
    pTime->wDayOfWeek = tm->tm_wday;

    pTime->wHour = tm->tm_hour;
    pTime->wMinute = tm->tm_min;
    pTime->wSecond = tm->tm_sec;

    return EXIT_SUCCESS;
}
