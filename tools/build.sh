#!/bin/bash

dpkg -s cmake >/dev/null 2>&1 || sudo apt install cmake -y || { echo "Error making cmake ready, quit."; exit 1; }
dpkg -s checkinstall >/dev/null 2>&1 || sudo apt install checkinstall -y || { echo "Error making checkinstall ready, quit"; exit 2; }

script=$(readlink -f "$0")
route=$(dirname "$script")

if [ -d "$route/../build" ]; then
    sudo rm -r $route/../build
fi
mkdir -p $route/../build
cd $route/../build
cmake .. -G "Unix Makefiles" || exit 3
make -j 8 || exit 4
sudo checkinstall -D --install=no --default --pkgname=sspdlog --pkgversion=0.2.1 || exit 5

