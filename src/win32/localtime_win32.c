//interface
#include "libtz/dt.h"
#include "libtz/dt_precise.h"

int localtime_tz(const time_t *time, const char *tzName, struct tm *result)
{
    dt_status_t status = DT_UNKNOWN_ERROR;
    dt_timestamp_t t = {0};
    dt_representation_t rep = {0};

    if (!time || !result)
        return DT_INVALID_ARGUMENT;

    status = dt_posix_time_to_timestamp(*time, 0, &t);
    if (status != DT_OK)
        return status;

    status = dt_timestamp_to_representation(&t, tzName, &rep);
    if (status != DT_OK)
        return status;

    status = dt_representation_to_tm(&rep, result);
    if (status != DT_OK)
        return status;

    return DT_OK;
}

int mktime_tz(const struct tm *tm, const char *tzName, time_t *result)
{
    dt_status_t status = DT_UNKNOWN_ERROR;
    dt_timestamp_t t = {0};
    dt_representation_t rep = {0};
    unsigned long nano = 0;

    if (!time || !result)
        return DT_INVALID_ARGUMENT;

    status = dt_tm_to_representation(tm, 0, &rep);
    if (status != DT_OK)
        return status;

    status = dt_representation_to_timestamp(&rep, tzName, &t, NULL);
    if (status != DT_OK)
        return status;

    status = dt_timestamp_to_posix_time(&t, result, &nano);
    if (status != DT_OK)
        return status;

    return DT_OK;
}

int strftime_tz(const struct tm *representation, const char *tz_name, const char *fmt,
                 char *str_buffer, size_t str_buffer_size) {
    dt_representation_t rep = {0};
    dt_status_t status = DT_UNKNOWN_ERROR;

    printf("%d\n", __LINE__);
    if (!representation || !tz_name || !fmt || !str_buffer || str_buffer_size <= 0)
        return DT_INVALID_ARGUMENT;
    printf("%d\n", __LINE__);

    status = dt_tm_to_representation(representation, 0, &rep);
    if (status != DT_OK)
        return status;
    printf("%d\n", __LINE__);

    return dt_to_string(&rep, tz_name, fmt, str_buffer, str_buffer_size);
}

int strptime_tz(const char *str, const char *fmt, struct tm *representation) {
    dt_representation_t rep = {0};
    dt_status_t status = DT_UNKNOWN_ERROR;

    if (!representation || !fmt || !str)
        return DT_INVALID_ARGUMENT;

    status = dt_from_string(str, fmt, &rep, NULL, NULL);
    if (status != DT_OK)
        return status;

    return dt_representation_to_tm(&rep, representation);
}
