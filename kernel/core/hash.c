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

portHASH_TYPE xHash_calculateHash( const portCHAR* data, portUINTEGER len )
{
	portULONG hash = len, tmp;
	portINTEGER rem;

    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--)
    {
        hash  += GET_16_BITS( data );
        tmp    = ( GET_16_BITS( data + 2 ) << 11 ) ^ hash;
        hash   = ( hash << 16 ) ^ tmp;
        data  += 2 * sizeof( portINTEGER );
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch( rem )
    {
        case 3:
        	hash += GET_16_BITS( data );
			hash ^= hash << 16;
			hash ^= data[ sizeof( portINTEGER ) ] << 18;
			hash += hash >> 11;
			break;
        case 2:
        	hash += GET_16_BITS( data );
			hash ^= hash << 11;
			hash += hash >> 17;
			break;
        case 1:
        	hash += *data;
			hash ^= hash << 10;
			hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}
