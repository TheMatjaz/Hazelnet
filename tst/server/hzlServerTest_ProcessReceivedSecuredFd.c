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
 * Tests of the hzl_ServerProcessReceived() function for the SADFD messages.
 *
 * @warning
 * REDUCING COVERAGE ON PURPOSE. NOT implementing all the testcases for all possible incorrect
 * content of the context, because they have already been checked for the hzl_ServerInit()
 * function and the inner checks are exactly the same, performed by the same internal
 * function hzl_ServerCheckCtxPointers().
 */

#include "hzlTest.h"

static void
hzlServerTest_ServerProcessReceivedSadfdMsgMustNotHaveTooLongPlaintextHeader0(void)
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
    // Dummy established-session state
    groupStates[0].currentCtrNonce = 0x112233;
    groupStates[0].currentStk[0] = 99;
    memset(&groupStates[0].currentStk[1], 0, 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            1,  // SID
            4,  // PTY == SADFD
            0x33, 0x22, 0x11,  // Ctrnonce
            5,  // ptlen
            11, 22, 33, 44, 55,  // ctext
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };
    size_t rxPduLen = 64;

    rxPdu[6] = 50;  // 1 more than max
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_TOO_LONG_CIPHERTEXT);

    rxPdu[6] = 49;  // Fits
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_TOO_LONG_CIPHERTEXT);
    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other checks are passing
}


static void
hzlServerTest_ServerProcessReceivedSadfdMsgMustNotHaveTooLongPlaintextHeader6(void)
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
    // Dummy established-session state
    groupStates[0].currentCtrNonce = 0x112233;
    groupStates[0].currentStk[0] = 99;
    memset(&groupStates[0].currentStk[1], 0, 15);  // The rest is zeros
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
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_TOO_LONG_CIPHERTEXT);

    rxPdu[4] = 51;  // Fits
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_TOO_LONG_CIPHERTEXT);
    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other checks are passing
}

static void
hzlServerTest_ServerProcessReceivedSadfdMsgMustHaveKnownGid(void)
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
    const uint8_t rxPdu[64] = {
            // Header 0
            13,  // GID
            1,  // SID
            4,  // PTY == SADFD
            0x33, 0x22, 0x11,  // Ctrnonce
            5,  // ptlen
            11, 22, 33, 44, 55,  // ctext
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };
    size_t rxPduLen = 64;

    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_UNKNOWN_GROUP);
}

static void
hzlServerTest_ServerProcessReceivedSadfdMsgMustHaveKnownSid(void)
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
    uint8_t rxPdu[64] = {
            // Header 0
            2,  // GID
            0,  // SID
            4,  // PTY == SADFD
            0x33, 0x22, 0x11,  // Ctrnonce
            5,  // ptlen
            11, 22, 33, 44, 55,  // ctext
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };
    size_t rxPduLen = 64;

    rxPdu[1] = 60; // SID out of bounds
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_UNKNOWN_SOURCE);

    rxPdu[1] = 1; // SID not in the group with GID==2
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_NOT_IN_GROUP);
}

static void
hzlServerTest_ServerProcessReceivedSadfdMsgMustBeLongEnoughForMetadata(void)
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
    // Dummy established-session state
    groupStates[0].currentCtrNonce = 1;
    groupStates[0].currentStk[0] = 99;
    memset(&groupStates[0].currentStk[1], 0, 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            1,  // SID
            4,  // PTY == SADFD
            0x33, 0x22, 0x11,  // Ctrnonce
            0,  // ptlen
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };
    size_t rxPduLen;

    rxPduLen = 14;  // Too short by 1 to contain even the metadata
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_SADFD);

    rxPduLen = 15;  // Header0 + metadata + empty ciphertext (correct)
    // may contain some padding which should be ignored
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_SADFD);
    atto_eq(err,
            HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other size checks are passing
}

