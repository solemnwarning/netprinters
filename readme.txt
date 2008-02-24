Network printer setup utility (netprinters.exe)

This utility is aimed at creation of networked printers on windows network
clients, it should run on windows 2000 or newer. Compiling the source requires
MS visual studio and MinGW, a pre-compiled binary is included if you don't want
to create your own build.

The configuration file format is: "<command> <machine> [<printer>]"
Printer names may include whitespace, both tabs and spaces may be used as
seperators and lines beginning with a "#" are ignored as comments.

The available commands are:

printer	- Add a networked printer by UNC path
default	- Set a default printer by UNC path
exiton	- Exit the program

Commands are only executed if the <machine> expression matches the hostname of
the local machine, hostnames may contain the * and ? wildcards. The printer and
default commands obviously require a printer name, exiton doesn't.

You can use netprinters.exe in a login script with the following code, assuming
your configuration file is named printers.txt and is in the root of NETLOGON.

NET USE L: %logonserver%\NETLOGON /persistent:no
L:\netprinters.exe L:\printers.txt
net use L: /DELETE

In order to make a custom build of netprinters open the Visual Studio command
line, cd to this directory and run build.bat
