#! /bin/bash

./mkhelp || exit

/home/heavy/KLAD/x-tools/esp8266/xtensa-lx106-elf/bin/esptool.py	\
    --port /dev/ttyUSB0 			\
    write_flash					\
    --flash_size 8m				\
    --flash_freq 20m				\
    --flash_mode dio				\
    0x50000	help.bin			\
    || exit

/home/heavy/KLAD/x-tools/esp8266/xtensa-lx106-elf/bin/esptool.py run || exit

telnet localhost 60485
