#!/usr/bin/env python3

import subprocess

# Loop through all ZFS volumes
zfs_list_result = subprocess.run(['/usr/local/bin/zfs', 'list', '-t', 'filesystem', '-H', '-o', 'name'], stdout=subprocess.PIPE).stdout.decode('utf-8').strip()
for volumne_name in zfs_list_result.split('\n'):
    if "/" in volumne_name:
        mounted_result = subprocess.run(['/usr/local/bin/zfs', 'get', '-H', '-o', 'value', 'mounted', volumne_name], stdout=subprocess.PIPE).stdout.decode('utf-8').strip()
        if mounted_result == "no":
            print("Mounting " + volumne_name + "...")
            dataset_name = volumne_name.split('/')[1]
            password = subprocess.run(['security', 'find-generic-password', '-a', dataset_name, '-w'], stdout=subprocess.PIPE).stdout.decode('utf-8').strip()
            subprocess.run(['sudo', '/usr/local/bin/zfs', 'mount', '-l', volumne_name], input=password.encode('utf-8'), check=True)