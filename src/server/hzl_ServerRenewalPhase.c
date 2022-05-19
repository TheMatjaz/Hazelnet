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
 * @internal Functions handling the renewal phase, from start to finish.
 */

#include "hzl_ServerInternal.h"
#include "hzl_CommonMessage.h"
#include "hzl_CommonHeader.h"
#include "hzl_CommonPayload.h"
#include "hzl_CommonEndian.h"
#include "hzl_ServerProcessReceived.h"

bool
hzl_ServerSessionRenewalPhaseIsActive(const hzl_ServerCtx_t* const ctx,
                                      const hzl_Gid_t gid)
{
    return !hzl_IsAllZeros(ctx->groupStates[gid].previousStk, HZL_STK_LEN);
}

inline static bool
hzl_ServerSessionRenewalPhaseIsOver(const hzl_ServerCtx_t* const ctx,
                                    const hzl_Timestamp_t now,
                                    const hzl_Gid_t gid)
{
    // The 2 and 6 are multipliers coming strictly from the CBS protocol specification.
    const bool haveEnoughSecuredMessagesBeenUsed =
            ctx->groupStates[gid].currentCtrNonce >=
            2U * ctx->groupConfigs[gid].maxCtrnonceDelayMsgs;
    const hzl_TimeDeltaMillis_t timeSinceNewSessionStart = hzl_TimeDelta(
            ctx->groupStates[gid].sessionStartInstant, now);
    const bool hasEnoughTimePassedSinceNewSessionStart =
            timeSinceNewSessionStart >
            6U * ctx->groupConfigs[gid].delayBetweenRenNotificationsMillis;
    return haveEnoughSecuredMessagesBeenUsed || hasEnoughTimePassedSinceNewSessionStart;
}

hzl_Err_t
hzl_ServerSessionRenewalPhaseEnter(hzl_ServerCtx_t* const ctx,
                                   const hzl_Gid_t gid)
{
    HZL_ERR_DECLARE(err);
    // Backup previous Session information
    memcpy(ctx->groupStates[gid].previousStk, ctx->groupStates[gid].currentStk, HZL_STK_LEN);
    ctx->groupStates[gid].previousRxLastMessageInstant =
            ctx->groupStates[gid].currentRxLastMessageInstant;
    ctx->groupStates[gid].previousCtrNonce = ctx->groupStates[gid].currentCtrNonce;
    // Start a new Session: set starting time, new random STK, reset counter nonce
    err = ctx->io.currentTime(&ctx->groupStates[gid].sessionStartInstant);
    HZL_ERR_CHECK(err);
    ctx->groupStates[gid].currentRxLastMessageInstant = ctx->groupStates[gid].sessionStartInstant;
    err = hzl_NonZeroTrng(ctx->groupStates[gid].currentStk, ctx->io.trng, HZL_STK_LEN);
    HZL_ERR_CHECK(err);
    ctx->groupStates[gid].currentCtrNonce = 0;
    return err;
}

/** @internal Verifies if a Group's current Session is expired and should be
 * thus renewed because it had too many messages sent or too much time passed
 * since the session being established, whichever comes first. */
inline static bool
hzl_ServerSessionIsExpired(const hzl_ServerCtx_t* const ctx,
                           const hzl_Timestamp_t now,
                           const hzl_Gid_t gid)
{
    const bool haveEnoughMessagesBeenSent =
            ctx->groupStates[gid].currentCtrNonce >= ctx->groupConfigs[gid].ctrNonceUpperLimit;
    const hzl_TimeDeltaMillis_t timeSinceSessionStart =
            hzl_TimeDelta(ctx->groupStates[gid].sessionStartInstant, now);
    const bool hasEnoughTimePassedSinceStart =
            timeSinceSessionStart > ctx->groupConfigs[gid].sessionDurationMillis;
    return haveEnoughMessagesBeenSent || hasEnoughTimePassedSinceStart;
}

inline static void
hzl_ServerSessionRenewalPhaseExit(hzl_ServerCtx_t* const ctx,
                                  const hzl_Gid_t gid)
{
    hzl_ZeroOut(ctx->groupStates[gid].previousStk, HZL_STK_LEN);
    ctx->groupStates[gid].previousRxLastMessageInstant = 0;
    ctx->groupStates[gid].previousCtrNonce = 0;
}

