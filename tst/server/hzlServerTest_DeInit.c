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
 * Tests of the hzl_ServerDeinit() function.
 */

#include "hzlTest.h"

static void
hzlServerTest_ServerDeInitCtxMustNotBeNull(void)
{
    hzl_Err_t err;

    err = hzl_ServerDeInit(NULL);

    atto_eq(err, HZL_ERR_NULL_CTX);
}

static void
hzlServerTest_ServerDeInitServerConfigMustNotBeNull(void)
{
    hzl_Err_t err;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = NULL,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    err = hzl_ServerDeInit(&ctx);

    atto_eq(err, HZL_ERR_NULL_CONFIG_SERVER);
}

static void
hzlServerTest_ServerDeInitGroupStatesMustNotBeNull(void)
{
    hzl_Err_t err;
    hzl_ServerCtx_t ctx = {
            .serverConfig = &HZL_TEST_CORRECT_SERVER_CONFIG,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = NULL,
            .io = HZL_TEST_CORRECT_IO,
    };

    err = hzl_ServerDeInit(&ctx);

    atto_eq(err, HZL_ERR_NULL_STATES_GROUPS);
}

static void
hzlServerTest_ServerDeInitClearsGroupStatesOnly(void)
{
    hzl_Err_t err;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &HZL_TEST_CORRECT_SERVER_CONFIG,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);

    err = hzl_ServerDeInit(&ctx);

    atto_eq(err, HZL_OK);
    atto_zeros(groupStates,
               HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS *
               sizeof(hzl_ServerGroupState_t));
    // The rest of the ctx is untouched
    atto_eq(ctx.serverConfig, &HZL_TEST_CORRECT_SERVER_CONFIG);
    atto_eq(ctx.clientConfigs, HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS);
    atto_eq(ctx.groupConfigs, HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS);
    atto_eq(ctx.groupStates, groupStates);
    atto_eq(ctx.io.currentTime, HZL_TEST_CORRECT_IO.currentTime);
    atto_eq(ctx.io.trng, HZL_TEST_CORRECT_IO.trng);
}

void hzlServerTest_ServerDeinit(void)
{
    hzlServerTest_ServerDeInitCtxMustNotBeNull();
    hzlServerTest_ServerDeInitServerConfigMustNotBeNull();
    hzlServerTest_ServerDeInitGroupStatesMustNotBeNull();
    hzlServerTest_ServerDeInitClearsGroupStatesOnly();
    HZL_TEST_PARTIAL_REPORT();
}
