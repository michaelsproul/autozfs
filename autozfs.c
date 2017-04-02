#include <stdlib.h>
#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>
#include <IOKit/storage/IOStorageProtocolCharacteristics.h>

#define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))

// Based on:
// https://developer.apple.com/library/content/documentation/DriversKernelHardware/Conceptual/DiskArbitrationProgGuide/ArbitrationBasics/ArbitrationBasics.html#//apple_ref/doc/uid/TP40009310-CH2-SW2

void zfsImportAll(DADiskRef UNUSED(disk), void * UNUSED(ctxt)) {
    printf("Wubba lubba dub dub! Importing your disks!\n");
    fflush(stdout);
    int exitCode = system("/usr/local/bin/zpool import -a");
    if (exitCode == 0) {
        printf("Done! Run `zpool list` to see if anything was imported.\n");
    } else {
        printf("Oh shit, that failed hard! Exit code for `zpool import -a`: %d\n", exitCode);
    }
    fflush(stdout);
}

int main() {
    DASessionRef sesh = DASessionCreate(kCFAllocatorDefault);

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

    DARegisterDiskPeekCallback(sesh, matchingDict, 0, zfsImportAll, NULL);

    DASessionScheduleWithRunLoop(sesh, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    CFRunLoopRun();

    // TODO: maybe run these on Ctrl-C?
    DASessionUnscheduleFromRunLoop(sesh, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    CFRelease(sesh);
}
