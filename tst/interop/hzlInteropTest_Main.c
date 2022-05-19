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
 * Main file and function, running all the test cases for the Client
 * and Server libraries interacting.
 *
 * In these tests we will assume instantaneous transmission,
 * thus anytime a Party builds a message, it will be written "directly"
 * on the bus and "immediately" received by all other parties.
 * This is emulated by the fact that we pass the message structures around
 * with pointers rather than with transmission functions.
 */

#include "hzlTest.h"

#define CAN_ID 0x123U

typedef enum hzlTest_Sid
{
    SERVER = 0U,
    ALICE = 1U,
    BOB = 2U,
    CHARLIE = 3U,
} hzlTest_Sid_t;

typedef enum hzlTest_Gid
{
    GID_SABC = 0U,
    GID_SBC = 1U,
    GID_SA = 2U,
    GID_SAB = 3U,
    GID_SC = 4u,
} hzlTest_Gid_t;

typedef struct hzlInteropTest_Bus
{
    hzl_ServerCtx_t* server;
    hzl_ClientCtx_t* alice;
    hzl_ClientCtx_t* bob;
    hzl_ClientCtx_t* charlie;
} hzlInteropTest_Bus_t;

static void
hzlInteropTest_BusInit(hzlInteropTest_Bus_t* const bus)
{
    hzl_Err_t err;
    err = hzl_ServerNew(&bus->server, "serverconfigfiles/Server.hzl");
    atto_eq(err, HZL_OK);
    err = hzl_ClientNew(&bus->alice, "clientconfigfiles/Alice.hzl");
    atto_eq(err, HZL_OK);
    err = hzl_ClientNew(&bus->bob, "clientconfigfiles/Bob.hzl");
    atto_eq(err, HZL_OK);
    err = hzl_ClientNew(&bus->charlie, "clientconfigfiles/Charlie.hzl");
    atto_eq(err, HZL_OK);
}

static void
hzlInteropTest_BusTeardown(hzlInteropTest_Bus_t* const bus)
{
    hzl_ServerFree(&bus->server);
    hzl_ClientFree(&bus->alice);
    hzl_ClientFree(&bus->bob);
    hzl_ClientFree(&bus->charlie);
}

static void
hzlInteropTest_UadExchange(hzlInteropTest_Bus_t* const bus)
{
    hzl_Err_t err;
    hzl_CbsPduMsg_t uad;
    hzl_CbsPduMsg_t nothing;
    hzl_RxSduMsg_t sdu;
    const uint8_t uadData[] = "hello";

    // Alice transmits UAD message for Bob and Server only
    err = hzl_ClientBuildUnsecured(&uad, bus->alice, uadData, sizeof(uadData), GID_SAB);
    atto_eq(err, HZL_OK);
    err = hzl_ServerProcessReceived(&nothing, &sdu, bus->server, uad.data, uad.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_OK);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.gid, GID_SAB);
    atto_eq(sdu.isForUser, true);
    atto_eq(sdu.sid, ALICE);
    atto_eq(sdu.dataLen, sizeof(uadData));
    atto_memeq(sdu.data, uadData, sdu.dataLen);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->bob, uad.data, uad.dataLen, CAN_ID);
    atto_eq(err, HZL_OK);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.gid, GID_SAB);
    atto_eq(sdu.isForUser, true);
    atto_eq(sdu.sid, ALICE);
    atto_eq(sdu.dataLen, sizeof(uadData));
    atto_memeq(sdu.data, uadData, sdu.dataLen);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->charlie, uad.data, uad.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_OK);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.gid, GID_SAB);
    atto_eq(sdu.isForUser, true);
    atto_eq(sdu.sid, ALICE);
    atto_eq(sdu.dataLen, sizeof(uadData));
    atto_memeq(sdu.data, uadData, sdu.dataLen);
}

