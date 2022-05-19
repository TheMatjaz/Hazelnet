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
 * @internal Implementation of the hzl_ClientProcessReceivedSecuredFd() function
 */

#include "hzl_ClientInternal.h"
#include "hzl_CommonMessage.h"
#include "hzl_CommonHeader.h"
#include "hzl_CommonPayload.h"
#include "hzl_CommonEndian.h"
#include "hzl_ClientProcessReceived.h"
#include "hzl_CommonInternal.h"

/**
 * @internal
 * Selects the STK to use during a Session renewal phase.
 */
inline static const uint8_t*
hzl_ClientChoosePreviusOrCurrentStk(const hzl_ClientGroup_t* const group,
                                    const bool isPreviousSession)
{
    if (isPreviousSession) { return group->state->previousStk; }
    else { return group->state->currentStk; }
}

hzl_Err_t
hzl_ClientProcessReceivedSecuredFd(hzl_RxSduMsg_t* unpackedMsg,
                                   const hzl_ClientCtx_t* ctx,
                                   const uint8_t* rxPdu,
                                   size_t rxPduLen,
                                   const hzl_Header_t* unpackedSadfdHeader,
                                   hzl_Timestamp_t rxTimestamp)
{
    HZL_ERR_DECLARE(err);
    hzl_ClientGroup_t group;
    err = hzl_ClientFindGroup(&group, ctx, unpackedSadfdHeader->gid);
    if (err == HZL_ERR_UNKNOWN_GROUP)
    {
        return HZL_ERR_MSG_IGNORED;
    }
    hzl_ClientSessionRenewalPhaseExitIfNeeded(&group, rxTimestamp);
    // Check current state for validity
    if (!hzl_ClientIsSessionEstablishedAndValid(&group))
    {
        return HZL_ERR_SESSION_NOT_ESTABLISHED;
    }
    // SADFD msg must be long enough to contain at least the metadata (case of empty SDU)
    const uint8_t packedHdrLen = hzl_HeaderLen(ctx->clientConfig->headerType);
    if (rxPduLen < packedHdrLen + HZL_SADFD_METADATA_IN_PAYLOAD_LEN)
    {
        // Cannot even read the metadata of the message, including the ciphertext length.
        // We would overflow valid memory.
        return HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_SADFD;
    }
    // Get the ctrnonce and check it
    const hzl_CtrNonce_t receivedCtrnonce = hzl_DecodeLe24(
            &rxPdu[packedHdrLen + HZL_SADFD_CTRNONCE_IDX]);
    bool isPreviousSession = false;
    err = hzl_ClientCheckRxCtrnonce(&isPreviousSession, &group, receivedCtrnonce, rxTimestamp);
    HZL_ERR_CHECK(err);

    // Decrypt the ciphertext into the plaintext user-data (a.k.a. SDU).
    const uint8_t ptlen = rxPdu[packedHdrLen + HZL_SADFD_PTLEN_IDX];
    const uint8_t ctlen = HZL_AEAD_PTLEN_TO_CTLEN(ptlen);
    const size_t pduLenInferredFromCtlen = packedHdrLen + HZL_SADFD_PAYLOAD_LEN(ctlen);
    if (pduLenInferredFromCtlen > rxPduLen ||
        pduLenInferredFromCtlen > HZL_MAX_CAN_FD_DATA_LEN)
    {
        // The ptlen field value implies a ciphertext length which exceeds the overall length
        // of the PDU (CAN FD frame) as provided by the underlying CAN FD layer.
        // Buffer overflow: we would read memory that may not be initialised by the CAN FD layer.
        // Note: we are NOT checking whether rxPduLen > HZL_MAX_CAN_FD_DATA_LEN on purpose,
        // as by doing so we achieve the same effect. We don't really care if the buffer where the
        // PDU lays is much longer than the PDU itself, as long as it's long-enough to hold it.
        return HZL_ERR_TOO_LONG_CIPHERTEXT;
    }
    hzl_Aead_t aead;
    hzl_CommonAeadInitSadfd(
            &aead,
            hzl_ClientChoosePreviusOrCurrentStk(&group, isPreviousSession),
            unpackedSadfdHeader,
            receivedCtrnonce,
            ptlen);
    const size_t processedPtLen = hzl_AeadDecryptUpdate(
            &aead,
            unpackedMsg->data,  // Output: plaintext
            &rxPdu[packedHdrLen + HZL_SADFD_CTEXT_IDX],  // Input: ciphertext
            ctlen);

    // Finish authenticated decryption and validate tag
    err = hzl_AeadDecryptFinish(
            &aead,
            &unpackedMsg->data[processedPtLen],
            &rxPdu[packedHdrLen + HZL_SADFD_TAG_IDX(ctlen)],
            HZL_SADFD_TAG_LEN);
    if (err != HZL_OK)
    {
        // Securely clear the decrypted data before returning. Some of the decrypted data may
        // be correct, as potential errors could be injected later on in the ciphertext or even
        // in the tag. Just to avoid any leakage of information or the user reading data that may
        // not be correct, as it is not validated with the tag, erase everything written so far.
        hzl_ZeroOut(unpackedMsg->data, ptlen);
        return err;
    }
    // Save the received counter nonce as local one and the reception timestamp.
    hzl_ClientGroupUpdateCtrnonceAndRxTimestamp(
            &group, receivedCtrnonce, rxTimestamp, isPreviousSession);
    // Copy decrypted metadata to the user's output struct
    unpackedMsg->wasSecured = true;
    unpackedMsg->isForUser = true;
    unpackedMsg->gid = unpackedSadfdHeader->gid;
    unpackedMsg->sid = unpackedSadfdHeader->sid;
    unpackedMsg->dataLen = ptlen;
    return HZL_OK;
}
