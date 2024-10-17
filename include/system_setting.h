#ifndef SYSTEM_SETTING_H
#define SYSTEM_SETTING_H

#include <string>

// Klipper Moonraker
#define KLIPPER_LOG_DIR "/home/mks/klipper_config"
#define GCODES_DIR "/home/mks/gcode_files"

// Makerbase-client

// 全局设置
struct SystemSetting
{
    bool ethernet;
    bool internet_enabled;
    std::string eth_mac;
    std::string language;
    std::string model_name;
    std::string server;
    std::string soc_version;
    std::vector<std::string> server_list;
};

extern SystemSetting sys_set;

void system_setting_init();
void system_read_server_list();
std::string system_get_mac_address(const std::string& interface);
std::string system_get_local_ip();

#endif // SYSTEM_SETTING_H
