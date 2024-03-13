
#!/bin/bash

#add command ctrontab
#@reboot /startApp.sh 

appfolder=firmware
app=nilm

rm -rf /home/sel/$appfolder/datalog.json

sudo nice -n -20 /home/sel/$appfolder/$app &

sleep 10

#initial configuration fails (to be updated in next version)
sudo pkill -15 $app 

sleep 10
#check for log file exist or not
MACaddr=$(cat /sys/class/net/wlan0/address | sed s/://g) 
#echo $SN
filename=/home/sel/$appfolder/log_$MACaddr.log
#echo $filename
if [ ! -f $filename ]
then
    touch $filename
fi
echo "$(date) - start application" >> $filename

#stop unwanted services after reboot
sudo systemctl stop bluetooth.service
sudo systemctl stop hciuart.service
sudo systemctl stop alsa-state.service


#start app with highest priority
sudo nice -n -20 /home/sel/$appfolder/$app 2>/home/sel/$appfolder/log_$MACaddr.log &