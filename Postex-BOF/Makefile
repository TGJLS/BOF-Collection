CC64 = x86_64-w64-mingw32-gcc
CC86 = i686-w64-mingw32-gcc
STRIP64 = x86_64-w64-mingw32-strip --strip-unneeded
STRIP86 = i686-w64-mingw32-strip --strip-unneeded
CFLAGS = -I ../_include -I _include -w -Wno-incompatible-pointer-types -Os -DBOF -c

all: bof

bof: clean
	@(mkdir _bin 2>/dev/null) && echo 'creating _bin directory' || echo '_bin directory exists'
	@($(CC64) $(CFLAGS) veeam_dumper/veeam_dumper.c -o _bin/veeam_dumper.x64.o && $(STRIP64) _bin/veeam_dumper.x64.o) && echo '[+] veeam_dumper x64' || echo '[!] veeam_dumper x64'
	@($(CC86) $(CFLAGS) veeam_dumper/veeam_dumper.c -o _bin/veeam_dumper.x32.o && $(STRIP86) _bin/veeam_dumper.x32.o) && echo '[+] veeam_dumper x32' || echo '[!] veeam_dumper x32'

clean:
	@(rm -rf _bin)
