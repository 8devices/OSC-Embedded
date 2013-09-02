/**
 * @file	OSCBundle.h
 * @author  Giedrius Medzevicius <giedrius@8devices.com>
 *
 * @section LICENSE
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 UAB 8devices
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * OSCBundle - class-like OSCBundle structure and functions for creating,
 * deleting and manipulating the contents of the OSCBundle.
 *
 */

#ifndef OSCBUNDLE_H_
#define OSCBUNDLE_H_

#include <stdint.h>
#include "OSCMessage.h"
#include "OSCPacketStream.h"

#define OSCTimetag_immediately	(1)

typedef union _OSCTimetag {
	uint64_t raw;
	struct {	// XXX: switched to correct for endianess
		uint32_t fraction;
		uint32_t seconds;

	} time;
} OSCTimetag;

typedef struct _OSCBundle OSCBundle;

OSCBundle*	OSCBundle_new(void);
OSCBundle*	OSCBundle_clone(OSCBundle *oscBundle);
void		OSCBundle_delete(OSCBundle *oscBundle);

void		OSCBundle_setTimetag(OSCBundle *oscBundle, uint64_t timetag);

OSCResult	OSCBundle_addMessage(OSCBundle *oscBundle, OSCMessage *oscMessage);
OSCResult	OSCBundle_addBundle(OSCBundle *oscBundle, OSCBundle *oscBundleIn);

OSCResult	OSCBundle_sendBundle(OSCBundle *oscBundle, OSCPacketStream *stream);
uint32_t	OSCBundle_getPaddedLength(OSCBundle *oscBundle);
void		OSCBundle_dump(OSCBundle *oscBundle, uint8_t *data);

#endif /* OSCBUNDLE_H_ */
