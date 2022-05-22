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
 * General public definitions used used by both the Hazelnet Server and Client.
 */

#ifndef HZL_H_
#define HZL_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Hazelnet Client and Server library version using semantic versioning.
 */
#define HZL_VERSION "v3.0.1"

/** Version of the CAN Bus Security protocol that the Client and Server libraries implement. */
#define HZL_CBS_PROTOCOL_VERSION_SUPPORTED "v1.3"

// Dependencies (partial list: more includes in the if-elif-endif checking the OS below)
#include <stdint.h>  /* For uint8_t, uint16_t etc. */
#include <stdbool.h> /* For bool, true, false */
#include <stddef.h>  /* For NULL, size_t */
#include <string.h>  /* For memcpy(), if available also memset_s() */

/**
 * @def HZL_OS_AVAILABLE
 * True when a desktop operating system was found.
 *
 * This implies that the following features are available and Hazelnet will
 * use them: file system, malloc, real time clock and true-random number
 * generator.
 *
 * Dev note 2021-01-13: tried replacing the OS-dependant current-time function
 * with the standard C11 `timespec_get()` from `time.h`, but it does not work:
 * the function cannot be found by the linker.
 */
/**
 * @def HZL_OS_AVAILABLE_WIN
 * True when a Windows operating system was found.
 * @see #HZL_OS_AVAILABLE
 */
/**
 * @def HZL_OS_AVAILABLE_NIX
 * True when a Unix-like operating system was found, e.g. Linux, macOS, *BSD
 * @see #HZL_OS_AVAILABLE
 */

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) \
 || defined(_WIN64) || defined(__NT__)

#define HZL_OS_AVAILABLE 1
#define HZL_OS_AVAILABLE_WIN 1
#define HZL_OS_AVAILABLE_NIX 0

#include <Windows.h> /* To avoid compilation errors for other Windows headers. */
#include <sysinfoapi.h> /* For GetSystemTimeAsFileTime(), FILETIME, ULARGE_INTEGER */
#include <bcrypt.h>  /* For BCryptGenRandom() - requires explicit linking to `bcrypt` lib. */
#include <stdio.h>   /* For config file IO */
#include <stdlib.h>  /* For calloc(), free() */

#if defined(_MSC_VER) && _MSC_VER <= 1916
// Fix for compilation with Visual Studio 2017 not supporting C11 yet.
#define _Static_assert static_assert
#endif

#elif defined(__linux__) \
 || defined(__APPLE__) || defined(__MACH__) \
 || defined(__unix__) || defined(__unix) || defined(unix)

#define HZL_OS_AVAILABLE 1
#define HZL_OS_AVAILABLE_WIN 0
#define HZL_OS_AVAILABLE_NIX 1

#include <sys/time.h> /* For gettimeofday() */
#include <stdio.h>    /* For config file IO and TRNG with /dev/urandom */
#include <stdlib.h>   /* For calloc(), free() */

#else

#define HZL_OS_AVAILABLE 0
#define HZL_OS_AVAILABLE_WIN 0
#define HZL_OS_AVAILABLE_NIX 0

#endif

/** @def HZL_API
 * Identifier of the public library API functions.
 * Used to add any exporting keywords in front of the functions for DLL compilation,
 * which happens on Windows-only. Empty on other OS. */
#if HZL_OS_AVAILABLE_WIN
#define HZL_API __declspec(dllexport)
#else
#define HZL_API
#endif

/** Identifier of the struct fields of the public API the user must set manually. */
#define HZL_SET_BY_USER

/** Source Identifier of the Server, always zero. */
#define HZL_SERVER_SID 0U

/** Group Identifier reserved for broadcasting, always zero. */
#define HZL_BROADCAST_GID 0U

/** Length of the Long Term Key in bytes. */
#define HZL_LTK_LEN 16U

/** Length of the Short Term Key in bytes. */
#define HZL_STK_LEN 16U

/** Macro checking whether the #hzl_Err_t error code is a
 * standard CBS security warning (case: true). False otherwise. */
#define HZL_IS_SECURITY_WARNING(err) ((err) >= 1 && (err) <= 15)

