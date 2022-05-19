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
 * Tests of the hzl_ServerProcessReceived() function for the REQ messages.
 *
 * @warning
 * REDUCING COVERAGE ON PURPOSE. NOT implementing all the testcases for all possible incorrect
 * content of the context, because they have already been checked for the hzl_ServerInit()
 * function and the inner checks are exactly the same, performed by the same internal
 * function hzl_ServerCheckCtxPointers().
 */

#include "hzlTest.h"

static void
hzlServerTest_ServerProcessReceivedRequestMsgMustHaveKnownGid(void)
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
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    uint8_t rxPdu[64] = {
            // Header 0
            30,  // GID
            1,  // SID != server
            2,  // PTY == REQ
            8, 9, 10, 11, 12, 13, 14, 15,  // Reqnonce
            20, 21, 22, 23, 24, 25, 26, 27,  // tag (incorrect)
            28, 29, 30, 31, 32, 33, 34, 35,  // tag (incorrect)
    };

    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_UNKNOWN_GROUP);

    rxPdu[0] = 0;
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_UNKNOWN_GROUP);
    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other checks are passing
}

static void
hzlServerTest_ServerProcessReceivedRequestMsgMustNotComeFromServer(void)
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
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            0,  // SID == server (INCORRECT!)
            2,  // PTY == REQ
            8, 9, 10, 11, 12, 13, 14, 15,  // Reqnonce
            20, 21, 22, 23, 24, 25, 26, 27,  // tag (incorrect)
            28, 29, 30, 31, 32, 33, 34, 35,  // tag (incorrect)
    };

    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_MESSAGE_FROM_MYSELF);

    rxPdu[1] = 1;
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_SECWARN_MESSAGE_FROM_MYSELF);
    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other checks are passing
}

static void
hzlServerTest_ServerProcessReceivedRequestMsgMustBeLongEnough(void)
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
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen;
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            1,  // SID != server
            2,  // PTY == REQ
            8, 9, 10, 11, 12, 13, 14, 15,  // Reqnonce
            20, 21, 22, 23, 24, 25, 26, 27,  // tag (incorrect)
            28, 29, 30, 31, 32, 33, 34, 35,  // tag (incorrect)
    };

    rxPduLen = 3 + 23;  // Header + payload (1 less than minimum)
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_REQ);

    rxPduLen = 3 + 24;  // Header + payload (minimum length)
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_REQ);
    atto_eq(err,
            HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other size checks are passing

    rxPduLen = 3 + 25;  // more than minimum is acceptable due to CAN FD paddings
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_REQ);
    atto_eq(err,
            HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other size checks are passing
}

static void
hzlServerTest_ServerProcessReceivedRequestMsgMustHaveNonZeroReqNonce(void)
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
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            1,  // SID != server
            2,  // PTY == REQ
            0, 0, 0, 0, 0, 0, 0, 0,  // Reqnonce
            20, 21, 22, 23, 24, 25, 26, 27,  // tag (incorrect)
            28, 29, 30, 31, 32, 33, 34, 35,  // tag (incorrect)
    };

    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_RECEIVED_ZERO_REQNONCE);

    rxPdu[4] = 42;
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_SECWARN_RECEIVED_ZERO_REQNONCE);
    atto_eq(err,
            HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other checks are passing
}

static void
hzlServerTest_ServerProcessReceivedRequestMsgSidMustBelongToGidGroup(void)
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
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    uint8_t rxPdu[64] = {
            // Header 0
            2,  // GID
            1,  // SID, which does NOT belong into Group with GID==2
            2,  // PTY == REQ
            8, 9, 10, 11, 12, 13, 14, 15,  // Reqnonce
            20, 21, 22, 23, 24, 25, 26, 27,  // tag (incorrect)
            28, 29, 30, 31, 32, 33, 34, 35,  // tag (incorrect)
    };

    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_NOT_IN_GROUP);

    rxPdu[1] = 2;
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_SECWARN_NOT_IN_GROUP);
    atto_eq(err,
            HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other checks are passing
}

