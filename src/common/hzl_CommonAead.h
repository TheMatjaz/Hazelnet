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
 * Hazelnet wrapper of the AEAD functions: Authenticated Encryption with
 * Associated Data.
 *
 * The idea of this wrapper is to provide a stable interface in case the
 * implementation of the AEAD function is changed from one cipher to another.
 * Changing the AEAD cipher means updating the define #HZL_AEAD_NONCE_LEN
 * and typedef #hzl_Aead_t in this file
 * and updating the functions in the hzl_Aead.c file.
 */

#ifndef HZL_AEAD_H_
#define HZL_AEAD_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "hzl_CommonInternal.h"
#include "ascon.h"

/**
 * @internal
 * Computes the size of the ciphertext based on the size of the plaintext.
 *
 * For the Ascon cipher, they match exactly. For block ciphers, the ciphertext
 * is generally longer than the plaintext to fit into a multiple of the
 * block size, e.g. 11 B become 16 B.
 */
#define HZL_AEAD_PTLEN_TO_CTLEN(ptlen) (ptlen)

/** @internal Length of the nonce passed to the AEAD cipher in bytes. */
#define HZL_AEAD_NONCE_LEN ASCON_AEAD_NONCE_LEN

/**
 * @internal
 * AEAD-function state.
 */
typedef ascon_aead_ctx_t hzl_Aead_t;

/**
 * @internal
 * Initialises the AEAD context for encryption or decryption.
 *
 * @param [out] ctx to initialise
 * @param [in] key secret AEAD key of 16 bytes
 * @param [in] nonce public unique value of #HZL_AEAD_NONCE_LEN bytes
 */
void
hzl_AeadInit(hzl_Aead_t* ctx,
             const uint8_t* key,
             const uint8_t* nonce);

/**
 * @internal
 * Processes associated data to be authenticated. To be called after
 * hzl_AeadInit().
 *
 * @param [in, out] ctx initialised context
 * @param [in] assocData data to authenticated, but not encrypted
 * @param [in] assocDataLen length of \p assocData in bytes
 */
void
hzl_AeadAssocDataUpdate(hzl_Aead_t* ctx,
                        const uint8_t* assocData,
                        size_t assocDataLen);

/**
 * @internal
 * Encrypts the plaintext into ciphertext, returning the amount of
 * ciphertext bytes written. The encryption may also occur in-place
 * (\p ciphertext == \p plaintext is a valid input).
 *
 * @param [in, out] ctx context with associated data already processed
 * @param [out] ciphertext output encrypted plaintext
 * @param [in] plaintext data to be authenticated and encrypted
 * @param [in] plaintextLen length of \p plaintext in bytes
 * @return amount of bytes written into \p ciphertext
 */
size_t
hzl_AeadEncryptUpdate(hzl_Aead_t* ctx,
                      uint8_t* ciphertext,
                      const uint8_t* plaintext,
                      size_t plaintextLen);

/**
 * @internal
 * Finalises the encryption, flushing the last ciphertext bytes to the
 * output buffer and writing a tag (MAC) of the desired length.
 * Securely cleans its own context after completion.
 *
 * @param [in, out] ctx context with plaintext already processed.
 * @param [out] ciphertext location where to write any final trailing bytes. Should be where the
 *        previous hzl_AeadEncryptUpdate() stopped (start of the ciphertext + amount of bytes
 *        returned by hzl_AeadEncryptUpdate())
 * @param [out] tag message authentication code
 * @param [in] tagLen length of the desired tag in bytes
 */
void
hzl_AeadEncryptFinish(hzl_Aead_t* ctx,
                      uint8_t* ciphertext,
                      uint8_t* tag,
                      uint8_t tagLen);

/**
 * @internal
 * Decrypts the ciphertext into plaintext, returning the amount of
 * plaintext bytes written. The decryption may also occur in-place
 * (\p ciphertext == \p plaintext is a valid input).
 *
 * @param [in, out] ctx context with associated data already processed
 * @param [out] plaintext decrypted data
 * @param [in] ciphertext data to validate and decrypt
 * @param [in] ciphertextLen length of \p ciphertext in bytes
 * @return amount of bytes written into \p plaintext
 */
size_t
hzl_AeadDecryptUpdate(hzl_Aead_t* ctx,
                      uint8_t* plaintext,
                      const uint8_t* ciphertext,
                      size_t ciphertextLen);

/**
 * @internal
 * Finalises the decryption, flushing the last plaintext bytes to the
 * output buffer and checking that the computed tag matches with the provided
 * expected one of the given length.
 * Securely cleans its own state after completion, regardless of the tag validity.
 *
 * @param [in, out] ctx context with ciphertext already processed.
 * @param [out] plaintext location where to write any final trailing bytes. Should be where the
 *        previous hzl_AeadDecryptUpdate() stopped (start of the plaintext + amount of bytes
 *        returned by hzl_AeadDecryptUpdate())
 * @param [in] tag message authentication code that came with the ciphertext
 * @param [in] tagLen length of \p tag in bytes
 *
 * @retval #HZL_OK if the tag is valid
 * @retval #HZL_ERR_SECWARN_INVALID_TAG if the tag is not valid and the plaintext
 *         should be considered garbage
 */
hzl_Err_t
hzl_AeadDecryptFinish(hzl_Aead_t* ctx,
                      uint8_t* plaintext,
                      const uint8_t* tag,
                      uint8_t tagLen);

#ifdef __cplusplus
}
#endif

#endif  /* HZL_AEAD_H_ */
