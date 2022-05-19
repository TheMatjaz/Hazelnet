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
 * Hazelnet Client public API.
 *
 * Hazelnet implements the CAN Bus Security (CBS) protocol, which secures the CAN FD
 * traffic providing encryption, authenticity and freshness of the messages.
 *
 * The user of the library must handle the physical transmission and reception manually
 * as this library only handles the building of messages to transmit and processing of
 * received messages. The internal library state keeps track of ongoing handshakes, timeouts
 * and other events per each Group.
 *
 * The intended usage of the library is listed in an example snippet in the readme.
 */

#ifndef HZL_CLIENT_H_
#define HZL_CLIENT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "hzl.h"

/**
 * Hazelnet Client constant configuration.
 *
 * Single instance per Client.
 * Initialised by the user, not modified by the Client.
 * Initialised by the user, not modified by the Client.
 */
typedef struct hzl_ClientConfig
{
    /**
     * Duration in milliseconds until a Request times out (t_{reqres}).
     *
     * Measured between the TX function call (may be asynchronous) of the Request
     * and the processing of the Response, so overhead latencies may be included.
     * May have any value in [0, 0xFFFF].
     */
    HZL_SET_BY_USER uint16_t timeoutReqToResMillis;
    /**
     * Symmetric Long Term Key of this Client (LTK_{AS}), uniquely identifying this Client
     * against the Server.
     *
     * Only the Server has another copy. Must not be all-zeros.
     */
    HZL_SET_BY_USER uint8_t ltk[HZL_LTK_LEN];
    /**
     * Source Identifier of this Client.
     *
     * Unique across all Clients. Must not be #HZL_SERVER_SID.
     */
    HZL_SET_BY_USER hzl_Sid_t sid;
    /**
     * Header Type (h) used by the network of CBS-enabled nodes.
     *
     * Must be the same as all other nodes, otherwise the messages will not
     * be interpreted correctly, both sent and received.
     * Must be one of #hzl_HeaderType_t enum fields.
     */
    HZL_SET_BY_USER uint8_t headerType;
    /**
     * Only Groups this client is in, i.e.
     * amount of #hzl_ClientGroupConfig_t instances.
     *
     * Must be >= 1.
     */
    HZL_SET_BY_USER uint8_t amountOfGroups;
    /** Padding to the next struct. */
    uint8_t unusedPadding[1];
} hzl_ClientConfig_t;

/** Double-checking the size of the hzl_ClientConfig_t struct to avoid
 * unexpected paddings. */
_Static_assert(sizeof(hzl_ClientConfig_t) == 22,
               "The size of the Client Config struct must be exactly 22 B");

/**
 * Hazelnet Client constant Group configuration.
 *
 * Unique per Group, multiple instances per Client.
 * Initialised by the user, not modified by the Client.
 */
typedef struct hzl_ClientGroupConfig
{
    /**
     * Maximum Counter Nonce Delay (D^{max}_G) in milliseconds,
     * used to filter out recent messages from old ones.
     *
     * Must be in [0, #HZL_LARGEST_MAX_COUNTER_NONCE_DELAY].
     */
    HZL_SET_BY_USER uint32_t maxCtrnonceDelayMsgs;
    /**
     * Maximum Silence Interval (S^{max}_G) in milliseconds,
     * used to filter out recent messages from old ones.
     *
     * May have any value in [0, 0xFFFF].
     */
    HZL_SET_BY_USER uint16_t maxSilenceIntervalMillis;
    /**
     * Client-Side Session Renewal time Duration (t^{ren}_G) in milliseconds.
     *
     * Used to know when then information about the expired Session must be deleted.
     * May have any value in [0, 0xFFFF].
     */
    HZL_SET_BY_USER uint16_t sessionRenewalDurationMillis;
    /**
     * Group Identifier of this Group (id_G).
     *
     * An array of these structs must be sorted strictly ascending according to
     * this field.
     */
    HZL_SET_BY_USER hzl_Gid_t gid;
    /** Padding to the next struct. */
    uint8_t unusedPadding[3];
} hzl_ClientGroupConfig_t;

/** Double-checking the size of the hzl_ClientGroupConfig_t struct to avoid
 * unexpected paddings. */
