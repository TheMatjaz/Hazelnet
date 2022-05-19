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
 * @internal Implementation of the hzl_ClientProcessReceivedResponse() function
 */

#include "hzl_ClientInternal.h"
#include "hzl_CommonHeader.h"
#include "hzl_CommonPayload.h"
#include "hzl_CommonEndian.h"
#include "hzl_ClientProcessReceived.h"
#include "hzl_CommonMessage.h"
#include "hzl_CommonInternal.h"

hzl_Err_t
hzl_ClientProcessReceivedResponse(const hzl_ClientCtx_t* ctx,
                                  const uint8_t* rxPdu,
                                  size_t rxPduLen,
                                  const hzl_Header_t* unpackedHdr,
                                  hzl_Timestamp_t rxTimestamp)
{
    HZL_ERR_DECLARE(err);
    // Is it coming from the Server?
    if (unpackedHdr->sid != HZL_SERVER_SID)
    {
        return HZL_ERR_SECWARN_SERVER_ONLY_MESSAGE;
    }
    // RES msg must be long enough to contain the required fields
    const uint8_t packedHdrLen = hzl_HeaderLen(ctx->clientConfig->headerType);
    if (rxPduLen < packedHdrLen + HZL_RES_PAYLOAD_LEN)
    {
        // We would overflow valid memory.
        return HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_RES;
    }
    // Is it for me?
    const hzl_Sid_t clientSid = rxPdu[packedHdrLen + HZL_RES_CLIENT_IDX];
    if (clientSid != ctx->clientConfig->sid) { return HZL_ERR_MSG_IGNORED; }
    hzl_ClientGroup_t group;
    err = hzl_ClientFindGroup(&group, ctx, unpackedHdr->gid);
    HZL_ERR_CHECK(err);
    err = hzl_ClientIsResponseAcceptable(ctx, &group, rxTimestamp);
    HZL_ERR_CHECK(err);
    // Is RX ctrnonce valid?
    const hzl_CtrNonce_t receivedCtrnonce = hzl_DecodeLe24(
            &rxPdu[packedHdrLen + HZL_RES_CTRNONCE_IDX]);
    if (HZL_IS_CTRNONCE_EXPIRED(receivedCtrnonce))
    {
        return HZL_ERR_SECWARN_RECEIVED_OVERFLOWN_NONCE;
    }
    uint8_t encodedRequestNonce[HZL_REQ_REQNONCE_LEN];
    hzl_EncodeLe64(encodedRequestNonce, group.state->requestNonce);
    // Authenticated decryption initialisation
    hzl_Aead_t aead;
    hzl_CommonAeadInitRes(&aead,
                          ctx->clientConfig->ltk,
                          unpackedHdr,
                          &rxPdu[packedHdrLen + HZL_RES_CTRNONCE_IDX],
                          encodedRequestNonce,
                          &rxPdu[packedHdrLen + HZL_RES_RESNONCE_IDX],
                          clientSid);
    // Decryption start
    uint8_t plaintextStk[HZL_RES_CTEXT_LEN] = {0};
    const size_t processedPtLen = hzl_AeadDecryptUpdate(
            &aead,
            plaintextStk,  // Output: plaintext
            &rxPdu[packedHdrLen + HZL_RES_CTEXT_IDX],  // Input: ciphertext
            HZL_RES_CTEXT_LEN);
    // Finish authenticated decryption and validate tag
    err = hzl_AeadDecryptFinish(
            &aead,
            &plaintextStk[processedPtLen],
            &rxPdu[packedHdrLen + HZL_RES_TAG_IDX],
            HZL_RES_TAG_LEN);
    if (err != HZL_OK)
    {
        // Securely clear the decrypted data before returning. Some of the decrypted data may
        // be correct, as potential errors could be injected later on in the ciphertext or even in
        // the tag. Just to avoid any leakage of the STK, erase everything written so far.
        hzl_ZeroOut(plaintextStk, sizeof(plaintextStk));
        return err;
    }
    if (hzl_IsAllZeros(plaintextStk, HZL_STK_LEN))
    {
        return HZL_ERR_SECWARN_RECEIVED_ZERO_KEY;
    }
    // Clear the request nonce to state that no Response is being expected anymore
    group.state->requestNonce = HZL_REQNONCE_NOT_EXPECTING_A_RESPONSE;
    // Save the received STK, counter nonce as current Session information
    memcpy(group.state->currentStk, plaintextStk, HZL_STK_LEN);
    group.state->currentCtrNonce = receivedCtrnonce;
    // Update the timestamps to indicate this is a valid reception and conclusion of the handshake
    group.state->currentRxLastMessageInstant = rxTimestamp;
    group.state->lastHandshakeEventInstant = rxTimestamp;
    return HZL_OK;
}
