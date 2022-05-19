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
 * Implementation of the hzl_ServerFree() function.
 */

#include "hzl_ServerOs.h"
#include "hzl_ServerInternal.h"
#include "hzl_CommonInternal.h"

#if HZL_OS_AVAILABLE

HZL_API void
hzl_ServerFree(hzl_ServerCtx_t** const pCtx)
{
    if (pCtx == NULL || *pCtx == NULL) { return; }
    hzl_ServerCtx_t* ctx = *pCtx;  // Dereference once to make the code more readable
    if (ctx->serverConfig != NULL)
    {
        HZL_SECURE_FREE(ctx->groupStates,
                        ctx->serverConfig->amountOfGroups *
                        sizeof(hzl_ServerGroupState_t));
        // Here we force the pointer to the constant configuration to be writable just once
        // because we have to clear the configuration securely before freeing it.
        HZL_SECURE_FREE(ctx->clientConfigs,
                        ctx->serverConfig->amountOfClients *
                        sizeof(hzl_ServerClientConfig_t));
        // Here we force the pointer to the constant configuration to be writable just once
        // because we have to clear the configuration securely before freeing it.
        HZL_SECURE_FREE(ctx->groupConfigs,
                        ctx->serverConfig->amountOfGroups *
                        sizeof(hzl_ServerGroupConfig_t));
    }
    // Here we force the pointer to the constant configuration to be writable just once
    // because we have to clear the configuration securely before freeing it.
    HZL_SECURE_FREE(ctx->serverConfig, sizeof(hzl_ServerConfig_t));
    HZL_SECURE_FREE(ctx, sizeof(hzl_ServerCtx_t));
    *pCtx = NULL;
}

#endif  /* HZL_OS_AVAILABLE */
