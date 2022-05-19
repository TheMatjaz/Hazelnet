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
 * Server-only internal definitions and functions.
 */

#ifndef HZL_SERVER_INTERNAL_H
#define HZL_SERVER_INTERNAL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "hzl.h"
#include "hzl_Server.h"
#include "hzl_CommonInternal.h"

/** Double-checking that the bitmap can hold the max amount of Clients. */
_Static_assert(
        sizeof(hzl_ServerBitMap_t) * 8U <= HZL_SERVER_MAX_AMOUNT_OF_CLIENTS,
        "The bitmap of Clients in the Group must be large enough to support "
        "the max amount of Clients.");

/**
 * @internal
 * Verifies only the pointers to the context itself, its data structures
 * and the IO function pointers are not NULL.
 *
 * Does not check the values within the structs.
 *
 * @param [in] ctx to check, may be NULL.
 */
hzl_Err_t
hzl_ServerCheckCtxPointers(const hzl_ServerCtx_t* ctx);

/**
 * @internal
 * Increments the Group's Counter Nonce by 1, unless its upper limit was reached and the Nonce
 * is already expired.
 */
void
hzl_ServerGroupIncrCurrentCtrnonce(const hzl_ServerCtx_t* ctx,
                                   hzl_Gid_t groupId);

/**
 * @internal
 * Increments the Group's previous Counter Nonce by 1, unless its upper limit was reached and the
 * Nonce is already expired.
 */
void
hzl_ServerGroupIncrPreviousCtrnonce(const hzl_ServerCtx_t* ctx,
                                    hzl_Gid_t groupId);

/** @internal Checks if the current Session is expired and, if so, it generates a new one,
 * builds a REN message as reaction PDU and starts the renewal phase. */
hzl_Err_t
hzl_ServerSessionRenewalPhaseEnterIfNeeded(hzl_CbsPduMsg_t* reactionPdu,
                                           hzl_ServerCtx_t* ctx,
                                           hzl_Timestamp_t now,
                                           hzl_Gid_t gid);

/** @internal Checks if the Session renewal phase is expired and, if so, erases the
 * information from the previous session. */
void
hzl_ServerSessionRenewalPhaseExitIfNeeded(hzl_ServerCtx_t* ctx,
                                          hzl_Timestamp_t now,
                                          hzl_Gid_t gid);

/** @internal True if the Group is currently in the Session Renewal phase. */
bool
hzl_ServerSessionRenewalPhaseIsActive(const hzl_ServerCtx_t* ctx,
                                      hzl_Gid_t gid);


/** @internal Builds a REN message. */
hzl_Err_t
hzl_ServerBuildMsgRenewal(hzl_CbsPduMsg_t* reactionPdu,
                          hzl_ServerCtx_t* ctx,
                          hzl_Gid_t gid);

/** @internal Start a session renewal phase, forcibly. */
hzl_Err_t
hzl_ServerSessionRenewalPhaseEnter(hzl_ServerCtx_t* ctx,
                                   hzl_Gid_t gid);

/** @internal Checks whether a Secured Application Data or Renewal message could already
 * be build and transmitted. In other words, returns true if at least one Client
 * has already Requested the Session Information and should be thus able to properly validate
 * the received secured message. */
bool
hzl_ServerDidAnyClientAlreadyRequest(const hzl_ServerCtx_t* ctx,
                                     hzl_Gid_t groupId);

/** @internal Stores the timestamp of last reception in the current Session, while taking
 * care of a corner case timing condition. */
void
hzl_ServerUpdateCurrentRxLastMessageInstant(const hzl_ServerCtx_t* ctx,
                                            hzl_Timestamp_t rxTimestamp,
                                            hzl_Gid_t gid);

#ifdef __cplusplus
}
#endif

#endif  /* HZL_SERVER_INTERNAL_H */
