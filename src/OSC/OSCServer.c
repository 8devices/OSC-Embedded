/*
 * OSCServer.c
 *
 *  Created on: 2013.06.23
 *      Author: Giedrius
 */

#include "OSC/OSCServer.h"
#include "OSC/OSCBundle.h"
#include "OSC/OSCMisc.h"

#include <MemoryManager/MemoryManager.h>

#include <stdlib.h>
#include <string.h>

typedef struct {
	char* address;
	OSCMethod method;
} OSCMessageHandlerEntry;

typedef struct _OSCMessageLinkedListEntry {
	OSCMessage *message;
	OSCTimetag timetag;
	struct _OSCMessageLinkedListEntry *nextEntry;
} OSCMessageLinkedListEntry;

typedef struct _OSCServer {
	OSCMessageHandlerEntry *handlers;
	uint32_t handlerCount;

	OSCMessageLinkedListEntry *storedMessages;
	OSCMessageLinkedListEntry *parsedMessages;

	OSCTimetag_get getTime;
} OSCServer;


OSCResult OSCServer_addParsedMessage(OSCServer *server, OSCMessage *message, uint64_t timetag);
OSCResult OSCServer_parseBundle(OSCServer *server, uint8_t *data, uint32_t size, uint64_t timetag);
OSCResult OSCServer_parseMessage(OSCServer *server, uint8_t *data, uint32_t size, uint64_t timetag);
OSCResult OSCServer_parsePacket(OSCServer *server, uint8_t *data, uint32_t size, uint64_t timetag);

void OSCServer_handleStoredMessages(OSCServer *server);
void OSCServer_handleParsedMessages(OSCServer *server);

OSCServer*	OSCServer_new(OSCTimetag_get func) {
	OSCServer *server = (OSCServer*)MemoryManager_malloc(sizeof(OSCServer));

	if (server == NULL)
		return NULL;

	server->handlers = NULL;
	server->handlerCount = 0;

	server->storedMessages = NULL;
	server->parsedMessages = NULL;

	server->getTime = func;

	return server;
}

void OSCServer_delete(OSCServer *oscServer) {
	if (oscServer->handlers != NULL) {
		uint32_t i;
		for (i=0; i<oscServer->handlerCount; i++) {
			MemoryManager_free(oscServer->handlers[i].address);
		}
		MemoryManager_free(oscServer->handlers);
	}

	OSCMessageLinkedListEntry *entry = oscServer->storedMessages;
	while (entry != NULL) {
		OSCMessage_delete(entry->message);
		OSCMessageLinkedListEntry *nextEntry = entry->nextEntry;
		MemoryManager_free(entry);
		entry = nextEntry;
	}

	entry = oscServer->parsedMessages;
	while (entry != NULL) {
		OSCMessage_delete(entry->message);
		OSCMessageLinkedListEntry *nextEntry = entry->nextEntry;
		MemoryManager_free(entry);
		entry = nextEntry;
	}

	MemoryManager_free(oscServer);
}

/*
 * Message handling
 */

OSCResult OSCServer_addMessageHandler(OSCServer *oscServer, const char* address, OSCMethod method) {
	//TODO: check if address is valid
	uint32_t len = strlen(address);
	char *addrCopy = (char*)MemoryManager_malloc(len+1); // include the null character

	if (addrCopy == NULL)
		return OSC_ALLOC_FAILED;

	OSCMessageHandlerEntry *newHandlers = (OSCMessageHandlerEntry*)MemoryManager_realloc(oscServer->handlers, sizeof(OSCMessageHandlerEntry)*(oscServer->handlerCount+1));

	if (newHandlers == NULL) {
		MemoryManager_free(addrCopy);
		return OSC_ALLOC_FAILED;
	}

	memcpy(addrCopy, address, len+1);

	oscServer->handlers = newHandlers;
	oscServer->handlers[oscServer->handlerCount].address = addrCopy;
	oscServer->handlers[oscServer->handlerCount].method  = method;
	oscServer->handlerCount++;

	return OSC_OK;
}

