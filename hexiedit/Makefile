all : hexiedit

CFLAGS:=-I../rawdraw -lX11 -lm

hexiedit : hexiedit.c
	gcc -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean :
	rm -rf hexiedit
