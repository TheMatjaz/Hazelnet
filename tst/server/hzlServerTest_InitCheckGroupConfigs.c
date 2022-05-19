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
 * Tests of the hzl_ServerInit() function, part focusing on the errors
 * caused by the Groups configs being incorrect.
 */

#include "hzlTest.h"

static void
hzlServerTest_ServerInitGroupConfigsMustBeNotNull(void)
{
    hzl_Err_t err;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &HZL_TEST_CORRECT_SERVER_CONFIG,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = NULL,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    err = hzl_ServerInit(&ctx);

    atto_eq(err, HZL_ERR_NULL_CONFIG_GROUPS);
}

static void
hzlServerTest_ServerInitGroupConfigsMustHaveSortedGidsStrictlyAscending(void)
{
    hzl_Err_t err;
    hzl_ServerGroupConfig_t modifiedGroupConfigs[HZL_MAX_TEST_AMOUNT_OF_GROUPS];
    memcpy(modifiedGroupConfigs,
           HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
           sizeof(hzl_ServerGroupConfig_t) * HZL_MAX_TEST_AMOUNT_OF_GROUPS);
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &HZL_TEST_CORRECT_SERVER_CONFIG,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = modifiedGroupConfigs,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    modifiedGroupConfigs[2].gid = 1; // Repeated GID
    atto_eq(modifiedGroupConfigs[1].gid, modifiedGroupConfigs[2].gid);
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_ERR_GIDS_ARE_NOT_PRESORTED_STRICTLY_ASCENDING);

    modifiedGroupConfigs[2].gid = 0; // Strictly descending
    atto_gt(modifiedGroupConfigs[1].gid, modifiedGroupConfigs[2].gid);
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_ERR_GIDS_ARE_NOT_PRESORTED_STRICTLY_ASCENDING);
}

static void
hzlServerTest_ServerInitGroupConfigsGidZeroMustExist(void)
{
    hzl_Err_t err;
    hzl_ServerGroupConfig_t modifiedGroupConfigs[HZL_MAX_TEST_AMOUNT_OF_GROUPS];
    memcpy(modifiedGroupConfigs,
           HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
           sizeof(hzl_ServerGroupConfig_t) * HZL_MAX_TEST_AMOUNT_OF_GROUPS);
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &HZL_TEST_CORRECT_SERVER_CONFIG,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = modifiedGroupConfigs,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    modifiedGroupConfigs[0].gid = 1;

    err = hzl_ServerInit(&ctx);

    atto_eq(err, HZL_ERR_MISSING_GID_0);
}

static void
hzlServerTest_ServerInitGroupConfigsMustHaveGidsWithoutGaps(void)
{
    hzl_Err_t err;
    hzl_ServerGroupConfig_t modifiedGroupConfigs[HZL_MAX_TEST_AMOUNT_OF_GROUPS];
    memcpy(modifiedGroupConfigs,
           HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
           sizeof(hzl_ServerGroupConfig_t) * HZL_MAX_TEST_AMOUNT_OF_GROUPS);
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &HZL_TEST_CORRECT_SERVER_CONFIG,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = modifiedGroupConfigs,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    modifiedGroupConfigs[1].gid = 3; // Gap between first and second GID
    atto_lt(modifiedGroupConfigs[0].gid + 1, modifiedGroupConfigs[1].gid);

    err = hzl_ServerInit(&ctx);

    atto_eq(err, HZL_ERR_GAP_IN_GIDS);
}

static void
hzlServerTest_ServerInitGroupConfigsMaxCounterNonceDelayMustBeInValidRange(void)
{
    hzl_Err_t err;
    hzl_ServerGroupConfig_t modifiedGroupConfigs[HZL_MAX_TEST_AMOUNT_OF_GROUPS];
    memcpy(modifiedGroupConfigs,
           HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
           sizeof(hzl_ServerGroupConfig_t) * HZL_MAX_TEST_AMOUNT_OF_GROUPS);
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &HZL_TEST_CORRECT_SERVER_CONFIG,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = modifiedGroupConfigs,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    modifiedGroupConfigs[1].maxCtrnonceDelayMsgs =
            HZL_LARGEST_MAX_COUNTER_NONCE_DELAY + 1;
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_ERR_INVALID_MAX_CTRNONCE_DELAY);

    modifiedGroupConfigs[1].maxCtrnonceDelayMsgs = HZL_LARGEST_MAX_COUNTER_NONCE_DELAY;
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
}

static void
hzlServerTest_ServerInitGroupConfigsCounterNonceUpperLimitMustBeInValidRange(void)
{
    hzl_Err_t err;
    hzl_ServerGroupConfig_t modifiedGroupConfigs[HZL_MAX_TEST_AMOUNT_OF_GROUPS];
    memcpy(modifiedGroupConfigs,
           HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
           sizeof(hzl_ServerGroupConfig_t) * HZL_MAX_TEST_AMOUNT_OF_GROUPS);
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &HZL_TEST_CORRECT_SERVER_CONFIG,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = modifiedGroupConfigs,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    modifiedGroupConfigs[1].ctrNonceUpperLimit =
            HZL_SERVER_MAX_COUNTER_NONCE_UPPER_LIMIT + 1;
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_ERR_TOO_LARGE_CTRNONCE_UPPER_LIMIT);

    modifiedGroupConfigs[1].ctrNonceUpperLimit = HZL_SERVER_MAX_COUNTER_NONCE_UPPER_LIMIT;
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
}