inline static void
hzl_RenHashInit(hzl_Hash_t* const hash,
                const uint8_t* const ltk,
                const hzl_Header_t* const unpackedRenHeader,
                const uint8_t* const encodedCtrnonce)
{
    // Authentication/validation of the msg with
    // tag = hash(LTK || label || GID || SID || PTY || ctrnonce)
    hzl_HashInit(hash);
    hzl_HashUpdate(hash, ltk, HZL_LTK_LEN);
    hzl_HashUpdate(hash, (uint8_t*) HZL_REN_LABEL, HZL_REN_LABEL_LEN);
    hzl_HashUpdate(hash, &unpackedRenHeader->gid, HZL_GID_LEN);
    hzl_HashUpdate(hash, &unpackedRenHeader->sid, HZL_SID_LEN);
    hzl_HashUpdate(hash, &unpackedRenHeader->pty, HZL_PTY_LEN);
    hzl_HashUpdate(hash, encodedCtrnonce, HZL_REN_CTRNONCE_LEN);
}

hzl_Err_t
hzl_ServerBuildMsgRenewal(hzl_CbsPduMsg_t* const reactionPdu,
                          hzl_ServerCtx_t* const ctx,
                          const hzl_Gid_t gid)
{
    // Prepare REN Header
    const hzl_Header_t unpackedRenHeader = {
            .gid = gid,
            .sid = HZL_SERVER_SID,
            .pty = HZL_PTY_REN,
    };
    const uint8_t packedHdrLen = hzl_HeaderLen(ctx->serverConfig->headerType);
    hzl_HeaderPackFunc const headerPackFunc =
            hzl_HeaderPackFuncForType(ctx->serverConfig->headerType);
    // Prepare REN Payload
    // Write the packed header at the beginning of the CAN FD frame's payload.
    headerPackFunc(reactionPdu->data, &unpackedRenHeader);
    // Write counter nonce after the header
    hzl_EncodeLe24(&reactionPdu->data[packedHdrLen + HZL_REN_CTRNONCE_IDX],
                   ctx->groupStates[gid].previousCtrNonce);
    // Authenticate the msg with
    // tag = hash(LTK || label || GID || SID || PTY || ctrnonce)
    hzl_Hash_t hash;
    hzl_RenHashInit(&hash, ctx->groupStates[gid].previousStk, &unpackedRenHeader,
                    &reactionPdu->data[packedHdrLen + HZL_REN_CTRNONCE_IDX]);
    hzl_HashDigest(&hash, &reactionPdu->data[packedHdrLen + HZL_REN_TAG_IDX],
                   HZL_REN_TAG_LEN);
    // Message is packed in binary format, ready to transmit
    reactionPdu->dataLen = packedHdrLen + HZL_REN_PAYLOAD_LEN;
    // Increment the counter nonce, regardless of transmission success
    hzl_ServerGroupIncrPreviousCtrnonce(ctx, gid);
    return HZL_OK; // Always succeeding
}

hzl_Err_t
hzl_ServerSessionRenewalPhaseEnterIfNeeded(hzl_CbsPduMsg_t* const reactionPdu,
                                           hzl_ServerCtx_t* const ctx,
                                           const hzl_Timestamp_t now,
                                           const hzl_Gid_t gid)
{
    HZL_ERR_DECLARE(err);
    if (hzl_ServerSessionIsExpired(ctx, now, gid))
    {
        err = hzl_ServerSessionRenewalPhaseEnter(ctx, gid);
        HZL_ERR_CHECK(err);
        err = hzl_ServerBuildMsgRenewal(reactionPdu, ctx, gid);
    }
    else
    {
        err = HZL_OK;
    }
    return err;
}

void
hzl_ServerSessionRenewalPhaseExitIfNeeded(hzl_ServerCtx_t* const ctx,
                                          const hzl_Timestamp_t now,
                                          const hzl_Gid_t gid)
{
    if (hzl_ServerSessionRenewalPhaseIsActive(ctx, gid)
        && hzl_ServerSessionRenewalPhaseIsOver(ctx, now, gid))
    {
        hzl_ServerSessionRenewalPhaseExit(ctx, gid);
    }
}
