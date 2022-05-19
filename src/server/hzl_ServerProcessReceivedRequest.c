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
 * Implementation of the hzl_ServerProcessReceivedRequest() function.
 */

#include "hzl_ServerInternal.h"
#include "hzl_CommonHeader.h"
#include "hzl_ServerProcessReceived.h"
#include "hzl_CommonPayload.h"
#include "hzl_CommonEndian.h"
#include "hzl_CommonHash.h"
#include "hzl_CommonMessage.h"

hzl_Err_t
hzl_ServerValidateSidAndGid(const hzl_ServerCtx_t* const ctx,
                            const hzl_Gid_t gid,
                            const hzl_Sid_t sid)
{
    if (sid >= ctx->serverConfig->amountOfClients)
    {
        // SIDs are contiguous: this one would cause an index out of bounds.
        return HZL_ERR_UNKNOWN_SOURCE;
    }
    if (gid >= ctx->serverConfig->amountOfGroups)
    {
        // GIDs are contiguous: this one would cause an index out of bounds.
        return HZL_ERR_UNKNOWN_GROUP;
    }
    // SID 0 is server: already checked for that.
    // SID 1 maps to bit at index 0, SID 2 to index 1 etc.
    const hzl_ServerBitMap_t sidAsBitFlag = 1U << (sid - 1U);
    if (!(ctx->groupConfigs[gid].clientSidsInGroupBitmap & sidAsBitFlag))
    {
        return HZL_ERR_SECWARN_NOT_IN_GROUP;
    }
    return HZL_OK;
}

inline static hzl_Err_t
hzl_ServerBuildMsgResponse(hzl_CbsPduMsg_t* const msgToTx,
                           const hzl_ServerCtx_t* const ctx,
                           const uint8_t* const encodedRequestNonce,
                           const hzl_Gid_t gid,
                           const hzl_Sid_t clientSid)
{
    HZL_ERR_DECLARE(err);
    // Prepare RES Header
    const hzl_Header_t unpackedResHeader = {
            .gid = gid,
            .sid = HZL_SERVER_SID,
            .pty = HZL_PTY_RES,
    };
    const uint8_t packedHdrLen = hzl_HeaderLen(ctx->serverConfig->headerType);
    hzl_HeaderPackFunc const headerPackFunc =
            hzl_HeaderPackFuncForType(ctx->serverConfig->headerType);
    // Prepare RES Payload
    // Write the packed header at the beginning of the CAN FD frame's payload.
    headerPackFunc(msgToTx->data, &unpackedResHeader);
    // Destination client
    msgToTx->data[packedHdrLen + HZL_RES_CLIENT_IDX] = clientSid;
    // Counter Nonce of the Group
    hzl_EncodeLe24(&msgToTx->data[packedHdrLen + HZL_RES_CTRNONCE_IDX],
                   ctx->groupStates[gid].currentCtrNonce);
    // Response nonce
    err = hzl_NonZeroTrng(&msgToTx->data[packedHdrLen + HZL_RES_RESNONCE_IDX],
                          ctx->io.trng,
                          sizeof(hzl_ResNonce_t));
    HZL_ERR_CHECK(err);
    // Authenticated decryption initialisation
    hzl_Aead_t aead;
    hzl_CommonAeadInitRes(&aead,
                          ctx->clientConfigs[clientSid - 1U].ltk,
                          &unpackedResHeader,
                          &msgToTx->data[packedHdrLen + HZL_RES_CTRNONCE_IDX],
                          encodedRequestNonce,
                          &msgToTx->data[packedHdrLen + HZL_RES_RESNONCE_IDX],
                          clientSid);
    // Encryption start
    const size_t processedCtLen = hzl_AeadEncryptUpdate(
            &aead,
            &msgToTx->data[packedHdrLen + HZL_RES_CTEXT_IDX],  // Output: ciphertext
            ctx->groupStates[gid].currentStk,  // Input: plaintext
            HZL_RES_CTEXT_LEN);
    // Finish authenticated encryption and generate tag
    hzl_AeadEncryptFinish(
            &aead,
            &msgToTx->data[packedHdrLen + HZL_RES_CTEXT_IDX + processedCtLen],  // Flush CT
            &msgToTx->data[packedHdrLen + HZL_RES_TAG_IDX],
            HZL_RES_TAG_LEN);
    // Message is packed in binary format, ready to transmit
    msgToTx->dataLen = packedHdrLen + HZL_RES_PAYLOAD_LEN;
    return HZL_OK;
}

