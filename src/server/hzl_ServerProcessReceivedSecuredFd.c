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
 * @internal Implementation of the hzl_ServerProcessReceivedSecuredFd() function
 */

#include "hzl_ServerInternal.h"
#include "hzl_CommonMessage.h"
#include "hzl_CommonHeader.h"
#include "hzl_CommonPayload.h"
#include "hzl_CommonEndian.h"
#include "hzl_ServerProcessReceived.h"

inline static bool
hzl_ServerIsCtrNonceOfPreviousSession(const hzl_ServerCtx_t* const ctx,
                                      const hzl_CtrNonce_t receivedCtrnonce,
                                      const hzl_Gid_t gid)
{
    const hzl_CtrNonce_t average = (hzl_CtrNonce_t)
            ((ctx->groupStates[gid].currentCtrNonce + ctx->groupStates[gid].previousCtrNonce) / 2U);
    return receivedCtrnonce >= average;
}

/**
 * @internal
 * Selects the STK to use during a Session renewal phase.
 */
inline static const uint8_t*
hzl_ServerChoosePreviusOrCurrentStk(const hzl_ServerCtx_t* const ctx,
                                    const bool isPreviousSession,
                                    const hzl_Gid_t gid)
{
    if (isPreviousSession) { return ctx->groupStates[gid].previousStk; }
    else { return ctx->groupStates[gid].currentStk; }
}

static hzl_Err_t
hzl_ServerCheckRxCtrnonce(bool* const isPreviousSession,
                          const hzl_ServerCtx_t* const ctx,
                          const hzl_CtrNonce_t receivedCtrnonce,
                          const hzl_Timestamp_t rxTimestamp,
                          const hzl_Gid_t gid)
{
    if (HZL_IS_CTRNONCE_EXPIRED(receivedCtrnonce))
    {
        return HZL_ERR_SECWARN_RECEIVED_OVERFLOWN_NONCE;
    }
    // Check if belongs to the old or new session during a renewal phase
    const bool isPrevious =
            isPreviousSession != NULL
            && hzl_ServerSessionRenewalPhaseIsActive(ctx, gid)
            && hzl_ServerIsCtrNonceOfPreviousSession(ctx, receivedCtrnonce, gid);
    hzl_Timestamp_t selectedLastRxTimestamp;
    hzl_CtrNonce_t selectedCtrNonce;
    if (isPrevious)
    {
        selectedLastRxTimestamp = ctx->groupStates[gid].previousRxLastMessageInstant;
        selectedCtrNonce = ctx->groupStates[gid].previousCtrNonce;
    }
    else
    {
        selectedLastRxTimestamp = ctx->groupStates[gid].currentRxLastMessageInstant;
        selectedCtrNonce = ctx->groupStates[gid].currentCtrNonce;
    }
    // Freshness of received ctrnonce compared to the ctrnonce of the last
    // received message of the previous or current session, depending where
    // the received message belongs.
    const hzl_CtrNonce_t delay = hzl_CommonCtrDelay(
            selectedLastRxTimestamp,
            rxTimestamp,
            ctx->groupConfigs[gid].maxCtrnonceDelayMsgs,
            ctx->groupConfigs[gid].maxSilenceIntervalMillis);
    // Casting to signed to avoid compiler errors. Counter nonces anyway use
    // only 24 bits, so the signed value is the same as the unsigned.
    const int32_t oldestToleratedCtrNonce = (int32_t) selectedCtrNonce - (int32_t) delay;
    if ((int32_t) receivedCtrnonce < oldestToleratedCtrNonce)
    {
        return HZL_ERR_SECWARN_OLD_MESSAGE;
    }
    if (isPreviousSession != NULL) { *isPreviousSession = isPrevious; }
    return HZL_OK;
}

static void
hzl_ServerGroupUpdateCtrnonceAndRxTimestamp(const hzl_ServerCtx_t* const ctx,
                                            const hzl_CtrNonce_t receivedCtrnonce,
                                            const hzl_Timestamp_t receptionTimestamp,
                                            const bool isPreviousSession,
                                            const hzl_Gid_t gid)
{
    if (isPreviousSession)
    {
        if (receivedCtrnonce > ctx->groupStates[gid].previousCtrNonce)
        {
            ctx->groupStates[gid].previousCtrNonce = receivedCtrnonce;
        }
        hzl_ServerGroupIncrPreviousCtrnonce(ctx, gid);
        ctx->groupStates[gid].previousRxLastMessageInstant = receptionTimestamp;
    }
    else
    {
        if (receivedCtrnonce > ctx->groupStates[gid].currentCtrNonce)
        {
            ctx->groupStates[gid].currentCtrNonce = receivedCtrnonce;
        }
        hzl_ServerGroupIncrCurrentCtrnonce(ctx, gid);
        hzl_ServerUpdateCurrentRxLastMessageInstant(ctx, receptionTimestamp, gid);
    }
}

hzl_Err_t
hzl_ServerProcessReceivedSecuredFd(hzl_CbsPduMsg_t* const reactionPdu,
                                   hzl_RxSduMsg_t* const unpackedMsg,
                                   hzl_ServerCtx_t* const ctx,
                                   const uint8_t* const rxPdu,
                                   const size_t rxPduLen,
                                   const hzl_Header_t* const unpackedSadfdHeader,
                                   const hzl_Timestamp_t rxTimestamp)
{
    HZL_ERR_DECLARE(err);
    err = hzl_ServerValidateSidAndGid(ctx, unpackedSadfdHeader->gid, unpackedSadfdHeader->sid);
    HZL_ERR_CHECK(err);
    // Check if the Session renewal phase must be terminated before processing the SADFD message
    // in order to avoid accepting messages belonging to the previous Session, if the previous
    // Session should NOT be considered anymore.
    hzl_ServerSessionRenewalPhaseExitIfNeeded(ctx, rxTimestamp, unpackedSadfdHeader->gid);
    // SADFD msg must be long enough to contain at least the metadata (case of empty SDU)
    const uint8_t packedHdrLen = hzl_HeaderLen(ctx->serverConfig->headerType);
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
    err = hzl_ServerCheckRxCtrnonce(
            &isPreviousSession, ctx, receivedCtrnonce, rxTimestamp, unpackedSadfdHeader->gid);
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
            hzl_ServerChoosePreviusOrCurrentStk(ctx, isPreviousSession,
                                                unpackedSadfdHeader->gid),
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
    hzl_ServerGroupUpdateCtrnonceAndRxTimestamp(ctx, receivedCtrnonce, rxTimestamp,
                                                isPreviousSession,
                                                unpackedSadfdHeader->gid);
    // Copy decrypted metadata to the user's output struct
    unpackedMsg->wasSecured = true;
    unpackedMsg->isForUser = true;
    unpackedMsg->gid = unpackedSadfdHeader->gid;
    unpackedMsg->sid = unpackedSadfdHeader->sid;
    unpackedMsg->dataLen = ptlen;
    // Check if the Session is expired and should be renewed, in order to send the REN message
    // using the ctrnonce that was already updated after the reception of the SADFD message just
    // processed.
    err = hzl_ServerSessionRenewalPhaseEnterIfNeeded(
            reactionPdu, ctx, rxTimestamp, unpackedSadfdHeader->gid);
    return err;
}
