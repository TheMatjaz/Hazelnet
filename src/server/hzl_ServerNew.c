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
 * Implementation of the hzl_ServerNew() function.
 */

#include "hzl_ServerOs.h"
#include "hzl_ServerInternal.h"
#include "hzl_CommonEndian.h"
#include "hzl_CommonInternal.h"

#if HZL_OS_AVAILABLE

/** @internal Loads an arbitrary amount of bytes from the stream, basically wrapping fopen()
 * to return a Hazelnet error code on unexpected stream end. */
inline static hzl_Err_t
hzl_LoadBytes(uint8_t* const bytes, FILE* const fileStream, const size_t amount)
{
    const size_t bufferedLen = fread(bytes, 1U, amount, fileStream);
    if (bufferedLen != amount) { return HZL_ERR_UNEXPECTED_EOF; }
    return HZL_OK;
}

/** @internal Verifies the file starts with `"HZLs\0" = {0x68, 0x7A, 0x73, 0x00}`
 * to double check the correct binary file was selected. */
static hzl_Err_t
hzl_CheckMagicNumber(FILE* const fileStream)
{
    HZL_ERR_DECLARE(err);
    uint8_t magicNumber[5U] = {0};
    err = hzl_LoadBytes(magicNumber, fileStream, sizeof(magicNumber));
    HZL_ERR_CHECK(err);
    if (magicNumber[0] != 'H'
        || magicNumber[1] != 'Z'
        || magicNumber[2] != 'L'
        || magicNumber[3] != 's'
        || magicNumber[4] != '\0')
    {
        return HZL_ERR_INVALID_FILE_MAGIC_NUMBER;
    }
    return err;
}

/** @internal Loads a uint32 in Little Endian encoding from the file. */
static hzl_Err_t
hzl_LoadUint32Le(uint32_t* const value, FILE* const fileStream)
{
    HZL_ERR_DECLARE(err);
    uint8_t bytes[sizeof(uint32_t)] = {0};
    err = hzl_LoadBytes(bytes, fileStream, sizeof(bytes));
    *value = hzl_DecodeLe32(bytes);
    return err;
}

/** @internal Loads a uint16 in Little Endian encoding from the file. */
static hzl_Err_t
hzl_LoadUint16Le(uint16_t* const value, FILE* const fileStream)
{
    HZL_ERR_DECLARE(err);
    uint8_t bytes[sizeof(uint16_t)] = {0};
    err = hzl_LoadBytes(bytes, fileStream, sizeof(bytes));
    *value = hzl_DecodeLe16(bytes);
    return err;
}

/** @internal Loads a uint8 in from the file (syntax sugar). */
inline static hzl_Err_t
hzl_LoadUint8(uint8_t* const value, FILE* const fileStream)
{
    return hzl_LoadBytes(value, fileStream, sizeof(uint8_t));
}

/** @internal Loads the Server Configuration structure from the file. */
inline static hzl_Err_t
hzl_LoadServerConfig(hzl_ServerConfig_t* const config, FILE* const fileStream)
{
    HZL_ERR_DECLARE(err);
    err = hzl_LoadUint8(&config->amountOfGroups, fileStream);
    HZL_ERR_CHECK(err);
    err = hzl_LoadUint8(&config->amountOfClients, fileStream);
    HZL_ERR_CHECK(err);
    err = hzl_LoadUint8(&config->headerType, fileStream);
    return err;
}

/** @internal Loads a single Client configuration structure from the file. */
inline static hzl_Err_t
hzl_LoadClientConfig(hzl_ServerClientConfig_t* const client, FILE* const fileStream)
{
    HZL_ERR_DECLARE(err);
    err = hzl_LoadUint8(&client->sid, fileStream);
    HZL_ERR_CHECK(err);
    err = hzl_LoadBytes(client->ltk, fileStream, HZL_LTK_LEN);
    return err;
}

