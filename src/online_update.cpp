#include <cstdlib>
#include <exception>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

#include <curl/curl.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "../include/event.h"
#include "../include/mks_log.h"
#include "../include/MakerbaseParseIni.h"
#include "../include/online_update.h"
#include "../include/send_msg.h"
#include "../include/system_setting.h"
#include "../include/ui.h"
#include "../include/mks_update.h"

//B
#include "../include/Http.hpp"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
extern int previous_page_id;


#define REMOTE_VERSION_PATH "/tmp/remote_version"
#define UPDATE_INFO_PATH "/tmp/update_info.ini"
#define SOC_DOWNLOAD_PATH "/tmp/QD_Max_SOC"
#define UI_DOWNLOAD_PATH "/tmp/QD_Max3_UI5.0"
#define SUCCESS 0
#define NO_UPDATE 1
#define GENERAL_EXCEPTION -1
#define NETWORK_FAILURE -4
#define DOWNLOAD_FAILURE -5
#define INSTALLATION_FAILURE -6

extern int tty_fd;
extern int current_page_id;

// 在线更新
static void execute_version_check_event();
static void execute_download_update_event();

static std::string local_soc_version;
static std::string remote_soc_version;
static std::string base_download_url;
static double progress;

// 在线更新 检查更新事件
// 点击在线更新时触发
void btn_online_update_click_handler()
{
    page_to(TJC_PAGE_SEARCH_SERVER);
	mksini_load_from_path("/root/xindi/version");
	local_soc_version = mksini_getstring("version", "soc", "V1.0.0");
	mksini_free();
    base_download_url = "https://download_" + sys_set.server + ".qidi3dprinter.com/QD_Plus3";
	// 与UI线程分离
    std::thread([]() {
        execute_version_check_event();
    }).detach();
}

// 在线更新 下载安装更新事件
// 点击确定时触发
void btn_update_click_handler()
{	
    page_to(TJC_PAGE_UPDATING);
    send_cmd_val(tty_fd, "j0", "0");
    send_cmd_vis(tty_fd, "j0", "1");
    send_cmd_vis(tty_fd, "t1", "1");
    send_cmd_vis(tty_fd, "t2", "1");
	MKSLOG("Downloading from remote server");

	// 与UI线程分离
	std::thread([]() {
		execute_download_update_event();
	}).detach();
}

// 定义进度回调函数的上下文
struct ProgressContext {
    double total_to_download;
    double downloaded_so_far;
    double *progress_percentage;
};

// 用于curl检测进度的回调 每次进度更新都会触发该函数
int CurlProgressCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    ProgressContext *context = static_cast<ProgressContext *>(clientp);
    if (dltotal > 0) {
        *context->progress_percentage = static_cast<double>(dlnow) / static_cast<double>(dltotal) * 100.0; //进度数值
		int temp = (int) (*context->progress_percentage);

        // 打印进度值
        std::cout << "Progress: " << temp << "%" << std::endl;

        if (current_page_id == TJC_PAGE_UPDATING) {
            send_cmd_val(tty_fd, "j0", std::to_string(temp));
            send_cmd_txt(tty_fd, "t1", std::to_string(temp) + "%");
        }
    }
    return 0;
}

// curl 回调
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    std::ofstream *outFile = (std::ofstream *)userp;
    outFile->write((char *)contents, size *nmemb);
    return size *nmemb;
}

