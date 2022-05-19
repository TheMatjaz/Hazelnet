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
 * Tests of the hzl_ServerBuildSecuredFd() function.
 *
 * @warning
 * REDUCING COVERAGE ON PURPOSE. NOT implementing all the testcases for all possible incorrect
 * content of the context, because they have already been checked for the hzl_ServerInit()
 * function and the inner checks are exactly the same, performed by the same internal
 * function hzl_ServerCheckCtxPointers().
 */

#include "hzlTest.h"

static void
hzlServerTest_ServerBuildSecuredFdMsgToTxMustBeNotNull(void)
{
    hzl_Err_t err;

    err = hzl_ServerBuildSecuredFd(NULL, NULL, (void*) 1, 0, 0);

    atto_eq(err, HZL_ERR_NULL_PDU);
}

static void
hzlServerTest_ServerBuildSecuredFdCtxMustBeNotNull(void)
{
    hzl_Err_t err;
    hzl_CbsPduMsg_t msgToTx = {0};

    err = hzl_ServerBuildSecuredFd(&msgToTx, NULL, NULL, 0, 0);

    atto_eq(err, HZL_ERR_NULL_CTX);
}

// NOTE: tests for all possible issues WITHIN the context are skipped. See warning in file header.

static void
hzlServerTest_ServerBuildSecuredFdMsgDataMustBeNotNullWhenPositiveDataLen(void)
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
    hzl_CbsPduMsg_t msgToTx = {0};

    err = hzl_ServerBuildSecuredFd(&msgToTx, &ctx, NULL, 1, 0);
    atto_eq(err, HZL_ERR_NULL_SDU);

    err = hzl_ServerBuildSecuredFd(&msgToTx, &ctx, NULL, 0, 0);
    atto_neq(err, HZL_ERR_NULL_SDU);  // Different error
}

static void
hzlServerTest_ServerBuildSecuredFdDataLenMustBeShortEnough(void)
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
    // Fake a Request being already received
    groupStates[0].currentRxLastMessageInstant = groupStates[0].sessionStartInstant + 1U;
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen;
    // Requirement for this test: the header 0 is 3 bytes long, thus at most 49 bytes of dataLen.
    atto_eq(ctx.serverConfig->headerType, HZL_HEADER_0);

    userDataLen = 49;  // Still fits
    err = hzl_ServerBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_OK);

    userDataLen = 50;  // Too much
    err = hzl_ServerBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_ERR_TOO_LONG_SDU);
}

static void
hzlServerTest_ServerBuildSecuredFdDataLenDependsOnHeaderLen(void)
{
    hzl_Err_t err;
    hzl_ServerConfig_t serverConfigWithNewHeaderType =
            HZL_TEST_CORRECT_SERVER_CONFIG;
    serverConfigWithNewHeaderType.headerType = HZL_HEADER_6;
    serverConfigWithNewHeaderType.amountOfGroups = 1;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &serverConfigWithNewHeaderType,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
    // Fake a Request being already received
    groupStates[0].currentRxLastMessageInstant = groupStates[0].sessionStartInstant + 1U;
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen;
    // Requirement for this test: the header 6 is 1 bytes long, thus at most 51 bytes of dataLen.
    atto_eq(ctx.serverConfig->headerType, HZL_HEADER_6);

    userDataLen = 51;  // Still fits
    err = hzl_ServerBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_OK);

    userDataLen = 52;  // Too much
    err = hzl_ServerBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_ERR_TOO_LONG_SDU);
}

static void
hzlServerTest_ServerBuildSecuredFdCompactHeaderPreventsTooManyGroups(void)
{
    hzl_Err_t err;
    hzl_ServerConfig_t serverConfigWithNewHeaderType =
            HZL_TEST_CORRECT_SERVER_CONFIG;
    serverConfigWithNewHeaderType.headerType = HZL_HEADER_4;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &serverConfigWithNewHeaderType,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
    // Fake a Request being already received
    groupStates[0].currentRxLastMessageInstant = groupStates[0].sessionStartInstant + 1U;
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen = 4;
    // Requirement for this test: the header 4 is 1 bytes long with only 4 groups.
    atto_eq(ctx.serverConfig->headerType, HZL_HEADER_4);

    err = hzl_ServerBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 33);

    atto_eq(err, HZL_ERR_GID_TOO_LARGE_FOR_CONFIGURED_HEADER_TYPE);
}

