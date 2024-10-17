#include <iostream>
#include <curl/curl.h>
#include <set>
#include <stack>
#include <algorithm>

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>

#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <fstream>
#include <string>
#include <sstream>
#include <map>

#include "../include/event.h"
#include "../include/app_client.h"
#include "../include/CurlWrapper.h"
#include "../include/KlippyGcodes.h"
#include "../include/send_msg.h"
#include "../include/ui.h"
#include "../include/system_setting.h"
#include "../include/Http.hpp"

#include "../include/mks_printer.h"
#include "../include/mks_file.h"
#include "../include/mks_update.h"
#include "../include/mks_wpa_cli.h"
#include "../include/MakerbaseSerial.h"
#include "../include/MakerbaseClient.h"
#include "../include/MakerbaseShell.h"
#include "../include/MakerbasePanel.h"
#include "../include/MakerbaseParseIni.h"
#include "../include/MakerbaseWiFi.h"
#include "../include/MakerbaseNetwork.h"
#include "../include/mks_log.h"

#include <nlohmann/json.hpp>

extern int tty_fd;
extern bool no_error;

extern MakerbaseClient *ep;
extern std::string get_user_token;
extern std::string get_username;
extern std::string get_avatar; 
extern bool is_in_qr_page;
extern bool is_in_user_page;

extern bool gimage_is_showed;
extern bool simage_is_showed;

extern int current_page_id;                    // 当前页面的id号
extern int previous_page_id;                   // 上一页面的id号
extern int next_page_id;                       // 下一页面的id号

/* Printer Extern Varible*/
bool mks_ethernet;

extern bool mks_led_status;
extern bool mks_beep_status;
extern bool mks_fila_status;
extern int mks_language_status;
extern int mks_extruder_target;
extern int mks_heater_bed_target;
extern int mks_hot_target;
extern std::string mks_babystep_value;
extern std::string mks_version_soc;
extern std::string mks_version_mcu;
extern std::string mks_version_ui;

extern std::string printer_webhooks_state;             
extern std::string printer_webhooks_state_message;
extern std::string current_webhooks_state_message;
extern float printer_gcode_move_speed_factor;
float current_speed_factor = 0;
extern float printer_gcode_move_speed;
float current_extruder_factor = 0;
extern float printer_gcode_move_extrude_factor;
extern float printer_gcode_move_homing_origin[4];                  // [X, Y, Z, E] - 返回应用于每个轴的“gcode 偏移”。例如，可以检查“Z”轴以确定通过“babystepping”应用了多少偏移量。
extern float printer_gcode_move_position[4];
extern float printer_gcode_move_gcode_position[4];
extern std::string printer_toolhead_homed_axes;
extern float printer_toolhead_print_time;
extern float printer_toolhead_extimated_print_time;
extern double printer_toolhead_position[4];
extern double printer_toolhead_axis_minimum[4];
extern double printer_toolhead_axis_maximum[4];
extern double x_position;
extern double y_position;
extern double z_position;
extern float e_position;
// gcode, z坐标
extern double gcode_z_position;

extern int printer_extruder_temperature;
int current_extruder_temperature = 0;
extern int printer_extruder_target;
int current_extruder_target = 0;
extern int printer_heater_bed_temperature;
int current_heater_bed_temperature = 0;
extern int printer_heater_bed_target;
int current_heater_bed_target = 0;
extern int printer_hot_temperature;
int current_hot_temperature = 0;
extern int printer_hot_target;
int current_hot_target = 0;
extern float printer_fan_speed;
extern float printer_heater_fan_speed;
extern float printer_heater_fan_my_nozzle_fan1_speed;
extern float printer_out_pin_fan0_value;
float current_out_pin_fan0_value = 0;
extern float printer_out_pin_fan2_value;
float current_out_pin_fan2_value = 0;

//2.1.2 CLL 新增fan3
extern float printer_out_pin_fan3_value;
float current_out_pin_fan3_value = 0;

extern float printer_out_pin_beep_value;
extern std::string printer_idle_timeout_state;
extern float printer_printing_time;
extern std::string printer_print_stats_filename;
extern float printer_print_stats_total_duration;
extern float printer_print_stats_print_duration;
extern float printer_print_stats_filament_used;
extern std::string printer_print_stats_state;          // 这个状态很有用
extern std::string printer_print_stats_message;        // error detected, error message
extern std::string printer_display_status_message;
extern int printer_display_status_progress;

extern float auto_level_dist;
extern bool auto_level_finished;
extern bool auto_level_enabled;

std::string str_manual_level_offset;
extern float manual_level_dist;
extern int manual_level_count;
extern bool manual_level_finished;

extern float printer_bed_mesh_mesh_min[2];
extern float printer_bed_mesh_mesh_max[2];
extern float printer_bed_mesh_profiles_mks_points[4][4];
extern float printer_bed_mesh_profiles_mks_mesh_params_tension;
extern float printer_bed_mesh_profiles_mks_mesh_params_mesh_x_pps;
extern std::string printer_bed_mesh_profiles_mks_mesh_params_algo;
extern float printer_bed_mesh_profiles_mks_mesh_params_min_x;
extern float printer_bed_mesh_profiles_mks_mesh_params_min_y;
extern float printer_bed_mesh_profiles_mks_mesh_params_x_count;
extern float printer_bed_mesh_profiles_mks_mesh_params_y_count;
extern float printer_bed_mesh_profiles_mks_mesh_params_mesh_y_pps;
extern float printer_bed_mesh_profiles_mks_mesh_params_max_x;
extern float printer_bed_mesh_profiles_mks_mesh_params_max_y;

extern float page_set_zoffset_x_y_position[16][2];

extern float page_set_zoffset_z_position[16];

extern bool fresh_page_set_zoffset_data;
extern bool refresh_page_auto_finish_data;
extern int page_set_zoffset_index;

// filament switch sensor fila
extern bool filament_switch_sensor_fila_filament_detected;
extern bool filament_switch_sensor_fila_enabled;

extern float printer_caselight_value;

extern int printer_pause_taget;

extern bool printer_pause_resume_is_paused;

extern float printer_set_offset;
extern float printer_z_offset;
extern float printer_intern_z_offset;
extern float printer_extern_z_offset;

extern float printer_move_dist;
extern int printer_filament_extruder_target;
extern int printer_filament_extruedr_dist;

// probe
extern float printer_probe_x_zoffset;
extern float printer_probe_y_zoffset;
extern float printer_probe_z_zoffset;

// printer info software version
extern std::string printer_info_software_version;

/* wifi */
extern std::string page_wifi_ssid_list[5];
extern int page_wifi_current_pages;
extern int page_wifi_ssid_list_pages;
extern std::string get_wifi_name;
/* 文件列表处理相关变量 */
// extern std::set<std::string> page_files_dirname_filename_list;

extern bool filelist_changed;

extern int page_files_pages;
extern int page_files_current_pages;
extern int page_files_folder_layers;
extern std::string page_files_previous_path;
extern std::string page_files_root_path;
extern std::string page_files_path;

extern std::string page_files_list_name[8];                                     // 文件列表显示文件名称
extern std::string page_files_list_show_name[8];                                // 文件列表名称
extern std::string page_files_list_show_type[8];        // 文件类型: [f]或者[d]，或者[n]

extern std::stack<std::string> page_files_path_stack;          // 路径栈
extern std::string page_files_root_path;                       // Klippy根目录
extern std::string page_files_previous_path;                   // 之前的路径
extern std::string page_files_path;                            // 文件所在路径
extern std::string page_files_print_files_path;                // 要打印的文件路径

extern int file_metadata_estimated_time;
extern std::string file_metadata_filename;
extern float file_metadata_filament_total;
extern int file_metadata_object_height;
extern std::string file_metadata_filament_name;
extern float file_metadata_filament_weight_total;
extern std::string file_metadata_gimage;
extern std::string file_metadata_simage;

//2.1.2 CLL 打印前判断耗材种类并弹窗
extern std::string file_metadata_filament_type;

extern int level_mode;

extern bool printing_keyboard_enabled;
// extern bool filament_keyboard_enabled;
extern bool auto_level_button_enabled;
extern bool manual_level_button_enabled;

extern bool show_preview_complete;
extern bool show_preview_gimage_completed;

/* 补偿值 */
float page_set_zoffset_number;

/* 共振补偿值 */
std::string page_syntony_shaper_freq_x = "";
std::string page_syntony_shaper_freq_y = "";
bool page_syntony_finished = false;

/* PID调节 */
bool page_pid_finished = false;

/* wifi */
extern struct mks_wifi_status_result_t status_result;
extern struct mks_wifi_status_t wpa_status;

std::string wifi_ip_address;

/* 打完关机 */
bool page_printing_shutdown_enable = false;

/* wifi键盘打开判断 */
extern bool printing_wifi_keyboard_enabled;


/* refresh about success*/
bool page_about_successed = false;

extern bool is_download_to_screen;          // main.cpp 里面的变量

/* update page */
extern int filesize;

/* server history totals */
extern int total_jobs;
extern double total_time;
extern double total_print_time;
extern double total_filament_used;

/* 800个小时 */
int mks_total_printed_minutes;

/* 设置打印调平的目标温度 */
extern int level_mode_printing_extruder_target;
extern int level_mode_printing_heater_bed_target;

extern bool level_mode_printing_is_printing_level;

/* wifi 相关 */
std::string current_connected_ssid_name = "";

extern bool page_wifi_list_ssid_button_enabled[5];

// page print filament
extern bool page_print_filament_extrude_restract_button;
extern bool page_filament_extrude_button;
extern bool page_filament_unload_button;

// oobe开箱引导
bool mks_oobe_enabled = false;
bool current_mks_oobe_enabled = false;

// 调平
bool start_pre_auto_level = false;
bool all_level_saving = false;

extern bool mks_file_parse_finished;

// reset
// extern bool page_reset_to_about;

// 正在保存页面
bool is_refresh_page_saving = false;

std::string temp_idle_state = "";

// ip
extern std::string mks_page_internet_ip;

//2023.4.20 打印过文件标红
//extern std::string printed_file_path;

//2023.4.27 使网页打印显示预览图
bool jump_to_print = false;

//2023.5.8 修改自动调平页面跳转
int printer_auto_level_heater_bed_target;

//2023.5.8 CLL 报错弹窗
extern std::string output_console;

//2023.5.11 CLL 修复页面反复跳转bug
bool printer_ready = false;

//2.1.2 CLL 新增热床调平
extern bool printer_bed_leveling_state;

//4.2.3 CLL 耗材确认弹窗新增不再提示按钮
extern bool preview_pop_1_on;
extern bool preview_pop_2_on;

//4.2.4 CLL 修复断料检测与退料冲突bug
bool previous_filament_sensor_state = false;

//4.2.4 CLL 修改调平
std::string previous_zoffset;

//4.2.5 CLL 新增息屏功能
extern bool previous_caselight_value;

extern int load_target;
extern bool load_mode;

std::map<int, Server_config> serverConfigs;
std::string selected_server;
int current_server_page;
int total_server_count;
bool open_qr_refreshed = false;
int mks_connection_method;

/* 更新页面处理 */
void refresh_page_show() {
    if (current_page_id != TJC_PAGE_PRINTING) {
        // MKSLOG_BLUE("来到这个地方, # %s", printer_print_stats_state.data());
        switch (current_page_id)
        {
        case TJC_PAGE_PRINT_ZOFFSET:
            break;
        
        case TJC_PAGE_PRINT_FILAMENT:
            /*
            if (printer_print_stats_state == "printing") {
                if (printer_print_stats_filename != "") {
                    MKSLOG_BLUE("跳入到打印函数\n");
                    page_to(TJC_PAGE_PRINTING);
                }
            }
            */
            break;
        
        case TJC_PAGE_STOP_PRINT:
        case TJC_PAGE_PRINT_F_POP:
        case TJC_PAGE_POWER_OFF:
        case TJC_PAGE_STOPPING:
        case TJC_PAGE_LEVEL_PRINT:
        case TJC_PAGE_POP_1:
        //2023.5.11 CLL 修复页面跳转bug
        case TJC_PAGE_GCODE_ERROR:
        case TJC_PAGE_DETECT_ERROR:
        case TJC_PAGE_RESET:
        case TJC_PAGE_PREVIEW:

        //2.1.2 CLL 打印前判断耗材种类并弹窗
        case TJC_PAGE_PREVIEW_POP_1:
        case TJC_PAGE_PREVIEW_POP_2:
            break;
        
        default:
            // MKSLOG_BLUE("来到这个地方, # %s", printer_print_stats_state.data());
            // MKSLOG_BLUE("来到这个地方, # %s", printer_print_stats_filename.data());
            if (printer_print_stats_state == "printing") {
                if (printer_print_stats_filename != "") {
                    sleep(5);
                    //4.2.10 CLL 修改断料检测开关逻辑
                    if (mks_fila_status == true) {
                        filament_sensor_switch(true);
                    }
                    MKSLOG_BLUE("跳入到打印函数\n");
                    //4.2.5 CLL 新增息屏功能
                    if (previous_caselight_value == true) {
                        led_on_off();
                        previous_caselight_value = false;
                    }
                    //4.2.7 CLL 修改网页打印信息订阅
                    if (jump_to_print == true) {
                        print_start();
                        printer_ready = false;
                        show_preview_complete = false;
                        page_to(TJC_PAGE_PREVIEW);
                        break;
                    } else {
                        print_start();
                        if (level_mode_printing_is_printing_level == false) {
                            //2023.4.27 使网页打印显示预览图
                            printer_ready = false;
                            //show_preview_complete = false;
                            //jump_to_print = true;
                            page_to(TJC_PAGE_PREVIEW);
                        } else {
                            page_to(TJC_PAGE_LEVEL_PRINT);
                        }
                    }
                }
            }
            break;
        }
    }

    /* 先屏蔽掉下面这一段 */
    /*
    if (current_page_id != TJC_PAGE_PRINT_FILAMENT) {
        // MKSLOG_BLUE("来到这个地方, # %s", printer_print_stats_state.data());
        switch (current_page_id)
        {
        case TJC_PAGE_PRINTING:
            if (printer_print_stats_state == "paused") {
                if (printer_print_stats_filename != "") {
                    MKSLOG_BLUE("跳入到打印暂停函数\n");
                    if (level_mode_printing_is_printing_level == false) {
                        page_to(TJC_PAGE_PRINT_FILAMENT);
                    }
                }
            }
            break;

        default:
            break;
        }
    }
    */

    if (current_page_id != TJC_PAGE_RESET) {
        switch (current_page_id)
        {
        case TJC_PAGE_RESET:
        case TJC_PAGE_POWER_OFF:
        case TJC_PAGE_ABOUT:
        case TJC_PAGE_SERVICE:
        case TJC_PAGE_RESTORING:
        case TJC_PAGE_RESTORE_CONFIG:
            break;
        
        default:
            // 喷头板断开的情况下跳到重启页面
            if (printer_webhooks_state == "shutdown" || printer_webhooks_state == "error") {
                //4.2.5 CLL 新增息屏功能
                if (previous_caselight_value == true) {
                    led_on_off();
                    previous_caselight_value = false;
                }
                page_to(TJC_PAGE_RESET);
                std::cout << "重启页面" << std::endl;
                if (current_webhooks_state_message != printer_webhooks_state_message) {
                    current_webhooks_state_message = printer_webhooks_state_message;
                    std::string temp = printer_webhooks_state_message;
                    replace(temp.begin(), temp.end(), '\n', '.');
                    replace(temp.begin(), temp.end(), '\'', ' ');
                    replace(temp.begin(), temp.end(), '\"', ' ');
                    send_cmd_txt(tty_fd, "t1", temp);
                }
            }
            break;
        }
    } else if (current_page_id == TJC_PAGE_RESET) {
        // MKSLOG_RED("Printer webhooks state: %s", printer_webhooks_state.c_str());
        // MKSLOG_RED("Printer idle timeout state: %s", printer_idle_timeout_state.c_str());
        if (printer_webhooks_state == "shutdown" || printer_webhooks_state == "error") {
            if (current_webhooks_state_message != printer_webhooks_state_message) {
                current_webhooks_state_message = printer_webhooks_state_message;
                std::string temp = printer_webhooks_state_message;
                replace(temp.begin(), temp.end(), '\n', '.');
                replace(temp.begin(), temp.end(), '\'', ' ');
                replace(temp.begin(), temp.end(), '\"', ' ');
                // send_cmd_txt(tty_fd, "t1", current_webhooks_state_message.substr(0, current_webhooks_state_message.find(",")));
                send_cmd_txt(tty_fd, "t1", temp);
            }
        }
        if (printer_webhooks_state == "ready") {
            page_to(TJC_PAGE_SYS_OK);
        }
    }

    /*
    if (current_page_id != TJC_PAGE_PREVIEW) {
        show_preview_complete = false;
    }
    */

    // std::cout << "####################==== " << current_page_id << std::endl;

    switch (current_page_id)
    {
    case TJC_PAGE_MAIN:
        refresh_page_main();
        break;

    case TJC_PAGE_FILE_LIST_1:
        break;

    case TJC_PAGE_FILE_LIST_2:
        break;

    case TJC_PAGE_PREVIEW:
        refresh_page_preview();
        break;

    case TJC_PAGE_PRINTING:
        refresh_page_printing();
        break;

    case TJC_PAGE_PRINT_FILAMENT:
        refresh_page_print_filament();
        break;

    case TJC_PAGE_MOVE:
        refresh_page_move();
        break;

    case TJC_PAGE_PRINT_ZOFFSET:
        refresh_page_printing_zoffset();
        break;

    case TJC_PAGE_PRINT_FINISH:
        break;

    case TJC_PAGE_FILAMENT:
        refresh_page_filament();
        break;

    case TJC_PAGE_LEVELING_INIT:
        refresh_page_leveling_init();
        break;

    case TJC_PAGE_AUTO_MOVE:
        refresh_page_auto_move();
        break;

    case TJC_PAGE_AUTO_FINISH:
        refresh_page_auto_finish();
        break;

    case TJC_PAGE_SYNTONY_MOVE:
        refresh_page_systony_move();
        break;

    case TJC_PAGE_SYNTONY_FINISH:
        refresh_page_syntony_finish();
        break;

    case TJC_PAGE_SET_ZOFFSET:
        refresh_page_set_zoffset();
        break;

    case TJC_PAGE_INTERNET:
        refresh_page_internet();
        break;

    case TJC_PAGE_MANUAL_MOVE_2:
        refresh_page_manual_move_2();
        break;

    case TJC_PAGE_SET_ZOFFSET_2:
        refresh_page_set_zoffset_2();
        break;

    case TJC_PAGE_PRINT_F_POP:
        break;

    case TJC_PAGE_STOPPING:
        refresh_page_stopping();
        break;

    case TJC_PAGE_AUTO_LEVEL:
        refresh_page_auto_level();
        break;

    case TJC_PAGE_ABOUT:
        refresh_page_about();
        break;

    case TJC_PAGE_WIFI_LIST_2:
        refresh_page_wifi_list_2();
        break;

    case TJC_PAGE_SAVING:
        refresh_page_saving();
        break;

    case TJC_PAGE_PID_WORKING:
        refresh_page_pid_working();
        break;

    case TJC_PAGE_PID_FINISH:
        refresh_page_pid_finish();
        break;

    case TJC_PAGE_WIFI_CONNECT:
        refresh_page_wifi_connect();
        break;

    case TJC_PAGE_UPDATE_SCREEN:
        refresh_page_update_screen();
        break;

    case TJC_PAGE_LEVEL_PRINT:
        refresh_page_level_print();
        break;

    case TJC_PAGE_WIFI_KEYBOARD:
        refresh_page_wifi_keyboard();
        break;

    case TJC_PAGE_OPEN_LEVELINIT:
        refresh_page_open_levelinit();
        break;

    case TJC_PAGE_OPEN_LEVELING:
        refresh_page_open_leveling();
        break;

    case TJC_PAGE_OPEN_LEVELED:
        refresh_page_open_leveled();
        break;

    case TJC_PAGE_OPEN_SYNTONY:
        refresh_page_open_syntony();
        break;
    
    case TJC_PAGE_SAVING_2:
        open_syntony_finish();
        break;

    case TJC_PAGE_FILAMENT_VIDEO_2:
        refresh_page_filament_video_2();
        break;

    //2023.5.8 修改自动调平页面跳转
    case TJC_PAGE_LEVEL_NULL_1:
        refresh_page_level_null_1();
        break;

    case TJC_PAGE_OPEN_HEATER_BED:
        refresh_page_open_heater_bed();
        break;

    //2.1.2 CLL 新增退料界面
    case TJC_PAGE_UNLOADING:
        refresh_page_unloading();
        break;

    case TJC_PAGE_PREVIEW_POP_1:
    case TJC_PAGE_PREVIEW_POP_2:
        refresh_page_preview_pop();
        break;

    //4.2.5 CLL 修复UI按下效果
    case TJC_PAGE_OPEN_LEVEL:
        refresh_page_open_level();
        break;

    case TJC_PAGE_RESTORING:
        refresh_page_restoring();
        break;

    case TJC_PAGE_LOADING:
        refresh_page_loading();
        break;

    case TJC_PAGE_PRE_HEATING_2:
        refresh_page_pre_heating_2();
        break;
    
    case TJC_PAGE_NETWORK_SET:
        refresh_page_show_ip();
        break;

    case TJC_PAGE_SERVER_SET:
        refresh_page_server_set();
        break;

    default:
        break;
    }
}