static void
hzlInteropTest_InitialisationPhase(hzlInteropTest_Bus_t* const bus)
{
    hzl_Err_t err;
    hzl_CbsPduMsg_t req;
    hzl_CbsPduMsg_t res;
    hzl_CbsPduMsg_t sadfd;
    hzl_CbsPduMsg_t nothing;
    hzl_RxSduMsg_t sdu;
    const uint8_t sadData[] = "secret";

    // Alice transmits a request to communicate securely with Bob and the Server
    err = hzl_ClientBuildRequest(&req, bus->alice, GID_SAB);
    atto_eq(err, HZL_OK);
    err = hzl_ServerProcessReceived(&res, &sdu, bus->server, req.data, req.dataLen, CAN_ID);
    atto_eq(err, HZL_OK);
    atto_gt(res.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->bob, req.data, req.dataLen, CAN_ID);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->charlie, req.data, req.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);

    // Server transmits the response, generated as a reaction above
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->alice, res.data, res.dataLen, CAN_ID);
    atto_eq(err, HZL_OK);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->bob, res.data, res.dataLen, CAN_ID);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->charlie, res.data, res.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);

    // Alice can now communicate securely with the Server only
    err = hzl_ClientBuildSecuredFd(&sadfd, bus->alice, sadData, sizeof(sadData), GID_SAB);
    atto_eq(err, HZL_OK);
    err = hzl_ServerProcessReceived(&nothing, &sdu, bus->server, sadfd.data, sadfd.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_OK);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.gid, GID_SAB);
    atto_eq(sdu.isForUser, true);
    atto_eq(sdu.sid, ALICE);
    atto_eq(sdu.dataLen, sizeof(sadData));
    atto_memeq(sdu.data, sadData, sdu.dataLen);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->bob, sadfd.data, sadfd.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_ERR_SESSION_NOT_ESTABLISHED);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->charlie, sadfd.data, sadfd.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);

    // Bob also request the Session information
    err = hzl_ClientBuildRequest(&req, bus->bob, GID_SAB);
    atto_eq(err, HZL_OK);
    err = hzl_ServerProcessReceived(&res, &sdu, bus->server, req.data, req.dataLen, CAN_ID);
    atto_eq(err, HZL_OK);
    atto_gt(res.dataLen, 0);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->alice, req.data, req.dataLen, CAN_ID);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->charlie, req.data, req.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);

    // Server transmits the response, generated as a reaction above
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->alice, res.data, res.dataLen, CAN_ID);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->bob, res.data, res.dataLen, CAN_ID);
    atto_eq(err, HZL_OK);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->charlie, res.data, res.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);

    // Alice, Bob and the Server can now communicate securely
    err = hzl_ClientBuildSecuredFd(&sadfd, bus->alice, sadData, sizeof(sadData), GID_SAB);
    atto_eq(err, HZL_OK);
    err = hzl_ServerProcessReceived(&nothing, &sdu, bus->server, sadfd.data, sadfd.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_OK);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.gid, GID_SAB);
    atto_eq(sdu.isForUser, true);
    atto_eq(sdu.sid, ALICE);
    atto_eq(sdu.dataLen, sizeof(sadData));
    atto_memeq(sdu.data, sadData, sdu.dataLen);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->bob, sadfd.data, sadfd.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_OK);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.gid, GID_SAB);
    atto_eq(sdu.isForUser, true);
    atto_eq(sdu.sid, ALICE);
    atto_eq(sdu.dataLen, sizeof(sadData));
    atto_memeq(sdu.data, sadData, sdu.dataLen);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->charlie, sadfd.data, sadfd.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);

    // Charlie does not have access to their group
    err = hzl_ClientBuildRequest(&req, bus->charlie, GID_SAB);
    atto_eq(err, HZL_ERR_UNKNOWN_GROUP);
    atto_eq(req.dataLen, 0);
}

