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

#if defined(_WIN32) && !defined(_TIMESPEC_DEFINED)
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

    //! Converts a localized representation to POSIX breakdown time structure. Nano seconds will be lost!
    /*!
     * \param representation Representation object
     * \param tm POSIX breakdown time structure for the representation [OUT]
     * \return Result status of the operation
     * \note This method does not validate representaion so invalid POSIX breakdown time structure could be returned
     */
    dt_status_t dt_representation_to_tm(const dt_representation_t *representation, struct tm *tm);

    //! Converts a POSIX breakdown time structure to representation
    /*!
     * \param tm POSIX breakdown time structure
     * \param nano_second Nano-second part of the representation
     * \param representation Representation of POSIX breakdown time structure [OUT]
     * \return Result status of the operation
     * \note Invalid representation could be returned if an invalid POSIX breakdown time structure has been provided
     */
    dt_status_t dt_tm_to_representation(const struct tm *tm, long nano_second, dt_representation_t *representation);

    /*! @}*/

#ifdef __cplusplus
}
#endif
#endif // DT_CONVERT_H
