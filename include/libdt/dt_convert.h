#ifndef DT_CONVERT_H
#define DT_CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif
    /*!
     * \defgroup TypeConversion Type conversion functions
     * @{
     */

    //! Loads timestamp from POSIX time
    /*!
     * \note POSIX time (time_t) supports greater than zero values only
     * \param time POSIX time value
     * \param nano_second Nano-seconds part of the timestamp
     * \param result Timestamp for the POSIX time value [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_posix_time_to_timestamp(time_t time, unsigned long nano_second, dt_timestamp_t *result);

    //! Converts timestamp to POSIX time value
    /*!
     * Function returns DT_INVALID_ARGUMENT if a timestamp could not be represented by POSIX time.
     * \note POSIX time (time_t) supports greater than zero values only
     * \param timestamp Timestamp to convert
     * \param time POSIX time for the timestamp [OUT]
     * \param nano_second Optional nano-seconds part of the timestamp (0L-999999999L), could be NULL if not used [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_timestamp_to_posix_time(const dt_timestamp_t *timestamp, time_t *time, unsigned long *nano_second);

#ifdef _WIN32
    struct timespec {
        time_t tv_sec;                          //!< Seconds
        long tv_nsec;                           //!< Nano-seconds
    };
#endif
    //! Converts interval to timespec structure
    /*!
     * \param interval Interval to convert to timespec structure
     * \param result Timespec structure [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_interval_to_timespec(const dt_interval_t *interval, struct timespec *result);

    //! Converts timespec structure to interval
    /*!
     * \param ts Timespec structure
     * \param result Interval [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_timespec_to_interval(const struct timespec *ts, dt_interval_t *result);

    //! Converts a localized representation to POSIX breakdown time structure. Nano seconds will be losted!
    /*!
     * \param representation Representation object
     * \param tm POSIX breakdown time structure for the representation [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_representation_to_tm(const dt_representation_t *representation, struct tm *tm);

    //! Converts a POSIX breakdown time structure to representation
    /*!
     * \param tm POSIX breakdown time structure
     * \param nano_second Nano-second part of the representation
     * \param representation Representation of POSIX breakdown time structure [OUT]
     * \return Result status of the operation
     */
    dt_status_t dt_tm_to_representation(const struct tm *tm, long nano_second, dt_representation_t *representation);

    /*! @}*/

#ifdef __cplusplus
}
#endif
#endif // DT_CONVERT_H
