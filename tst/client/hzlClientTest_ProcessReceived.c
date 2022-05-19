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
 * Tests of the hzl_ClientProcessReceived() function, generic part.
 *
 * @warning
 * REDUCING COVERAGE ON PURPOSE. NOT implementing all the testcases for all possible incorrect
 * content of the context, because they have already been checked for the hzl_ClientInit()
 * function and the inner checks are exactly the same, performed by the same internal
 * function hzl_ClientCheckCtxPointers().
 */

#include "hzlTest.h"

static void
hzlClientTest_ClientProcessReceivedMsgToTxMustNotBeNull(void)
{
    hzl_Err_t err;

    err = hzl_ClientProcessReceived(NULL, NULL, NULL, NULL, 0, 0xABC);

    atto_eq(err, HZL_ERR_NULL_PDU);
}

static void
hzlClientTest_ClientProcessReceivedUnpackedMsgMustNotBeNull(void)
{
    hzl_Err_t err;
    hzl_CbsPduMsg_t msgToTx = {0};

    err = hzl_ClientProcessReceived(&msgToTx, NULL, NULL, NULL, 0, 0xABC);

    atto_eq(err, HZL_ERR_NULL_SDU);
}

static void
hzlClientTest_ClientProcessReceivedCtxMustNotBeNull(void)
{
    hzl_Err_t err;
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, NULL, NULL, 0, 0xABC);

    atto_eq(err, HZL_ERR_NULL_CTX);
}

// NOTE: tests for all possible issues WITHIN the context are skipped. See warning in file header.

static void
hzlClientTest_ClientProcessReceivedRxDataMustNotBeNullWhenPositiveDataLen(void)
{
    hzl_Err_t err;
    hzl_ClientGroupState_t groupStates[HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS];
    hzl_ClientCtx_t ctx = {
            .clientConfig = &HZL_TEST_CORRECT_CLIENT_CONFIG,
            .groupConfigs = HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS,
            .groupStates = groupStates,
            .io = HZL_TEST_CORRECT_IO,
    };
    hzl_CbsPduMsg_t msgToTx = {0};
    hzl_RxSduMsg_t unpackedMsg = {0};

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, NULL, 1, 0xABC);
    atto_eq(err, HZL_ERR_NULL_PDU);

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, NULL, 0, 0xABC);
    atto_neq(err, HZL_ERR_NULL_PDU);  // Different error
}

static void
hzlClientTest_ClientProcessReceivedMsgMustHaveEnoughDataLenForCbsHeader0(void)
{
    hzl_Err_t err;
    hzl_ClientConfig_t clientConfigWithNewHeaderType =
            HZL_TEST_CORRECT_CLIENT_CONFIG;
    clientConfigWithNewHeaderType.headerType = HZL_HEADER_0;  // Header = 3 bytes
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
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {0, 42, 5, 0xFF};
    size_t rxPduLen;

    rxPduLen = 2;  // Too short
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_HEADER);

    rxPduLen = 3;  // Minimum
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_OK);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit

    rxPduLen = 4;  // > Minimum
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_OK);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit
}

static void
hzlClientTest_ClientProcessReceivedMsgMustHaveEnoughDataLenForCbsHeader4(void)
{
    hzl_Err_t err;
    hzl_ClientConfig_t clientConfigWithNewHeaderType =
            HZL_TEST_CORRECT_CLIENT_CONFIG;
    clientConfigWithNewHeaderType.headerType = HZL_HEADER_4;  // Header = 1 bytes
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
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {3U << 5U | 0U | 5U, 0xFF};  // Unsecured Application Data msg
    size_t rxPduLen;

    rxPduLen = 0;  // Too short
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_HEADER);

    rxPduLen = 1;  // Minimum
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_OK);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit

    rxPduLen = 4;  // > Minimum
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_OK);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit
}

static void
hzlClientTest_ClientProcessReceivedMsgMustHaveKnownPtyField(void)
{
    hzl_Err_t err;
    hzl_ClientConfig_t clientConfigWithNewHeaderType =
            HZL_TEST_CORRECT_CLIENT_CONFIG;
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
    hzl_RxSduMsg_t unpackedMsg = {0};
    uint8_t rxPdu[64] = {0, 42, 5, 0xFF};  // Unsecured Application Data msg
    size_t rxPduLen = 4;

    rxPdu[2] = 6;  // PTY > Max
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_INVALID_PAYLOAD_TYPE);

    rxPdu[2] = 5;  // PTY == Max
    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_OK);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit
}

static void
hzlClientTest_ClientProcessReceivedMsgMustNotHaveReceiversSid(void)
{
    hzl_Err_t err;
    hzl_ClientConfig_t clientConfigWithNewHeaderType =
            HZL_TEST_CORRECT_CLIENT_CONFIG;
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
    hzl_RxSduMsg_t unpackedMsg = {0};
    const uint8_t rxPdu[64] = {
            0, clientConfigWithNewHeaderType.sid, 5,
            0xFF
    };  // Unsecured Application Data msg
    size_t rxPduLen = 4;

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    atto_eq(err, HZL_ERR_SECWARN_MESSAGE_FROM_MYSELF);
}

void hzlClientTest_ClientProcessReceived(void)
{
    hzlClientTest_ClientProcessReceivedMsgToTxMustNotBeNull();
    hzlClientTest_ClientProcessReceivedUnpackedMsgMustNotBeNull();
    hzlClientTest_ClientProcessReceivedCtxMustNotBeNull();
    hzlClientTest_ClientProcessReceivedRxDataMustNotBeNullWhenPositiveDataLen();
    hzlClientTest_ClientProcessReceivedMsgMustHaveEnoughDataLenForCbsHeader0();
    hzlClientTest_ClientProcessReceivedMsgMustHaveEnoughDataLenForCbsHeader4();
    hzlClientTest_ClientProcessReceivedMsgMustHaveKnownPtyField();
    hzlClientTest_ClientProcessReceivedMsgMustNotHaveReceiversSid();
    HZL_TEST_PARTIAL_REPORT();
}
