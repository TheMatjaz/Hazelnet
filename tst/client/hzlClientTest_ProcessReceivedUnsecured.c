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
 * Tests of the hzl_ClientProcessReceived() function for the UAD messages.
 *
 * @warning
 * REDUCING COVERAGE ON PURPOSE. NOT implementing all the testcases for all possible incorrect
 * content of the context, because they have already been checked for the hzl_ClientInit()
 * function and the inner checks are exactly the same, performed by the same internal
 * function hzl_ClientCheckCtxPointers().
 */

#include "hzlTest.h"

static void
hzlClientTest_ClientProcessReceivedUadMsgSuccessfully(void)
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
    uint8_t rxPdu[64] = {0, 42, 5, 11, 22, 33, 44};  // Unsecured Application Data msg
    size_t rxPduLen = 7;

    err = hzl_ClientProcessReceived(&msgToTx, &unpackedMsg, &ctx, rxPdu, rxPduLen, 0xABC);

    atto_eq(err, HZL_OK);
    atto_eq(unpackedMsg.canId, 0xABC);
    atto_eq(unpackedMsg.dataLen, 4);
    atto_eq(unpackedMsg.gid, 0);
    atto_eq(unpackedMsg.sid, 42);
    atto_false(unpackedMsg.wasSecured);
    atto_true(unpackedMsg.isForUser);
    atto_eq(unpackedMsg.data[0], 11);
    atto_eq(unpackedMsg.data[1], 22);
    atto_eq(unpackedMsg.data[2], 33);
    atto_eq(unpackedMsg.data[3], 44);
    atto_zeros(&msgToTx, sizeof(hzl_CbsPduMsg_t)); // No msg to transmit
}

void hzlClientTest_ClientProcessReceivedUnsecured(void)
{
    hzlClientTest_ClientProcessReceivedUadMsgSuccessfully();
    HZL_TEST_PARTIAL_REPORT();
}
