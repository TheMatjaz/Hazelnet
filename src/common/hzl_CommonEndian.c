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
 * Implementation of the encoding/decoding of integers into/from binary buffers.
 */

#include "hzl_CommonEndian.h"

void
hzl_EncodeLe8(uint8_t* const encoded, const uint8_t value)
{
    encoded[0] = value;
}

void
hzl_EncodeLe16(uint8_t* encoded, const uint16_t value)
{
    encoded[0] = (uint8_t) (value >> 0U);
    encoded[1] = (uint8_t) (value >> 8U);
}

void
hzl_EncodeLe24(uint8_t* const encoded, const uint32_t value)
{
    encoded[0] = (uint8_t) (value >> 0U);
    encoded[1] = (uint8_t) (value >> 8U);
    encoded[2] = (uint8_t) (value >> 16U);
}

void
hzl_EncodeLe32(uint8_t* const encoded, const uint32_t value)
{
    encoded[0] = (uint8_t) (value >> 0U);
    encoded[1] = (uint8_t) (value >> 8U);
    encoded[2] = (uint8_t) (value >> 16U);
    encoded[3] = (uint8_t) (value >> 24U);
}

void
hzl_EncodeLe64(uint8_t* const encoded, const uint64_t value)
{
    encoded[0] = (uint8_t) (value >> 0U);
    encoded[1] = (uint8_t) (value >> 8U);
    encoded[2] = (uint8_t) (value >> 16U);
    encoded[3] = (uint8_t) (value >> 24U);
    encoded[4] = (uint8_t) (value >> 32U);
    encoded[5] = (uint8_t) (value >> 40U);
    encoded[6] = (uint8_t) (value >> 48U);
    encoded[7] = (uint8_t) (value >> 56U);
}

uint8_t
hzl_DecodeLe8(const uint8_t* const encoded)
{
    return encoded[0];
}

uint16_t
hzl_DecodeLe16(const uint8_t* const encoded)
{
    return (uint16_t) (encoded[1] << 8U | encoded[0]);
}

uint32_t
hzl_DecodeLe24(const uint8_t* const encoded)
{
    uint32_t value = encoded[0];
    value |= ((uint32_t) encoded[1]) << 8U;
    value |= ((uint32_t) encoded[2]) << 16U;
    return value;
}

uint32_t
hzl_DecodeLe32(const uint8_t* const encoded)
{
    uint32_t value = encoded[0];
    value |= ((uint32_t) encoded[1]) << 8U;
    value |= ((uint32_t) encoded[2]) << 16U;
    value |= ((uint32_t) encoded[3]) << 24U;
    return value;
}

uint64_t
hzl_DecodeLe64(const uint8_t* const encoded)
{
    uint64_t value = encoded[0];
    value |= ((uint64_t) encoded[1]) << 8U;
    value |= ((uint64_t) encoded[2]) << 16U;
    value |= ((uint64_t) encoded[3]) << 24U;
    value |= ((uint64_t) encoded[4]) << 32U;
    value |= ((uint64_t) encoded[5]) << 40U;
    value |= ((uint64_t) encoded[6]) << 48U;
    value |= ((uint64_t) encoded[7]) << 56U;
    return value;
}
