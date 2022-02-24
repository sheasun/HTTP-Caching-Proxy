#!/bin/bash
make clean

make

sudo ./main &


while true
do
    sleep 1
done
