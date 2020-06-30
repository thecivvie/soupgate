@ECHO OFF
REM comprexp.bat - by Tom Torfs, 1998-11-22
REM Rename to comprexp.cmd for OS/2.
REM Translate to an appropriate shell script for Linux.

REM This batch file is an example of how to export mail using
REM SoupGate if your tosser does not support uncompressed mail.

REM Uncompress the compressed mail packets (this must be
REM adjusted for your system)

pkunzip c:\mail\outbound\01a600d1.pnt\*.mo? c:\mailtemp\
pkunzip c:\mail\outbound\01a600d1.pnt\*.tu? c:\mailtemp\
pkunzip c:\mail\outbound\01a600d1.pnt\*.we? c:\mailtemp\
pkunzip c:\mail\outbound\01a600d1.pnt\*.th? c:\mailtemp\
pkunzip c:\mail\outbound\01a600d1.pnt\*.fr? c:\mailtemp\
pkunzip c:\mail\outbound\01a600d1.pnt\*.sa? c:\mailtemp\
pkunzip c:\mail\outbound\01a600d1.pnt\*.su? c:\mailtemp\

REM Export packets from temporary directory

soupgate export /pkt=c:\mailtemp

REM You should make sure to delete the exported mail here
REM (not included for safety reasons during testing)
