# autozfs

Daemon for OS X that listens for external drives being plugged in and automatically imports
ZFS pools.

If you use or want to use ZFS on an external hard drive, you might find this useful :)

## Installation

You'll need [OpenZFSOnOSX][] or similar installed.

To build:

```bash
$ git clone https://github.com/michaelsproul/autozfs.git
$ cd autozfs
$ make
```

To install:

```bash
$ sudo cp autozfs /usr/local/bin/
$ sudo cp autozfs.plist /Library/LaunchDaemons/
```

Then either reboot, or run:

```bash
$ sudo launchctl load /Library/LaunchDaemons/autozfs.plist
```

## How does it work?

A background daemon started via `launchd` and running as root listens for "Disk Arbitration"
events, and each time it detects a disk connection it runs `zpool import -a`. Simple!

## Debugging

* You should be able to see a process called `autozfs` running, check Activity Monitor, `ps`,
  `htop` or whatever.
* The dameon logs inane messages to `/private/var/log/autozfs.{log,err}`.

## Caveats

* OS X only (for now).
* IMPORTS ALL POOLS. This is hacky.
* Assumes `zpool` is installed in `/usr/local/bin`. This may not be true of all systems...
* Only auto-mounts USB devices, but it's an artifical restriction -- delete the USB stuff
  in the source and it should do Firewire/Thunderbolt/whatever just fine.

## License

Copyright (c) 2017, Michael Sproul.

Licensed under the terms of the 3-clause BSD license.

[OpenZFSOnOSX]: https://openzfsonosx.org/wiki/Downloads
