CC=clang
CFLAGS=-W -Wall -Wextra -std=c11
LDFLAGS=-framework CoreFoundation -framework DiskArbitration

autozfs: autozfs.c

install:
	sudo cp autozfs /usr/local/bin/
	sudo cp autozfs.plist /Library/LaunchDaemons/

uninstall:
	sudo rm /usr/local/bin/autozfs
	sudo rm /Library/LaunchDaemons/autozfs.plist
	sudo rm /private/var/log/autozfs.err
	sudo rm /private/var/log/autozfs.log

clean:
	rm -f autozfs

.PHONY: clean
