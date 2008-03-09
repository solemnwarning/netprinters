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
#include <lm.h>
#include <security.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define VERSION "v2.1"
#define WHITESPACE "\r\n\t "

#define EPRINTF(...) fprintf(stderr, __VA_ARGS__)
#define ARGN_IS(arg) (strcmp(argv[argn], arg) == 0)
#define fprintf(fh, ...) fprintf(fh, __VA_ARGS__); fflush(fh);
#define printf(...) printf(__VA_ARGS__); fflush(stdout);

static void print_usage(void);
static char **get_printers(void);
static char *win32_strerr(DWORD errnum);
static void list_printers(void);
static void connect_printer(char *printer);
static void default_printer(char *printer);
static void disconnect_printer(char *printer);
static void disconnect_by_expr(char *expr);
static void exec_script(char const *filename);
static void load_env(void);
static int expr_compare(char const *str, char const *expr);

static struct {
	char username[1024];
	char nbname[1024];
} userenv = {{'\0'},{'\0'}};

static void print_usage(void) {
	printf("Usage: netprinters.exe <arguments>\n");
	printf("Arguments:\n\n");
	
	printf("-c <UNC path>\tConnect to a printer\n");
	printf("-d <UNC path>\tSet default printer\n");
	printf("-r <expression>\tDelete any matching printer connections\n");
	printf("-l\t\tList connected printers\n");
	printf("-s <filename>\tExecute a netprinters script\n");
}