/** @internal Loads a single Group configuration structure from the file. */
inline static hzl_Err_t
hzl_LoadGroupConfig(hzl_ServerGroupConfig_t* const group, FILE* const fileStream)
{
    HZL_ERR_DECLARE(err);
    err = hzl_LoadUint32Le(&group->maxCtrnonceDelayMsgs, fileStream);
    HZL_ERR_CHECK(err);
    err = hzl_LoadUint32Le(&group->ctrNonceUpperLimit, fileStream);
    HZL_ERR_CHECK(err);
    err = hzl_LoadUint32Le(&group->sessionDurationMillis, fileStream);
    HZL_ERR_CHECK(err);
    err = hzl_LoadUint32Le(&group->delayBetweenRenNotificationsMillis, fileStream);
    HZL_ERR_CHECK(err);
    err = hzl_LoadUint32Le(&group->clientSidsInGroupBitmap, fileStream);
    HZL_ERR_CHECK(err);
    err = hzl_LoadUint16Le(&group->maxSilenceIntervalMillis, fileStream);
    HZL_ERR_CHECK(err);
    err = hzl_LoadUint8(&group->gid, fileStream);
    HZL_ERR_CHECK(err);
    err = hzl_LoadBytes(group->unusedPadding, fileStream, 1U);
    return err;
}

HZL_API hzl_Err_t
hzl_ServerNew(hzl_ServerCtx_t** const pCtx,
              const char* const fileName)
{
    HZL_ERR_DECLARE(err);
    FILE* fileStream = NULL;
    hzl_ServerCtx_t* ctx = NULL;
    if (pCtx == NULL) { return HZL_ERR_NULL_CTX; }
    *pCtx = NULL;  // Empty output in case of allocation errors.
    if (fileName == NULL) { return HZL_ERR_NULL_FILENAME; }
    fileStream = fopen(fileName, "r");
    if (fileStream == NULL) { return HZL_ERR_CANNOT_OPEN_CONFIG_FILE; }
    err = hzl_CheckMagicNumber(fileStream);
    HZL_ERR_CLEANUP(err);
    // At this point, the file was successfully opened and seems to be of the correct format.
    ctx = calloc(1U, sizeof(hzl_ServerCtx_t));
    if (ctx == NULL)
    {
        err = HZL_ERR_MALLOC_FAILED;
        goto cleanup;
    }
    ctx->serverConfig = calloc(1U, sizeof(hzl_ServerConfig_t));
    if (ctx->serverConfig == NULL)
    {
        err = HZL_ERR_MALLOC_FAILED;
        goto cleanup;
    }
    // Here we force the pointer to the constant configuration to be writable just once
    // because we have to fill the configuration in the first place.
    err = hzl_LoadServerConfig((hzl_ServerConfig_t*) ctx->serverConfig, fileStream);
    HZL_ERR_CLEANUP(err);
    ctx->clientConfigs = calloc(ctx->serverConfig->amountOfClients,
                                sizeof(hzl_ServerClientConfig_t));
    if (ctx->clientConfigs == NULL)
    {
        err = HZL_ERR_MALLOC_FAILED;
        goto cleanup;
    }
    for (size_t client = 0; client < ctx->serverConfig->amountOfClients; client++)
    {
        // Here we force the pointer to the constant configuration to be writable just once
        // because we have to fill the configuration in the first place.
        err = hzl_LoadClientConfig(
                (hzl_ServerClientConfig_t*) &ctx->clientConfigs[client],
                fileStream);
        HZL_ERR_CLEANUP(err);
    }

    ctx->groupConfigs = calloc(ctx->serverConfig->amountOfGroups,
                               sizeof(hzl_ServerGroupConfig_t));
    if (ctx->groupConfigs == NULL)
    {
        err = HZL_ERR_MALLOC_FAILED;
        goto cleanup;
    }
    for (size_t group = 0; group < ctx->serverConfig->amountOfGroups; group++)
    {
        // Here we force the pointer to the constant configuration to be writable just once
        // because we have to fill the configuration in the first place.
        err = hzl_LoadGroupConfig(
                (hzl_ServerGroupConfig_t*) &ctx->groupConfigs[group],
                fileStream);
        HZL_ERR_CLEANUP(err);
    }
    ctx->groupStates = calloc(
            ctx->serverConfig->amountOfGroups, sizeof(hzl_ServerGroupState_t));
    if (ctx->groupStates == NULL)
    {
        err = HZL_ERR_MALLOC_FAILED;
        goto cleanup;
    }
    ctx->io.currentTime = hzl_OsCurrentTime;
    ctx->io.trng = hzl_OsTrng;
    err = hzl_ServerInit(ctx);
    *pCtx = ctx;
    ctx = NULL;
    cleanup:
    {
        fclose(fileStream);
        if (err != HZL_OK) { hzl_ServerFree(&ctx); }
    }
    return err;
}

#endif  /* HZL_OS_AVAILABLE */