static void
hzlInteropTest_RenewalPhase(hzlInteropTest_Bus_t* const bus)
{
    hzl_Err_t err;
    hzl_CbsPduMsg_t reqAlice;
    hzl_CbsPduMsg_t reqBob;
    hzl_CbsPduMsg_t res;
    hzl_CbsPduMsg_t ren;
    hzl_CbsPduMsg_t sadfd;
    hzl_CbsPduMsg_t nothing;
    hzl_RxSduMsg_t sdu;
    const uint8_t sadData[] = "secret";

    // Server transmits the Renewal notification for Alice and Bob
    err = hzl_ServerForceSessionRenewal(&ren, bus->server, GID_SAB);
    atto_eq(err, HZL_OK);
    atto_gt(ren.dataLen, 0);
    err = hzl_ClientProcessReceived(&reqAlice, &sdu, bus->alice, ren.data, ren.dataLen, CAN_ID);
    atto_eq(err, HZL_OK);
    atto_gt(reqAlice.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);
    err = hzl_ClientProcessReceived(&reqBob, &sdu, bus->bob, ren.data, ren.dataLen, CAN_ID);
    atto_eq(err, HZL_OK);
    atto_gt(reqBob.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->charlie, ren.data, ren.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);

    // Server receives the Request from Alice. We now assume Bob is slower, just to see
    // that his SADFD are still supported during the renewal phase
    err = hzl_ServerProcessReceived(&res, &sdu, bus->server, reqAlice.data, reqAlice.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_OK);
    atto_gt(res.dataLen, 0);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->alice, res.data, res.dataLen, CAN_ID);
    atto_eq(err, HZL_OK);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->bob, res.data, res.dataLen, CAN_ID);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->charlie, res.data, res.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);

    // Alice can now communicate securely with the Server only. Bob still has the old STK.
    err = hzl_ClientBuildSecuredFd(&sadfd, bus->alice, sadData, sizeof(sadData), GID_SAB);
    atto_eq(err, HZL_OK);
    err = hzl_ServerProcessReceived(&nothing, &sdu, bus->server, sadfd.data, sadfd.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_OK);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.gid, GID_SAB);
    atto_eq(sdu.isForUser, true);
    atto_eq(sdu.sid, ALICE);
    atto_eq(sdu.dataLen, sizeof(sadData));
    atto_memeq(sdu.data, sadData, sdu.dataLen);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->bob, sadfd.data, sadfd.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_ERR_SECWARN_INVALID_TAG);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->charlie, sadfd.data, sadfd.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);

    // Bob can still communicate securely with the Server and Alice. Bob still has the old STK.
    err = hzl_ClientBuildSecuredFd(&sadfd, bus->bob, sadData, sizeof(sadData), GID_SAB);
    atto_eq(err, HZL_OK);
    err = hzl_ServerProcessReceived(&nothing, &sdu, bus->server, sadfd.data, sadfd.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_OK);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.gid, GID_SAB);
    atto_eq(sdu.isForUser, true);
    atto_eq(sdu.sid, BOB);
    atto_eq(sdu.dataLen, sizeof(sadData));
    atto_memeq(sdu.data, sadData, sdu.dataLen);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->alice, sadfd.data, sadfd.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_OK);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.gid, GID_SAB);
    atto_eq(sdu.isForUser, true);
    atto_eq(sdu.sid, BOB);
    atto_eq(sdu.dataLen, sizeof(sadData));
    atto_memeq(sdu.data, sadData, sdu.dataLen);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->charlie, sadfd.data, sadfd.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);

    // Server receives the Request from Bob.
    err = hzl_ServerProcessReceived(&res, &sdu, bus->server, reqBob.data, reqBob.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_OK);
    atto_gt(res.dataLen, 0);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->alice, res.data, res.dataLen, CAN_ID);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->bob, res.data, res.dataLen, CAN_ID);
    atto_eq(err, HZL_OK);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->charlie, res.data, res.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);

    // Alice, Bob and the Server can now communicate securely
    err = hzl_ClientBuildSecuredFd(&sadfd, bus->alice, sadData, sizeof(sadData), GID_SAB);
    atto_eq(err, HZL_OK);
    err = hzl_ServerProcessReceived(&nothing, &sdu, bus->server, sadfd.data, sadfd.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_OK);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.gid, GID_SAB);
    atto_eq(sdu.isForUser, true);
    atto_eq(sdu.sid, ALICE);
    atto_eq(sdu.dataLen, sizeof(sadData));
    atto_memeq(sdu.data, sadData, sdu.dataLen);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->bob, sadfd.data, sadfd.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_OK);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.gid, GID_SAB);
    atto_eq(sdu.isForUser, true);
    atto_eq(sdu.sid, ALICE);
    atto_eq(sdu.dataLen, sizeof(sadData));
    atto_memeq(sdu.data, sadData, sdu.dataLen);
    err = hzl_ClientProcessReceived(&nothing, &sdu, bus->charlie, sadfd.data, sadfd.dataLen,
                                    CAN_ID);
    atto_eq(err, HZL_ERR_MSG_IGNORED);
    atto_eq(nothing.dataLen, 0);
    atto_eq(sdu.dataLen, 0);
    atto_eq(sdu.isForUser, false);
}

/**
 * Main function.
 * @return 0 if all tests passed, non-zero otherwise.
 */
int main(void)
{
    hzlInteropTest_Bus_t bus;
    hzlInteropTest_BusInit(&bus);
    hzlInteropTest_UadExchange(&bus);
    hzlInteropTest_InitialisationPhase(&bus);
    hzlInteropTest_RenewalPhase(&bus);
    hzlInteropTest_BusTeardown(&bus);
    HZL_TEST_PARTIAL_REPORT();
    return atto_at_least_one_fail;
}
