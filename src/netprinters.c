/* NetPrinters - Source
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
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "compare.h"

#define VERSION "v1.01"

#define IS_WHITESPACE(x) (x == ' ' || x == '\t' || x == '\r' || x == '\n')
#define EPRINTF(...) fprintf(stderr, __VA_ARGS__)

static void print_usage(void);
static char **get_printers(void);
static char *win32_strerr(DWORD errnum);

static void print_usage(void) {
	printf("Usage: netprinters <arguments>\n");
	printf("Arguments:\n\n");
	
	printf("-c <UNC path>\tConnect to a printer\n");
	printf("-d <UNC path>\tSet default printer\n");
	printf("-r <UNC path>\tDisconnect from a printer\n");
	printf("-l\t\tList connected printers\n");
	printf("-q\t\tOnly print error messages\n");
	printf("-s <Filename>\tExecute a netprinters script\n");
	printf("-v\t\tPrint program information\n");
	printf("-h\t\tPrint this usage message\n");
}

/* Returns a NULL-terminated list of connected printers obtained from the
 * EnumPrinters() win32 call, or NULL on error.
*/
static char **get_printers(void) {
	PRINTER_INFO_4 *printers = NULL;
	DWORD size = 0, count, n;
	
	while(!EnumPrinters(PRINTER_ENUM_CONNECTIONS, NULL, 4, (void*)printers, size, &size, &count)) {
		if(GetLastError() != 122 && GetLastError() != 1784) {
			EPRINTF("Can't fetch printers: %s\n", win32_strerr(GetLastError()));
			goto GET_PRINTERS_END;
		}
		
		free(printers);
		if((printers = malloc(size)) == NULL) {
			EPRINTF("Can't allocate %u bytes\n", (unsigned int)size);
			goto GET_PRINTERS_END;
		}
	}
	
	char **retbuf = malloc(sizeof(char*) * (count+1));
	if(!retbuf) {
		EPRINTF("Can't allocate %d bytes\n", (int)(sizeof(char*) * (count+1)));
		goto GET_PRINTERS_END;
	}
	retbuf[count] = NULL;
	
	for(n = 0; n < count; n++) {
		char *pname = printers[n].pPrinterName;
		
		if((retbuf[n] = malloc(strlen(pname)+1)) == NULL) {
			EPRINTF("Can't allocate %u bytes\n", (strlen(pname)+1));
			break;
		}
		strcpy(retbuf[n], pname);
	}
	
	GET_PRINTERS_END:
	free(printers);
	return retbuf;
}

/* Equvilent of the strerr() function, using windows's backwards FormatMessage
 * API call.
 *
 * Returns string stored in static buffer
*/
static char *win32_strerr(DWORD errnum) {
	static char buf[1024] = {'\0'};
	
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errnum, 0, buf, 1023, NULL);
	return buf;	
}

int main(int argc, char** argv) {
	if(argc < 2) {
		print_usage();
		return 1;
	}
	
	printf("Network printer setup utility " VERSION "\n");
	printf("By Daniel Collins\n\n");
	
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
		EPRINTF("Can't init Winsock!\n");
		return 1;
	}
	
	char hostname[512] = {'\0'};
	if(gethostname(hostname, 511) == SOCKET_ERROR) {
		EPRINTF("Can't get hostname: %s\n", strerror(WSAGetLastError()));
		return 1;
	}
	printf("Local hostname:\t\t%s\n", hostname);
	
	FILE* printers_fh = fopen(argv[1], "r");
	if(printers_fh == NULL) {
		EPRINTF("Can't open %s: %s\n", argv[1], strerror(GetLastError()));
		return 1;
	}
	
	char linebuf[1024] = {'\0'};
	while(fgets(linebuf, 1024, printers_fh) != NULL) {
		char* command = NULL;
		char* machine = NULL;
		char* printer = NULL;
		
		size_t pos = 0;
		
		while(IS_WHITESPACE(linebuf[pos])) { pos++; }
		if(linebuf[pos] == '#' || linebuf[pos] == '\0') {
			continue;
		}
		
		command = linebuf+pos;
		while(!IS_WHITESPACE(linebuf[pos])) { pos++; }
		linebuf[pos++] = '\0';
		
		while(IS_WHITESPACE(linebuf[pos])) { pos++; }
		machine = linebuf+pos;
		while(!IS_WHITESPACE(linebuf[pos])) { pos++; }
		linebuf[pos++] = '\0';
		
		while(IS_WHITESPACE(linebuf[pos])) { pos++; }
		printer = linebuf+pos;
		
		for(pos = strlen(printer)-1; IS_WHITESPACE(printer[pos]);) { pos--; }
		printer[pos+1] = '\0';
		
		if(!str_compare(machine, hostname, STR_NOCASE | STR_WILDCARD1)) {
			continue;
		}
		
		if(str_compare(command, "printer", STR_NOCASE)) {
			printf("Adding printer:\t\t%s\n", printer);
			
			if(!AddPrinterConnection(printer)) {
				int errnum = GetLastError();
				
				EPRINTF("Can't add printer %s: %d (%s)\n", printer, errnum, strerror(errnum));
			}
		}
		if(str_compare(command, "default", STR_NOCASE)) {
			printf("Setting default:\t%s\n", printer);
			
			if(!SetDefaultPrinter(printer)) {
				int errnum = GetLastError();
				EPRINTF("Can't set default printer: %s\n", strerror(errnum));
			}
		}
		if(str_compare(command, "exiton", STR_NOCASE)) {
			printf("Exiting on '%s'\n", machine);
			return 0;
		}
	}
	if(ferror(printers_fh)) {
		EPRINTF("Can't read %s: %s\n", argv[1], strerror(GetLastError()));
		return 1;
	}
	
	if(fclose(printers_fh) != 0) {
		EPRINTF("Can't close %s: %s\n", argv[1], strerror(GetLastError()));
		return 1;
	}
	
	printf("\n");
	
	return 0;
}
