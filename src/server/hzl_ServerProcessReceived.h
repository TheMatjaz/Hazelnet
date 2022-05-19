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
 * Functions handling the various types of received messages on the Server side.
 */

#ifndef HZL_SERVER_PROCESS_RECEIVED_H_
#define HZL_SERVER_PROCESS_RECEIVED_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "hzl_CommonInternal.h"
#include "hzl_CommonHeader.h"

/** @internal Validates the GID and SID of the received message. */
hzl_Err_t
hzl_ServerValidateSidAndGid(const hzl_ServerCtx_t* ctx,
                            hzl_Gid_t gid,
                            hzl_Sid_t sid);

/**
 * @internal
 * Validates, decrypts and handles a received REQ message, preparing a RES reaction.
 *
 * @param [out] msgToTx generated Response for this Request
 * @param [in, out] ctx to access the Group configuration and alter its state
 * @param [in] rxPdu received raw REQ message
 * @param [in] rxPduLen length of \p rxPdu in bytes
 * @param [in] unpackedHdr metadata of the CBS message in unpacked format
 * @param [in] rxTimestamp timestamp of reception of the Request
 *
 * @return #HZL_OK on success or the proper error code if something is incorrect with the
 *        message or with the local state
 */
hzl_Err_t
hzl_ServerProcessReceivedRequest(hzl_CbsPduMsg_t* msgToTx,
                                 hzl_ServerCtx_t* ctx,
                                 const uint8_t* rxPdu,
                                 size_t rxPduLen,
                                 const hzl_Header_t* unpackedHdr,
                                 hzl_Timestamp_t rxTimestamp);

/**
 * @internal
 * Validates, decrypts and handles a received SADFD message, updating the local Counter Nonce.
 *
 * @param [out] reactionPdu REN message, generated if required. Contains 0 bytes of data otherwise.
 * @param [out] unpackedMsg decrypted, validated and unpacked data contained in the SADFD message
 * @param [in, out] ctx to access the Group configuration and alter its state
 * @param [in] rxPdu received raw SADFD message
 * @param [in] rxPduLen length of \p rxPdu in bytes
 * @param [in] unpackedSadfdHeader metadata of the CBS message in unpacked format
 * @param [in] rxTimestamp timestamp of reception of the SADFD message
 *
 * @return #HZL_OK on success or the proper error code if something is incorrect with the
 *        message or with the local state
 */
hzl_Err_t
hzl_ServerProcessReceivedSecuredFd(hzl_CbsPduMsg_t* reactionPdu,
                                   hzl_RxSduMsg_t* unpackedMsg,
                                   hzl_ServerCtx_t* ctx,
                                   const uint8_t* rxPdu,
                                   size_t rxPduLen,
                                   const hzl_Header_t* unpackedSadfdHeader,
                                   hzl_Timestamp_t rxTimestamp);

/**
 * @internal
 * Validates, decrypts and handles a received SADTP message, updating the local Counter Nonce.
 *
 * @param [out] unpackedMsg decrypted, validated and unpacked data contained in the SADFD message
 * @param [in, out] ctx to access the Group configuration and alter its state
 * @param [in] rxPdu received raw SADTP message
 * @param [in] rxPduLen length of \p rxPdu in bytes
 * @param [in] unpackedSadtpHeader metadata of the CBS message in unpacked format
 * @param [in] rxTimestamp timestamp of reception
 *
 * @return #HZL_OK on success or the proper error code if something is incorrect with the
 *        message or with the local state
 */
hzl_Err_t
hzl_ServerProcessReceivedSecuredTp(hzl_RxSduMsg_t* unpackedMsg,
                                   const hzl_ServerCtx_t* ctx,
                                   const uint8_t* rxPdu,
                                   size_t rxPduLen,
                                   const hzl_Header_t* unpackedSadtpHeader,
                                   hzl_Timestamp_t rxTimestamp);

#ifdef __cplusplus
}
#endif

#endif  /* HZL_SERVER_PROCESS_RECEIVED_H_ */
