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
 * Hazelnet Client public API, addon for Operating Systems.
 *
 * Hazelnet implements the CAN Bus Security (CBS) protocol, which secures the CAN FD
 * traffic providing encryption, authenticity and freshness of the messages.
 *
 * This header adds to the hzl_Client.h header, providing a simpler alternative for desktop
 * usage on an OS like Windows or Linux compared to an embedded usage. This API allows the user
 * to heap-allocate the Client context, load its configuration from file and assign the
 * timestamping and TRNG functions automatically by using the ones provided by the OS.
 * Similarly, heap-allocation of messages is also available.
 */

#ifndef HZL_CLIENT_OS_H_
#define HZL_CLIENT_OS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "hzl.h"
#include "hzl_Client.h"

#if HZL_OS_AVAILABLE

/**
 * Allocates a new context structure on the heap and fills it with the new configuration from the
 * file and OS functions for time and randomness.
 *
 * This is a handy replacement of hzl_ClientInit() to use when on an OS to simplify the
 * library usage. The function would automatically allocate the context and fill its configuration
 * fields from the file. The IO functions for TRNG and timestamping are also obtained from the
 * OS. The user must only supply the low level transmission function and the file name where to
 * take the configuration from.
 *
 * It's up to the user to free the context allocated by this function using hzl_ClientFree().
 *
 * ### File format
 * The file must have the following format with all multi-byte integers encoded as
 * little Endian and without any paddings between any value or between any struct:
 *
 * 1. "HZL\0" as a magic number in ASCII encoding, used to double-check that the loaded file
 *    is the correct one. That is: [0x48, 0x5A, 0x4C, 0x00] in binary;
 * 2. the whole #hzl_ClientConfig_t struct without any padding;
 * 3. an array of #hzl_ClientGroupConfig_t structs without any padding and with as many
 *    elements (structs) as specified in #hzl_ClientConfig_t.amountOfGroups.
 *
 * It's common to use the `.hzl` file extension to denote this file format.
 * To generate such binary file from a JSON file, the helper Python scripts in
 * `toolsupport/config` can be used.
 *
 * @param [out] pCtx where to load the new context. Must not be NULL.
 * @param [in] fileName path to the file to load the configuration from. Must not be NULL.
 *
 * @retval #HZL_OK on success.
 * @retval #HZL_ERR_NULL_CTX if \p pCtx is NULL.
 * @retval #HZL_ERR_NULL_FILENAME if \p fileName is NULL.
 * @retval #HZL_ERR_CANNOT_OPEN_CONFIG_FILE if the file cannot be opened (does not exists,
 *         or the permissions are not correct)
 * @retval #HZL_ERR_MALLOC_FAILED if the heap-allocation fails (out of memory).
 * @retval #HZL_ERR_UNEXPECTED_EOF if the file is too short: more data was expected
 *         during parsing. Probably is has incorrect syntax or amount of groups.
 * @retval Same values as hzl_ClientInit() in case the context has incorrect data or pointers.
 */
HZL_API hzl_Err_t
hzl_ClientNew(hzl_ClientCtx_t** pCtx,
              const char* fileName);

/**
 * Zeros-out the context, frees it and sets the pointer to it to NULL, to avoid use-after-free
 * and double-free.
 *
 * If some fields are partially-initialised, it clears and frees anything not-NULL.
 *
 * @warning
 * Only use on heap-allocated contexts, as created by hzl_ClientNew().
 *
 * @param [in] pCtx address of the pointer to the context. The address of it is used to
 *        set the pointer to NULL after the data has been freed. If NULL or if \p *pCtx is NULL,
 *        the function does nothing.
 */
HZL_API void
hzl_ClientFree(hzl_ClientCtx_t** pCtx);

/**
 * Allocates a new CAN FD message structure on the heap with enough space to hold up to 64 B of
 * data.
 *
 * It's up to the user to free the message allocated by this function using
 * hzl_ClientFreeMsg().
 *
 * @param [out] pMsg where to load the new message. Must not be NULL.
 * @retval #HZL_OK on success.
 * @retval #HZL_ERR_NULL_PDU if pMsg is NULL.
 * @retval #HZL_ERR_MALLOC_FAILED if the heap-allocation fails (out of memory).
 */
HZL_API hzl_Err_t
hzl_ClientNewMsg(hzl_CbsPduMsg_t** pMsg);

/**
 * Zeros-out the message, frees it and sets the pointer to it to NULL, to avoid use-after-free
 * and double-free.
 *
 * If some fields are partially-initialised, it clears and frees anything not-NULL.
 *
 * @warning
 * Only use on heap-allocated messages, as created by hzl_ClientNewMsg().
 *
 * @param [in] pMsg address of the pointer to the message. The address of it is used to
 *        set the pointer to NULL after the data has been freed. If NULL or if \p *pMsg is NULL,
 *        the function does nothing.
 */
HZL_API void
hzl_ClientFreeMsg(hzl_CbsPduMsg_t** pMsg);

#endif  /* HZL_OS_AVAILABLE */

#ifdef __cplusplus
}
#endif

#endif  /* HZL_CLIENT_OS_H_ */
