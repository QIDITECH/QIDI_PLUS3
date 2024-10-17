#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <fstream>
#include <ifaddrs.h>
#include <sstream>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <nlohmann/json.hpp>

#include "../include/app_client.h"
#include "../include/event.h"
#include "../include/mks_log.h"
#include "../include/MakerbaseParseIni.h"
#include "../include/system_setting.h"
#include "../include/server_control.h"

SystemSetting sys_set;

using json = nlohmann::json;

void system_setting_init()
{
    sys_set.model_name = "QIDI@x-plus_3";
    sys_set.soc_version = "V4.2.15";
    sys_set.eth_mac = system_get_mac_address("eth0");
    system_read_server_list();

    // 打印机设置
    // 因与mks_printer.cpp耦合 暂不做移植
    get_mks_babystep();

    // 从mksini读取其他配置
    mksini_load();

    // 系统设置
    sys_set.server = mksini_getstring("app_server", "name", "aws");
    sys_set.ethernet = mksini_getboolean("mks_ethernet", "enable", false); // 网络界面选择的是网线还是wifi true为网线
    sys_set.internet_enabled = mksini_getboolean("app_connection", "method", false); // true为互联网 false为仅局域网 仅局域网模式下应关闭所有互联网应用
    sys_set.language = mksini_getint("system", "language", 0);  // TODO: 疑似已废弃，语言选项似乎已保存在tjc屏幕中

    // App相关
    std::map<std::string, std::string> user_info;
    app_cli.device_code = mksini_getstring("app", "device_code", "");
    app_cli.device_id = mksini_getstring("app", "device_id", "");
    app_cli.subdomain = mksini_getstring("app", "subdomain", "");
    app_cli.last_unbind_done = mksini_getboolean("app", "last_unbind_done", true);
    user_info["avatar"] = mksini_getstring("app", "avatar", "");
    user_info["username"] = mksini_getstring("app", "username", "");
    user_info["user_token"] = mksini_getstring("app", "user_token", "");
    std::string bind_status = mksini_getstring("app", "bind_status", "UNBOUND");
    mksini_free();

    // // 若状态初始化失败，转入UNBOUND状态
    // if (!app_cli.set_bind_status(bind_status, user_info));
    //     app_cli.set_bind_status("UNBOUND");

    // 获取设备码
    // 1. 若设备码不符合格式，且处于非局域网模式，则尝试获取设备码 间隔5秒
    // 2. 获取到设备码后若为非局域网模式，进行QIDILink-client服务检查
    // TODO_UI：在切换为非局域网模式时再次调用该方法
    // app_cli.start_update_device_thread();

    // // 如果上次解绑未完成，启动线程完成解绑请求，间隔1秒
    // if (app_cli.last_unbind_done)
    //     app_cli.start_unbind_thread();

    // if (app_cli.get_bind_status() == "BOUND")
    //     app_cli.start_sync_status_thread();

}

void system_read_server_list()
{
    std::ifstream file(APP_SERVER_LIST_PATH);
    if (!file.is_open()) {
        MKSLOG_RED("[Error] Could not open the file: %s", APP_SERVER_LIST_PATH);
        return;
    }

    json servers;
    file >> servers;
    file.close();

    // 清空当前的 server_list
    sys_set.server_list.clear();

    for (const auto& server : servers["servers"]) 
    {
        sys_set.server_list.push_back(server["name"]);
    }
}

std::string system_get_mac_address(const std::string& interface) 
{
    int sockfd;
    struct ifreq ifr;
    char mac_address[18];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) 
    {
        MKSLOG_RED("[Error] Failed to create socket for interface: %s", interface);
        return "";
    }

    strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == -1) 
    {
        MKSLOG_RED("[Error] Failed to get MAC address for interface: %s", interface.c_str());
        close(sockfd);
        return "";
    }

    close(sockfd);

    unsigned char *mac = reinterpret_cast<unsigned char*>(ifr.ifr_hwaddr.sa_data);
    snprintf(mac_address, sizeof(mac_address), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    std::string mac_address_str(mac_address);
    MKSLOG("MAC Address for %s: %s", interface.c_str(), mac_address_str.c_str());

    return mac_address_str;
}

static std::string get_ip_address(const std::string& interface_name) 
{
    struct ifaddrs *ifaddr, *ifa;
    int family;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) 
    {
        MKSLOG_RED("[Error] getifaddrs %s failed", interface_name);
        return "";
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) 
    {
        if (ifa->ifa_addr == nullptr) 
            continue;

        family = ifa->ifa_addr->sa_family;

        if (ifa->ifa_name == interface_name && family == AF_INET) 
        {
            if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST) == 0) 
            {
                freeifaddrs(ifaddr);
                return std::string(host);
            }
        }
    }

    freeifaddrs(ifaddr);
    return "";
}

std::string system_get_local_ip()
{
    std::string local_ip;

    // 如果界面选择网线 优先查找eth0 反之优先wlan0 都没有ip时返回空字符串
    if (sys_set.ethernet) 
    {
        local_ip = get_ip_address("eth0");
        if (local_ip.empty())
            local_ip = get_ip_address("wlan0");
    } 
    else 
    {
        local_ip = get_ip_address("wlan0");
        if (local_ip.empty())
            local_ip = get_ip_address("eth0");
    }

    if (local_ip.empty()) 
        MKSLOG_RED("[Error] Failed to obtain IP address");

    return local_ip;
}