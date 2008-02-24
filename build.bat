rem Build netprinters.exe
rem Must be executed from the VS command shell

C:\MinGW\bin\gcc.exe -Wall -c -o netprinters.obj netprinters.c
CL stub.c netprinters.obj /link winspool.lib ws2_32.lib /out:netprinters.exe