/** Maximum length of the CAN FD frame's payload in bytes. */
#define HZL_MAX_CAN_FD_DATA_LEN 64U

/**
 * Amount of consecutive TRNG invocations that must provide all-zero bytes to give up
 * the random number generation. The probability that this happens is quit low:
 * Assuming that each TRNG call generates only 1 byte (which is not always the case,
 * often is more), there is 1/(2^8) chance for that byte to be zero and
 * 1/(256^maxTries) for every byte to be zero. For 20 tries, this is 1/(2^8^20)
 * == 1/(2^160), which is far less common than guessing the correct AES-128 key
 * of a ciphertext.
 */
#define HZL_MAX_TRNG_TRIES_FOR_NONZERO_VALUE 20U

/**
 * Largest Max-Counter-Nonce value allowed in the Group configurations.
 *
 * 2^22 = 4'194'304. Unitless.
 */
#define HZL_LARGEST_MAX_COUNTER_NONCE_DELAY (1U << 22U)

/** Hazelnet error code, returned by all API functions. */
typedef enum hzl_Err
{
    // Success
    HZL_OK = 0U,  //!< Successful completion of the operation

    // CBS standard security warnings
    /** Received message is not intact or not authentic or does not use
     * the proper values in the hashing/authenticating operations.
     * CBS standard security warning "INV". */
    HZL_ERR_SECWARN_INVALID_TAG = 1U,
    /** Received message contained the receiver's Source Identfier as the transmitter's identity.
     * CBS standard security warning "MFM". */
    HZL_ERR_SECWARN_MESSAGE_FROM_MYSELF = 2U,
    /** The Client received a Response addressed to it while not expecting any.
     * Client-side only. CBS standard security warning "NER". */
    HZL_ERR_SECWARN_NOT_EXPECTING_A_RESPONSE = 3U,
    /** Received message is of a type which only the Session Server can transmit, but the
     * transmitter's identity was not the Session Server. CBS standard security warning "SOM". */
    HZL_ERR_SECWARN_SERVER_ONLY_MESSAGE = 4U,
    /** The Client did not receive a Response to its Request within the preconfigured timeout.
     * Client-side only. CBS standard security warning "RTO". */
    HZL_ERR_SECWARN_RESPONSE_TIMEOUT = 5U,
    /** Received message contained a too-old counter nonce. CBS standard security warning "OLD". */
    HZL_ERR_SECWARN_OLD_MESSAGE = 6U,
    /** The Party is receiving too many suspect messages. CBS standard security warning "DOS".
     * NOTE: the Client library does NOT implement this check. */
    HZL_ERR_SECWARN_DENIAL_OF_SERVICE = 7U,
    /** The Client the Request originated from does not belong into the requested Group.
     * Server-side only. CBS standard security warning "NIG". */
    HZL_ERR_SECWARN_NOT_IN_GROUP = 8U,
    /** Received message contained a Counter Nonce that exceeded its maximum allowed value.
     * CBS standard security warning "RON". */
    HZL_ERR_SECWARN_RECEIVED_OVERFLOWN_NONCE = 9U,
    /** Received Response message, once decrypted, contained an all-zeros STKG that cannot be used.
     * Client-side only. CBS standard security warning "RZK". */
    HZL_ERR_SECWARN_RECEIVED_ZERO_KEY = 10U,
    HZL_ERR_SECWARN_RFU_1 = 11U,  //!< Reserved warning code for future use.
    HZL_ERR_SECWARN_RFU_2 = 12U,  //!< Reserved warning code for future use.
    HZL_ERR_SECWARN_RFU_3 = 13U,  //!< Reserved warning code for future use.
    HZL_ERR_SECWARN_RFU_4 = 14U,  //!< Reserved warning code for future use.
    HZL_ERR_SECWARN_RFU_5 = 15U,  //!< Reserved warning code for future use.

    // Generic errors
    /** Default error value that should never appear, as it should be replaced with another one.
     * Either the code is buggy or the functionality is not implemented yet. */
    HZL_ERR_PROGRAMMING = 16U,

    // ClientInit, ServerInit
    /** The function requires a pointer to the Hazelnet context, but NULL was provided. */
    HZL_ERR_NULL_CTX = 17U,
    /** The context contains a NULL pointer to the Client configuration struct.
     * @see #hzl_ClientCtx_t.clientConfig */
    HZL_ERR_NULL_CONFIG_CLIENT = 18U,
    /** The context contains a NULL pointer to the Server configuration struct.
     * @see #hzl_ServerCtx_t.serverConfig */
    HZL_ERR_NULL_CONFIG_SERVER = 19U,
    /** The Party configuration contains zero Groups.
     * @see #hzl_ClientConfig_t.amountOfGroups
     * @see #hzl_ServerConfig_t.amountOfGroups */
    HZL_ERR_ZERO_GROUPS = 20U,
    /** The configured Client's Long Term Key (or at least 1 of them for the Server) is
     * uninitialised, just full of zeros.
     * @see #hzl_ClientConfig_t.ltk
     * @see #hzl_ServerClientConfig_t.ltk */
    HZL_ERR_LTK_IS_ALL_ZEROS = 21U,
    /** The Party configuration contains an unknown or unsupported CBS Header Type.
     * @see #hzl_ClientConfig_t.headerType
     * @see #hzl_ServerConfig_t.headerType */
    HZL_ERR_INVALID_HEADER_TYPE = 22U,
    /** The Client configuration contains a Source Identifier (SID) equal to the one used by
     * the Server #HZL_SERVER_SID.
     * @see #hzl_ClientConfig_t.sid
     * @see #hzl_ServerClientConfig_t.sid */
    HZL_ERR_SERVER_SID_ASSIGNED_TO_CLIENT = 23U,
    /** The array of Client configurations must be sorted from smallest to largest SID
     * without repeated SIDs.
     * @see #hzl_ServerCtx_t.clientConfigs */
    HZL_ERR_SIDS_ARE_NOT_PRESORTED_STRICTLY_ASCENDING = 24U,
    /** The array of Client configurations must have all SIDs from 1
     * to #hzl_ServerConfig_t.amountOfClients without gaps, extremes included.
     * @see #hzl_ServerCtx_t.clientConfigs */
    HZL_ERR_GAP_IN_SIDS = 25U,
    /** The Party configuration contains a Source Identifier (SID) that has a value that would
     * not fit into the bits available for it in the CBS Header. Either choose a different (smaller)
     * SID or choose a different Header Type (**note: for the whole bus**) that would have more
     * bits available for the SID field.
     * @see #hzl_ClientConfig_t.sid
     * @see #hzl_ServerClientConfig_t.sid */
    HZL_ERR_SID_TOO_LARGE_FOR_CONFIGURED_HEADER_TYPE = 26U,
    /** The party configurations contains an amount of Groups that implies there is at least one
     * Group with a GID value that would not fit into the bits available for it in the CBS Header.
     * Either configure fewer Groups for this Party or choose a different Header Type
     * (**note: for the whole bus**) that would have more bits available for the GID field.
     * @see #hzl_ClientConfig_t.amountOfGroups
     * @see #hzl_ServerConfig_t.amountOfGroups */
    HZL_ERR_TOO_MANY_GROUPS_FOR_CONFIGURED_HEADER_TYPE = 27U,
    /** The Server configuration contains zero Clients.
     * @see #hzl_ServerConfig_t.amountOfClients */
    HZL_ERR_ZERO_CLIENTS = 28U,
    /** The Server configuration contains more Clients that it can hold in
     * the bitmap of Clients in each Group.
     * @see #HZL_SERVER_MAX_AMOUNT_OF_CLIENTS
     * @see #hzl_ServerConfig_t.amountOfClients */
    HZL_ERR_TOO_MANY_CLIENTS = 29U,
    /** The Server configurations contains an amount of Groups that implies there is at least one
     * Client with a SID value that would not fit into the bits available for it in the CBS Header.
     * Either configure fewer Clients or choose a different Header Type
     * (**note: for the whole bus**) that would have more bits available for the SID field.
     * @see #hzl_ServerConfig_t.amountOfClients */
    HZL_ERR_TOO_MANY_CLIENTS_FOR_CONFIGURED_HEADER_TYPE = 30U,
    /** The context contains a NULL pointer to the array of Client configuration structs.
     * @see #hzl_ServerCtx_t.clientConfigs */
    HZL_ERR_NULL_CONFIG_CLIENTS = 31U,
    /** The context contains a NULL pointer to the Client configuration
     * struct (on a Client) or array of (on a Server).
     * @see #hzl_ClientCtx_t.clientConfig
     * @see #hzl_ServerCtx_t.clientConfigs */
    HZL_ERR_NULL_CONFIG_GROUPS = 32U,
    /** The array of Group configurations must be sorted from smallest to largest GID
     * without repeated GIDs.
     * @see #hzl_ClientCtx_t.groupConfigs
     * @see #hzl_ServerCtx_t.groupConfigs */
    HZL_ERR_GIDS_ARE_NOT_PRESORTED_STRICTLY_ASCENDING = 33U,
    /** The array of Group configurations must contain all GIDs from 0 to
     * #hzl_ServerConfig_t.amountOfGroups-1 without gaps, expremes included.
     * @see #hzl_ClientCtx_t.groupConfigs
     * @see #hzl_ServerCtx_t.groupConfigs */
    HZL_ERR_GAP_IN_GIDS = 34U,
    /** The array of Group configurations must have the GID == 0 (broadcast Group)
     * #HZL_BROADCAST_GID.
     * The sorting rule implies it must be at the first position (array index 0).
     * @see #hzl_ClientCtx_t.groupConfigs
     * @see #hzl_ServerCtx_t.groupConfigs */
    HZL_ERR_MISSING_GID_0 = 35U,
    /** The array of Group configuration has at least one Group where the max Counter Nonce
     * Delay field is out of valid domain as specified by the CBS protocol.
     * @see #hzl_ClientGroupConfig_t.maxCtrnonceDelayMsgs
     * @see #hzl_ServerGroupConfig_t.maxCtrnonceDelayMsgs */
    HZL_ERR_INVALID_MAX_CTRNONCE_DELAY = 36U,
    /** The array of Group configurations has at least one Group with a GID value that would not fit
     * into the bits available for it in the CBS Header. Either configure the Group to use a smaller
     * GID (**note: for the whole bus**) or choose a different Header Type
     * (**note: for the whole bus**) that would have more bits available for the GID field.
     * @see #hzl_ClientGroupConfig_t.gid
     * @see #hzl_ServerGroupConfig_t.gid */
    HZL_ERR_GID_TOO_LARGE_FOR_CONFIGURED_HEADER_TYPE = 37U,
    /** The Server-side array of Group configuration has at least one Group where the Counter Nonce
     * upper limit field is out of valid domain as specified by the CBS protocol.
     * @see #hzl_ServerGroupConfig.ctrNonceUpperLimit */
    HZL_ERR_TOO_LARGE_CTRNONCE_UPPER_LIMIT = 38U,
    /** The Server-side array of Group configuration has at least one Group where the delay
     * between successive Session renewal notification messages field is out of valid domain as
     * specified by the CBS protocol.
     * @see #hzl_ServerGroupConfig.delayBetweenRenNotificationsMillis */
    HZL_ERR_INVALID_DELAY_BETWEEN_REN_NOTIFICATIONS = 39U,
    /** The Server-side array of Group configuration has at least one Group where the bitmap of
     * Clients in the Group contains no Clients at all.
     * @see #hzl_ServerGroupConfig.clientSidsInGroupBitmap */
    HZL_ERR_CLIENTS_BITMAP_ZERO_CLIENTS = 40U,
    /** The Server-side array of Group configuration has at least one Group where the bitmap of
     * Clients in the Group contains a Client not listed in the array of Client configurations.
     * @see #hzl_ServerGroupConfig.clientSidsInGroupBitmap */
    HZL_ERR_CLIENTS_BITMAP_UNKNOWN_SID = 41U,
    /** The Server-side array of Group configuration has the broadcast Group with
     * GID == #HZL_BROADCAST_GID that does not contain exactly all the Clients
     * listed in the array of Client configurations.
     * @see #hzl_ServerGroupConfig.clientSidsInGroupBitmap */
    HZL_ERR_CLIENTS_BITMAP_INVALID_BROADCAST_GROUP = 42U,
    /** The context contains a NULL pointer to the Group states array.
     * @see #hzl_ClientCtx_t.groupConfigs
     * @see #hzl_ServerCtx_t.groupConfigs */
    HZL_ERR_NULL_STATES_GROUPS = 43U,
    /** The function pointer to the timestamping function is NULL.
     * @see #hzl_Io_t.currentTime */
    HZL_ERR_NULL_CURRENT_TIME_FUNC = 44U,
    /** The function pointer to the true-random number generating function is NULL.
     * @see #hzl_Io_t.trng */
    HZL_ERR_NULL_TRNG_FUNC = 45U,

    // TX and RX function functions
    /** The pointer to the Protocol Data Unit (packed CBS message) to transmit or the just-received
     * one is NULL, but its indicated data length is not zero. */
    HZL_ERR_NULL_PDU = 60U,
    /** The pointer to the Service Data Unit (unpacked user data) to transmit or the just-received
     * one is NULL, but its indicated data length is not zero. */
    HZL_ERR_NULL_SDU = 61U,
    /** The user-specified Group (GID) for transmission or the received message's GID field
     * indicate a Group not available in the Party's configuration and thus cannot be
     * cryptographically processed. */
    HZL_ERR_UNKNOWN_GROUP = 62U,
    /** The received message's SID field indicates a Source not available in the Server's
     * configuration and thus cannot be cryptographically processed. */
    HZL_ERR_UNKNOWN_SOURCE = 63U,
    /** The Group the message is addressed to has no valid Session information in the local state.
     * The message cannot be transmitted securely (when the error occurs on TX)
     * or cannot be decrypted and validated (when on RX). */
    HZL_ERR_SESSION_NOT_ESTABLISHED = 64U,

    // TX functions
    /** The user-provided data to be transmitted is too long to fit into the specified message
     * type. */
    HZL_ERR_TOO_LONG_SDU = 70U,
    /** The building of the message could not be performed right now, as the handshake
     * process (exchange of Request and Response messages) is ongoing. The user has to retry
     * after the handshake is completed.
     * @see hzl_ClientBuildSecuredFd() */
    HZL_ERR_HANDSHAKE_ONGOING = 71U,
    /** The building of the Secured Application Data or a Session Renewal Notification could not be
     * performed on Server-side, because no Client would be able to validate/decrypt the message,
     * as no Client so far in the given Group has sent a Request to obtain the Session information.
     * @see hzl_ServerBuildSecuredFd() */
    HZL_ERR_NO_POTENTIAL_RECEIVER = 72U,
    /** The building of the message could not be performed right now, as the Session renewal
     * phase is ongoing. The user has to retry after is it completed.
     * @see hzl_ServerForceSessionRenewal() */
    HZL_ERR_RENEWAL_ONGOING = 73U,

    // RX functions
    /** The received message contains an unknown PTY field. Its data has an unknown structure. */
    HZL_ERR_INVALID_PAYLOAD_TYPE = 80U,
    /** The received message is too short to even contain the CBS header.
     * Its metadata cannot be parsed. */
    HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_HEADER = 81U,
    /** The received Secured Application Data over CAN FD (SADFD) message is too short to be a valid
     * SADFD message. Either it's too short to contain even the metadata parts of the SADFD message
     * or the in-message indicated SDU length (ptlen field) is too large compared to the overall
     * message length (example: 40 B long message overall, internally stating it has 60 B). */
    HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_SADFD = 82U,
    /** The received Request (REQ) message is too short to be a valid REQ message.
     * Its fields cannot be parsed. */
    HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_REQ = 83U,
    /** The received Response (RES) message is too short to be a valid RES message.
     * Its fields cannot be parsed. */
    HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_RES = 84U,
    /** The received Session Renewal Notification (REN) message is too short to be a valid
     * REN message. Its fields cannot be parsed. */
    HZL_ERR_TOO_SHORT_PDU_TO_CONTAIN_REN = 85U,
    /** The received Secured Application Data contains a too large value in the plaintext length
     * field, thus it cannot be processed, as it would overflow the valid memory. */
    HZL_ERR_TOO_LONG_CIPHERTEXT = 86U,
    /** The received message is ignored as it's not addressed to this Party or is redundant. */
    HZL_ERR_MSG_IGNORED = 87U,
    /** The received Request message contained an all-zeros Request Nonce. */
    HZL_ERR_SECWARN_RECEIVED_ZERO_REQNONCE = 88U,

    // Failed IO operation
    /** The timestamping function failed to provide the current time.
     * This could be an issue anywhere in the OS/driver providing the time down to the
     * clock hardware. */
    HZL_ERR_CANNOT_GET_CURRENT_TIME = 100U,
    /** The true-random number generation function failed to provide a random number.
     * This could be an issue anywhere in the OS/driver providing the number down to the
     * TRNG hardware (if any) not having enough entropy. */
    HZL_ERR_CANNOT_GENERATE_RANDOM = 101U,
    /** The true-random number generation failed to provide a non-zero arrays of bytes for too many
     * times in a row. As the required arrays of bytes must be not-all-zeros, in the (very rare)
     * case they are, simply new random values are requested to the TRNG. If this check fails
     * for way too many times in row, the random number generation is seriously corrupted, probably
     * a hardware issue.
     *
     * @see #HZL_MAX_TRNG_TRIES_FOR_NONZERO_VALUE. */
    HZL_ERR_CANNOT_GENERATE_NON_ZERO_RANDOM = 102U,

    // OS interaction functions
    /** The configuration file name string is NULL. */
    HZL_ERR_NULL_FILENAME = 120U,
    /** The configuration file could not be opened. This could be most commonly an incorrect path
     * to the file/incorrect file name or a reading-permission error. */
    HZL_ERR_CANNOT_OPEN_CONFIG_FILE = 121U,
    /** The configuration file was shorter than expected, some parts of the configuration
     * are left out. The length of the file is parametric to the amount of Groups. */
    HZL_ERR_UNEXPECTED_EOF = 122U,
    /** The magic number "HZL\0" (4 ASCII-encoded bytes) was not found at the beginning of the
     * file as expected. The value is used as a double-check that the loaded file is of the
     * correct format (to avoid loading say a JPEG instead of the Hazelnet configuration. */
    HZL_ERR_INVALID_FILE_MAGIC_NUMBER = 123U,
    /** Heap-memory allocation failure: out of memory. */
    HZL_ERR_MALLOC_FAILED = 124U,
} hzl_Err_t;

