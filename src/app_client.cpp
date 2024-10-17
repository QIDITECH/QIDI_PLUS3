#include <unistd.h>
#include <regex>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstring>
#include <net/if.h>
#include <sys/ioctl.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

// 图片转化库
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../include/app_client.h"
#include "../include/CurlWrapper.h"
#include "../include/event.h"
#include "../include/mks_log.h"
#include "../include/MakerbaseParseIni.h"

#include "../include/send_msg.h"
#include "../include/server_control.h"
#include "../include/system_setting.h"
#include "../include/ui.h"
#include "../include/utils.h"
#include "../include/Http.hpp"
#include <boost/algorithm/string.hpp>
#include<boost/format.hpp>

// // Makerbase-client
// #define DEV_INFO_PATH "/dev_info.txt"

extern int tty_fd;

static bool unbind_thread_running;
static bool sync_thread_running;

extern bool is_in_qr_page;
extern bool is_in_user_page;
std::string user_token;
std::string bind_code = "";
std::string get_user_token = "";
std::string get_username = "";
std::string get_avatar = ""; 

std::vector<std::string> lang={"zh", "ru", "en", "ja", "fr", "de", "it", "es", "ko", "pr", "ar", "tr", "he"};
int lang_index = 0;

/**
 * 1. [设备码] 通过update_device方法更新
 * 2. [打印机端登出] 将bind_status置为 unbound, 单开线程unbind_thread持续发送解绑请求直到成功
 * 3. [frp服务] 在bind_status变更时，状态为BOUND需启动QIDI-Link服务(非局域网模式下 
 *              但一般非局域网模式下也不会转入BOUND状态)，其他情况下关闭
 * 4. [仅局域网模式] 创建的请求线程中应在每次发送请求前检查状态，如果仅局域网应关闭当前请求线程
 *                  同时避免在仅局域网的模式下手动发送请求
 * 
 * 开机事项：
 * 1. 启动server_control中的udp_server
 * 2. 若last_unbind_done 为false 开线程执行解绑
 * 3. 若设备码为空且非局域网模式，开线程获取设备码
 * 
 * curl status code说明
 * #define SUCCESS 0 请求成功
 * #define CURL_TIMEOUT 1001 请求超时, 请检查网络连接
 * #define CURL_INITIALIZATION_FAIL 1002 初始化失败 (内存不足 或者 库未安装,冲突等)
 * #define CURL_FILE_OPEN_FAIL 1003 文件打开失败 (指定文件所在文件夹不存在 或者 程序权限不足 应检查自身程序)
 * #define CURL_RESOURCE_NOT_FOUND 1004 资源不存在
 * #define CURL_BAD_REQUEST 1005 请求错误 检查自身程序
 * #define CURL_SERVER_ERROR 1006 服务端错误
 * #define CURL_SSL_VERIFICATION_FAIL 1008 ssl证书验证失败 检查本机时间是否同步
 * #define CURL_DNS_RESOLVE_FAIL 1009 无法解析域名 (大概率没有互联网连接, 小概率本地url拼接错误或者域名未注册解析)
 * #define CURL_DISK_FULL 1010 磁盘空间已满
 * 是否解除当前打印机与QIDILink账户的绑定?
 * 
 * SUCCESS 解绑成功
 * -1 解绑失败，打印机未被绑定
 * CURL_TIMEOUT 请求超时, 请检查网络连接
 * CURL_INITIALIZATION_FAIL 连接初始化失败
 * CURL_FILE_OPEN_FAIL 1003 文件打开失败
 * CURL_SSL_VERIFICATION_FAIL SSL证书验证失败，请检查本机时间是否同步
 * CURL_RESOURCE_NOT_FOUND 请求的资源不存在
 * CURL_BAD_REQUEST 错误的请求
 * CURL_SERVER_ERROR 1006 服务端错误
 * CURL_DNS_RESOLVE_FAIL DNS解析失败，请检查网络连接
 * CURL_DISK_FULL 1010 磁盘空间已满
 * 以上说明仅供参考, 具体展示给用户的信息需要调整
 */

AppClient app_cli;

std::regex device_code_r("^[a-zA-Z0-9]{8}$");
std::regex bind_code_r("^[A-Za-z0-9]{6}$");

AppClient::AppClient() {}

std::string AppClient::get_bind_status()
{
    return bind_status;
}

