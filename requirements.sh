#!/usr/bin/env bash

sudo apt-get -y install valgrind
sudo apt-get -y install curl

mkdir -p ~/sources/
cd ~/sources/
curl -L -O https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.tar.bz2
tar --bzip2 -xf boost_1_68_0.tar.bz2

cd boost_1_68_0

./bootstrap.sh --prefix=/usr/local/
./b2
sudo ./b2 install


sudo ldconfig # update shared libs list