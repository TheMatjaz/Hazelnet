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
 * caused by the Clients configs being incorrect.
 */

#include "hzlTest.h"

static void
hzlServerTest_ServerInitClientConfigsMustBeNotNull(void)
{
    hzl_Err_t err;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &HZL_TEST_CORRECT_SERVER_CONFIG,
            .clientConfigs = NULL,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    err = hzl_ServerInit(&ctx);

    atto_eq(err, HZL_ERR_NULL_CONFIG_CLIENTS);
}

static void
hzlServerTest_ServerInitClientConfigsMustHaveNonZeroLtks(void)
{
    hzl_Err_t err;
    hzl_ServerClientConfig_t modifiedClientConfigs[HZL_DEFAULT_TEST_AMOUNT_OF_CLIENTS];
    memcpy(modifiedClientConfigs,
           HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
           sizeof(hzl_ServerClientConfig_t) * HZL_DEFAULT_TEST_AMOUNT_OF_CLIENTS);
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &HZL_TEST_CORRECT_SERVER_CONFIG,
            .clientConfigs = modifiedClientConfigs,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    memset(modifiedClientConfigs[1].ltk, 0, HZL_LTK_LEN);

    err = hzl_ServerInit(&ctx);

    atto_eq(err, HZL_ERR_LTK_IS_ALL_ZEROS);
}

static void
hzlServerTest_ServerInitClientConfigsMustHaveNonZeroSid(void)
{
    hzl_Err_t err;
    hzl_ServerClientConfig_t modifiedClientConfigs[HZL_DEFAULT_TEST_AMOUNT_OF_CLIENTS];
    memcpy(modifiedClientConfigs,
           HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
           sizeof(hzl_ServerClientConfig_t) * HZL_DEFAULT_TEST_AMOUNT_OF_CLIENTS);
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &HZL_TEST_CORRECT_SERVER_CONFIG,
            .clientConfigs = modifiedClientConfigs,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    modifiedClientConfigs[1].sid = 0;

    err = hzl_ServerInit(&ctx);

    atto_eq(err, HZL_ERR_SERVER_SID_ASSIGNED_TO_CLIENT);
}

static void
hzlServerTest_ServerInitClientConfigsMustHaveSortedSidsStrictlyAscending(void)
{
    hzl_Err_t err;
    hzl_ServerClientConfig_t modifiedClientConfigs[HZL_DEFAULT_TEST_AMOUNT_OF_CLIENTS];
    memcpy(modifiedClientConfigs,
           HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
           sizeof(hzl_ServerClientConfig_t) * HZL_DEFAULT_TEST_AMOUNT_OF_CLIENTS);
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &HZL_TEST_CORRECT_SERVER_CONFIG,
            .clientConfigs = modifiedClientConfigs,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    modifiedClientConfigs[1].sid = 1; // Repeated SID
    atto_eq(modifiedClientConfigs[0].sid, modifiedClientConfigs[1].sid);
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_ERR_SIDS_ARE_NOT_PRESORTED_STRICTLY_ASCENDING);

    modifiedClientConfigs[0].sid = 2; // Strictly descending
    atto_gt(modifiedClientConfigs[0].sid, modifiedClientConfigs[1].sid);
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_ERR_SIDS_ARE_NOT_PRESORTED_STRICTLY_ASCENDING);
}

static void
hzlServerTest_ServerInitClientConfigsMustHaveSidsWithoutGaps(void)
{
    hzl_Err_t err;
    hzl_ServerConfig_t modifiedServerConfig = HZL_TEST_CORRECT_SERVER_CONFIG;
    hzl_ServerClientConfig_t modifiedClientConfigs[HZL_DEFAULT_TEST_AMOUNT_OF_CLIENTS];
    memcpy(modifiedClientConfigs,
           HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
           sizeof(hzl_ServerClientConfig_t) * HZL_DEFAULT_TEST_AMOUNT_OF_CLIENTS);
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &modifiedServerConfig,
            .clientConfigs = modifiedClientConfigs,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    modifiedServerConfig.amountOfClients = 3; // To allow larger SID
    modifiedClientConfigs[1].sid = 3; // Gap between first and second SID
    atto_lt(modifiedClientConfigs[0].sid + 1, modifiedClientConfigs[1].sid);

    err = hzl_ServerInit(&ctx);

    atto_eq(err, HZL_ERR_GAP_IN_SIDS);
}

void hzlServerTest_ServerInitCheckClientConfigs(void)
{
    hzlServerTest_ServerInitClientConfigsMustBeNotNull();
    hzlServerTest_ServerInitClientConfigsMustHaveNonZeroLtks();
    hzlServerTest_ServerInitClientConfigsMustHaveNonZeroSid();
    hzlServerTest_ServerInitClientConfigsMustHaveSortedSidsStrictlyAscending();
    hzlServerTest_ServerInitClientConfigsMustHaveSidsWithoutGaps();
    HZL_TEST_PARTIAL_REPORT();
}