_Static_assert(sizeof(hzl_ClientGroupConfig_t) == 12,
               "The size of the Client Group Config struct must be exactly 12 B");

/**
 * Hazelnet Client variable State.
 *
 * Single instance per Group, multiple instances per Client.
 * Initialised, modified, managed and cleared fully by the Client:
 * the user MUST NOT touch its contents.
 */
typedef struct hzl_ClientGroupState
{
    /**
     * Random nonce used in the last Request (N^{req}_G).
     *
     * Non-zero when a Response is currently expected.
     */
    hzl_ReqNonce_t requestNonce;
    /**
     * Timestamp of when the Request was transmitted or when the Response was received (r_G).
     *
     * Used to check if its timeout is expired or when the old Session data should be cleared.
     */
    hzl_Timestamp_t lastHandshakeEventInstant;
    /**
     * Timestamp of when the last valid received secured message belonging to the currently
     * active Session was processed (m_G).
     *
     * Used to scale down the acceptable Counter Nonce interval.
     */
    hzl_Timestamp_t currentRxLastMessageInstant;
    /**
     * Timestamp of when the last valid received secured message belonging to the previously
     * active Session was processed (m^{old}_G), currently about to expire.
     *
     * Used to scale down the acceptable Counter Nonce interval.
     */
    hzl_Timestamp_t previousRxLastMessageInstant;
    /**
     * Counter Nonce of the the currently active Session (N^{ctr}_G).
     */
    hzl_CtrNonce_t currentCtrNonce;
    /**
     * Counter Nonce of the the previously active Session (N^{ctr,old}_G), currently
     * about to expire.
     */
    hzl_CtrNonce_t previousCtrNonce;
    /**
     * Short Term Key of the the currently active Session (STK_G).
     */
    uint8_t currentStk[HZL_LTK_LEN];
    /**
     * Short Term Key of the the previously active Session (STK^{old}_G), currently
     * about to expire.
     */
    uint8_t previousStk[HZL_LTK_LEN];
    /** Padding to the next struct. */
    uint8_t unusedPadding[4];
} hzl_ClientGroupState_t;

/** Double-checking the size of the hzl_ClientGroupState_t struct to avoid
 * unexpected paddings. */
_Static_assert(sizeof(hzl_ClientGroupState_t) == 64,
               "The size of the Client Group state struct must be exactly 64 B");

/**
 * Configuration and status of the HazelNet Client library.
 *
 * Initialised by the user on embedded, loaded from file on an OS.
 */
typedef struct hzl_ClientCtx
{
    /**
     * Pointer to **one** struct with the constant Client configuration.
     *
     * Set by the user to point to a valid configuration. Must be not NULL.
     */
    HZL_SET_BY_USER const hzl_ClientConfig_t* clientConfig;
    /**
     * Pointer to an **array** of structs, each with the constant configuration of one Group.
     *
     * The array must contain #hzl_ClientConfig_t.amountOfGroups elements
     * (structs). Set by the user to point to a valid configuration.
     * Must be not NULL.
     *
     * Constraints:
     * - structs must be sorted by the `gid` field strictly ascending without repetition
     * - the first struct (at index 0) must always be with `gid == 0`
     * - the array must contain at least one struct (thus the one with `gid == 0`)
     */
    HZL_SET_BY_USER const hzl_ClientGroupConfig_t* groupConfigs;
    /**
     * Pointer to an **array** of structs, each with the variable state of one Group.
     *
     * Set by the user to point to a memory location, does not have to be initialised. The Client
     * handles the initialisation on init and clears it at deinit. Must be not NULL.
     *
     * The array must contain #hzl_ClientConfig_t.amountOfGroups elements (structs),
     * which will be indexed in the same was as in the `groupConfigs` array; i.e.
     * `groupConfigs[i]` and `groupStates[i]` are respectively the configuration and state
     * of the same group, for every `i`.
     */
    HZL_SET_BY_USER hzl_ClientGroupState_t* groupStates;
    /**
     * Set of function pointers binding the API to the rest of the system.
     *
     * Including random number generation, timestamp generation and message transmission.
     */
    HZL_SET_BY_USER hzl_Io_t io;
} hzl_ClientCtx_t;

