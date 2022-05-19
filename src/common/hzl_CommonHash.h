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
 * Changing the Hash function means updating the typedef hzl_Hash_t in this file
 * and updating the functions in the hzl_Hash.c file.
 */

#ifndef HZL_HASH_H_
#define HZL_HASH_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "hzl_CommonInternal.h"
#include "ascon.h"

/**
 * State of the hash function.
 */
typedef ascon_hash_ctx_t hzl_Hash_t;

/**
 * Initialises the hash function to process data.
 *
 * @param [in, out] ctx to initialise
 */
void
hzl_HashInit(hzl_Hash_t* ctx);

/**
 * Feeds the given data into the hash state.
 *
 * @param [in, out] ctx context already initialised.
 * @param [in] data part of the message to hash.
 * @param [in] dataLen length of \p data in bytes.
 */
void
hzl_HashUpdate(hzl_Hash_t* ctx,
               const uint8_t* data,
               size_t dataLen);

/**
 * Finalises the processed data into a digest of the desired length.
 * Securely cleans its own state after completion.
 *
 * @param [in, out] ctx context with message already processed.
 * @param [in] digest location where to write the message digest.
 * @param [in] digestLen length of \p digest in bytes.
 */
void
hzl_HashDigest(hzl_Hash_t* ctx,
               uint8_t* digest,
               size_t digestLen);

/**
 * Finalises the processed data into a digest of the desired length and
 * checks it's equal to the expected one.
 * Securely cleans its own state after completion.
 *
 * @param [in, out] ctx context with message already processed.
 * @param [in] expectedDigest digest that came with the message, proving its
 *        integrity.
 * @param [in] digestLen length of \p expectedDigest in bytes. KEEP IT SHORT
 *        to avoid a stack overflow.
 *
 * @retval #HZL_OK if the digest is valid.
 * @retval #HZL_ERR_SECWARN_INVALID_TAG if the digest is not valid and
 *         the message should be considered garbage.
 */
hzl_Err_t
hzl_HashDigestCheck(hzl_Hash_t* ctx,
                    const uint8_t* expectedDigest,
                    size_t digestLen);

#ifdef __cplusplus
}
#endif

#endif  /* HZL_HASH_H_ */
