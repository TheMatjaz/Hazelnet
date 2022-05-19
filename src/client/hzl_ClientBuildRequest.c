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
 * @internal Implementation of the hzl_ClientBuildRequest() function.
 */

#include "hzl_ClientInternal.h"
#include "hzl_CommonHeader.h"
#include "hzl_CommonPayload.h"
#include "hzl_CommonHash.h"
#include "hzl_CommonEndian.h"
#include "hzl_CommonMessage.h"
#include "hzl_CommonInternal.h"

hzl_Err_t
hzl_ClientBuildMsgReq(hzl_CbsPduMsg_t* msgToTx,
                      const hzl_ClientCtx_t* ctx,
                      const struct hzl_ClientGroup* group)
{
    HZL_ERR_DECLARE(err);
    // Prepare REQ Header
    const hzl_Header_t unpackedReqHeader = {
            .gid = group->config->gid,
            .sid = ctx->clientConfig->sid,
            .pty = HZL_PTY_REQ,
    };
    const uint8_t packedHdrLen = hzl_HeaderLen(ctx->clientConfig->headerType);
    hzl_HeaderPackFunc const headerPackFunc =
            hzl_HeaderPackFuncForType(ctx->clientConfig->headerType);
    // Prepare REQ Payload
    // Write the packed header at the beginning of the CAN FD frame's payload.
    headerPackFunc(msgToTx->data, &unpackedReqHeader);
    // Write request nonce after the header
    hzl_ReqNonce_t requestNonce = 0;
    err = hzl_NonZeroTrng((uint8_t*) &requestNonce, ctx->io.trng, sizeof(hzl_ReqNonce_t));
    HZL_ERR_CHECK(err);
    hzl_EncodeLe64(&msgToTx->data[packedHdrLen + HZL_REQ_REQNONCE_IDX], requestNonce);
    // Authenticate the msg with
    // tag = hash(LTK || label || GID || SID || PTY || reqnonce)
    hzl_Hash_t hash;
    hzl_ReqHashInit(&hash, ctx->clientConfig->ltk, &unpackedReqHeader,
                    &msgToTx->data[packedHdrLen + HZL_REQ_REQNONCE_IDX]);
    hzl_HashDigest(&hash, &msgToTx->data[packedHdrLen + HZL_REQ_TAG_IDX],
                   HZL_REQ_TAG_LEN);
    // Set the Request transmission timestamp as late as possible within the function.
    err = hzl_ClientSetRequestTxTimeToNow(ctx, group);
    HZL_ERR_CHECK(err);
    // Now that everything succeeded, write the non-zero Request nonce into the state to
    // indicate that a handshake is currently ongoing.
    group->state->requestNonce = requestNonce;
    // Message is packed in binary format, ready to transmit
    msgToTx->dataLen = packedHdrLen + HZL_REQ_PAYLOAD_LEN;
    return HZL_OK;
}

HZL_API hzl_Err_t
hzl_ClientBuildRequest(hzl_CbsPduMsg_t* requestPdu,
                       hzl_ClientCtx_t* ctx,
                       hzl_Gid_t groupId)
{
    if (requestPdu == NULL) { return HZL_ERR_NULL_PDU; }
    requestPdu->dataLen = 0; // Make output message empty in case of later error.
    HZL_ERR_DECLARE(err);
    err = hzl_ClientCheckCtxPointers(ctx);
    HZL_ERR_CHECK(err);
    hzl_ClientGroup_t group;
    err = hzl_ClientFindGroup(&group, ctx, groupId);
    HZL_ERR_CHECK(err);
    bool isAHandshakeOngoing = false;
    err = hzl_ClientIsAHandShakeOngoing(&isAHandshakeOngoing, ctx, &group);
    HZL_ERR_CHECK(err);
    if (isAHandshakeOngoing)
    {
        // Do nothing until the previous handshake expired or completed.
        return HZL_ERR_HANDSHAKE_ONGOING;
    }
    else
    {
        // Start a new handshake
        return hzl_ClientBuildMsgReq(requestPdu, ctx, &group);
    }
}
