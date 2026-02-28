all : hexiedit hexiedit.exe

CFLAGS:=-I../rawdraw
LDFLAGS:=-lX11 -lm

hexiedit : hexiedit.c
	gcc -o $@ $^ $(CFLAGS) $(LDFLAGS)

hexiedit.exe : hexiedit.c
	x86_64-w64-mingw32-gcc -o $@ $^ -Os -s -lgdi32 -luser32 -I../rawdraw -ffunction-sections -flto

clean :
	rm -rf hexiedit hexiedit.exe

