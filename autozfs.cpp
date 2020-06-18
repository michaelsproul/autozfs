#include <cstdio>
#include <iostream>
#include <signal.h>
#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>
#include <IOKit/storage/IOStorageProtocolCharacteristics.h>

using namespace std;

DASessionRef sesh;

void removeBlankSpaceFromName(string &name) {
    if (isspace(name[name.size() - 1])) {
        name = name.substr(0, name.size() - 1);
    }
}

void zfsImport(DADiskRef disk, void *ctxt) {
    const string bsdDiskName = DADiskGetBSDName(disk);
    cout << "New disk plugged in." << endl;
    cout << "BSD name assigned to disk: " << bsdDiskName << endl;

    const string partitionTypeCommand = "diskutil list " + bsdDiskName + " | grep ZFS | awk '{print $2}' | tr -d '\n'";
    FILE *fp = popen(partitionTypeCommand.c_str(),"r");
    char partitionType[4];
    if (fgets(partitionType, 4, fp) != NULL) {
        if (strcmp(partitionType, "ZFS") == 0) {
            cout << "ZFS disk detected." << endl;

            const string diskPartitionCommand = "diskutil list " + bsdDiskName + " | grep ZFS | awk '{print $NF}' | tr -d '\n'";
            FILE *fp1 = popen(diskPartitionCommand.c_str(),"r");
            char bsdPartitionNameBuffer[100];
            if (fgets(bsdPartitionNameBuffer, 100, fp1) != NULL) {
                const string bsdPartitionName(bsdPartitionNameBuffer);
                cout << "BSD partition name: " << bsdPartitionName << endl;

                const string zpoolNameCommand = "diskutil info " + bsdPartitionName + " | grep \"Volume Name\" | awk '{ s = \"\"; for (i = 3; i <= NF; i++) s = s $i \" \"; print s }' | tr -d '\n'";
                FILE *fp2 = popen(zpoolNameCommand.c_str(),"r");
                char zpoolNameBuffer[1000];
                if (fgets(zpoolNameBuffer, 1000, fp2) != NULL) {
                    string zpoolName(zpoolNameBuffer);
                    removeBlankSpaceFromName(zpoolName);
                    cout << "Zpool name: " << zpoolName << endl;

                    const string zpoolImportCommand = "/usr/local/bin/zpool import \"" + zpoolName + "\"";
                    int exitCode = system(zpoolImportCommand.c_str());
                    if (exitCode == 0) {
                        cout << "Zpool " << zpoolName << " imported." << endl;
                    } else {
                        cout << "Command: " << zpoolImportCommand << " failed with error: " << exitCode << endl;
                    }
                }
            }
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