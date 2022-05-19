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
 * Tests of the hzl_ClientProcessReceived() function for the SADFD messages.
 *
 * @warning
 * REDUCING COVERAGE ON PURPOSE. NOT implementing all the testcases for all possible incorrect
 * content of the context, because they have already been checked for the hzl_ClientInit()
 * function and the inner checks are exactly the same, performed by the same internal
 * function hzl_ClientCheckCtxPointers().
 */

#include "hzlTest.h"

static void
hzlClientTest_ClientProcessReceivedSadfdMsgMustNotHaveTooLongPlaintextHeader0(void)
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
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            42,  // SID
            4,  // PTY == SADFD
            0x33, 0x22, 0x11,  // Ctrnonce
            5,  // ptlen
            11, 22, 33, 44, 55,  // ctext
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };
    size_t rxPduLen = 64;

    rxPdu[6] = 50;  // 1 more than max
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_TOO_LONG_CIPHERTEXT);

    rxPdu[6] = 49;  // Fits
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_TOO_LONG_CIPHERTEXT);
    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other checks are passing
}


static void
hzlClientTest_ClientProcessReceivedSadfdMsgMustNotHaveTooLongPlaintextHeader6(void)
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
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    uint8_t rxPdu[64] = {
            // Header 4: SID||PTY (SADFD)
            3U << 3U | 4U,
            0x33, 0x22, 0x11,  // Ctrnonce
            5,  // ptlen
            11, 22, 33, 44, 55,  // ctext
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };
    size_t rxPduLen = 64;

    rxPdu[4] = 52;  // 1 more than max
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_TOO_LONG_CIPHERTEXT);

    rxPdu[4] = 51;  // Fits
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_TOO_LONG_CIPHERTEXT);
    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other checks are passing
}

static void
hzlClientTest_ClientProcessReceivedSadfdMsgMustHaveKnownGid(void)
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
    const uint8_t rxPdu[64] = {
            // Header 0
            13,  // GID
            42,  // SID
            4,  // PTY == SADFD
            0x33, 0x22, 0x11,  // Ctrnonce
            5,  // ptlen
            11, 22, 33, 44, 55,  // ctext
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };
    size_t rxPduLen = 64;

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
}

static void
hzlClientTest_ClientProcessReceivedSadfdMsgMustBeLongEnoughForMetadata(void)
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
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            42,  // SID
            4,  // PTY == SADFD
            0x33, 0x22, 0x11,  // Ctrnonce
            0,  // ptlen
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };
    size_t rxPduLen;

    rxPduLen = 14;  // Too short by 1 to contain even the metadata
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_SADFD);

    rxPduLen = 15;  // Header0 + metadata + empty ciphertext (correct)
    // may contain some padding which should be ignored
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_SADFD);
    atto_eq(err,
            HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other size checks are passing
}

static void
hzlClientTest_ClientProcessReceivedSadfdMsgMustHaveNonExpiredCtrnonce(void)
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
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            42,  // SID
            4,  // PTY == SADFD
            0xFF, 0xFF, 0xFF,  // Ctrnonce EXPIRED
            5,  // ptlen
            11, 22, 33, 44, 55,  // ctext
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };
    size_t rxPduLen = 64;

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_RECEIVED_OVERFLOWN_NONCE);

    rxPdu[3] = 0xFE;  // 1 before expired
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_SECWARN_RECEIVED_OVERFLOWN_NONCE);
    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other checks are passing
}

static void
hzlClientTest_ClientProcessReceivedSadfdReceiverMustHaveNonZeroStk(void)
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
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            42,  // SID
            4,  // PTY == SADFD
            0x33, 0x22, 0x11,  // Ctrnonce
            5,  // ptlen
            11, 22, 33, 44, 55,  // ctext
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };
    size_t rxPduLen = 64;

    atto_zeros(ctx.groupStates[0].currentStk, 16);  // assertion for this test
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    atto_eq(err, HZL_ERR_SESSION_NOT_ESTABLISHED);
}