void refresh_page_internet() {
    std::cout << "refreshing internet" << std::endl;
    std::string local_ip = get_eth0_ip();
    if (local_ip.find(":") != -1)
        send_cmd_txt(tty_fd, "t0", "");
    else
        send_cmd_txt(tty_fd, "t0", local_ip);
}

void refresh_page_open_leveling() {
    if ((printer_idle_timeout_state == "Ready")) {
        page_to(TJC_PAGE_OPEN_LEVELED);
        auto_level_enabled = false;
    }
}

void refresh_page_open_syntony() {
    if (temp_idle_state != printer_idle_timeout_state) {
        temp_idle_state = printer_idle_timeout_state;
        MKSLOG_BLUE("Printer ide_timeout state: %s", printer_idle_timeout_state.c_str());
        MKSLOG_BLUE("Printer webhooks state: %s", printer_webhooks_state.c_str());
    }

    if ((printer_idle_timeout_state == "Ready")) {
        page_to(TJC_PAGE_SAVING_2);
    }
}

void refresh_page_open_leveled() {
    if ( printer_idle_timeout_state == "Idle" && printer_webhooks_state == "ready" ) {
        auto_level_finished = true;
    }

}

void refresh_page_open_levelinit() {

    if (start_pre_auto_level == false) {
        usleep(800000);           // 把这个延时打开
        start_pre_auto_level = true;
    }
    if (printer_idle_timeout_state == "Ready") {
        page_to(TJC_PAGE_OPEN_LEVEL);
    }
    
}

void refresh_page_wifi_keyboard() {
    if (printing_wifi_keyboard_enabled == true) {
        send_cmd_txt(tty_fd, "t0", get_wifi_name);
    }
}

void refresh_page_level_print() {
    send_cmd_txt(tty_fd, "t0", std::to_string(printer_extruder_temperature) + "/" + std::to_string(printer_extruder_target));
    send_cmd_txt(tty_fd, "t1", std::to_string(printer_heater_bed_temperature) + "/" + std::to_string(printer_heater_bed_target));
    if (printer_print_stats_state == "complete") {
        if (level_mode_printing_is_printing_level == false) {
            // complete_print();
            // page_to(TJC_PAGE_PRINT_FINISH);
        } else {
            complete_print();
            page_to(TJC_PAGE_CHOOSE_MODEL);
        }
    }
}

void refresh_page_update_screen() {
    
    if (is_download_to_screen == false) {
        sleep(3);
        init_download_to_screen();
        sleep(1);
        send_cmd_download(tty_fd, filesize);

        is_download_to_screen = true;
    }
    
}

void refresh_page_wifi_connect() {

}

void refresh_page_pid_working() {
    MKSLOG_BLUE("Printer ide_timeout state: %s", printer_idle_timeout_state.c_str());
    MKSLOG_BLUE("Printer webhooks state: %s", printer_webhooks_state.c_str());
    if ((printer_idle_timeout_state == "Ready")) {
        page_to(TJC_PAGE_PID_FINISH);
    }
}

void refresh_page_pid_finish() {
    MKSLOG_BLUE("Printer ide_timeout state: %s", printer_idle_timeout_state.c_str());
    MKSLOG_BLUE("Printer webhooks state: %s", printer_webhooks_state.c_str());
    if (page_pid_finished == false) {
        page_pid_finished = true;
        ep->Send(json_run_a_gcode("SAVE_CONFIG"));
    }

    
    if ((printer_idle_timeout_state == "Idle" && printer_webhooks_state == "ready") || (printer_idle_timeout_state == "Ready")) {
        // sleep(10);
        sleep(8);
        page_to(TJC_PAGE_LEVEL_MODE);
    }
    
}

void refresh_page_saving() {
    sleep(5);
    
    if ( (printer_idle_timeout_state == "Idle" && printer_webhooks_state == "ready") || (printer_idle_timeout_state == "Ready" && printer_webhooks_state == "ready") || (printer_idle_timeout_state == "Printing" && printer_webhooks_state == "ready")) {
        sleep(5);
        auto_level_finished = true;
        manual_level_finished = true;
        get_object_status();
        sub_object_status();
    }

    // 修改调平跳转逻辑，避免提前跳出
    if (auto_level_finished == true) {
        auto_level_finished = false;
        auto_level_button_enabled = true;
        start_pre_auto_level = false;
        sleep(5);
        get_object_status();
        sub_object_status();
        init_mks_status();
        system("sync");
        if (current_page_id == TJC_PAGE_SAVING) {
            page_to(TJC_PAGE_LEVEL_MODE);
        }
    }
    
    if (all_level_saving == true) {
        all_level_saving = false;
    }
    
}

//4.2.3 CLL 修复WiFi刷新bug
void refresh_page_wifi_list_2() {
    if (access("/var/run/wpa_supplicant/wlan0", F_OK) == 0){
        if (printing_wifi_keyboard_enabled == false) {
            send_cmd_txt(tty_fd, "t0", status_result.ip_address);
            // std::cout << "Status_result Wpa_state" << status_result.wpa_state << std::endl;
            if (strcmp(status_result.wpa_state, "COMPLETED") == 0) {
                // std::cout << "已连接" << std::endl;
    
            } else if (strcmp(status_result.wpa_state, "INACTIVE")) {
                // std::cout << "未连接" << std::endl;
            }
        } else {
            send_cmd_txt(tty_fd, "t0", get_wifi_name);
        }
    } else {
	    page_wifi_list_ssid_button_enabled[0] = false;
        page_wifi_list_ssid_button_enabled[1] = false;
        page_wifi_list_ssid_button_enabled[2] = false;
        page_wifi_list_ssid_button_enabled[3] = false;
        page_wifi_list_ssid_button_enabled[4] = false;
        send_cmd_txt(tty_fd, "t0", get_wifi_name);
        page_wifi_ssid_list_pages = 0;
        page_wifi_current_pages = 0;
    }
}

void refresh_page_syntony_finish() {
    MKSLOG_BLUE("Printer ide_timeout state: %s", printer_idle_timeout_state.c_str());
    MKSLOG_BLUE("Printer webhooks state: %s", printer_webhooks_state.c_str());
    if (page_syntony_finished == false) {
        page_syntony_finished = true;
        all_level_saving = false;
    }
    
    if (printer_idle_timeout_state == "Ready" && printer_webhooks_state == "ready") {
        MKSLOG_BLUE("Printer webhooks state: %s", printer_webhooks_state.c_str());
        sleep(15);
        system("sync");             // 保存好配置文件
        sleep(5);
        all_level_saving = false;
        init_mks_status();      // 重启后初始化之前保存的参数
        sub_object_status();
        get_object_status();
        page_to(TJC_PAGE_LEVEL_MODE);
        MKSLOG_RED("从739行跳出去");
    }
    
}

void refresh_page_no_updata() {
    if (page_about_successed == false) {
        send_cmd_txt(tty_fd, "t0", mks_version_soc);
        send_cmd_txt(tty_fd, "t1", mks_version_mcu);
        page_about_successed = true;
    }
    if (detect_update() == true) {
        page_about_successed = false;
        page_to(TJC_PAGE_ABOUT);
    }
    
    //2023.4.25-1 隐藏开机引导
    current_mks_oobe_enabled = get_mks_oobe_enabled();
    if (current_mks_oobe_enabled == false) {
        send_cmd_txt(tty_fd, "b1", "");
        send_cmd_picc(tty_fd, "b1", "132");
    } else {
        send_cmd_txt(tty_fd, "b1", "开机引导");
        send_cmd_picc(tty_fd, "b1", "367");
    }
    
    if (filelist_changed == true) {
        filelist_changed = false;
    }
}

void refresh_page_about() {
    if (page_about_successed == false) {
        send_cmd_txt(tty_fd, "t0", mks_version_soc);
        send_cmd_txt(tty_fd, "t1", mks_version_mcu);
        page_about_successed = true;
    }

    current_mks_oobe_enabled = get_mks_oobe_enabled();
    if (current_mks_oobe_enabled == false) {
        send_cmd_txt(tty_fd, "b1", "");
        send_cmd_picc(tty_fd, "b1", "132");
        send_cmd_vis(tty_fd, "b1", "0");
    } else {
        send_cmd_txt(tty_fd, "b1", "开机引导");
        send_cmd_picc(tty_fd, "b1", "367");
        send_cmd_vis(tty_fd, "b1", "1");
    }
    
    if (filelist_changed == true) {
        filelist_changed = false;
    }
}

//4.2.5 CLL 修复UI按下效果
void refresh_page_auto_level() {
    //4.2.10 使调平界面显示最终zoffset值
    send_cmd_txt(tty_fd, "t1", std::to_string(printer_gcode_move_homing_origin[2] - 0.15).substr(0,5) + "mm");
    if (auto_level_dist == (float)0.01) {
        send_cmd_picc(tty_fd, "b3", "95");
        send_cmd_picc2(tty_fd, "b3", "317");
        send_cmd_picc(tty_fd, "b4", "95");
        send_cmd_picc2(tty_fd, "b4", "316");
        send_cmd_picc(tty_fd, "b5", "95");
        send_cmd_picc2(tty_fd, "b5", "316");
        send_cmd_picc(tty_fd, "b6", "95");
        send_cmd_picc2(tty_fd, "b6", "316");

    } else if (auto_level_dist == (float)0.05) {
        send_cmd_picc(tty_fd, "b3", "96");
        send_cmd_picc2(tty_fd, "b3", "316");
        send_cmd_picc(tty_fd, "b4", "96");
        send_cmd_picc2(tty_fd, "b4", "317");
        send_cmd_picc(tty_fd, "b5", "96");
        send_cmd_picc2(tty_fd, "b5", "316");
        send_cmd_picc(tty_fd, "b6", "96");
        send_cmd_picc2(tty_fd, "b6", "316");
    } else if (auto_level_dist == (float)0.1) {
        send_cmd_picc(tty_fd, "b3", "97");
        send_cmd_picc2(tty_fd, "b3", "316");
        send_cmd_picc(tty_fd, "b4", "97");
        send_cmd_picc2(tty_fd, "b4", "316");
        send_cmd_picc(tty_fd, "b5", "97");
        send_cmd_picc2(tty_fd, "b5", "317");
        send_cmd_picc(tty_fd, "b6", "97");
        send_cmd_picc2(tty_fd, "b6", "316");
    } else if (auto_level_dist == (float)1) {
        send_cmd_picc(tty_fd, "b3", "98");
        send_cmd_picc2(tty_fd, "b3", "316");
        send_cmd_picc(tty_fd, "b4", "98");
        send_cmd_picc2(tty_fd, "b4", "316");
        send_cmd_picc(tty_fd, "b5", "98");
        send_cmd_picc2(tty_fd, "b5", "316");
        send_cmd_picc(tty_fd, "b6", "98");
        send_cmd_picc2(tty_fd, "b6", "317");
    }
}

void refresh_page_stopping() {
    MKSLOG_BLUE("Printer ide_timeout state: %s", printer_idle_timeout_state.c_str());
    MKSLOG_BLUE("Printer webhooks state: %s", printer_webhooks_state.c_str());
    if (printer_idle_timeout_state == "Ready") {
        // sleep(2);
        //2.1.2 CLL 修复网页显示预览图bug
        clear_previous_data();
        sleep(5);
        //2.1.2 CLL 打印完成或取消时保存zoffset值
        save_current_zoffset();
        // start_to_printing = false;
        page_to(TJC_PAGE_MAIN);
    }
}

//4.2.5 CLL 新增设置Z轴偏移页面
void refresh_page_set_zoffset_2() {
    //4.2.10 CLL 使调平界面显示最终zoffset值
    send_cmd_txt(tty_fd, "t1", std::to_string(printer_gcode_move_homing_origin[2] - 0.15).substr(0,5) + "mm");
    if (auto_level_dist == (float)0.01) {
        send_cmd_picc(tty_fd, "b3", "95");
        send_cmd_picc2(tty_fd, "b3", "317");
        send_cmd_picc(tty_fd, "b4", "95");
        send_cmd_picc2(tty_fd, "b4", "316");
        send_cmd_picc(tty_fd, "b5", "95");
        send_cmd_picc2(tty_fd, "b5", "316");
        send_cmd_picc(tty_fd, "b6", "95");
        send_cmd_picc2(tty_fd, "b6", "316");
    } else if (auto_level_dist == (float)0.05) {
        send_cmd_picc(tty_fd, "b3", "96");
        send_cmd_picc2(tty_fd, "b3", "316");
        send_cmd_picc(tty_fd, "b4", "96");
        send_cmd_picc2(tty_fd, "b4", "317");
        send_cmd_picc(tty_fd, "b5", "96");
        send_cmd_picc2(tty_fd, "b5", "316");
        send_cmd_picc(tty_fd, "b6", "96");
        send_cmd_picc2(tty_fd, "b6", "316");
    } else if (auto_level_dist == (float)0.1) {
        send_cmd_picc(tty_fd, "b3", "97");
        send_cmd_picc2(tty_fd, "b3", "316");
        send_cmd_picc(tty_fd, "b4", "97");
        send_cmd_picc2(tty_fd, "b4", "316");
        send_cmd_picc(tty_fd, "b5", "97");
        send_cmd_picc2(tty_fd, "b5", "317");
        send_cmd_picc(tty_fd, "b6", "97");
        send_cmd_picc2(tty_fd, "b6", "316");
    } else if (auto_level_dist == (float)1) {
        send_cmd_picc(tty_fd, "b3", "98");
        send_cmd_picc2(tty_fd, "b3", "316");
        send_cmd_picc(tty_fd, "b4", "98");
        send_cmd_picc2(tty_fd, "b4", "316");
        send_cmd_picc(tty_fd, "b5", "98");
        send_cmd_picc2(tty_fd, "b5", "316");
        send_cmd_picc(tty_fd, "b6", "98");
        send_cmd_picc2(tty_fd, "b6", "317");
    }
}

