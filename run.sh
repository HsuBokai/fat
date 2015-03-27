#!/bin/bash

device="/dev/sdc1"
dirName="temp"
targetFile="boot.bin"

exit 0
set -x

sudo mkfs.fat $device
mkdir $dirName
sudo mount $device $dirName
sudo cp $targetFile $dirName
#wait ${!}
sleep 1
sudo umount $dirName
rmdir $dirName
sudo head -n 100 $device > dump

