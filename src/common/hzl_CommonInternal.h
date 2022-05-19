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
 * Hazelnet internal header with utilities and functions used across other .c modules.
 */

#ifndef HZL_INTERNAL_H_
#define HZL_INTERNAL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "hzl.h"

/** @internal Length of the Group Identifier in bytes. */
#define HZL_GID_LEN 1U

/** @internal Length of the Source Identifier in bytes. */
#define HZL_SID_LEN 1U

/** @internal Length of the Payload Type identifier in bytes. */
#define HZL_PTY_LEN 1U

/** @internal Length of the Counter Nonce in bytes. */
#define HZL_CTRNONCE_LEN 3U

/** @internal Value of a Request Nonce in the local state when no Request was transmitted,
 * thus no Response is currently expected. */
#define HZL_REQNONCE_NOT_EXPECTING_A_RESPONSE 0U

/** @internal Maximum value of a uint24 Counter Nonce,
 * being also the value indicating expiration. */
#define HZL_MAX_CTRNONCE 0xFFFFFFU

/** @internal Macro checking if the Counter Nonce has reached is maximum and it should
 * not be used anymore. */
#define HZL_IS_CTRNONCE_EXPIRED(ctr) ((ctr) >= HZL_MAX_CTRNONCE)

/** @internal Amount of possible values the timestamp can have.
 * A 32 bit integer, thus 2^32 values. */
#define HZL_TIMESTAMP_DOMAIN_SIZE (1ULL << 32U)

/** @internal Syntax sugar macro to declare and initialse an error code in a consistent way. */
#define HZL_ERR_DECLARE(err) hzl_Err_t err = HZL_ERR_PROGRAMMING

/** @internal Syntax sugar macro to return if the error code indicates something went wrong. */
#define HZL_ERR_CHECK(err) if ((err) != HZL_OK) { return (err); }

/** @internal Syntax sugar macro to go to the "cleanup" section if the error code indicates
 * something went wrong. */
#define HZL_ERR_CLEANUP(err) if ((err) != HZL_OK) { goto cleanup; }

/** @internal Difference between two opaque timestamps expressed in clear in milliseconds. */
typedef uint32_t hzl_TimeDeltaMillis_t;

/**
 * @internal
 * Zeroes-out the memory section.
 *
 * Wrapper of memset to guarantee the compiler does not optimise it away.
 *
 * @param [in] buffer memory section to set to all-zeros
 * @param [in] amountOfBytes length of \p buffer in bytes
 */
void
hzl_ZeroOut(void* buffer, size_t amountOfBytes);

/**
 * @internal
 * Checks if all bytes are set to zero.
 *
 * @param [in] bytes to check
 * @param [in] amount number of bytes
 *
 * @return true if they are all zero, false otherwise
 */
bool
hzl_IsAllZeros(const uint8_t* bytes, size_t amount);

/**
 * @internal
 * Computes the elapsed time in milliseconds between two timestamps.
 *
 * Handles also one (and only one) roll-around of the \p end timestamp. The assumption is
 * that the \p end timestamp represents an instant after \p start, even if it the timer rolled
 * around once. In the examples below, the time delta (result of this function) is represented
 * by the dots.
 *
 *         0| ___________start.....end_____ |0xFFFFFFFF
 *         0| ...end__start................ |0xFFFFFFFF
 *
 * @param [in] start timestamp earlier in time
 * @param [in] end timestamp later in time, after \p start
 *
 * @return elapsed milliseconds between the two timestamps.
 */
hzl_TimeDeltaMillis_t
hzl_TimeDelta(hzl_Timestamp_t start,
              hzl_Timestamp_t end);

/**
 * @internal
 * Wrapper of the TRNG function making sure the output bytes are not all-zeros.
 *
 * @param [out] bytes where to write the random bytes
 * @param [in] trng the TRNG function
 * @param [in] amount number of random bytes to write to \p buffer
 *
 * @retval #HZL_OK on success.
 * @retval #HZL_ERR_CANNOT_GENERATE_RANDOM if the random generation fails.
 * @retval #HZL_ERR_CANNOT_GENERATE_NON_ZERO_RANDOM if the random generation is unable
 *         to provide non-zero arrays of bytes.
 *
 * @see #hzl_TrngFunc
 * @see #HZL_MAX_TRNG_TRIES_FOR_NONZERO_VALUE
 */
hzl_Err_t
hzl_NonZeroTrng(uint8_t* bytes,
                hzl_TrngFunc trng,
                size_t amount);

#if HZL_OS_AVAILABLE

/** @internal Implementation of the hzl_ClientNewMsg() and hzl_ServerNewMsg()/ */
hzl_Err_t
hzl_CommonNewMsg(hzl_CbsPduMsg_t** pMsg);

/** @internal Implementation of the hzl_ClientFreeMsg() and hzl_ServerFreeMsg()/ */
void
hzl_CommonFreeMsg(hzl_CbsPduMsg_t** pMsg);

/**
 * @internal
 * True random number generator function using the underlying desktop Operating System.
 *
 * @param [out] buffer where to write the random bytes
 * @param [in] amount number of random bytes to write to \p buffer
 *
 * @retval #HZL_OK on success
 * @retval #HZL_ERR_CANNOT_GENERATE_RANDOM if the random generation fails
 *
 * @see #hzl_TrngFunc
 */
hzl_Err_t
hzl_OsTrng(uint8_t* buffer, size_t amount);

/**
 * @internal
 * Timestamping function providing the current time using the underlying desktop Operating System.
 *
 * @param [out] timestamp current time

 * @retval #HZL_OK on success
 * @retval #HZL_ERR_CANNOT_GET_CURRENT_TIME is the timestamp generation fails
 *
 * @see #hzl_TimestampFunc
 */
hzl_Err_t
hzl_OsCurrentTime(hzl_Timestamp_t* timestamp);

/**
 * @internal
 * Zeros-out the memory region, frees it and sets the pointer to it to NULL, to avoid
 * use-after-free.
 *
 * Casts the pointers to writable pointers.
 *
 * @param [in, out] pointer pointer to allocated data. Is set to NULL after freeing.
 *        If NULL, the function does nothing.
 * @param [in] lengthInBytes size of the memory region to clear before freeing. Set to 0 to avoid
 *        clearing.
 */
#define HZL_SECURE_FREE(pointer, lengthInBytes) \
    if (pointer) { \
        hzl_ZeroOut((void*) (pointer), (lengthInBytes)); \
        free((void*) (pointer)); \
        (pointer) = NULL; \
    }

#endif  /* HZL_OS_AVAILABLE */

#ifdef __cplusplus
}
#endif

#endif  /* HZL_INTERNAL_H_ */
