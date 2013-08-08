#ifndef CODE_SAMPLE_H
#define CODE_SAMPLE_H

#include <stdio.h>

#if defined(_WIN32)

#include "code_sample_win.h"

#else

#include "code_sample_unix.h"

#endif

void fill_tm(struct tm *output, int tm_year, int tm_mon, int tm_mday,
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
        output->tm_year = tm_year;

}

void dump_tm(const struct tm *tm)
{
        printf("%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
}

void test_tm(const struct tm *tm, const char *tz_name)
{
        time_t tm_ts;
        assert(mktime_tz(tm, tz_name, &tm_ts) == 0);
        struct tm tm_ts_tm;
        assert(localtime_tz(&tm_ts, tz_name, &tm_ts_tm) == 0);
        struct tm tm_ts_tm_utc;
        assert(localtime_tz(&tm_ts, UTC_TZ_NAME, &tm_ts_tm_utc) == 0);
        printf("Datetime representation '");
        dump_tm(tm);
        printf("' in '%s' timezone is an %ld timestamp, round-up converted representation is '", tz_name, tm_ts);
        dump_tm(&tm_ts_tm);
        printf("', UTC representation is '");
        dump_tm(&tm_ts_tm_utc);
        printf("'\n");
}

#endif
