#! /bin/bash
./mkfs
curl -T httpfs.bin http://192.168.1.50/httpfs.bin