OSCResult OSCServer_removeMessageHandler(OSCServer *oscServer, const char* address, OSCMethod method) {

	uint32_t i;
	for (i=0; i<oscServer->handlerCount; i++) {
		if (oscServer->handlers[i].method == method && strcmp(oscServer->handlers[i].address, address) == 0)
			break;
	}

	if (i == oscServer->handlerCount)	// handler not found
		return OSC_ERROR;

	MemoryManager_free(oscServer->handlers[i].address);

	if (i+1 < oscServer->handlerCount) { // if it's not the last handler
		memmove(&oscServer->handlers[i], &oscServer->handlers[i+1], sizeof(OSCMessageHandlerEntry)*(oscServer->handlerCount-i-1));
	}

	OSCMessageHandlerEntry *newHandlers = (OSCMessageHandlerEntry*)MemoryManager_realloc(oscServer->handlers, sizeof(OSCMessageHandlerEntry)*(oscServer->handlerCount-1));

	if (newHandlers == NULL) {
		// TODO: make a better handling, as this should NEVER happen
		oscServer->handlerCount--;
		return OSC_ALLOC_FAILED;
	}

	oscServer->handlers = newHandlers;
	oscServer->handlerCount--;

	return OSC_OK;
}


/*
 * Message parsing
 */

OSCResult OSCServer_addParsedMessage(OSCServer *server, OSCMessage *message, uint64_t timetag) {
	OSCMessageLinkedListEntry *entry = (OSCMessageLinkedListEntry*)MemoryManager_malloc(sizeof(OSCMessageLinkedListEntry));

	if (entry == NULL)
		return OSC_ALLOC_FAILED;

	entry->message = message;
	entry->timetag.raw = timetag;
	entry->nextEntry = NULL;

	if (server->parsedMessages == NULL) {
		server->parsedMessages = entry;
	} else {
		OSCMessageLinkedListEntry *lastEntry;
		for (lastEntry=server->parsedMessages; lastEntry->nextEntry != NULL; lastEntry=lastEntry->nextEntry);
		lastEntry->nextEntry = entry;
	}

	return OSC_OK;
}

OSCResult OSCServer_parseBundle(OSCServer *server, uint8_t *data, uint32_t size, uint64_t timetag) {
	if (strcmp((char*)data, "#bundle") != 0) {
		return OSC_FORMAT_ERROR;
	}

	uint8_t *readPtr = data + 8; // skip "#bundle"

	uint64_t bundleTimetag = 0;
	bundleTimetag = (bundleTimetag << 8) | *readPtr++;
	bundleTimetag = (bundleTimetag << 8) | *readPtr++;
	bundleTimetag = (bundleTimetag << 8) | *readPtr++;
	bundleTimetag = (bundleTimetag << 8) | *readPtr++;
	bundleTimetag = (bundleTimetag << 8) | *readPtr++;
	bundleTimetag = (bundleTimetag << 8) | *readPtr++;
	bundleTimetag = (bundleTimetag << 8) | *readPtr++;
	bundleTimetag = (bundleTimetag << 8) | *readPtr++;

	if ((timetag != OSCTimetag_immediately) && (bundleTimetag < timetag))
		return OSC_FORMAT_ERROR;

	while (readPtr < (data+size)) {
		int32_t len = (readPtr[0] << 24) | (readPtr[1] << 16) | (readPtr[2] << 8) | (readPtr[3]);
		readPtr += 4;
		OSCResult res = OSCServer_parsePacket(server, readPtr, len, bundleTimetag);
		if (res != OSC_OK)
			return res;
		readPtr += len;
	}

	if (readPtr != (data+size))
		return OSC_FORMAT_ERROR;

	return OSC_OK;
}

