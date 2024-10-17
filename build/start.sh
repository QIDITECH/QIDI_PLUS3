#!/bin/bash
systemctl --no-reload enable armbian-resize-filesystem.service
systemctl start armbian-resize-filesystem.service

if dpkg-query -W -f='${Status}' chrony 2>/dev/null | grep -q "install ok installed"; then
    echo "Removing chrony..."
    apt-get remove -y chrony
fi

if dpkg-query -W -f='${Status}' ntp 2>/dev/null | grep -q "install ok installed"; then
    echo "Removing ntp..."
    apt-get remove -y ntp
fi

# 启用systemd-timesyncd服务
timedatectl set-ntp true
echo "NTP service enabled through systemd-timesyncd."

echo "Start makerbase-client"
time=$(date "+%Y%m%d%H%M%S")
# /root/makerbase-client/build/MakerbaseClient localhost > /root/mksclient/test-$time.log
/root/udp_server &
/root/xindi/build/xindi localhost