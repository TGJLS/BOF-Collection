CC64 = x86_64-w64-mingw32-gcc
CC86 = i686-w64-mingw32-gcc
STRIP64 = x86_64-w64-mingw32-strip --strip-unneeded
STRIP86 = i686-w64-mingw32-strip --strip-unneeded
CFLAGS = -I ../_include -I _include -w -Wno-incompatible-pointer-types -Os -DBOF -c

all: bof

bof: clean
	@(mkdir _bin 2>/dev/null) && echo 'creating _bin directory' || echo '_bin directory exists'
	@($(CC64) $(CFLAGS) list/list.c -o _bin/list.x64.o && $(STRIP64) _bin/list.x64.o) && echo '[+] list x64' || echo '[!] list x64'
	@($(CC86) $(CFLAGS) list/list.c -o _bin/list.x32.o && $(STRIP86) _bin/list.x32.o) && echo '[+] list x32' || echo '[!] list x32'
	@($(CC64) $(CFLAGS) kill/kill.c -o _bin/kill.x64.o && $(STRIP64) _bin/kill.x64.o) && echo '[+] kill x64' || echo '[!] kill x64'
	@($(CC86) $(CFLAGS) kill/kill.c -o _bin/kill.x32.o && $(STRIP86) _bin/kill.x32.o) && echo '[+] kill x32' || echo '[!] kill x32'
	@($(CC64) $(CFLAGS) run/run.c -o _bin/run.x64.o && $(STRIP64) _bin/run.x64.o) && echo '[+] run x64' || echo '[!] run x64'
	@($(CC86) $(CFLAGS) run/run.c -o _bin/run.x32.o && $(STRIP86) _bin/run.x32.o) && echo '[+] run x32' || echo '[!] run x32'
	@($(CC64) $(CFLAGS) grep/grep.c -o _bin/grep.x64.o && $(STRIP64) _bin/grep.x64.o) && echo '[+] grep x64' || echo '[!] grep x64'
	@($(CC86) $(CFLAGS) grep/grep.c -o _bin/grep.x32.o && $(STRIP86) _bin/grep.x32.o) && echo '[+] grep x32' || echo '[!] grep x32'
	@($(CC64) $(CFLAGS) suspend/suspend.c -o _bin/suspend.x64.o && $(STRIP64) _bin/suspend.x64.o) && echo '[+] suspend x64' || echo '[!] suspend x64'
	@($(CC86) $(CFLAGS) suspend/suspend.c -o _bin/suspend.x32.o && $(STRIP86) _bin/suspend.x32.o) && echo '[+] suspend x32' || echo '[!] suspend x32'
	@($(CC64) $(CFLAGS) resume/resume.c -o _bin/resume.x64.o && $(STRIP64) _bin/resume.x64.o) && echo '[+] resume x64' || echo '[!] resume x64'
	@($(CC86) $(CFLAGS) resume/resume.c -o _bin/resume.x32.o && $(STRIP86) _bin/resume.x32.o) && echo '[+] resume x32' || echo '[!] resume x32'

clean:
	@(rm -rf _bin)
