#include <stdlib.h>
//interface
#include "libtz/dt.h"
#include "libtz/dt_precise.h"

int localtime_tz(const time_t *time, const char *tzName, struct tm *result)
{
    dt_status_t status = DT_UNKNOWN_ERROR;
    dt_timestamp_t t = {0};
    dt_representation_t rep = {0};

    if (!time || !result || !tzName)
        return EXIT_FAILURE;

    status = dt_posix_time_to_timestamp(*time, 0, &t);
    if (status != DT_OK)
        return EXIT_FAILURE;

    status = dt_timestamp_to_representation(&t, tzName, &rep);
    if (status != DT_OK)
        return EXIT_FAILURE;

    status = dt_representation_to_tm(&rep, result);
    if (status != DT_OK)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int mktime_tz(const struct tm *tm, const char *tz, time_t *result)
{
    dt_status_t status = DT_UNKNOWN_ERROR;
    dt_timestamp_t t = {0};
    dt_representation_t rep = {0};
    unsigned long nano = 0;

    if (!tm || !result)
        return EXIT_FAILURE;

    status = dt_tm_to_representation(tm, 0, &rep);
    if (status != DT_OK)
        return EXIT_FAILURE;

    status = dt_representation_to_timestamp(&rep, tz, &t, NULL);
    if (status != DT_OK)
        return EXIT_FAILURE;

    status = dt_timestamp_to_posix_time(&t, result, &nano);
    if (status != DT_OK)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

