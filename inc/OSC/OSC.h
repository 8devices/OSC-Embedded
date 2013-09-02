/**
 * @file	OSC.h
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
 * OSC.h is the main include file for the OSC (Embedded) library.
 * OSC (Embedded) is Open Sound Control library written in C for embedded systems.
 * It consists of four main parts:
 *
 * 1) OSCMessage - class-like OSCMessage structure and functions for creating,
 * deleting and manipulating the contents of the OSCMessage.
 *
 * 2) OSCBundle - class-like OSCBundle structure and functions for creating,
 * deleting and manipulating the contents of the OSCBundle.
 *
 * 3) OSCPacketStream - interface-like structure which should be implemented by
 * the OSC library user and passed to OSCServer for it to be able to receive
 * OSCPackets or to OSCMessage and OSCBundle functions when sending them.
 *
 * 4) OSCServer - service-like structure and functions which initiate OSCServer,
 * read (and parse) OSCMessages (or OSCBundles) and calls appropriate handler functions
 *
 */

#ifndef OSC_H_
#define OSC_H_

#include "OSCBundle.h"
#include "OSCMessage.h"
#include "OSCPacketStream.h"
#include "OSCServer.h"


#endif /* OSC_H_ */