OSCResult OSCServer_parseMessage(OSCServer *server, uint8_t *data, uint32_t size, uint64_t timetag) {
	if (data[0] != '/')
		return OSC_FORMAT_ERROR;

	OSCMessage *msg = OSCMessage_new();

	if (msg == NULL)
		return OSC_ALLOC_FAILED;

	/*
	 * Header
	 */
	uint8_t *readPtr = data;

	if (OSCMessage_setAddress(msg, (char*)readPtr) != OSC_OK) {
		OSCMessage_delete(msg);
		return OSC_ALLOC_FAILED; // Note: only one possible error (yet)
	}
	readPtr += OSCMisc_getPaddedLength(strlen((char*)readPtr)+1);

	/*
	 * Type description
	 */
	if (*readPtr != ',') {
		OSCMessage_delete(msg);
		return OSC_FORMAT_ERROR;
	}

	uint32_t argCount = strlen((char*)readPtr) -1 ; // excluding the ',' character
	uint8_t *typesPtr = readPtr+1;

	readPtr += OSCMisc_getPaddedLength(strlen((char*)readPtr)+1);

	/*
	 * Arguments
	 */
	uint32_t i;
	for (i = 0; i < argCount; i++) {
		switch (typesPtr[i]) {
			case 'i': {
				int32_t tmp = (readPtr[0] << 24) | (readPtr[1] << 16) | (readPtr[2] << 8) | (readPtr[3]);
				if (OSCMessage_addArgument_int32(msg, tmp) != OSC_OK) {
					OSCMessage_delete(msg);
					return OSC_ALLOC_FAILED; // Note: only one possible error (yet)
				}
				readPtr += 4;
				break;
			}
			case 'f': {
				int32_t tmp = (readPtr[0] << 24) | (readPtr[1] << 16) | (readPtr[2] << 8) | (readPtr[3]);
				if (OSCMessage_addArgument_float(msg, *((float*)&tmp)) != OSC_OK) {
					OSCMessage_delete(msg);
					return OSC_ALLOC_FAILED; // Note: only one possible error (yet)
				}
				readPtr += 4;
				break;
			}
			case 's': {
				if (OSCMessage_addArgument_string(msg, (char*)readPtr) != OSC_OK) {
					OSCMessage_delete(msg);
					return OSC_ALLOC_FAILED; // Note: only one possible error (yet)
				}
				readPtr += OSCMisc_getPaddedLength(strlen((char*)readPtr)+1);
				break;
			}
			case 'b': {
				int32_t tmp = (readPtr[0] << 24) | (readPtr[1] << 16) | (readPtr[2] << 8) | (readPtr[3]);
				readPtr += 4;
				if (OSCMessage_addArgument_blob(msg, readPtr, tmp) != OSC_OK) {
					OSCMessage_delete(msg);
					return OSC_ALLOC_FAILED; // Note: only one possible error (yet)
				}
				readPtr += OSCMisc_getPaddedLength(tmp);
				break;
			}
			default: {
				OSCMessage_delete(msg);
				return OSC_FORMAT_ERROR;
			}
		}
	}

	/*
	 * Finalization
	 */
	if (readPtr != (data+size)) {
		OSCMessage_delete(msg);
		return OSC_FORMAT_ERROR;
	}

	if (OSCServer_addParsedMessage(server, msg, timetag) != OSC_OK) {
		OSCMessage_delete(msg);
		return OSC_ALLOC_FAILED; // Note: only possible error (yet)
	}

	return OSC_OK;
}

OSCResult OSCServer_parsePacket(OSCServer *server, uint8_t *data, uint32_t size, uint64_t timetag) {
	if (strcmp((char*)data, "#bundle") == 0) {
		return OSCServer_parseBundle(server, data, size, timetag);
	} else if (data[0] == '/') {
		return OSCServer_parseMessage(server, data, size, timetag);
	}

	return OSC_FORMAT_ERROR;
}

/*
 * Message handling (checking patterns and calling methods)
 */

