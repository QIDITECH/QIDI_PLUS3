#include <fstream>

#include <wpa_ctrl.h>

#include "include/MakerbaseClient.h"
#include "include/MoonrakerAPI.h"
#include "include/MakerbaseIPC.h"
#include "include/MakerbaseSerial.h"
#include "include/MakerbaseParseMessage.h"
#include "include/MakerbasePanel.h"
#include "include/MakerbaseParseIni.h"
#include "include/MakerbaseWiFi.h"
#include "include/MakerbaseNetwork.h"

#include "include/KlippyGcodes.h"

#include "include/mks_log.h"
#include "include/mks_preview.h"
#include "include/mks_init.h"
#include "include/mks_gpio.h"
#include "include/mks_update.h"
#include "include/mks_wpa_cli.h"

#include "include/ui.h"
#include "include/send_msg.h"
#include "include/KlippyRest.h"
#include "include/event.h"
#include "include/utils.h"
#include "include/system_setting.h"
#include "include/app_client.h"

extern int tty_fd;
extern int current_page_id;
extern int previous_page_id;

MakerbaseClient *ep;

extern std::string serial_by_id;
extern std::string str_gimage;

bool is_download_to_screen = false;

bool find_screen_tft_file = false;

int main(int argc, char** argv) {


	DIR *dir;
	struct dirent *entry;
	dir = opendir("/dev");
	if (dir == NULL) {
		perror("无法打开目录 /dev");
	}
	while ((entry = readdir(dir))) {
		if (strstr(entry->d_name, "/dev/sd") == entry->d_name) {
			if (strlen(entry->d_name) >= 8) {
				char *partition_suffix = entry->d_name + 7;
				if (partition_suffix[1] == '1') {
					char command[256];
                    snprintf(command, sizeof(command), "/usr/bin/systemctl --no-block restart makerbase-automount@%s.service", partition_suffix);
					system(command);
				}
			}
		}
	}

	// 开机检测U盘是否读取成功，否的话则重启服务
	if (((access("/dev/sda", F_OK) == 0 && access("/dev/sda1", F_OK) == 0) ||
		(access("/dev/sda1", F_OK) == 0 && access("/dev/sdb1", F_OK) == 0)) &&
		access("/home/mks/gcode_files/sda1", F_OK) != 0) {
			system("/usr/bin/systemctl --no-block restart makerbase-automount@sda1.service");
	}

	// U盘内的UI文件 -> 执行UI升级
    if (access("/home/mks/gcode_files/sda1/mksscreen.recovery", F_OK) == 0) {
        system("cp /home/mks/gcode_files/sda1/mksscreen.recovery /root/800_480.tft; sync");
    }

	// U盘内的MKS文件 -> 执行MKS升级
    if (access("/home/mks/gcode_files/sda1/mksclient.recovery", F_OK) == 0) {
        system("dpkg -i /home/mks/gcode_files/sda1/mksclient.recovery; sync");
    }

	// UI在线升级程序：文件读取 -> 升级
	if (access("/root/800_480.tft", F_OK) == 0) {
		find_screen_tft_file = true;
		MKSLOG_BLUE("找到tft升级文件");
	} else {
		find_screen_tft_file = false;
		MKSLOG_BLUE("没有找到tft升级文件");
	}

	// UI在线升级结束 -> 文件备份
	if (find_screen_tft_file == true) {
		system("/root/uart; mv /root/800_480.tft /root/800_480.tft.bak");
		find_screen_tft_file = false;
	}

	// 清除废弃的frp服务&文件
	if(clear_deprecated_services())
		MKSLOG("Succeeded in removing deprecated services.");

	// WIFI信息监听线程
	pthread_t wpa_recv_thread;
	pthread_create(&wpa_recv_thread, NULL, mks_wifi_hdlevent_thread, NULL);

	// 启动Moonraker连接线程
	std::string host = "localhost";
	std::string url = "ws://localhost:7125/websocket?";
	if (argc == 2) {
		host = argv[1];
		url = "ws://" + host + ":7125/websocket?";
	}
	ep = new MakerbaseClient(host, "7125");

	// 检查Moonraker连接
	int connection_retires = 0;	
	while (!ep->GetIsConnected()) {
		MKSLOG("Failed to connect to moonraker, retries: %d", ++connection_retires);
		ep->Close();
		ep->Connect(url);
		sleep(1);
		ep->GetStatus();
	}

	// Moonraker消息解析线程
	pthread_t tid_msg_parse;
	pthread_create(&tid_msg_parse, NULL, json_parse, NULL);

	int fd;		// 串口文件描述符
	char buff[4096];	// 缓冲区大小
	int count;			// 数量

	if ((fd = open("/dev/ttyS1", O_RDWR | O_NDELAY | O_NOCTTY)) < 0) {
		printf("Open tty failed\n");
	} else {
		tty_fd = fd;
		printf("Open tty success\n");
		set_option(fd, 115200, 8, 'N', 1);
		try
		{
			fcntl(fd, F_SETFL, FNDELAY);
			send_cmd_val(tty_fd, "logo.soc_version", "15"); // 检测SOC与UI是否匹配，4.2.15版本发送信息为15
			
			//4.2.1 CLL 修复开机读取不到参数
			get_total_time();
			sleep(2);
			sub_object_status();									// 订阅相关的参数

			sleep(2);

			get_object_status();									// 主动获取必备的参数
			sleep(2);
			system_setting_init();		// 系统设置初始化
			get_wlan0_status();
			mks_wpa_scan_scanresults();
			get_ssid_list_pages();
			mks_get_version();		// 获取SOC版本

			//B
			pthread_t bind_pthread;
			printf("Create thread\n");
			pthread_create(&bind_pthread, NULL, open_bind_thread, NULL);
			sleep(3);
			
			// 下面if：如果没有找到UI升级文件，就按照顺序开机
			if (find_screen_tft_file == false) {
				previous_page_id = TJC_PAGE_LOGO;
				if (get_mks_oobe_enabled() == true) {
					current_page_id = TJC_PAGE_OPEN_LANGUAGE;
				} else {
					current_page_id = TJC_PAGE_MAIN;
				}
				page_to(current_page_id);
			} 
		}
		catch(const std::exception& e)
		{
			std::cerr << "Page main error, " << e.what() << '\n';
		}
	}

	while(1) {
		if ((count = read(fd, buff, sizeof(buff))) > 0) {
			char *cmd = buff;
			parse_cmd_msg_from_tjc_screen(cmd);
			memset(buff, 0, sizeof(buff));
		}
		usleep(1000);
	}
	close(fd);
	return 0;
}
