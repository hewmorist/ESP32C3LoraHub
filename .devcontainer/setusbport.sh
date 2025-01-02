#!/bin/bash

sudo rm -f /dev/ttyUSB0
sudo mknod /dev/ttyUSB0 c 188 0 
sudo chmod 666 /dev/ttyUSB0

sudo rm -f /dev/ttyACM0
sudo mknod /dev/ttyACM0 c 166 0
sudo chmod 666 /dev/ttyACM0

sudo usermod -a -G dialout vscode