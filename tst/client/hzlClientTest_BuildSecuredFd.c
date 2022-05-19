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
 * Tests of the hzl_ClientBuildSecuredFd() function.
 *
 * @warning
 * REDUCING COVERAGE ON PURPOSE. NOT implementing all the testcases for all possible incorrect
 * content of the context, because they have already been checked for the hzl_ClientInit()
 * function and the inner checks are exactly the same, performed by the same internal
 * function hzl_ClientCheckCtxPointers().
 */

#include "hzlTest.h"

static void
hzlClientTest_ClientBuildSecuredFdMsgToTxMustBeNotNull(void)
{
    hzl_Err_t err;

    err = hzl_ClientBuildSecuredFd(NULL, NULL, (void*) 1, 0, 0);

    atto_eq(err, HZL_ERR_NULL_PDU);
}

static void
hzlClientTest_ClientBuildSecuredFdCtxMustBeNotNull(void)
{
    hzl_Err_t err;
    hzl_CbsPduMsg_t msgToTx = {0};

    err = hzl_ClientBuildSecuredFd(&msgToTx, NULL, NULL, 0, 0);

    atto_eq(err, HZL_ERR_NULL_CTX);
}

// NOTE: tests for all possible issues WITHIN the context are skipped. See warning in file header.

static void
hzlClientTest_ClientBuildSecuredFdMsgDataMustBeNotNullWhenPositiveDataLen(void)
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

    err = hzl_ClientBuildSecuredFd(&msgToTx, &ctx, NULL, 1, 0);
    atto_eq(err, HZL_ERR_NULL_SDU);

    err = hzl_ClientBuildSecuredFd(&msgToTx, &ctx, NULL, 0, 0);
    atto_neq(err, HZL_ERR_NULL_SDU); // Different error
}

static void
hzlClientTest_ClientBuildSecuredFdDataLenMustBeShortEnough(void)
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
    // Dummy established-session state
    groupStates[0].currentCtrNonce = 1;
    groupStates[0].currentStk[0] = 99;
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen;
    // Requirement for this test: the header 0 is 3 bytes long, thus at most 49 bytes of dataLen.
    atto_eq(ctx.clientConfig->headerType, HZL_HEADER_0);

    userDataLen = 49;  // Still fits
    err = hzl_ClientBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_OK);

    userDataLen = 50;  // Too much
    err = hzl_ClientBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_ERR_TOO_LONG_SDU);
}

static void
hzlClientTest_ClientBuildSecuredFdDataLenDependsOnHeaderLen(void)
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
    // Dummy established-session state
    groupStates[0].currentCtrNonce = 1;
    groupStates[0].currentStk[0] = 99;
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen;
    // Requirement for this test: the header 6 is 1 bytes long, thus at most 51 bytes of dataLen.
    atto_eq(ctx.clientConfig->headerType, HZL_HEADER_6);

    userDataLen = 51;  // Still fits
    err = hzl_ClientBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_OK);

    userDataLen = 52;  // Too much
    err = hzl_ClientBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_ERR_TOO_LONG_SDU);
}

static void
hzlClientTest_ClientBuildSecuredFdCompactHeaderPreventsTooManyGroups(void)
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
    // Dummy established-session state
    groupStates[0].currentCtrNonce = 1;
    groupStates[0].currentStk[0] = 99;
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen = 4;
    // Requirement for this test: the header 4 is 1 bytes long with only 4 groups.
    atto_eq(ctx.clientConfig->headerType, HZL_HEADER_4);

    err = hzl_ClientBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 33);

    atto_eq(err, HZL_ERR_GID_TOO_LARGE_FOR_CONFIGURED_HEADER_TYPE);
}

static void
hzlClientTest_ClientBuildSecuredFdGidMustBeInConfig(void)
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

    err = hzl_ClientBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen,
                                   231);  // Unexisting GID

    atto_eq(err, HZL_ERR_UNKNOWN_GROUP);
}

static void
hzlClientTest_ClientBuildSecuredFdRequiresAnEstablishedSession(void)
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
    // No session established for group 0
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen = 4;

    err = hzl_ClientBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_ERR_SESSION_NOT_ESTABLISHED);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t));

    // Fake a Request being transmitted, but no Response received yet
    groupStates[0].requestNonce = 123; // Non-zero
    err = hzl_ClientBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_ERR_SESSION_NOT_ESTABLISHED);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t));
}

