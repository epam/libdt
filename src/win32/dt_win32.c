#include <stdio.h>  /* for stdout, stderr, perror */
#include <time.h>   /* for struct tm */
#include <stdlib.h> /* for exit, malloc, atoi */
#include <string.h> /* for strlen, strcpy_s, strcat_s*/
#include <memory.h> /* for malloc, free*/

#include <libdt/dt.h>
#include <libdt/dt_posix.h>
#include "../dt_private.h"

// WinAPI
#include <Windows.h>
#include <WinReg.h>
#include <WinBase.h>
#include <winnt.h>


//Registry timezones database path
static const char REG_TIME_ZONES[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\";
static const char DYNAMIC_DST[] = "Dynamic DST";
static const DWORD YEAR_WRONG_VALUE = 0xFFFFFFFF; // any way wrong value for a year
static const size_t YEARS_ARRAY_SEED = 10; // seed which setups how often will be memeory reallocation given in years array
static const char DYNAMIC_DST_FIRST_ENTRY[] = "FirstEntry";
static const char DYNAMIC_DST_LAST_ENTRY[] = "LastEntry";

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

wchar_t *wcscpy_s (wchar_t *dest, size_t size, const wchar_t *source)
{
    (void *)&size;
    return wcscpy(dest, source);
}
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
    /*__in_opt*/ PDYNAMIC_TIME_ZONE_INFORMATION pdtzi,
    /*__out*/ LPTIME_ZONE_INFORMATION ptzi
);
//Helper functions prototypes
static int GetTimeZoneInformationByName(DYNAMIC_TIME_ZONE_INFORMATION *ptzi, const char szStandardName[]);
static int TmFromSystemTime(const SYSTEMTIME *pTime, struct tm *tm);
static int SystemTimeFromTm(SYSTEMTIME *pTime, const struct tm *tm);
static int UnixTimeToSystemTime(const time_t *t, LPSYSTEMTIME pst);
static int SystemTimeToUnixTime(SYSTEMTIME *systemTime, time_t *dosTime);
static BOOL FindCorrespondingYear(HKEY hkey_tz, DWORD targetYear, DWORD dstMaximumYear, DWORD dstMinimumYear, DWORD *findedYear);
//Tests is current windows version suitable to given version parts
static BOOL IsSuitableWindowsVersion(DWORD dwMajor, DWORD dwMinor);
char *libdt_strptime(const char *buf, const char *fmt, struct tm *tm);

typedef struct _YEARS_ARRAY {
    size_t size;
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
    if (time < 0 || !result) {
        return DT_INVALID_ARGUMENT;
    }

    result->second = (long)time;
    result->nano_second = nano_second;
    return DT_OK;
}

dt_status_t dt_timestamp_to_posix_time(const dt_timestamp_t *timestamp, time_t *time, unsigned long *nano_second)
{
    if (!timestamp || !time || timestamp->second < 0) {
        return DT_INVALID_ARGUMENT;
    }

    *time = timestamp->second;
    *nano_second = timestamp->nano_second;
    return DT_OK;
}