//B
std::string  AppClient::execute_command(const std::string &cmd) 
{
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) 
    {
        result += buffer.data();
    }
    return result;
}

bool AppClient::set_bind_status(std::string status, std::map<std::string, std::string> user_info) 
{
    std::lock_guard<std::mutex> lock(mutex_);
    ServerControl& server_control = ServerControl::getInstance();

    if (status == "UNBOUND")
    {
        server_control.stop_http_server();
        mksini_update("app", "bind_status", "UNBOUND");
        bind_status = "UNBOUND";
        return true;
    }

    if (status == "BOUND")
    {
        if (check_utf8_strlen(user_info["username"], 30) && // 校验用户名
            !user_info["token"].empty() &&                  // 校验用户token
            !user_info["id"].empty()                        // 校验设备id
            )
        {
            server_control.stop_http_server();
            avatar = user_info["avatar"]; // 头像图床地址为空时使用默认QIDI logo
            username = user_info["username"];
            user_token = user_info["token"];
            device_id = user_info["id"];
            system("systemctl start QIDILink-client");
            last_unbind_done = true;
            mksini_update("app", "bind_status", "BOUND");
            bind_status = "BOUND";
            start_sync_status_thread();
        }
        else 
        {
            MKSLOG_RED("Invalid user info, refuse to bind.");
            return false;
        }
        return true;
    }

    if (status == "PENDING")
    {
        server_control.start_http_server();
        mksini_update("app", "bind_status", "PENDING");
        bind_status = "PENDING";
        return true;
    }

    MKSLOG_YELLOW("Invalid status");
    return false;
}

void AppClient::write_frpc_json()
{
    std::ofstream config_file(APP_FRPC_CONFIG_PATH);
    if (config_file.is_open()) 
    {
        config_file << "{\n"
                    << "    \"serverAddr\": \"www." << sys_set.server << ".qidi3dprinter.com\",\n"
                    << "    \"serverPort\": 7080,\n"
                    << "    \"auth\": {\n"
                    << "        \"method\": \"token\",\n"
                    << "        \"token\": \"qidi tech\"\n"
                    << "    },\n"
                    << "    \"log\": {\n"
                    << "        \"to\": \"" << APP_CLIENT_DIR << "QIDILink-client.log\",\n"
                    << "        \"level\": \"info\",\n"
                    << "        \"maxDays\": 7\n"
                    << "    },\n"
                    << "    \"proxies\": [\n"
                    << "        {\n"
                    << "            \"name\": \"" << sys_set.model_name << "_" << sanitize_mac_address(sys_set.eth_mac) << "\",\n"
                    << "            \"type\": \"http\",\n"
                    << "            \"localPort\": 80,\n"
                    << "            \"transport\": {\n"
                    << "                \"bandwidthLimit\": \"100KB\",\n"
                    << "                \"useEncryption\": true\n"
                    << "            },\n"
                    << "            \"subdomain\": \"" << subdomain << "\"\n"
                    << "        }\n"
                    << "    ]\n"
                    << "}\n";
        config_file.close();
    }
    else 
        MKSLOG_RED("Failed to open file: %s", APP_FRPC_CONFIG_PATH);
}

// Write to the QIDILink-client.service file
void AppClient::write_service_file()
{
    std::ofstream service_file(APP_CLIENT_SERVICE_PATH);
    if (service_file.is_open()) 
    {
        service_file << "[Unit]\n"
                     << "Description=QIDILink Client Service\n"
                     << "After=network.target\n"
                     << "StartLimitIntervalSec=0\n\n"
                     << "[Service]\n"
                     << "Type=simple\n"
                     << "ExecStart=" << APP_CLIENT_PATH << " -c " << APP_FRPC_CONFIG_PATH << "\n"
                     << "Restart=on-failure\n"
                     << "RestartSec=10\n"
                     << "StartLimitBurst=0\n\n"
                     << "[Install]\n"
                     << "WantedBy=multi-user.target\n";
        service_file.close();
    } 
    else 
        MKSLOG_RED("Failed to open file: %s", APP_CLIENT_SERVICE_PATH);
}

