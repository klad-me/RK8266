#! /bin/bash

# Получаем номер билда
BUILD=`git rev-list --count HEAD`


# Удаляем старые файлы
if [ -d builds/$BUILD ]; then
    rm -rf builds/$BUILD
fi


# Собираем весь софт
cd soft

cd boot-2apps
make || exit
cd ..

cd EmuAPP
make || exit
cd ..

cd WiFiAPP
make || exit
cd ..

cd ..


# Создаем каталог для билда
mkdir -p builds/$BUILD || exit

# Копируем файлы
cp soft/boot-2apps/out/boot.bin builds/$BUILD/0x00000.bin || exit
cp soft/EmuAPP/out/emu-0x00000.bin builds/$BUILD/0x01000.bin || exit
cp soft/WiFiAPP/out/wifi.1.bin builds/$BUILD/0x10000.bin || exit
cp soft/WiFiAPP/httpfs/httpfs.bin builds/$BUILD/0x70000.bin || exit
