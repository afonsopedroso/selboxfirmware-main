## Guide to Run Project
NILM NETWORK MANAGER


## Install the Following Requirements
```
sudo apt install hostapd
sudo systemctl unmask hostapd
sudo systemctl enable hostapd
sudo apt install dnsmasq
pip install -r requirement.txt
```

### Crontab Settings
Put the entries below under root user in crontab
```
* * * * * wget -O - -q -t 1 http://localhost/check
*/15 * * * * wget -O - -q -t 1 http://localhost/reconnect
HOME=/home/sel/network_manager
@reboot python3 -m uvicorn --host 0.0.0.0 --port 80 --log-config log.ini main:app
```