static void
hzlServerTest_ServerInitGroupConfigsDelayBetweenRenMustBeInValidRange(void)
{
    hzl_Err_t err;
    hzl_ServerGroupConfig_t modifiedGroupConfigs[HZL_MAX_TEST_AMOUNT_OF_GROUPS];
    memcpy(modifiedGroupConfigs,
           HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
           sizeof(hzl_ServerGroupConfig_t) * HZL_MAX_TEST_AMOUNT_OF_GROUPS);
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &HZL_TEST_CORRECT_SERVER_CONFIG,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = modifiedGroupConfigs,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    modifiedGroupConfigs[1].delayBetweenRenNotificationsMillis = 0;
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_ERR_INVALID_DELAY_BETWEEN_REN_NOTIFICATIONS);

    modifiedGroupConfigs[1].delayBetweenRenNotificationsMillis = 1;
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);

    modifiedGroupConfigs[1].delayBetweenRenNotificationsMillis =
            modifiedGroupConfigs[1].sessionDurationMillis / 6U;
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_ERR_INVALID_DELAY_BETWEEN_REN_NOTIFICATIONS);

    modifiedGroupConfigs[1].delayBetweenRenNotificationsMillis =
            modifiedGroupConfigs[1].sessionDurationMillis / 6U - 1U;
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
}

static void
hzlServerTest_ServerInitGroupConfigsBitmapMustHaveAtLeastOneClient(void)
{
    hzl_Err_t err;
    hzl_ServerGroupConfig_t modifiedGroupConfigs[HZL_MAX_TEST_AMOUNT_OF_GROUPS];
    memcpy(modifiedGroupConfigs,
           HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
           sizeof(hzl_ServerGroupConfig_t) * HZL_MAX_TEST_AMOUNT_OF_GROUPS);
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &HZL_TEST_CORRECT_SERVER_CONFIG,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = modifiedGroupConfigs,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    modifiedGroupConfigs[1].clientSidsInGroupBitmap = 0;
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_ERR_CLIENTS_BITMAP_ZERO_CLIENTS);

    modifiedGroupConfigs[1].clientSidsInGroupBitmap = 1;
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
}

static void
hzlServerTest_ServerInitGroupConfigsBitmapMustHaveKnownClients(void)
{
    hzl_Err_t err;
    hzl_ServerGroupConfig_t modifiedGroupConfigs[HZL_MAX_TEST_AMOUNT_OF_GROUPS];
    memcpy(modifiedGroupConfigs,
           HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
           sizeof(hzl_ServerGroupConfig_t) * HZL_MAX_TEST_AMOUNT_OF_GROUPS);
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &HZL_TEST_CORRECT_SERVER_CONFIG,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = modifiedGroupConfigs,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    // Bits outside of the range of known clients
    modifiedGroupConfigs[1].clientSidsInGroupBitmap = 0xFFFFFFFF;
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_ERR_CLIENTS_BITMAP_UNKNOWN_SID);

    // ALL known clients, but no extra ones.
    modifiedGroupConfigs[1].clientSidsInGroupBitmap =
            (1U << ctx.serverConfig->amountOfClients) - 1U;
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
}

static void
hzlServerTest_ServerInitGroupConfigsBitmapMustHaveCompleteBroadcastGroup(void)
{
    hzl_Err_t err;
    hzl_ServerGroupConfig_t modifiedGroupConfigs[HZL_MAX_TEST_AMOUNT_OF_GROUPS];
    memcpy(modifiedGroupConfigs,
           HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
           sizeof(hzl_ServerGroupConfig_t) * HZL_MAX_TEST_AMOUNT_OF_GROUPS);
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &HZL_TEST_CORRECT_SERVER_CONFIG,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = modifiedGroupConfigs,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    hzl_ServerBitMap_t broadcastBitmap = 0;
    for (size_t i = 0U; i < ctx.serverConfig->amountOfClients; i++)
    {
        broadcastBitmap |= 1U << i;
    }

    // Subset of bits is rejected.
    modifiedGroupConfigs[0].clientSidsInGroupBitmap = broadcastBitmap >> 1U;
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_ERR_CLIENTS_BITMAP_INVALID_BROADCAST_GROUP);

    modifiedGroupConfigs[0].clientSidsInGroupBitmap = ~(broadcastBitmap & 2);
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_ERR_CLIENTS_BITMAP_INVALID_BROADCAST_GROUP);

    modifiedGroupConfigs[0].clientSidsInGroupBitmap = broadcastBitmap;
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);

    // One bit higher than the rest of the bitmap
    modifiedGroupConfigs[0].clientSidsInGroupBitmap = (broadcastBitmap << 1U) | 1U;
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
}

void hzlServerTest_ServerInitCheckGroupConfigs(void)
{
    hzlServerTest_ServerInitGroupConfigsMustBeNotNull();
    hzlServerTest_ServerInitGroupConfigsMustHaveSortedGidsStrictlyAscending();
    hzlServerTest_ServerInitGroupConfigsGidZeroMustExist();
    hzlServerTest_ServerInitGroupConfigsMustHaveGidsWithoutGaps();
    hzlServerTest_ServerInitGroupConfigsMaxCounterNonceDelayMustBeInValidRange();
    hzlServerTest_ServerInitGroupConfigsCounterNonceUpperLimitMustBeInValidRange();
    hzlServerTest_ServerInitGroupConfigsDelayBetweenRenMustBeInValidRange();
    hzlServerTest_ServerInitGroupConfigsBitmapMustHaveAtLeastOneClient();
    hzlServerTest_ServerInitGroupConfigsBitmapMustHaveKnownClients();
    hzlServerTest_ServerInitGroupConfigsBitmapMustHaveCompleteBroadcastGroup();
    HZL_TEST_PARTIAL_REPORT();
}
