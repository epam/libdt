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

#include <stdio.h>  /* for stdout, stderr, perror */
#include <time.h>   /* for struct tm */
#include <stdlib.h> /* for exit, malloc, atoi */
#include <string.h> /* for strlen, strcpy_s, strcat_s*/
#include <memory.h> /* for malloc, free*/

#include <libdt/dt.h>
#include <libdt/dt_posix.h>
#include "../tzmapping.h"

// WinAPI
#include <windows.h>
#include <winreg.h>
#include <winbase.h>
#include <winnt.h>


// Registry timezones database path
static const char REG_TIME_ZONES[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\";
static const char DYNAMIC_DST[] = "Dynamic DST";
static const DWORD YEAR_WRONG_VALUE = 0xFFFFFFFF; // any way wrong value for a year
static const size_t YEARS_ARRAY_SEED = 10; // seed which setups how often will be memeory reallocation given in years array
static const char DYNAMIC_DST_FIRST_ENTRY[] = "FirstEntry";
static const char DYNAMIC_DST_LAST_ENTRY[] = "LastEntry";
static const LPSTR REG_TZI = "TZI";
static const LPWSTR REG_DLT = L"Dlt";
static const LPWSTR REG_STD = L"Std";

#if ( (defined(_WIN32) || defined(WIN32) ) && ( defined(_MSC_VER) ) )
#define snprintf sprintf_s
#endif

#if __GNUC__
#include <wchar.h>
WINBASEAPI BOOL WINAPI TzSpecificLocalTimeToSystemTime(LPTIME_ZONE_INFORMATION, LPSYSTEMTIME, LPSYSTEMTIME);
//
// RRF - Registry Routine Flags (for RegGetValue)
//

#define RRF_RT_REG_NONE        0x00000001  // restrict type to REG_NONE      (other data types will not return ERROR_SUCCESS)
#define RRF_RT_REG_SZ          0x00000002  // restrict type to REG_SZ        (other data types will not return ERROR_SUCCESS) (automatically converts REG_EXPAND_SZ to REG_SZ unless RRF_NOEXPAND is specified)
#define RRF_RT_REG_EXPAND_SZ   0x00000004  // restrict type to REG_EXPAND_SZ (other data types will not return ERROR_SUCCESS) (must specify RRF_NOEXPAND or RegGetValue will fail with ERROR_INVALID_PARAMETER)
#define RRF_RT_REG_BINARY      0x00000008  // restrict type to REG_BINARY    (other data types will not return ERROR_SUCCESS)
#define RRF_RT_REG_DWORD       0x00000010  // restrict type to REG_DWORD     (other data types will not return ERROR_SUCCESS)
#define RRF_RT_REG_MULTI_SZ    0x00000020  // restrict type to REG_MULTI_SZ  (other data types will not return ERROR_SUCCESS)
#define RRF_RT_REG_QWORD       0x00000040  // restrict type to REG_QWORD     (other data types will not return ERROR_SUCCESS)

#define RRF_RT_DWORD           (RRF_RT_REG_BINARY | RRF_RT_REG_DWORD) // restrict type to *32-bit* RRF_RT_REG_BINARY or RRF_RT_REG_DWORD (other data types will not return ERROR_SUCCESS)
#define RRF_RT_QWORD           (RRF_RT_REG_BINARY | RRF_RT_REG_QWORD) // restrict type to *64-bit* RRF_RT_REG_BINARY or RRF_RT_REG_DWORD (other data types will not return ERROR_SUCCESS)
#define RRF_RT_ANY             0x0000ffff                             // no type restriction

#define RRF_NOEXPAND           0x10000000  // do not automatically expand environment strings if value is of type REG_EXPAND_SZ
#define RRF_ZEROONFAILURE      0x20000000  // if pvData is not NULL, set content to all zeros on failure

typedef struct _TIME_DYNAMIC_ZONE_INFORMATION {
    LONG Bias;
    WCHAR StandardName[ 32 ];
    SYSTEMTIME StandardDate;
    LONG StandardBias;
    WCHAR DaylightName[ 32 ];
    SYSTEMTIME DaylightDate;
    LONG DaylightBias;
    WCHAR TimeZoneKeyName[ 128 ];
    BOOLEAN DynamicDaylightTimeDisabled;
} DYNAMIC_TIME_ZONE_INFORMATION, *PDYNAMIC_TIME_ZONE_INFORMATION;
#define sscanf_s sscanf
#if !defined(_WSTRING_S_DEFINED)
wchar_t *wcscpy_s (wchar_t *dest, size_t size, const wchar_t *source)
{
    (void *)&size;
    return wcscpy(dest, source);
}
#endif
void qsort_s(void *base, size_t length, size_t size,
             int (*compare)(const void *, const void *), void *context)
{
    (void *)context;
    qsort(base, length, size, compare);
}


static int years_compare(const void *year1, const void *year2)
{
    const DWORD *y1 = year1;
    const DWORD *y2 = year2;
    if (*y1 < *y2) {
        return -1;
    } else if (*y1 > *y2) {
        return 1;
    } else {
        return 0;
    }
}

#else
static int years_compare(void *contex, const void *year1, const void *year2)
{
    const DWORD *y1 = year1;
    const DWORD *y2 = year2;
    (void *)contex;

    if (*y1 < *y2) {
        return -1;
    } else if (*y1 > *y2) {
        return 1;
    } else {
        return 0;
    }
}
#endif

//GetTimeZoneInformationForYear implementation for windows server 2003 and lower
static
BOOL
WINAPI
GetTimeZoneInformationForYearLower(
    /*__in*/ USHORT wYear,
    const dt_timezone_t *timezone,
    /*__in_opt PDYNAMIC_TIME_ZONE_INFORMATION pdtzi,*/
    /*__out*/ LPTIME_ZONE_INFORMATION ptzi
);
//Helper functions prototypes
static int GetTimeZoneInformationByName(DYNAMIC_TIME_ZONE_INFORMATION *ptzi, const char szStandardName[]);
static int TmFromSystemTime(const SYSTEMTIME *pTime, struct tm *tm);
static int SystemTimeFromTm(SYSTEMTIME *pTime, const struct tm *tm);
static int UnixTimeToSystemTime(const time_t *t, LPSYSTEMTIME pst);
static int SystemTimeToUnixTime(SYSTEMTIME *systemTime, time_t *dosTime);
//Tests is current windows version suitable to given version parts
static BOOL IsSuitableWindowsVersion(DWORD dwMajor, DWORD dwMinor);
char *libdt_strptime(const char *buf, const char *fmt, struct tm *tm);

typedef struct _YEARS_ARRAY {
    size_t count;// size in fact
    size_t size;// include reserved elements
    DWORD *years;
} YEARS_ARRAY;

typedef struct _REG_TZI_FORMAT {
    LONG Bias;
    LONG StandardBias;
    LONG DaylightBias;
    SYSTEMTIME StandardDate;
    SYSTEMTIME DaylightDate;
} REG_TZI_FORMAT;

static dt_status_t dt_filetime_to_timestamp(const PFILETIME ft, dt_timestamp_t *ts)
{
    LARGE_INTEGER li;

    if (!ft || !ts) {
        return DT_INVALID_ARGUMENT;
    }

    li.QuadPart = ft->dwHighDateTime;
    li.QuadPart <<= 32;
    li.QuadPart |= ft->dwLowDateTime;
    ts->second = (long)(li.QuadPart / 10000000);
    ts->nano_second = li.QuadPart % 10000000 * 100;
    return DT_OK;
}

static dt_status_t dt_timestamp_to_filetime(const dt_timestamp_t *ts, PFILETIME ft)
{
    LARGE_INTEGER li;

    if (!ts || !ft) {
        return DT_INVALID_ARGUMENT;
    }

    li.QuadPart = ts->second * 10000000;
    li.QuadPart += ts->nano_second / 100;
    ft->dwLowDateTime = (long)li.QuadPart;
    li.QuadPart >>= 32;
    ft->dwHighDateTime = (long)li.QuadPart;
    return DT_OK;
}

dt_status_t dt_now(dt_timestamp_t *result)
{
    FILETIME ft = {0};
    dt_status_t s;

    if (!result) {
        return DT_INVALID_ARGUMENT;
    }

    GetSystemTimeAsFileTime(&ft);
    s = dt_filetime_to_timestamp(&ft, result);
    return s;
}

dt_status_t dt_posix_time_to_timestamp(time_t time, unsigned long nano_second, dt_timestamp_t *result)
{
    if (!result || time == -1) {
        return DT_INVALID_ARGUMENT;
    }

    result->second = (long)time;
    result->nano_second = nano_second;
    return DT_OK;
}


dt_status_t dt_timestamp_to_representation(const dt_timestamp_t *timestamp, const dt_timezone_t *timezone, dt_representation_t *representation)
{
    DWORD dwError = -1;
    SYSTEMTIME tLocalTime = {0};
    SYSTEMTIME tUniversalTime = {0};
    TIME_ZONE_INFORMATION tzi = {0};
    time_t time = 0;
    unsigned long nano = 0;
    struct tm result = {0};
    dt_status_t status = DT_UNKNOWN_ERROR;

    if (timestamp == NULL || representation == NULL) {
        return DT_INVALID_ARGUMENT;
    }


    if (timezone == NULL ) {
        dwError = GetTimeZoneInformation(&tzi);
        if (dwError != 0) {
            return DT_TIMEZONE_NOT_FOUND;
        }
    } else if (timezone->dtzi == NULL) {
        return DT_INVALID_ARGUMENT;
    }


    status = dt_timestamp_to_posix_time(timestamp, &time, &nano);
    if (status != DT_OK) {
        return status;
    }

    if (UnixTimeToSystemTime(&time, &tUniversalTime)) {
        //return DT_CONVERT_ERROR;
        return DT_INVALID_ARGUMENT; // TODO: Maybe to return a DT_SYSTEM_CALL_ERROR?
    }

    if (timezone != NULL && (GetTimeZoneInformationForYearLower(tUniversalTime.wYear, timezone, &tzi) == FALSE)) {
        //return DT_CONVERT_ERROR;
        return DT_INVALID_ARGUMENT; // TODO: Maybe to return a DT_SYSTEM_CALL_ERROR?
    }

    if (SystemTimeToTzSpecificLocalTime(&tzi, &tUniversalTime, &tLocalTime) == FALSE) {
        //return DT_CONVERT_ERROR;
        return DT_INVALID_ARGUMENT; // TODO: Maybe to return a DT_SYSTEM_CALL_ERROR?
    }

    if (TmFromSystemTime(&tLocalTime, &result) != EXIT_SUCCESS) {
        //return DT_CONVERT_ERROR;
        return DT_INVALID_ARGUMENT; // TODO: Maybe to return a DT_SYSTEM_CALL_ERROR?
    }

    status = dt_tm_to_representation(&result, nano, representation);
    if (status != DT_OK) {
        return status;
    }

    return DT_OK;
}

dt_status_t dt_representation_to_timestamp(const dt_representation_t *representation, const dt_timezone_t *timezone,
                                           dt_timestamp_t *first_timestamp, dt_timestamp_t *second_timestamp)
{
    DWORD dwError;
    TIME_ZONE_INFORMATION tzi;
    SYSTEMTIME tUniversalTime = {0};
    SYSTEMTIME tLocalTime = {0};
    time_t time = 0;
    unsigned long nano = 0;
    struct tm tm = {0};
    dt_status_t status = DT_UNKNOWN_ERROR;

    if (!representation || !first_timestamp) {
        return DT_INVALID_ARGUMENT;
    }

    if (timezone == NULL ) {
        dwError = GetTimeZoneInformation(&tzi);
        if (dwError != 0) {
            return DT_TIMEZONE_NOT_FOUND;
        }
    } else if (timezone->dtzi == NULL) {
        return DT_INVALID_ARGUMENT;
    }

    nano = representation->nano_second;
    status = dt_representation_to_tm(representation, &tm);
    if (status != DT_OK) {
        return status;
    }

    if (SystemTimeFromTm(&tLocalTime, &tm) != EXIT_SUCCESS) {
        //return DT_CONVERT_ERROR;
        return DT_INVALID_ARGUMENT; // TODO: Maybe to return a DT_SYSTEM_CALL_ERROR?
    }

    if (timezone != NULL && (GetTimeZoneInformationForYearLower(tLocalTime.wYear, timezone, &tzi) == FALSE)) {
        return DT_TIMEZONE_NOT_FOUND;
    }


    if (TzSpecificLocalTimeToSystemTime((LPTIME_ZONE_INFORMATION)&tzi, (LPSYSTEMTIME)&tLocalTime, (LPSYSTEMTIME)&tUniversalTime) == FALSE) {
        //return DT_CONVERT_ERROR;
        return DT_INVALID_ARGUMENT; // TODO: Maybe to return a DT_SYSTEM_CALL_ERROR?
    }

    if (SystemTimeToUnixTime(&tUniversalTime, &time) != EXIT_SUCCESS) {
        //return DT_CONVERT_ERROR;
        return DT_INVALID_ARGUMENT; // TODO: Maybe to return a DT_SYSTEM_CALL_ERROR?
    }

    status = dt_posix_time_to_timestamp(time, nano, first_timestamp);
    if (status != DT_OK) {
        return status;
    }

    return DT_OK;
}

static int GetTziFromKey(const char szKey[], const char szValue[], REG_TZI_FORMAT *ptzi)
{
    int rc = EXIT_FAILURE;
    HKEY hkey_tz = NULL;
    DWORD dw = sizeof(REG_TZI_FORMAT);
    if (ptzi == NULL || szKey == NULL || szValue == NULL) {
        return EXIT_FAILURE;
    }

    memset(ptzi, 0, sizeof(REG_TZI_FORMAT));

    if (ERROR_SUCCESS !=  RegOpenKeyA(HKEY_LOCAL_MACHINE, szKey, &hkey_tz)) {
        return EXIT_FAILURE;
    }

    if (ERROR_SUCCESS != RegQueryValueExA(hkey_tz, szValue,
                                          NULL, NULL, (LPBYTE)ptzi, &dw)) {
        rc = EXIT_FAILURE;
    } else {
        rc = EXIT_SUCCESS;
    }

    RegCloseKey(hkey_tz);
    return rc;
}


static void RegTziToDynamicTimeZoneInfo(REG_TZI_FORMAT *regtzi, DYNAMIC_TIME_ZONE_INFORMATION *ptzi)
{
    ptzi->Bias = regtzi->Bias;
    ptzi->DaylightBias = regtzi->DaylightBias;
    ptzi->DaylightDate = regtzi->DaylightDate;
    ptzi->StandardBias = regtzi->StandardBias;
    ptzi->StandardDate = regtzi->StandardDate;

}

static int getTZIfromRegistry(char *tszSubkey, char *tszKey, DYNAMIC_TIME_ZONE_INFORMATION *ptzi, const char szStandardName[])
{
    HKEY hkey_tz = NULL;
    DWORD dw = 0;
    REG_TZI_FORMAT regtzi = {0,};

    if (ERROR_SUCCESS != (dw = RegOpenKeyA(HKEY_LOCAL_MACHINE, tszSubkey, &hkey_tz))) {

        RegCloseKey(hkey_tz);
        free(tszSubkey);
        return EXIT_FAILURE;
    }

    GetTziFromKey(tszSubkey, tszKey, &regtzi);

    dw = sizeof(ptzi->StandardName);
    if (ERROR_SUCCESS != (dw = RegQueryValueExW(hkey_tz, REG_STD, NULL, NULL, (LPBYTE)&ptzi->StandardName, &dw))) {
        RegCloseKey(hkey_tz);
        free(tszSubkey);
        return EXIT_FAILURE;
    }

    dw = sizeof(ptzi->StandardName);
    if (ERROR_SUCCESS != (dw = RegQueryValueExW(hkey_tz, REG_DLT, NULL, NULL, (LPBYTE)&ptzi->DaylightName, &dw))) {
        RegCloseKey(hkey_tz);
        free(tszSubkey);
        return EXIT_FAILURE;
    }

    RegTziToDynamicTimeZoneInfo(&regtzi, ptzi);
    MultiByteToWideChar(CP_ACP, MB_COMPOSITE, szStandardName, -1 , ptzi->TimeZoneKeyName, sizeof(ptzi->TimeZoneKeyName));

    RegCloseKey(hkey_tz);

    return EXIT_SUCCESS;
}

static int GetTimeZoneInformationByName(DYNAMIC_TIME_ZONE_INFORMATION *ptzi, const char szStandardName[])
{
    size_t subKeySize = 0;
    char *tszSubkey = NULL;

    if (ptzi == NULL || szStandardName == NULL) {
        return EXIT_FAILURE;
    }

    subKeySize = strlen(REG_TIME_ZONES) + strlen(szStandardName) + 1;
    tszSubkey = (char *)malloc(subKeySize);

    memset(tszSubkey, 0, subKeySize );
    memset(ptzi, 0, sizeof(DYNAMIC_TIME_ZONE_INFORMATION));

    snprintf(tszSubkey, subKeySize, "%s%s", REG_TIME_ZONES, szStandardName);

    if (getTZIfromRegistry(tszSubkey, REG_TZI, ptzi, szStandardName) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    free(tszSubkey);
    return EXIT_SUCCESS;
}

static int TmFromSystemTime(const SYSTEMTIME *pTime, struct tm *tm)
{
    if (tm == NULL || pTime == NULL) {
        return EXIT_FAILURE;
    }

    memset(tm, 0, sizeof(tm));

    tm->tm_year = pTime->wYear - 1900;
    tm->tm_mon = pTime->wMonth - 1;
    tm->tm_mday = pTime->wDay;
    tm->tm_wday = pTime->wDayOfWeek;

    tm->tm_hour = pTime->wHour;
    tm->tm_min = pTime->wMinute;
    tm->tm_sec = pTime->wSecond;

    return EXIT_SUCCESS;
}

//was gotten from microsoft support
static void UnixTimeToFileTime(const time_t *t, LPFILETIME pft)
{
    // Note that LONGLONG is a 64-bit value
    UINT64 ll;
    UINT64 time = *t;
    ll = (time * 10000000) + 116444736000000000;
    pft->dwLowDateTime = (DWORD)ll;
    pft->dwHighDateTime = ll >> 32;
}

//was gotten from microsoft support
static int UnixTimeToSystemTime(const time_t *t, LPSYSTEMTIME pst)
{
    FILETIME ft;

    UnixTimeToFileTime(t, &ft);
    if (FileTimeToSystemTime(&ft, pst) == FALSE) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

//was gotten from microsoft support
static int SystemTimeToUnixTime(SYSTEMTIME *systemTime, time_t *dosTime)
{
    LARGE_INTEGER jan1970FT = {0};
    LARGE_INTEGER utcFT = {0};
    UINT64 utcDosTime = 0;
    jan1970FT.QuadPart = 116444736000000000; // january 1st 1970


    if (SystemTimeToFileTime(systemTime, (FILETIME *)&utcFT) == FALSE) {
        return EXIT_FAILURE;
    }
    utcDosTime = (utcFT.QuadPart - jan1970FT.QuadPart) / 10000000;
    *dosTime = (time_t)utcDosTime;
    return EXIT_SUCCESS;
}


static int SystemTimeFromTm(SYSTEMTIME *pTime, const struct tm *tm)
{
    if (tm == NULL || pTime == NULL) {
        return EXIT_FAILURE;
    }

    memset(pTime, 0, sizeof(SYSTEMTIME));

    pTime->wYear = tm->tm_year + 1900;
    pTime->wMonth = tm->tm_mon + 1;
    pTime->wDay = tm->tm_mday;
    pTime->wDayOfWeek = tm->tm_wday;

    pTime->wHour = tm->tm_hour;
    pTime->wMinute = tm->tm_min;
    pTime->wSecond = tm->tm_sec;

    return EXIT_SUCCESS;
}

static BOOL IsSuitableWindowsVersion(DWORD dwMajor, DWORD dwMinor)
{
    DWORD dwVersion = 0;
    DWORD dwMajorVersion = 0;
    DWORD dwMinorVersion = 0;

    dwVersion = GetVersion();
    // Get the Windows version.
    dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
    dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));

    if (dwMajorVersion >= dwMajor && dwMinorVersion >= dwMinor) {
        return TRUE;
    }

    return FALSE;
}


//Inserts year to array of years at given index, in each YEARS_ARRAY_SEED years array will be realocated
//for cover more suitable case of usual years layout
//all not initialized years will be set to YEAR_WRONG_VALUE
static int InsertYearToArray(DWORD year, YEARS_ARRAY *array)
{
    size_t i = 0;
    DWORD index = array->count;

    if (array == 0) {
        return EXIT_FAILURE;
    }

    for (i = 0; i < array->count; i++) {
        if (array->years[i] == year) {
            return EXIT_FAILURE;
        }
    }

    while (index >= array->size) {
        array->years = realloc(array->years, (array->size + YEARS_ARRAY_SEED) * sizeof(DWORD));

        if (array->years == NULL) {
            return EXIT_FAILURE;
        }
        for (i = array->size; i < array->size + YEARS_ARRAY_SEED; i++) {
            array->years[i] = YEAR_WRONG_VALUE;
        }
        array->size += YEARS_ARRAY_SEED;
    }
    array->count++;

    array->years[index] = year;
    return EXIT_SUCCESS;
}

typedef struct dt_tz_data {
    DYNAMIC_TIME_ZONE_INFORMATION data;
    int year;
} dt_tz_data_t;

static
BOOL
WINAPI
GetTimeZoneInformationForYearLower(
    /*__in*/ USHORT wYear,
    const dt_timezone_t *timezone,
    /*__out*/ LPTIME_ZONE_INFORMATION ptzi
)
{
    dt_tz_data_t *reg_tz_data = timezone->reg_tz_data;
    size_t size = timezone->reg_tz_data_size;
    size_t i = 0;

    if (wYear < 1601 || ptzi == NULL || timezone == NULL) {
        return FALSE;
    }

    wcscpy_s(ptzi->StandardName, sizeof(ptzi->StandardName) / sizeof(WCHAR), timezone->dtzi->StandardName);
    wcscpy_s(ptzi->DaylightName, sizeof(ptzi->DaylightName) / sizeof(WCHAR), timezone->dtzi->DaylightName);
    ptzi->Bias = timezone->dtzi->Bias;
    ptzi->DaylightBias = timezone->dtzi->DaylightBias;
    ptzi->DaylightDate = timezone->dtzi->DaylightDate;
    ptzi->StandardBias = timezone->dtzi->StandardBias;
    ptzi->StandardDate = timezone->dtzi->StandardDate;

    for (i = 0; i < size; ++i) {
        size_t index = size - i - 1;
        dt_tz_data_t data = reg_tz_data[index];

        ptzi->Bias = data.data.Bias;
        ptzi->DaylightBias = data.data.DaylightBias;
        ptzi->DaylightDate = data.data.DaylightDate;
        ptzi->StandardBias = data.data.StandardBias;
        ptzi->StandardDate = data.data.StandardDate;
        if (data.year <= wYear) {
            return TRUE;
        }
    }
    return TRUE;
}

static int createYearsArray(HKEY hkey_tz, YEARS_ARRAY yearsArray)
{
    DWORD dw = sizeof(DWORD);
    DWORD wYear = 0;
    DWORD dwEnumIndex = 0;
    char yearValueName[255] = {0,};
    DWORD dwErrorCode = ERROR_RESOURCE_NOT_FOUND;

    for (dwErrorCode = ERROR_SUCCESS; dwErrorCode != ERROR_NO_MORE_ITEMS ||
            (dwErrorCode != ERROR_SUCCESS && dwErrorCode != ERROR_NO_MORE_ITEMS);) {
        dw = sizeof(yearValueName);
        dwErrorCode = RegEnumValueA(hkey_tz, dwEnumIndex, yearValueName, &dw, NULL, NULL, NULL, NULL);
        dwEnumIndex++;
        if (strcmp(yearValueName, DYNAMIC_DST_FIRST_ENTRY) == 0
                || strcmp(yearValueName, DYNAMIC_DST_LAST_ENTRY) == 0) {
            continue;
        }
        if (EOF == sscanf_s(yearValueName, "%d", &wYear)) {
            continue;
        }

        InsertYearToArray(wYear, &yearsArray);
    }
    if (dwErrorCode != ERROR_SUCCESS && dwErrorCode != ERROR_NO_MORE_ITEMS) {
        free(yearsArray.years);
        return EXIT_FAILURE;
    }

    qsort_s(yearsArray.years, yearsArray.count, sizeof(DWORD), years_compare, NULL);
    return EXIT_SUCCESS;
}

void readYearTZDataFromRegistry(char *keyPath, DWORD dwEnumIndex, YEARS_ARRAY yearsArray, dt_tz_data_t *reg_tz_data)
{
    REG_TZI_FORMAT regtzi = {0,};
    char yearValueName[255] = {0,};
    DWORD wYear = yearsArray.years[dwEnumIndex];
    reg_tz_data[dwEnumIndex].year = wYear;

    snprintf(yearValueName, sizeof(yearValueName), "%d", wYear);
    if (ERROR_SUCCESS == GetTziFromKey(keyPath, yearValueName, &regtzi)) {
        reg_tz_data[dwEnumIndex].year = wYear;
        reg_tz_data[dwEnumIndex].data.Bias = regtzi.Bias;
        reg_tz_data[dwEnumIndex].data.DaylightBias = regtzi.DaylightBias;
        reg_tz_data[dwEnumIndex].data.DaylightDate = regtzi.DaylightDate;
        reg_tz_data[dwEnumIndex].data.StandardBias = regtzi.StandardBias;
        reg_tz_data[dwEnumIndex].data.StandardDate = regtzi.StandardDate;
    }
}

static BOOL dt_timezone_read_registry(dt_timezone_t *timezone)
{
    HKEY hkey_tz = NULL;
    DWORD dwErrorCode = ERROR_RESOURCE_NOT_FOUND;
    BOOL  returnStatus = FALSE;
    char timeZoneName[128] = {0,};
    char *keyPath = NULL;
    size_t keyPathSize = sizeof(REG_TIME_ZONES) + sizeof(timeZoneName) + sizeof(DYNAMIC_DST) + sizeof('\\') + sizeof('\0');
    DWORD dwEnumIndex = 0;
    YEARS_ARRAY yearsArray = {0,};
    dt_tz_data_t *reg_tz_data = NULL;

    WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, timezone->dtzi->TimeZoneKeyName, sizeof(timezone->dtzi->TimeZoneKeyName), timeZoneName, sizeof(timeZoneName), "\0", NULL);
    keyPath = malloc(keyPathSize);
    snprintf(keyPath, keyPathSize, "%s%s\\%s", REG_TIME_ZONES, timeZoneName, DYNAMIC_DST);

    dwErrorCode = RegOpenKeyA(HKEY_LOCAL_MACHINE, keyPath, &hkey_tz);
    if (ERROR_SUCCESS != dwErrorCode) {
        if (ERROR_FILE_NOT_FOUND != dwErrorCode) {
            return FALSE;
        } else {
            timezone->reg_tz_data_size = 0;
            return TRUE;
        }
    }

    if (createYearsArray(hkey_tz, yearsArray) != EXIT_SUCCESS) {
        returnStatus = FALSE;
        goto GetTimeZoneInformationForYearLower_cleanup;
    }

    timezone->reg_tz_data_size = yearsArray.count;
    timezone->reg_tz_data = malloc(sizeof(dt_tz_data_t) * timezone->reg_tz_data_size);
    memset(timezone->reg_tz_data, 0, sizeof(dt_tz_data_t) * timezone->reg_tz_data_size);
    reg_tz_data = timezone->reg_tz_data;

    for (dwEnumIndex = 0; dwEnumIndex < yearsArray.count && yearsArray.years[dwEnumIndex] != YEAR_WRONG_VALUE; dwEnumIndex++) {
        readYearTZDataFromRegistry(keyPath, dwEnumIndex, yearsArray, reg_tz_data);
    }

    if (yearsArray.size > 0) {
        free(yearsArray.years);
    }

    returnStatus = TRUE;
GetTimeZoneInformationForYearLower_cleanup:
    RegCloseKey(hkey_tz);
    free(keyPath);

    return returnStatus;
}