// QIDILink-client.service initialization
void AppClient::init_frpc()
{
    MKSLOG("Initializing QIDILink.service");

    if (access(APP_CLIENT_PATH, F_OK) == 0)
    {
        system("chmod +x " APP_CLIENT_PATH);

        write_frpc_json();
        write_service_file();

        system("systemctl daemon-reload");
        system("systemctl enable --now QIDILink-client.service");

        MKSLOG("QIDILink-client service has been set up");
    } 
    else 
        MKSLOG_RED("FRPC does not exist: %s", APP_CLIENT_PATH);
}

// 请求服务器 获取提供给设备的frp地址以及设备码 一般在选择服务器和刷新设备码时调用
// 为了兼容服务器选择时的更新请求 添加服务器参数
int AppClient::update_device(std::string selected_server_name)
{
    MKSLOG("Sending update device request...");
    std::string read_buffer;
    CurlWrapper curl_wrapper;
    //B
    std::string SN = execute_command("cat /proc/device-tree/serial-number");
    std::cout << "SN = "<< SN << std::endl;
    std::vector<std::string> tokens;
    MKSLOG_YELLOW("initial model name: %s", sys_set.model_name.c_str());
    boost::split(tokens, sys_set.model_name, boost::is_any_of("@"));
    std::string post_fields;
    if(tokens.size() == 2){
        post_fields = "device_name=" + tokens[0] +"&machine_type=" + tokens[1] + "&mac_address=" + sys_set.eth_mac + "&local_ip=" + system_get_local_ip() + "&server=" + selected_server_name + "&sn="+SN;
    }else{
        post_fields = "device_name=" + sys_set.model_name + "&mac_address=" + sys_set.eth_mac + "&local_ip=" + system_get_local_ip()  + "&server=" + selected_server_name + "&sn="+SN;
    }

    MKSLOG("post_fields: %s", post_fields.c_str());

    int curl_status = curl_wrapper.sendPostRequest(sys_set.server == "aws" ? APP_UPDATE_DEVICE_URL_AWS : APP_UPDATE_DEVICE_URL_ALIYUN, post_fields, read_buffer);

    MKSLOG("read_buffer: %s", read_buffer.c_str());
    if (curl_status == SUCCESS)
    {
        try
        {
            json response = json::parse(read_buffer);
            int status = response["status"];
            /*
                The success or failure of the request also returns 0, 
                    which should be judged based on the message
            */
            if (status == 0) 
            {
                std::string url = response["data"]["url"];
                subdomain = url.substr(0, url.find('.'));
                device_code = response["data"]["device_code"];
                MKSLOG_GREEN("update_device: %s",device_code.c_str());

                if (!std::regex_match(device_code, device_code_r))
                {
                    MKSLOG_RED("Received device code does not match the pattern!");
                    return CURL_GENERAL_EXCEPTION;
                }

                mksini_load();
                mksini_set("app", "device_code", device_code);
                mksini_set("app", "subdomain", subdomain);
                mksini_save();
                mksini_free();

                system("systemctl stop QIDILink-client");
                write_frpc_json();

                // B
                mksini_load();
                // Initialize global variable user_token
                user_token = mksini_getstring("app", "token", "");
                mksini_free();
                bool first_time_update = false;
                if (user_token != ""){
                    MKSLOG_YELLOW("Start QIDILink Error and update_device is None");
                } 
                MKSLOG_RED("Reload QIDILink-client.service");
                system("systemctl start QIDILink-client");
                return SUCCESS;
            }
            else 
            {
                MKSLOG_YELLOW("Failed request: status=%d message=%s", status, response["message"].get<std::string>().c_str());
                return CURL_BAD_REQUEST;
            }
        }
        catch (const std::exception &ex)
        {
            MKSLOG_RED("Exception occurred: %s", ex.what());
            return CURL_GENERAL_EXCEPTION;
        }

    } else 
    {
        MKSLOG_YELLOW("Failed to send update device request: %d", curl_status);
        MKSLOG_BLUE("update_device 这里的刷新失败了。");
        return curl_status;
    }

    return CURL_UNKOWN_ERROR; // 仅占位用 防止编译器警告
}

int AppClient::update_device()
{
    return update_device(sys_set.server);
}