static void
hzlServerTest_ServerBuildSecuredFdGidMustBeInConfig(void)
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
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen = 4;

    err = hzl_ServerBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen,
                                   231);  // Unexisting GID

    atto_eq(err, HZL_ERR_UNKNOWN_GROUP);
}

static void
hzlServerTest_ServerBuildSecuredFdRequiresSomeClientsRequestedAlready(void)
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
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen = 4;

    err = hzl_ServerBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_ERR_NO_POTENTIAL_RECEIVER);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t));

    // Fake a Request being received
    groupStates[0].currentRxLastMessageInstant = groupStates[0].sessionStartInstant + 1U;
    err = hzl_ServerBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_OK);
}

static void
hzlServerTest_ServerBuildSecuredFdHeaderIsPackedBeforePayload(void)
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
    // Fake a Request being already received
    groupStates[0].currentRxLastMessageInstant = groupStates[0].sessionStartInstant + 1U;
    groupStates[0].currentStk[0] = 99;
    memset(&groupStates[0].currentStk[1], 0, 15);
    groupStates[0].currentCtrNonce = 0x112233U;
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen = 4;

    // Requirement for this test: the header 0 is 3 bytes long
    atto_eq(ctx.serverConfig->headerType, HZL_HEADER_0);
    err = hzl_ServerBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 0);

    atto_eq(err, HZL_OK);
    // Header 0 + ctrnonce + ptlen + dataLen + tag
    atto_eq(msgToTx.dataLen, 3 + 3 + 1 + 4 + 8);
    // Packed Header 0
    atto_eq(msgToTx.data[0], 0);  // GID from API call
    atto_eq(msgToTx.data[1], 0);  // SID from server
    atto_eq(msgToTx.data[2], 4);  // PTY SADFD
    // CBS SADFD-Payload
    // Ctrnonce is increased by 1 compared to the state
    atto_eq(msgToTx.data[3], 0x33);  // Ctrnonce low
    atto_eq(msgToTx.data[4], 0x22);  // Ctrnonce mid
    atto_eq(msgToTx.data[5], 0x11);  // Ctrnonce high
    atto_eq(msgToTx.data[6], userDataLen);  // Ptlen
    atto_memneq(&msgToTx.data[7], userData, userDataLen);  // CT
    // Tag: not verified here, not relevant to this test.
    // Ctrnonce was incremented in the state
    atto_eq(groupStates[0].currentCtrNonce, 0x112234);
}

static void
hzlServerTest_ServerBuildSecuredFdHeaderPackingDependsOnType(void)
{
    hzl_Err_t err;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerConfig_t serverConfigWithNewHeaderType =
            HZL_TEST_CORRECT_SERVER_CONFIG;
    serverConfigWithNewHeaderType.headerType = HZL_HEADER_4;
    hzl_ServerCtx_t ctx = {
            .serverConfig = &serverConfigWithNewHeaderType,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
    // Fake a Request being already received
    groupStates[2].currentRxLastMessageInstant = groupStates[0].sessionStartInstant + 1U;
    groupStates[2].currentCtrNonce = 0x112233U;
    groupStates[2].currentStk[0] = 99;
    memset(&groupStates[2].currentStk[1], 0, 15);
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen = 4;

    // Requirement for this test: the header 4 is 1 bytes long
    atto_eq(ctx.serverConfig->headerType, HZL_HEADER_4);
    err = hzl_ServerBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 2);

    atto_eq(err, HZL_OK);
    // Header 4 + ctrnonce + ptlen + dataLen + tag
    atto_eq(msgToTx.dataLen, 1 + 3 + 1 + 4 + 8);
    // Packed Header 4. Bits: |sssg gppp| (s=SID, g=GID, p=PTY)
    // SID from server, GID from API call, PTY for SADFD.
    const size_t expectedPackedHdr = 0 | 2U << 3U | 4U;
    atto_eq(msgToTx.data[0], expectedPackedHdr);
    // CBS SADFD-Payload
    // Ctrnonce is increased by 1 compared to the state
    atto_eq(msgToTx.data[1], 0x33);  // Ctrnonce low
    atto_eq(msgToTx.data[2], 0x22);  // Ctrnonce mid
    atto_eq(msgToTx.data[3], 0x11);  // Ctrnonce high
    atto_eq(msgToTx.data[4], userDataLen);  // Ptlen
    atto_memneq(&msgToTx.data[5], userData, userDataLen);  // CT
    // Tag: not verified here, not relevant to this test.
    // Ctrnonce was incremented in the state
    atto_eq(groupStates[2].currentCtrNonce, 0x112234);
}

