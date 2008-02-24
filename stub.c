/* Setup network printers - MSVC stub code
 *
 * By Daniel Collins <solemnwarning@solemnwarning.net>
 * Released to public domain
*/

#include <windows.h>

int rmain(int argc, char** argv);

int main(int argc, char** argv) {
	return rmain(argc, argv);
}

int set_printer(char const* printer) {
	return SetDefaultPrinter(printer);
}