static void
hzlServerTest_ServerProcessReceivedSadfdMsgMustHaveNonExpiredCtrnonce(void)
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
    // Dummy established-session state
    groupStates[0].currentCtrNonce = 1;
    groupStates[0].currentStk[0] = 99;
    memset(&groupStates[0].currentStk[1], 0, 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            1,  // SID
            4,  // PTY == SADFD
            0xFF, 0xFF, 0xFF,  // Ctrnonce EXPIRED
            5,  // ptlen
            11, 22, 33, 44, 55,  // ctext
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };
    size_t rxPduLen = 64;

    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_RECEIVED_OVERFLOWN_NONCE);

    rxPdu[3] = 0xFE;  // 1 before expired
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_neq(err, HZL_ERR_SECWARN_RECEIVED_OVERFLOWN_NONCE);
    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);  // Tag is wrong, but other checks are passing
}

static void
hzlServerTest_ServerProcessReceivedSadfdMsgMustHaveFreshCtrnonce(void)
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
    // Dummy established-session state
    groupStates[0].currentCtrNonce = 0xFFFF;  // Newer ctrnonce than received one
    groupStates[0].currentStk[0] = 99;
    memset(&groupStates[0].currentStk[1], 0, 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            1,  // SID
            4,  // PTY == SADFD
            0x0A, 0x0, 0x00,  // Ctrnonce: TOO OLD
            5,  // ptlen
            11, 22, 33, 44, 55,  // ctext
            20, 21, 22, 23, 24, 25, 26, 27  // tag (incorrect)
    };
    size_t rxPduLen = 64;

    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    atto_eq(err, HZL_ERR_SECWARN_OLD_MESSAGE);
}

static void
hzlServerTest_ServerProcessReceivedSadfdMsgWithEmptyCtextSuccessfully(void)
{
    hzl_Err_t err;
    hzl_ServerConfig_t serverConfigWithNewSid = HZL_TEST_CORRECT_SERVER_CONFIG;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &serverConfigWithNewSid,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
    // Dummy established-session state
    groupStates[0].currentCtrNonce = 20;
    groupStates[0].currentStk[0] = 99;
    memset(&groupStates[0].currentStk[1], 0, 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            1,  // SID
            4,  // PTY == SADFD
            0x03, 0x02, 0x01,  // Ctrnonce
            0,  // ptlen
            // ctext: empty
            0x3E, 0x13, 0x47, 0xEF, 0x13, 0x8E, 0x2B, 0x30,  // Tag (correct)
    };
    size_t rxPduLen = 64;

    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    atto_eq(err, HZL_OK);
    atto_eq(unpackedMsg.canId, 0xABC);
    atto_eq(unpackedMsg.dataLen, 0);
    atto_eq(unpackedMsg.gid, 0);
    atto_eq(unpackedMsg.sid, 1);
    atto_true(unpackedMsg.wasSecured);
    atto_true(unpackedMsg.isForUser);
    atto_zeros(unpackedMsg.data, 64);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit

    // New ctrnonce is stored in state, incremented
    atto_eq(groupStates[0].currentCtrNonce, 0x010203 + 1);
}

static void
hzlServerTest_ServerProcessReceivedSadfdMsgWithSomeCtextSuccessfully(void)
{
    hzl_Err_t err;
    hzl_ServerConfig_t serverConfigWithNewSid = HZL_TEST_CORRECT_SERVER_CONFIG;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &serverConfigWithNewSid,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
    // Dummy established-session state
    groupStates[0].currentCtrNonce = 20;
    groupStates[0].currentStk[0] = 99;
    memset(&groupStates[0].currentStk[1], 0, 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            1,  // SID
            4,  // PTY == SADFD
            0x03, 0x02, 0x01,  // Ctrnonce
            5,  // ptlen
            0x1D, 0x5A, 0x14, 0x41, 0x8F,  // ctext: "ABCDE" in ASCII encoding
            0xFA, 0x4F, 0x11, 0x4C, 0xF3, 0x33, 0x99, 0xD7,  // Tag (correct)
    };
    size_t rxPduLen = 64;

    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    atto_eq(err, HZL_OK);
    atto_eq(unpackedMsg.canId, 0xABC);
    atto_eq(unpackedMsg.dataLen, 5);
    atto_eq(unpackedMsg.gid, 0);
    atto_eq(unpackedMsg.sid, 1);
    atto_true(unpackedMsg.wasSecured);
    atto_true(unpackedMsg.isForUser);
    atto_memeq(unpackedMsg.data, "ABCDE", unpackedMsg.dataLen);
    atto_zeros(&unpackedMsg.data[5], 64 - 5);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit
}

