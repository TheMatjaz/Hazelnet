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
 * Implementation of the hzl_CommonNewMsg() and hzl_CommonFreeMsg() functions.
 */

#include "hzl.h"
#include "hzl_CommonInternal.h"

#if HZL_OS_AVAILABLE

hzl_Err_t
hzl_CommonNewMsg(hzl_CbsPduMsg_t** const pMsg)
{
    if (pMsg == NULL) { return HZL_ERR_NULL_PDU; }
    *pMsg = calloc(1U, sizeof(hzl_CbsPduMsg_t));
    if (*pMsg == NULL) { return HZL_ERR_MALLOC_FAILED; }
    return HZL_OK;
}

void
hzl_CommonFreeMsg(hzl_CbsPduMsg_t** const pMsg)
{
    if (pMsg == NULL) { return; }
    hzl_CbsPduMsg_t* msg = *pMsg;  // Dereference once to make the code more readable
    HZL_SECURE_FREE(msg, sizeof(hzl_CbsPduMsg_t));
    *pMsg = NULL;
}

#endif  /* HZL_OS_AVAILABLE */
