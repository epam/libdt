#ifndef TZ_H
#define TZ_H

#if BUILDING_LIBTZ && HAVE_VISIBILITY
#define LIBTZ_DLL_EXPORTED __attribute__((__visibility__("default")))
#else
#define LIBTZ_DLL_EXPORTED
#endif

LIBTZ_DLL_EXPORTED const struct state *tz_alloc(register const char * name);
LIBTZ_DLL_EXPORTED void tz_free(const struct state * const sp);

LIBTZ_DLL_EXPORTED struct tm * tz_localtime(const struct state * const sp, const time_t * const	timep);
LIBTZ_DLL_EXPORTED struct tm * tz_localtime_r(const struct state * const sp, const time_t * const timep, struct tm * tmp);

LIBTZ_DLL_EXPORTED time_t tz_mktime(const struct state * const sp, struct tm * const tmp);

#endif /* TZ_H */
