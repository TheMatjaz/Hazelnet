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
 * Functions that pack/unpack all standard CBS Headers and check the Header Type.
 */

#ifndef HZL_HEADER_H_
#define HZL_HEADER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "hzl.h"

/**
 * @internal
 * CBS-Payload types, used in the PTY field of the CBS-Header.
 *
 * Values fit into 3 bits.
 */
typedef enum hzl_PayloadType
{
    HZL_PTY_REN = 0U,  ///< Session Renewal Notification
    HZL_PTY_RES = 1U,  ///< Response
    HZL_PTY_REQ = 2U,  ///< Request
    HZL_PTY_SADTP = 3U,  ///< Secured Application Data over Transport Protocol
    HZL_PTY_SADFD = 4U,  ///< Secured Application Data over CAN FD
    HZL_PTY_UAD = 5U,  ///< Unsecured Application Data
    HZL_PTY_RFU1 = 6U,  ///< Reserved for future use
    HZL_PTY_RFU2 = 7U,  ///< Reserved for future use
} hzl_PayloadType_t;

/**
 * @internal
 * Signature of a packer function that encodes the #hzl_Header_t structure into
 * a binary buffer, making the header ready for transmission.
 *
 * @param [out] binary buffer where to write the packed encoded header
 * @param [in] hdr data structure to encode
 */
typedef void (* hzl_HeaderPackFunc)(uint8_t* binary,
                                    const hzl_Header_t* hdr);

/**
 * @internal
 * Signature of an unpacker function that decodes a the binary header from a
 * received CBS message into a #hzl_Header_t structure.
 *
 * @param [out] hdr data structure where to write the decoded data
 * @param [in] binary buffer with encoded header
 */
typedef void (* hzl_HeaderUnpackFunc)(hzl_Header_t* hdr,
                                      const uint8_t* binary);

/**
 * @internal
 * Validates if the value represents an actual standard CBS header type.
 *
 * @param [in] type header type value to check
 * @retval #HZL_OK on success
 * @retval #HZL_ERR_INVALID_HEADER_TYPE in case of illegal header type value
 */
hzl_Err_t
hzl_HeaderTypeCheck(uint8_t type);

/**
 * @internal
 * Provides the largest SID that still fits in the given CBS Header Type.
 *
 * @param [in] type header type
 * @return the max SID value or 0 in case of illegal header type
 */
hzl_Sid_t
hzl_HeaderTypeMaxSid(uint8_t type);

/**
 * @internal
 * Provides the largest GID that still fits in the given CBS Header Type.
 *
 * @param [in] type header type
 * @return the max GID value or 0 in case of illegal header type
 */
hzl_Gid_t
hzl_HeaderTypeMaxGid(uint8_t type);

/**
 * @internal
 * Provides the length in bytes of the encoded header of a given type.
 *
 * @param [in] type header
 * @return positive length in bytes or 0 in case of invalid header type
 */
uint8_t
hzl_HeaderLen(uint8_t type);

/**
 * @internal
 * Provides the packer function for the header of a given type.
 *
 * @param [in] type header
 * @return packer function pointer or NULL in case of invalid header type
 */
hzl_HeaderPackFunc
hzl_HeaderPackFuncForType(uint8_t type);

/**
 * @internal
 * Provides the unpacker function for the header of a given type.
 *
 * @param [in] type header
 * @return unpacker function pointer or NULL in case of invalid header type
 */
hzl_HeaderUnpackFunc
hzl_HeaderUnpackFuncForType(uint8_t type);

#ifdef __cplusplus
}
#endif

#endif  /* HZL_HEADER_H_ */
