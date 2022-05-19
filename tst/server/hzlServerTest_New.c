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
 * Tests of the hzl_ServerNew() function.
 */

#include "hzlTest.h"

#if HZL_OS_AVAILABLE

static void
hzlServerTest_ServerNewCtxMustBeNotNull(void)
{
    hzl_Err_t err;

    err = hzl_ServerNew(NULL, NULL);

    atto_eq(err, HZL_ERR_NULL_CTX);
}

static void
hzlServerTest_ServerNewFileNameMustBeNotNull(void)
{
    hzl_Err_t err;
    hzl_ServerCtx_t* ctx;

    err = hzl_ServerNew(&ctx, NULL);

    atto_eq(err, HZL_ERR_NULL_FILENAME);
}

static void
hzlServerTest_ServerNewFileMustExists(void)
{
    hzl_Err_t err;
    hzl_ServerCtx_t* ctx;

    err = hzl_ServerNew(&ctx, "__idontexits__fsidf2i783ry8734t2.txt");

    atto_eq(err, HZL_ERR_CANNOT_OPEN_CONFIG_FILE);
}

static void
hzlServerTest_ServerNewFileMustHaveProperMagicNumber(void)
{
    hzl_Err_t err;
    hzl_ServerCtx_t* ctx;

    err = hzl_ServerNew(&ctx, "serverconfigfiles/invalidMagicNumber.hzl");

    atto_eq(err, HZL_ERR_INVALID_FILE_MAGIC_NUMBER);
}

static void
hzlServerTest_ServerNewFileMustHaveProperLength(void)
{
    hzl_Err_t err;
    hzl_ServerCtx_t* ctx;

    err = hzl_ServerNew(&ctx, "serverconfigfiles/tooShortMagicNumber.hzl");
    atto_eq(err, HZL_ERR_UNEXPECTED_EOF);

    err = hzl_ServerNew(&ctx, "serverconfigfiles/tooShortServerConfig.hzl");
    atto_eq(err, HZL_ERR_UNEXPECTED_EOF);

    err = hzl_ServerNew(&ctx, "serverconfigfiles/tooShortClientsConfig.hzl");
    atto_eq(err, HZL_ERR_UNEXPECTED_EOF);

    err = hzl_ServerNew(&ctx, "serverconfigfiles/tooShortGroupsConfig.hzl");
    atto_eq(err, HZL_ERR_UNEXPECTED_EOF);
}

static void
hzlServerTest_ServerNewFileMustHaveValidConfig(void)
{
    hzl_Err_t err;
    hzl_ServerCtx_t* ctx;

    err = hzl_ServerNew(&ctx, "serverconfigfiles/invalidHeaderType.hzl");
    atto_eq(err, HZL_ERR_INVALID_HEADER_TYPE);

    err = hzl_ServerNew(&ctx, "serverconfigfiles/invalidSidOrder.hzl");
    atto_eq(err, HZL_ERR_SIDS_ARE_NOT_PRESORTED_STRICTLY_ASCENDING);

    err = hzl_ServerNew(&ctx, "serverconfigfiles/invalidLtk.hzl");
    atto_eq(err, HZL_ERR_LTK_IS_ALL_ZEROS);

    err = hzl_ServerNew(&ctx, "serverconfigfiles/invalidGidOrder.hzl");
    atto_eq(err, HZL_ERR_GAP_IN_GIDS);

    err = hzl_ServerNew(&ctx, "serverconfigfiles/invalidMaxCtrnonceDelay.hzl");
    atto_eq(err, HZL_ERR_INVALID_MAX_CTRNONCE_DELAY);
}

static bool
hzlServerTest_NotAllZeros(const uint8_t* bytes, size_t amount)
{
    bool someNotZero = false;
    while (amount--)
    {
        if (*bytes++ != 0)
        {
            someNotZero = true;
            break;
        }
    }
    return someNotZero;
}