static void
hzlClientTest_ClientProcessReceivedSadfdReceiverMustHaveNonExpiredCtrnonce(void)
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
    // Dummy expired-session state
    groupStates[0].currentCtrNonce = 0xFFFFFF;
    groupStates[0].currentStk[0] = 99;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            42,  // SID
            4,  // PTY == SADFD
            0xFE, 0xFF, 0xFF,  // Ctrnonce, ALMOST expired, still valid
            5,  // ptlen
            11, 22, 33, 44, 55,  // ctext
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };
    size_t rxPduLen = 64;

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SESSION_NOT_ESTABLISHED);

    groupStates[0].currentCtrNonce = 0xFFFFFE;
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_SESSION_NOT_ESTABLISHED);
    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other checks are passing
}

static void
hzlClientTest_ClientProcessReceivedSadfdMsgMustHaveFreshCtrnonce(void)
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
    groupStates[0].currentCtrNonce = 0xFFFF;  // Newer ctrnonce than received one
    groupStates[0].currentStk[0] = 99;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            42,  // SID
            4,  // PTY == SADFD
            0x0A, 0x0, 0x00,  // Ctrnonce: TOO OLD
            5,  // ptlen
            11, 22, 33, 44, 55,  // ctext
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };
    size_t rxPduLen = 64;

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    atto_eq(err, HZL_ERR_SECWARN_OLD_MESSAGE);
}

static void
hzlClientTest_ClientProcessReceivedSadfdMsgWithEmptyCtextSuccessfully(void)
{
    hzl_Err_t err;
    hzl_ClientConfig_t clientConfigWithNewSid = HZL_TEST_CORRECT_CLIENT_CONFIG;
    clientConfigWithNewSid.sid = 42;  // To avoid "message from myself" error
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientCtx_t ctx = {
            .clientConfig = &clientConfigWithNewSid,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
    // Dummy established-session state
    groupStates[0].currentCtrNonce = 20;
    groupStates[0].currentStk[0] = 99;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            13,  // SID
            4,  // PTY == SADFD
            0x03, 0x02, 0x01,  // Ctrnonce
            0,  // ptlen
            // ctext: empty
            0x8D, 0x4C, 0xAB, 0x67, 0x96, 0xB9, 0xF8, 0x5E,  // Tag (correct)
    };
    size_t rxPduLen = 64;

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    atto_eq(err, HZL_OK);
    atto_eq(unpackedMsg.canId, 0xABC);
    atto_eq(unpackedMsg.dataLen, 0);
    atto_eq(unpackedMsg.gid, 0);
    atto_eq(unpackedMsg.sid, 13);
    atto_true(unpackedMsg.wasSecured);
    atto_true(unpackedMsg.isForUser);
    atto_zeros(unpackedMsg.data, 64);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit

    // New ctrnonce is stored in state, incremented
    atto_eq(groupStates[0].currentCtrNonce, 0x010203 + 1);
}

static void
hzlClientTest_ClientProcessReceivedSadfdMsgWithSomeCtextSuccessfully(void)
{
    hzl_Err_t err;
    hzl_ClientConfig_t clientConfigWithNewSid = HZL_TEST_CORRECT_CLIENT_CONFIG;
    clientConfigWithNewSid.sid = 42;  // To avoid "message from myself" error
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientCtx_t ctx = {
            .clientConfig = &clientConfigWithNewSid,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
    // Dummy established-session state
    groupStates[0].currentCtrNonce = 20;
    groupStates[0].currentStk[0] = 99;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            13,  // SID
            4,  // PTY == SADFD
            0x03, 0x02, 0x01,  // Ctrnonce
            5,  // ptlen
            0xE0, 0x04, 0xD9, 0xD9, 0x05,  // ctext: "ABCDE" in ASCII encoding
            0xAB, 0x04, 0x46, 0x61, 0x2C, 0x54, 0x37, 0x1F,  // Tag (correct)
    };
    size_t rxPduLen = 64;

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    atto_eq(err, HZL_OK);
    atto_eq(unpackedMsg.canId, 0xABC);
    atto_eq(unpackedMsg.dataLen, 5);
    atto_eq(unpackedMsg.gid, 0);
    atto_eq(unpackedMsg.sid, 13);
    atto_true(unpackedMsg.wasSecured);
    atto_true(unpackedMsg.isForUser);
    atto_memeq(unpackedMsg.data, "ABCDE", unpackedMsg.dataLen);
    atto_zeros(&unpackedMsg.data[5], 64 - 5);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit
}

