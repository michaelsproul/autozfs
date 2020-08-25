#include <cstdio>
#include <iostream>
#include <signal.h>
#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>
#include <IOKit/storage/IOStorageProtocolCharacteristics.h>

using namespace std;

DASessionRef sesh;

// This is required to get rid of the newline character from the end of the name.
string removeLastCharacterFromName(char *name) {
    if (isspace(name[strlen(name) - 1])) {
        name[strlen(name) - 1] = '\0';
    }
    return name;
}

void mountZFSDatasets(string poolName) {
    // Loop through all datasets
    const string zfsDatasetNamesCommand = "/usr/local/bin/zfs list -t filesystem -H -o name";
    FILE *fp = popen(zfsDatasetNamesCommand.c_str(), "r");
    char datasetName[1000];
    while (fgets(datasetName, 1000, fp) != NULL) {
        string properDatasetName = removeLastCharacterFromName(datasetName);
        size_t foundSlash = properDatasetName.find("/");
        if (foundSlash != string::npos) {
            string token = properDatasetName.substr(0, properDatasetName.find("/"));
            string volumeName = properDatasetName.substr(properDatasetName.find("/") + 1);
            if (token.compare(poolName) == 0) {
                // This dataset belongs to this zpool.
                const string checkMountStatusCommand = "/usr/local/bin/zfs get -H -o value mounted \"" + properDatasetName + "\"";
                FILE *fp1 = popen(checkMountStatusCommand.c_str(), "r");
                char mountedStatusOutput[3];
                if (fgets(mountedStatusOutput, 3, fp1) != NULL) {
                    string mountedStatus = removeLastCharacterFromName(mountedStatusOutput);
                    if (mountedStatus.compare("no") == 0) {
                        // Mount this dataset.
                        const string datasetMountCommand = "security find-generic-password -a \"" + volumeName + "\" -w | sudo /usr/local/bin/zfs mount -l \"" + properDatasetName + "\"";
                        int exitCode = system(datasetMountCommand.c_str());
                        if (exitCode == 0) {
                            cout << "Dataset " << properDatasetName << " mounted." << endl;
                        } else {
                            cout << "Command: " << datasetMountCommand << " failed with error: " << exitCode << endl;
                        }
                    }
                }
            }
        }
    }
}

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

        const string zpoolImportCommand = "sudo /usr/local/bin/zpool import \"" + volumeName + "\"\0";
        int exitCode = system(zpoolImportCommand.c_str());
        if (exitCode == 0) {
            cout << "Zpool " << volumeName << " imported." << endl;
            mountZFSDatasets(volumeName);
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