void
hzl_ServerUpdateCurrentRxLastMessageInstant(const hzl_ServerCtx_t* const ctx,
                                            const hzl_Timestamp_t rxTimestamp,
                                            const hzl_Gid_t gid)
{
    ctx->groupStates[gid].currentRxLastMessageInstant = rxTimestamp;
    if (ctx->groupStates[gid].currentRxLastMessageInstant
        == ctx->groupStates[gid].sessionStartInstant)
    {
        // The Request was received IMMEDIATELY after the Session was started, within the
        // same millisecond. This happens on fast busses sometimes on the bus startup and
        // when running interop tests of the library on a desktop, where there is no
        // communication delay.
        // This is a valid situation, but we will save the timestamp of reception of
        // this message incremented by 1 millisecond, to force the currentRxLastMessageInstant
        // and sessionStartInstant to be different, as their equality is used to understand
        // whether there is at least one Client that has Requested the Session information.
        // There is one Client, otherwise we would not be in this function.
        ctx->groupStates[gid].currentRxLastMessageInstant++;
    }
}

hzl_Err_t
hzl_ServerProcessReceivedRequest(hzl_CbsPduMsg_t* const msgToTx,
                                 hzl_ServerCtx_t* const ctx,
                                 const uint8_t* const rxPdu,
                                 const size_t rxPduLen,
                                 const hzl_Header_t* const unpackedReqHeader,
                                 const hzl_Timestamp_t rxTimestamp)
{
    HZL_ERR_DECLARE(err);
    err = hzl_ServerValidateSidAndGid(ctx, unpackedReqHeader->gid, unpackedReqHeader->sid);
    HZL_ERR_CHECK(err);
    // REQ msg must be long enough to contain the required fields
    const uint8_t packedHdrLen = hzl_HeaderLen(ctx->serverConfig->headerType);
    if (rxPduLen < packedHdrLen + HZL_REQ_PAYLOAD_LEN)
    {
        // We would overflow valid memory.
        return HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_REQ;
    }
    const uint8_t* const encodedRequestNonce = &rxPdu[packedHdrLen + HZL_REQ_REQNONCE_IDX];
    if (hzl_IsAllZeros(encodedRequestNonce, HZL_REQ_REQNONCE_LEN))
    {
        return HZL_ERR_SECWARN_RECEIVED_ZERO_REQNONCE;
    }
    // Validate the msg with
    // tag = hash(LTK || label || GID || SID || PTY || reqnonce)
    hzl_Hash_t hash;
    hzl_ReqHashInit(&hash, ctx->clientConfigs[unpackedReqHeader->sid - 1U].ltk,
                    unpackedReqHeader, encodedRequestNonce);
    err = hzl_HashDigestCheck(&hash,
                              &rxPdu[packedHdrLen + HZL_REQ_TAG_IDX],
                              HZL_REQ_TAG_LEN);
    HZL_ERR_CHECK(err);
    hzl_ServerUpdateCurrentRxLastMessageInstant(ctx, rxTimestamp, unpackedReqHeader->gid);
    // Build a Response as a reaction
    return hzl_ServerBuildMsgResponse(msgToTx, ctx,
                                      encodedRequestNonce,
                                      unpackedReqHeader->gid, unpackedReqHeader->sid);
}
