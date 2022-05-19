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

hzl_Err_t
hzl_OsTrng(uint8_t* const buffer, const size_t amount)
{
    // Casting "amount" variable as the Hazelnet implementation never requires more
    // than a few bytes (usually 8 or 16) at the time. size_t is used just so the
    // user can use sizeof(buffer) when calling the function. ULONG is >= 32 bits anyhow.
    if (BCryptGenRandom(NULL, buffer, (ULONG) amount, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0)
    {
        return HZL_OK;
    }
    else
    {
        return HZL_ERR_CANNOT_GENERATE_RANDOM;
    }
}

#elif HZL_OS_AVAILABLE_NIX

hzl_Err_t
hzl_OsTrng(uint8_t* const buffer, const size_t amount)
{
    FILE* urandom = fopen("/dev/urandom", "r");
    size_t obtained = 0;
    if (urandom != NULL)
    {
        obtained = fread(buffer, sizeof(uint8_t), amount, urandom);
        fclose(urandom);
    }
    if (obtained == amount) { return HZL_OK; }
    else
    {
        return HZL_ERR_CANNOT_GENERATE_RANDOM;
    }
}

#endif
