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

#ifndef HZL_COMMONMESSAGE_H
#define HZL_COMMONMESSAGE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "hzl.h"
#include "hzl_CommonAead.h"
#include "hzl_CommonHeader.h"
#include "hzl_CommonHash.h"

/**
 * @internal
 * Builds an **unsecured** message in plaintext for both the Server and Client.
 * Implements the main behaviour and checks of hzl_ClientBuildUnsecured()
 * and hzl_ServerBuildUnsecured(). The caller has only to check the
 * context and pass the proper fields, extracted from the context.
 */
HZL_API hzl_Err_t
hzl_CommonBuildUnsecured(hzl_CbsPduMsg_t* unsecuredPdu,
                         const uint8_t* userData,
                         size_t userDataLen,
                         hzl_Gid_t groupId,
                         hzl_Sid_t sourceId,
                         uint8_t headerType);

/**
 * @internal
 * Validates a message to-be-transmitted provided by the user through the
 * public API.
 *
 * Checks for NULL pointers, invalid message sizes etc.
 *
 * @param [in] userData SDU as provided to the public API.
 * @param [in] userDataLen length of \p userData in bytes as provided to the public API.
 * @param [in] group GID of the destination group.
 * @param [in] metadataInPayloadLen length in bytes of the metadata surrounding the user data
 * within the CBS Payload, excluding the packed CBS Header.
 * @param [in] headerType to access the maximum GID and SID etc.
 *
 * @retval #HZL_OK on a valid message, the specific error code if something is incorrect
 */
hzl_Err_t
hzl_CommonCheckMsgBeforePacking(const uint8_t* userData,
                                size_t userDataLen,
                                hzl_Gid_t group,
                                size_t metadataInPayloadLen,
                                uint8_t headerType);

/** @internal
 * Verifies the basic integrity of the message data structure (NULL pointers,
 * minimum and maximum sizes). */
hzl_Err_t
hzl_CommonCheckReceivedGenericMsg(hzl_Header_t* unpackedHdr,
                                  const uint8_t* receivedPdu,
                                  size_t receivedPduLen,
                                  hzl_Sid_t receiverSid,
                                  uint8_t headerType);

/**
 * @internal
 * Extracts the data from the received unsecured message. There is no validation, as the data
 * is passed to the user as-is.
 *
 * @param [out] unpackedMsg unpacked data contained in the UAD message
 * @param [in] rxPdu received raw UAD message
 * @param [in] rxPduLen length of \p rxPdu in bytes
 * @param [in] unpackedUadHeader metadata of the CBS message in unpacked format
 * @param [in] headerType
 *
 * @return #HZL_OK always as there is no validation
 */
hzl_Err_t
hzl_CommonProcessReceivedUnsecured(hzl_RxSduMsg_t* unpackedMsg,
                                   const uint8_t* rxPdu,
                                   size_t rxPduLen,
                                   const hzl_Header_t* unpackedUadHeader,
                                   uint8_t headerType);

/**
 * @internal
 * Initialised AEAD cipher with the proper AEAD-nonce, label, key etc. as used to
 * secure a SADFD message.
 */
void
hzl_CommonAeadInitSadfd(hzl_Aead_t* aead,
                        const uint8_t* stk,
                        const hzl_Header_t* unpackedSadfdHeader,
                        hzl_CtrNonce_t ctrnonce,
                        uint8_t plaintextLen);

/**
 * @internal
 * Initialised AEAD cipher with the proper AEAD-nonce, label, key etc. as used to
 * secure a RES message.
 */
void
hzl_CommonAeadInitRes(hzl_Aead_t* aead,
                      const uint8_t* ltk,
                      const hzl_Header_t* unpackedResHeader,
                      const uint8_t* encodedCtrNonce,
                      const uint8_t* encodedRequestNonce,
                      const uint8_t* encodedResponseNonce,
                      hzl_Sid_t clientSid);

/**
 * @internal
 * Initialised Hash function with the proper reqnonce, label, key etc. as used to
 * secure a REQ message.
 */
void
hzl_ReqHashInit(hzl_Hash_t* hash,
                const uint8_t* ltk,
                const hzl_Header_t* unpackedReqHeader,
                const uint8_t* reqNonce);

/**
 * @internal
 * Computes the Counter Nonce Delay, i.e. the tolerance applied to a Counter Nonce
 * received at a given evaluation instant, considering when the last valid message was received
 * before.
 *
 * The function is defined according to the "Current Counter Nonce Delay" section of the
 * CBS specification.
 *
 * @param [in] lastValidRxMsgInstant timestamp of the last valid received message (`m`)
 * @param [in] evaluationInstant timestamp of the reception of the current Counter Nonce (`t`)
 * @param [in] maxCtrNonceDelay configuration parameter (`D`)
 * @param [in] maxSilenceInterval configuration parameter (`S`)
 * @return the allowed delay, expressed in the same unit as the Counter Nonce (i.e. unitless)
 */
hzl_CtrNonce_t
hzl_CommonCtrDelay(hzl_Timestamp_t lastValidRxMsgInstant,
                   hzl_Timestamp_t evaluationInstant,
                   hzl_CtrNonce_t maxCtrNonceDelay,
                   hzl_TimeDeltaMillis_t maxSilenceInterval);

#ifdef __cplusplus
}
#endif

#endif  /* HZL_COMMONMESSAGE_H */