// 检查服务状态并配置服务
void AppClient::check_client_service()
{
    std::string command = "systemctl is-enabled QIDILink-client.service > /dev/null 2>&1";
    int ret = system(command.c_str());
    
    if (ret == 0)
        MKSLOG("QIDILink-client.service already enabled");
    else 
    {
        // QIDILink-client.service is not enabled or does not exist
        command = "systemctl status QIDILink-client.service > /dev/null 2>&1";
        ret = system(command.c_str());
        
        if (ret != 0)
        {
            // QIDILink-client.service does not exist，call init_frpc
            MKSLOG("QIDILink-client.service does not exist, setting up...");
            init_frpc();
        } else
        {
            // QIDILink-client.service exists but is not enable service
            MKSLOG("QIDILink-client.service exists but disabled, enabling...");
            system("systemctl enable --now QIDILink-client.service");
        }
    }
}

/*
    When the QR code interface fails to refresh and generate the QR code every 90 seconds,
        the application image or text display is incorrect (network connection failure)
*/
int AppClient::refresh_bind_code(std::string& bind_code)
{
    MKSLOG("Sending refresh bind code request...");
    if (bind_status != "PENDING")
    {
        MKSLOG_YELLOW("Tring to refresh bind code while bind status is not pending");
        return APP_CLI_WRONG_STATUS;
    }

    std::string read_buffer;
    CurlWrapper curl_wrapper;

    std::string post_fields = "mac_address=" + sys_set.eth_mac + 
                            "&status=pending" +
                            "&refresh_user=false";

    int curl_status = curl_wrapper.sendPostRequest(CHECK_STATUS_URL_AWS, post_fields, read_buffer);

    if (curl_status == SUCCESS)
    {
        try
        {
            json response = json::parse(read_buffer);
            int status = response["status"];
            /*
                The success or failure of the request also returns 0, 
                    which should be judged based on the message
            */
            if (status == 0)
            {
                // if (status==pending) return <http 200> or <status 0>
                std::string res_bind_code = response["data"]["bind_code"];

                if (!std::regex_match(res_bind_code, bind_code_r))
                {
                    MKSLOG_RED("Received bind code does not match the pattern!");
                    return CURL_GENERAL_EXCEPTION;
                }
                bind_code = res_bind_code;
                MKSLOG("Bind code refreshed successfully: %s", bind_code.c_str());
                return SUCCESS;
            }
            else
            {
                MKSLOG_YELLOW("Failed request: status=%d message=%s", status, response["message"].get<std::string>().c_str());
                return CURL_BAD_REQUEST;
            }
        }
        catch (const std::exception &ex)
        {
            MKSLOG_RED("Exception occurred: %s", ex.what());
            return CURL_GENERAL_EXCEPTION;
        }
    }
    else
    {
        MKSLOG_YELLOW("Failed to send update device request: %d", curl_status);
        MKSLOG_BLUE("refresh_bind_code 这里的刷新失败了。");
        return curl_status;
    }
    
    return CURL_UNKOWN_ERROR; // 仅占位用 防止编译器警告
}

int AppClient::unbind_device()
{
    MKSLOG("Sending unbind device request...");

    // 正常打印机端解绑是先将自身状态置为UNBOUND再去发送请求直到成功
    // 此处判断防止出现后台尝试解绑时，用户又进行了绑定
    if (bind_status == "BOUND")
    {
        MKSLOG_YELLOW("Tring to unbind device while bound");
        return APP_CLI_WRONG_STATUS;
    }

    // 解绑需要之前储存的用户凭证
    if (app_cli.device_id.empty() || app_cli.user_token.empty())
    {
        MKSLOG_YELLOW("Tring to unbind device while no user crediential exists");
        return APP_CLI_WRONG_STATUS;
    }

    std::string read_buffer;
    CurlWrapper curl_wrapper;

    std::string token = "Bearer" + app_cli.user_token;
    std::map<std::string, std::string> headers = {
        {"Authorization", token.c_str()}
    };
    curl_wrapper.setRequestHeaders(headers);

    std::string post_fields = app_cli.device_id;

    int curl_status = curl_wrapper.sendPostRequest(UNBIND_DEVICE_URL_AWS, post_fields, read_buffer);

    if (curl_status == SUCCESS)
    {
        if (bind_status == "BOUND")
        {
            MKSLOG_YELLOW("Status changed while waiting for unbind response");
            return APP_CLI_WRONG_STATUS;
        }

        try
        {
            json response = json::parse(read_buffer);
            int status = response["status"];
            /*
                The success or failure of the request also returns 0, 
                    which should be judged based on the message
            */
            if (status == 0)
            {
                std::string message = response["message"];
                if (message == "设备解绑成功")
                {
                    last_unbind_done = true;
                    return SUCCESS;
                }
                else
                    return CURL_BAD_REQUEST; // 不好区分是服务端出错还是请求有错，暂且用该状态码
            }
            else
            {
                MKSLOG_YELLOW("Failed request: status=%d message=%s", status, response["message"].get<std::string>().c_str());
                return CURL_BAD_REQUEST;
            }
        }
        catch (const std::exception &ex)
        {
            MKSLOG_RED("Exception occurred: %s", ex.what());
            return CURL_GENERAL_EXCEPTION;
        }
    }
    else
    {
        MKSLOG_YELLOW("Failed to send unbind request: %d", curl_status);
        return curl_status;
    }

    return CURL_UNKOWN_ERROR; // 仅占位用 防止编译器警告
}