/**
 * Initialisation of the Client.
 *
 * To be called once before any other function.
 * Checks the configuration thoroughly, initialises the states.
 *
 * After initialisation, the Client will not have the Session information required to transmit a new
 * Secured message; a Request message must be transmitted first with the
 * hzl_ClientBuildRequest() function after initialisation and a Response must be received.
 *
 * @param [in, out] ctx prepared with the proper structs and function pointers by the user
 * before calling this function. Further initialisation performed by this function. See
 * #hzl_ClientCtx_t for details.
 *
 * @retval #HZL_OK on success.
 *
 * @retval #HZL_ERR_NULL_CTX
 * @retval #HZL_ERR_NULL_CONFIG_CLIENT
 * @retval #HZL_ERR_INVALID_HEADER_TYPE
 * @retval #HZL_ERR_ZERO_GROUPS
 * @retval #HZL_ERR_TOO_MANY_GROUPS_FOR_CONFIGURED_HEADER_TYPE
 * @retval #HZL_ERR_LTK_IS_ALL_ZEROS
 * @retval #HZL_ERR_SERVER_SID_ASSIGNED_TO_CLIENT
 * @retval #HZL_ERR_SID_TOO_LARGE_FOR_CONFIGURED_HEADER_TYPE
 * @retval #HZL_ERR_NULL_CONFIG_GROUPS
 * @retval #HZL_ERR_GIDS_ARE_NOT_PRESORTED_STRICTLY_ASCENDING
 * @retval #HZL_ERR_MISSING_GID_0
 * @retval #HZL_ERR_INVALID_MAX_CTRNONCE_DELAY
 * @retval #HZL_ERR_GID_TOO_LARGE_FOR_CONFIGURED_HEADER_TYPE
 * @retval #HZL_ERR_NULL_STATES_GROUPS
 * @retval #HZL_ERR_NULL_CURRENT_TIME_FUNC
 * @retval #HZL_ERR_NULL_TRNG_FUNC
 */
HZL_API hzl_Err_t
hzl_ClientInit(hzl_ClientCtx_t* ctx);

/**
 * Deinitialisation of the Client, securely clearing the state.
 *
 * To be called after the Client is not used anymore or before entering a low-power mode.
 *
 * Other fields of the context are untouched (configuration and IO functions), so the context may be
 * reused for another #hzl_ClientInit call after the deinitialisation.
 *
 * Clearing the state when not in use forces the Client to start new handshakes after waking up upon
 * transmitting a new Secured message. This can also be done manually with the
 * hzl_ClientBuildRequest() function.
 *
 * @param [in, out] ctx context with state to clear. Not NULL.
 *
 * @retval #HZL_OK on success.
 * @retval #HZL_ERR_NULL_CTX
 * @retval #HZL_ERR_NULL_CONFIG_CLIENT
 * @retval #HZL_ERR_NULL_STATES_GROUPS
 */
HZL_API hzl_Err_t
hzl_ClientDeInit(hzl_ClientCtx_t* ctx);

/**
 * Builds a Request message, asking for the Session information for a specific group,
 * even if the Session is already established, unless a handshake is already ongoing.
 *
 * The transmission of a Request is mandatory before hzl_ClientBuildSecuredFd() can be used.
 * This function is useful to perform the handshake in advance even if there is no Secured
 * Application Data to transmit right now, to spare time later one when there is some to transmit.
 *
 * @param [out] requestPdu REQ message in packed format, ready to transmit. Not NULL.
 * @param [in, out] ctx to access configurations and update the group states. Not NULL.
 * @param [in] groupId destination group identifier (the Parties that will be able to decrypt).
 *
 * @retval #HZL_OK on success.
 * @retval #HZL_ERR_HANDSHAKE_ONGOING if a handshake is already ongoing right now. It should
 *         be enough to just wait for the current handshake to complete, no need to build a new
 *         Request.
 * @retval Same values as hzl_ClientInit() in case the context has NULL pointers.
 * @retval #HZL_ERR_NULL_PDU if \p requestPdu is NULL.
 * @retval #HZL_ERR_GID_TOO_LARGE_FOR_CONFIGURED_HEADER_TYPE when \p groupId
 *         would not fit in the packed CBS header.
 * @retval #HZL_ERR_UNKNOWN_GROUP when \p group is not supported in the context's
 *         configuration.
 * @retval #HZL_ERR_CANNOT_GENERATE_RANDOM
 * @retval #HZL_ERR_CANNOT_GENERATE_NON_ZERO_RANDOM
 * @retval #HZL_ERR_CANNOT_GET_CURRENT_TIME
 */
