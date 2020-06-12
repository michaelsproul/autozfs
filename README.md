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
events. Each time it detects a disk connection it checks to see if the disk contains a ZFS partition.
If it finds a ZFS partition, it gets the name of the zpool from the partition name and imports it.
It then checks for top level datasets that belong to this new zpool and mounts them. 

## Assumptions

* zpool names contain no numbers, only alphabets and spaces.
* The passwords for any encrypted datasets are stored in the user's macOS keychain.
* Each zpool contains only one level of datasets within it. Multi-level dataset hierarchies wont automount.

## Debugging

* You should be able to see a process called `autozfs` running, check Activity Monitor, `ps`,
  `htop` or whatever.
* The daemon logs inane messages to `/private/var/log/autozfs.{log,err}`.

## Caveats

* OS X only (for now).
* IMPORTS ALL POOLS. This is hacky.
* Assumes `zpool` and `zfs` is installed in `/usr/local/bin`. This may not be true of all systems...
* Only auto-mounts USB devices, but it's an artificial restriction -- delete the USB stuff
  in the source and it should do Firewire/Thunderbolt/whatever just fine.

## License

Copyright (c) 2017, Michael Sproul.

Licensed under the terms of the 3-clause BSD license.

[OpenZFSOnOSX]: https://openzfsonosx.org/wiki/Downloads
