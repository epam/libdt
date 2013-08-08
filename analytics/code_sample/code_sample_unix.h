#include <pthread.h>

#define UTC_TZ_NAME "UTC"
#define MOSCOW_TZ_NAME "Europe/Moscow"
#define BERLIN_TZ_NAME "Europe/Berlin"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int localtime_tz(const time_t *time, const char *tz_name, struct tm *result)
{

        extern char **  environ;

        size_t longest = 1024;
        char ** fakeenv = NULL;
        char ** oldenv = NULL;
        int from = 0;
        int to = 0;
        int i = 0;

        if (time == NULL || tz_name == NULL || result == NULL) {
                return -1;
        }


        for (i = 0; environ[i] != NULL; ++i)
                continue;
        fakeenv = malloc((i + 2) * sizeof *fakeenv);
        if (fakeenv == NULL || (fakeenv[0] = malloc(longest + 4)) == NULL) {
                return -1;
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
                return -1;
        }
        oldenv = environ;
        environ = fakeenv;

        strcpy(&fakeenv[0][3], tz_name);
        tzset();
        localtime_r(time, result);

        environ = oldenv;
        free(fakeenv);
        if (pthread_mutex_unlock(&mutex)) {
                return -1;
        }

        return 0;
}


int mktime_tz(const struct tm *tm, const char *tz_name, time_t *result)
{
        extern char **  environ;

        size_t longest = 1024;
        char ** fakeenv = NULL;
        char ** oldenv = NULL;
        int from = 0;
        int to = 0;
        int i = 0;

        if (!result || tm == NULL || tz_name == NULL) {
                return -1;
        }

        for (i = 0; environ[i] != NULL; ++i) {
                continue;
        }
        fakeenv = malloc((i + 2) * sizeof *fakeenv);
        if (fakeenv == NULL || (fakeenv[0] = malloc(longest + 4)) == NULL) {
                return -1;
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
                return -1;
        }
        oldenv = environ;
        environ = fakeenv;

        strcpy(&fakeenv[0][3], tz_name);
        tzset();
        *result = mktime((struct tm *)tm);

        environ = oldenv;
        free(fakeenv);
        if (pthread_mutex_unlock(&mutex)) {
                return -1;
        }

        return 0;
}

