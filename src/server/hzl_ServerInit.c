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
 * Implementation of the hzl_ServerInit() function.
 */

#include "hzl.h"
#include "hzl_Server.h"
#include "hzl_ServerInternal.h"
#include "hzl_CommonHeader.h"

/** @internal Verifies the content of the Server Configuration structure. */
static hzl_Err_t
hzl_ServerInitCheckServerConfig(const hzl_ServerConfig_t* const config)
{
    HZL_ERR_DECLARE(err);
    err = hzl_HeaderTypeCheck(config->headerType);
    HZL_ERR_CHECK(err);
    if (config->amountOfGroups == 0) { return HZL_ERR_ZERO_GROUPS; }
    const hzl_Gid_t maxGid = hzl_HeaderTypeMaxGid(config->headerType);
    const size_t maxAmountOfGroups = maxGid + 1U;  // [0, maxGid] = maxGid+1 possible groups
    if (config->amountOfGroups > maxAmountOfGroups)
    {
        return HZL_ERR_TOO_MANY_GROUPS_FOR_CONFIGURED_HEADER_TYPE;
    }
    if (config->amountOfClients == 0) { return HZL_ERR_ZERO_CLIENTS; }
    const hzl_Sid_t maxSid = hzl_HeaderTypeMaxSid(config->headerType);
    if (config->amountOfClients > maxSid)
    {
        return HZL_ERR_TOO_MANY_CLIENTS_FOR_CONFIGURED_HEADER_TYPE;
    }
    if (config->amountOfClients > HZL_SERVER_MAX_AMOUNT_OF_CLIENTS)
    {
        return HZL_ERR_TOO_MANY_CLIENTS;
    }
    return HZL_OK;
}

/** @internal Verifies the content of the array of Client Configuration structures. */
static hzl_Err_t
hzl_ServerInitCheckClientConfigs(const hzl_ServerCtx_t* const ctx)
{
    for (size_t i = 0U; i < ctx->serverConfig->amountOfClients; i++)
    {
        if (hzl_IsAllZeros(ctx->clientConfigs[i].ltk, HZL_LTK_LEN))
        {
            return HZL_ERR_LTK_IS_ALL_ZEROS;
        }
        if (ctx->clientConfigs[i].sid == HZL_SERVER_SID)
        {
            return HZL_ERR_SERVER_SID_ASSIGNED_TO_CLIENT;
        }
        if (i > 0)
        {
            // Note: skipping first loop (index 0), as we are checking against the previous index
            if (ctx->clientConfigs[i - 1U].sid >= ctx->clientConfigs[i].sid)
            {
                return HZL_ERR_SIDS_ARE_NOT_PRESORTED_STRICTLY_ASCENDING;
            }
            if (ctx->clientConfigs[i - 1U].sid + 1U != ctx->clientConfigs[i].sid)
            {
                return HZL_ERR_GAP_IN_SIDS;
            }
        }
    }
    return HZL_OK;
}

/** @internal True if the delay between successive REN message is valid.
 * The division by 6 is defined as an upper limit in the CBS protocol. */
inline static bool
hzl_ServerIsValidDelayBetweenRen(const hzl_ServerGroupConfig_t* const groupConfig)
{
    return groupConfig->delayBetweenRenNotificationsMillis > 0
           && groupConfig->delayBetweenRenNotificationsMillis <
              groupConfig->sessionDurationMillis / 6U;
}

/** @internal Bitmap containing all possible SIDs for a given amount of Clients.
 * Operates on 64 bits in case the bitmap is expanded from 32 to 64 bits
 * (32 at the time of writing).
 * Example: if amountOfClients==3, then allClientSids==0b111
 * Example: if amountOfClients==32, then allClientSids==0xFFFFFFFF */
inline static hzl_ServerBitMap_t
hzl_ServerAllClientsBitmap(const hzl_ServerCtx_t* const ctx)
{
    return (hzl_ServerBitMap_t)
            (UINT64_MAX >> (64U - ctx->serverConfig->amountOfClients));
}

