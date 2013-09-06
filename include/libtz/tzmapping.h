#ifndef TZMAPPING_H
#define TZMAPPING_H
#include <libtz/dt_types.h>

typedef enum {
    TZA_ABBREVIATION,
    TZA_WIN_STANDARD_TIME,
    TZA_OLSEN_NAME,
    TZA_UNKNOWN
} tz_alias_kind_t;

typedef struct tz_alias_iterator tz_alias_iterator_t;
typedef struct tz_aliases tz_aliases_t;
typedef struct tz_alias {
    tz_alias_kind_t kind;
    const char* name;
} tz_alias_t;

#define TZMAP_START (tz_alias_iterator_t*)0x1
#ifdef __cplusplus
extern "C" {
#endif

dt_status_t tzmap_map(const char* tz_name, tz_aliases_t **aliases);
dt_status_t tzmap_iterate(const tz_aliases_t* aliases, tz_alias_iterator_t** iterator, tz_alias_t **alias);
dt_status_t tzmap_free(tz_aliases_t* aliases);
#ifdef __cplusplus
}
#endif
#endif // TZMAPPING_H