static void
hzlServerTest_ServerProcessReceivedSadfdMsgMustHaveFreshCtrnonceWithinTolerance(void)
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
    // Dummy established-session state
    groupStates[0].currentCtrNonce = 8;
    groupStates[0].currentStk[0] = 99;
    memset(&groupStates[0].currentStk[1], 0, 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPduWithCtrNonce5[] = {
            // Header 0
            0,  // GID
            1,  // SID
            4,  // PTY == SADFD
            0x05, 0x00, 0x00,  // Ctrnonce,
            0,  // ptlen
            // ctext: empty
            0x73, 0xBF, 0x48, 0x5B, 0xCF, 0x02, 0x53, 0x8A  // tag (correct)
    };
    const uint8_t rxPduWithCtrNonce7[] = {
            // Header 0
            0,  // GID
            1,  // SID
            4,  // PTY == SADFD
            0x07, 0x00, 0x00,  // Ctrnonce,
            0,  // ptlen
            // ctext: empty
            0x7B, 0xC9, 0x0E, 0x80, 0xA5, 0xA4, 0x7D, 0xEC  // tag (correct)
    };
    const uint8_t rxPduWithCtrNonce8[] = {
            // Header 0
            0,  // GID
            1,  // SID
            4,  // PTY == SADFD
            0x08, 0x00, 0x00,  // Ctrnonce,
            0,  // ptlen
            // ctext: empty
            0xB8, 0xCF, 0xEC, 0x07, 0x90, 0x95, 0x5D, 0x32  // tag (correct)
    };
    const uint8_t rxPduWithCtrNonce10[] = {
            // Header 0
            0,  // GID
            1,  // SID
            4,  // PTY == SADFD
            0x0A, 0x00, 0x00,  // Ctrnonce,
            0,  // ptlen
            // ctext: empty
            0xA6, 0x46, 0x69, 0xAE, 0x52, 0x2F, 0xD5, 0x5D  // tag (correct)
    };
    const uint8_t rxPduWithCtrNonce11[] = {
            // Header 0
            0,  // GID
            1,  // SID
            4,  // PTY == SADFD
            0x09, 0x00, 0x00,  // Ctrnonce,
            0,  // ptlen
            // ctext: empty
            0x95, 0x4D, 0x2E, 0xD2, 0xE0, 0x27, 0x41, 0x94  // tag (correct)
    };
    size_t rxPduLen = 16;

    // Equal to the current ctrnonce => accepted
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPduWithCtrNonce8, rxPduLen,
                                    0xABC);
    atto_eq(err, HZL_OK);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit

    atto_eq(groupStates[0].currentCtrNonce, 9);  // state ctrnonce was increased
    // Slightly older ctrnonce => still accepted
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPduWithCtrNonce8, rxPduLen,
                                    0xABC);
    atto_eq(err, HZL_OK);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit

    atto_eq(groupStates[0].currentCtrNonce, 10);  // state ctrnonce was increased
    // Too old ctrnonce compared to the max delay => rejected
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPduWithCtrNonce5, rxPduLen,
                                    0xABC);
    atto_eq(err, HZL_ERR_SECWARN_OLD_MESSAGE);

    atto_eq(groupStates[0].currentCtrNonce, 10);  // state ctrnonce was NOT increased
    // Slightly older ctrnonce => rejected, as too much time passed
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPduWithCtrNonce7, rxPduLen,
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
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPduWithCtrNonce10,
                                    rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_OLD_MESSAGE);

    atto_eq(groupStates[0].currentCtrNonce, 11);  // state ctrnonce was NOT increased
    //Equal to the current ctrnonce => accepted
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPduWithCtrNonce11,
                                    rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_OLD_MESSAGE);
}

