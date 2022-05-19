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
 * Implementation of the hzl_ServerForceSessionRenewal() function.
 */

#include "hzl.h"
#include "hzl_Server.h"
#include "hzl_ServerInternal.h"

HZL_API hzl_Err_t
hzl_ServerForceSessionRenewal(hzl_CbsPduMsg_t* const renewalPdu,
                              hzl_ServerCtx_t* const ctx,
                              const hzl_Gid_t groupId)
{
    if (renewalPdu == NULL) { return HZL_ERR_NULL_PDU; }
    renewalPdu->dataLen = 0; // Make output message empty in case of later error.
    HZL_ERR_DECLARE(err);
    err = hzl_ServerCheckCtxPointers(ctx);
    HZL_ERR_CHECK(err);
    if (groupId >= ctx->serverConfig->amountOfGroups)
    {
        return HZL_ERR_UNKNOWN_GROUP;
    }
    const bool renewalPhaseIsActive = hzl_ServerSessionRenewalPhaseIsActive(ctx, groupId);
    if (!renewalPhaseIsActive
        && !hzl_ServerDidAnyClientAlreadyRequest(ctx, groupId))
    {
        // Either the renewal phase is over or this function is called after initialisation
        // (before not even one renewal phase was started): there is no previous Session
        // information (anymore), only the current one and there is no Client that has it,
        // thus they would not be able to process a REN message correctly.
        return HZL_ERR_NO_POTENTIAL_RECEIVER;
    }
    if (!renewalPhaseIsActive)
    {
        // Enter the Session Renewal Phase only the first time this function
        // is called, if not already in it. This prevents re-entering it
        // in case repeated REN messages are built explicitly by the user.
        err = hzl_ServerSessionRenewalPhaseEnter(ctx, groupId);
        HZL_ERR_CHECK(err);
    }
    return hzl_ServerBuildMsgRenewal(renewalPdu, ctx, groupId);
}
