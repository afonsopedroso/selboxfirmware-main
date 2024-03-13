#!/bin/bash

sudo apt-get install git
sudo apt-get install openssl
sudo apt-get install libssl-dev
sudo apt-get install gcc-8
sudo apt-get install cmake
sudo modprobe spi-bcm2835;

cd /home/pi
wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.68.tar.gz
tar xvfz bcm2835-1.68.tar.gz
cd bcm2835-1.68;
./configure;
make;
sudo make check
sudo make install
cd /home/pi

#install json-c library
git clone https://github.com/json-c/json-c.git
mkdir json-c-build;
cd json-c-build;
cmake ../json-c;
sudo make install;
# sudo make test;
cd ~

#sudo echo 'LD_LIBRARY_PATH =  >> .profile
sudo echo 'export LD_LIBRARY_PATH=/usr/local/lib' >> .profile

sudo ldconfig