static void
hzlServerTest_ServerBuildSecuredFdMsgWithNoPayload(void)
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
    // Fake a Request being already received
    groupStates[0].currentRxLastMessageInstant = groupStates[0].sessionStartInstant + 1U;
    groupStates[0].currentStk[0] = 99;
    memset(&groupStates[0].currentStk[1], 0, 15);
    groupStates[0].currentCtrNonce = 0x010203U;
    groupStates[0].currentStk[0] = 99;
    memset(&groupStates[0].currentStk[1], 0, 15);
    hzl_CbsPduMsg_t msgToTx = {0};
    size_t userDataLen = 0;

    err = hzl_ServerBuildSecuredFd(&msgToTx, &ctx, NULL, userDataLen, 0);

    atto_eq(err, HZL_OK);
    // Header 0 + ctrnonce + ptlen + dataLen + tag
    atto_eq(msgToTx.dataLen, 3 + 3 + 1 + 0 + 8);
    // Packed Header 0
    atto_eq(msgToTx.data[0], 0);  // GID from API call
    atto_eq(msgToTx.data[1], 0);  // SID from server
    atto_eq(msgToTx.data[2], 4);  // PTY SADFD
    // CBS SADFD-Payload
    // Ctrnonce is increased by 1 compared to the state
    atto_eq(msgToTx.data[3], 0x03);  // Ctrnonce low
    atto_eq(msgToTx.data[4], 0x02);  // Ctrnonce mid
    atto_eq(msgToTx.data[5], 0x01);  // Ctrnonce high
    atto_eq(msgToTx.data[6], 0);  // Ptlen
    // Ciphertext: empty
    // Tag
    const uint8_t expectedTag[8] = {0xEB, 0xE5, 0x7B, 0x17, 0x89, 0xBC, 0xCA, 0xCD};
    atto_memeq(&msgToTx.data[7], expectedTag, 8);
    // Ctrnonce was incremented in the state
    atto_eq(groupStates[0].currentCtrNonce, 0x010204);
}

static void
hzlServerTest_ServerBuildSecuredFdSuccessfully(void)
{
    hzl_Err_t err;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerConfig_t serverConfigWithNewHeaderType =
            HZL_TEST_CORRECT_SERVER_CONFIG;
    hzl_ServerCtx_t ctx = {
            .serverConfig = &serverConfigWithNewHeaderType,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
    // Fake a Request being already received
    groupStates[0].currentRxLastMessageInstant = groupStates[0].sessionStartInstant + 1U;
    groupStates[0].currentCtrNonce = 0x010203U;
    groupStates[0].currentStk[0] = 99;
    memset(&groupStates[0].currentStk[1], 0, 15);
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {'A', 'B', 'C', 'D', 'E'};
    size_t userDataLen = 5;

    err = hzl_ServerBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_OK);
    // Header 0 + ctrnonce + ptlen + dataLen + tag
    atto_eq(msgToTx.dataLen, 3 + 3 + 1 + 5 + 8);
    // Packed Header 0
    atto_eq(msgToTx.data[0], 0);  // GID from API call
    atto_eq(msgToTx.data[1], 0);  // SID from server
    atto_eq(msgToTx.data[2], 4);  // PTY SADFD
    // CBS SADFD-Payload
    atto_eq(msgToTx.data[3], 0x03);  // Ctrnonce low
    atto_eq(msgToTx.data[4], 0x02);  // Ctrnonce mid
    atto_eq(msgToTx.data[5], 0x01);  // Ctrnonce high
    atto_eq(msgToTx.data[6], 5);  // Ptlen
    // Ciphertext
    const uint8_t expectedCtext[5] = {0xAF, 0xE4, 0x31, 0xE5, 0xBD};
    atto_memeq(&msgToTx.data[7], expectedCtext, 5);
    // Tag
    const uint8_t expectedTag[8] = {0x97, 0x96, 0xA0, 0x03, 0x46, 0x82, 0xE8, 0xF4};
    atto_memeq(&msgToTx.data[12], expectedTag, 8);
    // Ctrnonce was incremented in the state
    atto_eq(groupStates[0].currentCtrNonce, 0x010204);
}


