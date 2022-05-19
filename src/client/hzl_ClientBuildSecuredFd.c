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
 * Implementation of hzl_ClientBuildSecuredFd().
 */

#include "hzl_ClientInternal.h"
#include "hzl_CommonHeader.h"
#include "hzl_CommonPayload.h"
#include "hzl_CommonMessage.h"
#include "hzl_CommonEndian.h"
#include "hzl_CommonInternal.h"

inline static hzl_Err_t
hzl_ClientBuildMsgSadfd(hzl_CbsPduMsg_t* const msgToTx,
                        hzl_ClientCtx_t* const ctx,
                        const uint8_t* const userData,
                        const size_t userDataLen,
                        const hzl_ClientGroup_t* const group)
{
    // Prepare SADFD Header
    const hzl_Header_t unpackedSadfdHeader = {
            .gid = group->config->gid,
            .sid = ctx->clientConfig->sid,
            .pty = HZL_PTY_SADFD,
    };
    const uint8_t packedHdrLen = hzl_HeaderLen(ctx->clientConfig->headerType);
    hzl_HeaderPackFunc const headerPackFunc =
            hzl_HeaderPackFuncForType(ctx->clientConfig->headerType);
    // Prepare SADFD payload
    // Write the packed header at the beginning of the CAN FD frame's payload.
    headerPackFunc(msgToTx->data, &unpackedSadfdHeader);
    // Write counter nonce after the header
    hzl_EncodeLe24(&msgToTx->data[packedHdrLen + HZL_SADFD_CTRNONCE_IDX],
                   group->state->currentCtrNonce);
    msgToTx->data[packedHdrLen + HZL_SADFD_PTLEN_IDX] = (uint8_t) userDataLen;
    // Encrypt the plaintext (user-data a.k.a. SDU) into the ctext field of the SADFD message
    hzl_Aead_t aead;
    hzl_CommonAeadInitSadfd(&aead,
                            group->state->currentStk,
                            &unpackedSadfdHeader,
                            group->state->currentCtrNonce,
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
    hzl_ClientGroupIncrCurrentCtrnonce(group);
    return HZL_OK;
}

HZL_API hzl_Err_t
hzl_ClientBuildSecuredFd(hzl_CbsPduMsg_t* securedPdu,
                         hzl_ClientCtx_t* ctx,
                         const uint8_t* userData,
                         size_t userDataLen,
                         hzl_Gid_t groupId)
{
    if (securedPdu == NULL) { return HZL_ERR_NULL_PDU; }
    securedPdu->dataLen = 0; // Make output message empty in case of later error.
    HZL_ERR_DECLARE(err);
    err = hzl_ClientCheckCtxPointers(ctx);
    HZL_ERR_CHECK(err);
    err = hzl_CommonCheckMsgBeforePacking(
            userData, userDataLen, groupId,
            HZL_SADFD_METADATA_IN_PAYLOAD_LEN, ctx->clientConfig->headerType);
    HZL_ERR_CHECK(err);
    hzl_ClientGroup_t group;
    err = hzl_ClientFindGroup(&group, ctx, groupId);
    HZL_ERR_CHECK(err);
    if (!hzl_ClientIsSessionEstablishedAndValid(&group))
    {
        return HZL_ERR_SESSION_NOT_ESTABLISHED;
    }
    return hzl_ClientBuildMsgSadfd(securedPdu, ctx, userData, userDataLen, &group);
}
