#!/bin/bash

#---------------------------------------------------------------------------#
#  IMPORTANT
#  ADD to Crontab and set config.txt and comdline.xtx
#---------------------------------------------------------------------------#

# 0 * * * *       cd /home/sel/updater/ && ./updater &
# @reboot         sleep 30 && /home/sel/firmware/startApp.sh &
# * * * * * wget -O - -q -t 1 http://localhost/check
# */15 * * * * wget -O - -q -t 1 http://localhost/reconnect
# HOME=/home/sel/network_manager
# @reboot python3 -m uvicorn --host 0.0.0.0 --port 80 --log-config log.ini main:app

# #to set a static CPU speed include:
# arm_freq=1200
# arm_freq_min=1200

#sudo nano /boot/cmdline.txt and add isolcpus=1,2
# cmdline="serial0,115200 console=tty1 root=PARTUUID=$partuuid rootfstype=ext4 elevator=deadline fsck.repair=yes rootwait isolcpus=1,2"

#After everythin set install as su
#sudo su
#pip3 install -r /home/sel/network_manager/requirements.txt 

#---------------------------------------------------------------------------#
# wireless card macaddress 
#---------------------------------------------------------------------------#

# MACaddr=$(cat /sys/class/net/wlan0/address | sed s/://g) 
# filename=/home/pi/log_$MACaddr.log
# if [ ! -f $filename ]
# then
#    touch $filename
# fi
# echo "$(date) - initial setup complete" >> $filename

#---------------------------------------------------------------------------#

sudo chpasswd <<<"pi:Sel123_" #default for all participants

#set time to UTC
sudo timedatectl set-timezone UTC;
sudo raspi-config nonint do_wifi_country PT
rfkill unblock all

sudo apt-get update && sudo apt-get upgrade && sudo apt-get autoremove
sudo apt-get install git
sudo apt-get install cmake
sudo apt-get install openssl
sudo apt-get install libssl-dev
sudo apt-get install hostapd
sudo apt-get install python3-pip
sudo apt-get install dnsmasq
sudo apt-get install zip

#enable SPI
sudo modprobe spi-bcm2835;

#required for hotspot
sudo systemctl unmask hostapd;
sudo systemctl enable hostapd;

#install BCM libraries
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
sudo make test;
cd ~

#sudo echo 'LD_LIBRARY_PATH =  >> .profile
sudo echo 'export LD_LIBRARY_PATH=/usr/local/lib' >> .profile

sudo ldconfig

cd /home/pi
#generate ssl certificate
openssl req -x509 -nodes -days 365 -newkey rsa:1024 -keyout mycert.pem -out mycert.pem -subj "/C=PT/ST=Lisbon/L=Lisbon/O=SEL/OU=LivingEnergy/CN=www.smartenergylab.pt"

unzip sel.zip

sudo mkdir /home/sel
sudo mv /home/pi/sel/* /home/sel/
sudo rm -rf bcm* json* *.zip __* setupRpi.sh
pip3 install -r /home/sel/network_manager/requirements.txt 
#sudo mv /home/pi/mycert.pem /home/sel/

sudo reboot

#or
# sudo mv /boot/config.txt /boot/config.txt.bak
# sudo mv /home/pi/static/config.txt /boot/config.txt

# #to add update and app start at reboot to a cronjob
# sudo mv /etc/crontab /etc/crontab.bak
# sudo mv /home/pi/static/crontab /etc/crontab

#---------------------------------------------------------------------------#
# WARNING the folowing steps can only be performed directly on the device
# This should be addressed on the first firmware update/instalation
#---------------------------------------------------------------------------#


#---------------------------------------------------------------------------#
# to set a static CPU affinity adds isolcpus=1,2 at end of line
#---------------------------------------------------------------------------#

# touch /home/pi/static/cmdline.txt #generate static file with device PARTUUID
# partuuid=$(blkid -o value -s "PARTUUID" /dev/mmcblk0p2)
# #echo $partuuid
# cmdline="serial0,115200 console=tty1 root=PARTUUID=$partuuid rootfstype=ext4 elevator=deadline fsck.repair=yes rootwait isolcpus=1,2"
# echo $cmdline >> /home/pi/static/cmdline.txt
# sudo mv /boot/cmdline.txt /boot/cmdline.txt.bak
# sudo mv /home/pi/static/cmdline.txt /boot/cmdline.txt 



#---------------------------------------------------------------------------#
# TODO runs update manager to get the latest firmware version
# First installation should also set the prior commands
#---------------------------------------------------------------------------#
# ./updateManager.sh






