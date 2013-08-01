#ifndef LOCALTIME_H
#define LOCALTIME_H

#include <time.h>

/** function for convert time_t to struct tm with specific Time Zone.
 * @param tzName - name of time zone. Must be in format <Area>/<Place, such as Europe/Moscow or Asia/Oral.
 * @param time - time to format
 * @return NULL in error case, local time in other case
 */
struct tm * getLocalTime(const char *tzName, time_t time);

#endif // LOCALTIME_H
