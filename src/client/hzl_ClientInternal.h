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
 */

#ifndef HZL_CLIENT_INTERNAL_H_
#define HZL_CLIENT_INTERNAL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "hzl.h"
#include "hzl_Client.h"
#include "hzl_CommonInternal.h"

/** @internal Struct referencing the configuration and state of the same Group
 * on the Client. Used to avoid looking up the group every time. */
typedef struct hzl_ClientGroup
{
    const hzl_ClientGroupConfig_t* config;  ///< @internal Configuration of the Group
    hzl_ClientGroupState_t* state;  ///< @internal State of the Group
} hzl_ClientGroup_t;

/**
 * @internal
 * Verifies pointers of the context and the correctness of the configuration.
 * The context must not be used in case of error.
 *
 * @param [in] ctx to check, left unmodified.
 * @return Same return values as hzl_ClientInit().
 */
hzl_Err_t
hzl_ClientCheckCtx(const hzl_ClientCtx_t* ctx);

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
hzl_ClientCheckCtxPointers(const hzl_ClientCtx_t* ctx);

/**
 * @internal
 * Clears the state securely. Does not perform any memory safety checks.
 *
 * @param [in, out] ctx context with state to be cleared.
 */
void
hzl_ClientClearStateUnchecked(hzl_ClientCtx_t* ctx);

/**
 * @internal
 * Linear search through the Groups, providing a handle to their state and config from the GID.
 *
 * @param [out] group pointers to the Group state and config
 * @param [in] ctx to search through the Group configs for the GID
 * @param [in] groupId GID of the Group to search
 *
 * @retval #HZL_OK on success
 * @retval #HZL_ERR_UNKNOWN_GROUP when no Group with the given GID can be found.
 */
hzl_Err_t
hzl_ClientFindGroup(hzl_ClientGroup_t* group,
                    const hzl_ClientCtx_t* ctx,
                    hzl_Gid_t groupId);

/**
 * @internal
 * Checks if a Group has an established session
 *
 * @param [in] group to check
 * @return true when established and valid (may be used)
 */
bool
hzl_ClientIsSessionEstablishedAndValid(const hzl_ClientGroup_t* group);

/**
 * @internal
 * Checks if a Group has a handshake currently ongoing, i.e. waiting for a Response but timeout
 * was not reached yet.
 *
 * @param [out] isOngoing true if there is a handshake right now
 * @param [in] ctx to access timestamping functions and timeout configurations
 * @param [in] group to check
 *
 * @retval #HZL_OK on success
 * @retval #HZL_ERR_CANNOT_GET_CURRENT_TIME when requesting the current
 *         timer/clock to the OS or driver fails.
 */
hzl_Err_t
hzl_ClientIsAHandShakeOngoing(bool* isOngoing,
                              const hzl_ClientCtx_t* ctx,
                              const hzl_ClientGroup_t* group);

/**
 * @internal
 * Checks if a Group was still waiting for a Response message at the moment of reception.
 *
 * @param [in] ctx to access timestamping functions and timeout configurations
 * @param [in] group to check
 * @param [in] rxTimestamp timestamp of reception
 *
 * @retval #HZL_OK when the Group is expecting a Response
 * @retval #HZL_ERR_SECWARN_NOT_EXPECTING_A_RESPONSE when not expecting one
 * @retval #HZL_ERR_SECWARN_RESPONSE_TIMEOUT when expecting one, but it was received too late
 */
hzl_Err_t
hzl_ClientIsResponseAcceptable(const hzl_ClientCtx_t* ctx,
                               const hzl_ClientGroup_t* group,
                               hzl_Timestamp_t rxTimestamp);

/**
 * @internal
 * Checks if a Group can process a Session Renewal Notification message at the
 * current time.
 *
 * This happens when the Group is not in a handshake and is not in the
 * Session renewal phase, thus another REN message was already processed.
 *
 * @param [in] group to check
 * @return true when the REN message can be processed
 */
bool
hzl_ClientIsRenewalAcceptable(const hzl_ClientGroup_t* group);

/**
 * @internal
 * Builds, packs, authenticates and transmits and Request message for a given Group.
 *
 * @param [out] msgToTx CBS message in packed format, ready to transmit
 * @param [in, out] ctx to access the Group configuration and state
 * @param [in] group to send the Request for
 *
 * @retval #HZL_OK on success.
 * @retval #HZL_ERR_CANNOT_GENERATE_RANDOM if the random generation fails.
 * @retval #HZL_ERR_CANNOT_GENERATE_NON_ZERO_RANDOM if the random generation is unable
 *         to provide non-zero arrays of bytes.
 */