/** Standard CBS header types. */
typedef enum hzl_HeaderType
{
    HZL_HEADER_0 = 0U,
    HZL_HEADER_1 = 1U,
    HZL_HEADER_2 = 2U,
    HZL_HEADER_3 = 3U,
    HZL_HEADER_4 = 4U,
    HZL_HEADER_5 = 5U,
    HZL_HEADER_6 = 6U,
// Values [7, 32] are RFU.
} hzl_HeaderType_t;

/** Group Identifier data type. */
typedef uint8_t hzl_Gid_t;

/** Source Identifier data type. */
typedef uint8_t hzl_Sid_t;

/** Payload Type data type. */
typedef uint8_t hzl_Pty_t;

/** CAN message identifier data type, able to hold both 11- or 29-bits values. */
typedef uint32_t hzl_CanId_t;

/** Request Nonce data type. */
typedef uint64_t hzl_ReqNonce_t;

/** Response Nonce data type. */
typedef uint64_t hzl_ResNonce_t;

/**
 * Opaque timestamp (timer, rolling counter) with milliseconds accuracy.
 *
 * With 32 bits we can represent time intervals of up to 2^32-1 ms
 * = 4294967295 ms = 49 days, 17 hours, 2 minutes, 47.295 seconds,
 * which is more than enough for a relative timestamp with just some local
 * accuracy (to answer the question "has enough time passed since the last
 * timestamp").
 *
 * It does not matter what is the exact value of the timestamp, where is it
 * reference point. It's used only to measure time differences. A rolling
 * counter incremented every millisecond is enough.
 */
