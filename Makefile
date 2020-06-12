CC=clang
CFLAGS=-W -Wall -Wextra -std=c11
LDFLAGS=-framework CoreFoundation -framework DiskArbitration

autozfs: autozfs.c

install:
	sudo cp autozfs /usr/local/bin/
	sudo cp autozfs.plist /Library/LaunchDaemons/

clean:
	rm -f autozfs

.PHONY: clean
