#ifndef __app_client_h
#define __app_client_h

// 该模块用于管理QIDILink APP相关状态 FRP服务 发送请求到QIDILink服务器等

#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <thread>

#define APP_UPDATE_DEVICE_URL_ALIYUN "https://api2.qidi3dprinter.com/qidi/common/updateDeviceWithSn"
#define CHECK_STATUS_URL_ALIYUN "https://api2.qidi3dprinter.com/qidi/common/deviceCheckStatusWithSn"
#define UNBIND_DEVICE_URL_ALIYUN "https://api2.qidi3dprinter.com/qidi/user/unBindDevice"

#define APP_UPDATE_DEVICE_URL_AWS "https://api.qidi3dprinter.com/qidi/common/updateDeviceWithSn"
#define CHECK_STATUS_URL_AWS "https://api.qidi3dprinter.com/qidi/common/deviceCheckStatusWithSn"
#define UNBIND_DEVICE_URL_AWS "https://api.qidi3dprinter.com/qidi/user/unBindDevice"

#define APP_CLI_WRONG_STATUS 3001
#define APP_CLI_CHANGED_STATUS 3002
#define APP_QRCODE_PATH "/root/QIDILink-client/qrcode.png"

// APP
#define APP_CLIENT_DIR "/root/QIDILink-client/"
#define APP_CLIENT_PATH "/root/QIDILink-client/frpc"
#define APP_FRPC_CONFIG_PATH "/root/QIDILink-client/frpc.json"
#define APP_CLIENT_SERVICE_PATH "/etc/systemd/system/QIDILink-client.service"
#define APP_SERVER_LIST_PATH "/root/QIDILink-client/servers.json"
#define APP_UPDATE_DEVICE_URL "https://api.qidi3dprinter.com/qidi/common/updateDevice"

//B
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include "event.h"

class AppClient
{
public:
    AppClient();

    // 管理绑定状态
    std::string get_bind_status();
    bool set_bind_status(std::string status, std::map<std::string, std::string> user_info = {});

    // FRP 服务相关
    void check_client_service();
    void init_frpc();
    void write_frpc_json();
    void write_service_file();

    // 请求相关
    int update_device(std::string selected_server_name);
    int update_device();
    int unbind_device();
    int refresh_bind_code(std::string& bind_code);
    int sync_status(bool refresh_user = false);

    void start_update_device_thread();
    void start_unbind_thread();
    void start_sync_status_thread();

    //B
    std::string execute_command(const std::string &cmd);
    
    // TODO: 考虑将用户相关的装入struct
    bool last_unbind_done; // TODO: 考虑改成atomic
    std::string bind_code;
    std::string device_code;
    std::string device_id;
    std::string subdomain;
    std::string avatar;
    std::string username;
    std::string user_token;

private:
    std::mutex mutex_; // 设置状态锁 set_bind_status
    std::string bind_status; // 写枚举类型涉及到字符数字转换使用过于繁杂 采用私有的string
};

extern AppClient app_cli;

//B
void printKeysAndValues(const json& j);
void *open_bind_thread(void *arg);
#endif