static void
hzlClientTest_ClientProcessReceivedSadfdMsgMustHaveFreshCtrnonceWithinTolerance(void)
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
    groupStates[0].currentCtrNonce = 8;
    groupStates[0].currentStk[0] = 99;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzlTest_IoMockupCurrentTimeSucceeding(&groupStates[0].lastHandshakeEventInstant);
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPduWithCtrNonce5[] = {
            // Header 0
            0,  // GID
            20,  // SID
            4,  // PTY == SADFD
            0x05, 0x00, 0x00,  // Ctrnonce,
            0,  // ptlen
            // ctext: empty
            0x06, 0x8D, 0x31, 0xAC, 0x05, 0x09, 0x01, 0x1D  // tag (correct)
    };
    const uint8_t rxPduWithCtrNonce7[] = {
            // Header 0
            0,  // GID
            20,  // SID
            4,  // PTY == SADFD
            0x07, 0x00, 0x00,  // Ctrnonce,
            0,  // ptlen
            // ctext: empty
            0x98, 0x10, 0x96, 0x59, 0x7E, 0x4E, 0x22, 0x8D  // tag (correct)
    };
    const uint8_t rxPduWithCtrNonce8[] = {
            // Header 0
            0,  // GID
            20,  // SID
            4,  // PTY == SADFD
            0x08, 0x00, 0x00,  // Ctrnonce,
            0,  // ptlen
            // ctext: empty
            0xE3, 0x0C, 0x80, 0x16, 0xA6, 0x63, 0xA4, 0x22  // tag (correct)
    };
    const uint8_t rxPduWithCtrNonce10[] = {
            // Header 0
            0,  // GID
            20,  // SID
            4,  // PTY == SADFD
            0x0A, 0x00, 0x00,  // Ctrnonce,
            0,  // ptlen
            // ctext: empty
            0x3B, 0xBC, 0x88, 0x7A, 0x93, 0xB7, 0x62, 0x84  // tag (correct)
    };
    const uint8_t rxPduWithCtrNonce11[] = {
            // Header 0
            0,  // GID
            20,  // SID
            4,  // PTY == SADFD
            0x09, 0x00, 0x00,  // Ctrnonce,
            0,  // ptlen
            // ctext: empty
            0x32, 0x5F, 0x69, 0x9A, 0xE8, 0x1C, 0xC8, 0xA0  // tag (correct)
    };
    size_t rxPduLen = 16;

    // Equal to the current ctrnonce => accepted
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPduWithCtrNonce8, rxPduLen,
                                    0xABC);
    atto_eq(err, HZL_OK);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit

    atto_eq(groupStates[0].currentCtrNonce, 9);  // state ctrnonce was increased
    // Slightly older ctrnonce => still accepted
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPduWithCtrNonce8, rxPduLen,
                                    0xABC);
    atto_eq(err, HZL_OK);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit

    atto_eq(groupStates[0].currentCtrNonce, 10);  // state ctrnonce was increased
    // Too old ctrnonce compared to the max delay => rejected
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPduWithCtrNonce5, rxPduLen,
                                    0xABC);
    atto_eq(err, HZL_ERR_SECWARN_OLD_MESSAGE);

    atto_eq(groupStates[0].currentCtrNonce, 10);  // state ctrnonce was NOT increased
    // Slightly older ctrnonce => rejected, as too much time passed
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPduWithCtrNonce7, rxPduLen,
                                    0xABC);
    atto_eq(err, HZL_OK);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit

    // Virtually let too much time pass
    for (size_t i = 0; i < 500; i++)
    {
        hzlTest_IoMockupCurrentTimeSucceeding(NULL);
    }

    atto_eq(groupStates[0].currentCtrNonce, 11);  // state ctrnonce was increased
    // Slightly older ctrnonce => rejected, as too much time passed
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPduWithCtrNonce10,
                                    rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_OLD_MESSAGE);

    atto_eq(groupStates[0].currentCtrNonce, 11);  // state ctrnonce was NOT increased
    //Equal to the current ctrnonce => accepted
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPduWithCtrNonce11,
                                    rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_OLD_MESSAGE);
}

