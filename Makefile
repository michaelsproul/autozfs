CC=clang
CFLAGS=-W -Wall -Wextra -std=c11
LDFLAGS=-framework CoreFoundation -framework DiskArbitration

autozfs: autozfs.c

clean:
	rm -f autozfs

.PHONY: clean
