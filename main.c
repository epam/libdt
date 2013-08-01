#include "stdio.h"  /* for stdout, stderr, perror */
#include "time.h"   /* for struct tm */
#include "stdlib.h" /* for exit, malloc, atoi */
#include "string.h"

static char *abbr(struct tm * tmp);
static void dumptime(const struct tm * tmp);
static void show(char *zone, struct tm *tmp);

/** function for convert time_t to struct tm with specific Time Zone.
 * @param tzName - name of time zone. Must be in format <Area>/<Place, such as Europe/Moscow or Asia/Oral.
 * @param time - time to format
 * @return NULL in error case, local time in other case
 */
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

    oldenv = environ;
    environ = fakeenv;

    strcpy(&fakeenv[0][3], tzName);
    tzset();
    result = localtime(&time);

    environ = oldenv;
    free(fakeenv);
    return result;
}

int
main(int argc, char *argv[])
{
    int        i;
    time_t          now;

    (void) time(&now);

    // create new environment
    for (i = 1; i < argc; ++i) {
        struct tm *ltime = getLocalTime(argv[i], now);
        if (ltime)
            show(argv[i], ltime);
    }

    return EXIT_FAILURE;
}

// for example
static void
show(char *zone, struct tm *tmp)
{
    (void) printf("%s  ", zone);
    dumptime(tmp);
    (void) printf(" UTC");
    (void) printf(" = ");
    dumptime(tmp);
    if (tmp != NULL) {
        if (*abbr(tmp) != '\0')
            (void) printf(" %s", abbr(tmp));
    }
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

static void
dumptime(register const struct tm *timeptr)
{
    static const char   wday_name[][3] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    static const char   mon_name[][3] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    register const char *   wn;
    register const char *   mn;
    register int        lead;
    register int        trail;

    if (timeptr == NULL) {
        (void) printf("NULL");
        return;
    }
    if (timeptr->tm_wday < 0 || timeptr->tm_wday >=
            (int) (sizeof wday_name / sizeof wday_name[0]))
        wn = "???";
    else        wn = wday_name[timeptr->tm_wday];
    if (timeptr->tm_mon < 0 || timeptr->tm_mon >=
            (int) (sizeof mon_name / sizeof mon_name[0]))
        mn = "???";
    else        mn = mon_name[timeptr->tm_mon];
    (void) printf("%.3s %.3s%3d %.2d:%.2d:%.2d ",
                  wn, mn,
                  timeptr->tm_mday, timeptr->tm_hour,
                  timeptr->tm_min, timeptr->tm_sec);
#define DIVISOR 10
#ifndef TM_YEAR_BASE
#define TM_YEAR_BASE    1900
#endif /* !defined TM_YEAR_BASE */
    trail = timeptr->tm_year % DIVISOR + TM_YEAR_BASE % DIVISOR;
    lead = timeptr->tm_year / DIVISOR + TM_YEAR_BASE / DIVISOR +
            trail / DIVISOR;
    trail %= DIVISOR;
    if (trail < 0 && lead > 0) {
        trail += DIVISOR;
        --lead;
    } else if (lead < 0 && trail > 0) {
        trail -= DIVISOR;
        ++lead;
    }
    if (lead == 0)
        (void) printf("%d", trail);
    else    (void) printf("%d%d", lead, ((trail < 0) ? -trail : trail));
}

