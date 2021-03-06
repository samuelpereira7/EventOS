/*
    EventOS V1.0 - Copyright (C) 2014 Edielson Prevato Frigieri.

       This file is part of the EventOS distribution.

    EventTOS is free software; you can redistribute it and/or modify it under
    the terms of the MIT License (MIT).
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

 */

#ifndef PORTABLE_H_
#define PORTABLE_H_

#ifdef IAR_MSP430
	#include "..\portable\device\TEXAS\MSP430\portmacro.h"	
#endif

#ifdef LPCXPRESSO_LPC17xx
	#include "..\portable\device\NXP\LPC17xx\portmacro.h"	
#endif


/*
 * Setup the hardware ready for the scheduler to take control.  Must setup the system to sleep mode
 * and the control must be reached by a PendSV interruption.
 */
portBASE_TYPE 	xPortStartScheduler( void );

#endif /* PORTABLE_H_ */
