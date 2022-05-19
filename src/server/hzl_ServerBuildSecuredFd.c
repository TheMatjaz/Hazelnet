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
 * Implementation of hzl_ServerBuildSecuredFd().
 */

#include "hzl.h"
#include "hzl_ServerInternal.h"
#include "hzl_CommonPayload.h"
#include "hzl_CommonMessage.h"
#include "hzl_CommonEndian.h"

inline bool
hzl_ServerDidAnyClientAlreadyRequest(const hzl_ServerCtx_t* const ctx,
                                     const hzl_Gid_t groupId)
{
    // Upon Session initialisation or renewal, before the first Request in a Group,
    // currentRxLastMessageInstant is set to sessionStartInstant. When the Request is
    // received, currentRxLastMessageInstant is updated to a different value. This information
    // is used by the Server to know whether there is at least one Client in the Group
    // that is enabled to received Secured Application Data messages.
    // They will be equal again for one single millisecond when the timestamp rolls around,
    // 2^32 milliseconds after the session started (that is 49 DAYS!), but at that point the
    // Session will already be expired and renewed.
    return ctx->groupStates[groupId].currentRxLastMessageInstant !=
           ctx->groupStates[groupId].sessionStartInstant;
}

inline static hzl_Err_t
hzl_ServerBuildMsgSadfd(hzl_CbsPduMsg_t* const msgToTx,
                        hzl_ServerCtx_t* const ctx,
                        const uint8_t* const userData,
                        const size_t userDataLen,
                        const hzl_Gid_t groupId)
{
    // Prepare SADFD Header
    const hzl_Header_t unpackedSadfdHeader = {
            .gid = groupId,
            .sid = HZL_SERVER_SID,
            .pty = HZL_PTY_SADFD,
    };
    const uint8_t packedHdrLen = hzl_HeaderLen(ctx->serverConfig->headerType);
    hzl_HeaderPackFunc const headerPackFunc =
            hzl_HeaderPackFuncForType(ctx->serverConfig->headerType);
    // Prepare SADFD payload
    // Write the packed header at the beginning of the CAN FD frame's payload.
    headerPackFunc(msgToTx->data, &unpackedSadfdHeader);
    // Write counter nonce after the header
    hzl_EncodeLe24(&msgToTx->data[packedHdrLen + HZL_SADFD_CTRNONCE_IDX],
                   ctx->groupStates[groupId].currentCtrNonce);
    msgToTx->data[packedHdrLen + HZL_SADFD_PTLEN_IDX] = (uint8_t) userDataLen;
    // Encrypt the plaintext (user-data a.k.a. SDU) into the ctext field of the SADFD message
    hzl_Aead_t aead;
    hzl_CommonAeadInitSadfd(&aead,
                            ctx->groupStates[groupId].currentStk,
                            &unpackedSadfdHeader,
                            ctx->groupStates[groupId].currentCtrNonce,
                            (uint8_t) userDataLen);
    const size_t processedPtLen = hzl_AeadEncryptUpdate(
            &aead,
            &msgToTx->data[packedHdrLen + HZL_SADFD_CTEXT_IDX],  // Output: ciphertext
            userData,  // Input: plaintext
            userDataLen);
    // Finish authenticated encryption and write tag
    hzl_AeadEncryptFinish(
            &aead,
            &msgToTx->data[packedHdrLen + HZL_SADFD_CTEXT_IDX + processedPtLen],
            &msgToTx->data[packedHdrLen + HZL_SADFD_TAG_IDX(userDataLen)],
            HZL_SADFD_TAG_LEN);
    // Message is packed in binary format, ready to transmit
    msgToTx->dataLen = packedHdrLen + HZL_SADFD_PAYLOAD_LEN(userDataLen);
    // Increment the counter nonce, regardless of transmission success
    hzl_ServerGroupIncrCurrentCtrnonce(ctx, groupId);
    return HZL_OK;
}

HZL_API hzl_Err_t
hzl_ServerBuildSecuredFd(hzl_CbsPduMsg_t* const securedPdu,
                         hzl_ServerCtx_t* const ctx,
                         const uint8_t* const userData,
                         const size_t userDataLen,
                         const hzl_Gid_t groupId)
{
    if (securedPdu == NULL) { return HZL_ERR_NULL_PDU; }
    securedPdu->dataLen = 0; // Make output message empty in case of later error.
    HZL_ERR_DECLARE(err);
    err = hzl_ServerCheckCtxPointers(ctx);
    HZL_ERR_CHECK(err);
    err = hzl_CommonCheckMsgBeforePacking(
            userData, userDataLen, groupId,
            HZL_SADFD_METADATA_IN_PAYLOAD_LEN, ctx->serverConfig->headerType);
    HZL_ERR_CHECK(err);
    if (groupId >= ctx->serverConfig->amountOfGroups)
    {
        return HZL_ERR_UNKNOWN_GROUP;
    }
    if (!hzl_ServerDidAnyClientAlreadyRequest(ctx, groupId))
    {
        return HZL_ERR_NO_POTENTIAL_RECEIVER;
    }
    return hzl_ServerBuildMsgSadfd(securedPdu, ctx, userData, userDataLen, groupId);
}
