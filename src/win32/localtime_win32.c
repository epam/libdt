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
#include "libtz/dt.h"

//Registry timezones database path
static const char REG_TIME_ZONES[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\";
static const char DYNAMIC_DST[] = "Dynamic DST";
static const DWORD YEAR_WRONG_VALUE = 0xFFFFFFFF; // any way wrong value for a year
static const size_t YEARS_ARRAY_SEED = 10; // seed which setups how often will be memeory reallocation given in years array
static const char DYNAMIC_DST_FIRST_ENTRY[] = "FirstEntry";
static const char DYNAMIC_DST_LAST_ENTRY[] = "LastEntry";

#ifdef __cplusplus
extern "C" {
#endif
char * strptime(const char *buf, const char *fmt, struct tm *tm);

#ifdef __cplusplus
}
#endif
//GetTimeZoneInformationForYear implementation for windows server 2003 and lower
static
BOOL
WINAPI
GetTimeZoneInformationForYearLower(
    __in USHORT wYear,
    __in_opt PDYNAMIC_TIME_ZONE_INFORMATION pdtzi,
    __out LPTIME_ZONE_INFORMATION ptzi
    );
//Helper functions prototypes
static int GetTimeZoneInformationByName(DYNAMIC_TIME_ZONE_INFORMATION *ptzi, const char szStandardName[]);
static int TmFromSystemTime(const SYSTEMTIME * pTime, struct tm *tm);
static int SystemTimeFromTm(SYSTEMTIME *pTime, const struct tm *tm);
static int UnixTimeToSystemTime(const time_t *t, LPSYSTEMTIME pst);
static int SystemTimeToUnixTime(SYSTEMTIME *systemTime, time_t *dosTime);
static int years_compare(void* contex, const void* year1, const void* year2);
static BOOL FindCorrespondingYear(HKEY hkey_tz, DWORD targetYear, DWORD dstMaximumYear, DWORD dstMinimumYear, DWORD* findedYear);
//Tests is current windows version suitable to given version parts
static BOOL IsSuitableWindowsVersion(DWORD dwMajor, DWORD dwMinor);

typedef struct _YEARS_ARRAY {
    size_t size;
    DWORD* years;
} YEARS_ARRAY;

typedef struct _REG_TZI_FORMAT {
    LONG Bias;
    LONG StandardBias;
    LONG DaylightBias;
    SYSTEMTIME StandardDate;
    SYSTEMTIME DaylightDate;
} REG_TZI_FORMAT;

int localtime_tz(const time_t *time, const char *tzName, struct tm *result)
{

    DWORD dwError = -1;
    SYSTEMTIME tLocalTime = {0};
    SYSTEMTIME tUniversalTime = {0};
    DYNAMIC_TIME_ZONE_INFORMATION dtzi = {0};
    TIME_ZONE_INFORMATION tzi = {0};

    if (time == NULL || tzName == NULL || result == NULL) {
        return EXIT_FAILURE;
    }

    dwError = GetTimeZoneInformationByName(&dtzi, tzName);
    if (dwError != 0) {
        return EXIT_FAILURE;
    }

    if (UnixTimeToSystemTime(time, &tUniversalTime) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    if (GetTimeZoneInformationForYearLower(tUniversalTime.wYear, &dtzi, &tzi) == FALSE) {
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
    DYNAMIC_TIME_ZONE_INFORMATION dtzi;
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
        dwError = GetTimeZoneInformationByName(&dtzi, tzname);
        if (dwError != 0) {
            return EXIT_FAILURE;
        }
        if (GetTimeZoneInformationForYearLower(tLocalTime.wYear, &dtzi, &tzi) == FALSE) {
            return EXIT_FAILURE;
        }

        if (TzSpecificLocalTimeToSystemTime(&tzi, &tLocalTime, &tUniversalTime) == FALSE) {
            return EXIT_FAILURE;
        }

    }

    return SystemTimeToUnixTime(&tUniversalTime, result);
}

int strftime_tz(const struct tm *representation, const char *tz_name, const char *fmt,
                 char *str_buffer, size_t str_buffer_size) {
    size_t size = 0;
    if (!representation || !tz_name || !fmt || !str_buffer || str_buffer_size <= 0)
        return EXIT_FAILURE;

    size = strftime(str_buffer, str_buffer_size, fmt, representation);
    if (size > 0)
        return EXIT_SUCCESS;
    return EXIT_FAILURE;
}

int strptime_tz(const char *str, const char *fmt, struct tm *representation) {
    char *result = NULL;
    if (!representation || !str || !fmt)
        return EXIT_FAILURE;
    result = strptime(str, fmt, representation);
    if (result == NULL)
        return EXIT_FAILURE;
    if (*result != '\0')// end of string
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

static int GetTziFromKey(const char szKey[], const char szValue[], REG_TZI_FORMAT* ptzi)
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

    if (ERROR_SUCCESS != RegGetValueA(hkey_tz, NULL, szValue,
                                      RRF_RT_REG_BINARY, NULL, ptzi, &dw)) {
        rc = EXIT_FAILURE;
    }
    else {
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
    size_t subKeySize = strlen(REG_TIME_ZONES) + strlen(szStandardName) + 1;
    char* tszSubkey = (char *)malloc(subKeySize);
    if (ptzi == NULL || szStandardName == NULL) {
        return EXIT_FAILURE;
    }
    memset(tszSubkey, (int)NULL, subKeySize );
    memset(ptzi, 0, sizeof(DYNAMIC_TIME_ZONE_INFORMATION));

    sprintf_s(tszSubkey, subKeySize, "%s%s", REG_TIME_ZONES, szStandardName);

    if (ERROR_SUCCESS != (dw = RegOpenKeyA(HKEY_LOCAL_MACHINE, tszSubkey, &hkey_tz))) {
        rc = EXIT_FAILURE;
        goto ennd;
    }

    rc = 0;
    #define X(param, type, var) \
        do if ((dw = sizeof(var)), (ERROR_SUCCESS != (dw = RegGetValueW(hkey_tz, NULL, param, type, NULL, &var, &dw)))) { \
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

static int TmFromSystemTime(const SYSTEMTIME * pTime, struct tm *tm)
{
    if (tm == NULL || pTime == NULL) return EXIT_FAILURE;

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
static void UnixTimeToFileTime(const time_t *t, LPFILETIME pft) {
    // Note that LONGLONG is a 64-bit value
    UINT64 ll;
    UINT64 time = *t;
    ll = (time * 10000000) + 116444736000000000;
    pft->dwLowDateTime = (DWORD)ll;
    pft->dwHighDateTime = ll >> 32;
}

//was gotten from microsoft support
static int UnixTimeToSystemTime(const time_t *t, LPSYSTEMTIME pst) {
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
    jan1970FT.QuadPart = 116444736000000000I64; // january 1st 1970


    if (SystemTimeToFileTime(systemTime, (FILETIME*)&utcFT) == FALSE) {
        return EXIT_FAILURE;
    }
     utcDosTime = (utcFT.QuadPart - jan1970FT.QuadPart)/10000000;
    *dosTime = (time_t)utcDosTime;
     return EXIT_SUCCESS;
}


static int SystemTimeFromTm(SYSTEMTIME *pTime, const struct tm *tm)
{
    if (tm == NULL || pTime == NULL) return EXIT_FAILURE;

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
static int InsertYearToArray(DWORD year, YEARS_ARRAY* array, DWORD index) {
    if (array == 0) {
        return EXIT_FAILURE;
    }
    while (index >= array->size) {
        array->years = realloc(array->years, (array->size + YEARS_ARRAY_SEED) * sizeof(DWORD));
        if(array->years == 0) {
            return EXIT_FAILURE;
        }
        memset(array->years + (array->size * sizeof(DWORD)), YEAR_WRONG_VALUE, YEARS_ARRAY_SEED * sizeof(DWORD));
        array->size += YEARS_ARRAY_SEED;
    }
    array->years[index] = year;
    return EXIT_SUCCESS;
}

static BOOL FindCorrespondingYear(HKEY hkey_tz, DWORD targetYear, DWORD dstMaximumYear, DWORD dstMinimumYear, DWORD* findedYear)
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
    }
    else if (targetYear <= dstMinimumYear) {
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
        if (targetYear >= *findedYear) break;
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
    __in USHORT wYear,
    __in_opt PDYNAMIC_TIME_ZONE_INFORMATION pdtzi,
    __out LPTIME_ZONE_INFORMATION ptzi
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
    char* keyPath = NULL;
    size_t keyPathSize = sizeof(REG_TIME_ZONES) + sizeof(timeZoneName) + sizeof(DYNAMIC_DST) + sizeof('\\') + sizeof('\0');

    if (wYear < 1601 || pdtzi == NULL || ptzi == NULL) {
        return FALSE;
    }


    WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, pdtzi->TimeZoneKeyName, sizeof(pdtzi->TimeZoneKeyName), timeZoneName, sizeof(timeZoneName), "\0", NULL);
    keyPath = malloc(keyPathSize);
    sprintf_s(keyPath, keyPathSize, "%s%s\\%s", REG_TIME_ZONES, timeZoneName, DYNAMIC_DST);

    memset(ptzi, 0, sizeof(TIME_ZONE_INFORMATION));
    wcscpy_s(ptzi->StandardName, sizeof(ptzi->StandardName) / sizeof(WCHAR), pdtzi->StandardName);
    wcscpy_s(ptzi->DaylightName, sizeof(ptzi->DaylightName) / sizeof(WCHAR), pdtzi->DaylightName);
    dwErrorCode = RegOpenKeyA(HKEY_LOCAL_MACHINE, keyPath, &hkey_tz);
    if (ERROR_SUCCESS != dwErrorCode) {
        if (ERROR_FILE_NOT_FOUND != dwErrorCode) {
            return FALSE;
        }
        else {
            ptzi->Bias = pdtzi->Bias;
            ptzi->DaylightBias = pdtzi->DaylightBias;
            ptzi->DaylightDate = pdtzi->DaylightDate;
            ptzi->StandardBias = pdtzi->StandardBias;
            ptzi->StandardDate = pdtzi->StandardDate;
            return TRUE;
        }
    }
    dw = sizeof(DWORD);

    if (ERROR_SUCCESS != RegGetValueA(hkey_tz,
                                      NULL, DYNAMIC_DST_FIRST_ENTRY,
                                      RRF_RT_REG_DWORD, NULL, &dstMinimumYear, &dw)) {
        returnStatus = FALSE;
        goto GetTimeZoneInformationForYearLower_cleanup;
    }

    if (ERROR_SUCCESS != RegGetValueA(hkey_tz,
                                      NULL, DYNAMIC_DST_LAST_ENTRY,
                                      RRF_RT_REG_DWORD, NULL, &dstMaximumYear, &dw)) {
        returnStatus = FALSE;
        goto GetTimeZoneInformationForYearLower_cleanup;
    }

    if (FALSE == FindCorrespondingYear(hkey_tz, wYear , dstMaximumYear, dstMinimumYear, &findedYear)) {
        goto GetTimeZoneInformationForYearLower_cleanup;
    }

    sprintf_s(yearValueName, sizeof(yearValueName), "%d", findedYear);

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

static int years_compare(void* contex, const void* year1, const void* year2)
{
    const DWORD* y1 = year1;
    const DWORD* y2 = year2;
    (void*)contex;
    if (*y1 < *y2) return -1;
    else if(*y1 > *y2) return 1;
    else return 0;
}