dt_status_t dt_timezone_lookup(const char *timezone_name, dt_timezone_t *timezone)
{
    dt_status_t status = DT_UNKNOWN_ERROR;
    tz_aliases_t *aliases = NULL;
    tz_alias_iterator_t *it = TZMAP_BEGIN;
    tz_alias_t *alias = NULL;
    const char *native_tz_name = NULL;

    if (timezone == NULL || timezone_name == NULL) {
        return DT_INVALID_ARGUMENT;
    }

    timezone->reg_tz_data = NULL;
    timezone->reg_tz_data_size = 0;

    if ((status = tzmap_map(timezone_name, &aliases)) != DT_OK) {
        return status;
    }

    while ((status = tzmap_iterate(aliases, &it, &alias)) == DT_OK) {
        if (alias->kind == DT_PREFFERED_TZMAP_TYPE) {
            native_tz_name = alias->name;
            break;
        }
    }

    tzmap_free(aliases);

    if (native_tz_name != NULL) {
        timezone->dtzi = malloc(sizeof(*timezone->dtzi));
        if (GetTimeZoneInformationByName(timezone->dtzi, native_tz_name) == EXIT_SUCCESS) {
            if (dt_timezone_read_registry(timezone)) {
                status = DT_OK;
            } else {
                free(timezone->dtzi);
                status = DT_TIMEZONE_NOT_FOUND;
            }
        } else {
            free(timezone->dtzi);
            status = DT_TIMEZONE_NOT_FOUND;
        }

    }

    return status;
}

dt_status_t dt_timezone_cleanup(dt_timezone_t *timezone)
{
    if (timezone == NULL || timezone->dtzi == NULL) {
        return DT_INVALID_ARGUMENT;
    }
    if (timezone->reg_tz_data && timezone->reg_tz_data_size) {
        free(timezone->reg_tz_data);
        timezone->reg_tz_data = NULL;
    }
    free(timezone->dtzi);
    timezone->reg_tz_data_size = 0;

    return DT_OK;
}
