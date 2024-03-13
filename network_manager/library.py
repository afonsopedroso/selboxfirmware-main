import os
import traceback
import io
import json
import subprocess


NETWORK_FILENAME = "/home/sel/network_manager/network_files/network_file.json"


class Connector:
    def __init__(self):
        pass

    def reboot(self):
        # Reboot device
        # NOTE: Consider a method other than reboot in later update
        os.system("sudo systemctl reboot")

    def read_network_value_file(self, key=None):
        # Read values from network_file.json
        value = None
        try:
            value = None
            with open(NETWORK_FILENAME) as f:
                value = json.load(f)
            if key is not None:
                value = value[key] if key in value else None
        except Exception as e:
            traceback.print_exc()
        return value

    def write_network_value_file(self, key=None, value=None):
        # Write values to network_file.json
        data = self.read_network_value_file(key=None)
        data[key] = value if data is not None and key is not None and key in data else data
        try:
            with open(NETWORK_FILENAME, "w", encoding="utf-8") as f:
                f.write(json.dumps(data, ensure_ascii=False, indent=4))
        except Exception as e:
            traceback.print_exc()

    def remove_file(self, file=None):
        # cmd command to remove file
        try:
            os.system(f"sudo rm {file}")
        except Exception as e:
            traceback.print_exc()

    def copy_file(self, source=None, target=None):
        # cmd command to copy file
        try:
            os.system(f"sudo cp {source} {target}")
        except Exception as e:
            traceback.print_exc()

    def intialize(self):
        # Initialize network configuration process
        self.remove_file(file="/etc/dhcpcd.conf")
        self.remove_file(file="/etc/dnsmasq.conf")
        self.remove_file(file="/etc/hostapd/hostapd.conf")
        self.remove_file(file="/etc/wpa_supplicant/wpa_supplicant.conf")
        self.copy_file(source="/home/sel/network_manager/network_files/wpa_supplicant.conf.sel", target="/etc/wpa_supplicant/wpa_supplicant.conf")
        os.system("sudo /usr/sbin/rfkill unblock wlan")

    def delete_hotspot(self):
        # Delete access point
        self.intialize()
        self.copy_file(source="/home/sel/network_manager/network_files/dhcpcd.conf", target="/etc/dhcpcd.conf")
        self.copy_file(source="/home/sel/network_manager/network_files/dnsmasq.conf", target="/etc/dnsmasq.conf")

    def set_wifi_params(self, ssid=None, password=None):
        # Set wifi values
        self.write_network_value_file(key="wifi_ssid", value=ssid)
        self.write_network_value_file(key="wifi_password", value=password)

    def connect_to_wifi(self, ssid=None, password=None):
        # Connect to a selected wifi network
        self.write_network_value_file(key="mode", value="initialize connection")
        self.set_wifi_params(ssid=ssid, password=password)
        self.delete_hotspot()
        os.system(f"""sudo sed -i 's/ssid=""/ssid="{ssid}"/' /etc/wpa_supplicant/wpa_supplicant.conf""")
        os.system(f"""sudo sed -i 's/psk=""/psk="{password}"/' /etc/wpa_supplicant/wpa_supplicant.conf""")
        self.reboot()

    def get_mac_address(self):
        # Get device mac address
        mac_address = ""
        try:
            ps = subprocess.Popen(["/usr/sbin/ifconfig", "-a"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            output = subprocess.check_output("grep ether | head -1", stdin=ps.stdout, shell=True)
            output = output.decode("utf-8").strip(" ").split(" ")
            mac_address = output[1].replace(":", "")
        except subprocess.CalledProcessError as e:
            traceback.print_exc()
        return mac_address

    def create_hotspot(self):
        # Create network access point
        self.write_network_value_file(key="mode", value="hotspot")
        self.intialize()
        self.copy_file(source="/home/sel/network_manager/network_files/dhcpcd.conf.sel", target="/etc/dhcpcd.conf")
        server_ip = self.read_network_value_file(key="server_ip")
        os.system(f"""sudo sed -i 's/static ip_address=x/static ip_address={server_ip}\/24/' /etc/dhcpcd.conf""")
        self.copy_file(source="/home/sel/network_manager/network_files/dnsmasq.conf.sel", target="/etc/dnsmasq.conf")
        start_ip = self.read_network_value_file(key="start_ip")
        end_ip = self.read_network_value_file(key="end_ip")
        os.system(f"""sudo sed -i 's/dhcp-range=x/dhcp-range={start_ip},{end_ip},255.255.255.0,24h/' /etc/dnsmasq.conf""")
        mac_address = self.get_mac_address()
        os.system(f"""sudo sed -i 's/address=x/address=\/sel.{mac_address}\/{server_ip}/' /etc/dnsmasq.conf""")
        self.copy_file(source="/home/sel/network_manager/network_files/hostapd.conf.sel", target="/etc/hostapd/hostapd.conf")
        hotspot_ssid = self.read_network_value_file(key="hotspot_ssid")
        os.system(f"""sudo sed -i 's/ssid=x/ssid={hotspot_ssid}/' /etc/hostapd/hostapd.conf""")
        hotspot_password = self.read_network_value_file(key="hotspot_password")
        os.system(f"""sudo sed -i 's/wpa_passphrase=x/wpa_passphrase={hotspot_password}/' /etc/hostapd/hostapd.conf""")
        self.reboot()

    def there_is_active_connection(self):
        # Check is device is connection to a wifi
        try:
            ps = subprocess.Popen(["sudo", "/usr/sbin/iwgetid"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            subprocess.check_output(("grep", "ESSID"), stdin=ps.stdout)
            return True
        except subprocess.CalledProcessError as e:
            traceback.print_exc()
            return False

    def there_is_active_hotspot(self):
        # Check if device has an active access point
        try:
            ps = subprocess.Popen(["sudo", "/usr/sbin/ip", "addr", "show", "dev", "wlan0"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            server_ip = self.read_network_value_file(key="server_ip")
            subprocess.check_output(("grep", f"inet {server_ip}"), stdin=ps.stdout)
            return True
        except subprocess.CalledProcessError as e:
            traceback.print_exc()
            return False

    def get_all_wifi_network(self):
        # Get list of available wifi around
        wifi_list = []
        try:
            ps = subprocess.Popen(["sudo", "/usr/sbin/iwlist", "wlan0", "scan"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            output = subprocess.check_output("grep ESSID", stdin=ps.stdout, shell=True)
            output = output.decode("utf-8").strip(" ").split("\n")
            for line in output:
                network = line.replace(":", "").replace("ESSID", "").replace('"', "").strip(" ")
                if network != "":
                    wifi_list.append(network)
        except subprocess.CalledProcessError as e:
            traceback.print_exc()
        return wifi_list

    def network_check(self):
        # Periodically runs network check every minute
        mode = self.read_network_value_file(key="mode")
        if mode != "hotspot":
            if self.there_is_active_connection() is False:
                if mode != "normal":
                    reconnect = self.read_network_value_file(key="reconnect")
                    if reconnect == 0:
                        self.set_wifi_params(ssid="", password="")
                    self.create_hotspot()
            else:
                if mode != "normal":
                    self.write_network_value_file(key="mode", value="normal")
                    self.write_network_value_file(key="reconnect", value=0)
        else:
            if self.there_is_active_hotspot() is False:
                self.create_hotspot()

    def reconnect_to_wifi(self):
        # Peroidically runs network check every 15 minutes
        mode = self.read_network_value_file(key="mode")
        wifi_ssid = self.read_network_value_file(key="wifi_ssid")
        wifi_password = self.read_network_value_file(key="wifi_password")
        reconnect = self.read_network_value_file(key="reconnect")
        new_reconnect = reconnect + 1
        if (mode == "normal" or reconnect > 3) and self.there_is_active_connection() is False and wifi_ssid != "":
            self.write_network_value_file(key="reconnect", value=new_reconnect)
            if reconnect > 3:
                # After 45 minutes if device is not connected to a network or lost connection then attempt to reconnect
                self.connect_to_wifi(ssid=wifi_ssid, password=wifi_password)
        elif wifi_ssid == "":
            self.create_hotspot()


network_connector = Connector()