/* Returns a NULL-terminated list of connected printers obtained from the
 * EnumPrinters() win32 call, or NULL on error.
*/
static char **get_printers(void) {
	PRINTER_INFO_4 *printers = NULL;
	DWORD size = 0, count, n;
	char **retbuf = NULL;
	
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
	
	if(!(retbuf = malloc(sizeof(char*) * (count+1)))) {
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
	buf[strcspn(buf, "\r\n")] = '\0';
	return buf;	
}

/* List printers to stdout */
static void list_printers(void) {
	char **printers = get_printers();
	unsigned int pnum = 0;
	
	while(printers[pnum]) {
		puts(printers[pnum]);
		free(printers[pnum]);
		
		pnum++;
	}
	
	free(printers);
}

/* Connect to a network printer */
static void connect_printer(char *printer) {
	if(AddPrinterConnection((char*)printer)) {
		printf("Added printer:\t\t%s\n", printer);
	}else{
		EPRINTF(
			"Can't connect to printer %s: %s\n",
			printer, win32_strerr(GetLastError())
		);
	}
}

/* Set default printer */
static void default_printer(char *printer) {
	if(SetDefaultPrinter(printer)) {
		printf("Set default printer:\t%s\n", printer);
	}else{
		EPRINTF(
			"Can't make printer %s default: %s\n",
			printer, win32_strerr(GetLastError())
		);
	}
}

/* Disconnect from a printer */
static void disconnect_printer(char *printer) {
	if(DeletePrinterConnection(printer)) {
		printf("Disconnected from:\t%s\n", printer);
	}else{
		EPRINTF(
			"Can't disconnect from printer %s: %s\n",
			printer, win32_strerr(GetLastError())
		);
	}
}

/* Disconnect from any printers matching the supplied expression */
static void disconnect_by_expr(char *expr) {
	char **printers = get_printers();
	unsigned int pnum = 0;
	
	while(printers[pnum]) {
		char *pname = printers[pnum++];
		
		if(expr_compare(pname, expr)) {
			disconnect_printer(pname);
		}
		
		free(pname);
	}
	
	free(printers);
}

/* Parse and execute a NetPrinters script */
static void exec_script(char const *filename) {
	FILE *fh = fopen(filename, "r");
	if(!fh) {
		EPRINTF(
			"Can't open script %s: %s\n", filename,
			win32_strerr(GetLastError())
		);
		return;
	}
	
	char buf[1024];
	unsigned int lnum = 1;
	int sblock = 0;
	
	while(fgets(buf, 1024, fh)) {
		char *name = buf+strspn(buf, WHITESPACE);
		char *value = name+strcspn(name, WHITESPACE);
		
		if(value[0] != '\0') {
			value[0] = '\0';
			value++;
			value += strspn(value, WHITESPACE);
			value[strcspn(value, "\r\n")] = '\0';
		}
		
		if(name[0] == '\0') {
			sblock = 0;
		}
		if(name[0] == '#' || name[0] == '\0' || sblock) {
			lnum++;
			continue;
		}
		
		if(expr_compare(name, "AddPrinter")) {
			connect_printer(value);
		}else if(expr_compare(name, "DefaultPrinter")) {
			default_printer(value);
		}else if(expr_compare(name, "DeletePrinter")) {
			disconnect_by_expr(value);
		}else if(expr_compare(name, "NetBIOS")) {
			if(!expr_compare(userenv.nbname, value)) {
				sblock = 1;
			}
		}else if(expr_compare(name, "!NetBIOS")) {
			if(expr_compare(userenv.nbname, value)) {
				sblock = 1;
			}
		}else if(expr_compare(name, "Username")) {
			if(!expr_compare(userenv.username, value)) {
				sblock = 1;
			}
		}else if(expr_compare(name, "!Username")) {
			if(expr_compare(userenv.username, value)) {
				sblock = 1;
			}
		}else{
			EPRINTF(
				"Unknown directive %s at line %u\n",
				name, lnum
			);
		}
		
		lnum++;
	}
	
	if(fclose(fh) != 0) {
		EPRINTF(
			"Can't close script %s: %s\n", filename,
			win32_strerr(GetLastError())
		);
	}
}

/* Read the environment information into the userenv structure */
static void load_env(void) {
	DWORD bsize;
	
	bsize = sizeof(userenv.nbname)-1;
	GetComputerName(userenv.nbname, &bsize);
	
	bsize = sizeof(userenv.username)-1;
	GetUserName(userenv.username, &bsize);
}

/* Compare the supplied string and expression
 * Returns 1 upon match, zero otherwise.
*/
static int expr_compare(char const *str, char const *expr) {
	while(1) {
		if(expr[0] == '\0' && str[0] != '\0') {
			return 0;
		}
		if(str[0] == '\0') {
			if(expr[0] == '\0' || expr[0] == '*') {
				break;
			}
			
			return 0;
		}
		
		if(expr[0] == '*') {
			if(expr[1] == str[0]) {
				expr += 2;
			}
			str++;
			
			continue;
		}
		if(expr[0] == '?' && str[0] != '\0') {
			expr++;
			str++;
			
			continue;
		}
		if(expr[0] == '#' && isdigit(str[0])) {
			expr++;
			str++;
			
			continue;
		}
		if(tolower(expr[0]) == tolower(str[0])) {
			expr++;
			str++;
			
			continue;
		}
		
		return 0;
	}
	
	return 1;
}

int main(int argc, char** argv) {
	if(argc < 2) {
		print_usage();
		return 1;
	}
	
	printf("NetPrinters " VERSION "\n");
	printf("Copyright (C) 2008 Daniel Collins\n\n");
	
    	if(LOBYTE(LOWORD(GetVersion())) < 5) {
		EPRINTF("This program requires Windows 2000 or newer\n");
		return 1;
	}
	
	load_env();
	
	int argn = 1;
	while(argn < argc) {
		if(ARGN_IS("-c")) {
			if((argn + 1) == argc) {
				EPRINTF("-c requires an argument");
				return 1;
			}
			
			connect_printer(argv[++argn]);
		}else if(ARGN_IS("-d")) {
			if((argn + 1) == argc) {
				EPRINTF("-d requires an argument");
				return 1;
			}
			
			default_printer(argv[++argn]);
		}else if(ARGN_IS("-r")) {
			if((argn + 1) == argc) {
				EPRINTF("-r requires an argument");
				return 1;
			}
			
			disconnect_by_expr(argv[++argn]);
		}else if(ARGN_IS("-l")) {
			list_printers();
			return 0;
		}else if(ARGN_IS("-s")) {
			if((argn + 1) == argc) {
				EPRINTF("-s requires an argument");
				return 1;
			}
			
			exec_script(argv[++argn]);
		}else{
			EPRINTF("Unknown argument: %s\n", argv[argn]);
			return 1;
		}
		
		argn++;
	}
	
	return 0;
}
