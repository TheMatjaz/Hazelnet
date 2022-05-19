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
 * @internal Implementation of the hzl_ClientProcessReceivedRenewal() function
 */

#include "hzl_ClientInternal.h"
#include "hzl_CommonHeader.h"
#include "hzl_CommonPayload.h"
#include "hzl_CommonEndian.h"
#include "hzl_ClientProcessReceived.h"
#include "hzl_CommonHash.h"

hzl_Err_t
hzl_ClientProcessReceivedRenewal(hzl_CbsPduMsg_t* reactionPdu,
                                 const hzl_ClientCtx_t* ctx,
                                 const uint8_t* rxPdu,
                                 size_t rxPduLen,
                                 const hzl_Header_t* unpackedRenHeader,
                                 hzl_Timestamp_t rxTimestamp)
{
    reactionPdu->dataLen = 0;
    HZL_ERR_DECLARE(err);
    // Is it coming from the Server?
    if (unpackedRenHeader->sid != HZL_SERVER_SID)
    {
        return HZL_ERR_SECWARN_SERVER_ONLY_MESSAGE;
    }
    hzl_ClientGroup_t group;
    err = hzl_ClientFindGroup(&group, ctx, unpackedRenHeader->gid);
    if (err == HZL_ERR_UNKNOWN_GROUP)
    {
        return HZL_ERR_MSG_IGNORED;
    }
    // Check current state for validity
    if (!hzl_ClientIsSessionEstablishedAndValid(&group))
    {
        return HZL_ERR_SESSION_NOT_ESTABLISHED;
    }
    if (!hzl_ClientIsRenewalAcceptable(&group))
    {
        // This is repeated REN message. Maybe enough time has passed since the
        // Session renewal phase start, so check if it can be stopped.
        hzl_ClientSessionRenewalPhaseExitIfNeeded(&group, rxTimestamp);
        return HZL_ERR_MSG_IGNORED;
    }
    // REN msg must be long enough to contain the required fields
    const uint8_t packedHdrLen = hzl_HeaderLen(ctx->clientConfig->headerType);
    if (rxPduLen < packedHdrLen + HZL_REN_PAYLOAD_LEN)
    {
        // We would overflow valid memory.
        return HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_REN;
    }
    // Get the ctrnonce and check it
    const hzl_CtrNonce_t receivedCtrnonce = hzl_DecodeLe24(
            &rxPdu[packedHdrLen + HZL_REN_CTRNONCE_IDX]);
    err = hzl_ClientCheckRxCtrnonce(NULL, &group, receivedCtrnonce, rxTimestamp);
    HZL_ERR_CHECK(err);
    // Validate the tag
    // tag = hash(STK || label || GID || SID || PTY || ctrnonce)
    hzl_Hash_t hash;
    hzl_HashInit(&hash);
    hzl_HashUpdate(&hash, group.state->currentStk, HZL_STK_LEN);
    hzl_HashUpdate(&hash, (uint8_t*) HZL_REN_LABEL, HZL_REN_LABEL_LEN);
    hzl_HashUpdate(&hash, &unpackedRenHeader->gid, HZL_GID_LEN);
    hzl_HashUpdate(&hash, &unpackedRenHeader->sid, HZL_SID_LEN);
    hzl_HashUpdate(&hash, &unpackedRenHeader->pty, HZL_PTY_LEN);
    hzl_HashUpdate(&hash, &rxPdu[packedHdrLen + HZL_REN_CTRNONCE_IDX],
                   HZL_REN_CTRNONCE_LEN);
    err = hzl_HashDigestCheck(&hash,
                              &rxPdu[packedHdrLen + HZL_REN_TAG_IDX],
                              HZL_REN_TAG_LEN);
    HZL_ERR_CHECK(err);
    // Save the received counter nonce as local one and the reception timestamp.
    hzl_ClientGroupUpdateCtrnonceAndRxTimestamp(
            &group, receivedCtrnonce, rxTimestamp, false);
    // Enter the Client-side Session renewal phase
    hzl_ClientSessionRenewalPhaseEnter(&group);
    err = hzl_ClientBuildMsgReq(reactionPdu, ctx, &group);
    HZL_ERR_CHECK(err);
    return HZL_OK;
}
