#include <stdlib.h>
#include <libtz/tzmapping.h>
#include <timezones_map.h>
#include <string.h>

struct tz_alias_iterator {
    tz_alias_t node;
    tz_alias_iterator_t* next;
};

struct tz_aliases {
    tz_alias_iterator_t begin;
};

static inline tz_alias_iterator_t** insert_alias(tz_aliases_t* aliases, tz_alias_t* alias, tz_alias_iterator_t** insertPosition)
{
    tz_alias_iterator_t* inserted = NULL;
    tz_alias_iterator_t* searchIt = TZMAP_START;
    tz_alias_t* searchAlias = NULL;

    if (insertPosition == NULL) {
        return NULL;
    }
    while(tzmap_iterate(aliases, &searchIt, &searchAlias) == DT_OK) {
        if(strcmp(searchAlias->name, alias->name) == 0 && alias->kind == searchAlias->kind) {
            return insertPosition;
        }
    }

    *insertPosition = malloc(sizeof(tz_alias_iterator_t));
    inserted = *insertPosition;
    inserted->next = NULL;

    inserted->node.name = malloc(strlen(alias->name) + 1);
    inserted->node.name = alias->name;
    inserted->node.kind = alias->kind;
    return &inserted->next;
}


tz_alias_iterator_t** insert_mapping(tz_aliases_t* aliases, tz_alias_iterator_t** insertPosition, struct tz_unicode_mapping* tz)
{
    tz_alias_t alias = {0,};
    alias.kind = TZMAP_UNKNOWN;



    alias.name = tz->other;
    alias.kind = TZMAP_WIN_STANDARD_TIME;

    insertPosition = insert_alias(aliases, &alias, insertPosition);

    alias.name = tz->type;
    alias.kind = TZMAP_OLSEN_NAME;

    insertPosition = insert_alias(aliases, &alias, insertPosition);

    alias.name = tz->territory;
    alias.kind = TZMAP_ABBREVIATION;

    insertPosition = insert_alias(aliases, &alias, insertPosition);

    return insertPosition;
}

dt_status_t tzmap_map(const char* tz_name, tz_aliases_t **aliases)
{
    int i = 0;
    tz_aliases_t* list = NULL;
    tz_alias_iterator_t** insertPosition = NULL;



    if(tz_name == NULL || aliases == NULL) {
        return DT_INVALID_ARGUMENT;
    }

    list = (tz_aliases_t*)malloc(sizeof(tz_aliases_t));
    *aliases = list;
    insertPosition = &list->begin.next;
    list->begin.next = NULL;


    for (i = 0; i < tz_unicode_map_size; i++) {
            struct tz_unicode_mapping tz = tz_unicode_map[i];
            if (((!strcmp(tz.type, tz_name)) ||
                 (!strcmp(tz.other, tz_name)))) {
                insertPosition = insert_mapping(list, insertPosition, &tz);
            }
        }
    if (list->begin.next == NULL) {
        tzmap_free(list);
        *aliases = NULL;
        return DT_TIMEZONE_NOT_FOUND;
    }
    return DT_OK;

}

dt_status_t tzmap_iterate(const tz_aliases_t* aliases, tz_alias_iterator_t** iterator, tz_alias_t** alias)
{
    tz_alias_iterator_t* it = NULL;
    if (aliases == NULL || iterator == NULL || alias == NULL) {
        return DT_INVALID_ARGUMENT;
    }
    if(*iterator == TZMAP_START) {
        *iterator = aliases->begin.next;
    }


    it = *iterator;
    if (it == NULL) {
        return DT_NO_MORE_ITEMS;
    }

    *alias = &it->node;
    *iterator = it->next;


    return DT_OK;
}


dt_status_t tzmap_free(tz_aliases_t *aliases)
{
    tz_alias_iterator_t* it = NULL;
    tz_alias_iterator_t* forFree = NULL;
    if (aliases == NULL) return DT_INVALID_ARGUMENT;
    it = aliases->begin.next;
    while((forFree = it) != NULL) {
        it = it->next;
        free(forFree);
    }
    return DT_OK;
}