typedef uint32_t hzl_Timestamp_t;

/** Counter Nonce data type. */
typedef uint32_t hzl_CtrNonce_t;

/** Unpacked CBS Header. */
typedef struct hzl_Header
{
    hzl_Gid_t gid;  ///< Group IDentifier: set of parties enabled for reception
    hzl_Sid_t sid;  ///< Source IDentifier: transmitting party
    hzl_Pty_t pty;  ///< Payload TYpe: content of the CBS message
} hzl_Header_t;

// TODO expose CBS header to allow the user to know which CAN ID to assign
// TODO expose configuration/functions for packing the CBS header to allow the user to
// choose where to pack the header (CAN ID, CAN payload or mixed)
/**
 * Packed CBS PDU (Protocol Data Unit message) ready to be transmitted by the library user.
 */
typedef struct hzl_CbsPduMsg
{
    size_t dataLen;  ///< Length in bytes of the CBS-Payload.
    uint8_t data[HZL_MAX_CAN_FD_DATA_LEN];  ///< CBS-Payload.
} hzl_CbsPduMsg_t;

/** Unpacked received SDU (Service Data Unit message) after validation (and optional decryption). */
typedef struct hzl_RxMsg
{
    size_t dataLen;  ///< Length in bytes of the unpacked/decrypted user data.
    hzl_CanId_t canId;  ///< CAN ID the underlying frame used.
    hzl_Gid_t gid;  ///< Group IDentifier the message used (expected receivers).
    hzl_Sid_t sid;  ///< Source IDentifier the message used (claimed sender).
    bool wasSecured;  ///< True if it was encrypted and authenticated during transmission.
    bool isForUser;  ///< True if the message contains useful data for the user, false if internal.
    uint8_t data[HZL_MAX_CAN_FD_DATA_LEN];  ///< User data in plaintext of \p dataLen bytes.
} hzl_RxSduMsg_t;

