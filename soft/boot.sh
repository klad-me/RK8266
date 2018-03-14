#! /bin/bash

cd boot-2apps
make || exit
cd ..

cd EmuAPP
make || exit
cd ..

cd WiFiAPP
make || exit
cd ..

/home/heavy/KLAD/x-tools/esp8266/xtensa-lx106-elf/bin/esptool.py	\
    --port /dev/ttyUSB0 			\
    write_flash					\
    --flash_size 8m				\
    0x00000	boot-2apps/out/boot.bin		\
    0x01000	EmuAPP/out/emu-0x00000.bin	\
    0x10000	WiFiAPP/out/wifi.1.bin		\
    0x70000	WiFiAPP/httpfs/httpfs.bin	\
    || exit

telnet localhost 60485
