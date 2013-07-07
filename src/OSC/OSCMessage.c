/**
 * @file	OSCMessage.c
 * @author  Giedrius Medzevicius <giedrius@8devices.com>
 *
 * @section LICENSE
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 Giedrius Medzevicius
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
 */

#include "OSC/OSCMessage.h"

#include <stdlib.h>
#include <string.h>

#include <MemoryManager/MemoryManager.h>

#include "OSC/OSCMisc.h"


#define OSC_PREALLOC_SIZE	8

typedef struct _OSCArgument {
	uint32_t size;
	union {
		int32_t i;	// integer
		float f;	// float
		char* s;	// string
		uint8_t* b;	// blob
	} data;
} OSCArgument;


typedef struct _OSCMessage {
	char *address;			/* String containing message address */
	char *types;			/* Argument type string descriptor */
	uint32_t addressSize;	/* Size (length) of the allocated *address array */
	uint32_t typesSize;		/* Size (length) of the allocated *types array */
	uint32_t argumentCount;	/* Number of arguments */
	OSCArgument **arguments;	/* Argument pointer array */
} OSCMessage;

/*
 * Private functions
 */

OSCResult OSCMessage_addArgument(OSCMessage *oscMessage, char type, OSCArgument *oscArgument);


/*
 *
 */

OSCMessage* OSCMessage_new() {
	OSCMessage *msg = (OSCMessage*)MemoryManager_malloc(sizeof(OSCMessage));

	if (msg == NULL)
		return NULL;

	msg->address = NULL;
	msg->addressSize = 0;

	msg->types = NULL;
	msg->typesSize = 0;

	msg->arguments = NULL;
	msg->argumentCount = 0;

	OSCMessage_setAddress(msg, "/");

	return msg;
}

OSCMessage* OSCMessage_clone(OSCMessage *oscMessage) {
	OSCMessage *msg = (OSCMessage*) MemoryManager_malloc(sizeof(OSCMessage));

	if (msg == NULL)
		return NULL;

	OSCResult res = OSCMessage_setAddress(msg, oscMessage->address);

	if (res != OSC_OK) {
		OSCMessage_delete(msg);
		return NULL;
	}

	uint32_t i;
	for (i=0; i<oscMessage->argumentCount; i++) {
		OSCArgument* argument = (OSCArgument*) MemoryManager_malloc(sizeof(OSCArgument));

		if (argument == NULL ) {
			OSCMessage_delete(msg);
			return NULL ;
		}

		argument->size = oscMessage->arguments[i]->size;
		argument->data.i = oscMessage->arguments[i]->data.i;

		res = OSCMessage_addArgument(msg, oscMessage->types[i], argument);

		if (res != OSC_OK) {
			OSCMessage_delete(msg);
			return NULL ;
		}
	}

	return msg;
}

void OSCMessage_delete(OSCMessage *oscMessage) {
	MemoryManager_free(oscMessage->address);

	uint32_t i;
	for (i=0; i<oscMessage->argumentCount; i++) {
		if (oscMessage->types[i] == 'b' || oscMessage->types[i] == 's') {
			MemoryManager_free(oscMessage->arguments[i]->data.b);
		}
		MemoryManager_free(oscMessage->arguments[i]);
	}
	MemoryManager_free(oscMessage->arguments);
	MemoryManager_free(oscMessage->types);

	MemoryManager_free(oscMessage);
}

OSCResult OSCMessage_setAddress(OSCMessage *oscMessage, const char* str) {
	uint32_t len = strlen(str); /* Address length */

	if (oscMessage->addressSize < len+1) {
		uint32_t newSize = OSC_PREALLOC_SIZE*(len/OSC_PREALLOC_SIZE + 1); // TODO: make it faster (replace div and mult with bit shifts)
		char* newAddress = (char*)MemoryManager_malloc(newSize);

		if (newAddress == NULL) return OSC_ALLOC_FAILED;

		MemoryManager_free(oscMessage->address);
		oscMessage->address = newAddress;
		oscMessage->addressSize = newSize;
	}

	strcpy(oscMessage->address, str);
	oscMessage->address[len] = '\0';

	return OSC_OK;
}