dt_status_t dt_timestamp_to_representation(const dt_timestamp_t *timestamp, const dt_timezone_t *tz, dt_representation_t *representation)
{
    DWORD dwError = -1;
    SYSTEMTIME tLocalTime = {0};
    SYSTEMTIME tUniversalTime = {0};
    DYNAMIC_TIME_ZONE_INFORMATION dtzi = {0};
    TIME_ZONE_INFORMATION tzi = {0};
    time_t time = 0;
    unsigned long nano = 0;
    struct tm result = {0};
    dt_status_t status = DT_UNKNOWN_ERROR;

    if (timestamp == NULL || representation == NULL) {
        return DT_INVALID_ARGUMENT;
    }


    if (tz == NULL || tz->time_zone_name == NULL) {
        dwError = GetTimeZoneInformation((LPTIME_ZONE_INFORMATION)&dtzi);
        if (dwError != 0) {
            return DT_TIMEZONE_NOT_FOUND;
        }
    } else {
        dwError = GetTimeZoneInformationByName(&dtzi, tz->time_zone_name);
        if (dwError != 0) {
            return DT_TIMEZONE_NOT_FOUND;
        }
    }


    status = dt_timestamp_to_posix_time(timestamp, &time, &nano);
    if (status != DT_OK) {
        return status;
    }

    if (UnixTimeToSystemTime(&time, &tUniversalTime)) {
        return DT_CONVERT_ERROR;
    }

    if (GetTimeZoneInformationForYearLower(tUniversalTime.wYear, &dtzi, &tzi) == FALSE) {
        return DT_CONVERT_ERROR;
    }

    if (SystemTimeToTzSpecificLocalTime(&tzi, &tUniversalTime, &tLocalTime) == FALSE) {
        return DT_CONVERT_ERROR;
    }

    if (TmFromSystemTime(&tLocalTime, &result) != EXIT_SUCCESS) {
        return DT_CONVERT_ERROR;
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
    DYNAMIC_TIME_ZONE_INFORMATION dtzi;
    SYSTEMTIME tUniversalTime = {0};
    SYSTEMTIME tLocalTime = {0};
    time_t time = 0;
    unsigned long nano = 0;
    struct tm tm = {0};
    dt_status_t status = DT_UNKNOWN_ERROR;

    if (!representation || !first_timestamp) {
        return DT_INVALID_ARGUMENT;
    }

    nano = representation->nano_second;
    status = dt_representation_to_tm(representation, &tm);
    if (status != DT_OK) {
        return status;
    }

    if (SystemTimeFromTm(&tLocalTime, &tm) != EXIT_SUCCESS) {
        return DT_CONVERT_ERROR;
    }

    if (timezone != NULL && timezone->time_zone_name != NULL) {
        dwError = GetTimeZoneInformationByName(&dtzi, timezone->time_zone_name);
        if (dwError != 0) {
            return DT_TIMEZONE_NOT_FOUND;
        }

    } else {
        dwError = GetTimeZoneInformation((LPTIME_ZONE_INFORMATION)&dtzi);
        if (dwError != 0) {
            return DT_TIMEZONE_NOT_FOUND;
        }
    }

    if (GetTimeZoneInformationForYearLower(tLocalTime.wYear, &dtzi, &tzi) == FALSE) {
        return DT_TIMEZONE_NOT_FOUND;
    }


    if (TzSpecificLocalTimeToSystemTime((LPTIME_ZONE_INFORMATION)&tzi, (LPSYSTEMTIME)&tLocalTime, (LPSYSTEMTIME)&tUniversalTime) == FALSE) {
        return DT_CONVERT_ERROR;
    }

    if (SystemTimeToUnixTime(&tUniversalTime, &time) != EXIT_SUCCESS) {
        return DT_CONVERT_ERROR;
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

static int GetTimeZoneInformationByName(DYNAMIC_TIME_ZONE_INFORMATION *ptzi, const char szStandardName[])
{
    int rc = EXIT_FAILURE;
    HKEY hkey_tz = NULL;
    DWORD dw = 0;
    REG_TZI_FORMAT regtzi = {0,};
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

    if (ERROR_SUCCESS != (dw = RegOpenKeyA(HKEY_LOCAL_MACHINE, tszSubkey, &hkey_tz))) {
        rc = EXIT_FAILURE;
        goto ennd;
    }

    rc = 0;
#define X(param, type, var) \
    do if ((dw = sizeof(var)), (ERROR_SUCCESS != (dw = RegQueryValueExW(hkey_tz, param, NULL, NULL, (LPBYTE)&var, &dw)))) { \
            rc = EXIT_FAILURE; \
            goto ennd; \
        } while(0)
    GetTziFromKey(tszSubkey, "TZI", &regtzi);
    X(L"Std", RRF_RT_REG_SZ, ptzi->StandardName);
    X(L"Dlt", RRF_RT_REG_SZ, ptzi->DaylightName);
#undef X
    ptzi->Bias = regtzi.Bias;
    ptzi->DaylightBias = regtzi.DaylightBias;
    ptzi->DaylightDate = regtzi.DaylightDate;
    ptzi->StandardBias = regtzi.StandardBias;
    ptzi->StandardDate = regtzi.StandardDate;
    MultiByteToWideChar(CP_ACP, MB_COMPOSITE, szStandardName, -1 , ptzi->TimeZoneKeyName, sizeof(ptzi->TimeZoneKeyName));
ennd:
    RegCloseKey(hkey_tz);
    free(tszSubkey);
    return rc;
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
static int InsertYearToArray(DWORD year, YEARS_ARRAY *array, DWORD index)
{
    if (array == 0) {
        return EXIT_FAILURE;
    }

    while (index >= array->size) {
        array->years = realloc(array->years, (array->size + YEARS_ARRAY_SEED) * sizeof(DWORD));

        if (array->years == 0) {
            return EXIT_FAILURE;
        }

        memset(array->years + (array->size * sizeof(DWORD)), YEAR_WRONG_VALUE, YEARS_ARRAY_SEED * sizeof(DWORD));
        array->size += YEARS_ARRAY_SEED;
    }
    array->years[index] = year;
    return EXIT_SUCCESS;
}

static BOOL FindCorrespondingYear(HKEY hkey_tz, DWORD targetYear, DWORD dstMaximumYear, DWORD dstMinimumYear, DWORD *findedYear)
{
    DWORD dwEnumIndex = 0;
    DWORD dw = 0;
    DWORD dwErrorCode = ERROR_RESOURCE_NOT_FOUND;
    char yearValueName[255] = {0,};
    BOOL returnStatus = FALSE;
    YEARS_ARRAY yearsArray = {0,};
    if (targetYear >= dstMaximumYear) {
        *findedYear = dstMaximumYear;
        return TRUE;
    } else if (targetYear <= dstMinimumYear) {
        *findedYear = dstMinimumYear;
        return TRUE;
    }



    dwEnumIndex = 0;

    for (dwErrorCode = ERROR_SUCCESS; dwErrorCode != ERROR_NO_MORE_ITEMS ||
            (dwErrorCode != ERROR_SUCCESS && dwErrorCode != ERROR_NO_MORE_ITEMS);) {
        dw = sizeof(yearValueName);
        dwErrorCode = RegEnumValueA(hkey_tz, dwEnumIndex, yearValueName, &dw, NULL, NULL, NULL, NULL);
        dwEnumIndex++;
        if (strcmp(yearValueName, DYNAMIC_DST_FIRST_ENTRY) == 0
                || strcmp(yearValueName, DYNAMIC_DST_LAST_ENTRY) == 0) {
            continue;
        }


        if (EOF == sscanf_s(yearValueName, "%d", findedYear)) {
            continue;
        }

        InsertYearToArray(*findedYear, &yearsArray, dwEnumIndex);
    }
    if (dwErrorCode != ERROR_SUCCESS && dwErrorCode != ERROR_NO_MORE_ITEMS) {
        returnStatus = FALSE;
        free(yearsArray.years);
        *findedYear = YEAR_WRONG_VALUE;
        return FALSE;
    }



    qsort_s(yearsArray.years, yearsArray.size, sizeof(DWORD), years_compare, NULL);

    for (dwEnumIndex = 0; dwEnumIndex < yearsArray.size - 1 && yearsArray.years[dwEnumIndex] != YEAR_WRONG_VALUE; dwEnumIndex++) {
        *findedYear = yearsArray.years[dwEnumIndex];
        if (targetYear >= *findedYear) {
            break;
        }
    }

    if (yearsArray.size > 0) {
        free(yearsArray.years);
    }

    if (*findedYear == YEAR_WRONG_VALUE) {
        return FALSE;
    }

    return TRUE;
}

static
BOOL
WINAPI
GetTimeZoneInformationForYearLower(
    /*__in*/ USHORT wYear,
    /*__in_opt*/ PDYNAMIC_TIME_ZONE_INFORMATION pdtzi,
    /*__out*/ LPTIME_ZONE_INFORMATION ptzi
)
{
    HKEY hkey_tz = NULL;
    DWORD dw = 0;
    DWORD dwErrorCode = ERROR_RESOURCE_NOT_FOUND;
    DWORD dstMinimumYear = 1601;
    DWORD dstMaximumYear = 30827;
    DWORD findedYear = YEAR_WRONG_VALUE; //0xFFFFFFFF; any way wrong value
    REG_TZI_FORMAT regtzi = {0,};
    BOOL  returnStatus = FALSE;
    char yearValueName[255] = {0,};
    char timeZoneName[128] = {0,};
    char *keyPath = NULL;
    size_t keyPathSize = sizeof(REG_TIME_ZONES) + sizeof(timeZoneName) + sizeof(DYNAMIC_DST) + sizeof('\\') + sizeof('\0');

    if (wYear < 1601 || pdtzi == NULL || ptzi == NULL) {
        return FALSE;
    }


    WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, pdtzi->TimeZoneKeyName, sizeof(pdtzi->TimeZoneKeyName), timeZoneName, sizeof(timeZoneName), "\0", NULL);
    keyPath = malloc(keyPathSize);
    snprintf(keyPath, keyPathSize, "%s%s\\%s", REG_TIME_ZONES, timeZoneName, DYNAMIC_DST);

    memset(ptzi, 0, sizeof(TIME_ZONE_INFORMATION));

    wcscpy_s(ptzi->StandardName, sizeof(ptzi->StandardName) / sizeof(WCHAR), pdtzi->StandardName);
    wcscpy_s(ptzi->DaylightName, sizeof(ptzi->DaylightName) / sizeof(WCHAR), pdtzi->DaylightName);
    dwErrorCode = RegOpenKeyA(HKEY_LOCAL_MACHINE, keyPath, &hkey_tz);
    if (ERROR_SUCCESS != dwErrorCode) {
        if (ERROR_FILE_NOT_FOUND != dwErrorCode) {
            return FALSE;
        } else {
            ptzi->Bias = pdtzi->Bias;
            ptzi->DaylightBias = pdtzi->DaylightBias;
            ptzi->DaylightDate = pdtzi->DaylightDate;
            ptzi->StandardBias = pdtzi->StandardBias;
            ptzi->StandardDate = pdtzi->StandardDate;
            return TRUE;
        }
    }
    dw = sizeof(DWORD);

    if (ERROR_SUCCESS != RegQueryValueExA(hkey_tz,
                                          DYNAMIC_DST_FIRST_ENTRY,
                                          NULL, NULL, (LPBYTE)&dstMinimumYear, &dw)) {
        returnStatus = FALSE;
        goto GetTimeZoneInformationForYearLower_cleanup;
    }

    if (ERROR_SUCCESS != RegQueryValueExA(hkey_tz,
                                          DYNAMIC_DST_LAST_ENTRY,
                                          NULL, NULL, (LPBYTE)&dstMaximumYear, &dw)) {
        returnStatus = FALSE;
        goto GetTimeZoneInformationForYearLower_cleanup;
    }

    if (FALSE == FindCorrespondingYear(hkey_tz, wYear , dstMaximumYear, dstMinimumYear, &findedYear)) {
        goto GetTimeZoneInformationForYearLower_cleanup;
    }

    snprintf(yearValueName, sizeof(yearValueName), "%d", findedYear);

    if (GetTziFromKey(keyPath, yearValueName, &regtzi) == EXIT_FAILURE) {
        memset(ptzi, 0, sizeof(TIME_ZONE_INFORMATION));
        goto GetTimeZoneInformationForYearLower_cleanup;
    }

    ptzi->Bias = regtzi.Bias;
    ptzi->DaylightBias = regtzi.DaylightBias;
    ptzi->DaylightDate = regtzi.DaylightDate;
    ptzi->StandardBias = regtzi.StandardBias;
    ptzi->StandardDate = regtzi.StandardDate;
    returnStatus = TRUE;
GetTimeZoneInformationForYearLower_cleanup:
    RegCloseKey(hkey_tz);
    free(keyPath);

    return returnStatus;
}



dt_status_t dt_to_string(const dt_representation_t *representation, const char *fmt,
                         char *str_buffer, size_t str_buffer_size)
{
    size_t size = 0;
    struct tm tm = {0};
    dt_status_t status = DT_UNKNOWN_ERROR;

    if (!representation || !fmt || !str_buffer || str_buffer_size == 0) {
        return DT_INVALID_ARGUMENT;
    }

    status = dt_representation_to_tm(representation, &tm);

    size = strftime(str_buffer, str_buffer_size, fmt, &tm);
    if (size > 0) {
        return DT_OK;
    }

    return status;
}

dt_status_t dt_from_string(const char *str, const char *fmt, dt_representation_t *representation)
{
    char *result = NULL;
    struct tm tm = {0};
    dt_status_t status = DT_UNKNOWN_ERROR;

    if (!representation || !str || !fmt) {
        return DT_INVALID_ARGUMENT;
    }

    result = libdt_strptime(str, fmt, &tm);
    if (result == NULL) {
        return status;
    }
    if (*result != '\0') { // end of string
        return status;
    }

    status = dt_tm_to_representation_withoutcheck(&tm, 0, representation);
    if (status != DT_OK) {
        return status;
    }

    return DT_OK;

}

dt_status_t dt_timezone_lookup(const char *timezone_name, dt_timezone_t *timezone)
{
    dt_status_t status = DT_UNKNOWN_ERROR;
    tz_aliases_t *aliases = NULL;
    tz_alias_iterator_t *it = TZMAP_BEGIN;
    tz_alias_t *alias = NULL;

    if (timezone == NULL || timezone_name == NULL) {
        return DT_INVALID_ARGUMENT;
    }

    if ((status = tzmap_map(timezone_name, &aliases)) != DT_OK) {
        return status;
    }

    while ((status = tzmap_iterate(aliases, &it, &alias)) == DT_OK) {
        if (alias->kind == PREFFERED_TZMAP_TYPE) {
            timezone->time_zone_name = alias->name;
            tzmap_free(aliases);
            return DT_OK;
        }
    }

    return status;
}

dt_status_t dt_timezone_cleanup(dt_timezone_t *timezone)
{
    if (timezone == NULL) {
        return DT_INVALID_ARGUMENT;
    }
    return DT_OK;
}
