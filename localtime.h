#ifndef LOCALTIME_H
#define LOCALTIME_H

#include <time.h>
#ifndef EXIT_SUCCES
    #define EXIT_SUCCES 0
#endif
#ifndef EXIT_FAILURE
    #define EXIT_FAILURE -1
#endif
/** function for convert time_t to struct tm with specific Time Zone.
 * @param tzName - name of time zone. Must be in format <Area>/<Place, such as Europe/Moscow or Asia/Oral.
 * @param time - time to format
 * @param result - variable for result. Value will be set to local time representation
 * @return on success, EXIT_SUCCES is returned or EXIT_FAILURE in error case.
 */
int getLocalTime(const char *tzName, time_t time, struct tm *result);

int getUTCTime(const char *tzName, struct tm time, time_t *result);

#endif // LOCALTIME_H