HZL_API hzl_Err_t
hzl_ClientBuildRequest(hzl_CbsPduMsg_t* requestPdu,
                       hzl_ClientCtx_t* ctx,
                       hzl_Gid_t groupId);

/**
 * Builds an **unsecured** message in plaintext.
 *
 * This message may be transmitted at **any point in time**, as long as the context is
 * initialised. This means even before or during a handshake for the given Group.
 *
 * The message may be built and transmitted **even if the \p group does not exist** in the
 * configuration. This is allowed in order to communicate with some devices that don't have a
 * preconfigured Group in common with this one. No error is raised.
 *
 * @warning Use it only on data that was already secured by other means by the application or
 * for debugging purposes. The usage of this type of message to transmit plaintext data is
 * strongly discouraged.
 *
 * @param [out] unsecuredPdu UAD message in packed format, ready to transmit. Not NULL.
 * @param [in] ctx just to access configurations and functions. No change in the state. Not NULL.
 * @param [in] userData plaintext data (SDU) to pack not-obfuscated, not-authenticated. Can be
 *             NULL only if \p userDataLen is zero.
 * @param [in] userDataLen length of \p userData in bytes.
 * @param [in] groupId destination group identifier (expected receivers). It may also be a GID
 *        **not** listed in the context's configuration.
 *
 * @retval #HZL_OK on success.
 * @retval Same values as hzl_ClientInit() in case the context has NULL pointers.
 * @retval #HZL_ERR_NULL_PDU if \p unsecuredPdu is NULL.
 * @retval #HZL_ERR_NULL_SDU if \p userData is NULL and \p userDataLen is > 0.
 * @retval #HZL_ERR_TOO_LONG_SDU if \p userDataLen is too large for the underlying PDU.
 * @retval #HZL_ERR_GID_TOO_LARGE_FOR_CONFIGURED_HEADER_TYPE when \p groupId
 *         would not fit in the packed CBS header.
 */
HZL_API hzl_Err_t
hzl_ClientBuildUnsecured(hzl_CbsPduMsg_t* unsecuredPdu,
                         const hzl_ClientCtx_t* ctx,
                         const uint8_t* userData,
                         size_t userDataLen,
                         hzl_Gid_t groupId);

/**
 * Builds a secured message, encrypted, authenticated and timely, only for the given group
 * to be able to read.
 *
 * Before using this function, a Request message must be built with hzl_ClientBuildRequest(),
 * transmitted and a Response must be received and processed with hzl_ClientProcessReceived().
 *
 * @param [out] securedPdu CBS message in packed format, ready to transmit. Not NULL.
 * @param [in, out] ctx to access configurations and update the group states. Not NULL.
 * @param [in] userData plaintext data (SDU) to pack encrypted and authenticated. Can be
 *             NULL only if \p userDataLen is zero.
 * @param [in] userDataLen length of \p userData in bytes.
 * @param [in] groupId destination group identifier (the Parties that can decrypt).
 *
 * @retval #HZL_OK on successful building of the Secured Application Data.
 * @retval #HZL_ERR_SESSION_NOT_ESTABLISHED if the message could not be built yet,
 *         a Request must be started with a Request message and a Response must be received.
 *         Use hzl_ClientBuildRequest() first.
 * @retval Same values as hzl_ClientInit() in case the context has NULL pointers.
 * @retval #HZL_ERR_NULL_PDU if \p securedPdu is NULL.
 * @retval #HZL_ERR_NULL_SDU if \p userData is NULL and \p userDataLen is > 0.
 * @retval #HZL_ERR_TOO_LONG_SDU if \p userDataLen is too large for the underlying PDU.
 * @retval #HZL_ERR_GID_TOO_LARGE_FOR_CONFIGURED_HEADER_TYPE when \p groupId
 *         would not fit in the packed CBS header.
 * @retval #HZL_ERR_UNKNOWN_GROUP when \p group is not supported in the context's
 *         configuration.
 * @retval #HZL_ERR_CANNOT_GENERATE_RANDOM
 * @retval #HZL_ERR_CANNOT_GENERATE_NON_ZERO_RANDOM
 * @retval #HZL_ERR_CANNOT_GET_CURRENT_TIME
 */