static void
hzlClientTest_ClientProcessReceivedSadfdCurrentSessionAcceptedDuringRenewal(void)
{
    hzl_Err_t err;
    hzl_ClientConfig_t clientConfigWithNewSid = HZL_TEST_CORRECT_CLIENT_CONFIG;
    clientConfigWithNewSid.sid = 42;  // To avoid "message from myself" error
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientCtx_t ctx = {
            .clientConfig = &clientConfigWithNewSid,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
    // Dummy old session state as copied after getting a REN message
    groupStates[0].previousCtrNonce = 0xF11111;
    groupStates[0].previousStk[0] = 222;
    atto_zeros(&groupStates[0].previousStk[1], 15);  // The rest is zeros
    // Dummy new session state as obtained from a RES message
    groupStates[0].currentCtrNonce = 0x010200;
    groupStates[0].currentStk[0] = 99;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    // Fake timestamps after receiving the RES that the REN msg triggered
    hzl_Timestamp_t lastHandshakeInstant = 0;
    hzlTest_IoMockupCurrentTimeSucceeding(&lastHandshakeInstant);
    groupStates[0].lastHandshakeEventInstant = lastHandshakeInstant;
    groupStates[0].currentRxLastMessageInstant = lastHandshakeInstant;
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            13,  // SID
            4,  // PTY == SADFD
            0x03, 0x02, 0x01,  // Ctrnonce
            5,  // ptlen
            0xE0, 0x04, 0xD9, 0xD9, 0x05,  // ctext: "ABCDE" in ASCII encoding
            0xAB, 0x04, 0x46, 0x61, 0x2C, 0x54, 0x37, 0x1F,  // Tag (correct with NEW STK)
    };
    size_t rxPduLen = 64;

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    // Successfully received
    atto_eq(err, HZL_OK);
    atto_eq(unpackedMsg.canId, 0xABC);
    atto_eq(unpackedMsg.dataLen, 5);
    atto_eq(unpackedMsg.gid, 0);
    atto_eq(unpackedMsg.sid, 13);
    atto_true(unpackedMsg.wasSecured);
    atto_true(unpackedMsg.isForUser);
    atto_memeq(unpackedMsg.data, "ABCDE", unpackedMsg.dataLen);
    atto_zeros(&unpackedMsg.data[5], 64 - 5);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit
}

