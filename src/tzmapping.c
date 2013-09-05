#include <stdlib.h>
#include <libtz/tzmapping.h>

dt_status_t tzmap_map(const char* tz_name, tz_aliases_t **aliases)
{
    if(tz_name == NULL || aliases == NULL) {
        return DT_INVALID_ARGUMENT;
    }
    return DT_UNKNOWN_ERROR;
}

dt_status_t tzmap_iterate(const tz_aliases_t* aliases, tz_alias_iterator_t** iterator, tz_alias_t** alias)
{
    return DT_UNKNOWN_ERROR;
}
