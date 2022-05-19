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
 * Functions that pack/unpack all standard CBS Headers and check the Header Type.
 *
 * In the comments of the static packer/unpacker functions, the bits of each
 * field are indicated with:
 * `g` = GID bits, `s` = SID bits, `p` = PTY bits, `.` = unused bits
 */

#include "hzl_CommonHeader.h"

/**
 * @internal Computes the largest value of a \p bits long unsigned integer.
 * Works for \p bits <= 15, may also work for larger values on platforms where sizeof(unsigned int)
 * is 4 bytes or more.
 */
#define HZL_MAX_UINTx(bits) ((1U << (bits)) - 1U)

/** @internal `| gggg gggg | ssss ssss | pppp pppp |` */
static void
hzl_Header0Pack(uint8_t* binary, const hzl_Header_t* hdr)
{
    binary[0] = hdr->gid;
    binary[1] = hdr->sid;
    binary[2] = hdr->pty;
}

/** @internal `| gggg gggg | ssss sppp |` */
static void
hzl_Header1Pack(uint8_t* binary, const hzl_Header_t* hdr)
{
    binary[0] = hdr->gid;
    binary[1] = (uint8_t) (
            ((hdr->sid & 0x1FU) << 5U)
            | (hdr->pty & 0x07U)
    );
}

/** @internal `| ssss ssss | gggg gppp |` */
static void
hzl_Header2Pack(uint8_t* binary, const hzl_Header_t* hdr)
{
    binary[0] = hdr->sid;
    binary[1] = (uint8_t) (
            ((hdr->gid & 0x1FU) << 5U)
            | (hdr->pty & 0x07U)
    );
}

/** @internal `| gggs sppp |` */
static void
hzl_Header3Pack(uint8_t* binary, const hzl_Header_t* hdr)
{
    binary[0] = (uint8_t) (
            ((hdr->gid & 0x07U) << 5U)
            | ((hdr->sid & 0x03U) << 3U)
            | (hdr->pty & 0x07U)
    );
}

/** @internal `| sssg gppp |` */
static void
hzl_Header4Pack(uint8_t* binary, const hzl_Header_t* hdr)
{
    binary[0] = (uint8_t) (
            ((hdr->sid & 0x07U) << 5U)
            | ((hdr->gid & 0x03U) << 3U)
            | (hdr->pty & 0x07U)
    );
}

/** @internal `| ssss ssss | .... .ppp |` */
static void
hzl_Header5Pack(uint8_t* binary, const hzl_Header_t* hdr)
{
    binary[0] = hdr->sid;
    binary[1] = (uint8_t) (hdr->pty & 0x07U);
}

/** @internal `| ssss sppp |` */
static void
hzl_Header6Pack(uint8_t* binary, const hzl_Header_t* hdr)
{
    binary[0] = (uint8_t) (
            ((hdr->sid & 0x1FU) << 3U)
            | (hdr->pty & 0x07U)
    );
}

/** @internal `| gggg gggg | ssss ssss | pppp pppp |` */
static void
hzl_Header0Unpack(hzl_Header_t* hdr, const uint8_t* binary)
{
    hdr->gid = binary[0];
    hdr->sid = binary[1];
    hdr->pty = binary[2];
}

/** @internal `| gggg gggg | ssss sppp |` */
static void
hzl_Header1Unpack(hzl_Header_t* hdr, const uint8_t* binary)
{
    hdr->gid = binary[0];
    hdr->sid = binary[1] >> 3U;
    hdr->pty = (uint8_t) (binary[1] & 0x07U);
}

/** @internal `| ssss ssss | gggg gppp |` */
static void
hzl_Header2Unpack(hzl_Header_t* hdr, const uint8_t* binary)
{
    hdr->gid = binary[1] >> 3U;
    hdr->sid = binary[0];
    hdr->pty = (uint8_t) (binary[1] & 0x07U);
}

/** @internal `| gggs sppp |` */
static void
hzl_Header3Unpack(hzl_Header_t* hdr, const uint8_t* binary)
{
    hdr->gid = binary[0] >> 5U;
    hdr->sid = (uint8_t) ((binary[0] >> 3U) & 0x03U);
    hdr->pty = (uint8_t) (binary[0] & 0x07U);
}

