/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <asys/base.h>
#include <asys/string.h>

asys_native_long_t glibc_strtol(const char* string, char** end, int base) {
	asys_native_ulong_t cutoff, i;

	const char* s;
	const char* save;

	char c;

	unsigned limit;
	int overflow, ishex, sign;

	if(base < 0 || base == 1 || base > 36) base = 10;

	s = string;

	/* Skip white space.  */
	while(asys_character_is_blank(*s)) ++s;

	if(*s == '\0') goto noconv;

	/* Check for a sign.  */
	if(*s == '-') {
		sign = -1;
		++s;
	}
	else if(*s == '+') {
		sign = 1;
		++s;
	}
	else sign = 1;

	ishex = s[1] == 'X' || s[1] == 'x';
	if(base == 16 && s[0] == '0' && ishex) s += 2;

	/* If BASE is zero, figure it out ourselves.  */
	if(base == 0) {
		if(*s == '0') {
			if(ishex) {
				s += 2;
				base = 16;
			}
			else base = 8;
		}
		else base = 10;
	}

	/* Save the pointer so we can check later if anything happened.  */
	save = s;

	cutoff = ASYS_NATIVE_ULONG_MAX / (asys_native_ulong_t) base;
	limit = ASYS_NATIVE_ULONG_MAX % (asys_native_ulong_t) base;

	overflow = 0;
	i = 0;
	for(c = *s; c != '\0'; c = *++s) {
		if(asys_character_is_digit(c)) c -= '0';
		else if(asys_character_is_letter(c)) {
			c = (char) (asys_character_to_upper(c) - 'A' + 10);
		}
		else break;

		if(c >= base) break;

		/* Check for overflow.  */
		if(i > cutoff || (i == cutoff && c > (char) limit)) overflow = 1;
		else {
			i *= (asys_native_ulong_t) base;
			i += c;
		}
	}

	/* Check if anything actually happened.  */
	if(s == save) goto noconv;

	/* Store in ENDPTR the address of one character
	past the last character we converted.  */
	if(end) *end = (char*) s;

	if(overflow) return ASYS_NATIVE_ULONG_MAX;

	/* Return the result of the appropriate sign.  */
	return (asys_native_long_t) i * sign;

	noconv: {
		/* There was no number to convert.  */
		if(end) *end = (char*) string;
		return 0L;
	}
}
