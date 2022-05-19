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
hzlClientTest_ClientProcessReceivedResponseMsgMustHaveKnownGid(void)
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
    // Dummy request-sent session state
    groupStates[0].requestNonce = 444;
    hzlTest_IoMockupCurrentTimeSucceeding(&groupStates[0].lastHandshakeEventInstant);
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    uint8_t rxPdu[64] = {
            // Header 0
            30,  // GID
            0,  // SID == server
            1,  // PTY == RES
            13,  // Requesting client SID
            0x33, 0x22, 0x11,  // Ctrnonce
            8, 9, 10, 11, 12, 13, 14, 15,  // Resnonce
            // Assuming a reqnonce = {0, 1, 2, 3, 4, 5, 6, 7}
            11, 22, 33, 44, 55, 66, 77, 88,  // ctext
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_UNKNOWN_GROUP);

    rxPdu[0] = 0;
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_UNKNOWN_GROUP);
    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other checks are passing
}

static void
hzlClientTest_ClientProcessReceivedResponseMsgMustComeFromServer(void)
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
    // Dummy request-sent session state
    groupStates[0].requestNonce = 444;
    hzlTest_IoMockupCurrentTimeSucceeding(&groupStates[0].lastHandshakeEventInstant);
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            2,  // SID != server (INCORRECT)
            1,  // PTY == RES
            13,  // Requesting client SID
            0x33, 0x22, 0x11,  // Ctrnonce
            8, 9, 10, 11, 12, 13, 14, 15,  // Resnonce
            // Assuming a reqnonce = {0, 1, 2, 3, 4, 5, 6, 7}
            11, 22, 33, 44, 55, 66, 77, 88,  // ctext
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_SERVER_ONLY_MESSAGE);

    rxPdu[1] = 0;
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_SECWARN_SERVER_ONLY_MESSAGE);
    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other checks are passing
}

static void
hzlClientTest_ClientProcessReceivedResponseMsgMustBeLongEnough(void)
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
    // Dummy request-sent session state
    groupStates[0].requestNonce = 444;
    hzlTest_IoMockupCurrentTimeSucceeding(&groupStates[0].lastHandshakeEventInstant);
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen;
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            0,  // SID == server
            1,  // PTY == RES
            13,  // Requesting client SID
            0x33, 0x22, 0x11,  // Ctrnonce
            8, 9, 10, 11, 12, 13, 14, 15,  // Resnonce
            // Assuming a reqnonce = {0, 1, 2, 3, 4, 5, 6, 7}
            11, 22, 33, 44, 55, 66, 77, 88,  // ctext
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };

    rxPduLen = 3 + 43;  // Header + payload (1 less than minimum)
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_RES);

    rxPduLen = 3 + 44;  // Header + payload (minimum length)
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_RES);
    atto_eq(err,
            HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other size checks are passing

    rxPduLen = 3 + 45;  // more than minimum is acceptable due to CAN FD paddings
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_RES);
    atto_eq(err,
            HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other size checks are passing
}

static void
hzlClientTest_ClientProcessReceivedResponseMsgNotForMeIsIgnored(void)
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
    // Dummy request-sent session state
    groupStates[0].requestNonce = 444;
    hzlTest_IoMockupCurrentTimeSucceeding(&groupStates[0].lastHandshakeEventInstant);
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            0,  // SID == server
            1,  // PTY == RES
            13,  // Requesting client SID
            0x33, 0x22, 0x11,  // Ctrnonce
            8, 9, 10, 11, 12, 13, 14, 15,  // Resnonce
            // Assuming a reqnonce = {0, 1, 2, 3, 4, 5, 6, 7}
            11, 22, 33, 44, 55, 66, 77, 88,  // ctext
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };

    rxPdu[3] = 14;  // Incorrect Client SID (not for me)
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_MSG_IGNORED);

    rxPdu[3] = 13;  // Correct Client SID (for me)
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(err,
            HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other size checks are passing
}

static void
hzlClientTest_ClientProcessReceivedResponseMsgMustBeExpected(void)
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
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            0,  // SID == server
            1,  // PTY == RES
            13,  // Requesting client SID
            0x33, 0x22, 0x11,  // Ctrnonce
            8, 9, 10, 11, 12, 13, 14, 15,  // Resnonce
            // Assuming a reqnonce = {0, 1, 2, 3, 4, 5, 6, 7}
            11, 22, 33, 44, 55, 66, 77, 88,  // ctext
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };
    // NO request-sent session state
    atto_eq(groupStates[0].requestNonce, 0);

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_NOT_EXPECTING_A_RESPONSE);
}

static void
hzlClientTest_ClientProcessReceivedResponseMsgMustArriveWithinTimeout(void)
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
    // Dummy request-sent session state
    groupStates[0].requestNonce = 444;
    hzlTest_IoMockupCurrentTimeSucceeding(&groupStates[0].lastHandshakeEventInstant);
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            0,  // SID == server
            1,  // PTY == RES
            13,  // Requesting client SID
            0x33, 0x22, 0x11,  // Ctrnonce
            8, 9, 10, 11, 12, 13, 14, 15,  // Resnonce
            // Assuming a reqnonce = {0, 1, 2, 3, 4, 5, 6, 7}
            11, 22, 33, 44, 55, 66, 77, 88,  // ctext
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };

    // Make a lot of time virtually pass
    for (size_t i = 0; i < 500; i++)
    {
        hzlTest_IoMockupCurrentTimeSucceeding(NULL);
    }

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_RESPONSE_TIMEOUT);
}

