/**
 * @file	OSCPacketStream.h
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
 * OSCPacketStream defines the interface which is used by the OSC library and
 * should be implemented by the library user to be able to send and receive
 * packets.
 *
 */


#ifndef OSCPACKETSTREAM_H_
#define OSCPACKETSTREAM_H_

#include <stdlib.h>

typedef struct _OSCPacketStream {
	uint32_t (*getPacketSize)(void);		/**< Function that returns packet size or 0 if no packet is pending */
	void (*readPacket)(uint8_t *buf);		/**< Function that reads the contents of the packet and writes it to the buffer */
	void (*writePacket)(uint8_t *buf, uint32_t size);	/**< Function that forms a packet from the buffer and sends it */
} OSCPacketStream;

#endif /* OSCPACKETSTREAM_H_ */
