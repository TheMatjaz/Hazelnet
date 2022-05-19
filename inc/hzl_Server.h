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
 * Hazelnet Server public API.
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

#ifndef HZL_SERVER_H_
#define HZL_SERVER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "hzl.h"

/**
 * Maximum amount of Clients overall this Server supports.
 *
 * At Context initialisation the amount of Clients in #hzl_ServerConfig_t
 * may be any value <= this limit.
 *
 * The per-Group configuration #hzl_ServerGroupConfig_t contains a
 * static-sized bitmap of Clients in it which is limited to 32 bits to spare
 * memory. One bit is reserved for the Server.
 */
#define HZL_SERVER_MAX_AMOUNT_OF_CLIENTS 32U

/**
 * Integer data type used for the bitmap of Clients that must support
 * #HZL_SERVER_MAX_AMOUNT_OF_CLIENTS bits.
 */
typedef uint32_t hzl_ServerBitMap_t;

/**
 * Largest Max-Counter-Nonce value allowed in the Server configuration.
 *
 * 2^22 = 4'194'304. Unitless.
 */
#define HZL_SERVER_LARGEST_MAX_COUNTER_NONCE_DELAY (1U << 22U)

/**
 * Largest Counter-Nonce-Upper-Limit value allowed in the Server configuration.
 *
 * 2^24 - 2^7 = 16777088 = 0xFFFF80, i.e. 128 less than the counter nonce max.
 */
#define HZL_SERVER_MAX_COUNTER_NONCE_UPPER_LIMIT 0xFFFF80U

/**
 * Hazelnet Server constant configuration.
 *
 * Single instance on the Server.
 * Initialised by the user, not modified by the Server.
 */
typedef struct hzl_ServerConfig
{
    /**
     * Amount of #hzl_ServerGroupConfig_t instances.
     *
     * Must be >= 1.
     */
    HZL_SET_BY_USER uint8_t amountOfGroups;
    /**
     * Amount of Clients this Server knows, i.e. amount of
     * #hzl_ServerClientsConfig instances.
     *
     * Must be in [1, #HZL_SERVER_MAX_AMOUNT_OF_CLIENTS].
     */
    HZL_SET_BY_USER uint8_t amountOfClients;
    /**
     * Header Type (h) used by the network of CBS-enabled nodes.
     *
     * Must be the same as all other nodes, otherwise the messages will not
     * be interpreted correctly, both sent and received.
     * Must be one of #hzl_HeaderType_t enum fields.
     */
    HZL_SET_BY_USER uint8_t headerType;
} hzl_ServerConfig_t;

/** Double-checking the size of the hzl_ServerConfig_t struct to avoid
 * unexpected paddings. */
_Static_assert(sizeof(hzl_ServerConfig_t) == 3,
               "The size of the Server Config struct must be exactly 3 B");

/**
 * Hazelnet Server constant per-Client configuration.
 *
 * Unique per Client, multiple instances on the Server.
 * Initialised by the user, not modified by the Server.
 */
typedef struct hzl_ServerClientsConfig
{
    /**
     * Source Identifier of the Client.
     *
     * Unique across all Clients. Must not be #HZL_SERVER_SID.
     *
     * An array of these structs must be sorted so that each array index+1
     * matches this field. In other words, the structs must be sorted
     * by a strictly ascending SID, starting from 1 (included) and without
     * gaps.
     *
     * This field is not strictly required (can be inferred from the array
     * index), but it helps during debugging.
     */
    HZL_SET_BY_USER hzl_Sid_t sid;
    /**
     * Symmetric Long Term Key of the Client X (LTK_{XS}), uniquely
     * identifying the Client against the Server.
     *
     * Only the Client has another copy. Must not be all-zeros.
     */
    HZL_SET_BY_USER uint8_t ltk[HZL_LTK_LEN];
} hzl_ServerClientConfig_t;

/** Double-checking the size of the hzl_ServerClientConfig_t struct to avoid
 *  unexpected paddings. */
_Static_assert(sizeof(hzl_ServerClientConfig_t) == 17,
               "The size of the Server-side Client Config struct must be exactly 17 B");

/**
 * Hazelnet Server constant Group configuration.
 *
 * Unique per Group, multiple instances on the Server.
 * Initialised by the user, not modified by the Server.
 */