static void
hzlClientTest_ClientProcessReceivedResponseMsgMustHaveNonExpiredCtrnonce(void)
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
    // Dummy request-sent session state
    groupStates[0].requestNonce = 444;
    hzlTest_IoMockupCurrentTimeSucceeding(&groupStates[0].lastHandshakeEventInstant);
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            0,  // SID == server
            1,  // PTY == RES
            13,  // Requesting client SID
            0xFF, 0xFF, 0xFF,  // Ctrnonce EXPIRED
            8, 9, 10, 11, 12, 13, 14, 15,  // Resnonce
            // Assuming a reqnonce = {0, 1, 2, 3, 4, 5, 6, 7}
            11, 22, 33, 44, 55, 66, 77, 88,  // ctext
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_RECEIVED_OVERFLOWN_NONCE);

    rxPdu[4] = 0xFE;  // 1 before expired
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_SECWARN_RECEIVED_OVERFLOWN_NONCE);
    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other checks are passing
}

static void
hzlClientTest_ClientProcessReceivedResponseMsgWithValidTagSuccessfully(void)
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
    // Dummy request-sent session state
    groupStates[0].requestNonce = 0x0706050403020100ULL;
    hzl_Timestamp_t timestampOfReqTx = 0;
    hzlTest_IoMockupCurrentTimeSucceeding(&timestampOfReqTx);
    groupStates[0].lastHandshakeEventInstant = timestampOfReqTx;
    groupStates[0].currentRxLastMessageInstant = 33;
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            0,  // SID == server
            1,  // PTY == RES
            13,  // Requesting client SID
            0x33, 0x22, 0x11,  // Ctrnonce
            8, 9, 10, 11, 12, 13, 14, 15,  // Resnonce
            // Assuming a reqnonce = {0, 1, 2, 3, 4, 5, 6, 7}
            0x20, 0x80, 0x24, 0xBC, 0x7B, 0xE4, 0x24, 0xE1,
            0x85, 0xB7, 0x95, 0xFB, 0x00, 0xFA, 0x7F,
            0xB6,  // ctext, encrypting "The session key!" in ASCII encoding
            0xBE, 0x8E, 0x6D, 0x3C, 0x96, 0x56, 0x68, 0x35,
            0x5B, 0x5F, 0xE1, 0x88, 0x3A, 0xA5, 0x87, 0x7A,  // tag
    };

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
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit

    // Internal state has the session information ready
    atto_eq(groupStates[0].currentCtrNonce, 0x112233);
    atto_memeq(groupStates[0].currentStk, "The session key!", 16);

    // New timestamps are saved
    atto_gt(groupStates[0].currentRxLastMessageInstant, timestampOfReqTx);
    atto_eq(groupStates[0].lastHandshakeEventInstant, groupStates[0].currentRxLastMessageInstant);

    // Internal state clears the "waiting for response" information
    atto_eq(groupStates[0].requestNonce, 0);
}

static void
hzlClientTest_ClientProcessReceivedResponseMsgMustHaveNonZeroStkAfterDecryption(void)
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
    // Dummy request-sent session state
    groupStates[0].requestNonce = 0x0706050403020100ULL;
    hzlTest_IoMockupCurrentTimeSucceeding(&groupStates[0].lastHandshakeEventInstant);
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    size_t rxPduLen = 64;
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            0,  // SID == server
            1,  // PTY == RES
            13,  // Requesting client SID
            0x33, 0x22, 0x11,  // Ctrnonce
            8, 9, 10, 11, 12, 13, 14, 15,  // Resnonce
            // Assuming a reqnonce = {0, 1, 2, 3, 4, 5, 6, 7}
            0x74, 0xE8, 0x41, 0x9C, 0x08, 0x81, 0x57, 0x92,
            0x43, 0x62, 0xAA, 0xFD, 0xDA, 0xE5, 0x50, 0x51,  // ctext, encrypting an all-zeros
            0x57, 0x6B, 0x98, 0xF3, 0xD2, 0x85, 0xB1, 0x0D,
            0xC4, 0x4E, 0x46, 0xE8, 0x06, 0x89, 0x83, 0x0E,  // tag
    };

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    atto_eq(err, HZL_ERR_SECWARN_RECEIVED_ZERO_KEY);
}

void hzlClientTest_ClientProcessReceivedResponse(void)
{
    hzlClientTest_ClientProcessReceivedResponseMsgMustHaveKnownGid();
    hzlClientTest_ClientProcessReceivedResponseMsgMustComeFromServer();
    hzlClientTest_ClientProcessReceivedResponseMsgMustBeLongEnough();
    hzlClientTest_ClientProcessReceivedResponseMsgNotForMeIsIgnored();
    hzlClientTest_ClientProcessReceivedResponseMsgMustBeExpected();
    hzlClientTest_ClientProcessReceivedResponseMsgMustArriveWithinTimeout();
    hzlClientTest_ClientProcessReceivedResponseMsgMustHaveNonExpiredCtrnonce();
    hzlClientTest_ClientProcessReceivedResponseMsgWithValidTagSuccessfully();
    hzlClientTest_ClientProcessReceivedResponseMsgMustHaveNonZeroStkAfterDecryption();
    HZL_TEST_PARTIAL_REPORT();
}