static void
hzlServerTest_ServerProcessReceivedSadfdCurrentSessionAcceptedDuringRenewal(void)
{
    hzl_Err_t err;
    hzl_ServerConfig_t serverConfigWithNewSid = HZL_TEST_CORRECT_SERVER_CONFIG;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &serverConfigWithNewSid,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
    // Dummy old session state
    groupStates[0].previousCtrNonce = 0xF11111;
    groupStates[0].previousStk[0] = 222;
    memset(&groupStates[0].previousStk[1], 0, 15);  // The rest is zeros
    // Dummy new session state
    groupStates[0].currentCtrNonce = 0x010200;
    groupStates[0].currentStk[0] = 99;
    memset(&groupStates[0].currentStk[1], 0, 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            1,  // SID
            4,  // PTY == SADFD
            0x03, 0x02, 0x01,  // Ctrnonce
            5,  // ptlen
            0x1D, 0x5A, 0x14, 0x41, 0x8F,  // ctext: "ABCDE" in ASCII encoding
            0xFA, 0x4F, 0x11, 0x4C, 0xF3, 0x33, 0x99, 0xD7,  // Tag (correct)
    };
    size_t rxPduLen = 64;

    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    // Successfully received
    atto_eq(err, HZL_OK);
    atto_eq(unpackedMsg.canId, 0xABC);
    atto_eq(unpackedMsg.dataLen, 5);
    atto_eq(unpackedMsg.gid, 0);
    atto_eq(unpackedMsg.sid, 1);
    atto_true(unpackedMsg.wasSecured);
    atto_true(unpackedMsg.isForUser);
    atto_memeq(unpackedMsg.data, "ABCDE", unpackedMsg.dataLen);
    atto_zeros(&unpackedMsg.data[5], 64 - 5);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit
}

static void
hzlServerTest_ServerProcessReceivedSadfdPreviousSessionAcceptedDuringRenewal(void)
{
    hzl_Err_t err;
    hzl_ServerConfig_t serverConfigWithNewSid = HZL_TEST_CORRECT_SERVER_CONFIG;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &serverConfigWithNewSid,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
    // Dummy old session state
    groupStates[0].previousCtrNonce = 0x010200;
    groupStates[0].previousStk[0] = 99;
    memset(&groupStates[0].previousStk[1], 0, 15);  // The rest is zeros
    // Dummy new session state
    groupStates[0].currentCtrNonce = 3;
    groupStates[0].currentStk[0] = 100;
    memset(&groupStates[0].currentStk[1], 0, 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            1,  // SID
            4,  // PTY == SADFD
            0x03, 0x02, 0x01,  // Ctrnonce
            5,  // ptlen
            0x1D, 0x5A, 0x14, 0x41, 0x8F,  // ctext: "ABCDE" in ASCII encoding
            0xFA, 0x4F, 0x11, 0x4C, 0xF3, 0x33, 0x99, 0xD7,  // Tag (correct with OLD STK)
    };
    size_t rxPduLen = 64;

    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    // Successfully received
    atto_eq(err, HZL_OK);
    atto_eq(unpackedMsg.canId, 0xABC);
    atto_eq(unpackedMsg.dataLen, 5);
    atto_eq(unpackedMsg.gid, 0);
    atto_eq(unpackedMsg.sid, 1);
    atto_true(unpackedMsg.wasSecured);
    atto_true(unpackedMsg.isForUser);
    atto_memeq(unpackedMsg.data, "ABCDE", unpackedMsg.dataLen);
    atto_zeros(&unpackedMsg.data[5], 64 - 5);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit
}