// 仅在绑定状态下同步状态使用
// 1. 主动检查是否被解绑，以防服务器向打印机发送解绑请求失败
// 2. 在用户界面更新用户信息
// TODO_UI：进入二维码界面时若为绑定状态 主动发送一次并更新用户信息
int AppClient::sync_status(bool refresh_user)
{
    MKSLOG("Trying to sync bind status...");

    if (bind_status != "BOUND")
    {
        MKSLOG_YELLOW("Trying to sync status while not bound");
        return APP_CLI_WRONG_STATUS;
    }

    std::string read_buffer;
    CurlWrapper curl_wrapper;

    /**
     * /common/deviceCheckStatus接口服务端逻辑
     * 先检测mac_address是否存在对应user 没有返回status -1
     * 如果status为pending返回新的bind_code
     * 
     * 如果不为pending 则返回服务器当前status (这里打印机需要做出检测，如果为bound则一切正常，如果为unbound 则表示本地未同步解绑状态，需要进行解绑)
     * 同时检查refresh_user 是否为true 如果是则返回新的用户信息
     */

    std::string post_fields = "mac_address=" + sys_set.eth_mac + 
                            "&status=bound" +
                            "&refresh_user=" + (refresh_user ? "true" : "false");

    int curl_status = curl_wrapper.sendPostRequest(CHECK_STATUS_URL_AWS, post_fields, read_buffer);

    if (curl_status == SUCCESS)
    {
        try
        {
            json response = json::parse(read_buffer);
            int status = response["status"];
            if (status == 0) // 请求成功 ? 实际上不成功返回的也是0 要根据message判断
            {
                std::string res_status = response["data"]["status"];
                std::transform(res_status.begin(), res_status.end(), res_status.begin(), ::toupper);
                if (res_status == "UNBOUND")
                {
                    // 若本地为已绑定 服务端为未绑定则解绑
                    set_bind_status(res_status);
                    return APP_CLI_CHANGED_STATUS;
                }
                if (res_status == "BOUND")
                {
                    if (refresh_user)
                    {
                        device_id = response["data"]["id"];
                        username = response["data"]["username"];
                        user_token = response["data"]["token"];
                        avatar = response["data"]["avatar"];
                    }
                    return SUCCESS; // 状态无变化
                }
            }
            else
            {
                MKSLOG_YELLOW("Failed request: status=%d message=%s", status, response["message"].get<std::string>().c_str());
                return CURL_BAD_REQUEST;
            }
        }
        catch (const std::exception &ex)
        {
            MKSLOG_RED("Exception occurred: %s", ex.what());
            return CURL_GENERAL_EXCEPTION;
        }
    }
    else
    {
        MKSLOG_YELLOW("Failed to send unbind request: %d", curl_status);
        return curl_status;
    }

    return CURL_UNKOWN_ERROR;
}

// 开机在没有设备码且非局域网的情况下进行初始化 每隔10秒进行一次请求直到成功获取设备码
// TODO_UI: 在切换为非局域网时进行调用该线程
void AppClient::start_update_device_thread()
{
    std::thread([this]()
    {
        MKSLOG("Starting background update device thread");

        int retries = 0;

        while (!std::regex_match(device_code, device_code_r) && sys_set.internet_enabled) 
        {
            retries += 1;
            MKSLOG("Trying to init device code, times: %d", retries);
            if (update_device() == SUCCESS)
            {
                break;
            }
            else
                std::this_thread::sleep_for(std::chrono::seconds(5));
        }

        // 在非局域网模式检查服务
        if (sys_set.internet_enabled)
            check_client_service();
        else
            MKSLOG("LAN only mode, skip QIDILink-client check.");

        MKSLOG("Background update device thread exit");
    }).detach();
}

