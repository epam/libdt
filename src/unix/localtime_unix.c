#include "libtz/dt.h"
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include <timezones_map.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static char * findTimeZoneByName(const char *tz_name)
{
    int i = 0;
    char *nameOfTimeZone = NULL;
    for (i = 0; i < tz_unicode_map_size; i++) {
        struct tz_unicode_mapping tz = tz_unicode_map[i];
        if ((!strcmp(tz.type, tz_name)) ||
            (!strcmp(tz.territory, tz_name)) ||
            (!strcmp(tz.other, tz_name))) {
            nameOfTimeZone = tz.type;
            break;
        }
    }
    return nameOfTimeZone;
}

int localtime_tz(const time_t *time, const char *tz_name, struct tm *result)
{

    extern char ** environ;

    size_t longest = 1024;
    char ** fakeenv = NULL;
    char ** oldenv = NULL;
    int from = 0;
    int to = 0;
    int i = 0;
    char *nameOfTimeZone = NULL;

    if (time == NULL || tz_name == NULL || result == NULL) {
        return EXIT_FAILURE;
    }

    nameOfTimeZone = findTimeZoneByName(tz_name);

    if (!nameOfTimeZone)
        return EXIT_FAILURE;

    for (i = 0; environ[i] != NULL; ++i)
        continue;
    fakeenv = malloc((i + 2) * sizeof *fakeenv);
    if (fakeenv == NULL || (fakeenv[0] = malloc(longest + 4)) == NULL) {
        return EXIT_FAILURE;
    }

    to = 0;
    strcpy(fakeenv[to++], "TZ=");
    for (from = 0; environ[from] != NULL; ++from) {
        if (strncmp(environ[from], "TZ=", 3) != 0) {
            fakeenv[to++] = environ[from];
        }
    }
    fakeenv[to] = NULL;

    if (pthread_mutex_lock(&mutex)) {
        return EXIT_FAILURE;
    }
    oldenv = environ;
    environ = fakeenv;

    strcpy(&fakeenv[0][3], nameOfTimeZone);
    tzset();
    localtime_r(time, result);

    environ = oldenv;
    free(fakeenv);
    if (pthread_mutex_unlock(&mutex)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


int mktime_tz(const struct tm *tm, const char *tz_name, time_t *result)
{
    extern char ** environ;

    size_t longest = 1024;
    char ** fakeenv = NULL;
    char ** oldenv = NULL;
    int from = 0;
    int to = 0;
    int i = 0;
    char *nameOfTimeZone = NULL;

    if (!result || tm == NULL || tz_name == NULL) {
        return EXIT_FAILURE;
    }

    nameOfTimeZone = findTimeZoneByName(tz_name);

    if (!nameOfTimeZone)
        return EXIT_FAILURE;

    for (i = 0; environ[i] != NULL; ++i) {
        continue;
    }
    fakeenv = malloc((i + 2) * sizeof *fakeenv);
    if (fakeenv == NULL || (fakeenv[0] = malloc(longest + 4)) == NULL) {
        return EXIT_FAILURE;
    }

    to = 0;
    strcpy(fakeenv[to++], "TZ=");
    for (from = 0; environ[from] != NULL; ++from) {
        if (strncmp(environ[from], "TZ=", 3) != 0) {
            fakeenv[to++] = environ[from];
        }
    }
    fakeenv[to] = NULL;

    struct tm * tm_modifiable = (struct tm *)tm;
    tm_modifiable->tm_isdst = -1;

    if (pthread_mutex_lock(&mutex)) {
        return EXIT_FAILURE;
    }
    oldenv = environ;
    environ = fakeenv;

    strcpy(&fakeenv[0][3], nameOfTimeZone);
    tzset();
    *result = mktime(tm_modifiable);

    environ = oldenv;
    free(fakeenv);
    if (pthread_mutex_unlock(&mutex)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