static void
hzlServerTest_ServerProcessReceivedSadfdPreviousSessionRejectedAfterTooManyMsgs(void)
{
    hzl_Err_t err;
    hzl_ServerConfig_t serverConfigWithNewSid = HZL_TEST_CORRECT_SERVER_CONFIG;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &serverConfigWithNewSid,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
    // Dummy old session state
    groupStates[0].previousCtrNonce = 0x010202;
    groupStates[0].previousStk[0] = 99;
    memset(&groupStates[0].previousStk[1], 0, 15);  // The rest is zeros
    // Dummy new session state
    groupStates[0].currentCtrNonce = 3;
    groupStates[0].currentStk[0] = 100;
    memset(&groupStates[0].currentStk[1], 0, 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            1,  // SID
            4,  // PTY == SADFD
            0x03, 0x02, 0x01,  // Ctrnonce
            5,  // ptlen
            0x1D, 0x5A, 0x14, 0x41, 0x8F,  // ctext: "ABCDE" in ASCII encoding
            0xFA, 0x4F, 0x11, 0x4C, 0xF3, 0x33, 0x99, 0xD7,  // Tag (correct)
    };
    size_t rxPduLen = 64;

    groupStates[0].currentCtrNonce = ctx.groupConfigs[0].maxCtrnonceDelayMsgs * 2 - 1;  // still OK
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_OK);

    groupStates[0].currentCtrNonce = ctx.groupConfigs[0].maxCtrnonceDelayMsgs * 2;  // Too much
    atto_ge(groupStates[0].currentCtrNonce, ctx.groupConfigs[0].maxCtrnonceDelayMsgs * 2);
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);
    // After the ctrnonce of the current session reached the threshold, the data of the old
    // session is deleted.
    atto_eq(groupStates[0].previousCtrNonce, 0);
    atto_zeros(groupStates[0].previousStk, HZL_STK_LEN);
}

static void
hzlServerTest_ServerProcessReceivedSadfdPreviousSessionRejectedAfterTooMuchTime(void)
{
    hzl_Err_t err;
    hzl_ServerConfig_t serverConfigWithNewSid = HZL_TEST_CORRECT_SERVER_CONFIG;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &serverConfigWithNewSid,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
    // Dummy old session state
    groupStates[0].previousCtrNonce = 0x010200;
    groupStates[0].previousStk[0] = 99;
    memset(&groupStates[0].previousStk[1], 0, 15);  // The rest is zeros
    // Dummy new session state
    groupStates[0].currentCtrNonce = 3;
    groupStates[0].currentStk[0] = 100;
    memset(&groupStates[0].currentStk[1], 0, 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            1,  // SID
            4,  // PTY == SADFD
            0x03, 0x02, 0x01,  // Ctrnonce
            5,  // ptlen
            0x1D, 0x5A, 0x14, 0x41, 0x8F,  // ctext: "ABCDE" in ASCII encoding
            0xFA, 0x4F, 0x11, 0x4C, 0xF3, 0x33, 0x99, 0xD7,  // Tag (correct)
    };
    size_t rxPduLen = 64;

    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_OK);

    // Virtually let too much time pass
    for (size_t i = 0; i < 500; i++)
    {
        hzlTest_IoMockupCurrentTimeSucceeding(NULL);
    }

    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);
    // After the time since the last handshake reached the threshold, the data of the old
    // session is deleted.
    atto_eq(groupStates[0].previousCtrNonce, 0);
    atto_zeros(groupStates[0].previousStk, HZL_STK_LEN);
}

