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
 * Tests of the hzl_ClientProcessReceived() function for the RES messages.
 *
 * @warning
 * REDUCING COVERAGE ON PURPOSE. NOT implementing all the testcases for all possible incorrect
 * content of the context, because they have already been checked for the hzl_ClientInit()
 * function and the inner checks are exactly the same, performed by the same internal
 * function hzl_ClientCheckCtxPointers().
 */

#include "hzlTest.h"

static void
hzlClientTest_ClientProcessReceivedRenewalMsgMustHaveKnownGid(void)
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
    groupStates[0].currentStk[0] = 1;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    uint8_t rxPdu[64] = {
            53,  // GID
            0,  // SID == server
            0,  // PTY == REN
            0x33, 0x22, 0x11,  // Ctrnonce
            20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
            35  // tag (incorrect)
    };

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_MSG_IGNORED);

    rxPdu[0] = 0;
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other checks are passing
}

static void
hzlClientTest_ClientProcessReceivedRenewalMsgMustComeFromServer(void)
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
    groupStates[0].currentStk[0] = 1;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    uint8_t rxPdu[64] = {
            0,  // GID
            2,  // SID == server
            0,  // PTY == REN
            0x33, 0x22, 0x11,  // Ctrnonce
            20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
            35  // tag (incorrect)
    };

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_SERVER_ONLY_MESSAGE);

    rxPdu[1] = 0;
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_SECWARN_SERVER_ONLY_MESSAGE);
    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other checks are passing
}

static void
hzlClientTest_ClientProcessReceivedRenewalMsgMustBeLongEnough(void)
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
    groupStates[0].currentStk[0] = 1;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen;
    const uint8_t rxPdu[64] = {
            0,  // GID
            0,  // SID == server
            0,  // PTY == REN
            0x33, 0x22, 0x11,  // Ctrnonce
            20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
            35  // tag (incorrect)
    };

    rxPduLen = 3 + 18;  // Header + payload (1 less than minimum)
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_REN);

    rxPduLen = 3 + 19;  // Header + payload (minimum length)
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_REN);
    atto_eq(err,
            HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other size checks are passing

    rxPduLen = 3 + 20;  // more than minimum is acceptable due to CAN FD paddings
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_REN);
    atto_eq(err,
            HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other size checks are passing
}

static void
hzlClientTest_ClientProcessReceivedRenewalReceiverMustHaveNonZeroStk(void)
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
    // No established session for group 0
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    const uint8_t rxPdu[64] = {
            0,  // GID
            0,  // SID == server
            0,  // PTY == REN
            0x33, 0x22, 0x11,  // Ctrnonce
            20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
            35  // tag (incorrect)
    };

    atto_zeros(ctx.groupStates[0].currentStk, 16);  // assertion for this test
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    atto_eq(err, HZL_ERR_SESSION_NOT_ESTABLISHED);
}

static void
hzlClientTest_ClientProcessReceivedRenewalReceiverMustNotBeInHandshake(void)
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
    // Dummy established-session state but ongoing handshake
    groupStates[0].currentCtrNonce = 1;
    groupStates[0].currentStk[0] = 1;
    groupStates[0].requestNonce = 13; // non-zero
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    const uint8_t rxPdu[64] = {
            0,  // GID
            0,  // SID == server
            0,  // PTY == REN
            0x33, 0x22, 0x11,  // Ctrnonce
            20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
            35  // tag (incorrect)
    };

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit
}

static void
hzlClientTest_ClientProcessReceivedRenewalReceiverMustNotBeInRenewalPhase(void)
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
    groupStates[0].currentStk[0] = 1;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    // Dummy valid state of the previous session
    groupStates[0].previousCtrNonce = 0;
    groupStates[0].previousStk[0] = 111;
    atto_zeros(&groupStates[0].previousStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    const uint8_t rxPdu[64] = {
            0,  // GID
            0,  // SID == server
            0,  // PTY == REN
            0x33, 0x22, 0x11,  // Ctrnonce
            20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
            35  // tag (incorrect)
    };

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit
}

static void
hzlClientTest_ClientProcessReceivedRenewalReceiverMustNotHaveExpiredCtrNonce(void)
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
    // Dummy established-session state, with expired ctrnonce
    groupStates[0].currentCtrNonce = 0xFFFFFF;
    groupStates[0].currentStk[0] = 1;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    const uint8_t rxPdu[64] = {
            0,  // GID
            0,  // SID == server
            0,  // PTY == REN
            0x33, 0x22, 0x11,  // Ctrnonce
            20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
            35  // tag (incorrect)
    };

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    atto_eq(err, HZL_ERR_SESSION_NOT_ESTABLISHED);
}

static void
hzlClientTest_ClientProcessReceivedRenewalMsgMustHaveNonExpiredCtrnonce(void)
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
    groupStates[0].currentStk[0] = 1;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    uint8_t rxPdu[64] = {
            0,  // GID
            0,  // SID == server
            0,  // PTY == REN
            0xFF, 0xFF, 0xFF,  // Ctrnonce
            20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
            35  // tag (incorrect)
    };

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_RECEIVED_OVERFLOWN_NONCE);

    rxPdu[4] = 0xFE;  // 1 before expired
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_SECWARN_RECEIVED_OVERFLOWN_NONCE);
    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other checks are passing
}

