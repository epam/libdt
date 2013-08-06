#ifndef LOCALTIME_H
#define LOCALTIME_H

#include <time.h>
#ifndef EXIT_SUCCES
    #define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
    #define EXIT_FAILURE -1
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** function for convert time_t to local time with specific Time Zone.
 * @param time - time to format
 * @param tzName - name of time zone. Must be in format <Area>/<Place, such as Europe/Moscow or Asia/Oral.
 * @param result - variable for result. Value will be set to local time representation
 * @return on success, EXIT_SUCCESS is returned or EXIT_FAILURE in error case.
 */
int localtime_tz(const time_t *time, const char *tzName, struct tm *result);

/** function for convert local time with specific Time Zone, to time_t.
 * @param time - time to format
 * @param tzName - name of time zone. Must be in format <Area>/<Place, such as Europe/Moscow or Asia/Oral.
 * @param result - variable for result. Value will be set to local time representation
 * @return on success, EXIT_SUCCESS is returned or EXIT_FAILURE in error case.
 */
int mktime_tz(const struct tm *tm, const char * tzName, time_t *result);

#ifdef __cplusplus
}
#endif
#endif // LOCALTIME_H
