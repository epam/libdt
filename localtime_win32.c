#include <stdio.h>  /* for stdout, stderr, perror */
#include <time.h>   /* for struct tm */
#include <stdlib.h> /* for exit, malloc, atoi */
#include <string.h> /* for strlen, strcpy_s, strcat_s*/
#include <memory.h> /* for malloc, free*/

//for winapi
#include <windows.h>
#include <WinReg.h>

//interface
#include "localtime.h"

//Registry timezones database path
static const char REG_TIME_ZONES[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\";
int GetTimeZoneInformationByName(TIME_ZONE_INFORMATION *ptzi, const char StandardName[]);

int getLocalTime(const char *tzName, time_t time, struct tm *result)
{
    return EXIT_FAILURE;
}



typedef struct _REG_TZI_FORMAT
{
    LONG Bias;
    LONG StandardBias;
    LONG DaylightBias;
    SYSTEMTIME StandardDate;
    SYSTEMTIME DaylightDate;
} REG_TZI_FORMAT;



int GetTimeZoneInformationByName(TIME_ZONE_INFORMATION *ptzi, const char StandardName[]) {
    int rc;
    HKEY hkey_tz;
    DWORD dw;
    REG_TZI_FORMAT regtzi;
    size_t subKeySize = strlen(REG_TIME_ZONES) + strlen(StandardName) + 1;
    char* tszSubkey = (char *)malloc(subKeySize);
    memset(tszSubkey, NULL, subKeySize );
    sprintf_s(tszSubkey, subKeySize, "%s%s", REG_TIME_ZONES, StandardName);

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
