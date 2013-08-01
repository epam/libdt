#ifdef time_tz
# include "private.h"
#endif

#include "stdio.h"  /* for stdout, stderr, perror */
#include "string.h" /* for strcpy */
#include "sys/types.h"  /* for time_t */
#include "time.h"   /* for struct tm */
#include "stdlib.h" /* for exit, malloc, atoi */
#include "float.h"  /* for FLT_MAX and DBL_MAX */
#include "limits.h" /* for CHAR_BIT, LLONG_MAX */
#include "ctype.h"  /* for isalpha et al. */


#ifndef ZDUMP_LO_YEAR
#define ZDUMP_LO_YEAR   (-500)
#endif /* !defined ZDUMP_LO_YEAR */

#ifndef ZDUMP_HI_YEAR
#define ZDUMP_HI_YEAR   2500
#endif /* !defined ZDUMP_HI_YEAR */

#ifndef MAX_STRING_LENGTH
#define MAX_STRING_LENGTH   1024
#endif /* !defined MAX_STRING_LENGTH */

#ifndef TRUE
#define TRUE        1
#endif /* !defined TRUE */

#ifndef FALSE
#define FALSE       0
#endif /* !defined FALSE */

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS    0
#endif /* !defined EXIT_SUCCESS */

#ifndef EXIT_FAILURE
#define EXIT_FAILURE    1
#endif /* !defined EXIT_FAILURE */

#ifndef SECSPERMIN
#define SECSPERMIN  60
#endif /* !defined SECSPERMIN */

#ifndef MINSPERHOUR
#define MINSPERHOUR 60
#endif /* !defined MINSPERHOUR */

#ifndef SECSPERHOUR
#define SECSPERHOUR (SECSPERMIN * MINSPERHOUR)
#endif /* !defined SECSPERHOUR */

#ifndef HOURSPERDAY
#define HOURSPERDAY 24
#endif /* !defined HOURSPERDAY */

#ifndef EPOCH_YEAR
#define EPOCH_YEAR  1970
#endif /* !defined EPOCH_YEAR */

#ifndef TM_YEAR_BASE
#define TM_YEAR_BASE    1900
#endif /* !defined TM_YEAR_BASE */

#ifndef DAYSPERNYEAR
#define DAYSPERNYEAR    365
#endif /* !defined DAYSPERNYEAR */

#ifndef isleap
#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))
#endif /* !defined isleap */

#ifndef isleap_sum
/*
 * ** See tzfile.h for details on isleap_sum.
 * */
#define isleap_sum(a, b)    isleap((a) % 400 + (b) % 400)
#endif /* !defined isleap_sum */

#define SECSPERDAY  ((int_fast32_t) SECSPERHOUR * HOURSPERDAY)
#define SECSPERNYEAR    (SECSPERDAY * DAYSPERNYEAR)
#define SECSPERLYEAR    (SECSPERNYEAR + SECSPERDAY)

#ifndef HAVE_GETTEXT
#define HAVE_GETTEXT 0
#endif
#if HAVE_GETTEXT
#include "locale.h" /* for setlocale */
#include "libintl.h"
#endif /* HAVE_GETTEXT */

#ifndef GNUC_or_lint
#ifdef lint
#define GNUC_or_lint
#else /* !defined lint */
#ifdef __GNUC__
#define GNUC_or_lint
#endif /* defined __GNUC__ */
#endif /* !defined lint */
#endif /* !defined GNUC_or_lint */

#if 2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)
# define ATTRIBUTE_PURE __attribute__ ((__pure__))
#else
# define ATTRIBUTE_PURE /* empty */
#endif

/*
 * ** For the benefit of GNU folk...
 * ** `_(MSGID)' uses the current locale's message library string for MSGID.
 * ** The default is to use gettext if available, and use MSGID otherwise.
 * */

#ifndef _
#if HAVE_GETTEXT
#define _(msgid) gettext(msgid)
#else /* !HAVE_GETTEXT */
#define _(msgid) msgid
#endif /* !HAVE_GETTEXT */
#endif /* !defined _ */

#ifndef TZ_DOMAIN
#define TZ_DOMAIN "tz"
#endif /* !defined TZ_DOMAIN */

extern int  getopt(int argc, char * const argv[],
                   const char * options);

static char *   abbr(struct tm * tmp);
static void dumptime(const struct tm * tmp);
static void show(char *zone, struct tm *tmp);

struct tm getLocalTime(char *tzName, time_t time)
{
    extern char **  environ;
    static size_t longest = 1024;
    struct tm result;
    char ** fakeenv = NULL;
    int from;
    int to;
    int i = 0;

    for (i = 0; environ[i] != NULL; ++i)
        continue;
    fakeenv = malloc((i + 2) * sizeof *fakeenv);
    if (fakeenv == NULL || (fakeenv[0] = malloc(longest + 4)) == NULL) {
        (void) perror("progname");
        exit(EXIT_FAILURE);
    }
    to = 0;
    (void) strcpy(fakeenv[to++], "TZ=");
    for (from = 0; environ[from] != NULL; ++from)
        if (strncmp(environ[from], "TZ=", 3) != 0)
            fakeenv[to++] = environ[from];
    fakeenv[to] = NULL;
    environ = fakeenv;
    (void) strcpy(&fakeenv[0][3], tzName);
    tzset();
    localtime_r(&time, &result);
    return result;
}

int
main(int argc, char *argv[])
{
    register int        i;
    register char *     cutarg;
    time_t          now;
    time_t          t;
    time_t          newt;
    struct tm       tm;
    struct tm       newtm;
    register struct tm *    tmp;
    register struct tm *    newtmp;

    (void) time(&now);

    // create new environment
    for (i = 1; i < argc; ++i) {
        struct tm ltime = getLocalTime(argv[i], now);
        show(argv[i], &ltime);
    }

    return EXIT_FAILURE;
}
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
    /*
     *  ** The packaged versions of localtime and gmtime never put
     *  out-of-range
     *      ** values in tm_wday or tm_mon, but since this code
     *      might be compiled
     *          ** with other (perhaps experimental) versions,
     *          paranoia is in order.
     *              */
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