static void
hzlClientTest_ClientProcessReceivedRenewalMsgMustHaveFreshCtrnonce(void)
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
    groupStates[0].currentCtrNonce = 0x001000;
    groupStates[0].currentStk[0] = 1;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    const uint8_t rxPdu[64] = {
            0,  // GID
            0,  // SID == server
            0,  // PTY == REN
            0x0A, 0x00, 0x00,  // Ctrnonce
            20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
            35  // tag (incorrect)
    };

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    atto_eq(err, HZL_ERR_SECWARN_OLD_MESSAGE);
}

static void
hzlClientTest_ClientProcessReceivedRenewalMsgWithValidTagSuccessfully(void)
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
    groupStates[0].currentStk[0] = 1;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    const uint8_t rxPdu[64] = {
            0,  // GID
            0,  // SID == server
            0,  // PTY == REN
            0x33, 0x22, 0x11,  // Ctrnonce
            0xE9, 0x61, 0x78, 0x5E, 0xA3, 0x29, 0xB2, 0xE4, 0x02, 0x2F, 0x00, 0xD9, 0xBC,
            0xA4, 0xF1, 0x14  // tag (valid)
    };
    const hzl_Timestamp_t previousunpackedMsgTimestamp = groupStates[0].currentRxLastMessageInstant;

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    // Output contains empty message for the user
    atto_eq(err, HZL_OK);
    atto_eq(unpackedMsg.canId, 0xABC);
    atto_eq(unpackedMsg.dataLen, 0);
    atto_eq(unpackedMsg.gid, 0);
    atto_eq(unpackedMsg.sid, 0);
    atto_false(unpackedMsg.wasSecured);
    atto_false(unpackedMsg.isForUser);
    atto_zeros(&unpackedMsg.data, 64);

    // New ctrnonce is stored in state, incremented
    atto_eq(groupStates[0].currentCtrNonce, 0x112233 + 1);
    // New timestamps are saved
    atto_gt(groupStates[0].currentRxLastMessageInstant, previousunpackedMsgTimestamp);
    // Old session information is backed up
    atto_memeq(groupStates[0].previousStk, groupStates[0].currentStk, 16);
    atto_eq(groupStates[0].previousCtrNonce, groupStates[0].currentCtrNonce);
    // A request message is ready to transmit
    // Header 0 + reqnonce + tag
    atto_eq(msgToTx.dataLen, 3 + 8 + 16);
    // Packed Header 0
    atto_eq(msgToTx.data[0], 0);  // GID from API call
    atto_eq(msgToTx.data[1], 13);  // SID from client config
    atto_eq(msgToTx.data[2], 2);  // PTY REQ
    // Not checking the payload, it is already been done in the tests of the Request TX functions
}

static void
hzlClientTest_ClientProcessReceivedRenewalMsgIsIgnoredIfNotFirst(void)
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
    groupStates[0].currentStk[0] = 1;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    const uint8_t rxPdu[64] = {
            0,  // GID
            0,  // SID == server
            0,  // PTY == REN
            0x33, 0x22, 0x11,  // Ctrnonce
            0xE9, 0x61, 0x78, 0x5E, 0xA3, 0x29, 0xB2, 0xE4, 0x02, 0x2F, 0x00, 0xD9, 0xBC,
            0xA4, 0xF1, 0x14  // tag (valid)
    };

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_OK);
    // Empty message for the user
    atto_eq(unpackedMsg.canId, 0xABC);
    atto_eq(unpackedMsg.dataLen, 0);
    atto_eq(unpackedMsg.gid, 0);
    atto_eq(unpackedMsg.sid, 0);
    atto_false(unpackedMsg.wasSecured);
    atto_false(unpackedMsg.isForUser);
    atto_zeros(&unpackedMsg.data, 64);
    // A request message is ready to transmit
    // Header 0 + reqnonce + tag
    atto_eq(msgToTx.dataLen, 3 + 8 + 16);
    // Packed Header 0
    atto_eq(msgToTx.data[0], 0);  // GID from API call
    atto_eq(msgToTx.data[1], 13);  // SID from client config
    atto_eq(msgToTx.data[2], 2);  // PTY REQ
    // Not checking the payload, it is already been done in the tests of the Request TX functions
    memset(&msgToTx, 0, sizeof(hzl_CbsPduMsg_t));

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit
}

void hzlClientTest_ClientProcessReceivedRenewal(void)
{
    hzlClientTest_ClientProcessReceivedRenewalMsgMustHaveKnownGid();
    hzlClientTest_ClientProcessReceivedRenewalMsgMustComeFromServer();
    hzlClientTest_ClientProcessReceivedRenewalMsgMustBeLongEnough();
    hzlClientTest_ClientProcessReceivedRenewalReceiverMustHaveNonZeroStk();
    hzlClientTest_ClientProcessReceivedRenewalReceiverMustNotBeInHandshake();
    hzlClientTest_ClientProcessReceivedRenewalReceiverMustNotBeInRenewalPhase();
    hzlClientTest_ClientProcessReceivedRenewalMsgMustHaveNonExpiredCtrnonce();
    hzlClientTest_ClientProcessReceivedRenewalReceiverMustNotHaveExpiredCtrNonce();
    hzlClientTest_ClientProcessReceivedRenewalMsgMustHaveFreshCtrnonce();
    hzlClientTest_ClientProcessReceivedRenewalMsgWithValidTagSuccessfully();
    hzlClientTest_ClientProcessReceivedRenewalMsgIsIgnoredIfNotFirst();
    HZL_TEST_PARTIAL_REPORT();
}
