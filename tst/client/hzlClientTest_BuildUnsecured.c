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
 * Tests of the hzl_ClientBuildUnsecured() function.
 *
 * @warning
 * REDUCING COVERAGE ON PURPOSE. NOT implementing all the testcases for all possible incorrect
 * content of the context, because they have already been checked for the hzl_ClientInit()
 * function and the inner checks are exactly the same, performed by the same internal
 * function hzl_ClientCheckCtxPointers().
 */

#include "hzlTest.h"

static void
hzlClientTest_ClientBuildUnsecuredMsgToTxMustBeNotNull(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientCtx_t ctx = {
            .clientConfig = &HZL_TEST_CORRECT_CLIENT_CONFIG,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };

    err = hzl_ClientBuildUnsecured(NULL, &ctx, (void*) 1, 0, 0);

    atto_eq(err, HZL_ERR_NULL_PDU);
}

static void
hzlClientTest_ClientBuildUnsecuredCtxMustBeNotNull(void)
{
    hzl_Err_t err;
    hzl_CbsPduMsg_t msgToTx = {0};

    err = hzl_ClientBuildUnsecured(&msgToTx, NULL, NULL, 0, 0);

    atto_eq(err, HZL_ERR_NULL_CTX);
}

// NOTE: tests for all possible issues WITHIN the context are skipped. See warning in file header.

