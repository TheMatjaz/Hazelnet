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
 * Tests of the hzl_ClientNewMsg() function.
 */

#include "hzlTest.h"

#if HZL_OS_AVAILABLE

static void
hzlClientTest_ClientNewMsgMustBeNotNull(void)
{
    hzl_Err_t err;

    err = hzl_ClientNewMsg(NULL);

    atto_eq(err, HZL_ERR_NULL_PDU);
}

static void
hzlClientTest_ClientNewMsgValid(void)
{
    hzl_Err_t err;
    hzl_CbsPduMsg_t* msg;

    err = hzl_ClientNewMsg(&msg);

    atto_eq(err, HZL_OK);
    atto_neq(msg, NULL);
    atto_neq(msg->data, NULL);
    atto_eq(msg->dataLen, 0);
    // Check that the allocated memory for the message is actually cleared
    // which will also check that enough space was allocated, otherwise it
    // will (probably) segfault
    atto_zeros(msg->data, 64);

    hzl_ClientFreeMsg(&msg);
}

#endif  /* HZL_OS_AVAILABLE */

void hzlClientTest_ClientNewMsg(void)
{
#if HZL_OS_AVAILABLE
    hzlClientTest_ClientNewMsgMustBeNotNull();
    hzlClientTest_ClientNewMsgValid();
    HZL_TEST_PARTIAL_REPORT();
#endif  /* HZL_OS_AVAILABLE */
}
