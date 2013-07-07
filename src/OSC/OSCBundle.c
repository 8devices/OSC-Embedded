/**
 * @file	OSCBundle.c
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

#include "OSC/OSCBundle.h"

#include <MemoryManager/MemoryManager.h>

#include <stdlib.h>
#include <string.h>

typedef struct _OSCElement {
	enum { OSC_BUNDLE, OSC_MESSAGE } type;

	union {
		OSCBundle* bundle;
		OSCMessage* message;
		void*		ptr;
	} contents;
} OSCElement;

typedef struct _OSCBundle {
	OSCTimetag	timetag;
	uint32_t	elementCount;
	OSCElement** elements;
} OSCBundle;


OSCBundle* OSCBundle_new() {
	OSCBundle *bundle = (OSCBundle*) MemoryManager_malloc(sizeof(OSCBundle));

	if (bundle == NULL)
		return NULL;

	bundle->timetag.raw = OSCTimetag_immediately;

	bundle->elementCount = 0;
	bundle->elements = NULL;

	return bundle;
}

OSCBundle*	OSCBundle_clone(OSCBundle *oscBundle) {
	OSCBundle *bundle = (OSCBundle*) MemoryManager_malloc(sizeof(OSCBundle));

	if (bundle == NULL )
		return NULL;

	bundle->timetag.raw = oscBundle->timetag.raw;

	uint32_t i;
	for (i = 0; i < oscBundle->elementCount; i++) {
		OSCResult res = OSC_ERROR;
		switch (oscBundle->elements[i]->type) {
		case OSC_BUNDLE:
			res = OSCBundle_addBundle(bundle, oscBundle->elements[i]->contents.bundle);
			break;
		case OSC_MESSAGE:
			res = OSCBundle_addMessage(bundle, oscBundle->elements[i]->contents.message);
			break;
		}

		if (res != OSC_OK) {
			OSCBundle_delete(bundle);
			return NULL ;
		}
	}

	return bundle;
}

void OSCBundle_delete(OSCBundle *oscBundle) {
	uint32_t i;
	for (i=0; i<oscBundle->elementCount; i++) {
		MemoryManager_free(oscBundle->elements[i]->contents.ptr);
		MemoryManager_free(oscBundle->elements[i]);
	}
	MemoryManager_free(oscBundle->elements);

	MemoryManager_free(oscBundle);
}

void OSCBundle_setTimetag(OSCBundle *oscBundle, uint64_t timetag) {
	oscBundle->timetag.raw = timetag;
}

OSCResult OSCBundle_addElement(OSCBundle *oscBundle, OSCElement *oscElement) {
	OSCElement **elements = (OSCElement**)MemoryManager_realloc(oscBundle->elements, sizeof(OSCElement*)*(oscBundle->elementCount+1));

	if (elements == NULL)
		return OSC_ALLOC_FAILED;

	oscBundle->elements = elements;

	oscBundle->elements[oscBundle->elementCount] = oscElement;
	oscBundle->elementCount++;

	return OSC_OK;
}

OSCResult OSCBundle_addMessage(OSCBundle *oscBundle, OSCMessage *oscMessage) {
	OSCMessage *msg = OSCMessage_clone(oscMessage);

	if (msg == NULL)
		return OSC_ALLOC_FAILED;

	OSCElement *element = (OSCElement*)MemoryManager_malloc(sizeof(OSCElement));

	if (element == NULL) {
		MemoryManager_free(msg);
		return OSC_ALLOC_FAILED;
	}

	element->type = OSC_MESSAGE;
	element->contents.message = msg;

	OSCResult res = OSCBundle_addElement(oscBundle, element);

	if (res != OSC_OK) {
		MemoryManager_free(msg);
		MemoryManager_free(element);
		return res;
	}

	return OSC_OK;
}

OSCResult OSCBundle_addBundle(OSCBundle *oscBundle, OSCBundle *oscBundleIn) {
	OSCBundle *bundle = OSCBundle_clone(oscBundleIn);

	if (bundle == NULL )
		return OSC_ALLOC_FAILED;

	OSCElement *element = (OSCElement*) MemoryManager_malloc(sizeof(OSCElement));

	if (element == NULL ) {
		MemoryManager_free(bundle);
		return OSC_ALLOC_FAILED;
	}

	element->type = OSC_BUNDLE;
	element->contents.bundle = bundle;

	OSCResult res = OSCBundle_addElement(oscBundle, element);

	if (res != OSC_OK) {
		MemoryManager_free(bundle);
		MemoryManager_free(element);
		return res;
	}

	return OSC_OK;
}


/*
 * Functions for sending bundle
 */

OSCResult OSCBundle_sendBundle(OSCBundle *bundle, OSCPacketStream *stream) {

	uint32_t size = OSCBundle_getPaddedLength(bundle);

	uint8_t *data = (uint8_t*)MemoryManager_malloc(size);

	if (data == NULL)
		return OSC_ALLOC_FAILED;

	OSCBundle_dump(bundle, data);

	stream->writePacket(data, size);
	MemoryManager_free(data);

	return OSC_OK;
}

uint32_t OSCBundle_getPaddedLength(OSCBundle *bundle) {
	uint32_t size = 8 + 8; // "#bundle" + timetag

	uint32_t i;
	for (i = 0; i < bundle->elementCount; i++) {
		switch (bundle->elements[i]->type) {
			case OSC_BUNDLE:
				size += 4 + OSCBundle_getPaddedLength(bundle->elements[i]->contents.bundle);
				break;
			case OSC_MESSAGE:
				size += 4 + OSCMessage_getPaddedLength(bundle->elements[i]->contents.message);
				break;
		}
	}

	return size;
}

void OSCBundle_dump(OSCBundle *bundle, uint8_t *data) {
	memset(data, 0, OSCBundle_getPaddedLength(bundle)); // clear the memory (zerofill takes care of padding and null chars)

	uint8_t *ptr = data;
	memcpy(ptr, "#bundle", 7);
	ptr += 8;

	uint64_t timetag = bundle->timetag.raw;
	*ptr++ = (timetag >> 56);
	*ptr++ = (timetag >> 48);
	*ptr++ = (timetag >> 40);
	*ptr++ = (timetag >> 32);
	*ptr++ = (timetag >> 24);
	*ptr++ = (timetag >> 16);
	*ptr++ = (timetag >> 8);
	*ptr++ = (timetag & 0xFF);

	uint32_t i, size;
	for (i = 0; i < bundle->elementCount; i++) {
		switch (bundle->elements[i]->type) {
		case OSC_BUNDLE:
			size = OSCBundle_getPaddedLength(bundle->elements[i]->contents.bundle);
			*ptr++ = (size >> 24);
			*ptr++ = (size >> 16);
			*ptr++ = (size >> 8);
			*ptr++ = (size & 0xFF);

			OSCBundle_dump(bundle->elements[i]->contents.bundle, ptr);
			break;
		case OSC_MESSAGE:
			size = OSCMessage_getPaddedLength(bundle->elements[i]->contents.message);
			*ptr++ = (size >> 24);
			*ptr++ = (size >> 16);
			*ptr++ = (size >> 8);
			*ptr++ = (size & 0xFF);

			OSCMessage_dump(bundle->elements[i]->contents.message, ptr);
			break;
		}
	}
}
