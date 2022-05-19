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
 * Tests of the hzl_ServerProcessReceived() function for the RES messages.
 *
 * @warning
 * REDUCING COVERAGE ON PURPOSE. NOT implementing all the testcases for all possible incorrect
 * content of the context, because they have already been checked for the hzl_ServerInit()
 * function and the inner checks are exactly the same, performed by the same internal
 * function hzl_ServerCheckCtxPointers().
 */

#include "hzlTest.h"

static void
hzlServerTest_ServerProcessReceivedResponseTriggersWarning(void)
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
            0,  // SID
            1,  // PTY == RES
            8, 9, 10, 11, 12, 13, 14, 15,  // Resnonce
            20, 21, 22, 23, 24, 25, 26, 27,  // tag (incorrect)
            28, 29, 30, 31, 32, 33, 34, 35,  // tag (incorrect)
    };

    rxPdu[1] = 0; // SID == server
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_MESSAGE_FROM_MYSELF);

    rxPdu[1] = 1; // SID != server
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_SERVER_ONLY_MESSAGE);
}

static void
hzlServerTest_ServerProcessReceivedRenewalTriggersWarning(void)
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
            0,  // SID
            0,  // PTY == REN
            8, 9, 10, 11, 12, 13, 14, 15,  // Resnonce
            20, 21, 22, 23, 24, 25, 26, 27,  // tag (incorrect)
            28, 29, 30, 31, 32, 33, 34, 35,  // tag (incorrect)
    };

    rxPdu[1] = 0; // SID == server
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_MESSAGE_FROM_MYSELF);

    rxPdu[1] = 1; // SID != server
    err = hzl_ServerProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);
    atto_eq(err, HZL_ERR_SECWARN_SERVER_ONLY_MESSAGE);
}

void hzlServerTest_ServerProcessReceivedServerOnlyMsg(void)
{
    hzlServerTest_ServerProcessReceivedResponseTriggersWarning();
    hzlServerTest_ServerProcessReceivedRenewalTriggersWarning();
    HZL_TEST_PARTIAL_REPORT();
}