typedef struct hzl_ServerGroupConfig
{
    /**
     * Maximum Counter Nonce Delay (D^{max}_G) in milliseconds,
     * used to filter out recent messages from old ones.
     *
     * Must be in [0, #HZL_SERVER_LARGEST_MAX_COUNTER_NONCE_DELAY].
     */
    HZL_SET_BY_USER uint32_t maxCtrnonceDelayMsgs;
    /**
     * Counter Nonce upper limit (N^{exp}_G), used to know when the current
     * Session expires due to the amount of messages sent.
     *
     * Must be in [0, #HZL_SERVER_MAX_COUNTER_NONCE_UPPER_LIMIT].
     */
    HZL_SET_BY_USER hzl_CtrNonce_t ctrNonceUpperLimit;
    /**
     * Server-Side Session time Duration (s^{exp}_G) in milliseconds.
     *
     * Used to know when the current Session expires due to the enough time
     * passed since its establishment.
     *
     * May have any value in [0, 0xFFFFFFFF].
     */
    HZL_SET_BY_USER uint32_t sessionDurationMillis;
    /**
     * Delay between consecutive Session renewal notification (REN) messages
     * (t^{ntf}_G) in milliseconds.
     *
     * Used to know when to send the next REN message.
     *
     * Must be in ]0, floor(sessionDurationMillis/6)[.
     */
    HZL_SET_BY_USER uint32_t delayBetweenRenNotificationsMillis;
    /**
     * Bitmap of the Client SIDs included in this Group.
     *
     * If the bit is set (1), it's index is the SID of the included Client.
     * The first bit (index 0, least significant) indicates the Client with SID == 1;
     * the i-th bit (representing 2^i) indicates the Client with SID i.
     *
     * Constraints:
     * - Each Group must contain at least one Client, thus have at least one bit set.
     * - Each Group must contain only known Clients, thus is a SID is indicated in the bitmap,
     *   it must appear in the array of #hzl_ServerClientConfig_t structs.
     * - The broadcast Group with GID == #HZL_BROADCAST_GID must contain all the
     *   Clients listed in the array of #hzl_ServerClientConfig_t structs but can contain
     *   more than that (the extra bits are ignored). This allows to simply set all bits
     *   of the broadcast bitmap always.
     * - Each Client may appear in more than one Group.
     */
    HZL_SET_BY_USER hzl_ServerBitMap_t clientSidsInGroupBitmap;
    /**
     * Maximum Silence Interval (S^{max}_G) in milliseconds,
     * used to filter out recent messages from old ones.
     *
     * May have any value in [0, 0xFFFF].
     */
    HZL_SET_BY_USER uint16_t maxSilenceIntervalMillis;
    /**
     * Group Identifier of this Group (id_G).
     *
     * An array of these structs must be sorted so that each array index
     * matches this field. In other words, the structs must be sorted
     * by a strictly ascending GID, starting from 0 (included) and without
     * gaps.
     *
     * This field is not strictly required (can be inferred from the array
     * index), but it's using "free" space that would be anyhow occupied
     * by the padding at the struct end until the next one, and it helps
     * during debugging.
     */
    HZL_SET_BY_USER hzl_Gid_t gid;
    /** Padding to the next struct. */
    uint8_t unusedPadding[1];
} hzl_ServerGroupConfig_t;

/** Double-checking the size of the hzl_ServerGroupConfig_t struct to avoid
 *  unexpected paddings. */
_Static_assert(sizeof(hzl_ServerGroupConfig_t) == 24,
               "The size of the Server Group Config struct must be exactly 24 B");

/**
 * Hazelnet Server variable State.
 *
 * Single instance per Group, multiple instances per Server.
 * Initialised, modified, managed and cleared fully by the Server:
 * the user MUST NOT touch its contents.
 */
typedef struct hzl_ServerGroupState
{
    /**
     * Timestamp of when the Session was started.
     *
     * Used to check when it expires.
     */
    hzl_Timestamp_t sessionStartInstant;
    /**
     * Timestamp of when the last valid received secured message belonging to the currently
     * active Session was processed (m_G).
     *
     * Used to scale down the acceptable Counter Nonce interval and to known if at least
     * one Request was received from the Clients of this Group.
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
     * Short Term Key of the currently active Session (STK_G).
     */
    uint8_t currentStk[HZL_LTK_LEN];
    /**
     * Short Term Key of the the previously active Session (STK^{old}_G), currently
     * about to expire.
     */
    uint8_t previousStk[HZL_LTK_LEN];
} hzl_ServerGroupState_t;