static void
hzlClientTest_ClientBuildUnsecuredUserDataMustBeNotNullWhenPositiveDataLen(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientCtx_t ctx = {
            .clientConfig = &HZL_TEST_CORRECT_CLIENT_CONFIG,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
    hzl_CbsPduMsg_t msgToTx = {0};

    err = hzl_ClientBuildUnsecured(&msgToTx, &ctx, NULL, 1, 0);
    atto_eq(err, HZL_ERR_NULL_SDU);

    err = hzl_ClientBuildUnsecured(&msgToTx, &ctx, NULL, 0, 0);
    atto_eq(err, HZL_OK);
}

static void
hzlClientTest_ClientBuildUnsecuredDataLenMustBeShortEnough(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientCtx_t ctx = {
            .clientConfig = &HZL_TEST_CORRECT_CLIENT_CONFIG,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen;
    // Requirement for this test: the header 0 is 3 bytes long, thus at most 61 bytes of dataLen.
    atto_eq(ctx.clientConfig->headerType, HZL_HEADER_0);

    userDataLen = 61;  // Still fits
    err = hzl_ClientBuildUnsecured(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_OK);

    userDataLen = 62;  // Too much
    err = hzl_ClientBuildUnsecured(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_ERR_TOO_LONG_SDU);
}

static void
hzlClientTest_ClientBuildUnsecuredDataLenDependsOnHeaderLen(void)
{
    hzl_Err_t err;
    hzl_ClientConfig_t clientConfigWithNewHeaderType =
            HZL_TEST_CORRECT_CLIENT_CONFIG;
    clientConfigWithNewHeaderType.headerType = HZL_HEADER_6;
    clientConfigWithNewHeaderType.sid = 2;
    clientConfigWithNewHeaderType.amountOfGroups = 1;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientCtx_t ctx = {
            .clientConfig = &clientConfigWithNewHeaderType,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen;
    // Requirement for this test: the header 6 is 1 bytes long, thus at most 63 bytes of dataLen.
    atto_eq(ctx.clientConfig->headerType, HZL_HEADER_6);

    userDataLen = 63;  // Still fits
    err = hzl_ClientBuildUnsecured(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_OK);

    userDataLen = 64;  // Too much
    err = hzl_ClientBuildUnsecured(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_ERR_TOO_LONG_SDU);
}

static void
hzlClientTest_ClientBuildUnsecuredCompactHeaderPreventsTooManyGroups(void)
{
    hzl_Err_t err;
    hzl_ClientConfig_t clientConfigWithNewHeaderType =
            HZL_TEST_CORRECT_CLIENT_CONFIG;
    clientConfigWithNewHeaderType.headerType = HZL_HEADER_4;
    clientConfigWithNewHeaderType.sid = 2;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientCtx_t ctx = {
            .clientConfig = &clientConfigWithNewHeaderType,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen = 4;
    // Requirement for this test: the header 4 is 1 bytes long with only 4 groups.
    atto_eq(ctx.clientConfig->headerType, HZL_HEADER_4);

    err = hzl_ClientBuildUnsecured(&msgToTx, &ctx, userData, userDataLen, 33);

    atto_eq(err, HZL_ERR_GID_TOO_LARGE_FOR_CONFIGURED_HEADER_TYPE);
}

static void
hzlClientTest_ClientBuildUnsecuredGidsNotInConfigAreAccepted(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientCtx_t ctx = {
            .clientConfig = &HZL_TEST_CORRECT_CLIENT_CONFIG,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen = 4;

    err = hzl_ClientBuildUnsecured(&msgToTx, &ctx, userData, userDataLen,
                                   231);  // Unexisting GID

    atto_eq(err, HZL_OK);
}

static void
hzlClientTest_ClientBuildUnsecuredHeaderIsPackedBeforePayload(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientCtx_t ctx = {
            .clientConfig = &HZL_TEST_CORRECT_CLIENT_CONFIG,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen;
    // Requirement for this test: the header 0 is 3 bytes long
    atto_eq(ctx.clientConfig->headerType, HZL_HEADER_0);

    userDataLen = 0;
    err = hzl_ClientBuildUnsecured(&msgToTx, &ctx, userData, userDataLen,
                                   42);
    atto_eq(err, HZL_OK);
    // Message metadata
    atto_eq(msgToTx.dataLen, 3 + 0);  // Header 0 + dataLen
    // Packed Header 0
    atto_eq(msgToTx.data[0], 42);  // GID from API call
    atto_eq(msgToTx.data[1], 13);  // SID from client config
    atto_eq(msgToTx.data[2], 5);  // PTY UAD

    userDataLen = 4;
    err = hzl_ClientBuildUnsecured(&msgToTx, &ctx, userData, userDataLen, 42);
    atto_eq(err, HZL_OK);
    // Message metadata
    atto_eq(msgToTx.dataLen, 3 + 4);  // Header 0 + dataLen
    // Packed Header 0
    atto_eq(msgToTx.data[0], 42);  // GID from API call
    atto_eq(msgToTx.data[1], 13);  // SID from client config
    atto_eq(msgToTx.data[2], 5);  // PTY UAD
    // CBS UAD-Payload
    atto_eq(msgToTx.data[3], 1);
    atto_eq(msgToTx.data[4], 2);
    atto_eq(msgToTx.data[5], 3);
    atto_eq(msgToTx.data[6], 4);
}

static void
hzlClientTest_ClientBuildUnsecuredHeaderPackingDependsOnType(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientConfig_t clientConfigWithNewHeaderType =
            HZL_TEST_CORRECT_CLIENT_CONFIG;
    clientConfigWithNewHeaderType.headerType = HZL_HEADER_4;
    clientConfigWithNewHeaderType.sid = 3;
    hzl_ClientCtx_t ctx = {
            .clientConfig = &clientConfigWithNewHeaderType,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen = 4;
    // Requirement for this test: the header 4 is 1 bytes long
    atto_eq(ctx.clientConfig->headerType, HZL_HEADER_4);

    err = hzl_ClientBuildUnsecured(&msgToTx, &ctx, userData, userDataLen, 2);

    atto_eq(err, HZL_OK);
    // Message metadata
    atto_eq(msgToTx.dataLen, 1 + 4);  // Header 4 + dataLen
    // Packed Header 4. Bits: |sssg gppp| (s=SID, g=GID, p=PTY)
    // SID from client config, GID from API call, PTY for UAD.
    const size_t expectedPackedHdr = 3U << 5U | 2U << 3U | 5U;
    atto_eq(msgToTx.data[0], expectedPackedHdr);
    // CBS UAD-Payload
    atto_eq(msgToTx.data[1], 1);
    atto_eq(msgToTx.data[2], 2);
    atto_eq(msgToTx.data[3], 3);
    atto_eq(msgToTx.data[4], 4);
}

void hzlClientTest_ClientBuildUnsecured(void)
{
    hzlClientTest_ClientBuildUnsecuredMsgToTxMustBeNotNull();
    hzlClientTest_ClientBuildUnsecuredCtxMustBeNotNull();
    hzlClientTest_ClientBuildUnsecuredUserDataMustBeNotNullWhenPositiveDataLen();
    hzlClientTest_ClientBuildUnsecuredDataLenMustBeShortEnough();
    hzlClientTest_ClientBuildUnsecuredDataLenDependsOnHeaderLen();
    hzlClientTest_ClientBuildUnsecuredCompactHeaderPreventsTooManyGroups();
    hzlClientTest_ClientBuildUnsecuredGidsNotInConfigAreAccepted();
    hzlClientTest_ClientBuildUnsecuredHeaderIsPackedBeforePayload();
    hzlClientTest_ClientBuildUnsecuredHeaderPackingDependsOnType();
    HZL_TEST_PARTIAL_REPORT();
}