HZL_API hzl_Err_t
hzl_ClientBuildSecuredFd(hzl_CbsPduMsg_t* securedPdu,
                         hzl_ClientCtx_t* ctx,
                         const uint8_t* userData,
                         size_t userDataLen,
                         hzl_Gid_t groupId);

/**
 * Validates, unpacks and decrypts (if necessary) any received message, preparing an automatic
 * response when required.
 *
 * Handles automatically messages of different types, making a variety of checks for the
 * correctness, integrity, authenticity of the message (note: the latter is not guaranteed for
 * Unsecured messages). Updates the internal state automatically to keep track of the changing
 * nonces and Session states.
 *
 * @param [out] reactionPdu CBS message in packed format, ready to transmit, generated as
 *        an automatic, internal reaction to the just received message. Not NULL.
 *        No need to transmit if #hzl_CbsPduMsg_t.dataLen is zero. Not NULL.
 * @param [out] receivedUserData plaintext user data extracted out of \p receivedPdu.
 *        If the processed message contained no user data (was an internal message),
 *        the field #hzl_RxSduMsg_t.isForUser is false. The whole struct is securely
 *        cleared (zeroed out) before anything else is attempted; thus it's full of zeros
 *        in case of errors. This is done to clear any lingering data if the buffer is reused,
 *        to avoid leaking information about previously-decrypted messages. Not NULL.
 * @param [in, out] ctx to access configurations and update the group states. Not NULL.
 * @param [in] receivedPdu packed CBS message as received from the underlying layer. Not NULL.
 * @param [in] receivedPduLen length of \p receivedPdu in bytes.
 * @param [in] receivedCanId identifier of the underlying layer's PDU, passed as-is
 *        to \p receivedUserData.
 *
 * @retval #HZL_OK on success.
 * @retval #HZL_ERR_MSG_IGNORED when the message has another destination.
 * @retval Same values as hzl_ClientInit() in case the context has NULL pointers.
 * @retval #HZL_ERR_NULL_PDU if \p receivedUserData or \p receivedPduLen are NULL.
 * @retval #HZL_ERR_NULL_SDU if \p userData is NULL.
 * @retval #HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_HEADER when the \p canFdMsg->dataLen is too
 *         short to even contain a CBS message with the currently configured Header Type in the ctx.
 * @retval #HZL_ERR_INVALID_PAYLOAD_TYPE on unsupported PTY field in the CBS Header.
 * @retval #HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_SADFD,
 *         #HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_RES,
 *         #HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_REN when the received message is too short
 *         to contain the data is should as indicated in its header (or also in the payload length
 *         field in case of Secured Application Data messages).
 * @retval #HZL_ERR_TOO_LONG_CIPHERTEXT if the received Secured Application Data message
 *         claims to contain too much data to fit into the underlying layer's message.
 * @retval #HZL_ERR_SESSION_NOT_ESTABLISHED when the current state indicates no
 *         session key and counter nonce have been established for this group yet.
 * @retval #HZL_ERR_SECWARN_MESSAGE_FROM_MYSELF when the received message seems to originate
 *         from the receiver itself.
 * @retval #HZL_ERR_SECWARN_SERVER_ONLY_MESSAGE when the received message that can only
 *         be sent by the server but is has another SID
 * @retval #HZL_ERR_SECWARN_RECEIVED_OVERFLOWN_NONCE if the received nonce is overflown
 * @retval #HZL_ERR_SECWARN_OLD_MESSAGE if the received nonce is old, indicating a potential
 *         replay attack
 * @retval #HZL_ERR_SECWARN_INVALID_TAG when the message integrity and authenticity
 *         cannot be guaranteed
 */
HZL_API hzl_Err_t
hzl_ClientProcessReceived(hzl_CbsPduMsg_t* reactionPdu,
                          hzl_RxSduMsg_t* receivedUserData,
                          hzl_ClientCtx_t* ctx,
                          const uint8_t* receivedPdu,
                          size_t receivedPduLen,
                          hzl_CanId_t receivedCanId);

#ifdef __cplusplus
}
#endif

#endif  /* HZL_CLIENT_H_ */