static void
hzlServerTest_ServerProcessReceivedSadfdTriggersRenewalWhenCtrNonceHitsLimit(void)
{
    hzl_Err_t err;
    hzl_ServerConfig_t serverConfigWithNewSid = HZL_TEST_CORRECT_SERVER_CONFIG;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &serverConfigWithNewSid,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
    // Dummy session state, about to expire
    groupStates[0].currentCtrNonce = ctx.groupConfigs->ctrNonceUpperLimit - 1;
    groupStates[0].currentStk[0] = 99;
    memset(&groupStates[0].currentStk[1], 0, 15);  // The rest is zeros
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {
            // Header 0
            0,  // GID
            1,  // SID
            4,  // PTY == SADFD
            0xFF, 0xFF, 0xFE,  // Ctrnonce, 1 before expiration
            0,  // ptlen
            0x36, 0xF7, 0xFB, 0x86, 0x70, 0xBC, 0x4D, 0x9D,  // Tag (correct)
    };
    size_t rxPduLen = 64;

    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    atto_eq(err, HZL_OK);
    // Session was renewed
    atto_neq(groupStates[0].currentStk[0], 99);
    atto_eq(groupStates[0].previousStk[0], 99);
    // Greater by 1 because it was incremented after building the REN
    atto_eq(groupStates[0].previousCtrNonce, 0xFF0001);
    atto_eq(groupStates[0].currentCtrNonce, 0);
    // REN message available
    atto_eq(msgToTx.dataLen, 3 + 19);
    // Packed header 0
    atto_eq(msgToTx.data[0], 0);  // GID
    atto_eq(msgToTx.data[1], 0);  // SID from server
    atto_eq(msgToTx.data[2], 0);  // PTY REN
    // CBS SADFD-Payload
    atto_eq(msgToTx.data[3], 0x00);  // Ctrnonce low
    atto_eq(msgToTx.data[4], 0x00);  // Ctrnonce mid
    atto_eq(msgToTx.data[5], 0xFF);  // Ctrnonce high
    // Tag
    const uint8_t expectedTag[16] = {
            0x95, 0x91, 0xAC, 0xFD, 0xED, 0x53, 0xE1, 0x39,
            0x0E, 0x10, 0x22, 0xFC, 0x9E, 0xC2, 0x56, 0x80,
    };
    atto_memeq(&msgToTx.data[6], expectedTag, 16);
}