/** @internal Verifies the content of the array of Client Configuration structures. */
static hzl_Err_t
hzl_ServerInitCheckGroupConfigs(const hzl_ServerCtx_t* const ctx)
{
    const hzl_ServerBitMap_t allClientSids = hzl_ServerAllClientsBitmap(ctx);
    if (ctx->groupConfigs[0].gid != HZL_BROADCAST_GID) { return HZL_ERR_MISSING_GID_0; }
    if ((ctx->groupConfigs[0].clientSidsInGroupBitmap & allClientSids) != allClientSids)
    {
        // The broadcast group bitmap must contain ALL the bits that map to the Clients
        // listed in the Clients config, but may contain SOME higher set bits, which are ignored.
        // This allows to reuse the same configuration structures, but just limit the amount
        // of clients to smaller value, without needing to change the broadcast group.
        // This also allows to set the broadcast bitmap to 0xFF...FF without worrying too much
        // about it.
        return HZL_ERR_CLIENTS_BITMAP_INVALID_BROADCAST_GROUP;
    }
    for (size_t i = 0U; i < ctx->serverConfig->amountOfGroups; i++)
    {
        if (i > 0)
        {
            // Note: skipping first loop (index 0), as we are checking against the previous index
            if (ctx->groupConfigs[i - 1U].gid >= ctx->groupConfigs[i].gid)
            {
                return HZL_ERR_GIDS_ARE_NOT_PRESORTED_STRICTLY_ASCENDING;
            }
            if (ctx->groupConfigs[i - 1U].gid + 1U != ctx->groupConfigs[i].gid)
            {
                return HZL_ERR_GAP_IN_GIDS;
            }
            // Note: skipping the first loo (index 0) as the broadcast group is already checked
            // outside of the loop.
            if (ctx->groupConfigs[i].clientSidsInGroupBitmap == 0U)
            {
                return HZL_ERR_CLIENTS_BITMAP_ZERO_CLIENTS;
            }
            if (ctx->groupConfigs[i].clientSidsInGroupBitmap & ~allClientSids)
            {
                // The bitmap contains some set bits outside of the possible range.
                return HZL_ERR_CLIENTS_BITMAP_UNKNOWN_SID;
            }
        }
        if (ctx->groupConfigs[i].maxCtrnonceDelayMsgs > HZL_LARGEST_MAX_COUNTER_NONCE_DELAY)
        {
            return HZL_ERR_INVALID_MAX_CTRNONCE_DELAY;
        }
        if (ctx->groupConfigs[i].ctrNonceUpperLimit >
            HZL_SERVER_MAX_COUNTER_NONCE_UPPER_LIMIT)
        {
            return HZL_ERR_TOO_LARGE_CTRNONCE_UPPER_LIMIT;
        }
        if (!hzl_ServerIsValidDelayBetweenRen(&ctx->groupConfigs[i]))
        {
            return HZL_ERR_INVALID_DELAY_BETWEEN_REN_NOTIFICATIONS;
        }
    }
    return HZL_OK;
}

hzl_Err_t
hzl_ServerCheckCtxPointers(const hzl_ServerCtx_t* const ctx)
{
    if (ctx == NULL) { return HZL_ERR_NULL_CTX; }
    if (ctx->serverConfig == NULL) { return HZL_ERR_NULL_CONFIG_SERVER; }
    if (ctx->clientConfigs == NULL) { return HZL_ERR_NULL_CONFIG_CLIENTS; }
    if (ctx->groupConfigs == NULL) { return HZL_ERR_NULL_CONFIG_GROUPS; }
    if (ctx->groupStates == NULL) { return HZL_ERR_NULL_STATES_GROUPS; }
    if (ctx->io.currentTime == NULL) { return HZL_ERR_NULL_CURRENT_TIME_FUNC; }
    if (ctx->io.trng == NULL) { return HZL_ERR_NULL_TRNG_FUNC; }
    return HZL_OK;
}

static hzl_Err_t
hzl_ServerCheckCtx(const hzl_ServerCtx_t* const ctx)
{
    HZL_ERR_DECLARE(err);
    err = hzl_ServerCheckCtxPointers(ctx);
    HZL_ERR_CHECK(err);
    err = hzl_ServerInitCheckServerConfig(ctx->serverConfig);
    HZL_ERR_CHECK(err);
    err = hzl_ServerInitCheckClientConfigs(ctx);
    HZL_ERR_CHECK(err);
    return hzl_ServerInitCheckGroupConfigs(ctx);
}

/** @internal Starts the current session of all Groups, clearing the remaining
 * data in the Group state. Sets the new STK and session starting time. */
static hzl_Err_t
hzl_ServerInitStartAllSessions(hzl_ServerCtx_t* const ctx)
{
    HZL_ERR_DECLARE(err);
    for (size_t i = 0; i < ctx->serverConfig->amountOfGroups; i++)
    {
        err = ctx->io.currentTime(&ctx->groupStates[i].sessionStartInstant);
        HZL_ERR_CHECK(err);
        // Upon Session initialisation or renewal, before the first Request in a Group,
        // currentRxLastMessageInstant is set to sessionStartInstant. When the Request is
        // received, currentRxLastMessageInstant is updated to a different value. This information
        // is used by the Server to know whether there is at least one Client in the Group
        // that is enabled to received Secured Application Data messages.
        ctx->groupStates[i].currentRxLastMessageInstant = ctx->groupStates[i].sessionStartInstant;
        ctx->groupStates[i].previousRxLastMessageInstant = 0;
        ctx->groupStates[i].currentCtrNonce = 0;
        ctx->groupStates[i].previousCtrNonce = 0;
        err = hzl_NonZeroTrng(ctx->groupStates[i].currentStk, ctx->io.trng, HZL_STK_LEN);
        HZL_ERR_CHECK(err);
        hzl_ZeroOut(ctx->groupStates[i].previousStk, HZL_STK_LEN);
    }
    return err;
}

HZL_API hzl_Err_t
hzl_ServerInit(hzl_ServerCtx_t* const ctx)
{
    HZL_ERR_DECLARE(err);
    err = hzl_ServerCheckCtx(ctx);
    HZL_ERR_CHECK(err);
    return hzl_ServerInitStartAllSessions(ctx);
}
