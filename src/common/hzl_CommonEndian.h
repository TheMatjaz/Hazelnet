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
 * Encoding/decoding functions of integers into/from binary buffers
 * to guarantee the proper endianness, regardless of the platform architecture.
 */

#ifndef HZL_ENDIAN_H_
#define HZL_ENDIAN_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "hzl.h"

/**
 * Encodes an 8-bit as little endian.
 *
 * This function is just syntax sugar to match
 * the format of all the other encoding functions.
 *
 * @param [out] encoded buffer of at least 1 byte
 * @param [in] value integer to encode
 */
void hzl_EncodeLe8(uint8_t* encoded, uint8_t value);

/**
 * Encodes a 16-bit integer as little endian.
 *
 * @param [out] encoded buffer of at least 2 bytes
 * @param [in] value integer to encode
 */
void hzl_EncodeLe16(uint8_t* encoded, uint16_t value);

/**
 * Encodes a 24-bit integer as little endian.
 *
 * @param [out] encoded buffer of at least 3 bytes
 * @param [in] value integer to encode
 */
void hzl_EncodeLe24(uint8_t* encoded, uint32_t value);

/**
 * Encodes a 32-bit integer as little endian.
 *
 * @param [out] encoded buffer of at least 4 bytes
 * @param [in] value integer to encode
 */
void hzl_EncodeLe32(uint8_t* encoded, uint32_t value);

/**
 * Encodes a 64-bit integer as little endian.
 *
 * @param [out] encoded buffer of at least 8 bytes
 * @param [in] value integer to encode
 */
void hzl_EncodeLe64(uint8_t* encoded, uint64_t value);

/**
 * Decodes an 8-bit integer from a little endian buffer.
 *
 * This function is just syntax sugar to match
 * the format of all the other decoding functions.
 *
 * @param [in] encoded buffer of at least 1 byte
 * @return the integer
 */
uint8_t hzl_DecodeLe8(const uint8_t* encoded);

/**
 * Decodes an 8-bit integer from a little endian buffer.
 *
 * @param [in] encoded buffer of at least 2 byte
 * @return the integer
 */
uint16_t hzl_DecodeLe16(const uint8_t* encoded);

/**
 * Decodes a 24-bit integer from a little endian buffer.
 *
 * @param [in] encoded buffer of at least 3 byte
 * @return the integer
 */
uint32_t hzl_DecodeLe24(const uint8_t* encoded);

/**
 * Decodes a 32-bit integer from a little endian buffer.
 *
 * @param [in] encoded buffer of at least 4 byte
 * @return the integer
 */
uint32_t hzl_DecodeLe32(const uint8_t* encoded);

/**
 * Decodes a 64-bit integer from a little endian buffer.
 *
 * @param [in] encoded buffer of at least 8 byte
 * @return the integer
 */
uint64_t hzl_DecodeLe64(const uint8_t* encoded);

#ifdef __cplusplus
}
#endif

#endif  /* HZL_ENDIAN_H_ */
