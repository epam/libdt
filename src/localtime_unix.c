#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int getLocalTime(const char *tzName, time_t time, struct tm *result)
{
    extern char **  environ;

    size_t longest = 1024;
    char ** fakeenv = NULL;
    char ** oldenv = NULL;
    int from = 0;
    int to = 0;
    int i = 0;

    if (!result)
        return EXIT_FAILURE;

    for (i = 0; environ[i] != NULL; ++i)
        continue;
    fakeenv = malloc((i + 2) * sizeof *fakeenv);
    if (fakeenv == NULL || (fakeenv[0] = malloc(longest + 4)) == NULL) {
        return EXIT_FAILURE;
    }

    to = 0;
    strcpy(fakeenv[to++], "TZ=");
    for (from = 0; environ[from] != NULL; ++from)
        if (strncmp(environ[from], "TZ=", 3) != 0)
            fakeenv[to++] = environ[from];
    fakeenv[to] = NULL;

    if (pthread_mutex_lock(&mutex))
        return EXIT_FAILURE;
    oldenv = environ;
    environ = fakeenv;

    strcpy(&fakeenv[0][3], tzName);
    tzset();
    localtime_r(&time, result);

    environ = oldenv;
    free(fakeenv);
    if (pthread_mutex_unlock(&mutex))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}


int getUTCTime(const char *tzName, struct tm time, time_t *result)
{
    extern char **  environ;

    size_t longest = 1024;
    char ** fakeenv = NULL;
    char ** oldenv = NULL;
    int from = 0;
    int to = 0;
    int i = 0;

    if (!result)
        return EXIT_FAILURE;

    for (i = 0; environ[i] != NULL; ++i)
        continue;
    fakeenv = malloc((i + 2) * sizeof *fakeenv);
    if (fakeenv == NULL || (fakeenv[0] = malloc(longest + 4)) == NULL) {
        return EXIT_FAILURE;
    }

    to = 0;
    strcpy(fakeenv[to++], "TZ=");
    for (from = 0; environ[from] != NULL; ++from)
        if (strncmp(environ[from], "TZ=", 3) != 0)
            fakeenv[to++] = environ[from];
    fakeenv[to] = NULL;

    if (pthread_mutex_lock(&mutex))
        return EXIT_FAILURE;
    oldenv = environ;
    environ = fakeenv;

    strcpy(&fakeenv[0][3], tzName);
    tzset();
    *result = mktime(&time);

    environ = oldenv;
    free(fakeenv);
    if (pthread_mutex_unlock(&mutex))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
