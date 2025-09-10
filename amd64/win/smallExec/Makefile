# NMAKEFILE for small Windows executable
# Usage: nmake 

LDFLAGS = /link /entry:main

main.exe: main.obj
	cl $** User32.lib $(LDFLAGS) /Fe:$@

main.obj: main.c
	cl /c $**

clean:
	del *.obj *.exe