// 持续发送解绑请求直到收到服务器的确认消息
// TODO_UI: 1.页面点击登出后将last_unbind_done置为false并且执行一次
//          2.开机时执行一次
//          3.切换为非局域网模式时执行一次
// 
void AppClient::start_unbind_thread()
{
    std::thread([this]() 
    {
        if (unbind_thread_running)
        {
            MKSLOG_YELLOW("Another unbind thread running");
            return;
        }

        MKSLOG("Starting background unbind thread");

        int retries = 0;
        unbind_thread_running = true;

        while (!last_unbind_done && sys_set.internet_enabled) 
        {
            retries += 1;
            MKSLOG("Trying to sync unbind status with server, times: %d", retries);

            int status = unbind_device();

            if (status == SUCCESS) 
            {
                MKSLOG("Successfully unbind.");
                break;
            }
            else if(status == APP_CLI_WRONG_STATUS)
                break;
            else
                std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        MKSLOG("Background unbind thread exit");
        unbind_thread_running = false;
    }).detach();
}

void AppClient::start_sync_status_thread()
{
    std::thread([this]()
    {
        if (sync_thread_running)
        {
            MKSLOG_YELLOW("Another sync thread running");
            return;
        }

        MKSLOG("Starting background sync thread");

        sync_thread_running = true;
        int sync_count = 0;

        while (app_cli.get_bind_status() == "BOUND") 
        {
            MKSLOG("Trying to sync unbind status with server, times: %d", ++sync_count);

            int status = app_cli.sync_status();

            if (status == APP_CLI_CHANGED_STATUS)
            {
                sync_thread_running = false;
                break;
            }
            else
                std::this_thread::sleep_for(std::chrono::seconds(30));
        }
    }).detach();
}

//B 遍历json输入其中所有的key值和对应的value
void printKeysAndValues(const json& j)
{
    for (auto it = j.begin(); it != j.end(); ++it)
    {
        if (it->is_object())
        {
            printKeysAndValues(*it);
        }
        else
        {
            std::cout << "Key: " << it.key() << ", Value: " << *it << std::endl;
        }
    }
}

//B 绑定线程
void *open_bind_thread(void *arg) 
{
    std::thread([]() 
    {
        // 更新系统时间
        system("sudo apt remove -y chrony ntp");
        system("sudo timedatectl set-ntp true");

        is_in_qr_page = false;
        is_in_user_page = false;
        lang_index = 0;
        MKSLOG("Starting bind thread");

        mksini_load();
        // Initialize global variable user_token
        user_token = mksini_getstring("app", "token", "");
        mksini_free();
        bool first_time_update = false;
        
        if (sys_set.internet_enabled){
            app_cli.init_frpc();
        }
        else{
            MKSLOG("LAN only mode, skip QIDILink-client check.");
        }
        
        if(user_token == "" && !sys_set.internet_enabled)
        {
            MKSLOG_YELLOW("open_bind_thread: user_token is trying to get");
            system("systemctl stop QIDILink-client"); 
        }

        std::string mac_address = system_get_mac_address("eth0");
        std::string SN =  app_cli.execute_command("cat /proc/device-tree/serial-number");
        // std::cout << "SN = "<< SN << std::endl;
        while (true) 
        {
            /*
            打印机自身的状态由token值决定，打印机有三种状态有token值为绑定状态
                1.1 没token值的时候为解绑状态以及is_in_qr_page=true时为匹配状态
                1.2 接口不能传bool值这里用这种方式替换为string
            */
            std::string status = (user_token == "") ? "unbound" : "bound"; 
            std::string in_page = is_in_qr_page || is_in_user_page ? "true" : "false"; 

            if(!first_time_update && app_cli.update_device() == SUCCESS)
            {
                    first_time_update = true;
            }

            auto http = Http::post(std::move(sys_set.server == "aws" ? CHECK_STATUS_URL_AWS : CHECK_STATUS_URL_ALIYUN));
            if ((user_token != "" || is_in_qr_page || is_in_user_page) && sys_set.internet_enabled)
            {
                http.timeout_connect(5)
                    .header("Authorization", "Bearer ")
                    .form_add("sn", SN)
                    .form_add("status", status)
                    .form_add("in_page", in_page)
                    .on_error([&](std::string body, std::string error, unsigned status) {
                        std::cout << boost::format("Bind Error : %1%, HTTP %2%, body: `%3%`") % error % status % body << std::endl;
                    })
                    .on_complete([&](std::string body, unsigned) {
                        std::cout << boost::format("Bind : %1%") % body << std::endl;  
                        nlohmann::json response = nlohmann::json::parse(body);
                        try {
                            int status = response["status"];
                            /*
                                判断返回信息中是否包含token
                                    if(token != "") status = "bound";
                            */
                            if(response["data"].find("token") != response["data"].end()){

                                    system("systemctl restart QIDILink-client");
                                    std::string get_user_token = response["data"]["token"];
                                    std::string get_username = response["data"]["username"];
                                    std::string get_avatar = response["data"]["avatar"] != nullptr ? response["data"]["avatar"] : "";
                                    std::cout << "receive token = "<< get_user_token << std::endl;
                                    std::cout << "receive username = "<< get_username << std::endl;
                                    std::cout << "receive avatar = "<< get_avatar << std::endl;
                                    std::cout << "status changed"<< std::endl;
                                    user_token = get_user_token;
                                    mksini_load();
                                    mksini_set("app", "token", user_token);
                                    mksini_set("app", "username", get_username);
                                    mksini_set("app", "avatar", get_avatar);
                                    mksini_save();
                                    mksini_free();
                                    page_to(TJC_PAGE_QIDI_LINK_LOGIN_SUCCESS);
                                    send_cmd_txt(tty_fd,"t1", get_username);
                                    login_successed_page();
                                    bind_code = "";
                                }else if(response["data"].find("bind_code") != response["data"].end()){
                                    std::string rev_bind_code = response["data"]["bind_code"];
                                    std::cout << "receive bind_code = "<< rev_bind_code << std::endl;
                                    if(bind_code != rev_bind_code){
                                        bind_code = rev_bind_code;
                                        std::string data = "https://download_" + sys_set.server +
                                        ".qidi3dprinter.com/#/?" +
                                        "lang=" + lang[lang_index] +
                                        "&model=" + sys_set.model_name + 
                                        "&mac=" + sys_set.eth_mac + 
                                        "&server=" + sys_set.server + 
                                        "&ip=" + system_get_local_ip() + 
                                        "&code=" + bind_code +
                                        "&SN="+ SN;
                                        generate_qrcode(data, APP_QRCODE_PATH, 368);
                                        MKSLOG("QR code saved to %s", APP_QRCODE_PATH);
                                        go_to_showqr();
                                    }
                                }else if(response["data"].find("status") != response["data"].end()){
                                    std::string rev_status = response["data"]["status"];
                                    std::cout << "receive status = "<< rev_status << std::endl;
                                    if(!is_in_qr_page && !is_in_user_page){
                                        if(user_token != "" && rev_status == "unbound"){
                                            std::cout << "logout" << std::endl;
                                            user_token = "";
                                            mksini_load();
                                            mksini_set("app", "token", user_token);
                                            mksini_set("app", "username", "");
                                            mksini_set("app", "avatar", "");
                                            mksini_save();
                                            mksini_free();
                                            system("systemctl stop QIDILink-client");
                                        }
                                    }
                                }else if(response["data"].find("action") != response["data"].end()){
                                    std::string rev_action = response["data"]["action"];
                                    std::cout << rev_action<< std::endl;
                                    page_to(TJC_PAGE_QIDI_LINK);
                                    send_cmd_vis(tty_fd, "gm0", "1");
                                    user_token = "";
                                    bind_code = "";
                                    mksini_load();
                                    mksini_set("app", "token", user_token);
                                    mksini_set("app", "username", "");
                                    mksini_set("app", "avatar", "");
                                    mksini_save();
                                    mksini_free();
                                    system("systemctl stop QIDILink-client");
                                }else{
                                    printKeysAndValues(response["data"]);
                                }
                        }
                        catch(std::exception & e)
                        {
                            std::cout << "Parse json failed" << std::endl;  

                        }
                    })
                    .perform_sync();
            }
            for (int i = 0; i < (is_in_qr_page || is_in_user_page ? 2 : 90); ++i)
            {
                if (i % 30 == 0){
                    std::cout << "Sleeping for "<< i <<" second..." << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        MKSLOG("Background update device thread exit");
    }).detach();
    return NULL;
}