int download_to_file(const std::string &url, const std::string &local_path, double &progress_percentage) 
{
    try 
	{
        CURL *curl;
        CURLcode res;
        long http_code = 0;
        boost::filesystem::ofstream outFile(local_path, std::ios::binary);
        if (!outFile.is_open()) 
		{
            MKSLOG_RED("Failed to open file %s", local_path);
            return DOWNLOAD_FAILURE;
        }

 		ProgressContext progress_context = {0, 0, &progress_percentage};

        curl = curl_easy_init();
        if (curl) 
		{
            struct curl_slist *headers = nullptr;
            headers = curl_slist_append(headers, "Cache-Control: no-cache");

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outFile);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 10);
            curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
            curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, CurlProgressCallback);
            curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progress_context);

            res = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

            if (res != CURLE_OK) 
			{
				MKSLOG_RED("Error: Failed to download %s: %s", url.c_str(), curl_easy_strerror(res));
                curl_easy_cleanup(curl);
                curl_slist_free_all(headers);
                return DOWNLOAD_FAILURE;
            }
            if (http_code != 200) 
			{
                MKSLOG_RED("Error: Received HTTP code %s while downloading %s", std::to_string(http_code), url);
                curl_easy_cleanup(curl);
                curl_slist_free_all(headers);
                return DOWNLOAD_FAILURE;
            }

            outFile.close();
            MKSLOG("Successfully downloaded %s", local_path);
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
        } else 
		{
            MKSLOG_RED("Error: curl initialization failed");
            return NETWORK_FAILURE;
        }

        return SUCCESS;
    } 
	catch (const std::exception &e) 
	{
        MKSLOG_RED("Exception caught: %s", e.what());
        return GENERAL_EXCEPTION;
    }
}

// 为了不阻塞主线程 使用新线程执行脚本
static void execute_version_check_event() {
	// 执行版本检查
    std::string url;
    if(sys_set.server == "aws")
        url = "https://api.qidi3dprinter.com/code/x_plus3_version_info";
    else
        url = "https://api2.qidi3dprinter.com/code/x_plus3_version_info";
    Http http = Http::get(url);
    http.header("accept", "application/json")
        .timeout_connect(15)
        .timeout_max(15)
        .on_complete([&](std::string body, unsigned) {
        std::cout << "bind :" << body << std::endl;  
        try {
            json j = json::parse(body);

            if (j.find("message") != j.end()) {
                if (j["message"] == "success") {
                    if (j.find("firmware")!= j.end()) {
                        if (j["firmware"].empty()) {
                            std::cout << "No firmware version" << std::endl;
                        }
                        else {
                            if (j["firmware"].find("SOC") != j["firmware"].end()
                                && j["firmware"]["SOC"].find("description") != j["firmware"]["SOC"].end()
                                && j["firmware"]["SOC"].find("version") != j["firmware"]["SOC"].end()
                                && j["firmware"]["SOC"].find("force_update") != j["firmware"]["SOC"].end()) {
                                std::string version = j["firmware"]["SOC"]["version"];
                                std::string description = j["firmware"]["SOC"]["description"]["CN"];
                                bool force_update = j["firmware"]["SOC"]["force_update"].get<bool>();
                                if (version != local_soc_version) 
                                {
                                    page_to(TJC_PAGE_ONLINE_UPDATE);
                                    send_cmd_txt(tty_fd, "t_cn", j["firmware"]["SOC"]["description"]["CN"]);
                                    send_cmd_txt(tty_fd, "t_ru", j["firmware"]["SOC"]["description"]["RU"]);
                                    send_cmd_txt(tty_fd, "t_en", j["firmware"]["SOC"]["description"]["US"]);
                                    send_cmd_txt(tty_fd, "t_jp", j["firmware"]["SOC"]["description"]["JP"]);
                                    send_cmd_txt(tty_fd, "t_fr", j["firmware"]["SOC"]["description"]["FR"]);
                                    send_cmd_txt(tty_fd, "t_gr", j["firmware"]["SOC"]["description"]["DE"]);
                                    send_cmd_txt(tty_fd, "t0", local_soc_version);
                                    send_cmd_txt(tty_fd, "t1", version);
                                } else {
                                    page_to(TJC_PAGE_ABOUT);
                                    send_cmd_txt(tty_fd, "t0", local_soc_version);
                                    send_cmd_vis(tty_fd, "t4", "1");
                                    send_cmd_vis(tty_fd, "t5", "0");
                                }
                            }
                        }
                    }
                }
            }
        }
        catch (...) {
            std::cout << "parse error "<< std::endl;
            // 解析失败就要返回鸭
            page_to(TJC_PAGE_ABOUT);
            send_cmd_txt(tty_fd, "t0", local_soc_version);
            send_cmd_vis(tty_fd, "t5", "1");
            send_cmd_vis(tty_fd, "t4", "0");
        }
            })
        .on_error([&](std::string body, std::string error, unsigned int status) {
            page_to(TJC_PAGE_ABOUT);
            send_cmd_txt(tty_fd, "t0", local_soc_version);
            send_cmd_vis(tty_fd, "t5", "1");
            send_cmd_vis(tty_fd, "t4", "0");
    }).perform();
}

