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
 * Implementation of the hzl_ServerProcessReceived() function.
 */

#include "hzl.h"
#include "hzl_ServerInternal.h"
#include "hzl_CommonMessage.h"
#include "hzl_ServerProcessReceived.h"

HZL_API hzl_Err_t
hzl_ServerProcessReceived(hzl_CbsPduMsg_t* const reactionPdu,
                          hzl_RxSduMsg_t* const receivedUserData,
                          hzl_ServerCtx_t* const ctx,
                          const uint8_t* const receivedPdu,
                          const size_t receivedPduLen,
                          const hzl_CanId_t receivedCanId)
{
    if (reactionPdu == NULL) { return HZL_ERR_NULL_PDU; }
    if (receivedUserData == NULL) { return HZL_ERR_NULL_SDU; }
    HZL_ERR_DECLARE(err);
    err = hzl_ServerCheckCtxPointers(ctx);
    HZL_ERR_CHECK(err);
    // Get the RX timestamp ASAP to reduce the delays
    hzl_Timestamp_t rxTimestamp = 0;
    err = ctx->io.currentTime(&rxTimestamp);
    // Clear any data that may still linger in the output location, if it's reused.
    // By doing so we avoid the situation where the message buffer contains trailing data
    // from a previously-decrypted message that may be security-critical.
    hzl_ZeroOut(receivedUserData, sizeof(hzl_RxSduMsg_t));
    hzl_ZeroOut(reactionPdu, sizeof(hzl_CbsPduMsg_t));
    HZL_ERR_CHECK(err); // Return from any error of currentTime() only after the cleanups
    hzl_Header_t unpackedHdr;
    err = hzl_CommonCheckReceivedGenericMsg(
            &unpackedHdr, receivedPdu, receivedPduLen,
            HZL_SERVER_SID, ctx->serverConfig->headerType);
    HZL_ERR_CHECK(err);
    receivedUserData->canId = receivedCanId;
    switch (unpackedHdr.pty)
    {
        case HZL_PTY_REQ:
            return hzl_ServerProcessReceivedRequest(
                    reactionPdu, ctx,
                    receivedPdu, receivedPduLen, &unpackedHdr, rxTimestamp);

        case HZL_PTY_RES: // Fall-through to Server-only-msg error
        case HZL_PTY_REN:return HZL_ERR_SECWARN_SERVER_ONLY_MESSAGE;

        case HZL_PTY_SADTP:return HZL_ERR_PROGRAMMING; // TODO to be implemented

        case HZL_PTY_SADFD:
            return hzl_ServerProcessReceivedSecuredFd(
                    reactionPdu, receivedUserData,
                    ctx, receivedPdu, receivedPduLen, &unpackedHdr, rxTimestamp);

        case HZL_PTY_UAD:
            return hzl_CommonProcessReceivedUnsecured(
                    receivedUserData, receivedPdu,
                    receivedPduLen, &unpackedHdr, ctx->serverConfig->headerType);

        case HZL_PTY_RFU1:  // Fall-through to default
        case HZL_PTY_RFU2:  // Fall-through to default
        default:return HZL_ERR_INVALID_PAYLOAD_TYPE;
    }
}