static void
hzlServerTest_ServerNewFileValidIsAccepted(void)
{
    hzl_Err_t err;
    hzl_ServerCtx_t* ctx = NULL;

    err = hzl_ServerNew(&ctx, "serverconfigfiles/Server.hzl");

    atto_eq(err, HZL_OK);
    // Server Config
    atto_eq(ctx->serverConfig->amountOfGroups, 5);
    atto_eq(ctx->serverConfig->amountOfClients, 3);
    atto_eq(ctx->serverConfig->headerType, 0);

    // Client Configs
    const uint8_t expectedLtkForSid1[16] = {1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0};
    atto_memeq(ctx->clientConfigs[0].ltk, expectedLtkForSid1, sizeof(expectedLtkForSid1));
    atto_eq(ctx->clientConfigs[0].sid, 1);

    const uint8_t expectedLtkForSid2[16] = {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    atto_memeq(ctx->clientConfigs[1].ltk, expectedLtkForSid2, sizeof(expectedLtkForSid1));
    atto_eq(ctx->clientConfigs[1].sid, 2);

    const uint8_t expectedLtkForSid3[16] = {3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    atto_memeq(ctx->clientConfigs[2].ltk, expectedLtkForSid3, sizeof(expectedLtkForSid1));
    atto_eq(ctx->clientConfigs[2].sid, 3);

    // Group Configs
    atto_eq(ctx->groupConfigs[0].maxCtrnonceDelayMsgs, 22);
    atto_eq(ctx->groupConfigs[0].ctrNonceUpperLimit, 0xFF0000U);
    atto_eq(ctx->groupConfigs[0].sessionDurationMillis, 36000000);
    atto_eq(ctx->groupConfigs[0].delayBetweenRenNotificationsMillis, 10000);
    atto_eq(ctx->groupConfigs[0].clientSidsInGroupBitmap, 0xFFFFFFFFU);
    atto_eq(ctx->groupConfigs[0].maxSilenceIntervalMillis, 5000);
    atto_eq(ctx->groupConfigs[0].gid, 0);

    atto_eq(ctx->groupConfigs[1].maxCtrnonceDelayMsgs, 20);
    atto_eq(ctx->groupConfigs[1].ctrNonceUpperLimit, 1000);
    atto_eq(ctx->groupConfigs[1].sessionDurationMillis, 36000000);
    atto_eq(ctx->groupConfigs[1].delayBetweenRenNotificationsMillis, 5000);
    atto_eq(ctx->groupConfigs[1].clientSidsInGroupBitmap, 0x06U);
    atto_eq(ctx->groupConfigs[1].maxSilenceIntervalMillis, 5000);
    atto_eq(ctx->groupConfigs[1].gid, 1);

    atto_eq(ctx->groupConfigs[2].maxCtrnonceDelayMsgs, 33);
    atto_eq(ctx->groupConfigs[2].ctrNonceUpperLimit, 0xFF0000U);
    atto_eq(ctx->groupConfigs[2].sessionDurationMillis, 36000000);
    atto_eq(ctx->groupConfigs[2].delayBetweenRenNotificationsMillis, 5000);
    atto_eq(ctx->groupConfigs[2].clientSidsInGroupBitmap, 0x01U);
    atto_eq(ctx->groupConfigs[2].maxSilenceIntervalMillis, 5001);
    atto_eq(ctx->groupConfigs[2].gid, 2);

    atto_eq(ctx->groupConfigs[3].maxCtrnonceDelayMsgs, 44);
    atto_eq(ctx->groupConfigs[3].ctrNonceUpperLimit, 0xFF0000U);
    atto_eq(ctx->groupConfigs[3].sessionDurationMillis, 36000000);
    atto_eq(ctx->groupConfigs[3].delayBetweenRenNotificationsMillis, 5000);
    atto_eq(ctx->groupConfigs[3].clientSidsInGroupBitmap, 0x03U);
    atto_eq(ctx->groupConfigs[3].maxSilenceIntervalMillis, 5002);
    atto_eq(ctx->groupConfigs[3].gid, 3);

    atto_eq(ctx->groupConfigs[4].maxCtrnonceDelayMsgs, 20);
    atto_eq(ctx->groupConfigs[4].ctrNonceUpperLimit, 16710000);
    atto_eq(ctx->groupConfigs[4].sessionDurationMillis, 36000001);
    atto_eq(ctx->groupConfigs[4].delayBetweenRenNotificationsMillis, 5077);
    atto_eq(ctx->groupConfigs[4].clientSidsInGroupBitmap, 0x04U);
    atto_eq(ctx->groupConfigs[4].maxSilenceIntervalMillis, 5000);
    atto_eq(ctx->groupConfigs[4].gid, 4);

    // State is initialised
    for (size_t i = 0; i < ctx->serverConfig->amountOfGroups; i++)
    {
        // The STK contains true random values from the OS TRNG function
        atto_assert(hzlServerTest_NotAllZeros(ctx->groupStates[i].currentStk, HZL_STK_LEN));
        atto_eq(ctx->groupStates[i].currentCtrNonce, 0);
        // There was no message received yet
        atto_eq(ctx->groupStates[i].currentRxLastMessageInstant,
                ctx->groupStates[i].sessionStartInstant);
        atto_neq(ctx->groupStates[i].sessionStartInstant, 0);
        // Previous session is cleared
        atto_eq(ctx->groupStates[i].previousCtrNonce, 0);
        atto_zeros(ctx->groupStates[i].previousStk, HZL_STK_LEN);
    }
    atto_neq(ctx->io.trng, NULL);
    atto_neq(ctx->io.currentTime, NULL);

    hzl_ServerFree(&ctx);
}

#endif  /* HZL_OS_AVAILABLE */

void hzlServerTest_ServerNew(void)
{
#if HZL_OS_AVAILABLE
    hzlServerTest_ServerNewCtxMustBeNotNull();
    hzlServerTest_ServerNewFileNameMustBeNotNull();
    hzlServerTest_ServerNewFileMustExists();
    hzlServerTest_ServerNewFileMustHaveProperMagicNumber();
    hzlServerTest_ServerNewFileMustHaveProperLength();
    hzlServerTest_ServerNewFileMustHaveValidConfig();
    hzlServerTest_ServerNewFileValidIsAccepted();
    HZL_TEST_PARTIAL_REPORT();
#endif  /* HZL_OS_AVAILABLE */
}
