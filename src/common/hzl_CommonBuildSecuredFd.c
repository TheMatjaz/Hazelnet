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
 * Initialisation of the AEAD cipher to secure a SADFD message.
 */

#include "hzl.h"
#include "hzl_CommonMessage.h"
#include "hzl_CommonEndian.h"
#include "hzl_CommonPayload.h"

void
hzl_CommonAeadInitSadfd(hzl_Aead_t* const aead,
                        const uint8_t* const stk,
                        const hzl_Header_t* const unpackedSadfdHeader,
                        const hzl_CtrNonce_t ctrnonce,
                        const uint8_t plaintextLen)
{
    // Authenticated en/decryption initialisation with:
    // aeadKey = currentStk
    // aeadNonce = ctrnonce || GID || SID || 0...0 (the zero-padding IS required)
    uint8_t aeadNonce[HZL_AEAD_NONCE_LEN] = {0};
    hzl_EncodeLe24(&aeadNonce[HZL_SADFD_AEADNONCE_CTR_IDX], ctrnonce);
    aeadNonce[HZL_SADFD_AEADNONCE_GID_IDX] = unpackedSadfdHeader->gid;
    aeadNonce[HZL_SADFD_AEADNONCE_SID_IDX] = unpackedSadfdHeader->sid;
    hzl_AeadInit(aead, stk, aeadNonce);

    // Associated data = label || GID || SID || PTY || ptlen
    hzl_AeadAssocDataUpdate(aead, (uint8_t*) HZL_SADFD_LABEL, HZL_SADFD_LABEL_LEN);
    hzl_AeadAssocDataUpdate(aead, &unpackedSadfdHeader->gid, HZL_GID_LEN);
    hzl_AeadAssocDataUpdate(aead, &unpackedSadfdHeader->sid, HZL_SID_LEN);
    hzl_AeadAssocDataUpdate(aead, &unpackedSadfdHeader->pty, HZL_PTY_LEN);
    hzl_AeadAssocDataUpdate(aead, &plaintextLen, HZL_SADFD_PTLEN_LEN);
}
