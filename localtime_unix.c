#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct tm * getLocalTime(const char *tzName, time_t time)
{
    extern char **  environ;

    size_t longest = 1024;
    char ** fakeenv = NULL;
    char ** oldenv = NULL;
    int from = 0;
    int to = 0;
    int i = 0;
    struct tm *result = NULL;
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    for (i = 0; environ[i] != NULL; ++i)
        continue;
    fakeenv = malloc((i + 2) * sizeof *fakeenv);
    if (fakeenv == NULL || (fakeenv[0] = malloc(longest + 4)) == NULL) {
        return NULL;
    }

    to = 0;
    strcpy(fakeenv[to++], "TZ=");
    for (from = 0; environ[from] != NULL; ++from)
        if (strncmp(environ[from], "TZ=", 3) != 0)
            fakeenv[to++] = environ[from];
    fakeenv[to] = NULL;

    pthread_mutex_lock(&mutex);
    oldenv = environ;
    environ = fakeenv;

    strcpy(&fakeenv[0][3], tzName);
    tzset();
    result = localtime(&time);

    environ = oldenv;
    free(fakeenv);
    pthread_mutex_unlock(&mutex);

    return result;
}
