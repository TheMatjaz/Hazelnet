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
 * Hazelnet wrapper of the Hash function.
 *
 * The idea of this wrapper is to provide a stable interface in case the
 * implementation of the hash function is swapped with another.
 */

#include "hzl_CommonHash.h"
#include "ascon.h"

void
hzl_HashInit(hzl_Hash_t* const ctx)
{
    ascon_hash_xof_init(ctx);
}

void
hzl_HashUpdate(hzl_Hash_t* const ctx,
               const uint8_t* data,
               const size_t dataLen)
{
    ascon_hash_xof_update(ctx, data, dataLen);
}

void
hzl_HashDigest(hzl_Hash_t* const ctx,
               uint8_t* const digest,
               const size_t digestLen)
{
    ascon_hash_xof_final(ctx, digest, digestLen);
}

hzl_Err_t
hzl_HashDigestCheck(hzl_Hash_t* const ctx,
                    const uint8_t* const expectedDigest,
                    const size_t digestLen)
{
    if (ascon_hash_xof_final_matches(ctx, expectedDigest, digestLen) == ASCON_TAG_OK)
    {
        return HZL_OK;
    }
    else
    {
        return HZL_ERR_SECWARN_INVALID_TAG;
    }
}
