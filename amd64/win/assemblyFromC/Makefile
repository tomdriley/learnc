# Makefile for Windows (MSVC)

CC = cl
ASM = ml64
RM = del /Q

a.exe: ext.obj main.obj
	$(CC) ext.obj main.obj /Fea.exe

main.obj: main.c
	$(CC) /c main.c

ext.obj: ext.asm
	$(ASM) /c ext.asm

clean:
	$(RM) *.obj a.exe