static void
hzlServerTest_ServerProcessReceivedRequestMsgWithValidTagSuccessfully(void)
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
    hzl_Timestamp_t timestampOfReqRx = 0;
    hzlTest_IoMockupCurrentTimeSucceeding(&timestampOfReqRx);
    // Inner state indicates no REQ was received so far
    atto_eq(groupStates[0].currentRxLastMessageInstant, groupStates[0].sessionStartInstant);
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            1,  // SID != server
            2,  // PTY == REQ
            8, 9, 10, 11, 12, 13, 14, 15,  // Reqnonce
            // Assuming the LTK being [1, 0, 0, ..., 0]
            0xC7, 0x70, 0xFE, 0x35, 0x67, 0x85, 0x78, 0xD8,  // Tag (valid)
            0x2E, 0x78, 0x57, 0x90, 0xCD, 0x76, 0xC1, 0x1F,  // Tag (valid)
    };

    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    // Output contains empty message for the user
    atto_eq(err, HZL_OK);
    atto_eq(unpackedMsg.canId, 0xABC);
    atto_eq(unpackedMsg.dataLen, 0);
    atto_eq(unpackedMsg.gid, 0);
    atto_eq(unpackedMsg.sid, 0);
    atto_false(unpackedMsg.wasSecured);
    atto_false(unpackedMsg.isForUser);
    atto_zeros(&unpackedMsg.data, 64);

    // New timestamps are saved
    atto_gt(groupStates[0].currentRxLastMessageInstant, timestampOfReqRx);
    atto_neq(groupStates[0].currentRxLastMessageInstant, groupStates[0].sessionStartInstant);
}

static void
hzlServerTest_ServerProcessReceivedRequestMsgWithValidTagGeneratesResponse(void)
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
    hzl_Timestamp_t timestampOfReqRx = 0;
    hzlTest_IoMockupCurrentTimeSucceeding(&timestampOfReqRx);
    // Inner state indicates no REQ was received so far
    atto_eq(groupStates[0].currentRxLastMessageInstant, groupStates[0].sessionStartInstant);
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            1,  // SID != server
            2,  // PTY == REQ
            8, 9, 10, 11, 12, 13, 14, 15,  // Reqnonce
            // Assuming the LTK being [1, 0, 0, ..., 0]
            0xC7, 0x70, 0xFE, 0x35, 0x67, 0x85, 0x78, 0xD8,
            0x2E, 0x78, 0x57, 0x90, 0xCD, 0x76, 0xC1, 0x1F,  // Tag (valid)
    };

    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    atto_eq(err, HZL_OK);
    // Response generated for user to transmit
    atto_eq(msgToTx.dataLen, 3 + 44);
    // CBS header
    atto_eq(msgToTx.data[0], 0);  // GID
    atto_eq(msgToTx.data[1], 0);  // SID
    atto_eq(msgToTx.data[2], 1);  // RES
    // RES payload
    atto_eq(msgToTx.data[3], 1);  // Client SID
    atto_eq(msgToTx.data[4], 0x00); // Ctrnonce LS
    atto_eq(msgToTx.data[5], 0x00); // Ctrnonce
    atto_eq(msgToTx.data[6], 0x00); // Ctrnonce MS
    // Resnonce, generated with the dummy TRNG
    const uint8_t expectedDummyRandomResNonce[] = {0, 1, 2, 3, 4, 5, 6, 7};
    atto_memeq(&msgToTx.data[7], expectedDummyRandomResNonce, 8);
    // Assuming the LTK being [2, 0, 0, ..., 0]
    // Assuming a reqnonce of [8, 9, ..., 15]
    // Ciphertext: encrypted STK [0, 1, ..., 15]
    const uint8_t expectedDummyCiphertext[] = {
            0x35, 0x60, 0xC1, 0x5D, 0xEA, 0x4D, 0x17, 0x8A,
            0x28, 0x1B, 0x6E, 0xF2, 0xB0, 0xA7, 0x31, 0xFF,
    };
    atto_memeq(&msgToTx.data[7 + 8], expectedDummyCiphertext, 16);
    const uint8_t expectedTag[] = {
            0x3B, 0x51, 0x51, 0x7A, 0x01, 0x7E, 0x4F, 0x59,
            0x35, 0xE1, 0xA9, 0x8C, 0x80, 0xF9, 0xFB, 0x34,
    };
    atto_memeq(&msgToTx.data[7 + 8 + 16], expectedTag, 16);
}

void hzlServerTest_ServerProcessReceivedRequest(void)
{
    hzlServerTest_ServerProcessReceivedRequestMsgMustHaveKnownGid();
    hzlServerTest_ServerProcessReceivedRequestMsgMustNotComeFromServer();
    hzlServerTest_ServerProcessReceivedRequestMsgMustBeLongEnough();
    hzlServerTest_ServerProcessReceivedRequestMsgMustHaveNonZeroReqNonce();
    hzlServerTest_ServerProcessReceivedRequestMsgSidMustBelongToGidGroup();
    hzlServerTest_ServerProcessReceivedRequestMsgWithValidTagSuccessfully();
    hzlServerTest_ServerProcessReceivedRequestMsgWithValidTagGeneratesResponse();
    HZL_TEST_PARTIAL_REPORT();
}
