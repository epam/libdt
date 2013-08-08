/*
 * To build run from CLI:
 *
 * Unix:
 * $ gcc -o code_sample code_sample.c
 *
 * Windows:
 * TODO: Insert build command
 */

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include "code_sample.h"

int main(int argc, char *argv[])
{
        struct tm tm_01;
        fill_tm(&tm_01, 2009, 7, 15, 8, 0, 0);
        //fill_tm(&tm_01, 2010, 1, 15, 8, 0, 0);
        struct tm tm_02;
        fill_tm(&tm_02, 2013, 7, 15, 8, 0, 0);
        //fill_tm(&tm_02, 2013, 1, 15, 8, 0, 0);

        test_tm(&tm_01, MOSCOW_TZ_NAME);
        test_tm(&tm_02, MOSCOW_TZ_NAME);
        test_tm(&tm_01, BERLIN_TZ_NAME);
        test_tm(&tm_02, BERLIN_TZ_NAME);

        return 0;
}