void refresh_page_manual_move_2() {
    if ((printer_idle_timeout_state == "Ready")) {
        // fresh_page_set_zoffset_data = false;
        page_to(TJC_PAGE_SET_ZOFFSET_2);
    }
}
 //2023.4.20 修改调平数据显示
void refresh_page_set_zoffset() {

    //4.2.2 CLL 修改调平数据显示为至多36个
    std::string temp[6][6];
    for (int i = 0; i < printer_bed_mesh_profiles_mks_mesh_params_y_count; i++) {
    if (i == 6) {
        break;
    }
    for (int j = 0; j < printer_bed_mesh_profiles_mks_mesh_params_x_count; j++) {
        if (j == 6) {
            break;
        }
        temp[i][j] = std::to_string(printer_bed_mesh_profiles_mks_points[i][j]);
        temp[i][j] = temp[i][j].substr(0, temp[i][j].find(".") + 3);
        send_cmd_txt(tty_fd,"t"+std::to_string(i * 6 + j),temp[i][j]);
    }
    }
}

void refresh_page_systony_move() {
    if (temp_idle_state != printer_idle_timeout_state) {
        temp_idle_state = printer_idle_timeout_state;
        MKSLOG_BLUE("Printer ide_timeout state: %s", printer_idle_timeout_state.c_str());
        MKSLOG_BLUE("Printer webhooks state: %s", printer_webhooks_state.c_str());
    }
    usleep(100000);     // 延时个100毫秒
    if ((printer_idle_timeout_state == "Ready")) {
        page_to(TJC_PAGE_SYNTONY_FINISH);
    }
}

void refresh_page_manual_level() {
    send_cmd_txt(tty_fd, "t1", str_manual_level_offset);
}

void refresh_page_print_filament() {

    MKSLOG_BLUE("Printer idle_timeout state: %s", printer_idle_timeout_state.c_str());
    MKSLOG_BLUE("Printer webhooks state: %s", printer_webhooks_state.c_str());

    if (printer_extruder_target == 0) {
        send_cmd_pco(tty_fd, "t0", "65535");
        send_cmd_pco(tty_fd, "t1", "65535");
    } else {
        send_cmd_pco(tty_fd, "t0", "63488");
        send_cmd_pco(tty_fd, "t1", "63488");
    }

    send_cmd_picc(tty_fd, "q1", std::to_string(printer_display_status_progress + 139));
    send_cmd_txt(tty_fd, "t0", std::to_string(printer_extruder_temperature));
    send_cmd_txt(tty_fd, "t1", std::to_string(printer_extruder_target));
    send_cmd_txt(tty_fd, "t6", show_time((int)(printer_print_stats_print_duration)));
    send_cmd_txt(tty_fd, "t7", printer_print_stats_filename.substr(printer_print_stats_filename.rfind("/") +1 ));
    send_cmd_txt(tty_fd, "t8", show_time(get_cal_printing_time((int)(printer_print_stats_print_duration), file_metadata_estimated_time, printer_display_status_progress)));

    //2023.5.11 CLL 修复页面反复跳转bug
    if (printer_print_stats_state == "paused") {
        printer_ready = true;
    }

    //2023.4.20 网页暂停开始与UI相匹配
    if (printer_print_stats_state == "printing") {
        if (printer_ready == true) {
            //4.2.10 CLL 修改断料检测开关逻辑
            if (mks_fila_status == true) {
                filament_sensor_switch(true);
            }
            page_to(TJC_PAGE_PRINTING);
            printer_ready = false;
        }
    }

    if (printer_print_stats_state == "complete") {
        complete_print();
        //2.1.2 CLL 修复网页显示预览图bug
        clear_previous_data();
        sleep(5);
        //2.1.2 CLL 打印完成或取消时保存zoffset值
        save_current_zoffset();
        page_to(TJC_PAGE_PRINT_FINISH);
        //2.1.2 CLL 打印完成后用时仅刷新一次
        send_cmd_txt(tty_fd, "t0", show_time((int)(printer_print_stats_print_duration)));
    }

    //2023.5.8 CLL 网页停止后将跳转至打印停止界面
    if (printer_print_stats_state == "standby") {
        page_to(TJC_PAGE_STOPPING);
    }

    //2023.5.8 CLL 报错弹窗
    if (printer_print_stats_state == "error") {
        page_to(TJC_PAGE_GCODE_ERROR);
        send_cmd_txt(tty_fd, "t0", "gcode error:" + output_console);
    }

    //4.2.7 CLL 打印时长时间暂停造成的打印中止会跳转至主页面
    if (printer_idle_timeout_state == "Idle") {
        page_to(TJC_PAGE_MAIN);
    }

}

//2023.4.20 修改调平数据显示
void refresh_page_auto_finish() {

    if ( printer_idle_timeout_state == "Idle" && printer_webhooks_state == "ready" ) {
        auto_level_finished = true;
    }
}

//2023.4.20 修改调平数据显示
void refresh_page_manual_finish() {
    
}

void refresh_page_auto_move() {
    if ((printer_idle_timeout_state == "Ready")) {
    // if ((printer_idle_timeout_state == "Ready" || printer_idle_timeout_state == "Idle") && printer_webhooks_state == "Ready") {
        page_to(TJC_PAGE_AUTO_FINISH);
        auto_level_enabled = false;
    }
}

void refresh_page_leveling_init() {

    if (start_pre_auto_level == false) {
        usleep(800000);           // 把这个延时打开
        start_pre_auto_level = true;
    }
    if (printer_idle_timeout_state == "Ready") {
        switch (level_mode)
        {
        case TJC_PAGE_AUTO_LEVEL:
            page_to(TJC_PAGE_AUTO_LEVEL);
            // ep->Send(json_run_a_gcode("BED_MESH_CALIBRATE\nBED_MESH_PROFILE SAVE=\"name\"\n"));
            break;

        case TJC_PAGE_SET_ZOFFSET:
            page_to(TJC_PAGE_SET_ZOFFSET_2);
            break;
        
        default:
            break;
        }
    }
}

void refresh_page_move() {
    // std::string x_pos = std::to_string(printer_toolhead_position[0]);
    std::string x_pos = std::to_string(x_position);
    x_pos = x_pos.substr(0, x_pos.find(".") + 2);
    // std::string y_pos = std::to_string(printer_toolhead_position[1]);
    std::string y_pos = std::to_string(y_position);
    y_pos = y_pos.substr(0, y_pos.find(".") + 2);
    // std::string z_pos = std::to_string(printer_toolhead_position[2]);
    std::string z_pos = std::to_string(z_position);
    z_pos = z_pos.substr(0, z_pos.find(".") + 2);

    send_cmd_txt(tty_fd, "t0", x_pos);
    send_cmd_txt(tty_fd, "t1", y_pos);
    send_cmd_txt(tty_fd, "t2", z_pos);

    if (printer_move_dist == (float)0.1) {
        send_cmd_picc(tty_fd, "b16", "59");
        send_cmd_picc2(tty_fd, "b16", "436");
        send_cmd_picc(tty_fd, "b17", "59");
        send_cmd_picc2(tty_fd, "b17", "437");
        send_cmd_picc(tty_fd, "b18", "59");
        send_cmd_picc2(tty_fd, "b18", "437");
    } else if (printer_move_dist == (float)1.0) {
        send_cmd_picc(tty_fd, "b16", "60");
        send_cmd_picc2(tty_fd, "b16", "437");
        send_cmd_picc(tty_fd, "b17", "60");
        send_cmd_picc2(tty_fd, "b17", "436");
        send_cmd_picc(tty_fd, "b18", "60");
        send_cmd_picc2(tty_fd, "b18", "437");
    } else if (printer_move_dist == (float)10) {
        send_cmd_picc(tty_fd, "b16", "61");
        send_cmd_picc2(tty_fd, "b16", "437");
        send_cmd_picc(tty_fd, "b17", "61");
        send_cmd_picc2(tty_fd, "b17", "437");
        send_cmd_picc(tty_fd, "b18", "61");
        send_cmd_picc2(tty_fd, "b18", "436");
    }
}

void refresh_page_filament() {
    //4.2.5 CLL 修复UI按下效果
    if (printer_filament_extruedr_dist == 10) {
        send_cmd_picc(tty_fd, "b23", "67");
        send_cmd_picc2(tty_fd, "b23", "420");
        send_cmd_picc(tty_fd, "b24", "66");
        send_cmd_picc2(tty_fd, "b24", "421");
        send_cmd_picc(tty_fd, "b25", "66");
        send_cmd_picc2(tty_fd, "b25", "421");
    } else if (printer_filament_extruedr_dist == 50) {
        send_cmd_picc(tty_fd, "b23", "66");
        send_cmd_picc2(tty_fd, "b23", "421");
        send_cmd_picc(tty_fd, "b24", "67");
        send_cmd_picc2(tty_fd, "b24", "420");
        send_cmd_picc(tty_fd, "b25", "66");
        send_cmd_picc2(tty_fd, "b25", "421");
    } else if (printer_filament_extruedr_dist == 100) {
        send_cmd_picc(tty_fd, "b23", "66");
        send_cmd_picc2(tty_fd, "b23", "421");
        send_cmd_picc(tty_fd, "b24", "66");
        send_cmd_picc2(tty_fd, "b24", "421");
        send_cmd_picc(tty_fd, "b25", "67");
        send_cmd_picc2(tty_fd, "b25", "420");
    }

    send_cmd_txt(tty_fd, "t0", std::to_string(printer_extruder_temperature) + "/");
    send_cmd_txt(tty_fd, "t1", std::to_string(printer_heater_bed_temperature) + "/");
    send_cmd_txt(tty_fd, "t2", std::to_string(printer_hot_temperature) + "/");

    send_cmd_val(tty_fd, "n0", std::to_string(printer_extruder_target));
    send_cmd_val(tty_fd, "n1", std::to_string(printer_heater_bed_target));
    send_cmd_val(tty_fd, "n2", std::to_string(printer_hot_target));

    send_cmd_val(tty_fd, "n3", std::to_string((int)(printer_out_pin_fan0_value * 100)));
    send_cmd_val(tty_fd, "n4", std::to_string((int)(printer_out_pin_fan2_value * 100)));
    //2.1.2 CLL 新增fan3
    send_cmd_val(tty_fd, "n5", std::to_string((int)(printer_out_pin_fan3_value * 100)));

    if (printer_extruder_target > 0) {
        send_cmd_picc(tty_fd, "b0", "69");
        send_cmd_pco(tty_fd, "t0", "63488");
        send_cmd_pco(tty_fd, "n0", "63488");
    } else {
        send_cmd_picc(tty_fd, "b0", "70");
        send_cmd_pco(tty_fd, "t0", "65535");
        send_cmd_pco(tty_fd, "n0", "65535");
    }

    if (printer_heater_bed_target > 0) {
        send_cmd_picc(tty_fd, "b1", "71");
        send_cmd_pco(tty_fd, "t1", "63488");
        send_cmd_pco(tty_fd, "n1", "63488");
    } else {
        send_cmd_picc(tty_fd, "b1", "72");
        send_cmd_pco(tty_fd, "t1", "65535");
        send_cmd_pco(tty_fd, "n1", "65535");
    }

    if (printer_hot_target > 0) {
        send_cmd_picc(tty_fd, "b2", "73");
        send_cmd_pco(tty_fd, "t2", "63488");
        send_cmd_pco(tty_fd, "n2", "63488");
    } else {
        send_cmd_picc(tty_fd, "b2", "74");
        send_cmd_pco(tty_fd, "t2", "65535");
        send_cmd_pco(tty_fd, "n2", "65535");
    }

    //2.1.2 CLL 优化按钮状态更改
    if (printer_out_pin_fan0_value == 0) {
        send_cmd_vid(tty_fd, "gm1", "5");
        send_cmd_picc2(tty_fd, "b21", "420");
    } else {
        send_cmd_vid(tty_fd, "gm1", "6");
        send_cmd_picc2(tty_fd, "b21", "421");
    }

    if (printer_out_pin_fan2_value == 0) {
        send_cmd_vid(tty_fd, "gm2", "7");
        send_cmd_picc2(tty_fd, "b22", "420");
    } else {
        send_cmd_vid(tty_fd, "gm2", "8");
        send_cmd_picc2(tty_fd, "b22", "421");
    }

    if (mks_fila_status == true) {
        send_cmd_picc(tty_fd, "b5", "422");
        send_cmd_picc2(tty_fd, "b5", "421");
    } else {
        send_cmd_picc(tty_fd, "b5", "423");
        send_cmd_picc2(tty_fd, "b5", "420");
    }

    //2.1.2 CLL 新增fan3
    if (printer_out_pin_fan3_value == 0) {
        send_cmd_vid(tty_fd, "gm3", "18");
        send_cmd_picc2(tty_fd, "b3", "420");
    } else {
        send_cmd_vid(tty_fd, "gm3", "19");
        send_cmd_picc2(tty_fd, "b3", "421");
    }

    if (printer_idle_timeout_state == "Printing") {
        
        if (page_filament_extrude_button == true) {
            send_cmd_vid_en(tty_fd, "gm0", 1);
        } else {
            send_cmd_vid_en(tty_fd, "gm0", 0);
        }
    } else {
        send_cmd_vid_en(tty_fd, "gm0", 0);
        page_filament_extrude_button = false;
        if (page_filament_unload_button == true) {
            page_to(TJC_PAGE_FILAMENT_POP_5);
            page_filament_unload_button = false;
        }
        //4.2.7 CLL 修复耗材上下与断料检测冲突
        if (previous_filament_sensor_state == true) {
            set_filament_sensor();
            previous_filament_sensor_state = false;
        }
    }
}

void refresh_page_offset(float intern_zoffset) {
    printer_intern_z_offset = intern_zoffset;
    printer_z_offset = printer_intern_z_offset + printer_extern_z_offset;
}

void refresh_page_printing_zoffset() {
    std::string z_offset = std::to_string(printer_gcode_move_homing_origin[2]);
    std::string show_gcode_z = std::to_string(gcode_z_position);
    z_offset = z_offset.substr(0, z_offset.find(".") + 4);
    show_gcode_z = show_gcode_z.substr(0, show_gcode_z.find(".") + 4);
    if (z_offset != mks_babystep_value) {
        mks_babystep_value = z_offset;
        set_mks_babystep(mks_babystep_value);
    }
    send_cmd_txt(tty_fd, "t1", show_gcode_z);
    send_cmd_txt(tty_fd, "t2", z_offset);
    send_cmd_txt(tty_fd, "t7", printer_print_stats_filename.substr(printer_print_stats_filename.rfind("/") +1 ));
    send_cmd_txt(tty_fd, "t6", show_time((int)(printer_print_stats_print_duration)));
    send_cmd_txt(tty_fd, "t8", show_time(get_cal_printing_time((int)(printer_print_stats_print_duration), file_metadata_estimated_time, printer_display_status_progress)));
    send_cmd_picc(tty_fd, "q1", std::to_string(printer_display_status_progress + 139));
    if (printer_pause_resume_is_paused == false) {
        send_cmd_picc(tty_fd, "b9", "41");
        send_cmd_picc2(tty_fd, "b9", "41");
    } else {
        send_cmd_picc(tty_fd, "b9", "42");
        send_cmd_picc2(tty_fd, "b9", "42");
    }

    //2023.5.11 CLL 修复页面反复跳转bug
    if (printer_print_stats_state == "printing") {
        printer_ready = true;
    }

    //2023.4.20 网页暂停开始与UI相匹配
    if (printer_print_stats_state == "paused") {
        if (printer_ready == true) {
            filament_sensor_switch(false);
            page_to(TJC_PAGE_PRINT_FILAMENT);
            printer_ready = false;
        }
    }

    if (printer_print_stats_state == "complete") {
        if (level_mode_printing_is_printing_level == false) {
            filament_sensor_switch(false);
            complete_print();
            //2.1.2 CLL 修复网页显示预览图bug
            clear_previous_data();
            sleep(5);
            //2.1.2 CLL 打印完成或取消时保存zoffset值
            save_current_zoffset();
            page_to(TJC_PAGE_PRINT_FINISH);
            //2.1.2 CLL 打印完成后用时仅刷新一次
            send_cmd_txt(tty_fd, "t0", show_time((int)(printer_print_stats_print_duration)));
        } else {
            complete_print();
            page_to(TJC_PAGE_CHOOSE_MODEL);
        }
    }

    //2023.5.8 CLL 网页停止后将跳转至打印停止界面
    if (printer_print_stats_state == "standby") {
        filament_sensor_switch(false);
        page_to(TJC_PAGE_STOPPING);
    }
    //2023.5.8 CLL 报错弹窗
    if (printer_print_stats_state == "error") {
        page_to(TJC_PAGE_GCODE_ERROR);
        send_cmd_txt(tty_fd, "t0", "gcode error:" + output_console);
        filament_sensor_switch(false);
    }
    //2.1.2 CLL 在打印设置zoffset界面也会有断料提醒弹窗
    if (filament_switch_sensor_fila_enabled == true) {
        if (filament_switch_sensor_fila_filament_detected == false) {
            printer_ready = false;
            set_print_pause();
            page_to(TJC_PAGE_PRINT_F_POP);
        }
    }
}

