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
 * Functions handling the various types of received messages on the Client side.
 */

#ifndef HZL_CLIENT_PROCESS_RECEIVED_H_
#define HZL_CLIENT_PROCESS_RECEIVED_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "hzl_CommonInternal.h"
#include "hzl_CommonHeader.h"

/**
 * @internal
 * Validates, decrypts and handles a received RES message, setting the Session information for
 * the Group the RES is for.
 *
 * @param [in, out] ctx to access the Group configuration and alter its state
 * @param [in] rxPdu received raw RES message
 * @param [in] rxPduLen length of \p rxPdu in bytes
 * @param [in] unpackedHdr metadata of the CBS message in unpacked format
 * @param [in] rxTimestamp timestamp of reception
 *
 * @return #HZL_OK on success or the proper error code if something is incorrect with the
 *        message or with the local state
 */
hzl_Err_t
hzl_ClientProcessReceivedResponse(const hzl_ClientCtx_t* ctx,
                                  const uint8_t* rxPdu,
                                  size_t rxPduLen,
                                  const hzl_Header_t* unpackedHdr,
                                  hzl_Timestamp_t rxTimestamp);

/**
 * @internal
 * Validates and handles a received REN message, triggering the automatic generation and
 * transmission of a REQ for the Group the REN is for, unless it was already requrested.
 *
 * @param [out] reactionPdu REQ message auto-generated as a reaction
 * @param [in, out] ctx to access the Group configuration and alter its state for the REQ
 * @param [in] rxPdu received raw REN message
 * @param [in] rxPduLen length of \p rxPdu in bytes
 * @param [in] unpackedRenHeader metadata of the CBS message in unpacked format
 * @param [in] rxTimestamp timestamp of reception
 *
 * @return #HZL_OK on success or the proper error code if something is incorrect with the
 *        message or with the local state
 */
hzl_Err_t
hzl_ClientProcessReceivedRenewal(hzl_CbsPduMsg_t* reactionPdu,
                                 const hzl_ClientCtx_t* ctx,
                                 const uint8_t* rxPdu,
                                 size_t rxPduLen,
                                 const hzl_Header_t* unpackedRenHeader,
                                 hzl_Timestamp_t rxTimestamp);


/**
 * @internal
 * Validates, decrypts and handles a received SADFD message, updating the local Counter Nonce.
 *
 * @param [out] unpackedMsg decrypted, validated and unpacked data contained in the SADFD message
 * @param [in, out] ctx to access the Group configuration and alter its state
 * @param [in] rxPdu received raw SADFD message
 * @param [in] rxPduLen length of \p rxPdu in bytes
 * @param [in] unpackedSadfdHeader metadata of the CBS message in unpacked format
 * @param [in] rxTimestamp timestamp of reception
 *
 * @return #HZL_OK on success or the proper error code if something is incorrect with the
 *        message or with the local state
 */
hzl_Err_t
hzl_ClientProcessReceivedSecuredFd(hzl_RxSduMsg_t* unpackedMsg,
                                   const hzl_ClientCtx_t* ctx,
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
hzl_ClientProcessReceivedSecuredTp(hzl_RxSduMsg_t* unpackedMsg,
                                   const hzl_ClientCtx_t* ctx,
                                   const uint8_t* rxPdu,
                                   size_t rxPduLen,
                                   const hzl_Header_t* unpackedSadtpHeader,
                                   hzl_Timestamp_t rxTimestamp);

#ifdef __cplusplus
}
#endif

#endif  /* HZL_CLIENT_PROCESS_RECEIVED_H_ */
