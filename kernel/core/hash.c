/*
    EventOS V1.0 - Copyright (C) 2018 Samuel Pereira.

       This file is part of the EventOS distribution.

    EventOS is free software; you can redistribute it and/or modify it under
    the terms of the MIT License (MIT).
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
 */

#include "hash.h"

#ifndef GET_16_BITS
#define GET_16_BITS( d ) 	((((portULONG)(((const portUCHAR*)(d))[1])) << 8) \
							+ (portULONG)(((const portUCHAR*)(d))[0]))
#endif

//
// super hash function by Paul Hsieh
//

portHASH_TYPE xHash_calculateHash( const portCHAR* pcBuffer, portUINTEGER uiLength )
{
	portULONG ulHash = uiLength;
	portULONG ulTmp;
	portINTEGER iRem;

    if (uiLength <= 0 || pcBuffer == NULL) return 0;

    iRem = uiLength & 3;
    uiLength >>= 2;

    /* Main loop */
    for (;uiLength > 0; uiLength--)
    {
        ulHash  += GET_16_BITS( pcBuffer );
        ulTmp    = ( GET_16_BITS( pcBuffer + 2 ) << 11 ) ^ ulHash;
        ulHash   = ( ulHash << 16 ) ^ ulTmp;
        pcBuffer  += 2 * sizeof( portINTEGER );
        ulHash  += ulHash >> 11;
    }

    /* Handle end cases */
    switch( iRem )
    {
        case 3:
        	ulHash += GET_16_BITS( pcBuffer );
			ulHash ^= ulHash << 16;
			ulHash ^= pcBuffer[ sizeof( portINTEGER ) ] << 18;
			ulHash += ulHash >> 11;
			break;
        case 2:
        	ulHash += GET_16_BITS( pcBuffer );
			ulHash ^= ulHash << 11;
			ulHash += ulHash >> 17;
			break;
        case 1:
        	ulHash += *pcBuffer;
			ulHash ^= ulHash << 10;
			ulHash += ulHash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    ulHash ^= ulHash << 3;
    ulHash += ulHash >> 5;
    ulHash ^= ulHash << 4;
    ulHash += ulHash >> 17;
    ulHash ^= ulHash << 25;
    ulHash += ulHash >> 6;

    return ulHash;
}