void refresh_page_printing() {

    send_cmd_picc(tty_fd, "q0", std::to_string(printer_display_status_progress + 139));
    send_cmd_txt(tty_fd, "t6", show_time((int)(printer_print_stats_print_duration)));
    send_cmd_txt(tty_fd, "t7", printer_print_stats_filename.substr(printer_print_stats_filename.rfind("/") +1 ));
    send_cmd_txt(tty_fd, "t8", show_time(get_cal_printing_time((int)(printer_print_stats_print_duration), file_metadata_estimated_time, printer_display_status_progress)));


    if (printing_keyboard_enabled == false) {

        send_cmd_txt(tty_fd, "t0", std::to_string(printer_extruder_temperature) + "/");
        send_cmd_val(tty_fd, "n0", std::to_string(printer_extruder_target));
        if (printer_extruder_target == 0) {
            send_cmd_pco(tty_fd, "n0", "65535");
            send_cmd_pco(tty_fd, "t0", "65535");
        } else {
            send_cmd_pco(tty_fd, "n0", "63488");
            send_cmd_pco(tty_fd, "t0", "63488");
        }

        send_cmd_txt(tty_fd, "t1", std::to_string(printer_heater_bed_temperature) + "/");
        send_cmd_val(tty_fd, "n1", std::to_string(printer_heater_bed_target));
        if (printer_heater_bed_target == 0) {
            send_cmd_pco(tty_fd, "n1", "65535");
            send_cmd_pco(tty_fd, "t1", "65535");
        } else {
            send_cmd_pco(tty_fd, "n1", "63488");
            send_cmd_pco(tty_fd, "t1", "63488");
        }

        send_cmd_txt(tty_fd, "t2", std::to_string(printer_hot_temperature) + "/");
        send_cmd_val(tty_fd, "n2", std::to_string(printer_hot_target));
        if (printer_hot_target == 0) {
            send_cmd_pco(tty_fd, "n2", "65535");
            send_cmd_pco(tty_fd, "t2", "65535");
        } else {
            send_cmd_pco(tty_fd, "n2", "63488");
            send_cmd_pco(tty_fd, "t2", "63488");
        }

        if (printer_caselight_value == 0) {
            send_cmd_picc(tty_fd, "b10", "427");
            send_cmd_picc2(tty_fd, "b10", "426");
        } else {
            send_cmd_picc(tty_fd, "b10", "428");
            send_cmd_picc2(tty_fd, "b10", "425");
        }

        send_cmd_val(tty_fd, "n5", std::to_string((int)(printer_out_pin_fan0_value * 100)));
        send_cmd_val(tty_fd, "n6", std::to_string((int)(printer_out_pin_fan2_value * 100)));
        //2.1.2 CLL 新增fan3
        send_cmd_val(tty_fd, "n7", std::to_string((int)(printer_out_pin_fan3_value * 100)));

        /* 20230201 打印速度和流量设置。输入105，会变成104 */
        if (current_speed_factor != printer_gcode_move_speed_factor) {
            current_speed_factor = printer_gcode_move_speed_factor;
            send_cmd_val(tty_fd, "n3", std::to_string((int)(round(printer_gcode_move_speed_factor * 100))));
        }

        if (current_extruder_factor != printer_gcode_move_extrude_factor) {
            current_extruder_factor = printer_gcode_move_extrude_factor;
            send_cmd_val(tty_fd, "n4", std::to_string((int)(round(printer_gcode_move_extrude_factor * 100))));
        }
    } else {
        if (printer_pause_resume_is_paused == false) {
            send_cmd_picc(tty_fd, "b10", "41");
            send_cmd_picc2(tty_fd, "b10", "41");
        } else {
            send_cmd_picc(tty_fd, "b10", "42");
            send_cmd_picc2(tty_fd, "b10", "42");
        }
    }

    //2023.5.11 CLL 修复页面反复跳转bug
    if (printer_print_stats_state == "printing") {
        printer_ready = true;
    }

    //2023.4.20 网页暂停开始与UI相匹配
    if (printer_print_stats_state == "paused") {
        if (printer_ready == true) {
            filament_sensor_switch(false);
            page_to(TJC_PAGE_PRINT_FILAMENT);
            printer_ready = false;
        }
    }

    if (printer_print_stats_state == "complete") {
        filament_sensor_switch(false);
        complete_print();
        //2.1.2 CLL 修复网页显示预览图bug
        clear_previous_data();
        sleep(5);
        //2.1.2 CLL 打印完成或取消时保存zoffset值
        save_current_zoffset();
        page_to(TJC_PAGE_PRINT_FINISH);
        //2.1.2 CLL 打印完成后用时仅刷新一次
        send_cmd_txt(tty_fd, "t0", show_time((int)(printer_print_stats_print_duration)));
    }

    //2023.5.8 CLL 网页停止后将跳转至打印停止界面
    if (printer_print_stats_state == "standby") {
        filament_sensor_switch(false);
        page_to(TJC_PAGE_STOPPING);
    }

    //2023.5.8 CLL 报错弹窗
    if (printer_print_stats_state == "error") {
        page_to(TJC_PAGE_GCODE_ERROR);
        send_cmd_txt(tty_fd, "t0", "gcode error:" + output_console);
        filament_sensor_switch(false);
    }

    if (filament_switch_sensor_fila_enabled == true) {
        if (filament_switch_sensor_fila_filament_detected == false) {
            //2.1.2 断料弹窗出现后会暂停打印
            printer_ready = false;
            set_print_pause();
            page_to(TJC_PAGE_PRINT_F_POP);
        }
    }
}

void clear_page_printing_arg() {
    current_extruder_temperature = 0;
    current_extruder_target = 0;
    current_heater_bed_target = 0;
    current_heater_bed_temperature = 0;
    current_hot_target = 0;
    current_hot_temperature = 0;
    current_out_pin_fan0_value = 0;
    current_out_pin_fan2_value = 0;
    //2.1.2 CLL 新增fan3
    current_out_pin_fan3_value = 0;

    current_speed_factor = 0;
    current_extruder_factor = 0;
}

//2023.4.27 网页打印显示预览图
void refresh_page_preview() {
    // MKSLOG_RED("进入到刷新预览图的页面, show_preview_complete %d", show_preview_complete);
    if (mks_file_parse_finished == true ) {
        if (show_preview_complete == false) {

            //4.2.5 CLL 打印文件仅显示文件名
            send_cmd_txt(tty_fd, "t0", file_metadata_filename.substr(file_metadata_filename.rfind("/") + 1));

            MKSLOG_BLUE("file_metadata_estimated_time %d", file_metadata_estimated_time);
            MKSLOG_BLUE("file_metadata_filament_weight_total %f", file_metadata_filament_weight_total);
            MKSLOG_BLUE("file_metadata_filament_total %f", file_metadata_filament_total);
            MKSLOG_BLUE("file_metadata_filament_name %s", file_metadata_filament_name.c_str());

            if (file_metadata_estimated_time) {
                send_cmd_txt(tty_fd, "t1", show_time(file_metadata_estimated_time));
            } else {
                send_cmd_txt(tty_fd, "t1", "-");
            }

            if (file_metadata_filament_weight_total) {
                std::string temp = std::to_string(file_metadata_filament_weight_total);
                
                send_cmd_txt(tty_fd, "t2", temp.substr(0, temp.find(".") + 2) + "g");
            } else {
                send_cmd_txt(tty_fd, "t2", "-");
            }

            if (file_metadata_filament_total) {
                std::string temp = std::to_string(file_metadata_filament_total / 1000);
                send_cmd_txt(tty_fd, "t3", temp.substr(0, temp.find(".") + 2) + "m");
            } else {
                send_cmd_txt(tty_fd, "t3", "-");
            }

        //2.1.2 CLL 识别耗材种类并弹窗
            if (file_metadata_filament_type != "") {
                send_cmd_txt(tty_fd, "t4", file_metadata_filament_type);
            } else if (file_metadata_filament_name != ""){
                send_cmd_txt(tty_fd, "t4", file_metadata_filament_name);
            } else {
                send_cmd_txt(tty_fd, "t4", "-");
            }

        // 刷小图
            if (show_preview_gimage_completed == false) {
                
                send_cmd_txt(tty_fd, "preview.cp0_text", "");
                send_cmd_txt(tty_fd, "preview.add", "");
                if (file_metadata_simage != "") {
                    std::cout << "刷小图" << std::endl;
                    int num = 1024;
                    int len = file_metadata_simage.length();
                    int end = num;
                    std::string s;
                    for (int start = 0; start < len;) {
                        if (end > len) {
                            s = file_metadata_simage.substr(start, len - start);
                            send_cmd_txt(tty_fd, "add", s);
                            tcdrain(tty_fd);
                            send_cmd_txt_plus(tty_fd, "cp0_text", "cp0_text", "add");
                            tcdrain(tty_fd);
                            break;
                        }
                        s = file_metadata_simage.substr(start, num);
                        start = end;
                        end = end + num;
                        send_cmd_txt(tty_fd, "add", s);
                        tcdrain(tty_fd);
                        send_cmd_txt_plus(tty_fd, "cp0_text", "cp0_text", "add");
                        tcdrain(tty_fd);
                    }
                }

                // 刷大图
                if (jump_to_print == false) {
                    send_cmd_cp_close(tty_fd, "preview.cp0");
                    if (file_metadata_gimage != "") {
                        std::cout << "刷大图" << std::endl;
                        // std::cout << "图片字符串的大小为: " << file_metadata_gimage.length() << std::endl;
                        // std::cout << "图片的字节大小为：" << file_metadata_gimage.size() << std::endl;
                        int num = 1024;
                        int len = file_metadata_gimage.length();
                        int end = num;
                        std::string g;
                        // int i = 0;
                        for (int start = 0; start < len;) {
                            if (end > len) {
                                g = file_metadata_gimage.substr(start, len - start);
                                // std::cout << "字串" << i <<  ":" << s << std::endl;
                                send_cmd_cp_image(tty_fd, "cp0", g);
                                tcdrain(tty_fd);
                                break;
                            }
                            g = file_metadata_gimage.substr(start, num);
                            start = end;
                            end = end + num;
                            // std::cout << "字串" << i <<  ":" << s << std::endl;
                            // i++;
                            send_cmd_cp_image(tty_fd, "cp0", g);
                            // usleep(40000);
                            tcdrain(tty_fd);
                        }
                    }
                    show_preview_gimage_completed = true;
                    //2.1.2 CLL 新增热床调平
                    bed_leveling_switch(true);
                }
            }

            show_preview_complete = true;
            if (jump_to_print == true){
                //2.1.2 开始打印前发送PRINT_START_QD
                print_start();
                //2.1.2 CLL 新增网页打印过文件标红
                //printed_file_path = "/" + printer_print_stats_filename;
                jump_to_print = false;
                //2.1.2 CLL 新增热床调平
                bed_leveling_switch(true);
                //2.1.2 CLL 打印前判断耗材种类并弹窗
                check_filament_type();
            }
        }
    }
    //2.1.2 CLL 新增热床调平
    if (printer_bed_leveling_state == true) {
        send_cmd_picc(tty_fd, "b3", "418");
        send_cmd_picc2(tty_fd, "b3", "418");
    }else if (printer_bed_leveling_state == false) {
        send_cmd_picc(tty_fd, "b3", "417");
        send_cmd_picc(tty_fd, "b3", "417");
    }
}

void refresh_page_main() {
    send_cmd_val(tty_fd, "n0", std::to_string(printer_extruder_temperature));
    send_cmd_val(tty_fd, "n1", std::to_string(printer_heater_bed_temperature));
    send_cmd_val(tty_fd, "n2", std::to_string(printer_hot_temperature));

    // MKSLOG_RED("LED灯的状态: %f", printer_caselight_value);
    if (printer_caselight_value == 0) {             // LED灯的logo
        send_cmd_picc(tty_fd, "b0", "1");
    } else {
        send_cmd_picc(tty_fd, "b0", "2");
    }

    // MKSLOG_RED("BEEP的状态: %f", printer_out_pin_beep_value);
    if (printer_out_pin_beep_value == 0) {
        send_cmd_picc(tty_fd, "b1", "3");
    } else {
        send_cmd_picc(tty_fd, "b1", "4");
    }

    if (printer_extruder_target == 0) {
        send_cmd_pco(tty_fd, "n0", "65535");
        send_cmd_picc(tty_fd, "q0", "6");
    } else {
        send_cmd_pco(tty_fd, "n0", "63488");
        send_cmd_picc(tty_fd, "q0", "5");
    }

    if (printer_heater_bed_target == 0) {
        send_cmd_pco(tty_fd, "n1", "65535");
        send_cmd_picc(tty_fd, "q1", "8");
    } else {
        send_cmd_pco(tty_fd, "n1", "63488");
        send_cmd_picc(tty_fd, "q1", "7");
    }

    if (printer_hot_target == 0) {
        send_cmd_pco(tty_fd, "n2", "65535");
        send_cmd_picc(tty_fd, "q2", "10");
    } else {
        send_cmd_pco(tty_fd, "n2", "63488");
        send_cmd_picc(tty_fd, "q2", "9");
    }
}

void refresh_page_files_list_1() {
    for (int i = 0; i < 8; i++) {
        //2023.4.20 打印过文件标红
        send_cmd_txt(tty_fd, "t" + std::to_string(i), page_files_list_show_name[i]);
        //4.2.5 CLL 新增上次打印文件在文件列表第一页第一个显示
        if (page_files_list_show_type[i] =="[c]") {
            send_cmd_picc(tty_fd, "b" + std::to_string(i), "434");
            send_cmd_picc2(tty_fd, "b" + std::to_string(i), "435");
        } else if (page_files_list_show_type[i] == "[d]") {
            send_cmd_picc(tty_fd, "b" + std::to_string(i), std::to_string(13 + i));
            send_cmd_picc2(tty_fd, "b" + std::to_string(i), std::to_string(13 + i));
        } else if (page_files_list_show_type[i] == "[f]") {
            send_cmd_picc(tty_fd, "b" + std::to_string(i), std::to_string(21 + i));
            send_cmd_picc2(tty_fd, "b" + std::to_string(i), std::to_string(21 + i));
        } else if (page_files_list_show_type[i] == "[n]") {
            send_cmd_picc(tty_fd, "b" + std::to_string(i), "11");
            send_cmd_picc2(tty_fd, "b" + std::to_string(i), "11");
        }
    }
    if (page_files_current_pages == 0) {
        send_cmd_picc(tty_fd, "b8", std::to_string(30));
        send_cmd_picc2(tty_fd, "b8", std::to_string(30));
    } else {
        send_cmd_picc(tty_fd, "b8", std::to_string(29));
        send_cmd_picc2(tty_fd, "b8", std::to_string(29));
    }
    if (page_files_current_pages == page_files_pages) {
        send_cmd_picc(tty_fd, "b9", std::to_string(32));
        send_cmd_picc2(tty_fd, "b9", std::to_string(32));
    } else {
        send_cmd_picc(tty_fd, "b9", std::to_string(31));
        send_cmd_picc2(tty_fd, "b9", std::to_string(31));
    }
}

void refresh_page_files_list_2() {
    for (int i = 0; i < 8; i++) {
        send_cmd_txt(tty_fd, "t" + std::to_string(i), page_files_list_show_name[i]);
        if (page_files_list_show_type[i] == "[d]") {
            send_cmd_picc(tty_fd, "b" + std::to_string(i), std::to_string(13 + i));
            send_cmd_picc2(tty_fd, "b" + std::to_string(i), std::to_string(13 + i));
        } else if (page_files_list_show_type[i] == "[f]") {
            send_cmd_picc(tty_fd, "b" + std::to_string(i), std::to_string(21 + i));
            send_cmd_picc2(tty_fd, "b" + std::to_string(i), std::to_string(21 + i));
        } else if (page_files_list_show_type[i] == "[n]") {
            send_cmd_picc(tty_fd, "b" + std::to_string(i), "12");
            send_cmd_picc2(tty_fd, "b" + std::to_string(i), "12");
        }
    }
    send_cmd_txt(tty_fd, "t8", page_files_path);
    if (page_files_current_pages == 0) {
        send_cmd_picc(tty_fd, "b9", std::to_string(35));
        send_cmd_picc2(tty_fd, "b9", std::to_string(35));
    } else {
        send_cmd_picc(tty_fd, "b9", std::to_string(34));
        send_cmd_picc2(tty_fd, "b9", std::to_string(34));
    }
    if (page_files_current_pages == page_files_pages) {
        send_cmd_picc(tty_fd, "b10", std::to_string(37));
        send_cmd_picc2(tty_fd, "b10", std::to_string(37));
    } else {
        send_cmd_picc(tty_fd, "b10", std::to_string(36));
        send_cmd_picc2(tty_fd, "b10", std::to_string(36));
    }
}

void refresh_page_files(int pages) {
    // page_files_folder_layers = 0;
    // page_files_previous_path = "";
    // page_files_root_path = "gcodes/";
    // page_files_path = "";
    get_page_files_filelist(page_files_root_path + page_files_path);
    // page_files_current_pages = 0;
    set_page_files_show_list(pages);
}

/* 订阅内容处理 */
void sub_object_status() {
    ep->Send(json_subscribe_to_printer_object_status(subscribe_objects_status()));
}

void get_object_status() {
    ep->Send(json_query_printer_object_status(subscribe_objects_status()));
}

void get_file_estimated_time(std::string filename) {
    ep->Send(json_get_gcode_metadata(filename));
}

