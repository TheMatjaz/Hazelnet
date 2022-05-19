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
 */

#include "hzl.h"
#include "hzl_CommonInternal.h"
#include "hzl_CommonHeader.h"
#include "hzl_CommonPayload.h"
#include "hzl_CommonMessage.h"

HZL_API hzl_Err_t
hzl_CommonBuildUnsecured(hzl_CbsPduMsg_t* const unsecuredPdu,
                         const uint8_t* const userData,
                         const size_t userDataLen,
                         const hzl_Gid_t groupId,
                         const hzl_Sid_t sourceId,
                         const uint8_t headerType)
{
    if (unsecuredPdu == NULL) { return HZL_ERR_NULL_PDU; }
    unsecuredPdu->dataLen = 0; // Make output message empty in case of later error.
    HZL_ERR_DECLARE(err);
    err = hzl_CommonCheckMsgBeforePacking(
            userData, userDataLen, groupId,
            HZL_UAD_METADATA_IN_PAYLOAD_LEN, headerType);
    HZL_ERR_CHECK(err);
    // Prepare UAD Header
    const hzl_Header_t unpackedUadHeader = {
            .gid = groupId,
            .sid = sourceId,
            .pty = HZL_PTY_UAD,
    };
    const uint8_t packedHdrLen = hzl_HeaderLen(headerType);
    hzl_HeaderPackFunc const headerPackFunc =
            hzl_HeaderPackFuncForType(headerType);
    // Prepare UAD payload
    // Write the packed header at the beginning of the CAN FD frame's payload.
    headerPackFunc(unsecuredPdu->data, &unpackedUadHeader);
    // Copy user-data (SDU) to the right of the packed header.
    memcpy(unsecuredPdu->data + packedHdrLen, userData, userDataLen);
    // Message is packed in binary format, ready to transmit
    unsecuredPdu->dataLen = packedHdrLen + userDataLen;
    return HZL_OK;
}
