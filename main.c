#include <stdio.h>  /* for stdout, stderr, perror */
#include <time.h>   /* for struct tm */
#include <stdlib.h> /* for exit, malloc, atoi */

#include "localtime.h"

static char *abbr(struct tm * tmp);
static void dumptime(const struct tm * tmp);
static void show(char *zone, struct tm *tmp);

int
main(int argc, char *argv[])
{
    int        i;
    time_t          now;

    (void) time(&now);

    // create new environment
    for (i = 1; i < argc; ++i) {
        struct tm ltime;
        if (!localtime_tz(&now, argv[i], &ltime))
            show(argv[i], &ltime);
    }

    return EXIT_FAILURE;
}

// for example
static void
show(char *zone, struct tm *tmp)
{
    time_t nowTime = 0;
    (void) printf("%s = ", zone);
    printf("%s", asctime(tmp));
    if (tmp != NULL) {
        if (*abbr(tmp) != '\0')
            (void) printf(" %s", abbr(tmp));
    }


    (void) printf(", UTC");
    (void) printf(" = ");
    mktime_tz(tmp, zone, &nowTime);
    printf("%s", asctime(gmtime(&nowTime)));
    (void) printf("\n");
}

static char *
abbr(struct tm *tmp)
{
    extern char *   tzname[2];
    register char * result;
    static char nada;

    if (tmp->tm_isdst != 0 && tmp->tm_isdst != 1)
        return &nada;
    result = tzname[tmp->tm_isdst];
    return (result == NULL) ? &nada : result;
}
