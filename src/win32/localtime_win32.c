//interface
#include "libtz/dt.h"

#ifdef __cplusplus
extern "C" {
#endif
char * strptime(const char *buf, const char *fmt, struct tm *tm);

#ifdef __cplusplus
}
#endif

int localtime_tz(const time_t *time, const char *tzName, struct tm *result)
{
}

int mktime_tz(const struct tm *tm, const char *tzname, time_t *result)
{
}

int strftime_tz(const struct tm *representation, const char *tz_name, const char *fmt,
                 char *str_buffer, size_t str_buffer_size) {
    /*size_t size = 0;
    if (!representation || !tz_name || !fmt || !str_buffer || str_buffer_size <= 0)
        return EXIT_FAILURE;

    size = strftime(str_buffer, str_buffer_size, fmt, representation);
    if (size > 0)
        return EXIT_SUCCESS;
    return EXIT_FAILURE;*/
}

int strptime_tz(const char *str, const char *fmt, struct tm *representation) {
    /*char *result = NULL;
    if (!representation || !str || !fmt)
        return EXIT_FAILURE;
    result = strptime(str, fmt, representation);
    if (result == NULL)
        return EXIT_FAILURE;
    if (*result != '\0')// end of string
        return EXIT_FAILURE;
    return EXIT_SUCCESS;*/
}
