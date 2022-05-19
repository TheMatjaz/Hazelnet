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
 * @internal Implementation of operations on Client Group configuration and states.
 */

#include "hzl_ClientInternal.h"
#include "hzl_CommonMessage.h"
#include "hzl_CommonInternal.h"

static bool
hzl_ClientSessionRenewalPhaseIsActive(const hzl_ClientGroup_t* group);

hzl_Err_t
hzl_ClientFindGroup(hzl_ClientGroup_t* const group,
                    const hzl_ClientCtx_t* const ctx,
                    const hzl_Gid_t groupId)
{
    for (size_t i = 0; i < ctx->clientConfig->amountOfGroups; i++)
    {
        if (ctx->groupConfigs[i].gid == groupId)
        {
            group->config = &ctx->groupConfigs[i];
            group->state = &ctx->groupStates[i];
            return HZL_OK;
        }
    }
    return HZL_ERR_UNKNOWN_GROUP;
}

bool
hzl_ClientIsSessionEstablishedAndValid(const hzl_ClientGroup_t* const group)
{
    return !hzl_IsAllZeros(group->state->currentStk, HZL_STK_LEN)
           && !HZL_IS_CTRNONCE_EXPIRED(group->state->currentCtrNonce);
}

hzl_Err_t
hzl_ClientIsAHandShakeOngoing(bool* const isOngoing,
                              const hzl_ClientCtx_t* const ctx,
                              const hzl_ClientGroup_t* const group)
{
    HZL_ERR_DECLARE(err);
    const bool wasRequestTransmitted = group->state->requestNonce
                                       != HZL_REQNONCE_NOT_EXPECTING_A_RESPONSE;
    hzl_Timestamp_t now;
    err = ctx->io.currentTime(&now);
    HZL_ERR_CHECK(err);
    const hzl_TimeDeltaMillis_t elapsedSinceTimestamp = hzl_TimeDelta(
            group->state->lastHandshakeEventInstant, now);
    bool isResponseTimeoutExpired =
            (elapsedSinceTimestamp > ctx->clientConfig->timeoutReqToResMillis);
    *isOngoing = wasRequestTransmitted && !isResponseTimeoutExpired;
    return err;
}

hzl_Err_t
hzl_ClientIsResponseAcceptable(const hzl_ClientCtx_t* const ctx,
                               const hzl_ClientGroup_t* const group,
                               const hzl_Timestamp_t rxTimestamp)
{
    const bool wasRequestTransmitted = group->state->requestNonce
                                       != HZL_REQNONCE_NOT_EXPECTING_A_RESPONSE;
    if (!wasRequestTransmitted)
    {
        return HZL_ERR_SECWARN_NOT_EXPECTING_A_RESPONSE;
    }
    const hzl_TimeDeltaMillis_t deltaReqTxToResRx = hzl_TimeDelta(
            group->state->lastHandshakeEventInstant, rxTimestamp);
    if (deltaReqTxToResRx >
        ctx->clientConfig->timeoutReqToResMillis)
    {
        return HZL_ERR_SECWARN_RESPONSE_TIMEOUT;
    }
    return HZL_OK;
}

bool
hzl_ClientIsRenewalAcceptable(const hzl_ClientGroup_t* group)
{
    return !hzl_ClientSessionRenewalPhaseIsActive(group)
           && group->state->requestNonce == HZL_REQNONCE_NOT_EXPECTING_A_RESPONSE;
}

hzl_Err_t
hzl_ClientSetRequestTxTimeToNow(const hzl_ClientCtx_t* const ctx,
                                const hzl_ClientGroup_t* const group)
{
    HZL_ERR_DECLARE(err);
    hzl_Timestamp_t tempTimestamp = 0;
    // Pass a temporary location to the currentTime to avoid overwriting the old
    // timestamp in case of errors.
    err = ctx->io.currentTime(&tempTimestamp);
    HZL_ERR_CHECK(err);
    group->state->lastHandshakeEventInstant = tempTimestamp;
    return err;
}

void
hzl_ClientGroupIncrCurrentCtrnonce(const hzl_ClientGroup_t* const group)
{
    if (!HZL_IS_CTRNONCE_EXPIRED(group->state->currentCtrNonce))
    {
        group->state->currentCtrNonce++;
    }
}

inline static void
hzl_ClientGroupIncrPreviousCtrnonce(const hzl_ClientGroup_t* const group)
{
    if (!HZL_IS_CTRNONCE_EXPIRED(group->state->previousCtrNonce))
    {
        group->state->previousCtrNonce++;
    }
}

void
hzl_ClientGroupUpdateCtrnonceAndRxTimestamp(const hzl_ClientGroup_t* const group,
                                            const hzl_CtrNonce_t receivedCtrnonce,
                                            const hzl_Timestamp_t receptionTimestamp,
                                            const bool isPreviousSession)
{
    if (isPreviousSession)
    {
        if (receivedCtrnonce > group->state->previousCtrNonce)
        {
            group->state->previousCtrNonce = receivedCtrnonce;
        }
        hzl_ClientGroupIncrPreviousCtrnonce(group);
        group->state->previousRxLastMessageInstant = receptionTimestamp;
    }
    else
    {
        if (receivedCtrnonce > group->state->currentCtrNonce)
        {
            group->state->currentCtrNonce = receivedCtrnonce;
        }
        hzl_ClientGroupIncrCurrentCtrnonce(group);
        group->state->currentRxLastMessageInstant = receptionTimestamp;
    }
}