static void
hzlClientTest_ClientProcessReceivedSadfdPreviousSessionAcceptedDuringRenewal(void)
{
    hzl_Err_t err;
    hzl_ClientConfig_t clientConfigWithNewSid = HZL_TEST_CORRECT_CLIENT_CONFIG;
    clientConfigWithNewSid.sid = 42;  // To avoid "message from myself" error
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientCtx_t ctx = {
            .clientConfig = &clientConfigWithNewSid,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
    // Dummy old session state as copied after getting a REN message
    groupStates[0].previousCtrNonce = 0x010200;
    groupStates[0].previousStk[0] = 99;
    atto_zeros(&groupStates[0].previousStk[1], 15);  // The rest is zeros
    // Dummy new session state as obtained from a RES message
    groupStates[0].currentCtrNonce = 3;
    groupStates[0].currentStk[0] = 100;
    // Fake timestamps after receiving the RES that the REN msg triggered
    hzl_Timestamp_t lastHandshakeInstant = 0;
    hzlTest_IoMockupCurrentTimeSucceeding(&lastHandshakeInstant);
    groupStates[0].lastHandshakeEventInstant = lastHandshakeInstant;
    groupStates[0].currentRxLastMessageInstant = lastHandshakeInstant;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            13,  // SID
            4,  // PTY == SADFD
            0x03, 0x02, 0x01,  // Ctrnonce
            5,  // ptlen
            0xE0, 0x04, 0xD9, 0xD9, 0x05,  // ctext: "ABCDE" in ASCII encoding
            0xAB, 0x04, 0x46, 0x61, 0x2C, 0x54, 0x37, 0x1F,  // Tag (correct with OLD STK)
    };
    size_t rxPduLen = 64;

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    // Successfully received
    atto_eq(err, HZL_OK);
    atto_eq(unpackedMsg.canId, 0xABC);
    atto_eq(unpackedMsg.dataLen, 5);
    atto_eq(unpackedMsg.gid, 0);
    atto_eq(unpackedMsg.sid, 13);
    atto_true(unpackedMsg.wasSecured);
    atto_true(unpackedMsg.isForUser);
    atto_memeq(unpackedMsg.data, "ABCDE", unpackedMsg.dataLen);
    atto_zeros(&unpackedMsg.data[5], 64 - 5);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit
}

static void
hzlClientTest_ClientProcessReceivedSadfdPreviousSessionRejectedAfterTooManyMsgs(void)
{
    hzl_Err_t err;
    hzl_ClientConfig_t clientConfigWithNewSid = HZL_TEST_CORRECT_CLIENT_CONFIG;
    clientConfigWithNewSid.sid = 42;  // To avoid "message from myself" error
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientCtx_t ctx = {
            .clientConfig = &clientConfigWithNewSid,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
    // Dummy old session state as copied after getting a REN message
    groupStates[0].previousCtrNonce = 0x010202;
    groupStates[0].previousStk[0] = 99;
    atto_zeros(&groupStates[0].previousStk[1], 15);  // The rest is zeros
    // Dummy new session state as obtained from a RES message
    groupStates[0].currentCtrNonce = 3;
    groupStates[0].currentStk[0] = 100;
    // Fake timestamps after receiving the RES that the REN msg triggered
    hzl_Timestamp_t lastHandshakeInstant = 0;
    hzlTest_IoMockupCurrentTimeSucceeding(&lastHandshakeInstant);
    groupStates[0].lastHandshakeEventInstant = lastHandshakeInstant;
    groupStates[0].currentRxLastMessageInstant = lastHandshakeInstant;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            13,  // SID
            4,  // PTY == SADFD
            0x03, 0x02, 0x01,  // Ctrnonce
            5,  // ptlen
            0xE0, 0x04, 0xD9, 0xD9, 0x05,  // ctext: "ABCDE" in ASCII encoding
            0xAB, 0x04, 0x46, 0x61, 0x2C, 0x54, 0x37, 0x1F,  // Tag (correct with OLD STK)
    };
    size_t rxPduLen = 64;

    groupStates[0].currentCtrNonce = ctx.groupConfigs[0].maxCtrnonceDelayMsgs * 2 - 1;  // still OK
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_OK);

    groupStates[0].currentCtrNonce = ctx.groupConfigs[0].maxCtrnonceDelayMsgs * 2;  // Too much
    atto_ge(groupStates[0].currentCtrNonce, ctx.groupConfigs[0].maxCtrnonceDelayMsgs * 2);
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);
    // After the ctrnonce of the current session reached the threshold, the data of the old
    // session is deleted.
    atto_eq(groupStates[0].previousCtrNonce, 0);
    atto_zeros(groupStates[0].previousStk, HZL_STK_LEN);
}

