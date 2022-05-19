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
 * Common includes and definitions used across the test suite.
 */

#ifndef HZL_TEST_H_
#define HZL_TEST_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "atto.h"
#include "hzl.h"
#include "hzl_Client.h"
#include "hzl_ClientOs.h"
#include "hzl_Server.h"
#include "hzl_ServerOs.h"

/**
 * @def HZL_TEST_PARTIAL_REPORT
 * When on an OS, print a message showing that a category of tests has passed.
 * Does nothing on embedded systems.
 * Useful to understand if all testcases have been run.
 */
#if HZL_OS_AVAILABLE
#define HZL_TEST_PARTIAL_REPORT() do { \
        if (!atto_at_least_one_fail) { printf("Pass | %s\n", __func__); } \
    } while(0)
#else
#define HZL_TEST_PARTIAL_REPORT()
#endif

/** Default amount of Groups in the sample correct configuration. */
#define HZL_DEFAULT_TEST_AMOUNT_OF_GROUPS 3U
/** Maximum allocated amount of Groups in the sample correct configuration. */
#define HZL_MAX_TEST_AMOUNT_OF_GROUPS 10U
/** Default amount of Clients in the sample correct configuration. */
#define HZL_DEFAULT_TEST_AMOUNT_OF_CLIENTS 2U
/** Maximum allocated amount of Croups in the sample correct configuration. */
#define HZL_MAX_TEST_AMOUNT_OF_CLIENTS 4U
/** Sample correct Client configuration. Copy and tweak into a wrong one if needed. */
extern const hzl_ClientConfig_t HZL_TEST_CORRECT_CLIENT_CONFIG;
/** Sample correct Groups configuration array for a Client.
 * Copy and tweak into a wrong one if needed. */
extern const hzl_ClientGroupConfig_t
        HZL_TEST_CLIENT_CORRECT_GROUP_CONFIGS[HZL_MAX_TEST_AMOUNT_OF_GROUPS];
/** Sample correct Server configuration. Copy and tweak into a wrong one if needed. */
extern const hzl_ServerConfig_t HZL_TEST_CORRECT_SERVER_CONFIG;
/** Sample correct Groups configuration array for the Server.
 * Copy and tweak into a wrong one if needed. */
extern const hzl_ServerGroupConfig_t
        HZL_TEST_SERVER_CORRECT_GROUP_CONFIGS[HZL_MAX_TEST_AMOUNT_OF_GROUPS];
/** Sample correct Clients' configuration array for the Server.
 * Copy and tweak into a wrong one if needed. */
extern const hzl_ServerClientConfig_t
        HZL_TEST_SERVER_CORRECT_CLIENT_CONFIGS[HZL_MAX_TEST_AMOUNT_OF_CLIENTS];
/** Sample correct io configuration. Copy and tweak into a wrong one if needed. */
extern const hzl_Io_t HZL_TEST_CORRECT_IO;

// Mockup IO functions, simulating the same behaviour, both failing and succeeding versions.
hzl_Err_t
hzlTest_IoMockupTrngFailing(uint8_t* bytes, size_t amount);

hzl_Err_t
hzlTest_IoMockupTrngFailingJustZeros(uint8_t* bytes, size_t amount);

hzl_Err_t
hzlTest_IoMockupTrngSucceeding(uint8_t* bytes, size_t amount);

hzl_Err_t
hzlTest_IoMockupCurrentTimeFailing(hzl_Timestamp_t* timestamp);

hzl_Err_t
hzlTest_IoMockupCurrentTimeSucceeding(hzl_Timestamp_t* timestamp);

// Client test running functions, grouping test cases.
void hzlClientTest_ClientInit(void);

void hzlClientTest_ClientInitCheckClientConfig(void);

void hzlClientTest_ClientInitCheckGroupConfigs(void);

void hzlClientTest_ClientInitCheckIo(void);

void hzlClientTest_ClientDeinit(void);

void hzlClientTest_ClientNew(void);

void hzlClientTest_ClientNewMsg(void);

void hzlClientTest_ClientBuildRequest(void);

void hzlClientTest_ClientBuildUnsecured(void);

void hzlClientTest_ClientBuildSecuredFd(void);

void hzlClientTest_ClientProcessReceived(void);

void hzlClientTest_ClientProcessReceivedUnsecured(void);

void hzlClientTest_ClientProcessReceivedSecuredFd(void);

void hzlClientTest_ClientProcessReceivedRequest(void);

void hzlClientTest_ClientProcessReceivedResponse(void);

void hzlClientTest_ClientProcessReceivedRenewal(void);

// Server test running functions, grouping test cases.
void hzlServerTest_ServerInit(void);

void hzlServerTest_ServerInitCheckServerConfig(void);

void hzlServerTest_ServerInitCheckClientConfigs(void);

void hzlServerTest_ServerInitCheckGroupConfigs(void);

void hzlServerTest_ServerInitCheckIo(void);

void hzlServerTest_ServerDeinit(void);

void hzlServerTest_ServerNew(void);

void hzlServerTest_ServerBuildUnsecured(void);

void hzlServerTest_ServerBuildSecuredFd(void);

void hzlServerTest_ServerProcessReceived(void);

void hzlServerTest_ServerProcessReceivedRequest(void);

void hzlServerTest_ServerProcessReceivedServerOnlyMsg(void);

void hzlServerTest_ServerProcessReceivedUnsecured(void);

void hzlServerTest_ServerProcessReceivedSecuredFd(void);

void hzlServerTest_ServerForceSessionRenewal(void);

#ifdef __cplusplus
}
#endif

#endif  /* HZL_TEST_H_ */
