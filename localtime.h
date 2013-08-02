#ifndef LOCALTIME_H
#define LOCALTIME_H

#include <time.h>

/** function for convert time_t to struct tm with specific Time Zone.
 * @param tzName - name of time zone. Must be in format <Area>/<Place, such as Europe/Moscow or Asia/Oral.
 * @param time - time to format
 * @param result - variable for result. Value will be set to local time representation
 * @return on success, EXIT_SUCCES is returned or EXIT_FAILURE in error case.
 */
int getLocalTime(const char *tzName, time_t time, struct tm *result);

#endif // LOCALTIME_H