hzl_Err_t
hzl_ClientBuildMsgReq(hzl_CbsPduMsg_t* msgToTx,
                      const hzl_ClientCtx_t* ctx,
                      const hzl_ClientGroup_t* group);

/**
 * @internal
 * Verifies the received counter nonce of an application data message.
 * Checks for received expired counter nonces and for too-old nonces.
 * The tolerance for what "too old" is getting lower over time, thus the longer the silence
 * between 2 consecutive messages, the more recent must the received counter nonce be.
 *
 * @param [in, out] isPreviousSession true if the CtrNonce belongs to the previous session
 *              if the Group is currently in the Session renewal phase.
 *              If NULL, the check for the previous vs current session is skipped entirely
 *              and just the current session is used.
 * @param [in] group destination Group of the message.
 * @param [in] receivedCtrnonce counter nonce of the received message.
 * @param [in] rxTimestamp timestamp of reception.
 *
 * @retval #HZL_OK on a valid nonce.
 * @retval #HZL_ERR_SECWARN_RECEIVED_OVERFLOWN_NONCE on a received
 *         expired nonce.
 * @retval #HZL_ERR_SECWARN_OLD_MESSAGE on a received too-old nonce.
 */
hzl_Err_t
hzl_ClientCheckRxCtrnonce(bool* isPreviousSession,
                          const hzl_ClientGroup_t* group,
                          hzl_CtrNonce_t receivedCtrnonce,
                          hzl_Timestamp_t rxTimestamp);

/**
 * @internal
 * Saves the current time as the timestamp of the transmission of the Request message for a Group,
 * to later know if the Response has been received in time.
 *
 * @param [in, out] ctx to access the timestamping function
 * @param [in] group to set the timestamp for
 *
 * @retval #HZL_OK on success
 * @retval #HZL_ERR_CANNOT_GET_CURRENT_TIME when requesting the current
 *         timer/clock to the OS or driver fails.
 */
hzl_Err_t
hzl_ClientSetRequestTxTimeToNow(const hzl_ClientCtx_t* ctx,
                                const hzl_ClientGroup_t* group);

/**
 * @internal
 * Increments the Group's Counter Nonce by 1, unless its upper limit was reached and the Nonce
 * is already expired.
 *
 * @param [in, out] group to increment the Counter Nonce of.
 */
void
hzl_ClientGroupIncrCurrentCtrnonce(const hzl_ClientGroup_t* group);

/**
 * @internal
 * Updates the Groups's Counter Nonce and last-reception timestamp upon reception of
 * a valid message with a Counter Nonce.
 *
 * Selects the storing into the current or previous session based on the passed
 * parameter. Takes the largest between the local and received and stores it
 * incremented by 1.
 *
 * @param [in] group to store the Counter Nonce into.
 * @param [in] receivedCtrnonce as contained in a Secured Application Data message.
 * @param [in] receptionTimestamp instant when the Secured Application Data message
 *        was received.
 * @param [in] isPreviousSession true if the counter nonce to be incremented is
 *             the one of the previous session during the Session renewal phase.
 */
void
hzl_ClientGroupUpdateCtrnonceAndRxTimestamp(const hzl_ClientGroup_t* group,
                                            hzl_CtrNonce_t receivedCtrnonce,
                                            hzl_Timestamp_t receptionTimestamp,
                                            bool isPreviousSession);

/**
 * @internal
 * Stores the current Session information as the previous (old) Session
 * to use to handle lingering old message on the bus or RX queue.
 * @param [in] group to start the renewal phase for
 */
void
hzl_ClientSessionRenewalPhaseEnter(const hzl_ClientGroup_t* group);

/**
 * @internal
 * Terminates the Session renewal phase if the conditions are met, clearing the
 * previous Session information.
 * @param [in] group to check and potentially stop the renewal phase for
 * @param [in] now current timestamp
 */
void
hzl_ClientSessionRenewalPhaseExitIfNeeded(const hzl_ClientGroup_t* group,
                                          hzl_Timestamp_t now);

#ifdef __cplusplus
}
#endif

#endif  /* HZL_CLIENT_INTERNAL_H_ */
