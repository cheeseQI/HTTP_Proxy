#!/bin/bash
make clean
make
echo 'start program...'
./main &
while true ; do continue ; done