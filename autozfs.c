#include <stdlib.h>
#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>
#include <IOKit/storage/IOStorageProtocolCharacteristics.h>

#define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))

// Based on:
// https://developer.apple.com/library/content/documentation/DriversKernelHardware/Conceptual/DiskArbitrationProgGuide/ArbitrationBasics/ArbitrationBasics.html#//apple_ref/doc/uid/TP40009310-CH2-SW2

char * removeLastCharacterFromDatasetName(char *datasetName) {
    datasetName[strlen(datasetName) - 1] = '\0';
    return datasetName;
}

char * getPoolNameFromDiskUtilLine(char *line) {
    char * token = strtok(line, " "); // Get first column
    token = strtok(NULL, " "); // Get second column
    token = strtok(NULL, " "); // Get third column (this is where the name starts)

    char *name = "";
    asprintf(&name, "%s", token);
    token = strtok(NULL, " "); // Get fourth column
    while (token != NULL) {
        if (isdigit(token[0])) {
            break;
        }
        asprintf(&name, "%s %s", name, token);
        token = strtok(NULL, " ");
    }

    return name;
}

void mountZFSDatasetsForPool(char *zpoolName) {
    char *zfsDatasetNamesCommand = "/usr/local/bin/zfs list -t filesystem -H -o name";

    FILE *fp = popen(zfsDatasetNamesCommand,"r");
    char datasetName[1000];
    while (fgets(datasetName, 1000, fp) != NULL) {
        char *properDatasetName = removeLastCharacterFromDatasetName(datasetName);

        char * token = strtok(properDatasetName, "/");
        if (strcmp(token, zpoolName) == 0) {
            token = strtok(NULL, "/");
            if (token != NULL) {
                char *mountableDatasetName;
                asprintf(&mountableDatasetName, "%s/%s", properDatasetName, token);

                char *datasetMountCommand;
                asprintf(&datasetMountCommand, "security find-generic-password -a '%s' -w | /usr/local/bin/zfs mount -l '%s'", token, mountableDatasetName);

                int exitCode = system(datasetMountCommand);
                if (exitCode == 0) {
                    printf("dataset %s mounted.\n", mountableDatasetName);
                } else {
                    printf("Error! Exit code for `%s`: %d\n", datasetMountCommand, exitCode);
                }
            }
        }
    }
}

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
            asprintf(&zpoolNameCommand, "diskutil list %s | grep ZFS | tr -d '\n'", bsdDiskName);

            FILE *fp1 = popen(zpoolNameCommand,"r");
            char zpoolNameLine[1000];
            if (fgets(zpoolNameLine, 1000, fp1) != NULL) {
                char *zpoolName = getPoolNameFromDiskUtilLine(zpoolNameLine);
                printf("zpool name: %s\n", zpoolName);

                char *zpoolImportCommand;
                asprintf(&zpoolImportCommand, "/usr/local/bin/zpool import '%s'", zpoolName);

                int exitCode = system(zpoolImportCommand);
                if (exitCode == 0) {
                    printf("zpool %s imported.\n", zpoolName);
                    mountZFSDatasetsForPool(zpoolName);
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
