
/*
 ***************************************************************************
 *  Copyright 2007,2008 Impinj, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ***************************************************************************
 */


#include "ltkc_platform.h"
#include "ltkc_base.h"
#include "ltkc_frame.h"


/*
 * <0  => Error
 *  0  => Done, length is *pMessageLength
 * >0  => Need more, return value is number of required bytes
 */

LLRP_tSFrameExtract
LLRP_FrameExtract (
  const unsigned char *         pBuffer,
  unsigned int                  nBuffer)
{
    LLRP_tSFrameExtract         frameExtract;

    memset(&frameExtract, 0, sizeof frameExtract);

    if(19 > nBuffer)
    {
        frameExtract.MessageLength = 19u;
        frameExtract.nBytesNeeded = frameExtract.MessageLength - nBuffer;
        frameExtract.eStatus = LLRP_FRAME_NEED_MORE;
    }
    else
    {
        frameExtract.MessageLength = pBuffer[11];
        frameExtract.MessageLength <<= 8u;
        frameExtract.MessageLength |= pBuffer[12];
        frameExtract.MessageLength <<= 8u;
        frameExtract.MessageLength |= pBuffer[13];
        frameExtract.MessageLength <<= 8u;
        frameExtract.MessageLength |= pBuffer[14];

        /*
         * Should we be picky about reserved bits?
         */
/*
        frameExtract.MessageType = VersType & 0x3FFu;
        frameExtract.ProtocolVersion = (VersType >> 10u) & 0x7u;

        frameExtract.MessageID = pBuffer[6];
        frameExtract.MessageID <<= 8u;
        frameExtract.MessageID |= pBuffer[7];
        frameExtract.MessageID <<= 8u;
        frameExtract.MessageID |= pBuffer[8];
        frameExtract.MessageID <<= 8u;
        frameExtract.MessageID |= pBuffer[9];
*/
		if(nBuffer >= frameExtract.MessageLength + 19u)
        {
            frameExtract.nBytesNeeded = 0;
            frameExtract.eStatus = LLRP_FRAME_READY;
        }
        else
        {
            frameExtract.nBytesNeeded = frameExtract.MessageLength + 19 - nBuffer;
            frameExtract.eStatus = LLRP_FRAME_NEED_MORE;
        }
    }

    return frameExtract;
}