void OSCServer_handleStoredMessages(OSCServer *server) {
	if (server->storedMessages == NULL || server->handlerCount == 0)
		return;

	uint64_t now = server->getTime();
	if (now == OSCTimetag_immediately)
		now = 0xffffffffffffffff;

	OSCMessageLinkedListEntry *prevEntry = NULL;
	OSCMessageLinkedListEntry *entry;
	for (entry=server->storedMessages; entry != NULL; entry=entry->nextEntry) {
		if (entry->timetag.raw > now) continue;
		//TODO: check for message timeout

		uint8_t executed = 0;

		uint32_t i;
		for (i=0; i<server->handlerCount; i++) {
			if (OSCMisc_matchStringPattern(server->handlers[i].address, OSCMessage_getAddress(entry->message))) {
				server->handlers[i].method(entry->message);
				executed = 1;
			}
		}

		if (executed) {
			if (prevEntry == NULL)
				server->storedMessages = entry->nextEntry;
			else
				prevEntry->nextEntry = entry->nextEntry;

			OSCMessage_delete(entry->message);
			MemoryManager_free(entry);
		} else {
			prevEntry = entry;
		}

	}
}

void OSCServer_handleParsedMessages(OSCServer *server) {
	if (server->parsedMessages == NULL || server->handlerCount == 0)
		return;

	uint64_t now = server->getTime();
	if (now == OSCTimetag_immediately)
		now = 0xffffffffffffffff;

	OSCMessageLinkedListEntry *prevEntry = NULL;
	OSCMessageLinkedListEntry *entry;
	for (entry = server->parsedMessages; entry != NULL ;
			entry = entry->nextEntry) {
		if (entry->timetag.raw > now)
			continue;
		//TODO: check for message timeout

		uint8_t executed = 0;

		uint32_t i;
		for (i = 0; i < server->handlerCount; i++) {
			if (OSCMisc_matchStringPattern(server->handlers[i].address,	OSCMessage_getAddress(entry->message))) {
				server->handlers[i].method(entry->message);
				executed = 1;
			}
		}

		if (executed) {
			if (prevEntry == NULL )
				server->parsedMessages = entry->nextEntry;
			else
				prevEntry->nextEntry = entry->nextEntry;

			OSCMessage_delete(entry->message);
			MemoryManager_free(entry);
		} else {
			prevEntry = entry;
		}

	}
}

/*
 * Server runtime
 */

void OSCServer_cycle(OSCServer *oscServer, OSCPacketStream *stream) {

	OSCServer_handleStoredMessages(oscServer);

	uint32_t size;
	while ((size=stream->getPacketSize()) > 0) {
		uint8_t *data = (uint8_t*)MemoryManager_malloc(size);

		if (data == NULL)
			break;

		/*
		 * Read and parse packet
		 */
		stream->readPacket(data);
		OSCResult res = OSCServer_parsePacket(oscServer, data, size, OSCTimetag_immediately);
		MemoryManager_free(data);

		/*
		 * Handle or store parsed messages OR delete them on packet failure
		 */
		if (res == OSC_OK) { // execute parsed messages and store not executed ones
			OSCServer_handleParsedMessages(oscServer);

			if (oscServer->storedMessages == NULL) {
				oscServer->storedMessages = oscServer->parsedMessages;
			} else {
				OSCMessageLinkedListEntry *lastEntry;
				for (lastEntry = oscServer->storedMessages; lastEntry->nextEntry != NULL; lastEntry = lastEntry->nextEntry);
				lastEntry->nextEntry = oscServer->parsedMessages;
			}
			oscServer->parsedMessages = NULL;
		} else {	// delete parsed messages
			OSCMessageLinkedListEntry *entry = oscServer->parsedMessages;
			while (entry != NULL) {
				MemoryManager_free(entry->message);
				OSCMessageLinkedListEntry * nextEntry = entry->nextEntry;
				MemoryManager_free(entry);
				entry = nextEntry;
			}
			oscServer->parsedMessages = NULL;
		}

	}
}

void OSCServer_loop(OSCServer *oscServer, OSCPacketStream *stream) {
	while (1)
		OSCServer_cycle(oscServer, stream);
}
