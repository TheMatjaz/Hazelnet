/*
 * Copyright © 2020-2022, Matjaž Guštin <dev@matjaz.it>
 * <https://matjaz.it>. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of nor the names of its contributors may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS “AS IS”
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 * @internal
 * Implementation of the hzl_OsTrng() function for different operating systems.
 */

#include "hzl_CommonInternal.h"

#if HZL_OS_AVAILABLE_WIN

/**
 * Time passed between 1601-01-01T00:00:00.000 UTC and 1970-01-01T00:00:00.000 UTC
 * in milliseconds.
 *
 * Useful to convert a time expressed in milliseconds since the Windows Epoch to one
 * since the Unix Epoch or vice-versa.
 *
 * Breakdown of the calculation:
 * - The difference is exactly 369 years: {1601, 1602, ..., 1969}
 * - Of these there are 89 leap years: {1604, 1608, ..., 1968}. Each leap year has 366 days.
 *   Note: 1700, 1800 and 1900 are NOT leap years, according to the Gregorian calendar.
 * - No leap seconds are included, as they were introduced in 1972.
 * - Thus:
 *      369 years * 365 days/year + 89 years * 1 leap_day/year = 134774 days
 *      134774 days * 24 h/day * 3600 s/h * 1000 ms/s = 11644473600000 ms
 */
#define HZL_MILLIS_FROM_WINDOWS_EPOCH_TO_UNIX_EPOCH  11644473600000ULL

/**
 * Amount of Windows ticks representing a millisecond.
 *
 * The FILETIME data structure provides time with a tick resolution of 100 ns
 * = 0.1 us = 0.0001 ms, thus 10000 ticks per millisecond.
 */
#define HZL_WINDOWS_TICKS_PER_MILLISECOND 10000U

hzl_Err_t
hzl_OsCurrentTime(hzl_Timestamp_t* const timestamp)
{
    // Timestamp is never NULL, guaranteed by the caller.
    // GetSystemTimeAsFileTime provides the amount of 100-nanosecond intervals (ticks)
    // since 1601-01-01T00:00:00.000 UTC.
    FILETIME now;
    GetSystemTimeAsFileTime(&now);
    // Convert FILETIME from a struct to total amount of ticks since the Windows epoch.
    ULARGE_INTEGER nowAsTotalTicks;
    nowAsTotalTicks.LowPart = now.dwLowDateTime;
    nowAsTotalTicks.HighPart = now.dwHighDateTime;
    // Convert the ticks since Windows epoch to milliseconds since it.
    const ULONGLONG nowAsTotalMillis = nowAsTotalTicks.QuadPart / HZL_WINDOWS_TICKS_PER_MILLISECOND;
    // Truncate the high bits, a we only need timestamps that show us a relative
    // time for a short timeframe (some days at the very most).
    // No rounding, we don't need that kind of accuracy.
    *timestamp = (hzl_Timestamp_t) nowAsTotalMillis;
    return HZL_OK;
}

#elif HZL_OS_AVAILABLE_NIX

hzl_Err_t
hzl_OsCurrentTime(hzl_Timestamp_t* const timestamp)
{
    // Timestamp is never NULL, guaranteed by the caller.
    struct timeval now;
    // gettimeofday provides the amount of seconds.microseconds since
    // 1970-01-01T00:00:00.000 UTC, a.k.a. Epoch, a.k.a. Unix time.
    if (gettimeofday(&now, NULL) == 0)
    {
        // Convert seconds and remainder microseconds since Unix Epoch to milliseconds since it.
        // Truncate the high bits, a we only need timestamps that show us a relative
        // time for a short timeframe (some days at the very most).
        // No rounding, we don't need this kind of accuracy.
        *timestamp = (hzl_Timestamp_t) ((hzl_Timestamp_t) now.tv_sec * 1000U);
        *timestamp += (hzl_Timestamp_t) ((hzl_Timestamp_t) now.tv_usec / 1000U);
        return HZL_OK;
    }
    else
    {
        return HZL_ERR_CANNOT_GET_CURRENT_TIME;
    }
}

#endif
