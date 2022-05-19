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
 * Implementation of the hzl_ClientInit() function.
 */

#include "hzl.h"
#include "hzl_Client.h"
#include "hzl_ClientInternal.h"
#include "hzl_CommonHeader.h"
#include "hzl_CommonInternal.h"

/** @internal Verifies the content of the Client Configuration structure. */
static hzl_Err_t
hzl_ClientInitCheckClientConfig(const hzl_ClientConfig_t* const config)
{
    HZL_ERR_DECLARE(err);
    if (hzl_IsAllZeros(config->ltk, HZL_LTK_LEN))
    {
        return HZL_ERR_LTK_IS_ALL_ZEROS;
    }
    if (config->sid == HZL_SERVER_SID) { return HZL_ERR_SERVER_SID_ASSIGNED_TO_CLIENT; }
    err = hzl_HeaderTypeCheck(config->headerType);
    HZL_ERR_CHECK(err);
    const hzl_Sid_t maxSid = hzl_HeaderTypeMaxSid(config->headerType);
    if (config->sid > maxSid) { return HZL_ERR_SID_TOO_LARGE_FOR_CONFIGURED_HEADER_TYPE; }
    if (config->amountOfGroups == 0) { return HZL_ERR_ZERO_GROUPS; }
    const hzl_Gid_t maxGid = hzl_HeaderTypeMaxGid(config->headerType);
    const size_t maxAmountOfGroups = maxGid + 1U;  // [0, maxGid] = maxGid+1 possible groups
    if (config->amountOfGroups > maxAmountOfGroups)
    {
        return HZL_ERR_TOO_MANY_GROUPS_FOR_CONFIGURED_HEADER_TYPE;
    }
    return err;
}

/** @internal Verifies the content of the Groups Configurations array of structures. */
static hzl_Err_t
hzl_ClientInitCheckGroupConfigs(const hzl_ClientConfig_t* const clientConfig,
                                const hzl_ClientGroupConfig_t* const groupConfigs)
{
    if (groupConfigs[0].gid != HZL_BROADCAST_GID) { return HZL_ERR_MISSING_GID_0; }
    const hzl_Gid_t maxGid = hzl_HeaderTypeMaxGid(clientConfig->headerType);
    for (size_t i = 0U; i < clientConfig->amountOfGroups; i++)
    {
        if (groupConfigs[i].maxCtrnonceDelayMsgs >
            HZL_LARGEST_MAX_COUNTER_NONCE_DELAY)
        {
            return HZL_ERR_INVALID_MAX_CTRNONCE_DELAY;
        }
        if (groupConfigs[i].gid > maxGid)
        {
            return HZL_ERR_GID_TOO_LARGE_FOR_CONFIGURED_HEADER_TYPE;
        }
        if (i > 0)
        {
            // Note: skipping first loop (index 0), as we are checking against the previous index
            if (groupConfigs[i - 1U].gid >= groupConfigs[i].gid)
            {
                return HZL_ERR_GIDS_ARE_NOT_PRESORTED_STRICTLY_ASCENDING;
            }
        }
    }
    return HZL_OK;
}

hzl_Err_t
hzl_ClientCheckCtxPointers(const hzl_ClientCtx_t* const ctx)
{
    if (ctx == NULL) { return HZL_ERR_NULL_CTX; }
    if (ctx->clientConfig == NULL) { return HZL_ERR_NULL_CONFIG_CLIENT; }
    if (ctx->groupConfigs == NULL) { return HZL_ERR_NULL_CONFIG_GROUPS; }
    if (ctx->groupStates == NULL) { return HZL_ERR_NULL_STATES_GROUPS; }
    if (ctx->io.currentTime == NULL) { return HZL_ERR_NULL_CURRENT_TIME_FUNC; }
    if (ctx->io.trng == NULL) { return HZL_ERR_NULL_TRNG_FUNC; }
    return HZL_OK;
}

hzl_Err_t
hzl_ClientCheckCtx(const hzl_ClientCtx_t* const ctx)
{
    HZL_ERR_DECLARE(err);
    err = hzl_ClientCheckCtxPointers(ctx);
    HZL_ERR_CHECK(err);
    err = hzl_ClientInitCheckClientConfig(ctx->clientConfig);
    HZL_ERR_CHECK(err);
    err = hzl_ClientInitCheckGroupConfigs(ctx->clientConfig, ctx->groupConfigs);
    return err;
}

void
hzl_ClientClearStateUnchecked(hzl_ClientCtx_t* const ctx)
{
    hzl_ZeroOut(ctx->groupStates,
                ctx->clientConfig->amountOfGroups * sizeof(hzl_ClientGroupState_t));
}

HZL_API hzl_Err_t
hzl_ClientInit(hzl_ClientCtx_t* const ctx)
{
    HZL_ERR_DECLARE(err);
    err = hzl_ClientCheckCtx(ctx);
    HZL_ERR_CHECK(err);
    hzl_ClientClearStateUnchecked(ctx);
    return err;
}
