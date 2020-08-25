CXX=g++
CXXFLAGS=-Wall -pedantic -g -std=c++17
CC=$(CXX)
CFLAGS=$(CXXFLAGS)
LDFLAGS=-framework CoreFoundation -framework DiskArbitration

autozfs: autozfs.cpp

run: autozfs
	./autozfs

install: autozfs
	sudo cp autozfs /usr/local/bin/
	sudo cp autozfs.plist /Library/LaunchDaemons/
	sudo launchctl load -w /Library/LaunchDaemons/autozfs.plist

uninstall:
	sudo launchctl unload -w /Library/LaunchDaemons/autozfs.plist
	sudo rm /usr/local/bin/autozfs
	sudo rm /Library/LaunchDaemons/autozfs.plist
	sudo rm /private/var/log/autozfs.err
	sudo rm /private/var/log/autozfs.log

start:
	sudo launchctl load -w /Library/LaunchDaemons/autozfs.plist

stop:
	sudo launchctl unload -w /Library/LaunchDaemons/autozfs.plist

restart: stop start

clean:
	rm -f autozfs

.PHONY: clean