void delete_file(std::string filepath) {
    filelist_changed = false;
    ep->Send(json_file_delete(filepath));
    while (!filelist_changed);
}

void start_printing(std::string filepath) {
    ep->Send(json_print_a_file(filepath));
}

void set_target(std::string heater, int target) {
    ep->Send(json_run_a_gcode(set_heater_temp(heater, target)));
}

void set_extruder_target(int target) {
    set_target("extruder", target);
}

void set_heater_bed_target(int target) {
    set_target("heater_bed", target);
}

void set_hot_target(int target) {
    // set_target("chamber", target);  // 原版
    // TODO：修改腔室加热的变量名
    set_target("hot", target);
}

void set_fan(int speed) {
    ep->Send(json_run_a_gcode(set_fan_speed(speed)));
}

void set_fan0(int speed) {
    ep->Send(json_run_a_gcode(set_fan0_speed(speed)));
}

void set_fan2(int speed) {
    ep->Send(json_run_a_gcode(set_fan2_speed(speed)));
}

//2.1.2 CLL 新增fan3
void set_fan3(int speed) {
    ep->Send(json_run_a_gcode(set_fan3_speed(speed)));
}

void set_intern_zoffset(float offset) {
    printer_set_offset = offset;
     if (offset == (float)0.01) {
         send_cmd_picc(tty_fd, "q0", "48");
     } else if (offset == (float)0.05) {
         send_cmd_picc(tty_fd, "q0", "49");
     } else if (offset == (float)0.1) {
         send_cmd_picc(tty_fd, "q0", "50");
     }  else if (offset == (float)1) {
         send_cmd_picc(tty_fd, "q0", "51");
     }
}

void set_zoffset(bool positive) {
    if (positive == true) {
        ep->Send(json_run_a_gcode("SET_GCODE_OFFSET Z_ADJUST=+" + std::to_string(printer_set_offset) + " MOVE=1"));
    } else {
        ep->Send(json_run_a_gcode("SET_GCODE_OFFSET Z_ADJUST=-" + std::to_string(printer_set_offset) + " MOVE=1"));
    }
}

//4.2.5 CLL 修复UI按下效果
void set_move_dist(float dist) {
    printer_move_dist = dist;
}

void set_printer_speed(int speed) {
    std::cout << "Rate = " << std::to_string(speed) << std::endl;
    ep->Send(json_run_a_gcode(set_speed_rate(std::to_string(speed))));
}

void set_printer_flow(int rate) {
    ep->Send(json_run_a_gcode("M221 S" + (std::to_string(rate))));
}

std::string show_time(int seconds) {
    return std::to_string((int)(seconds/3600)) + "h" + std::to_string((int)((seconds % 3600) / 60)) + "m";
}

void move_home() {
    ep->Send(json_run_a_gcode("G28\n"));
}

void move_x_decrease() {
    ep->Send(move(AXIS_X, "-" + std::to_string(printer_move_dist), 130));
}

void move_x_increase() {
    ep->Send(move(AXIS_X, "+" + std::to_string(printer_move_dist), 130));
}

void move_y_decrease() {
    ep->Send(move(AXIS_Y, "-" + std::to_string(printer_move_dist), 130));
}

void move_y_increase() {
    ep->Send(move(AXIS_Y, "+" + std::to_string(printer_move_dist), 130));
}

void move_z_decrease() {
    ep->Send(move(AXIS_Z, "-" + std::to_string(printer_move_dist), 10));
}

void move_z_increase() {
    ep->Send(move(AXIS_Z, "+" + std::to_string(printer_move_dist), 10));
}

bool get_filament_detected() {
    return filament_switch_sensor_fila_filament_detected;
}

bool get_filament_detected_enable() {
    return filament_switch_sensor_fila_enabled;
}

bool get_print_pause_resume() {
    return printer_pause_resume_is_paused;
}

void set_print_pause_resume() {
    if (printer_pause_resume_is_paused == false) {
        ep->Send(json_run_a_gcode("PAUSE"));
    } else {
        ep->Send(json_run_a_gcode("RESUME"));
    }
}

void set_print_pause() {
    filament_sensor_switch(false);
    ep->Send(json_run_a_gcode("PAUSE"));
}

void set_print_resume() {
    if (get_mks_fila_status() == true) {
        filament_sensor_switch(true);
    }
    ep->Send(json_run_a_gcode("RESUME"));
}

void cancel_print() {
    filament_sensor_switch(false);
    printer_print_stats_filename = "";
    clear_cp0_image();
    filament_sensor_switch(false);
    /* 执行这个避免一直加热 */
    ep->Send(json_run_a_gcode("CANCEL_PRINT"));
    // std::cout << "Total time: " << get_cal_printed_time(total_print_time) << std::endl;
    int printed_minutes = get_cal_printed_time((int)(printer_print_stats_print_duration));
    get_mks_total_printed_time();
    mks_total_printed_minutes = mks_total_printed_minutes + printed_minutes;
    std::cout << "@@@@@@ " << printed_minutes << std::endl;
    set_mks_total_printed_time(mks_total_printed_minutes);
    usleep(10000);
    sdcard_reset_file();
}

void sdcard_reset_file() {
    ep->Send(json_run_a_gcode("SDCARD_RESET_FILE"));
}

void set_auto_level_dist(float dist) {
    auto_level_dist = dist;
}

//4.2.4 CLL 修改调平
void pre_auto_level_init() {
    
    std::cout << "然后进入到这里" << std::endl;
    auto_level_dist = 0.05;                     // 自动调平默认选为0.05  20230207 kenneth.lin
    auto_level_finished = false;
    if (start_pre_auto_level == false) {
        printer_idle_timeout_state = "Printing";
    }
    page_to(TJC_PAGE_LEVELING_INIT);
    sleep(1);
    get_mks_babystep();
    previous_zoffset = mks_babystep_value;
    float z;
    z = std::stof(previous_zoffset);
    z = z + 0.45;
    previous_zoffset = std::to_string(z);
    ep->Send(json_run_a_gcode("SET_GCODE_OFFSET Z=0 MOVE=0\n"));
    ep->Send(json_run_a_gcode("M4029\n"));
    ep->Send(json_run_a_gcode("SET_GCODE_OFFSET Z=0.15 MOVE=1\n"));
}

//4.2.4 CLL 修改调平
void start_auto_level() {
    printer_idle_timeout_state = "Printing";
    auto_level_finished = false;
    ep->Send(json_run_a_gcode("SET_GCODE_OFFSET Z=0 MOVE=1"));
    ep->Send(json_run_a_gcode("BED_MESH_CALIBRATE\n"));
    ep->Send(json_run_a_gcode("G0 X0 Y0 Z50 F6000\n"));
}

/* 完成自动调平 */
void finish_auto_level() {
    //4.2.2 CLL 修复卡在自动调平完成页面
    if ( auto_level_finished == false ||printer_idle_timeout_state == "Idle") {
        ep->Send(json_run_a_gcode("G0 Z50 F600\nG1 X0 Y0 G9000\nSAVE_CONFIG\n"));
        all_level_saving = false;
        page_to(TJC_PAGE_SAVING);
    }
}

void finish_manual_level() {
    if ( manual_level_finished == false) {
        //2023.4.27 手动调平结束归零
        ep->Send(json_run_a_gcode("G0 X0 Y0 Z50 F6000\nSAVE_CONFIG"));
        // TODO: 调平结束新增内容
        ep->Send(json_run_a_gcode("SAVE_CONFIG\n"));
        all_level_saving = false;
        page_to(TJC_PAGE_SAVING);
    }
}

void set_manual_level_dist(float dist) {
    if (dist == (float)0.01) {
        send_cmd_picc(tty_fd, "q0", "95");
    } else if (dist == (float)0.05) {
        send_cmd_picc(tty_fd, "q0", "96");
    } else if (dist == (float)0.1) {
        send_cmd_picc(tty_fd, "q0", "97");
    } else if (dist == (float)1) {
        send_cmd_picc(tty_fd, "q0", "98");
    }
    manual_level_dist = dist;
}

void pre_manual_level_init() {
    manual_level_count = 16;
    manual_level_finished = false;
    std::cout << "进入到手动调平" << std::endl;
    
    set_mks_babystep("-0.150");                  // 手动调平z_offset直接改为-0.1
    page_to(TJC_PAGE_LEVELING_INIT);
    ep->Send(json_run_a_gcode("G28\nABORT\nBED_MESH_CLEAR\n"));
    printer_idle_timeout_state = "Printing";
    ep->Send(json_run_a_gcode("G30"));
    ep->Send(json_run_a_gcode("TESTZ z=-9"));
}

void pre_set_zoffset_init() {
    page_set_zoffset_z_position[0] = printer_bed_mesh_profiles_mks_points[0][0];
    page_set_zoffset_z_position[1] = printer_bed_mesh_profiles_mks_points[0][1];
    page_set_zoffset_z_position[2] = printer_bed_mesh_profiles_mks_points[0][2];
    page_set_zoffset_z_position[3] = printer_bed_mesh_profiles_mks_points[0][3];
    page_set_zoffset_z_position[4] = printer_bed_mesh_profiles_mks_points[1][0];
    page_set_zoffset_z_position[5] = printer_bed_mesh_profiles_mks_points[1][1];
    page_set_zoffset_z_position[6] = printer_bed_mesh_profiles_mks_points[1][2];
    page_set_zoffset_z_position[7] = printer_bed_mesh_profiles_mks_points[1][3];
    page_set_zoffset_z_position[8] = printer_bed_mesh_profiles_mks_points[2][0];
    page_set_zoffset_z_position[9] = printer_bed_mesh_profiles_mks_points[2][1];
    page_set_zoffset_z_position[10] = printer_bed_mesh_profiles_mks_points[2][2];
    page_set_zoffset_z_position[11] = printer_bed_mesh_profiles_mks_points[2][3];
    page_set_zoffset_z_position[12] = printer_bed_mesh_profiles_mks_points[3][0];
    page_set_zoffset_z_position[13] = printer_bed_mesh_profiles_mks_points[3][1];
    page_set_zoffset_z_position[14] = printer_bed_mesh_profiles_mks_points[3][2];
    page_set_zoffset_z_position[15] = printer_bed_mesh_profiles_mks_points[3][3];

    float x_dist = (printer_bed_mesh_profiles_mks_mesh_params_max_x - printer_bed_mesh_profiles_mks_mesh_params_min_x) / 3;
    float y_dist = (printer_bed_mesh_profiles_mks_mesh_params_max_y - printer_bed_mesh_profiles_mks_mesh_params_min_y) / 3;

    /* 1 */
    page_set_zoffset_x_y_position[0][0] = {printer_bed_mesh_profiles_mks_mesh_params_min_x};
    page_set_zoffset_x_y_position[0][1] = {printer_bed_mesh_profiles_mks_mesh_params_min_y};
    /* 2 */
    page_set_zoffset_x_y_position[1][0] = {printer_bed_mesh_profiles_mks_mesh_params_min_x};
    page_set_zoffset_x_y_position[1][1] = {printer_bed_mesh_profiles_mks_mesh_params_min_y + y_dist};
    /* 3 */
    page_set_zoffset_x_y_position[2][0] = {printer_bed_mesh_profiles_mks_mesh_params_min_x};
    page_set_zoffset_x_y_position[2][1] = {printer_bed_mesh_profiles_mks_mesh_params_min_y + y_dist * 2};
    /* 4 */
    page_set_zoffset_x_y_position[3][0] = {printer_bed_mesh_profiles_mks_mesh_params_min_x};
    page_set_zoffset_x_y_position[3][1] = {printer_bed_mesh_profiles_mks_mesh_params_min_y + y_dist * 3};

    /* 5 */
    page_set_zoffset_x_y_position[4][0] = {printer_bed_mesh_profiles_mks_mesh_params_min_x + x_dist};
    page_set_zoffset_x_y_position[4][1] = {printer_bed_mesh_profiles_mks_mesh_params_min_y};
    /* 6 */
    page_set_zoffset_x_y_position[5][0] = {printer_bed_mesh_profiles_mks_mesh_params_min_x + x_dist};
    page_set_zoffset_x_y_position[5][1] = {printer_bed_mesh_profiles_mks_mesh_params_min_y + y_dist};
    /* 7 */
    page_set_zoffset_x_y_position[6][0] = {printer_bed_mesh_profiles_mks_mesh_params_min_x + x_dist};
    page_set_zoffset_x_y_position[6][1] = {printer_bed_mesh_profiles_mks_mesh_params_min_y + y_dist * 2};
    /* 8 */
    page_set_zoffset_x_y_position[7][0] = {printer_bed_mesh_profiles_mks_mesh_params_min_x + x_dist};
    page_set_zoffset_x_y_position[7][1] = {printer_bed_mesh_profiles_mks_mesh_params_min_y + y_dist * 3};

    /* 9 */
    page_set_zoffset_x_y_position[8][0] = {printer_bed_mesh_profiles_mks_mesh_params_min_x + x_dist * 2};
    page_set_zoffset_x_y_position[8][1] = {printer_bed_mesh_profiles_mks_mesh_params_min_y};
    /* 10 */
    page_set_zoffset_x_y_position[9][0] = {printer_bed_mesh_profiles_mks_mesh_params_min_x + x_dist * 2};
    page_set_zoffset_x_y_position[9][1] = {printer_bed_mesh_profiles_mks_mesh_params_min_y + y_dist};
    /* 11 */
    page_set_zoffset_x_y_position[10][0] = {printer_bed_mesh_profiles_mks_mesh_params_min_x + x_dist * 2};
    page_set_zoffset_x_y_position[10][1] = {printer_bed_mesh_profiles_mks_mesh_params_min_y + y_dist * 2};
    /* 12 */
    page_set_zoffset_x_y_position[11][0] = {printer_bed_mesh_profiles_mks_mesh_params_min_x + x_dist * 2};
    page_set_zoffset_x_y_position[11][1] = {printer_bed_mesh_profiles_mks_mesh_params_min_y + y_dist * 3};

    /* 13 */
    page_set_zoffset_x_y_position[12][0] = {printer_bed_mesh_profiles_mks_mesh_params_min_x + x_dist * 3};
    page_set_zoffset_x_y_position[12][1] = {printer_bed_mesh_profiles_mks_mesh_params_min_y};
    /* 14 */
    page_set_zoffset_x_y_position[13][0] = {printer_bed_mesh_profiles_mks_mesh_params_min_x + x_dist * 3};
    page_set_zoffset_x_y_position[13][1] = {printer_bed_mesh_profiles_mks_mesh_params_min_y + y_dist};
    /* 15 */
    page_set_zoffset_x_y_position[14][0] = {printer_bed_mesh_profiles_mks_mesh_params_min_x + x_dist * 3};
    page_set_zoffset_x_y_position[14][1] = {printer_bed_mesh_profiles_mks_mesh_params_min_y + y_dist * 2};
    /* 16 */
    page_set_zoffset_x_y_position[15][0] = {printer_bed_mesh_profiles_mks_mesh_params_min_x + x_dist * 3};
    page_set_zoffset_x_y_position[15][1] = {printer_bed_mesh_profiles_mks_mesh_params_min_y + y_dist * 3};


    std::cout << "进入到补偿值设置页面" << std::endl;
    page_to(TJC_PAGE_LEVELING_INIT);
    printer_idle_timeout_state = "Printing";
    int xtemp = (printer_toolhead_axis_minimum[0] + printer_toolhead_axis_maximum[0]) / 2 - printer_probe_x_zoffset;
    int ytemp = (printer_toolhead_axis_minimum[1] + printer_toolhead_axis_maximum[1]) / 2 - printer_probe_y_zoffset;
    ep->Send(json_run_a_gcode("G28\n"));
    ep->Send(json_run_a_gcode("G1 X" + std::to_string(xtemp) + " Y" + std::to_string(ytemp) + "F3600\nG1 Z0 F1200\n"));
}

//4.2.4 CLL 修改调平
void start_auto_level_dist(bool positive) {
    if (positive == true) {
        //ep->Send(json_run_a_gcode("TESTZ Z=" + std::to_string(auto_level_dist)));
        ep->Send(json_run_a_gcode("SET_GCODE_OFFSET Z_ADJUST=" + std::to_string(auto_level_dist) + " MOVE=1"));
    } else {
        //ep->Send(json_run_a_gcode("TESTZ Z=-" + std::to_string(auto_level_dist)));
        ep->Send(json_run_a_gcode("SET_GCODE_OFFSET Z_ADJUST=-" + std::to_string(auto_level_dist) + " MOVE=1"));
    }
}

void start_manual_level_dist(bool positive) {
    if (positive == true) {
        ep->Send(json_run_a_gcode("TESTZ Z=" + std::to_string(manual_level_dist)));
    } else {
        ep->Send(json_run_a_gcode("TESTZ Z=-" + std::to_string(manual_level_dist)));
    }
}

void set_filament_extruder_target(bool positive) {
    get_mks_extruder_target();
    // printer_filament_extruder_target = printer_extruder_target;
    printer_filament_extruder_target = mks_extruder_target;
    if (positive == true) {
        printer_filament_extruder_target += 3;
    } else {
        printer_filament_extruder_target -= 3;
    }
    
    //2023.4.28 设置喷头温度上限为350
    if (printer_filament_extruder_target >=350) {
        printer_filament_extruder_target =350;
    }
    
    if (printer_filament_extruder_target < 0) {
        printer_filament_extruder_target = 0;
        set_extruder_target(0);
        //set_mks_extruder_target(0);
    } else {
        set_extruder_target(printer_filament_extruder_target);
        //4.2.7 CLL 设置喷头温度保存下限为170
        if (printer_filament_extruder_target >170) {
            set_mks_extruder_target(printer_filament_extruder_target);
        }
    }
}