char* OSCMessage_getAddress(OSCMessage *oscMessage) {
	return oscMessage->address;
}


OSCResult OSCMessage_addArgument(OSCMessage *oscMessage, char type, OSCArgument *oscArgument) {
	if (oscMessage->typesSize < oscMessage->argumentCount + 1) {
		uint32_t newSize = oscMessage->typesSize + OSC_PREALLOC_SIZE;
		char* newTypes = (char*)MemoryManager_realloc(oscMessage->types, newSize);

		if (newTypes == NULL) return OSC_ALLOC_FAILED;

		oscMessage->typesSize = newSize;
		oscMessage->types = newTypes;
	}

	OSCArgument** newArguments = (OSCArgument**)MemoryManager_realloc(oscMessage->arguments, sizeof(OSCArgument*)*(oscMessage->argumentCount+1));

	if (newArguments == NULL) return OSC_ALLOC_FAILED;

	oscMessage->arguments = newArguments;

	/* Done alloc, now set things up */

	oscMessage->types[oscMessage->argumentCount] = type;
	oscMessage->arguments[oscMessage->argumentCount] = oscArgument;
	oscMessage->argumentCount++;

	return OSC_OK;
}


OSCResult OSCMessage_addArgument_int32(OSCMessage *oscMessage, int32_t i) {
	OSCArgument* oscArgument = (OSCArgument*)MemoryManager_malloc(sizeof(OSCArgument));

	if (oscArgument == NULL) return OSC_ALLOC_FAILED;

	oscArgument->size = 4;
	oscArgument->data.i = i;

	OSCResult res = OSCMessage_addArgument(oscMessage, 'i', oscArgument);

	if (res != OSC_OK)
		MemoryManager_free(oscArgument);

	return res;
}

OSCResult OSCMessage_addArgument_float(OSCMessage *oscMessage, float f) {
	OSCArgument* oscArgument = (OSCArgument*)MemoryManager_malloc(sizeof(OSCArgument));

	if (oscArgument == NULL)
		return OSC_ALLOC_FAILED;

	oscArgument->size = 4;
	oscArgument->data.f = f;

	OSCResult res = OSCMessage_addArgument(oscMessage, 'f', oscArgument);

	if (res != OSC_OK)
		MemoryManager_free(oscArgument);

	return res;
}

OSCResult OSCMessage_addArgument_string(OSCMessage *oscMessage, const char* s) {
	/* Allocate OSCArgument */
	OSCArgument* oscArgument = (OSCArgument*)MemoryManager_malloc(sizeof(OSCArgument));

	if (oscArgument == NULL)
		return OSC_ALLOC_FAILED;

	/* Allocate string */
	uint32_t len = strlen(s);
	char* stringCopy = (char*)MemoryManager_malloc(len+1);

	if (stringCopy == NULL) {
		MemoryManager_free(oscArgument);
		return OSC_ALLOC_FAILED;
	}

	/* Do the rest */

	oscArgument->size = len+1;
	oscArgument->data.s = stringCopy;
	strcpy(oscArgument->data.s, s);

	OSCResult res = OSCMessage_addArgument(oscMessage, 's', oscArgument);

	if (res != OSC_OK)
		MemoryManager_free(oscArgument);

	return res;
}

OSCResult OSCMessage_addArgument_blob(OSCMessage *oscMessage, uint8_t *blob, int32_t size) {
	/* Allocate OSCArgument */
	OSCArgument *oscArgument = (OSCArgument*)MemoryManager_malloc(sizeof(OSCArgument));

	if (oscArgument == NULL)
		return OSC_ALLOC_FAILED;

	/* Allocate blob memory */
	uint8_t *newBlob = (uint8_t*)MemoryManager_malloc(size);

	if (newBlob == NULL) {
		MemoryManager_free(oscArgument);
		return OSC_ALLOC_FAILED;
	}

	/* Do the rest */

	oscArgument->size = size;
	oscArgument->data.b = newBlob;
	memcpy(oscArgument->data.b, blob, size);

	OSCResult res = OSCMessage_addArgument(oscMessage, 'b', oscArgument);

	if (res != OSC_OK)
		MemoryManager_free(oscArgument);

	return res;
}