/** @internal `| sssg gppp |` */
static void
hzl_Header4Unpack(hzl_Header_t* hdr, const uint8_t* binary)
{
    hdr->gid = (binary[0] >> 3U) & 0x03U;
    hdr->sid = binary[0] >> 5U;
    hdr->pty = (uint8_t) (binary[0] & 0x07U);
}

/** @internal `| ssss ssss | .... .ppp |` */
static void
hzl_Header5Unpack(hzl_Header_t* hdr, const uint8_t* binary)
{
    hdr->gid = HZL_BROADCAST_GID;
    hdr->sid = binary[0];
    hdr->pty = (uint8_t) (binary[1] & 0x07U);
}

/** @internal `| ssss sppp |` */
static void
hzl_Header6Unpack(hzl_Header_t* hdr, const uint8_t* binary)
{
    hdr->gid = HZL_BROADCAST_GID;
    hdr->sid = binary[0] >> 3U;
    hdr->pty = (uint8_t) (binary[0] & 0x07U);
}

hzl_Err_t
hzl_HeaderTypeCheck(const uint8_t type)
{
    if (type > HZL_HEADER_6) { return HZL_ERR_INVALID_HEADER_TYPE; }
    else { return HZL_OK; }
}

uint8_t
hzl_HeaderLen(const uint8_t type)
{
    switch (type)
    {
        case HZL_HEADER_0:return 3U;
        case HZL_HEADER_1: // Fall-though
        case HZL_HEADER_2: // Fall-though
        case HZL_HEADER_5:return 2U;
        case HZL_HEADER_3: // Fall-though
        case HZL_HEADER_4: // Fall-though
        case HZL_HEADER_6:return 1U;
        default:return 0;
    }
}

hzl_Sid_t
hzl_HeaderTypeMaxSid(const uint8_t type)
{
    switch (type)
    {
        case HZL_HEADER_0:return HZL_MAX_UINTx(8U);
        case HZL_HEADER_1:return HZL_MAX_UINTx(5U);
        case HZL_HEADER_2:return HZL_MAX_UINTx(8U);
        case HZL_HEADER_3:return HZL_MAX_UINTx(2U);
        case HZL_HEADER_4:return HZL_MAX_UINTx(3U);
        case HZL_HEADER_5:return HZL_MAX_UINTx(8U);
        case HZL_HEADER_6:return HZL_MAX_UINTx(5U);
        default:return 0U;
    }
}

hzl_Gid_t
hzl_HeaderTypeMaxGid(const uint8_t type)
{
    switch (type)
    {
        case HZL_HEADER_0: // Fall-though
        case HZL_HEADER_1:return HZL_MAX_UINTx(8U);
        case HZL_HEADER_2:return HZL_MAX_UINTx(5U);
        case HZL_HEADER_3:return HZL_MAX_UINTx(3U);
        case HZL_HEADER_4:return HZL_MAX_UINTx(2U);
        case HZL_HEADER_5: // Fall-though
        case HZL_HEADER_6:return HZL_MAX_UINTx(0U);
        default:return 0U;
    }
}

hzl_HeaderPackFunc
hzl_HeaderPackFuncForType(const uint8_t type)
{
    switch (type)
    {
        case HZL_HEADER_0:return hzl_Header0Pack;
        case HZL_HEADER_1:return hzl_Header1Pack;
        case HZL_HEADER_2:return hzl_Header2Pack;
        case HZL_HEADER_3:return hzl_Header3Pack;
        case HZL_HEADER_4:return hzl_Header4Pack;
        case HZL_HEADER_5:return hzl_Header5Pack;
        case HZL_HEADER_6:return hzl_Header6Pack;
        default:return NULL;
    }
}

hzl_HeaderUnpackFunc
hzl_HeaderUnpackFuncForType(const uint8_t type)
{
    switch (type)
    {
        case HZL_HEADER_0:return hzl_Header0Unpack;
        case HZL_HEADER_1:return hzl_Header1Unpack;
        case HZL_HEADER_2:return hzl_Header2Unpack;
        case HZL_HEADER_3:return hzl_Header3Unpack;
        case HZL_HEADER_4:return hzl_Header4Unpack;
        case HZL_HEADER_5:return hzl_Header5Unpack;
        case HZL_HEADER_6:return hzl_Header6Unpack;
        default:return NULL;
    }
}