void
hzl_ClientSessionRenewalPhaseEnter(const hzl_ClientGroup_t* const group)
{
    memcpy(group->state->previousStk, group->state->currentStk, HZL_STK_LEN);
    group->state->previousRxLastMessageInstant = group->state->currentRxLastMessageInstant;
    group->state->previousCtrNonce = group->state->currentCtrNonce;
}

inline static bool
hzl_ClientSessionRenewalPhaseIsActive(const hzl_ClientGroup_t* const group)
{
    return !hzl_IsAllZeros(group->state->previousStk, HZL_STK_LEN);
}

inline static void
hzl_ClientSessionRenewalPhaseExit(const hzl_ClientGroup_t* const group)
{
    hzl_ZeroOut(group->state->previousStk, HZL_STK_LEN);
    group->state->previousRxLastMessageInstant = 0;
    group->state->previousCtrNonce = 0;
}

inline static bool
hzl_ClientSessionRenewalPhaseIsOver(const hzl_ClientGroup_t* const group,
                                    const hzl_Timestamp_t now)
{
    // The 2 is a multiplier coming strictly from the CBS protocol specification.
    const bool haveEnoughSecuredMessagesBeenUsed =
            group->state->currentCtrNonce >= 2U * group->config->maxCtrnonceDelayMsgs;
    const hzl_TimeDeltaMillis_t deltaSinceRxResponse = hzl_TimeDelta(
            group->state->lastHandshakeEventInstant, now);
    const bool hasEnoughTimePassedSinceResponse =
            deltaSinceRxResponse > group->config->sessionRenewalDurationMillis;
    return haveEnoughSecuredMessagesBeenUsed || hasEnoughTimePassedSinceResponse;
}

void
hzl_ClientSessionRenewalPhaseExitIfNeeded(const hzl_ClientGroup_t* const group,
                                          const hzl_Timestamp_t now)
{
    if (hzl_ClientSessionRenewalPhaseIsActive(group)
        && hzl_ClientSessionRenewalPhaseIsOver(group, now))
    {
        hzl_ClientSessionRenewalPhaseExit(group);
    }
}

inline static bool
hzl_ClientIsCtrNonceOfPreviousSession(const hzl_ClientGroup_t* const group,
                                      const hzl_CtrNonce_t receivedCtrnonce)
{
    const hzl_CtrNonce_t average = (hzl_CtrNonce_t)
            ((group->state->currentCtrNonce + group->state->previousCtrNonce) / 2U);
    return receivedCtrnonce >= average;
}

hzl_Err_t
hzl_ClientCheckRxCtrnonce(bool* const isPreviousSession,
                          const hzl_ClientGroup_t* const group,
                          const hzl_CtrNonce_t receivedCtrnonce,
                          const hzl_Timestamp_t rxTimestamp)
{
    if (HZL_IS_CTRNONCE_EXPIRED(receivedCtrnonce))
    {
        return HZL_ERR_SECWARN_RECEIVED_OVERFLOWN_NONCE;
    }
    // Check if belongs to the old or new session during a renewal phase
    const bool isPrevious =
            isPreviousSession != NULL
            && hzl_ClientSessionRenewalPhaseIsActive(group)
            && hzl_ClientIsCtrNonceOfPreviousSession(group, receivedCtrnonce);
    hzl_Timestamp_t selectedLastRxTimestamp;
    hzl_CtrNonce_t selectedCtrNonce;
    if (isPrevious)
    {
        selectedLastRxTimestamp = group->state->previousRxLastMessageInstant;
        selectedCtrNonce = group->state->previousCtrNonce;
    }
    else
    {
        selectedLastRxTimestamp = group->state->currentRxLastMessageInstant;
        selectedCtrNonce = group->state->currentCtrNonce;
    }
    // Freshness of received ctrnonce compared to the ctrnonce of the last
    // received message of the previous or current session, depending where
    // the received message belongs.
    const hzl_CtrNonce_t delay = hzl_CommonCtrDelay(
            selectedLastRxTimestamp,
            rxTimestamp,
            group->config->maxCtrnonceDelayMsgs,
            group->config->maxSilenceIntervalMillis);
    // Casting to signed to avoid compiler errors. Counter nonces anyway use
    // only 24 bits, so the signed value is the same as the unsigned.
    const int32_t oldestToleratedCtrNonce = (int32_t) selectedCtrNonce - (int32_t) delay;
    if ((int32_t) receivedCtrnonce < oldestToleratedCtrNonce)
    {
        return HZL_ERR_SECWARN_OLD_MESSAGE;
    }
    if (isPreviousSession != NULL)
    {
        *isPreviousSession = isPrevious;
    }
    return HZL_OK;
}
