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
 * caused by the client config being incorrect.
 */

#include "hzlTest.h"

static void
hzlClientTest_ClientInitConfigMustBeNotNull(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientCtx_t ctx = {
            .clientConfig = NULL,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    err = hzl_ClientInit(&ctx);

    atto_eq(err, HZL_ERR_NULL_CONFIG_CLIENT);
}

static void
hzlClientTest_ClientInitConfigAmountOfGroupsMustBePositive(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientConfig_t incorrectConfig = HZL_TEST_CORRECT_CLIENT_CONFIG;
    incorrectConfig.amountOfGroups = 0;
    hzl_ClientCtx_t ctx = {
            .clientConfig = &incorrectConfig,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    err = hzl_ClientInit(&ctx);

    atto_eq(err, HZL_ERR_ZERO_GROUPS);
}

static void
hzlClientTest_ClientInitConfigLtkMustBeNonZeros(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientConfig_t incorrectConfig = HZL_TEST_CORRECT_CLIENT_CONFIG;
    memset(incorrectConfig.ltk, 0, HZL_LTK_LEN);
    hzl_ClientCtx_t ctx = {
            .clientConfig = &incorrectConfig,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    err = hzl_ClientInit(&ctx);

    atto_eq(err, HZL_ERR_LTK_IS_ALL_ZEROS);
}

static void
hzlClientTest_ClientInitConfigHeaderTypeMustBeStandard(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientConfig_t incorrectConfig = HZL_TEST_CORRECT_CLIENT_CONFIG;
    incorrectConfig.headerType = 99;
    hzl_ClientCtx_t ctx = {
            .clientConfig = &incorrectConfig,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    err = hzl_ClientInit(&ctx);

    atto_eq(err, HZL_ERR_INVALID_HEADER_TYPE);
}

static void
hzlClientTest_ClientInitConfigClientSidMustBeNonZero(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientConfig_t incorrectConfig = HZL_TEST_CORRECT_CLIENT_CONFIG;
    incorrectConfig.sid = HZL_SERVER_SID;
    hzl_ClientCtx_t ctx = {
            .clientConfig = &incorrectConfig,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    err = hzl_ClientInit(&ctx);

    atto_eq(err, HZL_ERR_SERVER_SID_ASSIGNED_TO_CLIENT);
}

static void
hzlClientTest_ClientInitConfigClientSidMustFitForHeaderType(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientConfig_t clientConfigWithSmallHeaderType =
            HZL_TEST_CORRECT_CLIENT_CONFIG;
    clientConfigWithSmallHeaderType.headerType = HZL_HEADER_4;  // At most 8 SIDs, so 7 = max
    hzl_ClientCtx_t ctx = {
            .clientConfig = &clientConfigWithSmallHeaderType,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    clientConfigWithSmallHeaderType.sid = 8;  // 1 larger than max
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_ERR_SID_TOO_LARGE_FOR_CONFIGURED_HEADER_TYPE);

    clientConfigWithSmallHeaderType.sid = 7;  // Fits
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
}

static void
hzlClientTest_ClientInitConfigClientAmountOfGroupsMustFitForHeaderType(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_MAX_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientConfig_t clientConfigWithSmallHeaderType =
            HZL_TEST_CORRECT_CLIENT_CONFIG;
    clientConfigWithSmallHeaderType.sid = 2;
    hzl_ClientCtx_t ctx = {
            .clientConfig = &clientConfigWithSmallHeaderType,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    clientConfigWithSmallHeaderType.headerType = HZL_HEADER_3;  // max 8 groups
    clientConfigWithSmallHeaderType.amountOfGroups = 9;  // 1 more than max
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_ERR_TOO_MANY_GROUPS_FOR_CONFIGURED_HEADER_TYPE);
    clientConfigWithSmallHeaderType.amountOfGroups = 8;  // == max
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_ERR_GID_TOO_LARGE_FOR_CONFIGURED_HEADER_TYPE); // EXPECTED error!
    // Because the HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS contains (on purpose) a gap
    // (the GID == 1 is missing), if we provide 8 groups, the last will have GID == 8, with the
    // maxGID being 7, thus is rejected. BUT this is a different check, so the amount of Groups
    // by itself is accepted if it's within the range allowed by the header.

    clientConfigWithSmallHeaderType.headerType = HZL_HEADER_5;  // max 1 group
    clientConfigWithSmallHeaderType.amountOfGroups = 2;  // 1 more than max
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_ERR_TOO_MANY_GROUPS_FOR_CONFIGURED_HEADER_TYPE);
    clientConfigWithSmallHeaderType.amountOfGroups = 1;  // == max
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
}

static void
hzlClientTest_ClientInitConfigClientMustHaveOnlyBroadcastWhenHeader5Or6(void)
{
    // Headers 5 or 6 don't have a field for the GID, as this is assumed to
    // be the only group. Thus there must be only 1 group configuration and it
    // must be the one for the GID 0.
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientConfig_t clientConfigWithSmallHeaderType =
            HZL_TEST_CORRECT_CLIENT_CONFIG;
    clientConfigWithSmallHeaderType.headerType = HZL_HEADER_6;  // No GID field
    hzl_ClientCtx_t ctx = {
            .clientConfig = &clientConfigWithSmallHeaderType,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    clientConfigWithSmallHeaderType.amountOfGroups = 2;  // 1 more than max
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_ERR_TOO_MANY_GROUPS_FOR_CONFIGURED_HEADER_TYPE);

    clientConfigWithSmallHeaderType.amountOfGroups = 1;  // == max
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
}

void hzlClientTest_ClientInitCheckClientConfig(void)
{
    hzlClientTest_ClientInitConfigMustBeNotNull();
    hzlClientTest_ClientInitConfigAmountOfGroupsMustBePositive();
    hzlClientTest_ClientInitConfigLtkMustBeNonZeros();
    hzlClientTest_ClientInitConfigHeaderTypeMustBeStandard();
    hzlClientTest_ClientInitConfigClientSidMustBeNonZero();
    hzlClientTest_ClientInitConfigClientSidMustFitForHeaderType();
    hzlClientTest_ClientInitConfigClientAmountOfGroupsMustFitForHeaderType();
    hzlClientTest_ClientInitConfigClientMustHaveOnlyBroadcastWhenHeader5Or6();
    HZL_TEST_PARTIAL_REPORT();
}