/** Double-checking the size of the hzl_ServerGroupState_t struct to avoid
 *  unexpected paddings. */
_Static_assert(sizeof(hzl_ServerGroupState_t) == 52,
               "The size of the Server Group State struct must be exactly 52 B");

/**
 * Configuration and status of the HazelNet Server library.
 *
 * Initialised by the user on embedded, loaded from file on an OS.
 */
typedef struct hzl_ServerCtx
{
    /**
     * Pointer to **one** struct with the constant Server configuration.
     *
     * Set by the user to point to a valid configuration. Must be not NULL.
     */
    HZL_SET_BY_USER const hzl_ServerConfig_t* serverConfig;
    /**
     * Pointer to an **array** of structs, each with the constant configuration
     * of one Client.
     *
     * The array must contain #hzl_ServerConfig_t.amountOfClients elements
     * (structs). Set by the user to point to a valid configuration.
     * Must be not NULL.
     *
     * Constraints:
     * - Structs must be sorted by the `sid` field strictly ascending without repetition
     *   and without gaps, starting from the `sid == 1` (as `sid == 0` is the Server itself).
     *   In other words, the array index `i` must contain the struct with `sid == i+1`.
     * - The first struct (at index 0) must always be with `sid == 1`.
     * - The array must contain at least one struct (thus the one with `gid == 1`).
     */
    HZL_SET_BY_USER const hzl_ServerClientConfig_t* clientConfigs;
    /**
     * Pointer to an **array** of structs, each with the constant configuration
     * of one Group.
     *
     * The array must contain #hzl_ServerConfig_t.amountOfGroups elements
     * (structs). Set by the user to point to a valid configuration.
     * Must be not NULL.
     *
     * Constraints:
     * - Structs must be sorted by the `gid` field strictly ascending without repetition
     *   and without gaps. In other words, the array index `i` must
     *   contain the struct with `gid == i`.
     * - The first struct (at index 0) must always be with `gid == 0`.
     * - The array must contain at least one struct (thus the one with `gid == 0`).
     */
    HZL_SET_BY_USER const hzl_ServerGroupConfig_t* groupConfigs;
    /**
     * Pointer to an **array** of structs, each with the variable state of one Group.
     *
     * Set by the user to point to a memory location, does not have to be initialised. The Server
     * handles the initialisation on init and clears it at deinit. Must be not NULL.
     *
     * The array must contain #hzl_ServerConfig_t.amountOfGroups elements (structs),
     * which will be indexed in the same was as in the `groupConfigs` array; i.e.
     * `groupConfigs[i]` and `groupStates[i]` are respectively the configuration and state
     * of the same group, for every `i`.
     */
    HZL_SET_BY_USER hzl_ServerGroupState_t* groupStates;
    /**
     * Set of function pointers binding the API to the rest of the system.
     *
     * Including random number generation, timestamp generation and message transmission.
     */
    HZL_SET_BY_USER hzl_Io_t io;
} hzl_ServerCtx_t;