static void
hzlClientTest_ClientBuildSecuredFdHeaderIsPackedBeforePayload(void)
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
    // Dummy established-session state
    groupStates[0].currentCtrNonce = 0x112233;
    groupStates[0].currentStk[0] = 99;
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen = 4;

    // Requirement for this test: the header 0 is 3 bytes long
    atto_eq(ctx.clientConfig->headerType, HZL_HEADER_0);
    err = hzl_ClientBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 0);

    atto_eq(err, HZL_OK);
    // Header 0 + ctrnonce + ptlen + dataLen + tag
    atto_eq(msgToTx.dataLen, 3 + 3 + 1 + 4 + 8);
    // Packed Header 0
    atto_eq(msgToTx.data[0], 0);  // GID from API call
    atto_eq(msgToTx.data[1], 13);  // SID from client config
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
hzlClientTest_ClientBuildSecuredFdHeaderPackingDependsOnType(void)
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
    // Dummy established-session state
    groupStates[1].currentCtrNonce = 0x112233;  // Group with idx == 1 has GID == 2
    groupStates[1].currentStk[0] = 99;
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen = 4;

    // Requirement for this test: the header 4 is 1 bytes long
    atto_eq(ctx.clientConfig->headerType, HZL_HEADER_4);
    err = hzl_ClientBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 2);

    atto_eq(err, HZL_OK);
    // Header 4 + ctrnonce + ptlen + dataLen + tag
    atto_eq(msgToTx.dataLen, 1 + 3 + 1 + 4 + 8);
    // Packed Header 4. Bits: |sssg gppp| (s=SID, g=GID, p=PTY)
    // SID from client config, GID from API call, PTY for SADFD.
    const size_t expectedPackedHdr = 3U << 5U | 2U << 3U | 4U;
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
    atto_eq(groupStates[1].currentCtrNonce, 0x112234);
}

static void
hzlClientTest_ClientBuildSecuredFdMaxCtrnonceRequiresHandshake(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientConfig_t clientConfigWithNewHeaderType =
            HZL_TEST_CORRECT_CLIENT_CONFIG;
    hzl_ClientCtx_t ctx = {
            .clientConfig = &clientConfigWithNewHeaderType,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
    // Dummy established-session state, with expired ctrnonce, should require a new handshake
    groupStates[0].currentCtrNonce = 0xFFFFFF;
    groupStates[0].currentStk[0] = 99;
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {1, 2, 3, 4};
    size_t userDataLen = 4;

    err = hzl_ClientBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 0);

    atto_eq(err, HZL_ERR_SESSION_NOT_ESTABLISHED);
}

static void
hzlClientTest_ClientBuildSecuredFdMsgWithNoPayload(void)
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
    // Dummy established-session state
    groupStates[0].currentCtrNonce = 0x010203;
    groupStates[0].currentStk[0] = 99;
    hzl_CbsPduMsg_t msgToTx = {0};
    size_t userDataLen = 0;

    err = hzl_ClientBuildSecuredFd(&msgToTx, &ctx, NULL, userDataLen, 0);

    atto_eq(err, HZL_OK);
    // Header 0 + ctrnonce + ptlen + dataLen + tag
    atto_eq(msgToTx.dataLen, 3 + 3 + 1 + 0 + 8);
    // Packed Header 0
    atto_eq(msgToTx.data[0], 0);  // GID from API call
    atto_eq(msgToTx.data[1], 13);  // SID from client config
    atto_eq(msgToTx.data[2], 4);  // PTY SADFD
    // CBS SADFD-Payload
    // Ctrnonce is increased by 1 compared to the state
    atto_eq(msgToTx.data[3], 0x03);  // Ctrnonce low
    atto_eq(msgToTx.data[4], 0x02);  // Ctrnonce mid
    atto_eq(msgToTx.data[5], 0x01);  // Ctrnonce high
    atto_eq(msgToTx.data[6], 0);  // Ptlen
    // Ciphertext: empty
    // Tag
    const uint8_t expectedTag[8] = {0x8D, 0x4C, 0xAB, 0x67, 0x96, 0xB9, 0xF8, 0x5E};
    atto_memeq(&msgToTx.data[7], expectedTag, 8);
    // Ctrnonce was incremented in the state
    atto_eq(groupStates[0].currentCtrNonce, 0x010204);
}

