// vim: shiftwidth=4 softtabstop=4
/* Copyright (c) 2013, EPAM Systems. All rights reserved.

Authors:
Ilya Storozhilov <Ilya_Storozhilov@epam.com>,
Andrey Kuznetsov <Andrey_Kuznetsov@epam.com>,
Maxim Kot <Maxim_Kot@epam.com>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#ifndef TZMAPPING_H
#define TZMAPPING_H

#include <libdt/export.h>
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
    LIBDT_EXPORT dt_status_t tzmap_map(const char *tz_name, tz_aliases_t **aliases);

    //! Iterates through tz_aliases_t collection
    /*!
     * @param aliases colection to iterate through
     * @param iterator pointer for iterator which will be used to iterate through collection
     * @param alias pointer to store finded alias
     * @return DT_OK on success otherwise corresponding status from dt_status_t
     */
    LIBDT_EXPORT dt_status_t tzmap_iterate(const tz_aliases_t *aliases, tz_alias_iterator_t **iterator, tz_alias_t **alias);

    //! Free resources of tz_aliases_t collection
    /*!
     * @param aliases colection which resources will be free
     * @return DT_OK on success otherwise corresponding status from dt_status_t
     */
    LIBDT_EXPORT dt_status_t tzmap_free(tz_aliases_t *aliases);

    /*! @}*/
#ifdef __cplusplus
}
#endif

#endif // TZMAPPING_H