static void
hzlClientTest_ClientProcessReceivedSadfdPreviousSessionRejectedAfterTooMuchTime(void)
{
    hzl_Err_t err;
    hzl_ClientConfig_t clientConfigWithNewSid = HZL_TEST_CORRECT_CLIENT_CONFIG;
    clientConfigWithNewSid.sid = 42;  // To avoid "message from myself" error
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientCtx_t ctx = {
            .clientConfig = &clientConfigWithNewSid,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ClientInit(&ctx);
    atto_eq(err, HZL_OK);
    // Dummy old session state as copied after getting a REN message
    groupStates[0].previousCtrNonce = 0x010200;
    groupStates[0].previousStk[0] = 99;
    atto_zeros(&groupStates[0].previousStk[1], 15);  // The rest is zeros
    // Dummy new session state as obtained from a RES message
    groupStates[0].currentCtrNonce = 3;
    groupStates[0].currentStk[0] = 100;
    // Fake timestamps after receiving the RES that the REN msg triggered
    hzl_Timestamp_t lastHandshakeInstant = 0;
    hzlTest_IoMockupCurrentTimeSucceeding(&lastHandshakeInstant);
    groupStates[0].lastHandshakeEventInstant = lastHandshakeInstant;
    groupStates[0].currentRxLastMessageInstant = lastHandshakeInstant;
    atto_zeros(&groupStates[0].currentStk[1], 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            13,  // SID
            4,  // PTY == SADFD
            0x03, 0x02, 0x01,  // Ctrnonce
            5,  // ptlen
            0xE0, 0x04, 0xD9, 0xD9, 0x05,  // ctext: "ABCDE" in ASCII encoding
            0xAB, 0x04, 0x46, 0x61, 0x2C, 0x54, 0x37, 0x1F,  // Tag (correct with OLD STK)
    };
    size_t rxPduLen = 64;

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_OK);

    // Virtually let too much time pass
    for (size_t i = 0; i < 500; i++)
    {
        hzlTest_IoMockupCurrentTimeSucceeding(NULL);
    }

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);
    // After the time since the last handshake reached the threshold, the data of the old
    // session is deleted.
    atto_eq(groupStates[0].previousCtrNonce, 0);
    atto_zeros(groupStates[0].previousStk, HZL_STK_LEN);
}

void hzlClientTest_ClientProcessReceivedSecuredFd(void)
{
    hzlClientTest_ClientProcessReceivedSadfdMsgMustNotHaveTooLongPlaintextHeader0();
    hzlClientTest_ClientProcessReceivedSadfdMsgMustNotHaveTooLongPlaintextHeader6();
    hzlClientTest_ClientProcessReceivedSadfdMsgMustHaveKnownGid();
    hzlClientTest_ClientProcessReceivedSadfdMsgMustBeLongEnoughForMetadata();
    hzlClientTest_ClientProcessReceivedSadfdMsgMustHaveNonExpiredCtrnonce();
    hzlClientTest_ClientProcessReceivedSadfdReceiverMustHaveNonZeroStk();
    hzlClientTest_ClientProcessReceivedSadfdReceiverMustHaveNonExpiredCtrnonce();
    hzlClientTest_ClientProcessReceivedSadfdMsgMustHaveFreshCtrnonce();
    hzlClientTest_ClientProcessReceivedSadfdMsgWithEmptyCtextSuccessfully();
    hzlClientTest_ClientProcessReceivedSadfdMsgWithSomeCtextSuccessfully();
    hzlClientTest_ClientProcessReceivedSadfdMsgMustHaveFreshCtrnonceWithinTolerance();
    hzlClientTest_ClientProcessReceivedSadfdCurrentSessionAcceptedDuringRenewal();
    hzlClientTest_ClientProcessReceivedSadfdPreviousSessionAcceptedDuringRenewal();
    hzlClientTest_ClientProcessReceivedSadfdPreviousSessionRejectedAfterTooManyMsgs();
    hzlClientTest_ClientProcessReceivedSadfdPreviousSessionRejectedAfterTooMuchTime();
    HZL_TEST_PARTIAL_REPORT();
}