void set_print_filament_dist(int dist) {
    printer_filament_extruedr_dist = dist;
}

void start_retract() {
    ep->Send(json_run_a_gcode("M83\nG1 E-" + std::to_string(printer_filament_extruedr_dist) + " F300\n"));
    if (current_page_id == TJC_PAGE_FILAMENT) {
        send_cmd_vid(tty_fd, "gm0", "9");
        // send_cmd_vid_en(tty_fd, "gm0", 1);
    } else if (current_page_id == TJC_PAGE_PRINT_FILAMENT) {
        // send_cmd_vid(tty_fd, "gm0", "2");
        // send_cmd_vid_en(tty_fd, "gm0", 1);
    }
}

void start_extrude() {
    ep->Send(json_run_a_gcode("M83\nG1 E" + std::to_string(printer_filament_extruedr_dist) + " F300\n"));
    if (current_page_id == TJC_PAGE_FILAMENT) {
        send_cmd_vid(tty_fd, "gm0", "10");
        // send_cmd_vid_en(tty_fd, "gm0", 1);
    } else if (current_page_id == TJC_PAGE_PRINT_FILAMENT) {
        // send_cmd_vid(tty_fd, "gm0", "3");
    }
}

std::string get_ip(std::string net) {
    char result[MAX_FILE_LEN] = {0};
    std::string cmd = "ifconfig " + net + " | awk 'NR==2{print $2}' | tr -d '\n\r'";
    execute_cmd(cmd.data(), result);
    return result;
}

void move_home_tips() {
    if (printer_move_dist == (float)0.1) {
        page_to(TJC_PAGE_MOVE_POP_1);
    } else if (printer_move_dist == (float)1.0) {
        page_to(TJC_PAGE_MOVE_POP_2);
    } else if (printer_move_dist == (float)10) {
        page_to(TJC_PAGE_MOVE_POP_3);
    }
}

void filament_tips() {
    if (current_page_id == TJC_PAGE_FILAMENT) {
        page_to(TJC_PAGE_FILAMENT_POP);
    } else if (current_page_id == TJC_PAGE_PRINT_FILAMENT) {
        page_to(TJC_PAGE_FILAMENT_POP_3);
    }
}

void move_tips() {
    if (current_page_id == TJC_PAGE_OPEN_VIDEO_4) {
        return;
    }
    if (current_page_id == TJC_PAGE_MOVE) {
        page_to(TJC_PAGE_POP_1);
    } else if (current_page_id == TJC_PAGE_PRINTING || current_page_id == TJC_PAGE_PRINT_ZOFFSET || current_page_id == TJC_PAGE_PRINT_FILAMENT) {
        cancel_print();
        page_to(TJC_PAGE_POP_1);
    } else if (current_page_id == TJC_PAGE_AUTO_LEVEL) {
        page_to(TJC_PAGE_POP_1);
    } else {
        page_to(TJC_PAGE_MOVE);
        page_to(TJC_PAGE_POP_1);
    } //2023.4.27 使坐标移动超出范围弹窗在全部界面都会弹出
}

void reset_klipper() {
    ep->Send(json_run_a_gcode("RESTART\n"));
}

void reset_firmware() {
    ep->Send(json_run_a_gcode("FIRMWARE_RESTART\n"));
}

void finish_print() {
    sdcard_reset_file();
    clear_cp0_image();
    clear_page_preview();
    show_preview_complete = false;
    page_to(TJC_PAGE_MAIN);
}

void set_filament_sensor() {
    std::cout << "filament_switch_sensor fila = " << filament_switch_sensor_fila_enabled << std::endl; 
    //4.2.10 CLL 修改断料检测开关逻辑
    if (mks_fila_status == true) {
        mks_fila_status = false;
        set_mks_fila_status();
    } else {
        mks_fila_status = true;
        set_mks_fila_status();
    }
}

void motors_off() {
    ep->Send(json_emergency_stop());
    sleep(1);
    ep->Send(json_run_a_gcode("FIRMWARE_RESTART\n"));     // 电机解锁改为急停
}

void beep_on_off() {
    if (printer_out_pin_beep_value == 0) {
        ep->Send(json_run_a_gcode("beep_on"));
        mks_beep_status = true;
    } else {
        ep->Send(json_run_a_gcode("beep_off"));
        mks_beep_status = false;
    }
}

void led_on_off() {
    if (printer_caselight_value == 0) {
        ep->Send(json_run_a_gcode("SET_PIN PIN=caselight VALUE=1"));
        mks_led_status = true;
    } else {
        ep->Send(json_run_a_gcode("SET_PIN PIN=caselight VALUE=0"));
        //4.2.8 CLL 息屏不保存状态
        if (previous_caselight_value == false) {
            mks_led_status = false;
        }
    }
}

void move_to_certain_position(int i) {
    page_set_zoffset_index = i;
    page_set_zoffset_number = 0;
    std::string move_gcode = "G1 X" + std::to_string(page_set_zoffset_x_y_position[i][0] + 7.7)
                             + " Y" + std::to_string(page_set_zoffset_x_y_position[i][1] - 40.4)
                             + " Z" + std::to_string(page_set_zoffset_z_position[i]);
    page_set_zoffset_number = page_set_zoffset_z_position[i];
    printer_idle_timeout_state = "Printing";
    ep->Send(json_run_a_gcode(move_gcode));
    page_to(TJC_PAGE_MANUAL_MOVE_2);
}

void shutdown_mcu() {
    system("echo \"SET_PIN PIN=pwc VALUE=0\" > /root/mcu_shutdown.txt");
    ep->Send(json_run_a_gcode("SET_PIN PIN=pwc VALUE=0"));
}

void firmware_reset() {
    ep->Send(json_run_a_gcode("FIRMWARE_RESTART\n"));
}

void go_to_page_power_off() {
    page_to(TJC_PAGE_POWER_OFF);
}

int get_mks_led_status() {
    mksini_load();
    mks_led_status = mksini_getboolean("led", "enable", 0);
    mksini_free();
    return mks_led_status;
}

void set_mks_led_status() {
    mksini_load();
    mksini_set("led", "enable", std::to_string(mks_led_status));
    mksini_save();
    mksini_free();
}

int get_mks_beep_status() {
    mksini_load();
    mks_beep_status = mksini_getboolean("beep", "enable", 0);
    mksini_free();
    return mks_beep_status;
}

void set_mks_beep_status() {
    mksini_load();
    mksini_set("beep", "enable", std::to_string(mks_beep_status));
    mksini_save();
    mksini_free();
    system("sync");
}

void get_mks_language_status() {
    mksini_load();
    mks_language_status = mksini_getint("system", "language", 0);
    mksini_free();
}

void set_mks_language_status() {
    mksini_load();
    mksini_set("system", "language", std::to_string(mks_language_status));
    mksini_save();
    mksini_free();
}

void get_mks_extruder_target() {
    mksini_load();
    mks_extruder_target = mksini_getint("target", "extruder", 200);
    mksini_free();
}

void set_mks_extruder_target(int target) {
    mksini_load();
    mksini_set("target", "extruder", std::to_string(target));
    mksini_save();
    mksini_free();
    system("sync");
}

void get_mks_heater_bed_target() {
    mksini_load();
    mks_heater_bed_target = mksini_getint("target", "heaterbed", 40);
    mksini_free();
}

void set_mks_heater_bed_target(int target) {
    mksini_load();
    std::cout << "######## " << target << std::endl;
    mksini_set("target", "heaterbed", std::to_string(target));
    mksini_save();
    mksini_free();
    system("sync");
}

void get_mks_hot_target() {
    mksini_load();
    mks_hot_target = mksini_getint("target", "hot", 40);
    mksini_free();
}

void set_mks_hot_target(int target) {
    mksini_load();
    std::cout << "######## " << target << std::endl;
    mksini_set("target", "hot", std::to_string(target));
    mksini_save();
    mksini_free();
    system("sync");
}

void filament_extruder_target() {
    get_mks_extruder_target();
    if (printer_extruder_target == 0) {
        set_extruder_target(mks_extruder_target);
    } else {
        set_extruder_target(0);
    }
}

void filament_heater_bed_target() {
    get_mks_heater_bed_target();
    if (0 == printer_heater_bed_target) {
        set_heater_bed_target(mks_heater_bed_target);
    } else {
        set_heater_bed_target(0);
    }
}

void filament_hot_target() {
    get_mks_hot_target();
    if (0 == printer_hot_target) {
        set_hot_target(mks_hot_target);
    } else {
        set_hot_target(0);
    }
}

void filament_fan0() {
    if (printer_out_pin_fan0_value == 0) {
        set_fan0(100);
    }else {
        set_fan0(0);
    }
}

void filament_fan2() {
    if (printer_out_pin_fan2_value == 0) {
        set_fan2(100);
    }else {
        set_fan2(0);
    }
}

//2.1.2 CLL 新增fan3
void filament_fan3() {
    if (printer_out_pin_fan3_value == 0) {
        set_fan3(100);
    }else {
        set_fan3(0);
    }
}

void go_to_reset() {
    if (printer_webhooks_state == "shutdown") {
        page_to(TJC_PAGE_RESET);
    } else {
        page_to(TJC_PAGE_SYS_OK);
    }
}

void go_to_about() {

    ep->Send(json_get_klippy_host_information());
    page_to(TJC_PAGE_ABOUT);
    send_cmd_txt(tty_fd, "t0", mks_version_soc);
    if (!sys_set.internet_enabled)
    {
        // LAN mode: Online update display turns gray, button enable turns off
        send_cmd_tsw(tty_fd, "b5", "0");
        send_cmd_picc(tty_fd, "b5", "332");
        send_cmd_pco(tty_fd, "b5", "38066");
        send_cmd_pco2(tty_fd, "b5", "38066");
    } 
    else {
        // Networking mode: Online update display is normal, button enable is enabled
        send_cmd_tsw(tty_fd, "b5", "1");
        send_cmd_picc(tty_fd, "b5", "132");
        send_cmd_pco(tty_fd, "b5", "65535");
        send_cmd_pco2(tty_fd, "b5", "65535");
    }
}

//4.2.3 CLL 修复WiFi刷新bug
void go_to_network() {
    MKSLOG_GREEN("detected_wlan0 status is %d", detected_wlan0());
    if (detected_wlan0()) {
        mks_wpa_cli_open_connection();
        page_wifi_list_ssid_button_enabled[0] = false;
        page_wifi_list_ssid_button_enabled[1] = false;
        page_wifi_list_ssid_button_enabled[2] = false;
        page_wifi_list_ssid_button_enabled[3] = false;
        page_wifi_list_ssid_button_enabled[4] = false;
        page_wifi_ssid_list_pages = 0;
        page_wifi_current_pages = 0;
        if (strcmp(status_result.wpa_state, "COMPLETED") == 0) {
            current_connected_ssid_name = status_result.ssid;       // 如果已经连接wifi，获取wifi的名字
        } else if (strcmp(status_result.wpa_state, "INACTIVE")) {
            current_connected_ssid_name.clear();                    // 如果没连接wifi，清除掉当前已连接wifi的名字
        }
        page_to(TJC_PAGE_WIFI_LIST_2);
        scan_ssid_and_show();
    } else {
        mks_page_internet_ip = get_eth0_ip();
        page_to(TJC_PAGE_INTERNET);
        send_cmd_txt(tty_fd, "t0", mks_page_internet_ip);
    }
}

//4.2.3 CLL 修复WiFi刷新bug
void scan_ssid_and_show() {
    if (access("/var/run/wpa_supplicant/wlan0", F_OK) == 0) {
        get_wlan0_status();
        mks_wpa_scan_scanresults();
        get_ssid_list_pages();
        page_wifi_current_pages = 0;
        set_page_wifi_ssid_list(page_wifi_current_pages);
        refresh_page_wifi_list();
        send_cmd_vis(tty_fd, "gm0", "0");
    } else {
	    mks_page_internet_ip = get_eth0_ip();
		page_to(TJC_PAGE_INTERNET);
		send_cmd_txt(tty_fd, "t0", mks_page_internet_ip);
    }
}

//4.2.3 CLL 修复WiFi刷新bug
void refresh_page_wifi_list() {
    if (access("/var/run/wpa_supplicant/wlan0", F_OK) == 0) {
        for (int i = 0; i < 5; i++) {
            std::cout << "刷新的wifi: " << page_wifi_ssid_list[i] << std::endl;
            if (0 == page_wifi_current_pages) {
                if (0 == i) {
                    if (strcmp(status_result.wpa_state, "COMPLETED") == 0) {
                        send_cmd_txt(tty_fd, "t" + std::to_string(i+1), status_result.ssid);
                    } else {
                        send_cmd_txt(tty_fd, "t" + std::to_string(i+1), page_wifi_ssid_list[i]);
                    }
                } else {
                    send_cmd_txt(tty_fd, "t" + std::to_string(i+1), page_wifi_ssid_list[i]);
                }
            } else {
                send_cmd_txt(tty_fd, "t" + std::to_string(i+1), page_wifi_ssid_list[i]);
            }

            MKSLOG_BLUE("%d/%d", page_wifi_current_pages + 1, page_wifi_ssid_list_pages);
            if (page_wifi_current_pages == 0) {
                if (strcmp(status_result.wpa_state, "COMPLETED") == 0) {
                    send_cmd_picc(tty_fd, "wifi1", "101");
                    page_wifi_list_ssid_button_enabled[0] = false;
                } else {
                    send_cmd_picc(tty_fd, "wifi1", "102");
                    page_wifi_list_ssid_button_enabled[0] = true;
                }
            } else {
                send_cmd_picc(tty_fd, "wifi1", "102");
                page_wifi_list_ssid_button_enabled[0] = true;
            }

            if (page_wifi_ssid_list[i] == "") {
                std::cout << "刷新为没有锁的图片" << std::endl;
                send_cmd_picc(tty_fd, "wifi" + std::to_string(i + 1), "358");
                send_cmd_picc2(tty_fd, "wifi" + std::to_string(i + 1), "359");
                page_wifi_list_ssid_button_enabled[i] = false;
            } else {
                std::cout << "刷新为有锁的图片" << std::endl;
                send_cmd_picc(tty_fd, "wifi" + std::to_string(i + 1), "102");
                send_cmd_picc2(tty_fd, "wifi" + std::to_string(i + 1), "323");
                page_wifi_list_ssid_button_enabled[i] = true;
            }

            if (page_wifi_ssid_list_pages == 0) {
                send_cmd_picc(tty_fd, "b0", "104");
                send_cmd_picc(tty_fd, "b1", "106");
            } else {
                if (page_wifi_current_pages == 0) {
                    send_cmd_picc(tty_fd, "b0", "104");
                } else {
                    send_cmd_picc(tty_fd, "b0", "103");
                }
                if (page_wifi_current_pages == page_wifi_ssid_list_pages - 1) {
                    send_cmd_picc(tty_fd, "b1", "106");
                } else {
                    send_cmd_picc(tty_fd, "b1", "105");
                }
            }
        }
    }

}

void get_wifi_list_ssid(int index) {
    get_wifi_name.clear();
    get_wifi_name = page_wifi_ssid_list[index];
}

void set_print_filament_target() {
    if (0 == printer_extruder_target) {
        get_mks_extruder_target();
        set_extruder_target(mks_extruder_target);
    } else {
        set_extruder_target(0);
    }
}

void complete_print() {
    if (page_printing_shutdown_enable == false) {
        ep->Send(json_run_a_gcode("PRINT_END"));
    } else {
        ep->Send(json_run_a_gcode("PRINT_END_POWEROFF"));
    }
    int printed_minutes = get_cal_printed_time((int)(printer_print_stats_print_duration));
    get_mks_total_printed_time();
    mks_total_printed_minutes = mks_total_printed_minutes + printed_minutes;
    std::cout << "@@@@@@ " << printed_minutes << std::endl;
    set_mks_total_printed_time(mks_total_printed_minutes);
}

//2023.5.8 CLL 修复网页打印图片显示bug
void back_to_main() {
    clear_previous_data();
    if (current_page_id != TJC_PAGE_POP_1) {
        page_to(TJC_PAGE_MAIN);
    }
}

// 进入共振补偿，共振补偿的问题依然有待修复
void go_to_syntony_move() {
    page_syntony_finished = false;
    printer_idle_timeout_state = "Printing";
    page_to(TJC_PAGE_SYNTONY_MOVE);
    printer_idle_timeout_state = "Printing";
    ep->Send(json_run_a_gcode("M901\n"));
}

void print_ssid_psk(char *psk) {
    MKSLOG_RED("SSID is %s", get_wifi_name.c_str());
    MKSLOG_RED("PSK is %s", psk);
    mks_set_ssid(const_cast<char*>(get_wifi_name.c_str()));
    mks_set_psk(psk);
}

/* 20221229, 挤出之前加热到280,  */
void filament_pop_2_yes() {
    if (printer_extruder_temperature > 280) {
        filament_unload();
        page_to(TJC_PAGE_FILAMENT);
    } else {
        page_to(TJC_PAGE_FILAMENT_POP);
    }
}

void clear_page_preview() {
    file_metadata_filename = "";
    file_metadata_estimated_time = 0;
    file_metadata_filament_weight_total = 0;
    file_metadata_filament_name = "";
    file_metadata_simage = "";
    file_metadata_gimage = "";
}

