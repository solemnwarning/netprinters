/* Setup network printers
 * By Daniel Collins <solemnwarning@solemnwarning.net>
 * Released to public domain
 *
 * This must be linked with winspool.lib, ws2_32.lib and stub.c
*/

#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define VERSION "v1.01"

#define IS_WHITESPACE(x) (x == ' ' || x == '\t' || x == '\r' || x == '\n')
#define EPRINTF(...) fprintf(stderr, __VA_ARGS__)

int set_printer(char const* printer);

/* Compare two hostnames, parsing wildcards in 'host1'
 *
 * Returns 1 on hostname match, 0 otherwise
*/
static int host_compare(char const* host1, char const* host2) {
	for(;;) {
		if(tolower(host1[0]) == tolower(host2[0])) {
			if(host1[0] == '\0') {
				break;
			}
			
			host1++;
			host2++;
			continue;
		}
		
		if(host1[0] == '?' && host2[0] != '\0') {
			host1++;
			host2++;
			continue;
		}

		if(host1[0] == '*') {
			if(host2[0] == '\0' || host1[1] == '\0') {
				break;
			}
			if(tolower(host1[1]) == tolower(host2[0])) {
				host1 += 2;
			}
			host2++;
			continue;
		}
		
		return 0;
	}

	return 1;
}

int ncasecmp(char const* a, char const* b) {
	size_t pos = 0;
	
	while(1) {
		if(tolower(a[pos]) != tolower(b[pos])) {
			return 0;
		}
		if(a[pos] == '\0') {
			break;
		}
		
		pos++;
	}
	
	return 1;
}

int rmain(int argc, char** argv) {
	if(argc < 2) {
		EPRINTF("Usage: %s <printer list>\n", argv[0]);
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
		
		if(!host_compare(machine, hostname)) {
			continue;
		}
		
		if(ncasecmp(command, "printer")) {
			printf("Adding printer:\t\t%s\n", printer);
			
			if(!AddPrinterConnection(printer)) {
				int errnum = GetLastError();
				
				EPRINTF("Can't add printer %s: %d (%s)\n", printer, errnum, strerror(errnum));
			}
		}
		if(ncasecmp(command, "default")) {
			printf("Setting default:\t%s\n", printer);
			
			if(!set_printer(printer)) {
				int errnum = GetLastError();
				EPRINTF("Can't set default printer: %s\n", strerror(errnum));
			}
		}
		if(ncasecmp(command, "exiton")) {
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
