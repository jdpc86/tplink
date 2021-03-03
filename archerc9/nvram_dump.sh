#!/bin/ash -x
for p in `nvrammanager --show | awk '{print $5}'` 

do

nvrammanager -r /mnt/sda1/nvram_$p.txt -p $p

done
