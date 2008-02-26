/* NetPrinters - Printer functions
 * Copyright (C) 2008 Daniel Collins <solemnwarning@solemnwarning.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *	* Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 *
 *	* Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
 *
 *	* Neither the name of the author nor the names of its contributors may
 *	  be used to endorse or promote products derived from this software
 *	  without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define EPRINTF(...) fprintf(stderr, __VA_ARGS__)

/* Like the perror() function, but uses windows's backwards API to get error
 * messages
*/
static char* np_errmsg(void) {
	static char errmsg[1024] = {'\0'};
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		0,
		errmsg,
		1023,
		NULL
	);
	errmsg[strcspn(errmsg, "\n\r")] = '\0';
	
	return errmsg;
}

/* Add a printer connection by UNC path */
void printer_add(char* uncpath) {
	if(!AddPrinterConnection(uncpath)) {
		EPRINTF("Can't add printer %s: %s\n", uncpath, np_errmsg());
	}else{
		printf("Added printer:\t\t%s\n", uncpath);
	}
}

/* Set default printer by UNC path */
void printer_default(char* uncpath) {
	if(!SetDefaultPrinter(uncpath)) {
		EPRINTF("Can't set default printer %s: %s\n", uncpath, np_errmsg());
	}else{
		printf("Set default printer:\t%s\n", uncpath);
	}
}
