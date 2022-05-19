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
 * Tests of the hzl_ClientNew() function.
 */

#include "hzlTest.h"

#if HZL_OS_AVAILABLE

static void
hzlClientTest_ClientNewCtxMustBeNotNull(void)
{
    hzl_Err_t err;

    err = hzl_ClientNew(NULL, NULL);

    atto_eq(err, HZL_ERR_NULL_CTX);
}

static void
hzlClientTest_ClientNewFileNameMustBeNotNull(void)
{
    hzl_Err_t err;
    hzl_ClientCtx_t* ctx;

    err = hzl_ClientNew(&ctx, NULL);

    atto_eq(err, HZL_ERR_NULL_FILENAME);
}

static void
hzlClientTest_ClientNewFileMustExists(void)
{
    hzl_Err_t err;
    hzl_ClientCtx_t* ctx;

    err = hzl_ClientNew(&ctx, "__idontexits__fsidf2i783ry8734t2.txt");

    atto_eq(err, HZL_ERR_CANNOT_OPEN_CONFIG_FILE);
}

static void
hzlClientTest_ClientNewFileMustHaveProperMagicNumber(void)
{
    hzl_Err_t err;
    hzl_ClientCtx_t* ctx;

    err = hzl_ClientNew(&ctx, "clientconfigfiles/invalidMagicNumber.hzl");

    atto_eq(err, HZL_ERR_INVALID_FILE_MAGIC_NUMBER);
}

static void
hzlClientTest_ClientNewFileMustHaveProperLength(void)
{
    hzl_Err_t err;
    hzl_ClientCtx_t* ctx;

    err = hzl_ClientNew(&ctx, "clientconfigfiles/tooShortMagicNumber.hzl");
    atto_eq(err, HZL_ERR_UNEXPECTED_EOF);

    err = hzl_ClientNew(&ctx, "clientconfigfiles/tooShortClientConfig.hzl");
    atto_eq(err, HZL_ERR_UNEXPECTED_EOF);

    err = hzl_ClientNew(&ctx, "clientconfigfiles/tooShortGroupsConfig.hzl");
    atto_eq(err, HZL_ERR_UNEXPECTED_EOF);
}

static void
hzlClientTest_ClientNewFileMustHaveValidConfig(void)
{
    hzl_Err_t err;
    hzl_ClientCtx_t* ctx;

    err = hzl_ClientNew(&ctx, "clientconfigfiles/invalidSid.hzl");
    atto_eq(err, HZL_ERR_SERVER_SID_ASSIGNED_TO_CLIENT);

    err = hzl_ClientNew(&ctx, "clientconfigfiles/missingGidZero.hzl");
    atto_eq(err, HZL_ERR_MISSING_GID_0);

    err = hzl_ClientNew(&ctx, "clientconfigfiles/invalidMaxCtrnonceDelay.hzl");
    atto_eq(err, HZL_ERR_INVALID_MAX_CTRNONCE_DELAY);
}

static void
hzlClientTest_ClientNewFileAliceIsAccepted(void)
{
    hzl_Err_t err;
    hzl_ClientCtx_t* ctx;

    err = hzl_ClientNew(&ctx, "clientconfigfiles/Alice.hzl");

    atto_eq(err, HZL_OK);
    atto_eq(ctx->clientConfig->timeoutReqToResMillis, 10000);
    const uint8_t expectedLtk[16] = {1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0};
    atto_memeq(ctx->clientConfig->ltk, expectedLtk, sizeof(expectedLtk));
    atto_eq(ctx->clientConfig->sid, 1);
    atto_eq(ctx->clientConfig->headerType, 0);
    atto_eq(ctx->clientConfig->amountOfGroups, 3);
    atto_eq(ctx->groupConfigs[0].maxCtrnonceDelayMsgs, 22);
    atto_eq(ctx->groupConfigs[0].maxSilenceIntervalMillis, 5000);
    atto_eq(ctx->groupConfigs[0].sessionRenewalDurationMillis, 10000);
    atto_eq(ctx->groupConfigs[0].gid, 0);
    atto_eq(ctx->groupConfigs[1].maxCtrnonceDelayMsgs, 33);
    atto_eq(ctx->groupConfigs[1].maxSilenceIntervalMillis, 5001);
    atto_eq(ctx->groupConfigs[1].sessionRenewalDurationMillis, 30000);
    atto_eq(ctx->groupConfigs[1].gid, 2);
    atto_eq(ctx->groupConfigs[2].maxCtrnonceDelayMsgs, 44);
    atto_eq(ctx->groupConfigs[2].maxSilenceIntervalMillis, 5002);
    atto_eq(ctx->groupConfigs[2].sessionRenewalDurationMillis, 5000);
    atto_eq(ctx->groupConfigs[2].gid, 3);
    atto_zeros(ctx->groupStates,
               ctx->clientConfig->amountOfGroups * sizeof(hzl_ClientGroupState_t));
    atto_neq(ctx->io.trng, NULL);
    atto_neq(ctx->io.currentTime, NULL);

    hzl_ClientFree(&ctx);
}

static void
hzlClientTest_ClientNewBobAndCharlieAreAccepted(void)
{
    hzl_Err_t err;
    hzl_ClientCtx_t* ctx;

    err = hzl_ClientNew(&ctx, "clientconfigfiles/Bob.hzl");
    atto_eq(err, HZL_OK);
    hzl_ClientFree(&ctx);

    err = hzl_ClientNew(&ctx, "clientconfigfiles/Charlie.hzl");
    atto_eq(err, HZL_OK);
    hzl_ClientFree(&ctx);
}

static void
hzlClientTest_ClientNewOsIoFunctionsWork(void)
{
    hzl_Err_t err;
    hzl_ClientCtx_t* ctx;
    err = hzl_ClientNew(&ctx, "clientconfigfiles/Alice.hzl");
    atto_eq(err, HZL_OK);
    hzl_CbsPduMsg_t msgToTx = {0};
    atto_eq(ctx->groupStates[0].lastHandshakeEventInstant, 0);
    atto_eq(ctx->groupStates[0].requestNonce, 0);

    err = hzl_ClientBuildRequest(&msgToTx, ctx, 0);

    atto_eq(err, HZL_OK);
    atto_neq(ctx->groupStates[0].lastHandshakeEventInstant, 0);
    atto_neq(ctx->groupStates[0].requestNonce, 0);
}

#endif  /* HZL_OS_AVAILABLE */

void hzlClientTest_ClientNew(void)
{
#if HZL_OS_AVAILABLE
    hzlClientTest_ClientNewCtxMustBeNotNull();
    hzlClientTest_ClientNewFileNameMustBeNotNull();
    hzlClientTest_ClientNewFileMustExists();
    hzlClientTest_ClientNewFileMustHaveProperMagicNumber();
    hzlClientTest_ClientNewFileMustHaveProperLength();
    hzlClientTest_ClientNewFileMustHaveValidConfig();
    hzlClientTest_ClientNewFileAliceIsAccepted();
    hzlClientTest_ClientNewBobAndCharlieAreAccepted();
    hzlClientTest_ClientNewOsIoFunctionsWork();
    HZL_TEST_PARTIAL_REPORT();
#endif  /* HZL_OS_AVAILABLE */
}
