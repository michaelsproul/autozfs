#include <cstdio>
#include <iostream>
#include <signal.h>
#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>
#include <IOKit/storage/IOStorageProtocolCharacteristics.h>

using namespace std;

DASessionRef sesh;

void zfsImport(DADiskRef disk, void *ctxt) {
    const string bsdDiskName = DADiskGetBSDName(disk);
    cout << "New disk plugged in." << endl;
    cout << "BSD name assigned to disk: " << bsdDiskName << endl;

    string bsdPartitionName = bsdDiskName + "s1";
    DADiskRef diskPartition = DADiskCreateFromBSDName(NULL, sesh, bsdPartitionName.c_str());

    CFDictionaryRef partinfo = DADiskCopyDescription(diskPartition);

    CFStringRef partitionType = (CFStringRef)CFDictionaryGetValue(partinfo, kDADiskDescriptionVolumeKindKey);
    if (CFEqual(partitionType, CFSTR("zfs"))) {
        cout << "ZFS disk detected." << endl;

        CFStringRef volumeNameRef = (CFStringRef)CFDictionaryGetValue(partinfo, kDADiskDescriptionVolumeNameKey);
        const string volumeName = CFStringGetCStringPtr(volumeNameRef, kCFStringEncodingUTF8);
        cout << "Volume name: " << volumeName << endl;

        const string zpoolImportCommand = "/usr/local/bin/zpool import \"" + volumeName + "\"\0";
        int exitCode = system(zpoolImportCommand.c_str());
        if (exitCode == 0) {
            cout << "Zpool " << volumeName << " imported." << endl;
        } else {
            cout << "Command: " << zpoolImportCommand << " failed with error: " << exitCode << endl;
        }
    }
}

void exitHandler(int signum) {
    cout << "Deregistering from system... " << endl;
    DASessionUnscheduleFromRunLoop(sesh, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    CFRelease(sesh);
    exit(signum);
}

int main() {
    sesh = DASessionCreate(kCFAllocatorDefault);

    // Register method for ctrl+c callback
    signal(SIGINT, exitHandler);

    CFMutableDictionaryRef matchingDict = CFDictionaryCreateMutable(
        kCFAllocatorDefault,
        0,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);

    CFDictionaryAddValue(matchingDict,
        kDADiskDescriptionDeviceProtocolKey,
        CFSTR(kIOPropertyPhysicalInterconnectTypeUSB));

    CFDictionaryAddValue(matchingDict,
        kDADiskDescriptionMediaWholeKey,
        kCFBooleanTrue);

    DARegisterDiskPeekCallback(sesh, matchingDict, 0, zfsImport, NULL);

    DASessionScheduleWithRunLoop(sesh, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    CFRunLoopRun();
}