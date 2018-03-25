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

cd mkfw
rm -f 0x00000.bin fota.bin
./mkfw || exit
cd ..

cd ..


# Создаем каталог для билда
mkdir -p builds/$BUILD || exit

# Копируем файлы
cp soft/mkfw/0x00000.bin builds/$BUILD || exit
cp soft/mkfw/fota.bin builds/$BUILD || exit

echo "Build $BUILD done"