static void
hzlClientTest_ClientBuildSecuredFdSuccessfully(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientConfig_t clientConfigWithNewHeaderType =
            HZL_TEST_CORRECT_CLIENT_CONFIG;
    hzl_ClientCtx_t ctx = {
            .clientConfig = &clientConfigWithNewHeaderType,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
    groupStates[0].currentCtrNonce = 0x010203;
    groupStates[0].currentStk[0] = 99;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {'A', 'B', 'C', 'D', 'E'};
    size_t userDataLen = 5;

    err = hzl_ClientBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_OK);
    // Header 0 + ctrnonce + ptlen + dataLen + tag
    atto_eq(msgToTx.dataLen, 3 + 3 + 1 + 5 + 8);
    // Packed Header 0
    atto_eq(msgToTx.data[0], 0);  // GID from API call
    atto_eq(msgToTx.data[1], 13);  // SID from client config
    atto_eq(msgToTx.data[2], 4);  // PTY SADFD
    // CBS SADFD-Payload
    // Ctrnonce is increased by 1 compared to the state
    atto_eq(msgToTx.data[3], 0x03);  // Ctrnonce low
    atto_eq(msgToTx.data[4], 0x02);  // Ctrnonce mid
    atto_eq(msgToTx.data[5], 0x01);  // Ctrnonce high
    atto_eq(msgToTx.data[6], 5);  // Ptlen
    // Ciphertext
    const uint8_t expectedCtext[5] = {0xE0, 0x04, 0xD9, 0xD9, 0x05};
    atto_memeq(&msgToTx.data[7], expectedCtext, 5);
    // Tag
    const uint8_t expectedTag[8] = {0xAB, 0x04, 0x46, 0x61, 0x2C, 0x54, 0x37, 0x1F};
    atto_memeq(&msgToTx.data[12], expectedTag, 8);
    // Ctrnonce was incremented in the state
    atto_eq(groupStates[0].currentCtrNonce, 0x010204);
}


static void
hzlClientTest_ClientBuildSecuredFdSuccessfullyUsesNewKeyDuringRenewalPhase(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientConfig_t clientConfigWithNewHeaderType =
            HZL_TEST_CORRECT_CLIENT_CONFIG;
    hzl_ClientCtx_t ctx = {
            .clientConfig = &clientConfigWithNewHeaderType,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
    groupStates[0].currentCtrNonce = 0x010203;
    groupStates[0].currentStk[0] = 99;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    // Dummy old session state as copied after getting a REN message
    groupStates[0].previousCtrNonce = 0x111111;
    groupStates[0].previousStk[0] = 150;
    atto_zeros(&groupStates[0].previousStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    const uint8_t userData[64] = {'A', 'B', 'C', 'D', 'E'};
    size_t userDataLen = 5;

    err = hzl_ClientBuildSecuredFd(&msgToTx, &ctx, userData, userDataLen, 0);
    atto_eq(err, HZL_OK);
    // Header 0 + ctrnonce + ptlen + dataLen + tag
    atto_eq(msgToTx.dataLen, 3 + 3 + 1 + 5 + 8);
    // Packed Header 0
    atto_eq(msgToTx.data[0], 0);  // GID from API call
    atto_eq(msgToTx.data[1], 13);  // SID from client config
    atto_eq(msgToTx.data[2], 4);  // PTY SADFD
    // CBS SADFD-Payload
    // Ctrnonce is increased by 1 compared to the state
    atto_eq(msgToTx.data[3], 0x03);  // Ctrnonce low
    atto_eq(msgToTx.data[4], 0x02);  // Ctrnonce mid
    atto_eq(msgToTx.data[5], 0x01);  // Ctrnonce high
    atto_eq(msgToTx.data[6], 5);  // Ptlen
    // Ciphertext
    const uint8_t expectedCtext[5] = {0xE0, 0x04, 0xD9, 0xD9, 0x05}; // With new STK
    atto_memeq(&msgToTx.data[7], expectedCtext, 5);
    // Tag
    const uint8_t expectedTag[8] = {0xAB, 0x04, 0x46, 0x61, 0x2C, 0x54, 0x37, 0x1F};
    atto_memeq(&msgToTx.data[12], expectedTag, 8);
    // Ctrnonce was incremented in the state
    atto_eq(groupStates[0].currentCtrNonce, 0x010204);
}

void hzlClientTest_ClientBuildSecuredFd(void)
{
    hzlClientTest_ClientBuildSecuredFdMsgToTxMustBeNotNull();
    hzlClientTest_ClientBuildSecuredFdCtxMustBeNotNull();
    hzlClientTest_ClientBuildSecuredFdRequiresAnEstablishedSession();
    hzlClientTest_ClientBuildSecuredFdMsgDataMustBeNotNullWhenPositiveDataLen();
    hzlClientTest_ClientBuildSecuredFdDataLenMustBeShortEnough();
    hzlClientTest_ClientBuildSecuredFdDataLenDependsOnHeaderLen();
    hzlClientTest_ClientBuildSecuredFdCompactHeaderPreventsTooManyGroups();
    hzlClientTest_ClientBuildSecuredFdGidMustBeInConfig();
    hzlClientTest_ClientBuildSecuredFdHeaderIsPackedBeforePayload();
    hzlClientTest_ClientBuildSecuredFdHeaderPackingDependsOnType();
    hzlClientTest_ClientBuildSecuredFdMaxCtrnonceRequiresHandshake();
    hzlClientTest_ClientBuildSecuredFdMsgWithNoPayload();
    hzlClientTest_ClientBuildSecuredFdSuccessfully();
    hzlClientTest_ClientBuildSecuredFdSuccessfullyUsesNewKeyDuringRenewalPhase();
    HZL_TEST_PARTIAL_REPORT();
}
