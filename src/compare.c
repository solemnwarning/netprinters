/* Flexible string comparison function with wildcard support
 * Version 1.23 (21/12/2007)
 * Copyright (C) 2007 Daniel Collins <solemnwarning@solemnwarning.net>
 *
 * Released to public domain, I take no responsibility for this code or it's
 * users, it may work, it may not, it may destroy the universe but it has no
 * warranty, so no sending lawyers my way if this code breaks somthing.
 *
 * If you find a bug please email me :)
*/

/* Version history:
 *
 * 1.0 (11/06/2007):
 * Initial release
 *
 * 1.1 (11/06/2007):
 * Fixed a bug:
 *	maxlen was read with stdarg, whether the STR_MAXLEN flag was set or
 *	not, this causes undefined behavior according to C so it could crash
 *	or cause other errors on some platforms.
 *
 * 1.2 (23/08/2007):
 * Rewrote to be more flexible
 * Fixed a bug:
 *	The * wildcard between two characters in one string caused a mismatch
 *	when the other string has 0 characters in its place because I used ++
 *	where I should have used += 2.
 *
 * 1.21 (09/09/2007):
 * Created a header for this function (compare.h)
 *
 * 1.22 (06/11/2007):
 * Added code to make str_compare() handle NULL pointers as strings.
 *	(It treats both NULL as match, one NULL as mismatch)
 *
 * 1.23 (21/12/2007):
 * Fixed a bug:
 *	If STR_MAXLEN was used and a maxlen of zero was passed, str_compare()
 *	would act as if STR_MAXLEN had not been used.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

#include "compare.h"

/* Compare str1 and str2
 * Returns 1 on match, 0 on mismatch
*/
int str_compare(char const* str1, char const* str2, int flags, ...) {
	if(str1 == NULL && str2 == NULL) {
		return(1);
	}
	if(str1 == NULL || str2 == NULL) {
		return(0);
	}
	
	size_t maxlen = 0;
	size_t compared = 0;
	
	va_list arglist;
	va_start(arglist, flags);
	if(flags & STR_MAXLEN) {
		maxlen = va_arg(arglist, size_t);
	}
	va_end(arglist);
	
	for(; !(flags & STR_MAXLEN) || compared < maxlen; compared++) {
		if(str1[0] == str2[0]) {
			if(str1[0] == '\0') {
				break;
			}
			
			str1++;
			str2++;
			continue;
		}
		if(flags & STR_NOCASE && (tolower(str1[0]) == tolower(str2[0]))) {
			str1++;
			str2++;
			continue;
		}
		if((flags & STR_WILDCARDS || flags & STR_WILDCARD1) && (str1[0] == '*' || str1[0] == '?')) {
			if(str1[0] == '?' && str2[0] != '\0') {
				str1++;
				str2++;
				continue;
			}
			if(str1[0] == '*') {
				if(str2[0] == '\0' || str1[1] == '\0') {
					break;
				}
				if(str1[1] == str2[0]) {
					str1 += 2;
				}
				str2++;
				continue;
			}
		}
		if((flags & STR_WILDCARDS || flags & STR_WILDCARD2) && (str2[0] == '*' || str2[0] == '?')) {
			if(str2[0] == '?' && str1[0] != '\0') {
				str1++;
				str2++;
				continue;
			}
			if(str2[0] == '*') {
				if(str1[0] == '\0' || str2[1] == '\0') {
					break;
				}
				if(str2[1] == str1[0]) {
					str2 += 2;
				}
				str1++;
				continue;
			}
		}
		
		return(0);
	}
	return(1);
}