void set_mks_babystep(std::string value) {
    mksini_load();
    mksini_set("babystep", "value", value);
    mksini_save();
    mksini_free();
    system("sync");
}

void get_mks_babystep() {
    mksini_load();
    mks_babystep_value = mksini_getstring("babystep", "value", "0.000");
    mksini_free();
}

void clear_cp0_image() {
    send_cmd_cp_close(tty_fd, "cp0");
    send_cmd_txt(tty_fd, "cp0_text", "");
    send_cmd_txt(tty_fd, "add", "");
    show_preview_gimage_completed = false;
    mks_file_parse_finished = false;
    file_metadata_simage.clear();
    file_metadata_gimage.clear();
}

void printer_set_babystep() {
    get_mks_babystep();
    ep->Send(json_run_a_gcode("SET_GCODE_OFFSET Z=" + mks_babystep_value + " MOVE=0"));
}

int get_mks_fila_status() {
    mksini_load();
    mks_fila_status = mksini_getboolean("fila", "enable", 0);
    mksini_free();
    return mks_fila_status;
}

void set_mks_fila_status() {
    mksini_load();
    mksini_set("fila", "enable", std::to_string(mks_fila_status));
    mksini_save();
    mksini_free();
    system("sync");
}

void init_mks_status() {
    get_mks_net_status();
    get_mks_fila_status(); // CLL 每次启动获取断料检测状态
    get_mks_total_printed_time();
    get_mks_babystep();
    printer_set_babystep();
    get_mks_connection_method();
    //4.2.10 开机自动设置声音关、灯开、断料检测关
    ep->Send(json_run_a_gcode("beep_off\nSET_PIN PIN=caselight VALUE=1\nSET_FILAMENT_SENSOR SENSOR=fila ENABLE=0\n"));
}

void after_scan_refresh_page() {
    if (current_page_id == TJC_PAGE_WIFI_LIST_2) {
        get_ssid_list_pages();
        page_wifi_current_pages = 0;
        set_page_wifi_ssid_list(page_wifi_current_pages);
        refresh_page_wifi_list();
    }
}

int detect_disk() {
    if (access("/dev/sda", F_OK) == 0) {
        if (access("/dev/sda1", F_OK) == 0) {
            if (access("/home/mks/gcode_files/sda1", F_OK) != 0) {
                system("/usr/bin/systemctl --no-block restart makerbase-automount@sda1.service");
                sleep(1);
            }
        }
        // usleep(600000);
        
        return 0;
    } else {
        return -1;
    }
}

void set_printing_shutdown() {
    if (page_printing_shutdown_enable == false) {
        page_printing_shutdown_enable = true;
    } else {
        page_printing_shutdown_enable = false;
    }
}

void go_to_pid_working() {
    page_pid_finished = false;
    printer_idle_timeout_state = "Printing";
    page_to(TJC_PAGE_PID_WORKING);
    ep->Send(json_run_a_gcode("M303 E0 S220\n"));
}

void mks_get_version() {
    mksini_load_from_path("/root/xindi/version");
	mks_version_soc = mksini_getstring("version", "soc", "V1.0.0");
    mksversion_free();
    MKSLOG_GREEN("SOC Version：%s", mks_version_soc.c_str());
}

void wifi_save_config() {
    page_to(TJC_PAGE_WIFI_SAVE);
    mks_save_config();
    sleep(2);
    get_wlan0_status();
}

void disable_page_about_successed() {
    page_about_successed = true;
}

void finish_tjc_update() {
    if (access("/root/800_480.tft", F_OK) == 0) {
        system("mv /root/800_480.tft /root/800_480.tft.bak; sync");
    }
}

void filament_unload() {
    ep->Send(json_run_a_gcode("M109 S" + std::to_string(load_target) + "\n"));
    ep->Send(json_run_a_gcode("M603\n"));
}

void filament_load() {
    ep->Send(json_run_a_gcode("M109 S" + std::to_string(load_target) + "\n"));
    ep->Send(json_run_a_gcode("M604\n"));
}


int get_cal_printed_time(int print_time) {
    int printed_time = 0;
    printed_time = print_time / 60;
    return printed_time;
}

int get_mks_total_printed_time() {
    mksini_load();
    mks_total_printed_minutes = mksini_getint("total", "time", 0);
    mksini_free();
    return mks_total_printed_minutes;
}

void set_mks_total_printed_time(int printed_time) {
    mksini_load();
    std::cout << "######## " << printed_time << std::endl;
    mksini_set("total", "time", std::to_string(printed_time));
    mksini_save();
    mksini_free();
    system("sync");
}

void get_total_time() {
    ep->Send(json_get_job_totals());
}

void do_not_x_clear() {
    set_mks_total_printed_time(36000);
}

void do_x_clear() {
    set_mks_total_printed_time(0);
}

void level_mode_printing_set_target() {
    set_heater_bed_target(level_mode_printing_heater_bed_target);
    ep->Send(json_run_a_gcode("M190 S" + std::to_string(level_mode_printing_heater_bed_target)));
    set_extruder_target(level_mode_printing_extruder_target);
    ep->Send(json_run_a_gcode("M109 S" + std::to_string(level_mode_printing_extruder_target)));
}

void level_mode_printing_print_file() {
    start_printing("LEVEL_PRINTING.gcode");
}

void update_finished_tips() {
    sleep(5);
    system("sync");
    system("systemctl restart makerbase-client.service");
}

bool get_mks_oobe_enabled() {
    mksini_load();
    mks_oobe_enabled = mksini_getboolean("oobe", "enable", 0);
    mksini_free();
    return mks_oobe_enabled;
}

void set_mks_oobe_enabled(bool enable) {
    mksini_load();
    mksini_set("oobe", "enable", std::to_string(enable));
    mksini_save();
    mksini_free();
    system("sync");
}

//4.2.4 CLL 修改调平
void pre_open_auto_level_init() {
    page_to(TJC_PAGE_OPEN_LEVELINIT);
    std::cout << "然后进入到这里" << std::endl;
    auto_level_dist = 0.05;
    auto_level_finished = false;
    if (start_pre_auto_level == false) {
        printer_idle_timeout_state = "Printing";
    }
    get_mks_babystep();
    previous_zoffset = mks_babystep_value;
    float z;
    z = std::stof(previous_zoffset);
    z = z + 0.45;
    previous_zoffset = std::to_string(z);
    ep->Send(json_run_a_gcode("SET_GCODE_OFFSET Z=0 MOVE=0\n"));
    ep->Send(json_run_a_gcode("M4029\n"));
    ep->Send(json_run_a_gcode("SET_GCODE_OFFSET Z=0.15 MOVE=1\n"));// CLL 修改每次开始调平时，基准值为0
}

void open_go_to_syntony_move() {
    page_syntony_finished = false;
    printer_idle_timeout_state = "Printing";
    page_to(TJC_PAGE_OPEN_SYNTONY);
    ep->Send(json_run_a_gcode("M901\n"));
}

void move_motors_off() {
    ep->Send(json_run_a_gcode("M84\n"));
}

void open_syntony_finish() {
    MKSLOG_RED("Printer ide_timeout state: %s", printer_idle_timeout_state.c_str());
    MKSLOG_BLUE("Printer webhooks state: %s", printer_webhooks_state.c_str());
    if (page_syntony_finished == false) {
        page_syntony_finished = true;
    }

    if (printer_idle_timeout_state == "Ready" && printer_webhooks_state == "ready") {
        system("sync");
        sleep(5);
        init_mks_status();
        auto_level_finished = true;
        page_to(TJC_PAGE_FILAMENT_VIDEO_1);
    }
}

void open_set_print_filament_target() {
    if (0 == printer_extruder_target) {
        get_mks_extruder_target();
        set_extruder_target(mks_extruder_target);
    } else {
        set_extruder_target(0);
    }
}

void open_set_filament_extruder_target(bool positive) {
    get_mks_extruder_target();
    printer_filament_extruder_target = mks_extruder_target;
    if (positive == true) {
        printer_filament_extruder_target += 3;
    } else {
        printer_filament_extruder_target -= 3;
    }

    if (printer_filament_extruder_target < 0) {
        printer_filament_extruder_target = 0;
        set_extruder_target(0);
        set_mks_extruder_target(0);
    } else {
        set_extruder_target(printer_filament_extruder_target);
        set_mks_extruder_target(printer_filament_extruder_target);
    }
}

void refresh_page_filament_video_2() {
    if (printer_extruder_target == 0) {
        send_cmd_pco(tty_fd, "t4", "65535");
        send_cmd_pco(tty_fd, "t3", "65535");
    } else {
        send_cmd_pco(tty_fd, "t4", "63488");
        send_cmd_pco(tty_fd, "t3", "63488");
    }

    send_cmd_txt(tty_fd, "t4", std::to_string(printer_extruder_temperature));
    send_cmd_txt(tty_fd, "t3", std::to_string(printer_extruder_target));
}

void open_start_extrude() {
    //ep->Send(json_run_a_gcode("M83\nG1 E10 F300\n"));
    //2023.5.8 CLL 修改开机进料每次20mm
    ep->Send(json_run_a_gcode("M83\nG1 E20 F300\n"));
}

void open_null_2_enter() {
    printer_idle_timeout_state = "Printing";
	//2023.4.27 修改开机调平前平台移动代码
    //ep->Send(json_run_a_gcode("G91\nG1 Z-100 F600\nG1 X-100 Y-100 F1200\nG90"));
    //ep->Send(json_run_a_gcode("G90\nG1 Z50 F300\n G1 X50 Y50 F1200"));
}

void open_more_level_finish() {
    ep->Send(json_run_a_gcode("G28\n"));
    set_extruder_target(0);
    set_mks_oobe_enabled(false);            // 完成关闭OOBE的操作
    page_to(TJC_PAGE_MAIN);
}

void open_down_50() {
    ep->Send(json_run_a_gcode("G28\nG1 Z50\n"));
}

void close_mcu_port() {
    ep->Send(json_run_a_gcode("CLOSE_MCU_PORT\n"));
}

//2023.5.11 CLL 打印前发送"PRINT_START_QD"指令
void print_start() {
    //4.3.11 CLL 打印开始前不再发送"PRINT_START_QD"
    //ep->Send(json_run_a_gcode("PRINT_START_QD\n"));
}

int get_mks_net_status() {
    mksini_load();
    mks_ethernet = mksini_getboolean("mks_ethernet", "enable", 0);
    mksini_free();
    return mks_ethernet;
}

void set_mks_net_wifi() {
    mksini_load();
    mksini_set("mks_ethernet", "enable", std::to_string(0));
    mksini_save();
    mksini_free();
}

void set_mks_net_eth() {
    mksini_load();
    mksini_set("mks_ethernet", "enable", std::to_string(1));
    mksini_save();
    mksini_free();
}

//2023.5.8 CLL 修改自动调平页面跳转
void refresh_page_level_null_1() {
    send_cmd_txt(tty_fd, "t0", std::to_string(printer_heater_bed_temperature) + "/");
    send_cmd_val(tty_fd, "n0", std::to_string(printer_heater_bed_target));
    if (printer_heater_bed_target > 0) {
        send_cmd_picc(tty_fd, "b2", "400");
        send_cmd_picc2(tty_fd, "b2", "398");
        send_cmd_pco(tty_fd, "t0", "63488");
        send_cmd_pco(tty_fd, "n0", "63488");
    } else {
        send_cmd_picc(tty_fd, "b2", "401");
        send_cmd_picc2(tty_fd, "b2", "397");
        send_cmd_pco(tty_fd, "t0", "65535");
        send_cmd_pco(tty_fd, "n0", "65535");
    }
}

//2023.5.8 CLL 修改自动调平页面跳转
void set_auto_level_heater_bed_target(bool positive) {
    get_mks_heater_bed_target();
    printer_auto_level_heater_bed_target = mks_heater_bed_target;
    if (positive == true) {
        printer_auto_level_heater_bed_target += 3;
    }else {
        printer_auto_level_heater_bed_target -= 3;
    }
    if (printer_auto_level_heater_bed_target >120) {
        printer_auto_level_heater_bed_target =120;
    }
    if (printer_auto_level_heater_bed_target < 0) {
        printer_auto_level_heater_bed_target = 0;
    }
    set_heater_bed_target(printer_auto_level_heater_bed_target);
    set_mks_heater_bed_target(printer_auto_level_heater_bed_target);
}

//2023.5.8 CLL 修复网页打印图片显示bug
void clear_previous_data() {
    sdcard_reset_file();
    clear_cp0_image();
    clear_page_preview();
    show_preview_complete = false;
    printing_keyboard_enabled = false;
    jump_to_print = false;
}

//2023.5.8 CLL 报错弹窗
void detect_error() {
    //2023.5.11 CLL 修复页面跳转bug
    switch (current_page_id)
    {
    case TJC_PAGE_PRINTING:
    case TJC_PAGE_PRINT_ZOFFSET:
    case TJC_PAGE_PRINT_FILAMENT:
    case TJC_PAGE_GCODE_ERROR:
        break;
    
    default:
        if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
            //4.2.5 新增息屏功能
            if (previous_caselight_value == true) {
                led_on_off();
                previous_caselight_value = false;
            }
            page_to(TJC_PAGE_DETECT_ERROR);
            send_cmd_txt(tty_fd, "t0", output_console);
        }
        break;
    }
}

//2023.5.11 CLL 修改开机引导自动调平页面跳转
void refresh_page_open_heater_bed() {
    send_cmd_txt(tty_fd, "t0", std::to_string(printer_heater_bed_temperature) + "/");
    send_cmd_val(tty_fd, "n0", std::to_string(printer_heater_bed_target));
    if (printer_heater_bed_target > 0) {
        send_cmd_picc(tty_fd, "b2", "409");
        send_cmd_picc2(tty_fd, "b2", "407");
        send_cmd_pco(tty_fd, "t0", "63488");
        send_cmd_pco(tty_fd, "n0", "63488");
    } else {
        send_cmd_picc(tty_fd, "b2", "410");
        send_cmd_picc2(tty_fd, "b2", "406");
        send_cmd_pco(tty_fd, "t0", "65535");
        send_cmd_pco(tty_fd, "n0", "65535");
    }
}

//2.1.2 CLL 修改开机引导流程
void open_heater_bed_up() {
    page_to(TJC_PAGE_OPEN_VIDEO_4);
    //4.2.10 CLL 修改开机引导平台上移距离
    ep->Send(json_run_a_gcode("G91\nG1 Z-30 F600\nG1 X-30 Y-30 F1200\nG90\n"));
    ep->Send(json_run_a_gcode("SET_KINEMATIC_POSITION Z=130\n"));
}

//2.1.2 CLL 新增热床调平
void bed_leveling_switch(bool positive) {
    if (positive == true) {
        printer_bed_leveling_state = true;
        ep->Send(json_run_a_gcode("G31"));
    }else {
        printer_bed_leveling_state = false;
        ep->Send(json_run_a_gcode("G32"));
    }
}

//2.1.2 CLL 每次打印完成或取消保存一次zoffset值
void save_current_zoffset() {
    std::string z_offset = std::to_string(printer_gcode_move_homing_origin[2]);
    z_offset = z_offset.substr(0, z_offset.find(".") + 4);
    switch (current_page_id)
    {
    case TJC_PAGE_AUTO_MOVE:
    case TJC_PAGE_OPEN_LEVELING:
        printer_idle_timeout_state = "Printing";
        float z;
        z = std::stof(z_offset);
        z = z - 0.15;
        if (z > -5 && z < 5) { // CLL 修改zoffset只会保存-5 ~ 5 的值
            z_offset = std::to_string(z);
            mks_babystep_value = z_offset;
            set_mks_babystep(mks_babystep_value);
            MKSLOG_RED("当前zoffset值保存为%s",mks_babystep_value.c_str());
        }
        break;

    //4.2.5 CLL 新增设置Z轴偏移页面
    case TJC_PAGE_SET_ZOFFSET_2:
        printer_idle_timeout_state = "Printing";
        page_to(TJC_PAGE_SAVING);
        z = std::stof(z_offset);
        z = z - 0.15;
        if (z > -5 && z < 5) { // CLL 修改zoffset只会保存-5 ~ 5 的值
            z_offset = std::to_string(z);
            mks_babystep_value = z_offset;
            set_mks_babystep(mks_babystep_value);
            MKSLOG_RED("当前zoffset值保存为%s",mks_babystep_value.c_str());
        }
        ep->Send(json_run_a_gcode("G0 X0 Y0 Z50 F5000\n"));
        ep->Send(json_run_a_gcode("SAVE_CONFIG\n"));
        break;
    
    default:
        if (z_offset != mks_babystep_value && z_offset.find("0.000") != -1) {
            if (std::stof(z_offset) > -5 && std::stof(z_offset) < 5) {
                mks_babystep_value = z_offset;
                set_mks_babystep(mks_babystep_value);
                MKSLOG_RED("当前zoffset值保存为:%s",mks_babystep_value.c_str());
            }
        }
        break;
    }
}

//2.1.2 CLL 打印前判断耗材种类并弹窗
void check_filament_type() {
    std::string filament_type;
    if (file_metadata_filament_type != "") {
        filament_type = file_metadata_filament_type;
    } else {
        filament_type = file_metadata_filament_name;
    }
    std::transform(filament_type.begin(), filament_type.end(), filament_type.begin(), tolower);
    MKSLOG_YELLOW("filament_type : %s",filament_type.c_str());
    //4.2.3 CLL 耗材确认弹窗新增不再提示按钮
    if ((filament_type.find("pla") != -1 || filament_type.find("petg") != -1) && preview_pop_1_on == true) {
        page_to(TJC_PAGE_PREVIEW_POP_1);
    }else if (filament_type.find("abs") != -1 && preview_pop_2_on == true) {
        page_to(TJC_PAGE_PREVIEW_POP_2);
    }else {
        page_to(TJC_PAGE_PRINTING);
    }
}

