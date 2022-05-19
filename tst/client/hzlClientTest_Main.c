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
 * Main file and function, running all the test cases for the Client lib.
 */

#include "hzlTest.h"

/**
 * Main function, running all test cases for the Client lib.
 * @return 0 if all tests passed, non-zero otherwise.
 */
int main(void)
{
    hzlClientTest_ClientInit();
    hzlClientTest_ClientInitCheckClientConfig();
    hzlClientTest_ClientInitCheckGroupConfigs();
    hzlClientTest_ClientInitCheckIo();
    hzlClientTest_ClientDeinit();
    hzlClientTest_ClientNew();
    hzlClientTest_ClientNewMsg();
    hzlClientTest_ClientBuildRequest();
    hzlClientTest_ClientBuildUnsecured();
    hzlClientTest_ClientBuildSecuredFd();
    hzlClientTest_ClientProcessReceived();
    hzlClientTest_ClientProcessReceivedUnsecured();
    hzlClientTest_ClientProcessReceivedSecuredFd();
    hzlClientTest_ClientProcessReceivedRequest();
    hzlClientTest_ClientProcessReceivedResponse();
    hzlClientTest_ClientProcessReceivedRenewal();
    HZL_TEST_PARTIAL_REPORT();
    return atto_at_least_one_fail;
}
