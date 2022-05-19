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
 * Tests of the hzl_ServerForceSessionRenewal() function.
 *
 * @warning
 * REDUCING COVERAGE ON PURPOSE. NOT implementing all the testcases for all possible incorrect
 * content of the context, because they have already been checked for the hzl_ServerInit()
 * function and the inner checks are exactly the same, performed by the same internal
 * function hzl_ServerCheckCtxPointers().
 */

#include "hzlTest.h"

static void
hzlServerTest_ServerForceSessionRenewalMsgToTxMustBeNotNull(void)
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

    err = hzl_ServerForceSessionRenewal(NULL, &ctx, 1);

    atto_eq(err, HZL_ERR_NULL_PDU);
}

static void
hzlServerTest_ServerForceSessionRenewalCtxMustBeNotNull(void)
{
    hzl_Err_t err;
    hzl_CbsPduMsg_t msgToTx = {0};

    err = hzl_ServerForceSessionRenewal(&msgToTx, NULL, 1);

    atto_eq(err, HZL_ERR_NULL_CTX);
}

static void
hzlServerTest_ServerForceSessionRenewalGidMustBeValid(void)
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
    // Assume at least one Client Requested the state already:
    // the last RX message was after the start of the session.
    hzlTest_IoMockupCurrentTimeSucceeding(
            &groupStates[ctx.serverConfig->amountOfGroups - 1U].currentRxLastMessageInstant);

    err = hzl_ServerForceSessionRenewal(&msgToTx, &ctx, ctx.serverConfig->amountOfGroups);
    atto_eq(err, HZL_ERR_UNKNOWN_GROUP);

    err = hzl_ServerForceSessionRenewal(&msgToTx, &ctx,
                                        (uint8_t) (ctx.serverConfig->amountOfGroups - 1U));
    atto_eq(err, HZL_OK);
}

static void
hzlServerTest_ServerForceSessionRenewalRequiresSomeClientsRequestedAlready(void)
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

    err = hzl_ServerForceSessionRenewal(&msgToTx, &ctx, 1);
    atto_eq(err, HZL_ERR_NO_POTENTIAL_RECEIVER);
}

static void
hzlServerTest_ServerForceSessionRenewalRenewsAndBuildsRenMsg(void)
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
    // Dummy non-zero ctrnonce to see that it's used
    groupStates[1].currentCtrNonce = 0x110022;
    // Dummy STK
    groupStates[1].currentStk[0] = 99;
    memset(&groupStates[1].currentStk[1], 0, 15);  // The rest is zeros
    // Assume at least one Client Requested the state already:
    // the last RX message was after the start of the session.
    hzlTest_IoMockupCurrentTimeSucceeding(&groupStates[1].currentRxLastMessageInstant);

    err = hzl_ServerForceSessionRenewal(&msgToTx, &ctx, 1);

    atto_eq(err, HZL_OK);
    // Session was renewed
    atto_neq(groupStates[1].currentStk[0], 99);
    atto_eq(groupStates[1].previousStk[0], 99);
    // Greater by 1 because it was incremented after building the REN
    atto_eq(groupStates[1].previousCtrNonce, 0x110023);
    atto_eq(groupStates[1].currentCtrNonce, 0);
    // REN message available
    atto_eq(msgToTx.dataLen, 3 + 19);
    // Packed header 0
    atto_eq(msgToTx.data[0], 1);  // GID
    atto_eq(msgToTx.data[1], 0);  // SID from server
    atto_eq(msgToTx.data[2], 0);  // PTY REN
    // CBS SADFD-Payload
    atto_eq(msgToTx.data[3], 0x22);  // Ctrnonce low
    atto_eq(msgToTx.data[4], 0x00);  // Ctrnonce mid
    atto_eq(msgToTx.data[5], 0x11);  // Ctrnonce high
    // Tag
    const uint8_t expectedTag[16] = {
            0x41, 0x4D, 0xE4, 0x0E, 0x04, 0xD7, 0xB3, 0xC4,
            0x43, 0x89, 0x78, 0x82, 0xDE, 0xA1, 0x3D, 0xAE,
    };
    atto_memeq(&msgToTx.data[6], expectedTag, 16);
    // Ctrnonce was incremented in the state
    atto_eq(groupStates[1].previousCtrNonce, 0x110023);
}

static void
hzlServerTest_ServerForceSessionRenewalOnlyBuildRenMsgDuringExistingRenewal(void)
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
    // Dummy non-zero ctrnonce to see that it's used
    groupStates[1].currentCtrNonce = 0x110022;
    // Dummy STK
    groupStates[1].currentStk[0] = 99;
    memset(&groupStates[1].currentStk[1], 0, 15);  // The rest is zeros
    // Assume at least one Client Requested the state already:
    // the last RX message was after the start of the session.
    hzlTest_IoMockupCurrentTimeSucceeding(&groupStates[1].currentRxLastMessageInstant);

    err = hzl_ServerForceSessionRenewal(&msgToTx, &ctx, 1);
    atto_eq(err, HZL_OK);
    // Session was renewed
    atto_neq(groupStates[1].currentStk[0], 99);
    atto_eq(groupStates[1].previousStk[0], 99);
    // Copying STKs to compare later
    uint8_t currentStkAtFirstRenewalCall[16];
    memcpy(currentStkAtFirstRenewalCall, groupStates[1].currentStk, 16);
    // Greater by 1 because it was incremented after building the REN
    atto_eq(groupStates[1].previousCtrNonce, 0x110023);
    atto_eq(groupStates[1].currentCtrNonce, 0);
    // REN message available
    atto_eq(msgToTx.dataLen, 3 + 19);

    err = hzl_ServerForceSessionRenewal(&msgToTx, &ctx, 1);
    atto_eq(err, HZL_OK);
    // Session was NOT renewed
    atto_neq(groupStates[1].currentStk[0], 99);
    atto_eq(groupStates[1].previousStk[0], 99);
    atto_memeq(groupStates[1].currentStk, currentStkAtFirstRenewalCall, 16);
    // Greater by 2 because it was incremented after building the SECOND REN
    atto_eq(groupStates[1].previousCtrNonce, 0x110024);
    atto_eq(groupStates[1].currentCtrNonce, 0);
    // REN message available
    atto_eq(msgToTx.dataLen, 3 + 19);

    err = hzl_ServerForceSessionRenewal(&msgToTx, &ctx, 1);
    atto_eq(err, HZL_OK);
    // Session was NOT renewed
    atto_neq(groupStates[1].currentStk[0], 99);
    atto_eq(groupStates[1].previousStk[0], 99);
    atto_memeq(groupStates[1].currentStk, currentStkAtFirstRenewalCall, 16);
    // Greater by 3 because it was incremented after building the THIRD REN
    atto_eq(groupStates[1].previousCtrNonce, 0x110025);
    atto_eq(groupStates[1].currentCtrNonce, 0);
    // REN message available
    atto_eq(msgToTx.dataLen, 3 + 19);
}

void hzlServerTest_ServerForceSessionRenewal(void)
{
    hzlServerTest_ServerForceSessionRenewalMsgToTxMustBeNotNull();
    hzlServerTest_ServerForceSessionRenewalCtxMustBeNotNull();
    hzlServerTest_ServerForceSessionRenewalGidMustBeValid();
    hzlServerTest_ServerForceSessionRenewalRequiresSomeClientsRequestedAlready();
    hzlServerTest_ServerForceSessionRenewalRenewsAndBuildsRenMsg();
    hzlServerTest_ServerForceSessionRenewalOnlyBuildRenMsgDuringExistingRenewal();
    HZL_TEST_PARTIAL_REPORT();
}
