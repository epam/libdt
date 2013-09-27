#ifndef _DT_H
#define _DT_H

/*
 * Cross-platform date/time handling library for C.
 * Main header file.
 *
 * Authors: Ilya Storozhilov <Ilya_Storozhilov@epam.com>, Andrey Kuznetsov
 * <Andrey_Kuznetsov@epam.com>, Maxim Kot <Maxim_Kot@epam.com>
 * License: Public Domain, http://en.wikipedia.org/wiki/Public_domain
 */


#include <libdt/dt_types.h>
#include <libdt/dt_convert.h>
#ifdef __cplusplus
extern "C" {
#endif

    /*!
     * \defgroup GeneralPurpose General purpose functions
     * @{
     */

    //! Checks for leap year
    /*!
     * \param year Year to check (Non-positive years rre always not leap)
     * \return Check result
     */
    dt_bool_t dt_is_leap_year(int year);

    //! Returns string represenatation of the datetime error code
    const char *dt_strerror(dt_status_t status);

    //! Validates a date/time representation values set
    /*!
     * \param year Year
     * \param month Month
     * \param day Day
     * \param hour Hour
     * \param minute Minute
     * \param second Second
     * \param nano_second Nano-second
     * \return Validation result
     */
    //TODO: Leap seconds support!
    //TODO: Refactor it like validate_timestamp or validate_interval
    dt_bool_t dt_validate_representation(int year, unsigned short month, unsigned short day, unsigned short hour, unsigned short minute, unsigned short second,
                                         unsigned long nano_second);

    /*! @}*/

    /*!
     * \defgroup Timestamp Timestamp functions
     * @{
     */


    /*!
     * \brief dt_validate_timestamp Validates timestamp
     * \param timestamp timestamp to validation
     * \return DT_TRUE if timestamp is valid, otherwise DT_FALSE
     */
    dt_bool_t dt_validate_timestamp(const dt_timestamp_t *timestamp);

    //! Returns a current timestamp
    /*!
     * \param result Current timestamp [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_now(dt_timestamp_t *result);

    //! Compares two timestamps
    /*!
     * \param lhs First timestamp to compare
     * \param rhs Second timestamp to compare
     * \param result Zero if timestamps are equal, less than zero if first timestamp is before second and more than zero otherwise [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_compare_timestamps(const dt_timestamp_t *lhs, const dt_timestamp_t *rhs, dt_compare_result_t *result);

    //! Obtains an offset between timestamps
    /*!
     * \param lhs First timestamp
     * \param rhs Second timestamp
     * \param result Offset b/w timestamps [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_offset_between(const dt_timestamp_t *lhs, const dt_timestamp_t *rhs, dt_offset_t *result);

    /*!
     * \brief dt_validate_offset Validates offset
     * \param offset offset to validating
     * \return DT_TRUE if offset is valid, otherwise DT_FALSE
     */
    dt_bool_t dt_validate_offset(const dt_offset_t *offset);

    //! Applies offset on the timestamp
    /*!
     * \param lhs Timestamp to apply the offset on
     * \param rhs Offset to apply on timestamp
     * \param result Result offset [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_apply_offset(const dt_timestamp_t *lhs, const dt_offset_t *rhs, dt_timestamp_t *result);

    /*! @}*/

    /*!
     * \defgroup Interval Interval functions
     * @{
     */

    //! Initializes time interval
    /*!
     * Seconds part is to be subtracted from nano-seconds and appended to seconds
     * \param seconds Seconds
     * \param nano_seconds Nano-seconds
     * \param result interval to initialize [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_init_interval(unsigned long seconds, unsigned long nano_seconds, dt_interval_t *result);

    /*! Validates interval
     * \param test interval to validation
     * \return DT_FALSE if test is NULL or test is not valid, otherwise DT_TRUE
    */
    dt_bool_t dt_validate_interval(const dt_interval_t *test);

    //! Compares two intervals
    /*!
     * \param lhs First interval to compare
     * \param rhs Second interval to compare
     * \param result Zero if intervals are equal, less than zero if first intervals is less than second and more than zero otherwise [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_compare_intervals(const dt_interval_t *lhs, const dt_interval_t *rhs, dt_compare_result_t *result);

    //! Summarizes two intervals
    /*!
     * \param lhs First interval to summarize
     * \param rhs Second interval to summarize
     * \param result Result interval [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_sum_intervals(const dt_interval_t *lhs, const dt_interval_t *rhs, dt_interval_t *result);

    //! Subtracts two intervals
    /*!
     * \note Zero interval is returned if the greater is subtracting from the lesser one
     * \param lhs First interval to subtract
     * \param rhs Second interval to subtract
     * \param result Result interval [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_sub_intervals(const dt_interval_t *lhs, const dt_interval_t *rhs, dt_interval_t *result);

    //! Multiplies an interval
    /*!
     * \param lhs First interval to multiply
     * \param rhs Multiplier
     * \param result Result interval [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_mul_interval(const dt_interval_t *lhs, double rhs, dt_interval_t *result);

    /*! @}*/

    /*!
     * \defgroup Representation Representation functions
     * @{
     */

    //! Initializes representetion with error checking
    /*!
     * \param year Year
     * \param month Month
     * \param day Day
     * \param hour Hour
     * \param minute Minute
     * \param second Second
     * \param nano_second Nano-second
     * \param result Representation to init [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_init_representation(int year, unsigned short month, unsigned short day, unsigned short hour, unsigned short minute, unsigned short second,
                                       unsigned long nano_second,
                                       dt_representation_t *result);

    //! Lookups timezone object for future usage in corresponding api.
    //! If function succesed timezone object must be freed with dt_timezone_cleanup function
    /*!
     * \param timezone_name name of timezone for lookup, it can be in olsen database format, or in windows standard time format
     * \param timezone [IN/OUT]pointer to timezone object
     * \return Result status of the operation
     * \sa dt_timezone_cleanup
     */
    dt_status_t dt_timezone_lookup(const char *timezone_name, dt_timezone_t *timezone);

    //! Free resources connected with timezone object
    //! @note memory allocated for dt_timezone_t objec won't be free
    /*!
     * \param timezone pointer to timezone object
     * \return Result status of the operation
     */
    dt_status_t dt_timezone_cleanup(dt_timezone_t *timezone);

    //! Represents a timestamp using a timezone name
    /*!
     * \param timestamp Timestamp to represent
     * \param timezone Timezone or NULL if local timezone is considered
     * \param representation Timestamp representation [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_timestamp_to_representation(const dt_timestamp_t *timestamp, const dt_timezone_t *timezone, dt_representation_t *representation);

    //! Returns a timestamps for a representation in timezone by it's name
    /*!
     * It is possible for the representation to have two timestamps, e.g. when a time is "going back" for
     * an hour.
     * \param representation Representation to fetch a timestamp of
     * \param timezone Timezone or NULL if local timezone is considered
     * \param first_timestamp First representation's timestamp [OUT]
     * \param second_timestamp Optional second representation's timestamp (can be NULL, not supported at the moment) [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_representation_to_timestamp(const dt_representation_t *representation, const dt_timezone_t *timezone,
                                               dt_timestamp_t *first_timestamp, dt_timestamp_t *second_timestamp);

    //! Returns representation's week day number
    /*!
     * \param representation Representation object
     * \param day_of_week Representation's day of week (1-7, 1 is Sunday) [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_representation_day_of_week(const dt_representation_t *representation, int *day_of_week);

    //! Returns representation's day of year
    /*!
     * \param representation Representation object
     * \param day_of_year Representation's day of year (1-365/366) [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_representation_day_of_year(const dt_representation_t *representation, int *day_of_year);

    /*! @}*/

    /*!
     * \defgroup StringConversion String conversion functions
     * @{
     */

    //! Converts representation to string
    /*!
     * \param representation Representation to convert
     * \param fmt Format string, see strptime()/strftime() plus "%f" for nano-seconds
     * \param str_buffer Buffer to fill [OUT]
     * \param str_buffer_size A size of the buffer to fill
     * \return Result status of the operation
     */
    dt_status_t dt_to_string(const dt_representation_t *representation, const char *fmt,
                             char *str_buffer, size_t str_buffer_size);

    //! Converts string to representation
    /*!
     * \param str A NULL-terminated string to parse
     * \param fmt Format string, see strptime()/strftime() plus "%f" for nano-seconds
     * \param representation Representation object to fill [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_from_string(const char *str, const char *fmt, dt_representation_t *representation);

    /*! @}*/

#ifdef __cplusplus
}
#endif

#endif // _DT_H
