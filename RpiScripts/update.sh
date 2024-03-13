#!/bin/bash

echo "Update Template"

UPDATEFILES=$"/home/sel/updater/tmp"

#sudo chpasswd <<<"pi:Sel123_" #default for all participants

#******************************************************************************
#UPDATE nilm App
#******************************************************************************
DEST=$"/home/sel/firmware"
echo "Updating Nilm 1/2"
sudo rm -rf $DEST/nilm
sudo cp $UPDATEFILES/firmware/nilm $DEST/nilm
echo "Updating Nilm 2/2"
sudo rm -rf $DEST/startApp.sh
sudo cp $UPDATEFILES/firmware/startApp.sh $DEST/startApp.sh

#******************************************************************************
#UPDATE updater App
#******************************************************************************
DEST=$"/home/sel/updater"
echo "Updating Updater 1/1"
sudo rm -rf $DEST/updater/updater
sudo cp $UPDATEFILES/updater/updater $DEST/updater

#******************************************************************************
#UPDATE network_manager (WARNING DO NOT REPLACE network_file.json)
#******************************************************************************
DEST=$"/home/sel/network_manager"
#update network_manage/library.py
echo "Updating Network Manager 1/7"
sudo rm -rf $DEST/library.py
sudo cp $UPDATEFILES/network_manager/library.py $DEST/
#update network_manage/main.py
echo "Updating Network Manager 2/7"
sudo rm -rf $DEST/main.py
sudo cp $UPDATEFILES/network_manager/main.py $DEST/
#update network_manager/network_files/dhcpcd.conf
echo "Updating Network Manager 3/7"
sudo rm -rf $DEST/network_files/dhcpcd.conf
sudo cp $UPDATEFILES/network_manager/network_files/dhcpcd.conf $DEST/network_files/
#update network_manager/static
echo "Updating Network Manager 4/7"
sudo rm -rf $DEST/static/css/*
sudo cp -r $UPDATEFILES/network_manager/static/css/* $DEST/static/css/
echo "Updating Network Manager 5/7"
sudo rm -rf $DEST/static/images/*
sudo cp -r $UPDATEFILES/network_manager/static/images/* $DEST/static/images/
#update network_manager/templates
echo "Updating Network Manager 6/7"
sudo rm -rf $DEST/templates/*
sudo cp $UPDATEFILES/network_manager/templates/* $DEST/templates/
#update network_manager/templates
echo "Updating Network Manager 7/7"
sudo rm -rf $DEST/log.ini
sudo cp $UPDATEFILES/network_manager/log.ini $DEST/

sudo rm -rf /home/sel/updater/tmp /home/sel/updater/*.zip

sudo reboot