/**
 * Initialisation of the Server.
 *
 * To be called once before any other function.
 * Checks the configuration thoroughly, initialises the states.
 *
 * After initialisation, the Server will have all Session information generated and ready to
 * transmit Responses upon Requests and ready to transmit a new Secured message.
 *
 * @warning
 * The Server must be fully initialised before the Clients, otherwise it cannot supply the
 * Session information to them upon Request.
 *
 * @param [in, out] ctx prepared with the proper structs and function pointers by the user
 * before calling this function. Further initialisation performed by this function. See
 * #hzl_ServerCtx_t for details.
 *
 * @retval #HZL_OK on success.
 *
 * @retval #HZL_ERR_NULL_CTX
 * @retval #HZL_ERR_NULL_CONFIG_SERVER
 * @retval #HZL_ERR_INVALID_HEADER_TYPE
 * @retval #HZL_ERR_ZERO_GROUPS
 * @retval #HZL_ERR_TOO_MANY_GROUPS_FOR_CONFIGURED_HEADER_TYPE
 * @retval #HZL_ERR_ZERO_CLIENTS
 * @retval #HZL_ERR_TOO_MANY_CLIENTS_FOR_CONFIGURED_HEADER_TYPE
 * @retval #HZL_ERR_NULL_CONFIG_CLIENTS
 * @retval #HZL_ERR_LTK_IS_ALL_ZEROS
 * @retval #HZL_ERR_SERVER_SID_ASSIGNED_TO_CLIENT
 * @retval #HZL_ERR_SIDS_ARE_NOT_PRESORTED_STRICTLY_ASCENDING
 * @retval #HZL_ERR_GAP_IN_SIDS
 * @retval #HZL_ERR_NULL_CONFIG_GROUPS
 * @retval #HZL_ERR_GIDS_ARE_NOT_PRESORTED_STRICTLY_ASCENDING
 * @retval #HZL_ERR_GAP_IN_GIDS
 * @retval #HZL_ERR_MISSING_GID_0
 * @retval #HZL_ERR_INVALID_MAX_CTRNONCE_DELAY
 * @retval #HZL_ERR_TOO_LARGE_CTRNONCE_UPPER_LIMIT
 * @retval #HZL_ERR_INVALID_DELAY_BETWEEN_REN_NOTIFICATIONS
 * @retval #HZL_ERR_CLIENTS_BITMAP_ZERO_CLIENTS
 * @retval #HZL_ERR_CLIENTS_BITMAP_UNKNOWN_SID
 * @retval #HZL_ERR_CLIENTS_BITMAP_INVALID_BROADCAST_GROUP
 * @retval #HZL_ERR_NULL_STATES_GROUPS
 * @retval #HZL_ERR_NULL_CURRENT_TIME_FUNC
 * @retval #HZL_ERR_NULL_TRNG_FUNC
 * @retval #HZL_ERR_CANNOT_GET_CURRENT_TIME
 * @retval #HZL_ERR_CANNOT_GENERATE_RANDOM
 * @retval #HZL_ERR_CANNOT_GENERATE_NON_ZERO_RANDOM
 */
HZL_API hzl_Err_t
hzl_ServerInit(hzl_ServerCtx_t* ctx);

/**
 * Deinitialisation of the Server, securely clearing the state.
 *
 * To be called after the Server is not used anymore or before entering a low-power mode.
 *
 * Other fields of the context are untouched (configuration and IO functions), so the context may be
 * reused for another hzl_ServerInit() call after the deinitialisation.
 *
 * @warning
 * Deinitialising the Server means that no Client will be able to obtain the current session
 * information, effectively preventing handshakes and thus secure communication. Be sure to run
 * it only when the Server is really not in use anymore by any Client.
 *
 * @param [in, out] ctx context with state to clear. Not NULL.
 *
 * @retval #HZL_OK on success.
 * @retval #HZL_ERR_NULL_CTX
 * @retval #HZL_ERR_NULL_CONFIG_SERVER
 * @retval #HZL_ERR_NULL_STATES_GROUPS
 */
HZL_API hzl_Err_t
hzl_ServerDeInit(hzl_ServerCtx_t* ctx);

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
 * @retval Same values as hzl_ServerInit() in case the context has NULL pointers.
 * @retval #HZL_ERR_NULL_PDU if \p unsecuredPdu is NULL.
 * @retval #HZL_ERR_NULL_SDU if \p userData is NULL and \p userDataLen is > 0.
 * @retval #HZL_ERR_TOO_LONG_SDU if \p userDataLen is too large for the underlying PDU.
 * @retval #HZL_ERR_GID_TOO_LARGE_FOR_CONFIGURED_HEADER_TYPE when \p groupId
 *         would not fit in the packed CBS header.
 */
HZL_API hzl_Err_t
hzl_ServerBuildUnsecured(hzl_CbsPduMsg_t* unsecuredPdu,
                         const hzl_ServerCtx_t* ctx,
                         const uint8_t* userData,
                         size_t userDataLen,
                         hzl_Gid_t groupId);