/********************************
 *     在线更新 下载更新逻辑
 ********************************/
int install_update(const std::string &component_name, const std::string &local_path) 
{
    try 
	{
        MKSLOG("Start installing %s", component_name);

        if (component_name.find("SOC") != std::string::npos) 
		{
            std::string install_cmd = "dpkg -i --force-overwrite " + local_path;
            int result = system(install_cmd.c_str());
            if (result != 0)
			{
                MKSLOG_RED("Error: Failed to install %s", component_name);
                return INSTALLATION_FAILURE;
            } else 
			{
                MKSLOG("Successfully installed %s", component_name);
                boost::filesystem::remove(local_path);
            }
        }
		else if (component_name.find("UI") != std::string::npos)
		{
			// UI安装逻辑
            std::string install_cmd = "cp " + local_path + " /root/800_480.tft";
            int result = system(install_cmd.c_str());
            if (result != 0)
			{
                MKSLOG_RED("Error: Failed to install %s", component_name);
                return INSTALLATION_FAILURE;
            } else 
			{
                MKSLOG("Successfully installed %s", component_name);
                boost::filesystem::remove(local_path);
            }
		}
		else
		{
            MKSLOG_RED("Component %s not found.", component_name);
            return INSTALLATION_FAILURE;
        }

        return SUCCESS;
    } 
	catch (const std::exception& e) 
	{
        MKSLOG_RED("Exception caught: %s", e.what());
        return GENERAL_EXCEPTION;
    }
}

int update_system() 
{
	// 下载更新
    std::string soc_url = base_download_url + "/QD_Plus_SOC";
	std::string ui_url  = base_download_url + "/QD_Plus3_UI5.0";
    MKSLOG_GREEN("soc_url: %s", soc_url.c_str());
    
    send_cmd_txt(tty_fd, "t2", "Downloading file SOC");
    int soc_download_status = download_to_file(soc_url, SOC_DOWNLOAD_PATH, progress);
    send_cmd_val(tty_fd, "j0", "0");
    send_cmd_txt(tty_fd, "t2", "Downloading file UI");
    int ui_download_status = download_to_file(ui_url, UI_DOWNLOAD_PATH, progress);

    if (soc_download_status != SUCCESS)
        return soc_download_status;

    if (ui_download_status != SUCCESS)
        return ui_download_status;

	// 安装更新
    send_cmd_txt(tty_fd, "t2", "Installing SOC");
    int soc_install_status = install_update("SOC", SOC_DOWNLOAD_PATH);
    send_cmd_txt(tty_fd, "t2", "Installing UI");
	int ui_install_status = install_update("UI", UI_DOWNLOAD_PATH);

    if (soc_install_status != SUCCESS)
        return soc_install_status;

    if (ui_install_status != SUCCESS)
        return ui_install_status;

    return SUCCESS;
}

static void execute_download_update_event() 
{
    int status = update_system();
	std::string msg;

    MKSLOG("Download update status code: %d", status);

	switch (status)
	{
        //TODO_UI: 展示错误原因
		case GENERAL_EXCEPTION:
		case NETWORK_FAILURE:
		case DOWNLOAD_FAILURE:
		case INSTALLATION_FAILURE:
            page_to(TJC_PAGE_ABOUT);
            send_cmd_vis(tty_fd, "t3", "1");
			break;
		case SUCCESS:
            // 更新成功则重启makerbase-client
            system("systemctl restart makerbase-client");
			break;
		default:
			MKSLOG_RED("Uncaught exception code: %d", status);
	}
}