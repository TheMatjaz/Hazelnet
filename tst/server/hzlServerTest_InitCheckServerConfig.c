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
 * caused by the Server config being incorrect.
 */

#include "hzlTest.h"

static void
hzlServerTest_ServerInitConfigMustBeNotNull(void)
{
    hzl_Err_t err;
    hzl_ServerCtx_t ctx = {
            .serverConfig = NULL,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = NULL,
            .io = HZL_TEST_CORRECT_IO,
    };

    err = hzl_ServerInit(&ctx);

    atto_eq(err, HZL_ERR_NULL_CONFIG_SERVER);
}

static void
hzlServerTest_ServerInitConfigAmountOfGroupsMustBePositive(void)
{
    hzl_Err_t err;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerConfig_t modifiedServerConfig = HZL_TEST_CORRECT_SERVER_CONFIG;
    hzl_ServerCtx_t ctx = {
            .serverConfig = &modifiedServerConfig,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    modifiedServerConfig.amountOfGroups = 0;

    err = hzl_ServerInit(&ctx);

    atto_eq(err, HZL_ERR_ZERO_GROUPS);
}

static void
hzlServerTest_ServerInitConfigAmountOfClientsMustBePositive(void)
{
    hzl_Err_t err;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerConfig_t modifiedServerConfig = HZL_TEST_CORRECT_SERVER_CONFIG;
    hzl_ServerCtx_t ctx = {
            .serverConfig = &modifiedServerConfig,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    modifiedServerConfig.amountOfClients = 0;

    err = hzl_ServerInit(&ctx);

    atto_eq(err, HZL_ERR_ZERO_CLIENTS);
}

static void
hzlServerTest_ServerInitConfigHeaderTypeMustBeStandard(void)
{
    hzl_Err_t err;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerConfig_t modifiedServerConfig = HZL_TEST_CORRECT_SERVER_CONFIG;
    hzl_ServerCtx_t ctx = {
            .serverConfig = &modifiedServerConfig,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    modifiedServerConfig.headerType = 99;

    err = hzl_ServerInit(&ctx);

    atto_eq(err, HZL_ERR_INVALID_HEADER_TYPE);
}

static void
hzlServerTest_ServerInitConfigServerAmountOfGroupsMustFitForHeaderType(void)
{
    hzl_Err_t err;
    hzl_ServerGroupState_t groupStates[HZL_MAX_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerConfig_t modifiedServerConfig = HZL_TEST_CORRECT_SERVER_CONFIG;
    hzl_ServerCtx_t ctx = {
            .serverConfig = &modifiedServerConfig,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    modifiedServerConfig.headerType = HZL_HEADER_3;  // max 8 groups, max 3 clients
    modifiedServerConfig.amountOfClients = 3; // To avoid HZL_ERR_TOO_MANY_CLIENTS_FOR_CONFIGURED_HEADER_TYPE
    modifiedServerConfig.amountOfGroups = 9;  // 1 more than max
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_ERR_TOO_MANY_GROUPS_FOR_CONFIGURED_HEADER_TYPE);

    modifiedServerConfig.amountOfGroups = 8;  // == max
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);

    modifiedServerConfig.headerType = HZL_HEADER_5;  // max 1 group
    modifiedServerConfig.amountOfGroups = 2;  // 1 more than max
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_ERR_TOO_MANY_GROUPS_FOR_CONFIGURED_HEADER_TYPE);

    modifiedServerConfig.amountOfGroups = 1;  // == max
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
}

static void
hzlServerTest_ServerInitConfigServerAmountOfClientsMustFitForHeaderType(void)
{
    hzl_Err_t err;
    hzl_ServerGroupState_t groupStates[HZL_MAX_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerConfig_t modifiedServerConfig = HZL_TEST_CORRECT_SERVER_CONFIG;
    hzl_ServerCtx_t ctx = {
            .serverConfig = &modifiedServerConfig,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    modifiedServerConfig.headerType = HZL_HEADER_3;  // max 4 SIDs = 3 clients
    modifiedServerConfig.amountOfClients = 4;  // 1 more than max
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_ERR_TOO_MANY_CLIENTS_FOR_CONFIGURED_HEADER_TYPE);
    modifiedServerConfig.amountOfClients = 3;  // == max
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
}

static void
hzlServerTest_ServerInitConfigServerAmountOfClientsMustFitInBitmap(void)
{
    hzl_Err_t err;
    hzl_ServerGroupState_t groupStates[HZL_MAX_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerConfig_t modifiedServerConfig = HZL_TEST_CORRECT_SERVER_CONFIG;
    hzl_ServerClientConfig_t modifiedClientConfigs[HZL_SERVER_MAX_AMOUNT_OF_CLIENTS];
    for (size_t i = 0; i < HZL_SERVER_MAX_AMOUNT_OF_CLIENTS; i++)
    {
        // Copy same config over many times. It's not about the correct clients config,
        // just about the amount of clients. We don't want to read invalid memory.
        modifiedClientConfigs[i] = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS[0];
    }
    hzl_ServerCtx_t ctx = {
            .serverConfig = &modifiedServerConfig,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    modifiedServerConfig.amountOfClients = HZL_SERVER_MAX_AMOUNT_OF_CLIENTS + 1;
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_ERR_TOO_MANY_CLIENTS);

    modifiedServerConfig.amountOfClients = HZL_SERVER_MAX_AMOUNT_OF_CLIENTS;
    err = hzl_ServerInit(&ctx);
    atto_neq(err, HZL_ERR_TOO_MANY_CLIENTS); // Some of the client configs are wrong
    // Avoid "unused variable" error: we don't need its value, but need the valid memory location
    (void) modifiedClientConfigs;
}

static void
hzlServerTest_ServerInitConfigServerMustHaveOnlyBroadcastWhenHeader5Or6(void)
{
    // Headers 5 or 6 don't have a field for the GID, as this is assumed to
    // be the only group. Thus there must be only 1 group configuration and it
    // must be the one for the GID 0.
    hzl_Err_t err;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerConfig_t modifiedServerConfig = HZL_TEST_CORRECT_SERVER_CONFIG;
    modifiedServerConfig.headerType = HZL_HEADER_6;  // No GID field
    hzl_ServerCtx_t ctx = {
            .serverConfig = &modifiedServerConfig,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    modifiedServerConfig.amountOfGroups = 2;  // 1 more than max
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_ERR_TOO_MANY_GROUPS_FOR_CONFIGURED_HEADER_TYPE);

    modifiedServerConfig.amountOfGroups = 1;  // == max
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
}

void hzlServerTest_ServerInitCheckServerConfig(void)
{
    hzlServerTest_ServerInitConfigMustBeNotNull();
    hzlServerTest_ServerInitConfigAmountOfGroupsMustBePositive();
    hzlServerTest_ServerInitConfigAmountOfClientsMustBePositive();
    hzlServerTest_ServerInitConfigHeaderTypeMustBeStandard();
    hzlServerTest_ServerInitConfigServerAmountOfGroupsMustFitForHeaderType();
    hzlServerTest_ServerInitConfigServerAmountOfClientsMustFitForHeaderType();
    hzlServerTest_ServerInitConfigServerAmountOfClientsMustFitInBitmap();
    hzlServerTest_ServerInitConfigServerMustHaveOnlyBroadcastWhenHeader5Or6();
    HZL_TEST_PARTIAL_REPORT();
}