uint32_t OSCMessage_getArgumentCount(OSCMessage *oscMessage) {
	return oscMessage->argumentCount;
}

char OSCMessage_getArgumentType(OSCMessage *oscMessage, uint32_t position) {
	if (position < oscMessage->argumentCount)
		return oscMessage->types[position];

	return '\0'; // no argument
}

int32_t OSCMessage_getArgument_int32 (OSCMessage *oscMessage, uint32_t position) {
	if (position < oscMessage->argumentCount) {
		return oscMessage->arguments[position]->data.i;
	}

	return 0;
}

float OSCMessage_getArgument_float(OSCMessage *oscMessage, uint32_t position) {
	if (position < oscMessage->argumentCount) {
		return oscMessage->arguments[position]->data.f;
	}

	return 0.0f;
}

char* OSCMessage_getArgument_string(OSCMessage *oscMessage, uint32_t position) {
	if (position < oscMessage->argumentCount) {
		return oscMessage->arguments[position]->data.s;
	}

	return NULL;
}

uint8_t* OSCMessage_getArgument_blob(OSCMessage *oscMessage, uint32_t position, uint32_t *size) {
	if (position < oscMessage->argumentCount) {
		*size = oscMessage->arguments[position]->size;
		return oscMessage->arguments[position]->data.b;
	}

	*size = 0;
	return NULL;
}

/*
 * Functions for message sending
 */

OSCResult OSCMessage_sendMessage(OSCMessage *oscMessage, OSCPacketStream *stream) {
	uint32_t size = OSCMessage_getPaddedLength(oscMessage);

	uint8_t *data = (uint8_t*)MemoryManager_malloc(size);

	if (data == NULL)
		return OSC_ALLOC_FAILED;

	OSCMessage_dump(oscMessage, data);

	stream->writePacket(data, size);
	MemoryManager_free(data);

	return OSC_OK;
}

uint32_t OSCMessage_getPaddedLength(OSCMessage *oscMessage) {
	uint32_t size = OSCMisc_getPaddedLength(strlen(oscMessage->address) + 1) 	// address size (including null)
					+ OSCMisc_getPaddedLength(oscMessage->argumentCount + 2);	// type descriptor (including , and null)

	uint32_t i;
	for (i = 0; i < oscMessage->argumentCount; i++) {
		if (oscMessage->types[i] == 'b')
			size += 4;

		size += OSCMisc_getPaddedLength(oscMessage->arguments[i]->size);
	}

	return size;
}

void OSCMessage_dump(OSCMessage *oscMessage, uint8_t *data) {
	memset(data, 0, OSCMessage_getPaddedLength(oscMessage)); // clear the memory (zerofill takes care of padding and null chars)

	uint8_t *ptr = data;
	memcpy(ptr, oscMessage->address, strlen(oscMessage->address));
	ptr += OSCMisc_getPaddedLength(strlen(oscMessage->address) + 1);// address size (including null)

	*ptr = ',';
	memcpy(ptr + 1, oscMessage->types, oscMessage->argumentCount);
	ptr += OSCMisc_getPaddedLength(oscMessage->argumentCount + 2);// type descriptor (including , and null)

	uint32_t i, tmp;
	for (i = 0; i < oscMessage->argumentCount; i++) {
		switch (oscMessage->types[i]) {
			case 'i':
			case 'f': { // let's assume int and float has the same endianess
				tmp = oscMessage->arguments[i]->data.i;
				*ptr++ = (tmp >> 24);
				*ptr++ = (tmp >> 16);
				*ptr++ = (tmp >> 8);
				*ptr++ = (tmp & 0xFF);
				break;
			}
			case 'b': {
				tmp = oscMessage->arguments[i]->size;
				*ptr++ = (tmp >> 24);
				*ptr++ = (tmp >> 16);
				*ptr++ = (tmp >> 8);
				*ptr++ = (tmp & 0xFF);
			}
			case 's': {
				memcpy(ptr, oscMessage->arguments[i]->data.s, oscMessage->arguments[i]->size);
				ptr += OSCMisc_getPaddedLength(oscMessage->arguments[i]->size);
				break;
			}
		}
	}
}
