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
 * Tests of the hzl_ClientInit() function, general part.
 */

#include "hzlTest.h"

static void
hzlClientTest_ClientInitCtxMustBeNotNull(void)
{
    hzl_Err_t err;

    err = hzl_ClientInit(NULL);

    atto_eq(err, HZL_ERR_NULL_CTX);
}

static void
hzlClientTest_ClientInitGroupStatesMustBeNotNull(void)
{
    hzl_Err_t err;
    hzl_ClientCtx_t ctx = {
            .clientConfig = &HZL_TEST_CORRECT_CLIENT_CONFIG,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = NULL,
            .io = HZL_TEST_CORRECT_IO,
    };

    err = hzl_ClientInit(&ctx);

    atto_eq(err, HZL_ERR_NULL_STATES_GROUPS);
}

static void
hzlClientTest_ClientInitCorrectCtxSucceeds(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    // Set first and last byte of the states array to non-zero to see if it changes
    groupStates[0].requestNonce = 0x0102030405060708ULL;
    groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS - 1].previousStk[
            HZL_LTK_LEN - 1] = 3;
    hzl_ClientCtx_t ctx = {
            .clientConfig = &HZL_TEST_CORRECT_CLIENT_CONFIG,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    err = hzl_ClientInit(&ctx);

    atto_eq(err, HZL_OK);
    atto_zeros(ctx.groupStates,
               ctx.clientConfig->amountOfGroups * sizeof(hzl_ClientGroupState_t));
}

void hzlClientTest_ClientInit(void)
{
    hzlClientTest_ClientInitCtxMustBeNotNull();
    hzlClientTest_ClientInitGroupStatesMustBeNotNull();
    hzlClientTest_ClientInitCorrectCtxSucceeds();
    HZL_TEST_PARTIAL_REPORT();
}
