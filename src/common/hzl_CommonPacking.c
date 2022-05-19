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
 * Validation of a message before transmission, packing of it and the
 * final transmission.
 */

#include "hzl.h"
#include "hzl_CommonMessage.h"

hzl_Err_t
hzl_CommonCheckMsgBeforePacking(const uint8_t* const userData,
                                const size_t userDataLen,
                                const hzl_Gid_t group,
                                const size_t metadataInPayloadLen,
                                const uint8_t headerType)
{
    if (userData == NULL && userDataLen != 0) { return HZL_ERR_NULL_SDU; }
    const hzl_Gid_t maxGid = hzl_HeaderTypeMaxGid(headerType);
    if (group > maxGid) { return HZL_ERR_GID_TOO_LARGE_FOR_CONFIGURED_HEADER_TYPE; }
    const uint8_t packedHdrLen = hzl_HeaderLen(headerType);
    const size_t maxDataLen = HZL_MAX_CAN_FD_DATA_LEN - packedHdrLen - metadataInPayloadLen;
    if (userDataLen > maxDataLen) { return HZL_ERR_TOO_LONG_SDU; }
    return HZL_OK;
}


/** @internal Verifies the basic integirty of the message data structure (NULL pointers,
 * minimum and maximum sizes). */
hzl_Err_t
hzl_CommonCheckReceivedGenericMsg(hzl_Header_t* const unpackedHdr,
                                  const uint8_t* const receivedPdu,
                                  const size_t receivedPduLen,
                                  const hzl_Sid_t receiverSid,
                                  const uint8_t headerType)
{
    if (receivedPdu == NULL && receivedPduLen != 0) { return HZL_ERR_NULL_PDU; }
    const uint8_t packedHdrLen = hzl_HeaderLen(headerType);
    if (receivedPduLen < packedHdrLen) { return HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_HEADER; }
    hzl_HeaderUnpackFunc headerUnpackerFunc = hzl_HeaderUnpackFuncForType(headerType);
    headerUnpackerFunc(unpackedHdr, receivedPdu);
    if (unpackedHdr->sid == receiverSid) { return HZL_ERR_SECWARN_MESSAGE_FROM_MYSELF; }
    return HZL_OK;
}
