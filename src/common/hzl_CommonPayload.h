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
 * @internal Macros defining fields in the message payloads
 */

#ifndef HZL_PAYLOAD_H_
#define HZL_PAYLOAD_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "hzl_CommonInternal.h"
#include "hzl_CommonAead.h"

// Unsecured Application Data (UAD)
#define HZL_UAD_METADATA_IN_PAYLOAD_LEN 0U

// Request (REQ)
#define HZL_REQ_LABEL "cbs_request"
#define HZL_REQ_LABEL_LEN 11U

#define HZL_REQ_REQNONCE_IDX 0U
#define HZL_REQ_REQNONCE_LEN 8U
#define HZL_REQ_REQNONCE_END (HZL_REQ_REQNONCE_IDX + HZL_REQ_REQNONCE_LEN)

#define HZL_REQ_TAG_IDX HZL_REQ_REQNONCE_END
#define HZL_REQ_TAG_LEN 16U
#define HZL_REQ_TAG_END (HZL_REQ_TAG_IDX + HZL_REQ_TAG_LEN)

#define HZL_REQ_PAYLOAD_LEN (HZL_REQ_REQNONCE_LEN + HZL_REQ_TAG_LEN)

_Static_assert(sizeof(hzl_ReqNonce_t) == HZL_REQ_REQNONCE_LEN,
               "Request nonce must be 8 bytes long");
_Static_assert(HZL_REQ_TAG_END == HZL_REQ_PAYLOAD_LEN,
               "End of the last REQ field must be consistent with the REQ payload length.");
_Static_assert(HZL_REQ_PAYLOAD_LEN == 24,
               "Request Payload must be exactly 24 bytes long");

// Response (RES)
#define HZL_RES_LABEL "cbs_response"
#define HZL_RES_LABEL_LEN 12U

#define HZL_RES_CLIENT_IDX 0U
#define HZL_RES_CLIENT_LEN 1U
#define HZL_RES_CLIENT_END (HZL_RES_CLIENT_IDX + HZL_RES_CLIENT_LEN)

#define HZL_RES_CTRNONCE_IDX HZL_RES_CLIENT_END
#define HZL_RES_CTRNONCE_LEN HZL_CTRNONCE_LEN
#define HZL_RES_CTRNONCE_END (HZL_RES_CTRNONCE_IDX + HZL_RES_CTRNONCE_LEN)

#define HZL_RES_RESNONCE_IDX HZL_RES_CTRNONCE_END
#define HZL_RES_RESNONCE_LEN 8U
#define HZL_RES_RESNONCE_END (HZL_RES_RESNONCE_IDX + HZL_RES_RESNONCE_LEN)

#define HZL_RES_CTEXT_IDX HZL_RES_RESNONCE_END
#define HZL_RES_CTEXT_LEN HZL_AEAD_PTLEN_TO_CTLEN(HZL_STK_LEN)
#define HZL_RES_CTEXT_END (HZL_RES_CTEXT_IDX + HZL_RES_CTEXT_LEN)

#define HZL_RES_TAG_IDX HZL_RES_CTEXT_END
#define HZL_RES_TAG_LEN 16U
#define HZL_RES_TAG_END (HZL_RES_TAG_IDX + HZL_RES_TAG_LEN)

#define HZL_RES_PAYLOAD_LEN ( \
        HZL_RES_CLIENT_LEN \
        + HZL_RES_CTRNONCE_LEN \
        + HZL_RES_RESNONCE_LEN \
        + HZL_RES_CTEXT_LEN \
        + HZL_REQ_TAG_LEN)

_Static_assert(sizeof(hzl_ResNonce_t) == HZL_RES_RESNONCE_LEN,
               "Response nonce must be 8 bytes long");
_Static_assert(HZL_RES_TAG_END == HZL_RES_PAYLOAD_LEN,
               "End of the last RES field must be consistent with the RES payload length.");
_Static_assert(HZL_RES_PAYLOAD_LEN == 44,
               "Response Payload must be exactly 44 bytes long");

#define HZL_RES_AEADNONCE_REQNONCE_IDX 0U
#define HZL_RES_AEADNONCE_REQNONCE_END ( \
        HZL_RES_AEADNONCE_REQNONCE_IDX + \
        HZL_REQ_REQNONCE_LEN)
#define HZL_RES_AEADNONCE_RESNONCE_IDX HZL_RES_AEADNONCE_REQNONCE_END
#define HZL_RES_AEADNONCE_RESNONCE_END ( \
        HZL_RES_AEADNONCE_RESNONCE_IDX + \
        HZL_RES_RESNONCE_LEN)

