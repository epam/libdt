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

//interface
#include "libtz/localtime.h"

//Registry timezones database path
static const char REG_TIME_ZONES[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\";
int GetTimeZoneInformationByName(TIME_ZONE_INFORMATION *ptzi, const char szStandardName[]);
int TimeFromSystemTime(const SYSTEMTIME * pTime, struct tm *tm);
void UnixTimeToSystemTime(const time_t *t, LPSYSTEMTIME pst);

typedef struct _REG_TZI_FORMAT {
    LONG Bias;
    LONG StandardBias;
    LONG DaylightBias;
    SYSTEMTIME StandardDate;
    SYSTEMTIME DaylightDate;
} REG_TZI_FORMAT;

int localtime_tz(const time_t *time, const char *tzName, struct tm *result) {

    DWORD dw;
    SYSTEMTIME ;
    SYSTEMTIME tLocalTime;
    SYSTEMTIME tUniversalTime;
    TIME_ZONE_INFORMATION tzi;

    if(tzName == NULL || result == NULL) {
        return EXIT_FAILURE;
    }

    dw = GetTimeZoneInformationByName(&tzi, tzName);
    if (dw != 0) {
        return EXIT_FAILURE;
    }

    UnixTimeToSystemTime(time, &tUniversalTime);
    SystemTimeToTzSpecificLocalTime(&tzi, &tUniversalTime, &tLocalTime);

    return TimeFromSystemTime(&tLocalTime, result);
}

int mktime_tz(const struct tm *tm, const char *tzname, time_t *result) {
    return EXIT_FAILURE;
}

int GetTimeZoneInformationByName(TIME_ZONE_INFORMATION *ptzi, const char szStandardName[]) {
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

int TimeFromSystemTime(const SYSTEMTIME * pTime, struct tm *tm)
{
    if(tm == NULL) return EXIT_FAILURE;

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
    INT64 ll;
    ll = Int32x32To64(*t, 10000000) + 116444736000000000;
    pft->dwLowDateTime = (DWORD)ll;
    pft->dwHighDateTime = ll >> 32;
}

//was gotten from microsoft support
void UnixTimeToSystemTime(const time_t *t, LPSYSTEMTIME pst) {
    FILETIME ft;

    UnixTimeToFileTime(t, &ft);
    FileTimeToSystemTime(&ft, pst);
}
