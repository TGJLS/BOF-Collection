CC64 = x86_64-w64-mingw32-gcc
CC86 = i686-w64-mingw32-gcc
STRIP64 = x86_64-w64-mingw32-strip --strip-unneeded
STRIP86 = i686-w64-mingw32-strip --strip-unneeded
CFLAGS = -I . -I ../_include -w -Wno-incompatible-pointer-types -Os -DBOF -c

all: bof

bof: clean
	@(mkdir _bin 2>/dev/null) && echo 'creating _bin directory' || echo '_bin directory exists'
	@($(CC64) $(CFLAGS) steal/steal.c -o _bin/steal.x64.o && $(STRIP64) _bin/steal.x64.o) && echo '[+] steal x64' || echo '[!] steal x64'
	@($(CC86) $(CFLAGS) steal/steal.c -o _bin/steal.x32.o && $(STRIP86) _bin/steal.x32.o) && echo '[+] steal x32' || echo '[!] steal x32'
	@($(CC64) $(CFLAGS) use/use.c -o _bin/use.x64.o && $(STRIP64) _bin/use.x64.o) && echo '[+] use x64' || echo '[!] use x64'
	@($(CC86) $(CFLAGS) use/use.c -o _bin/use.x32.o && $(STRIP86) _bin/use.x32.o) && echo '[+] use x32' || echo '[!] use x32'
	@($(CC64) $(CFLAGS) make/make.c -o _bin/make.x64.o && $(STRIP64) _bin/make.x64.o) && echo '[+] make x64' || echo '[!] make x64'
	@($(CC86) $(CFLAGS) make/make.c -o _bin/make.x32.o && $(STRIP86) _bin/make.x32.o) && echo '[+] make x32' || echo '[!] make x32'
	@($(CC64) $(CFLAGS) rm/rm.c -o _bin/rm.x64.o && $(STRIP64) _bin/rm.x64.o) && echo '[+] rm x64' || echo '[!] rm x64'
	@($(CC86) $(CFLAGS) rm/rm.c -o _bin/rm.x32.o && $(STRIP86) _bin/rm.x32.o) && echo '[+] rm x32' || echo '[!] rm x32'
	@($(CC64) $(CFLAGS) revert/revert.c -o _bin/revert.x64.o && $(STRIP64) _bin/revert.x64.o) && echo '[+] revert x64' || echo '[!] revert x64'
	@($(CC86) $(CFLAGS) revert/revert.c -o _bin/revert.x32.o && $(STRIP86) _bin/revert.x32.o) && echo '[+] revert x32' || echo '[!] revert x32'
	@($(CC64) $(CFLAGS) privget/privget.c -o _bin/privget.x64.o && $(STRIP64) _bin/privget.x64.o) && echo '[+] privget x64' || echo '[!] privget x64'
	@($(CC86) $(CFLAGS) privget/privget.c -o _bin/privget.x32.o && $(STRIP86) _bin/privget.x32.o) && echo '[+] privget x32' || echo '[!] privget x32'

clean:
	@(rm -rf _bin)
