/**
 * @file	OSCServer.h
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
 * OSCServer - service-like structure and functions which initiate OSCServer,
 * read (and parse) OSCMessages (or OSCBundles) and calls appropriate handler functions.
 * OSCServer is created by calling OSCServer_new function. When the server is initiated,
 * message handler nodes can be added by calling OSCServer_addMessageHandler. The given
 * handler will be automatically called when the message a with matching address pattern
 * is received. Handler can be added or removed (using removeMessageHandler function)
 * at any time. OSCServer works in cycles, which should be initiated externally. Calling
 * OSCServer_cycle will perform one server cycle, while OSCServer_loop will continuously
 * perform server cycles. If at some point of program execution OSCServer is not needed
 * anymore, it should be deleted using OSCServer_delete function, which will free all
 * allocated resources.
 *
 */

#ifndef OSCSERVER_H_
#define OSCSERVER_H_

#include "OSCMessage.h"
#include "OSCPacketStream.h"

/**
 * \struct OSCServer is a structure which represents OSCServer instance and has all
 * private (hidden) members. OSCServer should only be referenced as a pointer.
 */
typedef struct _OSCServer OSCServer;

/**
 * \typedef OSCMethod describe the format of the OSCMessage handler function.
 */
typedef void (*OSCMethod)(OSCMessage* oscMessage);


typedef uint64_t (*OSCTimetag_get)(void);

/**
 * Creates a new instance of OSCServer.
 *
 * @param func A function which, when called, should return a current time (OSCTimetag).
 *
 * @return A pointer to a newly created OSCServer or NULL if error occurred.
 */
OSCServer*	OSCServer_new(OSCTimetag_get func);

/**
 * Frees the resources allocated by the OSCServer.
 *
 * @param oscServer A pointer to the OSCServer instance.
 */
void		OSCServer_delete(OSCServer *oscServer);


/**
 * Adds an OSC node with name address and associates a callback message handler with it.
 *
 * @param oscServer A pointer to the OSCServer instance.
 *
 * @param address A name (address) of the node.
 *
 * @param handler A callback handler function for the node.
 *
 * @return OSC_OK if the node and handler were added successfully or error code.
 */
OSCResult	OSCServer_addMessageHandler(OSCServer *oscServer, const char *address, OSCMethod handler);

/**
 * Removes an OSC node with name address and associated callback message handler function.
 *
 * @param oscServer A pointer to the OSCServer instance.
 *
 * @param address A name (address) of the node.
 *
 * @param handler A callback handler function for the node.
 *
 * @return OSC_OK if the node and handler were removed or error code.
 */
OSCResult	OSCServer_removeMessageHandler(OSCServer *oscServer, const char *address, OSCMethod handler);



/**
 * Performs one server cycle (handles old messages, reads, parses and handles the new ones).
 *
 * @param oscServer A pointer to the OSCServer instance.
 *
 * @param stream A pointer to the implemented OSCPacketStream interface.
 *
 */
void		OSCServer_cycle(OSCServer *oscServer, OSCPacketStream *stream);

/**
 * Performs server cycles forever.
 *
 * @param oscServer A pointer to the OSCServer instance.
 *
 * @param stream A pointer to the implemented OSCPacketStream interface.
 *
 */
void		OSCServer_loop(OSCServer *oscServer, OSCPacketStream *stream);

#endif /* OSCSERVER_H_ */