/**
 * True-random number generator function.
 *
 * @param [out] bytes where to write the \p amount of random bytes.
 * Untouched on error. It's never NULL.
 * @param [in] amount number of bytes to generate. May be zero.
 *
 * @retval #HZL_OK on success
 * @retval #HZL_ERR_CANNOT_GENERATE_RANDOM on error.
 */
typedef hzl_Err_t (* hzl_TrngFunc)(uint8_t* bytes, size_t amount);

/**
 * Current-time timestamp generating function with millisecond accuracy.
 *
 * The generated timestamp does not necessarily represent the current date
 * and time, it's only used to measure how much time passed since it was
 * generated.
 * The timestamp must represent milliseconds from whatever point in time
 * (e.g. 1970-01-01 00:00:00 or also last Tuesday at 11:55).
 *
 * One example implementation is taking the current Unix time (epoch format)
 * in milliseconds. Another is to provide a rolling counter in milliseconds
 * without a specified starting point in time. If said counter is expected
 * to roll around in a reasonable time,
 *
 * @param [out] timestamp current time. Untouched on error. It's never NULL.
 *
 * @retval #HZL_OK on success
 * @retval #HZL_ERR_CANNOT_GET_CURRENT_TIME when requesting the current
 *         timer/clock to the OS or driver fails.
 */
typedef hzl_Err_t (* hzl_TimestampFunc)(hzl_Timestamp_t* timestamp);

/**
 * Functions used by Hazelnet to interact with the rest of the system in order to
 * obtain random numbers, the current time and to transmit messages.
 */
typedef struct
{
    /**
     * True random number generator function.
     *
     * If an OS is available, it can be set to NULL and the OS TRNG will be
     * selected automatically on initialisation, replacing the NULL pointer.
     */
    HZL_SET_BY_USER hzl_TrngFunc trng;

    /**
     * Current-time timestamp generating function.
     *
     * If an OS is available, it can be set to NULL and the OS time function
     * will be selected automatically on initialisation, replacing the NULL
     * pointer.
     */
    HZL_SET_BY_USER hzl_TimestampFunc currentTime;
} hzl_Io_t;

#ifdef __cplusplus
}
#endif

#endif  /* HZL_H_ */
