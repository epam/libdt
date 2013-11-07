// vim: shiftwidth=4 softtabstop=4

#ifndef TZMAPPING_H
#define TZMAPPING_H

#include <libdt/dt_types.h>

/*!
 * \defgroup TimezoneMapping Mapping from olsen to standard windows time and back
 * @{
 */

//! Alias kinds
typedef enum {
    DT_TZMAP_ABBREVIATION,         //!< Abbreviation or territory
    DT_TZMAP_WIN_STANDARD_TIME,    //!< Windows standard time
    DT_TZMAP_OLSEN_NAME,           //!< Olsen name
    DT_TZMAP_UNKNOWN               //!< Unknown alias kind, used for error cases ususualy
} tz_alias_kind_t;

#if defined(__CYGWIN__) || defined(WIN32)
#define DT_PREFFERED_TZMAP_TYPE DT_TZMAP_WIN_STANDARD_TIME
#else
#define DT_PREFFERED_TZMAP_TYPE DT_TZMAP_OLSEN_NAME
#endif

//! Iterator for enum available alliases
typedef struct tz_alias_iterator tz_alias_iterator_t;

//! Aliases collection
typedef struct tz_aliases tz_aliases_t;

//! Timezone name alias
typedef struct tz_alias {
    tz_alias_kind_t kind;       //!< Alias kind
    const char *name;           //!< Alternative time zone name
} tz_alias_t;

//! Start iterator for any tz_aliases_t collection
#define TZMAP_BEGIN (tz_alias_iterator_t*)0x1

#ifdef __cplusplus
extern "C" {
#endif

    //! Creates tz_aliases_t collection, by given timezone name
    //! You must manually free aliases after usage by tzmap_free() if this function has been called successfully
    /*!
     * @param tz_name name of time zone for maping
     * @param aliases pointer to store result of function
     * @return DT_OK on success otherwise corresponding status from dt_status_t
     * @sa tzmap_free
     */
    dt_status_t tzmap_map(const char *tz_name, tz_aliases_t **aliases);

    //! Iterates through tz_aliases_t collection
    /*!
     * @param aliases colection to iterate through
     * @param iterator pointer for iterator which will be used to iterate through collection
     * @param alias pointer to store finded alias
     * @return DT_OK on success otherwise corresponding status from dt_status_t
     */
    dt_status_t tzmap_iterate(const tz_aliases_t *aliases, tz_alias_iterator_t **iterator, tz_alias_t **alias);

    //! Free resources of tz_aliases_t collection
    /*!
     * @param aliases colection which resources will be free
     * @return DT_OK on success otherwise corresponding status from dt_status_t
     */
    dt_status_t tzmap_free(tz_aliases_t *aliases);

    /*! @}*/
#ifdef __cplusplus
}
#endif

#endif // TZMAPPING_H
