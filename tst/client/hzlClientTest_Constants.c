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

const hzl_ClientConfig_t HZL_TEST_CORRECT_CLIENT_CONFIG = {
        .amountOfGroups = HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS,
        .headerType = HZL_HEADER_0,
        .ltk = {1}, // Not all-zeros
        .sid = 13,
        .timeoutReqToResMillis = 5000,
};

const hzl_ClientGroupConfig_t
        HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS[HZL_MAX_TEST_AMOUNT_OF_GROUPS] = {
        [0]={
                .gid = 0,
                .maxCtrnonceDelayMsgs = 4,
                .maxSilenceIntervalMillis = 10000,
                .sessionRenewalDurationMillis = 4999,
        },
        [1]={
                .gid = 2,
                .maxCtrnonceDelayMsgs = 20,
                .maxSilenceIntervalMillis = 5000,
                .sessionRenewalDurationMillis = 5001,
        },
        [2]={
                .gid = 3,
                .maxCtrnonceDelayMsgs = 20,
                .maxSilenceIntervalMillis = 5000,
                .sessionRenewalDurationMillis = 5001,
        },
        // Larger than HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS on purpose,
        // allowing some test cases to use more Groups than default.
        [3]={
                .gid = 4,
                .maxCtrnonceDelayMsgs = 20,
                .maxSilenceIntervalMillis = 5000,
                .sessionRenewalDurationMillis = 5001,
        },
        [4]={
                .gid = 5,
                .maxCtrnonceDelayMsgs = 20,
                .maxSilenceIntervalMillis = 5000,
                .sessionRenewalDurationMillis = 5001,
        },
        [5]={
                .gid = 6,
                .maxCtrnonceDelayMsgs = 20,
                .maxSilenceIntervalMillis = 5000,
                .sessionRenewalDurationMillis = 5001,
        },
        [6]={
                .gid = 7,
                .maxCtrnonceDelayMsgs = 20,
                .maxSilenceIntervalMillis = 5000,
                .sessionRenewalDurationMillis = 5001,
        },
        [7]={
                .gid = 8,
                .maxCtrnonceDelayMsgs = 20,
                .maxSilenceIntervalMillis = 5000,
                .sessionRenewalDurationMillis = 5001,
        },
        [8]={
                .gid = 9,
                .maxCtrnonceDelayMsgs = 20,
                .maxSilenceIntervalMillis = 5000,
                .sessionRenewalDurationMillis = 5001,
        },
        [9]={
                .gid = 250,
                .maxCtrnonceDelayMsgs = 20,
                .maxSilenceIntervalMillis = 5000,
                .sessionRenewalDurationMillis = 5001,
        },
};
