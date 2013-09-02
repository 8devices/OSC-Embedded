/**
 * @file	OSCMisc.c
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
 */


#include "OSC/OSCMisc.h"

//TODO: fix these
#define false	0
#define true	1


uint8_t OSCMisc_matchStringPattern(const char *str, const char *p) {
	int negate;
	int match;
	char c;

	while (*p) {
		if (!*str && *p != '*')
			return false;

		switch (c = *p++) {

		case '*':
			while (*p == '*' && *p != '/')
				p++;

			if (!*p)
				return true;

			//                if (*p != '?' && *p != '[' && *p != '\\')
			if (*p != '?' && *p != '[' && *p != '{')
				while (*str && *p != *str)
					str++;

			while (*str) {
				if (OSCMisc_matchStringPattern(str, p))
					return true;
				str++;
			}
			return false;

		case '?':
			if (*str)
				break;
			return false;
			/*
			 * set specification is inclusive, that is [a-z] is a, z and
			 * everything in between. this means [z-a] may be interpreted
			 * as a set that contains z, a and nothing in between.
			 */
		case '[':
			if (*p != '!')
				negate = false;
			else {
				negate = true;
				p++;
			}

			match = false;

			while (!match && (c = *p++)) {
				if (!*p)
					return false;
				if (*p == '-') { /* c-c */
					if (!*++p)
						return false;
					if (*p != ']') {
						if (*str == c || *str == *p || (*str > c && *str < *p))
							match = true;
					} else { /* c-] */
						if (*str >= c)
							match = true;
						break;
					}
				} else { /* cc or c] */
					if (c == *str)
						match = true;
					if (*p != ']') {
						if (*p == *str)
							match = true;
					} else
						break;
				}
			}

			if (negate == match)
				return false;
			/*
			 * if there is a match, skip past the cset and continue on
			 */
			while (*p && *p != ']')
				p++;
			if (!*p++) /* oops! */
				return false;
			break;

			/*
			 * {astring,bstring,cstring}
			 */
		case '{': {
			// *p is now first character in the {brace list}
			const char *place = str;        // to backtrack
			const char *remainder = p;      // to forwardtrack

			// find the end of the brace list
			while (*remainder && *remainder != '}')
				remainder++;
			if (!*remainder++) /* oops! */
				return false;

			c = *p++;

			while (c) {
				if (c == ',') {
					if (OSCMisc_matchStringPattern(str, remainder)) {
						return true;
					} else {
						// backtrack on test string
						str = place;
						// continue testing,
						// skip comma
						if (!*p++)  // oops
							return false;
					}
				} else if (c == '}') {
					// continue normal pattern matching
					if (!*p && !*str)
						return true;
					str--;  // str is incremented again below
					break;
				} else if (c == *str) {
					str++;
					if (!*str && *remainder)
						return false;
				} else {    // skip to next comma
					str = place;
					while (*p != ',' && *p != '}' && *p)
						p++;
					if (*p == ',')
						p++;
					else if (*p == '}') {
						return false;
					}
				}
				c = *p++;
			}
		}

			break;

			/* Not part of OSC pattern matching
			 case '\\':
			 if (*p)
			 c = *p++;
			 */

		default:
			if (c != *str)
				return false;
			break;

		}
		str++;
	}

	return !*str;
}
