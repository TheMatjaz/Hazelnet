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
 * Tests of the hzl_ServerInit() function, general part.
 */

#include "hzlTest.h"

static void
hzlServerTest_ServerInitCtxMustBeNotNull(void)
{
    hzl_Err_t err;

    err = hzl_ServerInit(NULL);

    atto_eq(err, HZL_ERR_NULL_CTX);
}

static void
hzlServerTest_ServerInitGroupStatesMustBeNotNull(void)
{
    hzl_Err_t err;
    hzl_ServerCtx_t ctx = {
            .serverConfig = &HZL_TEST_CORRECT_SERVER_CONFIG,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = NULL,
            .io = HZL_TEST_CORRECT_IO,
    };

    err = hzl_ServerInit(&ctx);

    atto_eq(err, HZL_ERR_NULL_STATES_GROUPS);
}

static void
hzlServerTest_ServerInitFailingIoCurrentTime(void)
{
    hzl_Err_t err;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &HZL_TEST_CORRECT_SERVER_CONFIG,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = {
                    .currentTime = hzlTest_IoMockupCurrentTimeFailing,
                    .trng = hzlTest_IoMockupTrngSucceeding,
            },
    };

    err = hzl_ServerInit(&ctx);

    atto_eq(err, HZL_ERR_CANNOT_GET_CURRENT_TIME);
}

static void
hzlServerTest_ServerInitFailingIoTrng(void)
{
    hzl_Err_t err;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &HZL_TEST_CORRECT_SERVER_CONFIG,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = {
                    .currentTime = hzlTest_IoMockupCurrentTimeSucceeding,
                    .trng = NULL,
            },
    };

    ctx.io.trng = hzlTest_IoMockupTrngFailing;
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_ERR_CANNOT_GENERATE_RANDOM);

    ctx.io.trng = hzlTest_IoMockupTrngFailingJustZeros;
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_ERR_CANNOT_GENERATE_NON_ZERO_RANDOM);
}

static void
hzlServerTest_ServerInitCorrectCtxSucceedsInitsAllSessions(void)
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
    memset(groupStates, 0xFF, sizeof(groupStates)); // Set to garbage to see the difference
    hzl_Timestamp_t timestampBefore;
    hzl_Timestamp_t timestampAfter;
    hzlTest_IoMockupCurrentTimeSucceeding(&timestampBefore);

    err = hzl_ServerInit(&ctx);

    hzlTest_IoMockupCurrentTimeSucceeding(&timestampAfter);
    atto_eq(err, HZL_OK);
    const uint8_t expectedRandomStk[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    for (size_t i = 0; i < ctx.serverConfig->amountOfGroups; i++)
    {
        // The session timestamp was initialised during the call
        atto_gt(ctx.groupStates[i].sessionStartInstant, timestampBefore);
        atto_lt(ctx.groupStates[i].sessionStartInstant, timestampAfter);
        // The STK contains "fake random" values from the mockup TRNG function
        atto_memeq(ctx.groupStates[i].currentStk, expectedRandomStk, HZL_STK_LEN);
        atto_eq(ctx.groupStates[i].currentCtrNonce, 0);
        // There was no message received yet
        atto_eq(ctx.groupStates[i].currentRxLastMessageInstant,
                ctx.groupStates[i].sessionStartInstant);
        // Previous session is cleared
        atto_eq(ctx.groupStates[i].previousCtrNonce, 0);
        atto_zeros(ctx.groupStates[i].previousStk, HZL_STK_LEN);
    }
}

void hzlServerTest_ServerInit(void)
{
    hzlServerTest_ServerInitCtxMustBeNotNull();
    hzlServerTest_ServerInitGroupStatesMustBeNotNull();
    hzlServerTest_ServerInitFailingIoCurrentTime();
    hzlServerTest_ServerInitFailingIoTrng();
    hzlServerTest_ServerInitCorrectCtxSucceedsInitsAllSessions();
    HZL_TEST_PARTIAL_REPORT();
}
