# autozfs

Daemon for OS X that listens for external drives being plugged in and automatically imports
ZFS pools.

If you use or want to use ZFS on an external hard drive, you might find this useful :)

## Installation

You'll need [OpenZFSOnOSX][] or similar installed.

To build and install:

```bash
$ git clone https://github.com/virajchitnis/autozfs.git
$ cd autozfs
$ make install
```

2. Open the `Automator` app and create a new `Folder Action`. 
3. Choose `/Volumes` as the folder.
4. Drag the `Run Shell Script` action from the left pane to the right pane and paste the location of the `zpool_mount_all.py` script into the box.

![Folder Action Screenshot](/Folder_Action.png?raw=true "Folder Action Screenshot")

## Persmissions

In order for this service to work as expected, you need to allow `autozfs` `Full Disk Access` in the macOS privacy pane. This can only be done after the first disk has been plugged in. Until the zpool import for the first disk fails, `autozfs` **will not** show up in the privacy pane.

## How does it work?

A background daemon started via `launchd` and running as root listens for "Disk Arbitration"
events. Each time it detects a disk connection it checks to see if the disk contains a ZFS partition.
If it finds a ZFS partition, it gets the name of the zpool from the partition name and imports it.

The python script for mounting all datasets simply loops through all datasets and mounts ones where the
`mounted` property returns `no`.

## Debugging

* You should be able to see a process called `autozfs` running, check Activity Monitor, `ps`,
  `htop` or whatever.
* The daemon logs inane messages to `/private/var/log/autozfs.{log,err}`.

## Caveats

* OS X only.
* Assumes `zpool` and `zfs` is installed in `/usr/local/bin`. This may not be true of all systems...
* Only auto-mounts USB devices, but it's an artificial restriction -- delete the USB stuff
  in the source and it should do Firewire/Thunderbolt/whatever just fine.

## License

Copyright (c) 2017, Michael Sproul.

Licensed under the terms of the 3-clause BSD license.

[OpenZFSOnOSX]: https://openzfsonosx.org/wiki/Downloads
