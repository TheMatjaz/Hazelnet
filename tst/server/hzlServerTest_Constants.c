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
 * Constants used across the test suite, including correct configurations,
 * easy to change to a broken one if needed.
 */

#include "hzlTest.h"

const hzl_ServerConfig_t HZL_TEST_CORRECT_SERVER_CONFIG = {
        .amountOfGroups = HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS,
        .amountOfClients = HZL_MAX_TEST_AMOUNT_OF_CLIENTS,
        .headerType = HZL_HEADER_0,
};

const hzl_ServerClientConfig_t
        HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS[HZL_MAX_TEST_AMOUNT_OF_CLIENTS] = {
        [0]={
                .ltk = {1},// Not all-zeros
                .sid = 1,
        },
        [1]={
                .ltk = {2},// Not all-zeros
                .sid = 2,
        },
        // Larger than HZL_DEFAULT_TEST_AMOUNT_OF_CLIENTS on purpose,
        // allowing some test cases to use more Clients than default.
        [2]={
                .ltk = {3},// Not all-zeros
                .sid = 3,
        },
        [3]={
                .ltk = {4},// Not all-zeros
                .sid = 4,
        },
};

const hzl_ServerGroupConfig_t
        HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS[HZL_MAX_TEST_AMOUNT_OF_GROUPS] = {
        [0]={
                .gid = 0,
                .maxCtrnonceDelayMsgs = 4,
                .ctrNonceUpperLimit = 0xFF0000,
                .sessionDurationMillis = 50000, // Shorter on purpose to simplify expiration tests
                .delayBetweenRenNotificationsMillis = 4000,
                .clientSidsInGroupBitmap = 0xFFFFFFFFU,  // Broadcast
                .maxSilenceIntervalMillis = 5000,
        },
        [1]={
                .gid = 1,
                .maxCtrnonceDelayMsgs = 4,
                .ctrNonceUpperLimit = 0xFF0000,
                .sessionDurationMillis = 1200000,
                .delayBetweenRenNotificationsMillis = 4000,
                .clientSidsInGroupBitmap = 1,
                .maxSilenceIntervalMillis = 5000,
        },
        [2]={
                .gid = 2,
                .maxCtrnonceDelayMsgs = 4,
                .ctrNonceUpperLimit = 0xFF0000,
                .sessionDurationMillis = 1200000,
                .delayBetweenRenNotificationsMillis = 4000,
                .clientSidsInGroupBitmap = 2, // SID == 1 does NOT belong
                .maxSilenceIntervalMillis = 5000,
        },
        // Larger than HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS on purpose,
        // allowing some test cases to use more Groups than default.
        [3]={
                .gid = 3,
                .maxCtrnonceDelayMsgs = 4,
                .ctrNonceUpperLimit = 0xFF0000,
                .sessionDurationMillis = 1200000,
                .delayBetweenRenNotificationsMillis = 4000,
                .clientSidsInGroupBitmap = 3,
                .maxSilenceIntervalMillis = 5000,
        },
        [4]={
                .gid = 4,
                .maxCtrnonceDelayMsgs = 4,
                .ctrNonceUpperLimit = 0xFF0000,
                .sessionDurationMillis = 1200000,
                .delayBetweenRenNotificationsMillis = 4000,
                .clientSidsInGroupBitmap = 4,
                .maxSilenceIntervalMillis = 5000,
        },
        [5]={
                .gid = 5,
                .maxCtrnonceDelayMsgs = 4,
                .ctrNonceUpperLimit = 0xFF0000,
                .sessionDurationMillis = 1200000,
                .delayBetweenRenNotificationsMillis = 4000,
                .clientSidsInGroupBitmap = 5,
                .maxSilenceIntervalMillis = 5000,
        },
        [6]={
                .gid = 6,
                .maxCtrnonceDelayMsgs = 4,
                .ctrNonceUpperLimit = 0xFF0000,
                .sessionDurationMillis = 1200000,
                .delayBetweenRenNotificationsMillis = 4000,
                .clientSidsInGroupBitmap = 6,
                .maxSilenceIntervalMillis = 5000,
        },
        [7]={
                .gid = 7,
                .maxCtrnonceDelayMsgs = 4,
                .ctrNonceUpperLimit = 0xFF0000,
                .sessionDurationMillis = 1200000,
                .delayBetweenRenNotificationsMillis = 4000,
                .clientSidsInGroupBitmap = 7,
                .maxSilenceIntervalMillis = 5000,
        },
        [8]={
                .gid = 8,
                .maxCtrnonceDelayMsgs = 4,
                .ctrNonceUpperLimit = 0xFF0000,
                .sessionDurationMillis = 1200000,
                .delayBetweenRenNotificationsMillis = 4000,
                .clientSidsInGroupBitmap = 8,
                .maxSilenceIntervalMillis = 5000,
        },
        [9]={
                .gid = 9,
                .maxCtrnonceDelayMsgs = 4,
                .ctrNonceUpperLimit = 0xFF0000,
                .sessionDurationMillis = 1200000,
                .delayBetweenRenNotificationsMillis = 4000,
                .clientSidsInGroupBitmap = 9,
                .maxSilenceIntervalMillis = 5000,
        },
};
