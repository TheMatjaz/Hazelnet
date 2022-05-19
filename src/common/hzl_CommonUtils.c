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
 * Simple utility functions.
 */

#include "hzl_CommonInternal.h"

bool
hzl_IsAllZeros(const uint8_t* bytes, size_t amount)
{
    while (amount--)
    {
        if (*(bytes++) != 0U) { return false; }
    }
    return true;
}

hzl_Err_t
hzl_NonZeroTrng(uint8_t* const bytes,
                hzl_TrngFunc const trng,
                const size_t amount)
{
    HZL_ERR_DECLARE(err);
    if (trng == NULL) { return HZL_ERR_NULL_TRNG_FUNC; }
    size_t tries = 0;
    do
    {
        err = trng(bytes, amount);
        HZL_ERR_CHECK(err);
        if (tries > HZL_MAX_TRNG_TRIES_FOR_NONZERO_VALUE)
        {
            return HZL_ERR_CANNOT_GENERATE_NON_ZERO_RANDOM;
        }
        tries++;
    }
    while (hzl_IsAllZeros(bytes, amount));
    return err;
}

void
hzl_ZeroOut(void* const buffer, size_t amountOfBytes)
{
#if defined(memset_s)
    // C11 standard function, guaranteed to set the memory and not to be optimised away by the
    // compiler, but noy many compilers implement it as of 2021.
    memset_s(buffer, amountOfBytes, 0, amountOfBytes);
#else
    // Trying to cast the destination memory to volatile and writing manually,
    // but IT STILL MAY NOT WORK. See the following links for details
    // https://www.daemonology.net/blog/2014-09-04-how-to-zero-a-buffer.html
    // https://www.daemonology.net/blog/2014-09-05-erratum.html
    // https://www.daemonology.net/blog/2014-09-06-zeroing-buffers-is-insufficient.html
    volatile uint8_t* volBuffer = buffer;
    while (amountOfBytes--)
    {
        *volBuffer++ = 0;
    }
#endif
}
