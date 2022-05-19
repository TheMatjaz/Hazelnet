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
 * Initialisation of the Hash function cipher to secure a REQ message.
 */

#include "hzl_CommonHash.h"
#include "hzl_CommonMessage.h"
#include "hzl_CommonEndian.h"
#include "hzl_CommonPayload.h"

void
hzl_ReqHashInit(hzl_Hash_t* const hash,
                const uint8_t* const ltk,
                const hzl_Header_t* const unpackedReqHeader,
                const uint8_t* const reqNonce)
{
    // Authentication/validation of the msg with
    // tag = hash(LTK || label || GID || SID || PTY || reqnonce)
    hzl_HashInit(hash);
    hzl_HashUpdate(hash, ltk, HZL_LTK_LEN);
    hzl_HashUpdate(hash, (uint8_t*) HZL_REQ_LABEL, HZL_REQ_LABEL_LEN);
    hzl_HashUpdate(hash, &unpackedReqHeader->gid, HZL_GID_LEN);
    hzl_HashUpdate(hash, &unpackedReqHeader->sid, HZL_SID_LEN);
    hzl_HashUpdate(hash, &unpackedReqHeader->pty, HZL_PTY_LEN);
    hzl_HashUpdate(hash, reqNonce, HZL_REQ_REQNONCE_LEN);
}