_Static_assert(HZL_RES_AEADNONCE_RESNONCE_END <= HZL_AEAD_NONCE_LEN,
               "RES msg AEAD nonce is large enough to fit reqnonce||resnonce");

// Session Renewal Notification (REN)
#define HZL_REN_LABEL "cbs_renewal"
#define HZL_REN_LABEL_LEN 11U

#define HZL_REN_CTRNONCE_IDX 0U
#define HZL_REN_CTRNONCE_LEN HZL_CTRNONCE_LEN
#define HZL_REN_CTRNONCE_END (HZL_REN_CTRNONCE_IDX + HZL_REN_CTRNONCE_LEN)

#define HZL_REN_TAG_IDX HZL_REN_CTRNONCE_END
#define HZL_REN_TAG_LEN 16U
#define HZL_REN_TAG_END (HZL_REN_TAG_IDX + HZL_REN_TAG_LEN)

#define HZL_REN_PAYLOAD_LEN (HZL_REN_CTRNONCE_LEN + HZL_REN_TAG_LEN)

_Static_assert(HZL_REN_TAG_END == HZL_REN_PAYLOAD_LEN,
               "End of the last REN field must be consistent with the REN payload length.");
_Static_assert(HZL_REN_PAYLOAD_LEN == 19,
               "Renewal Payload must be exactly 19 bytes long");

// Secured Application Data over CAN-FD (SADFD)
#define HZL_SADFD_LABEL "cbs_secured_fd"
#define HZL_SADFD_LABEL_LEN 14U

#define HZL_SADFD_CTRNONCE_IDX 0U
#define HZL_SADFD_CTRNONCE_LEN HZL_CTRNONCE_LEN
#define HZL_SADFD_CTRNONCE_END (HZL_SADFD_CTRNONCE_IDX + HZL_SADFD_CTRNONCE_LEN)

#define HZL_SADFD_PTLEN_IDX HZL_SADFD_CTRNONCE_END
#define HZL_SADFD_PTLEN_LEN 1U
#define HZL_SADFD_PTLEN_END (HZL_SADFD_PTLEN_IDX + HZL_SADFD_PTLEN_LEN)

#define HZL_SADFD_CTEXT_IDX HZL_SADFD_PTLEN_END
#define HZL_SADFD_CTEXT_END(ctlen) (HZL_SADFD_PTLEN_END + (ctlen))

#define HZL_SADFD_TAG_IDX(ctlen) HZL_SADFD_CTEXT_END(ctlen)
#define HZL_SADFD_TAG_LEN 8U
#define HZL_SADFD_TAG_END(ctlen) (HZL_SADFD_TAG_IDX(ctlen) + HZL_SADFD_TAG_LEN)

#define HZL_SADFD_METADATA_IN_PAYLOAD_LEN (\
        HZL_SADFD_CTRNONCE_LEN \
        + HZL_SADFD_PTLEN_LEN \
        + HZL_SADFD_TAG_LEN)
#define HZL_SADFD_PAYLOAD_LEN(ctlen) (HZL_SADFD_METADATA_IN_PAYLOAD_LEN + (ctlen))

_Static_assert(HZL_SADFD_TAG_END(0) == HZL_SADFD_METADATA_IN_PAYLOAD_LEN,
               "The length of an SADFD message without data must be consistent with the "
               "metadata length");
_Static_assert(HZL_SADFD_PAYLOAD_LEN(0) == 12,
               "SADFD Payload with an empty SDU must be exactly 12 bytes long");

#define HZL_SADFD_AEADNONCE_CTR_IDX 0U
#define HZL_SADFD_AEADNONCE_CTR_END ( \
        HZL_SADFD_AEADNONCE_CTR_IDX + \
        HZL_CTRNONCE_LEN)
#define HZL_SADFD_AEADNONCE_GID_IDX HZL_SADFD_AEADNONCE_CTR_END
#define HZL_SADFD_AEADNONCE_GID_END (HZL_SADFD_AEADNONCE_GID_IDX + HZL_GID_LEN)
#define HZL_SADFD_AEADNONCE_SID_IDX HZL_SADFD_AEADNONCE_GID_END
#define HZL_SADFD_AEADNONCE_SID_END (HZL_SADFD_AEADNONCE_SID_IDX + HZL_SID_LEN)

_Static_assert(HZL_SADFD_AEADNONCE_SID_END <= HZL_AEAD_NONCE_LEN,
               "SAD msg AEAD nonce is large enough to fit ctrnonce||GID||SID");

#ifdef __cplusplus
}
#endif

#endif  /* HZL_PAYLOAD_H_ */