static void
hzlServerTest_ServerProcessReceivedSadfdTriggersRenewalWhenTooMuchTimePassed(void)
{
    hzl_Err_t err;
    hzl_ServerConfig_t serverConfigWithNewSid = HZL_TEST_CORRECT_SERVER_CONFIG;
    hzl_ServerGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ServerCtx_t ctx = {
            .serverConfig = &serverConfigWithNewSid,
            .clientConfigs = HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS,
            .groupConfigs = HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    err = hzl_ServerInit(&ctx);
    atto_eq(err, HZL_OK);
    // Dummy session state
    groupStates[0].currentCtrNonce = 0x010203;
    groupStates[0].currentStk[0] = 99;
    memset(&groupStates[0].currentStk[1], 0, 15);  // The rest is zeros
    // Setting the Session start to NOW
    hzlTest_IoMockupCurrentTimeSucceeding(&groupStates[0].sessionStartInstant);
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu1[64] = {
            // Header 0
            0,  // GID
            1,  // SID
            4,  // PTY == SADFD
            0x03, 0x02, 0x01,  // ctrnonce
            0,  // ptlen
            0x3E, 0x13, 0x47, 0xEF, 0x13, 0x8E, 0x2B, 0x30,  // Tag (correct)
    };
    size_t rxPduLen = 64;

    // Right now the session works as it should
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu1, rxPduLen, 0xABC);
    atto_eq(err, HZL_OK);
    // Session was NOT renewed
    atto_eq(groupStates[0].currentStk[0], 99);
    atto_neq(groupStates[0].previousStk[0], 99);
    atto_eq(groupStates[0].previousCtrNonce, 0);
    atto_eq(groupStates[0].currentCtrNonce, 0x010204);

    // Making the session expire in time: let virtually too much time pass
    for (size_t i = 0; i < 700; i++)
    {
        hzlTest_IoMockupCurrentTimeSucceeding(NULL);
    }

    // We need a second message with the exactly fresh nonce because ctrdelay() will
    // reject it otherwise
    const uint8_t rxPdu2[64] = {
            // Header 0
            0,  // GID
            1,  // SID
            4,  // PTY == SADFD
            0x04, 0x02, 0x01,  // ctrnonce
            0,  // ptlen
            0x26, 0xEC, 0xC9, 0x6C, 0x6F, 0xDD, 0x9A, 0x9D,  // Tag (correct)
    };
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu2, rxPduLen, 0xABC);
    atto_eq(err, HZL_OK);
    // Session was renewed
    atto_neq(groupStates[0].currentStk[0], 99);
    atto_eq(groupStates[0].previousStk[0], 99);
    // Greater by 1 because it was incremented after building the REN
    atto_eq(groupStates[0].previousCtrNonce, 0x010206);
    atto_eq(groupStates[0].currentCtrNonce, 0);
    // REN message available
    atto_eq(msgToTx.dataLen, 3 + 19);
    // Packed header 0
    atto_eq(msgToTx.data[0], 0);  // GID
    atto_eq(msgToTx.data[1], 0);  // SID from server
    atto_eq(msgToTx.data[2], 0);  // PTY REN
    // CBS SADFD-Payload
    atto_eq(msgToTx.data[3], 0x05);  // Ctrnonce low
    atto_eq(msgToTx.data[4], 0x02);  // Ctrnonce mid
    atto_eq(msgToTx.data[5], 0x01);  // Ctrnonce high
    // Tag
    const uint8_t expectedTag[16] = {
            0xDF, 0xFC, 0xDB, 0x2E, 0x93, 0x72, 0x57, 0x55,
            0x5B, 0xEE, 0x24, 0xAC, 0xA6, 0x7A, 0x19, 0xCC,
    };
    atto_memeq(&msgToTx.data[6], expectedTag, 16);
}

void hzlServerTest_ServerProcessReceivedSecuredFd(void)
{
    hzlServerTest_ServerProcessReceivedSadfdMsgMustNotHaveTooLongPlaintextHeader0();
    hzlServerTest_ServerProcessReceivedSadfdMsgMustNotHaveTooLongPlaintextHeader6();
    hzlServerTest_ServerProcessReceivedSadfdMsgMustHaveKnownGid();
    hzlServerTest_ServerProcessReceivedSadfdMsgMustHaveKnownSid();
    hzlServerTest_ServerProcessReceivedSadfdMsgMustBeLongEnoughForMetadata();
    hzlServerTest_ServerProcessReceivedSadfdMsgMustHaveNonExpiredCtrnonce();
    hzlServerTest_ServerProcessReceivedSadfdMsgMustHaveFreshCtrnonce();
    hzlServerTest_ServerProcessReceivedSadfdMsgWithEmptyCtextSuccessfully();
    hzlServerTest_ServerProcessReceivedSadfdMsgWithSomeCtextSuccessfully();
    hzlServerTest_ServerProcessReceivedSadfdMsgMustHaveFreshCtrnonceWithinTolerance();
    hzlServerTest_ServerProcessReceivedSadfdCurrentSessionAcceptedDuringRenewal();
    hzlServerTest_ServerProcessReceivedSadfdPreviousSessionAcceptedDuringRenewal();
    hzlServerTest_ServerProcessReceivedSadfdPreviousSessionRejectedAfterTooManyMsgs();
    hzlServerTest_ServerProcessReceivedSadfdPreviousSessionRejectedAfterTooMuchTime();
    hzlServerTest_ServerProcessReceivedSadfdTriggersRenewalWhenCtrNonceHitsLimit();
    hzlServerTest_ServerProcessReceivedSadfdTriggersRenewalWhenTooMuchTimePassed();
    HZL_TEST_PARTIAL_REPORT();
}