//2.1.2 CLL 新增退料界面
void refresh_page_unloading() {
    if (printer_idle_timeout_state == "Ready") {
        page_to(TJC_PAGE_UNLOAD_FINISH);
    }
}

void refresh_page_loading() {
    if (printer_idle_timeout_state == "Ready") {
        page_to(TJC_PAGE_LOAD_FINISH);
    }
}

void refresh_page_preview_pop() {
    if (printer_print_stats_state == "standby") {
        page_to(TJC_PAGE_STOPPING);
    }
    if (printer_print_stats_state == "error") {
        page_to(TJC_PAGE_GCODE_ERROR);
        send_cmd_txt(tty_fd, "t0", "gcode error:" + output_console);
    }

    if (filament_switch_sensor_fila_enabled == true) {
        if (filament_switch_sensor_fila_filament_detected == false) {
            printer_ready = false;
            page_to(TJC_PAGE_PRINT_F_POP);
            set_print_pause();
            clear_page_printing_arg();
        }
    }
}

//4.2.1 CLL 修复无法读取文件名中带空格文件
std::string replaceCharacters(const std::string& path, const std::string& searchChars, const std::string& replacement) {
    std::string result = path;
    for (char c : searchChars) {
        std::size_t found = result.find(c);
        while (found != std::string::npos) {
            result.replace(found, 1, replacement);
            found = result.find(c, found + replacement.length());
        }
    }
    return result;
}

//4.2.5 CLL 修复UI按下效果
void refresh_page_open_level() {
    if (auto_level_dist == (float)0.01) {
        send_cmd_picc(tty_fd, "b3", "260");
        send_cmd_picc2(tty_fd, "b3", "284");
        send_cmd_picc(tty_fd, "b4", "260");
        send_cmd_picc2(tty_fd, "b4", "283");
        send_cmd_picc(tty_fd, "b5", "260");
        send_cmd_picc2(tty_fd, "b5", "283");
        send_cmd_picc(tty_fd, "b6", "260");
        send_cmd_picc2(tty_fd, "b6", "283");
    } else if (auto_level_dist == (float)0.05) {
        send_cmd_picc(tty_fd, "b3", "261");
        send_cmd_picc2(tty_fd, "b3", "283");
        send_cmd_picc(tty_fd, "b4", "261");
        send_cmd_picc2(tty_fd, "b4", "284");
        send_cmd_picc(tty_fd, "b5", "261");
        send_cmd_picc2(tty_fd, "b5", "283");
        send_cmd_picc(tty_fd, "b6", "261");
        send_cmd_picc2(tty_fd, "b6", "283");
    } else if (auto_level_dist == (float)0.1) {
        send_cmd_picc(tty_fd, "b3", "262");
        send_cmd_picc2(tty_fd, "b3", "283");
        send_cmd_picc(tty_fd, "b4", "262");
        send_cmd_picc2(tty_fd, "b4", "283");
        send_cmd_picc(tty_fd, "b5", "262");
        send_cmd_picc2(tty_fd, "b5", "284");
        send_cmd_picc(tty_fd, "b6", "262");
        send_cmd_picc2(tty_fd, "b6", "283");
    } else if (auto_level_dist == (float)1) {
        send_cmd_picc(tty_fd, "b3", "263");
        send_cmd_picc2(tty_fd, "b3", "283");
        send_cmd_picc(tty_fd, "b4", "263");
        send_cmd_picc2(tty_fd, "b4", "283");
        send_cmd_picc(tty_fd, "b5", "263");
        send_cmd_picc2(tty_fd, "b5", "283");
        send_cmd_picc(tty_fd, "b6", "263");
        send_cmd_picc2(tty_fd, "b6", "284");
    }
}

//4.2.7 CLL 新增恢复出厂设置功能
void restore_config() {
    system("rm /home/mks/gcode_files/.cache/*");
    system("curl -X POST http://127.0.0.1:7125/server/history/reset_totals");
    system("curl -X DELETE 'http://127.0.0.1:7125/server/history/job?all=true'");
    system("rm /home/mks/gcode_files/.cache/*\n");
    system("cp /home/mks/klipper_config/config.mksini.bak /home/mks/klipper_config/config.mksini\n");
    page_to(TJC_PAGE_MAIN);
}

void refresh_page_restoring() {
    if (printer_idle_timeout_state == "Ready" && printer_webhooks_state == "ready") {
        page_to(TJC_PAGE_MAIN);
    }
}

//4.2.10 CLL 新增输出日志文件功能
void print_log() {
    if (detect_disk() == -1) {
        page_to(TJC_PAGE_PRINT_LOG_F);
    } else {
        system("mkdir /home/mks/gcode_files/sda1/QD_Log");
        system("cp /home/mks/klipper_logs/klippy.log /home/mks/gcode_files/sda1/QD_Log/klippy.log\n");
        system("cp /home/mks/klipper_logs/moonraker.log /home/mks/gcode_files/sda1/QD_Log/moonraker.log\n");
        page_to(TJC_PAGE_PRINT_LOG_S);
    }
}

//4.2.10 CLL 修改断料检测开关逻辑
void filament_sensor_switch(bool status) {
    if (status == true) {
        ep->Send(json_run_a_gcode("SET_FILAMENT_SENSOR SENSOR=fila ENABLE=1\n"));
    } else if (status == false) {
        ep->Send(json_run_a_gcode("SET_FILAMENT_SENSOR SENSOR=fila ENABLE=0\n"));
    }
}

//4.2.10 CLL 新增共振补偿超时强制跳转
void send_gcode(std::string command) {
    ep->Send(json_run_a_gcode(command));
}

void refresh_page_pre_heating_2() {
    send_cmd_txt(tty_fd, "t1", "(" + std::to_string(printer_extruder_temperature) + "/" + std::to_string(printer_extruder_target) + ")");
    if (printer_extruder_temperature == load_target) {
        if (load_mode == true) {
            page_to(TJC_PAGE_LOADING);
        } else {
            page_to(TJC_PAGE_UNLOADING);
        }
    }
}

void go_to_showqr() {

    send_cmd_vis(tty_fd, "gm0", "1");
    if (strcmp(status_result.wpa_state, "COMPLETED") == 0 || sys_set.internet_enabled)
    {
        send_cmd_cp_close(tty_fd, "cp0");
        refresh_files_list_picture("/root/QIDILink-client/qrcode.png", 368, 0);
        send_cmd_vis(tty_fd, "gm0", "0");
        send_cmd_vis(tty_fd, "t0", "1");
        MKSLOG_GREEN("QR code refresh successful, please scan the code to log in!");
    }
    else
    {
        send_cmd_vis(tty_fd, "gm0", "0");
        page_to(TJC_PAGE_QIDI_LINK_LOG_FAIL);
    }
}

void refresh_files_list_picture(std::string path, int pixel, int i) {
    file_metadata_simage.clear();
    file_metadata_gimage.clear();
    output_imgdata(path, pixel);
    std::ifstream infile("/home/mks/tjc"); // 打开文件
    if (!infile) {
        std::cerr << "无法打开文件 " << "/home/mks/tjc" << std::endl;
        return;
    }
    std::stringstream buffer;
    buffer << infile.rdbuf();
    file_metadata_gimage = buffer.str();
    infile.close();
    send_cmd_cp_close(tty_fd, "cp"+std::to_string(i));
    if (file_metadata_gimage != "") {
        std::cout << "刷文件图" << std::endl;
        int num = 1024;
        int len = file_metadata_gimage.length();
        int end = num;
        std::string g;
        for (int start = 0; start < len;) {
            if (end > len) {
                g = file_metadata_gimage.substr(start, len - start);
                tcdrain(tty_fd);
                send_cmd_cp_image(tty_fd, "cp" + std::to_string(i), g);
                break;
            }
            g = file_metadata_gimage.substr(start, num);
            start = end;
            end = end + num;
            tcdrain(tty_fd);
            send_cmd_cp_image(tty_fd, "cp" + std::to_string(i), g);
        }
    }
    send_cmd_vis(tty_fd, "cp" + std::to_string(i), "1");
}

void go_to_server_set(int n) {
    current_server_page = n;
    total_server_count = 0;
    serverConfigs.clear();
    if (sys_set.internet_enabled && strcmp(status_result.wpa_state, "COMPLETED") == 0) {
        page_to(TJC_PAGE_SEARCH_SERVER);
        update_server(0);
    }
    app_cli.update_device();
    page_to(TJC_PAGE_SERVER_SET); 
    refresh_page_server_set();
}

void get_mks_selected_server() {
    mksini_load();
    selected_server = mksini_getstring("app_server", "name", "");
    mksini_free();
    std::cout << selected_server << std::endl;
}

void updateServerConfig(std::vector<std::string> &lines, const Server_config &config)
{
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (lines[i] == "[app_server]")
        {
            // Check if it has reached the end of the file
            if (i + 1 < lines.size())
            {
                // Update the next line directly without considering the number of spaces
                lines[i + 1] = "name = " + config.name;
                return;
            }
        }
    }

    // If the [app_derver] section is not found, add it at the end of the file
    lines.push_back("[app_server]");
    lines.push_back("name = " + config.name);
}

void update_server(int choice)
{
    // Connect to the server to retrieve the JSON file
    if (choice == 0)
    {
        get_mks_selected_server();
        std::string server_for_command = selected_server.empty() ? "aws" : selected_server;
        std::string url =  server_for_command == "aws"? "https://api.qidi3dprinter.com/code/servers" :"https://api2.qidi3dprinter.com/code/servers";
        Http http = Http::get(url);
        http.header("accept", "application/json")
            .timeout_connect(30)
            .timeout_max(30)
            .on_complete([&](std::string body, unsigned) {
                std::cout << "server :" << body << std::endl;
                try {
                    json j = json::parse(body);
                    total_server_count = 0;
                    int id = 1;
                    for (const auto& server : j["servers"]) {
                        Server_config config = {"", server["name"]};
                        serverConfigs[id] = config;
                        std::cout << id << ": " << config.name << std::endl;
                        id++;
                        total_server_count++;
                    }
                }
                catch (...) {
                    std::cout << "parse error "<< std::endl;
                    return;
                }
            })
            .on_error([&](std::string body, std::string error, unsigned int status) {
                std::cout << "http error :"<< error << "  body :"<< body << "   status : "  <<  status <<std::endl;
                std::cerr << "Failed to download server list. HTTP error or connection issue." << std::endl;
                return;
        }).perform();
    } else {
        if (serverConfigs.find(choice) == serverConfigs.end())
        {
            std::cout << "Invalid server choice.\n";
            return;
        }

        Server_config config = serverConfigs[choice];

        if (app_cli.update_device(config.name) == SUCCESS)
        {
            no_error = true;
            // Update config.mksini
            mksini_update("app_server", "name", config.name);

            // Update memory
            sys_set.server = config.name;
            MKSLOG("sys_set.server = %s", sys_set.server.c_str());

            // Stop QIDILink-client.service
            std::cout << "Stopping QIDILink-client.service...\n";
            system("sudo systemctl stop QIDILink-client.service");

            // Update frpc.json
            app_cli.write_frpc_json();

            // Restart QIDILink-client.service
            if (sys_set.internet_enabled)
            {
                MKSLOG("Restarting QIDILink-client.service...");
                system("sudo systemctl start QIDILink-client");
            }

            MKSLOG("Server changed to %s successfully.", config.name.c_str());
        } else {
            no_error = false;
            page_to(TJC_PAGE_SERVER_SET);
            MKSLOG("Server changed to %s Failded.", config.name.c_str());
        }
    }
}

void refresh_page_server_set() {

    // Refresh the IP at the top of the page
    if (sys_set.ethernet == 0) 
    {
        send_cmd_txt(tty_fd, "t0", status_result.ip_address);
    }
    else 
    {
        std::string local_ip = get_eth0_ip();
        if (local_ip.find(":") != -1)
            send_cmd_txt(tty_fd, "t0", "");
        else
            send_cmd_txt(tty_fd, "t0", local_ip);
    }

    // Set the display effect of the up and down page buttons
    if (current_server_page == 0) {
        send_cmd_picc(tty_fd, "b0", "104");
        send_cmd_picc2(tty_fd, "b0", "322");
    } else {
        send_cmd_picc(tty_fd, "b0", "103");
        send_cmd_picc2(tty_fd, "b0", "323");
    }

    if ((current_server_page + 1) * 5 >= total_server_count) {
        send_cmd_picc(tty_fd, "b1", "104");
        send_cmd_picc2(tty_fd, "b1", "322");
    } else {
        send_cmd_picc(tty_fd, "b1", "103");
        send_cmd_picc2(tty_fd, "b1", "323");
    }

    for (int i = 0; i < 5; i++)
    {
        if (i + current_server_page * 5 + 1 > total_server_count)
            break;
        send_cmd_txt(tty_fd, "servert" + std::to_string(i + 1), serverConfigs[1 + i + current_server_page * 5].name);
        get_mks_selected_server();  // refresh the value of selected_server
        if (selected_server == serverConfigs[1 + i + current_server_page * 5].name) {
            send_cmd_picc(tty_fd, "server" + std::to_string(i + 1), "463");
            send_cmd_picc2(tty_fd, "server" + std::to_string(i + 1), "461");
        } else {
            send_cmd_picc(tty_fd, "server" + std::to_string(i + 1), "462");
            send_cmd_picc2(tty_fd, "server" + std::to_string(i + 1), "460");
        }
    }
}

void get_mks_connection_method() {
    mksini_load();
    mks_connection_method = mksini_getint("app_connection", "method", 0);
    mksini_free();
}

void set_mks_connection_method(int target) {
    mksini_load();
    std::cout << "######## " << target << std::endl;
    mksini_set("app_connection", "method", std::to_string(target));
    mksini_save();
    mksini_free();
    system("sync");
}

std::string run_python_code(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

void refresh_page_show_ip() {
    
    if (app_cli.device_code != ""){
        send_cmd_txt(tty_fd, "t2", app_cli.device_code);
    }

    if (sys_set.ethernet == 0) {
        send_cmd_txt(tty_fd, "t0", status_result.ip_address);
        send_cmd_picc(tty_fd, "b0", "454");
        send_cmd_picc2(tty_fd, "b0", "453");
        send_cmd_pic(tty_fd, "b[0]", "453");
    } else {
        std::string local_ip = get_eth0_ip();
        if (local_ip.find(":") != -1)
        {
            send_cmd_txt(tty_fd, "t0", "");
        } else {
            send_cmd_txt(tty_fd, "t0", local_ip);
        }
        send_cmd_picc(tty_fd, "b0", "473");
        send_cmd_picc2(tty_fd, "b0", "472");
        send_cmd_pic(tty_fd, "b[0]", "472");
    }

    if (sys_set.internet_enabled)
    {
        send_cmd_tsw(tty_fd, "b1", "1");
        send_cmd_tsw(tty_fd, "b3", "1");
        send_cmd_tsw(tty_fd, "b4", "1");
        send_cmd_picc(tty_fd, "b1", "453");
        send_cmd_picc(tty_fd, "b3", "453");
        send_cmd_picc(tty_fd, "b4", "453");
        send_cmd_picc(tty_fd, "b5", "455");
        send_cmd_picc2(tty_fd, "b5", "454");
        send_cmd_pco(tty_fd, "b3", "65535");
        send_cmd_pco(tty_fd, "b4", "65535");
        send_cmd_pco(tty_fd, "t1", "65535");
        send_cmd_pco(tty_fd, "t2", "65535");
    }
    else{
        send_cmd_vis(tty_fd, "gm0", "0");
        send_cmd_tsw(tty_fd, "b1", "0");
        send_cmd_tsw(tty_fd, "b3", "0");
        send_cmd_tsw(tty_fd, "b4", "0");
        send_cmd_picc(tty_fd, "b1", "454");
        send_cmd_picc(tty_fd, "b3", "454");
        send_cmd_picc(tty_fd, "b4", "454");
        send_cmd_picc(tty_fd, "b5", "456");
        send_cmd_picc2(tty_fd, "b5", "455");
        send_cmd_pco(tty_fd, "b3", "33808");
        send_cmd_pco(tty_fd, "b4", "33808");
        send_cmd_pco(tty_fd, "t1", "33808");
        send_cmd_pco(tty_fd, "t2", "33808");
    }
}

void refresh_device_code() {
    send_cmd_vis(tty_fd, "b1", "0");
    send_cmd_vis(tty_fd, "gm0", "1");
    if (app_cli.update_device() == 0)
    {
        if (app_cli.device_code != "")
        {
            send_cmd_txt(tty_fd, "t2", app_cli.device_code);
            send_cmd_vis(tty_fd, "gm0", "0");
            send_cmd_vis(tty_fd, "b1", "1");
        }
        return;
    }
    send_cmd_vis(tty_fd, "gm0", "0");
    send_cmd_vis(tty_fd, "b1", "1");
}

void login_successed_page()
{
    send_cmd_vis(tty_fd, "t0", "1");
    send_cmd_vis(tty_fd, "t1", "1");
    send_cmd_vis(tty_fd, "b1", "1");
}

void refresh_lan_model_frpc()
{
    if (!sys_set.internet_enabled)
    {
        system("systemctl stop QIDILink-client");
    }
}