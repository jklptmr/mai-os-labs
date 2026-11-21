CC = gcc
CFLAGS = -Wall -Wextra

all: libimpl1.dylib libimpl2.dylib prog1 prog2

libimpl1.dylib: impl1.c contracts.h
	$(CC) $(CFLAGS) -shared -fPIC impl1.c -o libimpl1.dylib

libimpl2.dylib: impl2.c contracts.h
	$(CC) $(CFLAGS) -shared -fPIC impl2.c -o libimpl2.dylib

prog1: prog1.c libimpl1.dylib
	$(CC) $(CFLAGS) prog1.c -L. -limpl1 -Wl,-rpath,. -o prog1

prog2: prog2.c libimpl1.dylib libimpl2.dylib
	$(CC) $(CFLAGS) prog2.c -o prog2

clean:
	rm -f libimpl1.dylib libimpl2.dylib prog1 prog2
