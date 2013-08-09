/*
 * To build run from CLI:
 *
 * Unix:
 * $ gcc -o code_sample code_sample.c
 *
 * Windows:
 * $ cl code_sample.c /link Advapi32.lib
 */

#include "code_sample.h"

int main(int argc, char *argv[])
{
        struct tm tm;

        fill_tm(&tm, 2009, 7, 15, 8, 0, 0);
        test_tm(&tm, MOSCOW_TZ_NAME);
        fill_tm(&tm, 2009, 1, 15, 8, 0, 0);
        test_tm(&tm, MOSCOW_TZ_NAME);
        fill_tm(&tm, 2013, 7, 15, 8, 0, 0);
        test_tm(&tm, MOSCOW_TZ_NAME);
        fill_tm(&tm, 2013, 1, 15, 8, 0, 0);
        test_tm(&tm, MOSCOW_TZ_NAME);

        fill_tm(&tm, 2009, 7, 15, 8, 0, 0);
        test_tm(&tm, BERLIN_TZ_NAME);
        fill_tm(&tm, 2009, 1, 15, 8, 0, 0);
        test_tm(&tm, BERLIN_TZ_NAME);
        fill_tm(&tm, 2013, 7, 15, 8, 0, 0);
        test_tm(&tm, BERLIN_TZ_NAME);
        fill_tm(&tm, 2013, 1, 15, 8, 0, 0);
        test_tm(&tm, BERLIN_TZ_NAME);

        return 0;
}
