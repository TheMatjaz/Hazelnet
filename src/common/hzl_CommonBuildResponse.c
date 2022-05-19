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
 * Initialisation of the AEAD cipher to secure a RES message.
 */

#include "hzl.h"
#include "hzl_CommonMessage.h"
#include "hzl_CommonEndian.h"
#include "hzl_CommonPayload.h"

void
hzl_CommonAeadInitRes(hzl_Aead_t* const aead,
                      const uint8_t* const ltk,
                      const hzl_Header_t* const unpackedResHeader,
                      const uint8_t* const encodedCtrNonce,
                      const uint8_t* const encodedRequestNonce,
                      const uint8_t* const encodedResponseNonce,
                      const hzl_Sid_t clientSid)
{
    // Authenticated de/cryption initialisation with:
    // aeadKey = LTK
    // aeadNonce = reqnonce || resnonce
    uint8_t aeadNonce[HZL_AEAD_NONCE_LEN] = {0};
    memcpy(&aeadNonce[HZL_RES_AEADNONCE_REQNONCE_IDX], encodedRequestNonce, HZL_REQ_REQNONCE_LEN);
    memcpy(&aeadNonce[HZL_RES_AEADNONCE_RESNONCE_IDX], encodedResponseNonce, HZL_RES_RESNONCE_LEN);
    hzl_AeadInit(aead, ltk, aeadNonce);

    // Associated data = label || GID || SID || PTY || clientSid || receivedCtrnonce
    hzl_AeadAssocDataUpdate(aead, (uint8_t*) HZL_RES_LABEL, HZL_RES_LABEL_LEN);
    hzl_AeadAssocDataUpdate(aead, &unpackedResHeader->gid, HZL_GID_LEN);
    hzl_AeadAssocDataUpdate(aead, &unpackedResHeader->sid, HZL_SID_LEN);
    hzl_AeadAssocDataUpdate(aead, &unpackedResHeader->pty, HZL_PTY_LEN);
    hzl_AeadAssocDataUpdate(aead, &clientSid, HZL_RES_CLIENT_LEN);
    hzl_AeadAssocDataUpdate(aead, encodedCtrNonce, HZL_RES_CTRNONCE_LEN);
}
