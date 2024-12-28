#!/bin/bash

# restart udev so for new rules
# NOTE: Need to issue usbipd attach --wsl --<busid> from Windows powershell 

sudo service udev restart


# make it so mc exits in current dir
source home/vscode/.bashrc