/**
 * Builds a secured message, encrypted, authenticated and timely, only for the given group
 * to be able to read.
 *
 * @param [out] securedPdu CBS message in packed format, ready to transmit. Not NULL.
 * @param [in, out] ctx to access configurations and update the group states. Not NULL.
 * @param [in] userData plaintext data (SDU) to pack encrypted and authenticated. Can be
 *             NULL only if \p userDataLen is zero.
 * @param [in] userDataLen length of \p userData in bytes.
 * @param [in] groupId destination group identifier (the Parties that can decrypt).
 *
 * @retval #HZL_OK on successful building of the Secured Application Data.
 * @retval #HZL_ERR_NO_POTENTIAL_RECEIVER if no Request was received yet from any Client
 *         in the requested Group \p groupId. If transmitted, the message could not be
 *         decrypted by anyone, because no Client has yet the Session information.
 * @retval Same values as hzl_ServerInit() in case the context has NULL pointers.
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
hzl_ServerBuildSecuredFd(hzl_CbsPduMsg_t* securedPdu,
                         hzl_ServerCtx_t* ctx,
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
 *        an automatic, internal reaction to the just received message.
 *        No need to transmit if #hzl_CbsPduMsg_t.dataLen is zero. Not NULL.
 *        **Transmit any data even if the return error code is non-OK.**
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
 * @retval Same values as hzl_ServerInit() in case the context has NULL pointers.
 * @retval #HZL_ERR_NULL_PDU if \p receivedUserData or \p receivedPduLen are NULL.
 * @retval #HZL_ERR_NULL_SDU if \p userData is NULL.
 * @retval #HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_HEADER when the \p canFdMsg->dataLen is too
 *         short to even contain a CBS message with the currently configured Header Type in the ctx.
 * @retval #HZL_ERR_INVALID_PAYLOAD_TYPE on unsupported PTY field in the CBS Header.
 * @retval #HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_SADFD,
 *         #HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_REQ when the received message is too short
 *         to contain the data is should as indicated in its header (or also in the payload length
 *         field in case of Secured Application Data messages).
 * @retval #HZL_ERR_TOO_LONG_CIPHERTEXT if the received Secured Application Data message
 *         claims to contain too much data to fit into the underlying layer's message.
 * @retval #HZL_ERR_SESSION_NOT_ESTABLISHED when the current state indicates no
 *         session key and counter nonce have been established for this group yet.
 * @retval #HZL_ERR_SECWARN_MESSAGE_FROM_MYSELF when the received message seems to originate
 *         from the receiver itself.
 * @retval #HZL_ERR_SECWARN_SERVER_ONLY_MESSAGE when the received message that can only
 *         be sent by the server
 * @retval #HZL_ERR_SECWARN_RECEIVED_OVERFLOWN_NONCE if the received nonce is overflown
 * @retval #HZL_ERR_SECWARN_OLD_MESSAGE if the received nonce is old, indicating a potential
 *         replay attack
 * @retval #HZL_ERR_SECWARN_INVALID_TAG when the message integrity and authenticity
 *         cannot be guaranteed
 * @retval #HZL_ERR_CANNOT_GENERATE_RANDOM
 * @retval #HZL_ERR_CANNOT_GENERATE_NON_ZERO_RANDOM
 * @retval #HZL_ERR_CANNOT_GET_CURRENT_TIME
 */
HZL_API hzl_Err_t
hzl_ServerProcessReceived(hzl_CbsPduMsg_t* reactionPdu,
                          hzl_RxSduMsg_t* receivedUserData,
                          hzl_ServerCtx_t* ctx,
                          const uint8_t* receivedPdu,
                          size_t receivedPduLen,
                          hzl_CanId_t receivedCanId);

/**
 * Forcibly start a Session Renewal Phase, unless one is already ongoing or no Clients
 * are currently enabled (have Requested the STK) to process the REN message.
 *
 * @param [out] renewalPdu REN message in packed format, ready to transmit. Not NULL.
 * @param [in, out] ctx to access configurations and update the group states. Not NULL.
 * @param [in] groupId destination group identifier (the Parties that can decrypt).
 *
 * @retval #HZL_OK on success.
 * @retval #HZL_ERR_NO_POTENTIAL_RECEIVER if no Request was received yet from any Client
 *         in the requested Group \p groupId. If transmitted, the message could not be
 *         validated by anyone, because no Client has yet the Session information.
 * @retval Same values as hzl_ServerInit() in case the context has NULL pointers.
 * @retval #HZL_ERR_NULL_PDU if \p renewalPdu is NULL.
 * @retval #HZL_ERR_UNKNOWN_GROUP when \p group is not supported in the context's
 *         configuration.
 * @retval #HZL_ERR_CANNOT_GET_CURRENT_TIME
 */
HZL_API hzl_Err_t
hzl_ServerForceSessionRenewal(hzl_CbsPduMsg_t* renewalPdu,
                              hzl_ServerCtx_t* ctx,
                              hzl_Gid_t groupId);


#ifdef __cplusplus
}
#endif

#endif  /* HZL_SERVER_H_ */
