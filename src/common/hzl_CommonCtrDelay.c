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
 * @internal Implementation of the ctrdelay() function from the CBS specification.
 */

#include "hzl_CommonInternal.h"
#include "hzl_CommonMessage.h"

/**
 * @internal
 * Computes the ceiling of the given float in [0, 2^32-2].
 *
 * Equivalent to a simplified `ceilf()` from `math.h`, but implemented here to avoid including
 * `math.h` as a library dependency, as it would be the only used function from it.
 *
 * @param [in] x value in [0, 2^32-2] to round up
 * @return \p x rounded up to the nearest larger integer. If \p x is already an integer, the
 *        value is unchanged.
 */
inline static uint32_t
hzl_CeilUint32(const float x)
{
    const uint32_t xFloored = (uint32_t) x;
    return xFloored + ((float) xFloored < x);
}

hzl_CtrNonce_t
hzl_CommonCtrDelay(const hzl_Timestamp_t lastValidRxMsgInstant,
                   const hzl_Timestamp_t evaluationInstant,
                   const hzl_CtrNonce_t maxCtrNonceDelay,
                   const hzl_TimeDeltaMillis_t maxSilenceInterval)
{
    const hzl_TimeDeltaMillis_t sinceLastMsg = hzl_TimeDelta(
            lastValidRxMsgInstant, evaluationInstant);
    // This condition is equivalent to the max{ceil(...), 0} in the ctrdelay() specification.
    // It also prevents division by 0 when maxSilenceInterval == 0, as sinceLastMsg >= 0 always.
    if (sinceLastMsg >= maxSilenceInterval)
    {
        // Too much time passed since the last message; no tolerance is given for the received
        // counter nonce. It must be equal or newer than the local one.
        return 0;
    }
    // Due to the sinceLastMsg >= maxSilenceInterval condition, this fraction is always in [0, 1].
    const float elapsedAsMaxSilenceFraction = (float) sinceLastMsg / (float) maxSilenceInterval;
    const float delay = (float) maxCtrNonceDelay * (1.0f - elapsedAsMaxSilenceFraction);
    return (hzl_CtrNonce_t) hzl_CeilUint32(delay);
}
