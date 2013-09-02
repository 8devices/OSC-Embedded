/**
 * @file	OSCMessage.h
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
 * OSCMessage - class-like OSCMessage structure and functions for creating,
 * deleting and manipulating the contents of the OSCMessage.
 *
 */

#ifndef OSCMESSAGE_H_
#define OSCMESSAGE_H_

#include <stdint.h>
#include "OSCPacketStream.h"

typedef enum { OSC_OK=0, OSC_ERROR=1, OSC_ALLOC_FAILED, OSC_FORMAT_ERROR } OSCResult;

typedef struct _OSCMessage OSCMessage;

OSCMessage*	OSCMessage_new(void);
OSCMessage* OSCMessage_clone(OSCMessage *oscMessage);
void		OSCMessage_delete(OSCMessage *oscMessage);

OSCResult	OSCMessage_setAddress(OSCMessage *oscMessage, const char* str);
char*		OSCMessage_getAddress(OSCMessage *oscMessage);


OSCResult	OSCMessage_addArgument_int32(OSCMessage *oscMessage, int32_t i);
OSCResult	OSCMessage_addArgument_float(OSCMessage *oscMessage, float f);
OSCResult	OSCMessage_addArgument_string(OSCMessage *oscMessage, const char* s);
OSCResult	OSCMessage_addArgument_blob(OSCMessage *oscMessage, uint8_t *blob, int32_t size);

uint32_t	OSCMessage_getArgumentCount(OSCMessage *oscMessage);
char		OSCMessage_getArgumentType(OSCMessage *oscMessage, uint32_t position);

int32_t		OSCMessage_getArgument_int32 (OSCMessage *oscMessage, uint32_t position);
float		OSCMessage_getArgument_float(OSCMessage *oscMessage, uint32_t position);
char*		OSCMessage_getArgument_string(OSCMessage *oscMessage, uint32_t position);
uint8_t*	OSCMessage_getArgument_blob(OSCMessage *oscMessage, uint32_t position, uint32_t *size);

OSCResult	OSCMessage_sendMessage(OSCMessage *oscMessage, OSCPacketStream *stream);
uint32_t	OSCMessage_getPaddedLength(OSCMessage *oscMessage);
void		OSCMessage_dump(OSCMessage *oscMessage, uint8_t *data);


#endif /* OSCMESSAGE_H_ */
