#include <stdlib.h>
#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>
#include <IOKit/storage/IOStorageProtocolCharacteristics.h>

#define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))

// Based on:
// https://developer.apple.com/library/content/documentation/DriversKernelHardware/Conceptual/DiskArbitrationProgGuide/ArbitrationBasics/ArbitrationBasics.html#//apple_ref/doc/uid/TP40009310-CH2-SW2

void zfsImportAll(DADiskRef disk, void * UNUSED(ctxt)) {
    const char *bsdDiskName = DADiskGetBSDName(disk);
    printf("New disk plugged in.\n");
    printf("BSD name assigned to disk: %s\n", bsdDiskName);
    fflush(stdout);

    char *partitionTypeCommand;
    asprintf(&partitionTypeCommand, "diskutil list %s | grep ZFS | awk '{print $2}' | tr -d '\n'", bsdDiskName);

    FILE *fp = popen(partitionTypeCommand,"r");
    char partitionType[4];
    if (fgets(partitionType, 4, fp) != NULL) {
        if (strcmp(partitionType, "ZFS") == 0) {
            printf("ZFS disk detected.\n");

            char *zpoolNameCommand;
            asprintf(&zpoolNameCommand, "diskutil list %s | grep ZFS | awk '{print $3}' | tr -d '\n'", bsdDiskName);

            FILE *fp1 = popen(zpoolNameCommand,"r");
            char zpoolName[1000];
            if (fgets(zpoolName, 1000, fp1) != NULL) {
                printf("zpool name: %s\n", zpoolName);

                char *zpoolImportCommand;
                asprintf(&zpoolImportCommand, "/usr/local/bin/zpool import '%s'", zpoolName);

                int exitCode = system(zpoolImportCommand);
                if (exitCode == 0) {
                    printf("zpool %s imported.\n", zpoolName);
                } else {
                    printf("Error! Exit code for `%s`: %d\n", zpoolImportCommand, exitCode);
                }
            }
        }
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
