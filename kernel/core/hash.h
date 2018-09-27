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

#ifndef HASH_H_
#define HASH_H_

#include "stdio.h"
#include "portable.h"

/**
	This method calculates the hash of a given buffer.
 	@param     pcBuffer: data buffer
			   uiLength: length of the data buffer
	@return portHASH_TYPE - hash number that was calculated
	@author samuelpereira7
	@date   26/09/2018
*/
inline portHASH_TYPE xHash_calculateHash( const portCHAR* pcBuffer, portUINTEGER uiLength );

#endif /* HASH_H_ */