static void
hzlServerTest_ServerBuildSecuredFdSuccessfullyUsesNewKeyDuringRenewalPhase(void)
{
    hzl_Err_t err;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerConfig_t serverConfigWithNewHeaderType =
            HZL_TEST_CORRECT_SERVER_CONFIG;
    hzl_ServerCtx_t ctx = {
            .serverConfig = &serverConfigWithNewHeaderType,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
    // Fake a Request being already received
    groupStates[0].currentRxLastMessageInstant = groupStates[0].sessionStartInstant + 1U;
    groupStates[0].currentCtrNonce = 0x010203U;
    groupStates[0].currentStk[0] = 99;
    memset(&groupStates[0].currentStk[1], 0, 15);
    // Dummy old session state as copied after getting a REN message
    groupStates[0].previousCtrNonce = 0x111111;
    groupStates[0].previousStk[0] = 150;
    memset(&groupStates[0].previousStk[1], 0, 15);
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {'A', 'B', 'C', 'D', 'E'};
    size_t userDataLen = 5;

    err = hzl_ServerBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_OK);
    // Header 0 + ctrnonce + ptlen + dataLen + tag
    atto_eq(msgToTx.dataLen, 3 + 3 + 1 + 5 + 8);
    // Packed Header 0
    atto_eq(msgToTx.data[0], 0);  // GID from API call
    atto_eq(msgToTx.data[1], 0);  // SID from server
    atto_eq(msgToTx.data[2], 4);  // PTY SADFD
    // CBS SADFD-Payload
    // Ctrnonce is increased by 1 compared to the state
    atto_eq(msgToTx.data[3], 0x03);  // Ctrnonce low
    atto_eq(msgToTx.data[4], 0x02);  // Ctrnonce mid
    atto_eq(msgToTx.data[5], 0x01);  // Ctrnonce high
    atto_eq(msgToTx.data[6], 5);  // Ptlen
    // Ciphertext
    const uint8_t expectedCtext[5] = {0xAF, 0xE4, 0x31, 0xE5, 0xBD}; // With new STK
    atto_memeq(&msgToTx.data[7], expectedCtext, 5);
    // Tag
    const uint8_t expectedTag[8] = {0x97, 0x96, 0xA0, 0x03, 0x46, 0x82, 0xE8, 0xF4};
    atto_memeq(&msgToTx.data[12], expectedTag, 8);
    // Ctrnonce was incremented in the state
    atto_eq(groupStates[0].currentCtrNonce, 0x010204);
}

void hzlServerTest_ServerBuildSecuredFd(void)
{
    hzlServerTest_ServerBuildSecuredFdMsgToTxMustBeNotNull();
    hzlServerTest_ServerBuildSecuredFdCtxMustBeNotNull();
    hzlServerTest_ServerBuildSecuredFdRequiresSomeClientsRequestedAlready();
    hzlServerTest_ServerBuildSecuredFdMsgDataMustBeNotNullWhenPositiveDataLen();
    hzlServerTest_ServerBuildSecuredFdDataLenMustBeShortEnough();
    hzlServerTest_ServerBuildSecuredFdDataLenDependsOnHeaderLen();
    hzlServerTest_ServerBuildSecuredFdCompactHeaderPreventsTooManyGroups();
    hzlServerTest_ServerBuildSecuredFdGidMustBeInConfig();
    hzlServerTest_ServerBuildSecuredFdHeaderIsPackedBeforePayload();
    hzlServerTest_ServerBuildSecuredFdHeaderPackingDependsOnType();
    hzlServerTest_ServerBuildSecuredFdMsgWithNoPayload();
    hzlServerTest_ServerBuildSecuredFdSuccessfully();
    hzlServerTest_ServerBuildSecuredFdSuccessfullyUsesNewKeyDuringRenewalPhase();
    HZL_TEST_PARTIAL_REPORT();
}
