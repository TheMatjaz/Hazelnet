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
 * Tests of the hzl_ClientInit() function, part focusing on the errors
 * caused by the Group configs being incorrect.
 */

#include "hzlTest.h"

static void
hzlClientTest_ClientInitGroupConfigsMustBeNotNull(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    groupStates[0].currentCtrNonce = 42;
    hzl_ClientCtx_t ctx = {
            .clientConfig = &HZL_TEST_CORRECT_CLIENT_CONFIG,
            .groupConfigs = NULL,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    err = hzl_ClientInit(&ctx);

    atto_eq(err, HZL_ERR_NULL_CONFIG_GROUPS);
}

static void
hzlClientTest_ClientInitGroupConfigsGidsMustNotRepeat(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientGroupConfig_t
            modifiedGroupConfigs[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    memcpy(modifiedGroupConfigs, HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
           sizeof(hzl_ClientGroupConfig_t) * HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS);
    modifiedGroupConfigs[0].gid = 0;
    modifiedGroupConfigs[1].gid = 2;  // Repeated GID = not strictly ascending
    modifiedGroupConfigs[2].gid = 2;
    hzl_ClientCtx_t ctx = {
            .clientConfig = &HZL_TEST_CORRECT_CLIENT_CONFIG,
            .groupConfigs = modifiedGroupConfigs,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    err = hzl_ClientInit(&ctx);

    atto_eq(err, HZL_ERR_GIDS_ARE_NOT_PRESORTED_STRICTLY_ASCENDING);
    // Strict ascension requires the GIDs to be different AND increasing
}

static void
hzlClientTest_ClientInitGroupConfigsGidsMustBeSortedAscending(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientGroupConfig_t
            modifiedGroupConfigs[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    memcpy(modifiedGroupConfigs, HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
           sizeof(hzl_ClientGroupConfig_t) * HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS);
    modifiedGroupConfigs[0].gid = 0;
    modifiedGroupConfigs[1].gid = 2;  // Descending step = not strictly ascending
    modifiedGroupConfigs[2].gid = 1;
    hzl_ClientCtx_t ctx = {
            .clientConfig = &HZL_TEST_CORRECT_CLIENT_CONFIG,
            .groupConfigs = modifiedGroupConfigs,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    err = hzl_ClientInit(&ctx);

    atto_eq(err, HZL_ERR_GIDS_ARE_NOT_PRESORTED_STRICTLY_ASCENDING);
    // Strict ascension requires the GIDs to be different AND increasing
}

static void
hzlClientTest_ClientInitGroupConfigsGidZeroMustExist(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientGroupConfig_t
            modifiedGroupConfigs[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    memcpy(modifiedGroupConfigs, HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
           sizeof(hzl_ClientGroupConfig_t) * HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS);
    modifiedGroupConfigs[0].gid = 1;
    modifiedGroupConfigs[1].gid = 2;
    modifiedGroupConfigs[2].gid = 3;
    hzl_ClientCtx_t ctx = {
            .clientConfig = &HZL_TEST_CORRECT_CLIENT_CONFIG,
            .groupConfigs = modifiedGroupConfigs,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    err = hzl_ClientInit(&ctx);

    atto_eq(err, HZL_ERR_MISSING_GID_0);
}

static void
hzlClientTest_ClientInitGroupConfigsMaxCounterNonceDelayMustBeInValidRange(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientGroupConfig_t
            modifiedGroupConfigs[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    memcpy(modifiedGroupConfigs, HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
           sizeof(hzl_ClientGroupConfig_t) * HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS);
    hzl_ClientCtx_t ctx = {
            .clientConfig = &HZL_TEST_CORRECT_CLIENT_CONFIG,
            .groupConfigs = modifiedGroupConfigs,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    modifiedGroupConfigs[0].maxCtrnonceDelayMsgs =
            HZL_LARGEST_MAX_COUNTER_NONCE_DELAY + 1U;
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_ERR_INVALID_MAX_CTRNONCE_DELAY);

    modifiedGroupConfigs[0].maxCtrnonceDelayMsgs =
            HZL_LARGEST_MAX_COUNTER_NONCE_DELAY;
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
}

static void
hzlClientTest_ClientInitGroupConfigsGidsMustFitForHeaderType(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientConfig_t clientConfigWithSmallHeaderType =
            HZL_TEST_CORRECT_CLIENT_CONFIG;
    clientConfigWithSmallHeaderType.headerType = HZL_HEADER_4;  // At most 4 GIDs, so 3 = max
    clientConfigWithSmallHeaderType.sid = 1; // Adapt the SID to one that fits Header Type 4
    clientConfigWithSmallHeaderType.amountOfGroups = 2;  // Reduce groups to avoid unsorted GIDs
    hzl_ClientGroupConfig_t
            modifiedGroupConfigs[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    memcpy(modifiedGroupConfigs, HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
           sizeof(hzl_ClientGroupConfig_t) * HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS);
    hzl_ClientCtx_t ctx = {
            .clientConfig = &clientConfigWithSmallHeaderType,
            .groupConfigs = modifiedGroupConfigs,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    modifiedGroupConfigs[1].gid = 4;  // 1 larger than max
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_ERR_GID_TOO_LARGE_FOR_CONFIGURED_HEADER_TYPE);

    modifiedGroupConfigs[1].gid = 3;  // Fits
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
}

void hzlClientTest_ClientInitCheckGroupConfigs(void)
{
    hzlClientTest_ClientInitGroupConfigsMustBeNotNull();
    hzlClientTest_ClientInitGroupConfigsGidsMustNotRepeat();
    hzlClientTest_ClientInitGroupConfigsGidsMustBeSortedAscending();
    hzlClientTest_ClientInitGroupConfigsGidZeroMustExist();
    hzlClientTest_ClientInitGroupConfigsMaxCounterNonceDelayMustBeInValidRange();
    hzlClientTest_ClientInitGroupConfigsGidsMustFitForHeaderType();
    HZL_TEST_PARTIAL_REPORT();
}
