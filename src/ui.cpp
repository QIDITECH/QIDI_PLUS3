#include "../include/ui.h"
#include "../include/send_msg.h"
#include "../include/mks_log.h"
#include "../include/event.h"
#include "../include/mks_file.h"
#include "../include/mks_preview.h"
#include "../include/mks_update.h"
#include "../include/mks_wpa_cli.h"

#include "../include/MakerbaseNetwork.h"
#include "../include/MakerbaseSerial.h"
#include "../include/MakerbaseWiFi.h"

#include <stack>

// #define DEFAULT_DIR "gcodes/sda1/"
#define DEFAULT_DIR "gcodes/"

int tty_fd = -1;

// 补偿值
extern bool fresh_page_set_zoffset_data;

// update
extern bool detected_soc_data;
extern bool detected_mcu_data;
extern bool detected_ui_data;

// bool start_to_printing = false;
// preview
extern std::vector<std::string> gimage;
extern std::vector<std::string> simage;

extern std::string str_gimage;

// extern std::vector<std::string> gimage_temp;
// extern std::vector<std::string> simage_temp;
// preview

// wifi start
extern int page_wifi_ssid_list_pages;
extern int page_wifi_current_pages;
// wifi end

extern int page_files_pages;
extern int page_files_current_pages;
extern int page_files_folder_layers;

extern std::string page_files_list_name[8];                                     // 文件列表显示文件名称
extern std::string page_files_list_show_name[8];                                // 文件列表名称
extern std::string page_files_list_show_type[8];   

extern std::stack<std::string> page_files_path_stack;          // 路径栈
extern std::string page_files_root_path;                       // Klippy根目录
extern std::string page_files_previous_path;                   // 之前的路径
extern std::string page_files_path;                            // 文件所在路径
extern std::string page_files_print_files_path;                // 要打印的文件路径

int current_page_id;                    // 当前页面的id号
int previous_page_id;                   // 上一页面的id号
int next_page_id;                       // 下一页面的id号

int event_id;
int page_id;
int widget_id;
int type_id;

int level_mode;                         // 调平模式

bool show_preview_complete = false;
bool show_preview_gimage_completed = false;

bool printing_keyboard_enabled = false;
// bool filament_keyboard_enabled = false;
bool auto_level_button_enabled = true;
bool manual_level_button_enabled = true;

bool printing_wifi_keyboard_enabled = false;

extern std::string printer_idle_timeout_state;
extern std::string printer_print_stats_state;

// 打印调平
int level_mode_printing_extruder_target = 0;
int level_mode_printing_heater_bed_target = 0;

bool level_mode_printing_is_printing_level = false;

bool page_wifi_list_ssid_button_enabled[5] = {false};

// page print filament
bool page_print_filament_extrude_restract_button = false;
bool page_filament_extrude_button = false;
bool page_filament_unload_button = false;

// page reset
// bool page_reset_to_about = false;
extern std::string mks_page_internet_ip;

extern bool mks_file_parse_finished;

//2023.4.20 打印过文件标红
std::string printed_file_path;

//2023.4.26-1 修改退料流程
extern int printer_extruder_temperature;
extern int printer_extruder_target;

//2023.5.11 CLL 修复页面跳转bug
extern bool printer_ready;

extern std::string printer_webhooks_state;

//2.1.2 CLL 新增热床调平
bool printer_bed_leveling_state;

//2.1.2 CLL 修复页面跳转bug
extern bool printer_ready;
extern std::string printer_webhooks_state;

void parse_cmd_msg_from_tjc_screen(char *cmd) {
    event_id = cmd[0];
    MKSLOG_BLUE("#########################%s", cmd);
    MKSLOG_RED("0x%x", cmd[0]);
    MKSLOG_RED("0x%x", cmd[1]);
    MKSLOG_RED("0x%x", cmd[2]);
    MKSLOG_RED("0x%x", cmd[3]);
    switch (event_id)
    {
    case 0x03:
        // MKSLOG_BLUE("读取到屏幕固件数据发生错误，进入到恢复屏幕的模式");
        break;

    case 0x05:
        std::cout << "可以开始发送数据了" << std::endl;
        // set_option(tty_fd, 921600, 8, 'N', 1);
        // set_option(tty_fd, 230400, 8, 'N', 1);
        download_to_screen();
        // 收到0x05消息，意味着可以开始发送串口屏幕的信息
        break;

    case 0x1a:
        std::cout << "变量名称无效" << std::endl;
        break;

    case 0x65:
        page_id = cmd[1];
        widget_id = cmd[2];
        type_id = cmd[3];
        tjc_event_clicked_handler(page_id, widget_id, type_id);
        break;

    case 0x66:
        break;

    case 0x67:
        break;

    case 0x68:
        break;
    
    case 0x70:
        MKSLOG_RED("0x%x", event_id);
        // page_id = cmd[1];
        // widget_id = cmd[2];
        // type_id = cmd[3];
        tjc_event_keyboard(cmd);
        break;

    case 0x71:
        page_id = cmd[1];
        tjc_event_setted_handler(cmd[1], cmd[2], cmd[3], cmd[4]);
        break;

    case 0x86:
    case 0x87:
    case 0x88:
    case 0x89:
        MKSLOG_RED("0x%x", event_id);
        break;

    case 0x91:
        current_page_id = TJC_PAGE_LOGO;
        page_to(TJC_PAGE_UPDATED);
        break;

    case 0xfd:
        break;
    case 0xfe:
        set_option(tty_fd, 230400, 8, 'N', 1);
        download_to_screen();
        break;

    case 0xff:
        MKSLOG_RED("0x%x", event_id);
        break;
    
    default:
        break;
    }
    cmd = NULL;
}

void page_to(int page_id) {
    previous_page_id = current_page_id;
    current_page_id = page_id;
    send_cmd_page(tty_fd, std::to_string(page_id));
}

void tjc_event_clicked_handler(int page_id, int widget_id, int type_id) {
    std::cout << "+++++++++++++++++++" << page_id << std::endl;
    std::cout << "+++++++++++++++++++" << widget_id << std::endl;
    std::cout << "+++++++++++++++++++" << type_id << std::endl;
    switch (page_id)
    {
    // 开箱第一项
    case TJC_PAGE_OPEN_LANGUAGE:
        switch (widget_id)
        {
        case TJC_PAGE_OPEN_LANGUAGE_NEXT:
            page_to(TJC_PAGE_OPEN_VIDEO_1);
            break;
        
        //2023.4.20 开机引导跳过
        case TJC_PAGE_OPEN_LANGUAGE_SKIP:
            //2023.5.12 CLL 开机引导跳过确认弹窗
            page_to(TJC_PAGE_OPEN_POP_1);
            //page_to(TJC_PAGE_MAIN);
            //set_mks_oobe_enabled(false);
            break;

        default:
            break;
        }

        break;

    // 开箱第二项
    case TJC_PAGE_OPEN_VIDEO_1:
        switch (widget_id)
        {
        case TJC_PAGE_OPEN_VIDEO_1_PREVIOUS:
            // page_to(TJC_PAGE_OPEN_LANGUAGE);
            break;

        case TJC_PAGE_OPEN_VIDEO_1_RIGHT:
            page_to(TJC_PAGE_OPEN_VIDEO_2);
            break;

        default:
            break;
        }
        break;

    // 开箱第三项
    case TJC_PAGE_OPEN_VIDEO_2:
        switch (widget_id)
        {
        case TJC_PAGE_OPEN_VIDEO_2_PREVIOUS:
            // page_to(TJC_PAGE_OPEN_LANGUAGE);
            break;

        case TJC_PAGE_OPEN_VIDEO_2_RIGHT:
            page_to(TJC_PAGE_OPEN_VIDEO_3);
            break;
        
        default:
            break;
        }
        break;

    // 开机第四项
    case TJC_PAGE_OPEN_VIDEO_3:
        switch (widget_id)
        {
        case TJC_PAGE_OPEN_VIDEO_3_PREVIOUS:
            // page_to(TJC_PAGE_OPEN_LANGUAGE);
            break;

        case TJC_PAGE_OPEN_VIDEO_3_RIGHT:
            // page_to(TJC_PAGE_OPEN_VIDEO_4);
            // start_auto_level();
            //2023.5.11 CLL 修改开机引导自动调平页面跳转
            //page_to(TJC_PAGE_NULL_2);
            //2.1.2 CLL 修改开机引导流程
            //page_to(TJC_PAGE_OPEN_HEATER_BED);
            open_heater_bed_up();
            break;

        case TJC_PAGE_OPEN_VIDEO_3_EXTRUDE:
            open_start_extrude();
            break;

        default:
            break;
        }
        break;
    
    case TJC_PAGE_OPEN_VIDEO_4:
        switch (widget_id)
        {
        case TJC_PAGE_OPEN_VIDEO_4_PREVIOUS:
            // page_to(TJC_PAGE_OPEN_LANGUAGE);
            break;
        
        case TJC_PAGE_OPEN_VIDEO_4_NEXT:
            // page_to(TJC_PAGE_OPEN_LEVELINIT);
            // start_pre_auto_level = false;
            // start_auto_level();
            /*2.1.2 CLL 修改开机引导流程
            if (printer_idle_timeout_state == "Ready") {
                pre_open_auto_level_init();
            } else {

            }
            */
            page_to(TJC_PAGE_OPEN_HEATER_BED);
            break;

        default:
            break;
        }
        break;

    // 开机第五项
    case TJC_PAGE_OPEN_LEVELINIT:
        switch (widget_id)
        {
        default:
            break;
        }
        break;

    case TJC_PAGE_OPEN_LEVELING:
        break;

    // 开机第六项
    case TJC_PAGE_OPEN_LEVEL:
        switch (widget_id)
        {
        case TJC_PAGE_OPEN_LEVEL_001:
            set_auto_level_dist(0.01);
            break;

        case TJC_PAGE_OPEN_LEVEL_005:
            set_auto_level_dist(0.05);
            break;

        case TJC_PAGE_OPEN_LEVEL_01:
            set_auto_level_dist(0.1);
            break;

        case TJC_PAGE_OPEN_LEVEL__1:
            set_auto_level_dist(1);
            break;

        case TJC_PAGE_OPEN_LEVEL_UP:
            start_auto_level_dist(false);
            break;

        case TJC_PAGE_OPEN_LEVEL_DOWN:
            start_auto_level_dist(true);
            break;

        case TJC_PAGE_OPEN_LEVEL_ENTER:
            page_to(TJC_PAGE_OPEN_LEVELING);
            start_auto_level();
            break;

        default:
            break;
        }
        break;

    // 开机第七项
    case TJC_PAGE_OPEN_LEVELED:
        switch (widget_id)
        {
        case TJC_PAGE_OPEN_LEVELED_FINISH:
            //4.2.1 CLL 修复卡在自动调平页面
            //if (auto_level_button_enabled == true) {
                auto_level_button_enabled = false;
                // page_to(TJC_PAGE_OPEN_SYNTONY);
                open_go_to_syntony_move();
            //}
            
            // std::cout << "共振补偿按下按钮" << std::endl;
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_NULL_2:
        switch (widget_id)
        {
        case TJC_PAGE_NULL_2_BACK:
            // page_to(TJC_PAGE_OPEN_VIDEO_3);
            break;

        case TJC_PAGE_NULL_2_ENTER:
            //2023.4.27 防止开机引导卡死
            //2023.4.28 删除"取出垫片"界面
            //page_to(TJC_PAGE_OPEN_VIDEO_4);
            page_to(TJC_PAGE_OPEN_LEVELINIT);
            open_null_2_enter();
            pre_open_auto_level_init();
            break;

        default:
            break;
        }
        break;

    // 开机第九项
    case TJC_PAGE_FILAMENT_VIDEO_1:
        switch (widget_id)
        {
        case TJC_PAGE_FILAMENT_VIDEO_1_RIGHT:
            //2023.5.13 CLL 防止点击过快报错
            if (printer_idle_timeout_state == "Ready" ||printer_idle_timeout_state == "Idle") {
                open_down_50();
                page_to(TJC_PAGE_FILAMENT_VIDEO_2);
            }
            break;
        default:
            break;
        }
        break;

    // 开机第十项
    case TJC_PAGE_FILAMENT_VIDEO_2:
        switch (widget_id)
        {
        case TJC_PAGE_FILAMENT_VIDEO_2_DECREASE:
            set_filament_extruder_target(false);
            break;
        
        case TJC_PAGE_FILAMENT_VIDEO_2_INCREASE:
            set_filament_extruder_target(true);
            break;

        case TJC_PAGE_FILAMENT_VIDEO_2_RIGHT:
            page_to(TJC_PAGE_FILAMENT_VIDEO_3);
            break;

        case TJC_PAGE_FILAMENT_VIDEO_2_TARGET:
            open_set_print_filament_target();
            break;

        default:
            break;
        }
        break;

    // 开机第十一项
    case TJC_PAGE_FILAMENT_VIDEO_3:
        switch (widget_id)
        {
        case TJC_PAGE_FILAMENT_VIDEO_3_EXTRUDE:
            open_start_extrude();
            break;
        
        case TJC_PAGE_FILAMENT_VIDEO_3_NEXT:
            page_to(TJC_PAGE_MORE_LEVEL);
            break;

        default:
            break;
        }
        break;

    // 开箱第十二项
    case TJC_PAGE_MORE_LEVEL:
        switch (widget_id)
        {
        case TJC_PAGE_MORE_LEVEL_FINISH:
            //2023.5.12 CLL 修复开机引导完成热床仍显示加热bug
            get_object_status();
            open_more_level_finish();
            break;

        case TJC_PAGE_MORE_LEVEL_PRINT:
            // page_to(TJC_PAGE_MORE_LEVEL_PLU);
            break;

        default:
            break;
        }
        break;

    // 开箱第十三项
    case TJC_PAGE_MORE_LEVEL_PLU:
        switch (widget_id)
        {
        case TJC_PAGE_MORE_LEVEL_PLU_BACK:
            page_to(TJC_PAGE_MORE_LEVEL);
            break;

        case TJC_PAGE_MORE_LEVEL_PLU_ENTER:
            page_to(TJC_PAGE_MAIN);
            break;
        }
        break;
    
    case TJC_PAGE_MAIN:
        switch (widget_id)
        {
        case TJC_PAGE_MAIN_BTN_HOME:
            MKSLOG_BLUE("按下Home按钮");
            break;

        case TJC_PAGE_MAIN_BTN_FILE:
            /*
            page_to(TJC_PAGE_FILE_LIST_1);
            printer_set_babystep();
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();
            get_object_status();
            */
            
            // 屏蔽内置文件系统
            //if (detect_disk() == 0) { //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();
                get_object_status();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            break;

        case TJC_PAGE_MAIN_BTN_TOOL:
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_MAIN_BTN_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            break;
        
        case TJC_PAGE_MAIN_LIGHT:
            led_on_off();
            break;

        case TJC_PAGE_MAIN_VOICE:
            beep_on_off();
            break;

        case TJC_PAGE_MAIN_LOCK:
            motors_off();
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_FILE_LIST_1:
        switch (widget_id)
        {
        case TJC_PAGE_FILE_LIST_1_BTN_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_FILE:
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_TOOL:
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_SERVICE:
            page_to(TJC_PAGE_SERVICE);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_1:
            clear_cp0_image();
            get_sub_dir_files_list(0);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_2:
            clear_cp0_image();
            get_sub_dir_files_list(1);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_3:
            clear_cp0_image();
            get_sub_dir_files_list(2);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_4:
            clear_cp0_image();
            get_sub_dir_files_list(3);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_5:
            clear_cp0_image();
            get_sub_dir_files_list(4);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_6:
            clear_cp0_image();
            get_sub_dir_files_list(5);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_7:
            clear_cp0_image();
            get_sub_dir_files_list(6);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_8:
            clear_cp0_image();
            get_sub_dir_files_list(7);
            break;

        case TJC_PAGE_FILE_LIST_1_BTN_PREVIOUS:
            if (page_files_current_pages > 0) {
                // std::cout << "############################" << page_files_current_pages << std::endl;
                // page_files_current_pages = (page_files_current_pages + page_files_pages) % (page_files_pages + 1);
                page_files_current_pages -= 1;
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();
            }
            break;
        
        case TJC_PAGE_FILE_LIST_1_BTN_NEXT:
            if (page_files_current_pages < page_files_pages) {
                // std::cout << "############################" << page_files_current_pages << std::endl;
                // page_files_current_pages = (page_files_current_pages + 1) % (page_files_pages + 1);
                page_files_current_pages += 1;
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();
            }
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_FILE_LIST_2:
        switch (widget_id)
        {
        case TJC_PAGE_FILE_LIST_2_BTN_1:
            get_sub_dir_files_list(0);
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_2:
            get_sub_dir_files_list(1);
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_3:
            get_sub_dir_files_list(2);
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_4:
            get_sub_dir_files_list(3);
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_5:
            get_sub_dir_files_list(4);
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_6:
            get_sub_dir_files_list(5);
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_7:
            get_sub_dir_files_list(6);
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_8:
            get_sub_dir_files_list(7);
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_BACK:
            // page_files_folder_layers--;
            get_parenet_dir_files_list();
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_PREVIOUS:
            if (page_files_current_pages > 0) {
                // std::cout << "############################" << page_files_current_pages << std::endl;
                // page_files_current_pages = (page_files_current_pages + page_files_pages) % (page_files_pages + 1);
                page_files_current_pages -= 1;
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_2();
            }
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_NEXT:
            if (page_files_current_pages < page_files_pages) {
                // std::cout << "############################" << page_files_current_pages << std::endl;
                // page_files_current_pages = (page_files_current_pages + 1) % (page_files_pages + 1);
                page_files_current_pages += 1;
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_2();
            }
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_HOME:
            page_to(TJC_PAGE_MAIN);
            break;
        
        case TJC_PAGE_FILE_LIST_2_BTN_FILE:
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_TOOL:
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_FILE_LIST_2_BTN_SERVICE:
            page_to(TJC_PAGE_SERVICE);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_DELETE_FILE:
        switch (widget_id)
        {
        case TJC_PAGE_DELETE_FILE_YES:
            delete_file(page_files_root_path + page_files_print_files_path);
            get_parenet_dir_files_list();
            clear_page_preview();
            show_preview_complete = false;
            clear_cp0_image();
            break;

        case TJC_PAGE_DELETE_FILE_NO:
            page_to(TJC_PAGE_PREVIEW);
            // clear_page_preview();
            show_preview_complete = false;
            // clear_cp0_image();
            break;
        }
        break;
    
    case TJC_PAGE_PREVIEW:
        switch (widget_id)
        {
        case TJC_PAGE_PREVIEW_BTN_BACK:
            if (mks_file_parse_finished == false) {
                get_parenet_dir_files_list();
                clear_page_preview();                   // 返回时清除读取的数据
                show_preview_complete = false;
                // show_preview_gimage_completed = false;
                clear_cp0_image();
            } else {
                if (show_preview_complete == true) {        // 当且仅当预览加载完成才可以按下按钮
                    get_parenet_dir_files_list();
                    clear_page_preview();                   // 返回时清除读取的数据
                    show_preview_complete = false;
                    // show_preview_gimage_completed = false;
                    clear_cp0_image();
                }
            }
            break;

        case TJC_PAGE_PREVIEW_BTN_DELETE:
            if (show_preview_complete == true) {        // 当且仅当预览加载完成才可以按下按钮
                page_to(TJC_PAGE_DELETE_FILE);
                // delete_file(page_files_root_path + page_files_print_files_path);
                // get_parenet_dir_files_list();
                // show_preview_complete = false;
                // show_preview_gimage_completed = false;
                // clear_cp0_image();
            }
            break;

        case TJC_PAGE_PREVIEW_BTN_START:
            //2023.4.20 打印过文件标红
            printed_file_path = page_files_print_files_path;
            std::cout << "文件路径：" << page_files_print_files_path << std::endl;
            if (show_preview_complete == true) {        // 当且仅当预览加载完成才可以按下按钮
                show_preview_complete = false;
                //2023.5.9 CLL 删除清理X轴动画
                //if (get_mks_total_printed_time() > 48000) {
                //    page_to(TJC_PAGE_X_CLEAR_PLUS3);
                //} else {
                    if (get_filament_detected_enable() == true) {
                        if (get_filament_detected() == true) {
                            std::cout << "没有检测到断料" << std::endl;
                            //2.1.2 CLL 判断耗材种类并弹窗
                            //page_to(TJC_PAGE_PRINTING);
                            check_filament_type();
                            printer_print_stats_state = "printing";
                            print_start();
                            start_printing(page_files_print_files_path);
                        } else {
                            std::cout << "检测到断料" << std::endl;
                            page_to(TJC_PAGE_PRINT_F_POP);
                        }
                    } else {
                        print_start();
                        start_printing(page_files_print_files_path);
                        printer_print_stats_state = "printing";
                        //2.1.2 CLL 判断耗材种类并弹窗
                        //page_to(TJC_PAGE_PRINTING);
                        check_filament_type();
                    }
                //}
                // clear_page_preview();
                // show_preview_gimage_completed = false;
                show_preview_complete = false;
                //2023.5.11 CLL 修复页面跳转bug
                printer_ready = false;
            }
            break;
        
        //2.1.2 CLL 新增热床调平
        case TJC_PAGE_PREVIEW_BED_LEVELING:
            if (printer_bed_leveling_state == true) {
                bed_leveling_switch(false);
            }else if(printer_bed_leveling_state == false) {
                bed_leveling_switch(true);
            }

        default:
            break;
        }
        break;

    //2.1.2 CLL 重置打印界面
    case TJC_PAGE_PRINTING:
        switch (widget_id)
        {
        case TJC_PAGE_PRINTING_SET_EXTRUDER:
            printing_keyboard_enabled = true;
            clear_page_printing_arg();
            break;

        case TJC_PAGE_PRINTING_SET_HEATER_BED:
            printing_keyboard_enabled = true;
            clear_page_printing_arg();
            break;

        case TJC_PAGE_PRINTING_SET_HOT:
            printing_keyboard_enabled = true;
            clear_page_printing_arg();
            break;

        case TJC_PAGE_PRINTING_SET_FAN_0:
            printing_keyboard_enabled = true;
            clear_page_printing_arg();
            break;

        case TJC_PAGE_PRINTING_SET_FAN_1:
            printing_keyboard_enabled = true;
            clear_page_printing_arg();
            break;

        //2.1.2 CLL 新增fan3
        case TJC_PAGE_PRINTING_SET_FAN_3:
            printing_keyboard_enabled =true;
            clear_page_printing_arg();
            break;

        case TJC_PAGE_PRINTING_SET_SPEED:
            printing_keyboard_enabled = true;
            clear_page_printing_arg();
            break;

        case TJC_PAGE_PRINTING_SET_FLOW:
            printing_keyboard_enabled = true;
            clear_page_printing_arg();
            break;

        case TJC_PAGE_PRINTING_BTN_Z_OFFSET:
            page_to(TJC_PAGE_PRINT_ZOFFSET);
            set_intern_zoffset(0.05);
            clear_page_printing_arg();
            break;

        /*
        case TJC_PAGE_RPINTING_BTN_SHUTDOWN:
            set_printing_shutdown();
            break;
        */
        case TJC_PAGE_PRINTING_BTN_PAUSE_RESUME:
            printer_ready = false;
            set_print_pause();
            clear_page_printing_arg();
            //4.2.2 CLL 优化页面跳转
            page_to(TJC_PAGE_PRINT_FILAMENT);
            //2023.5.11 CLL 修复页面跳转bug
            /*
            if (get_print_pause_resume() == false) {
                set_print_pause();
                page_to(TJC_PAGE_PRINT_FILAMENT);
                clear_page_printing_arg();
            } else {
                set_print_resume();
            }
            */
            break;

        case TJC_PAGE_PRINTING_BTN_STOP:
            page_to(TJC_PAGE_STOP_PRINT);
            clear_page_printing_arg();
            break;

        case TJC_PAGE_PRINTING_BTN_LIGHT:
            led_on_off();
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_PRINT_FILAMENT:
        switch (widget_id)
        {
        case TJC_PAGE_PRINT_FILAMENT_PAUSE_RESUME:
            MKSLOG_BLUE("get_filament_detected_enable: %d", get_filament_detected_enable());
            MKSLOG_BLUE("get_filament_detected: %d", get_filament_detected());
            if (get_filament_detected_enable() == true) {
                if (get_filament_detected() == true) {
                    //4.2.2 CLL 优化页面跳转
                    printer_ready = false;
                    set_print_resume();
                    page_to(TJC_PAGE_PRINTING);
                } else {
                    page_to(TJC_PAGE_PRINT_F_POP);
                }
            } else {
                //4.2.2 CLL 优化页面跳转
                printer_ready = false;
                set_print_resume();
                page_to(TJC_PAGE_PRINTING);
            }
            // if (get_print_pause_resume() == true) {
            
            // }
            break;

        case TJC_PAGE_PRINT_FILAMENT_STOP:
            page_to(TJC_PAGE_STOP_PRINT);
            clear_page_printing_arg();
            break;

        case TJC_PAGE_PRINT_FILAMENT_TARGET:
            set_print_filament_target();
            break;

        case TJC_PAGE_PRINT_FILAMENT_TARGET_UP:
            set_filament_extruder_target(true);
            break;

        case TJC_PAGE_PRINT_FILAMENT_TARGET_DOWN:
            set_filament_extruder_target(false);
            break;

        case TJC_PAGE_PRINT_FILAMENT_RETRACT:
            printer_idle_timeout_state = "Printing";
            page_print_filament_extrude_restract_button = true;
            start_retract();
            break;

        case TJC_PAGE_PRINT_FILAMENT_EXTRUDE:
            printer_idle_timeout_state = "Printing";
            page_print_filament_extrude_restract_button = true;
            start_extrude();
            break;

        case TJC_PAGE_PRINT_FILAMENT_10:
            set_print_filament_dist(10);
            break;

        case TJC_PAGE_PRINT_FILAMENT_20:
            set_print_filament_dist(20);
            break;

        case TJC_PAGE_PRINT_FILAMENT_50:
            set_print_filament_dist(50);
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_MOVE:
        switch (widget_id)
        {
        case TJC_PAGE_MOVE_BTN_FILE:
            /*
            set_move_dist(0.1);             // 恢复步长为0.1
            page_to(TJC_PAGE_FILE_LIST_1);
            printer_set_babystep();
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();
            get_object_status();
            */
            
            //if (detect_disk() == 0) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();
                get_object_status();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            
            break;

        case TJC_PAGE_MOVE_BTN_HOME:
            // set_move_dist(0.1);             // 恢复步长为0.1
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_MOVE_BTN_SERVICE:
            // set_move_dist(0.1);             // 恢复步长为0.1
            page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_MOVE_FILAMENT:
            // set_move_dist(0.1);             // 恢复步长为0.1
            page_to(TJC_PAGE_FILAMENT);
            break;

        case TJC_PAGE_MOVE_LEVEL_MODE:
            // set_move_dist(0.1);             // 恢复步长为0.1
            page_to(TJC_PAGE_LEVEL_MODE);
            break;

        case TJC_PAGE_MOVE_NETWORK:
            // set_move_dist(0.1);             // 恢复步长为0.1
            go_to_network();
            // send_cmd_txt(tty_fd, "t0", get_wlan0_ip().data());
            break;

        case TJC_PAGE_MOVE_01:
            set_move_dist(0.1);
            break;

        case TJC_PAGE_MOVE_1:
            set_move_dist(1.0);
            break;

        case TJC_PAGE_MOVE_10:
            set_move_dist(10.0);
            break;

        case TJC_PAGE_MOVE_X_UP:
            move_x_increase();
            break;

        case TJC_PAGE_MOVE_X_DOWN:
            move_x_decrease();
            break;

        case TJC_PAGE_MOVE_Y_DOWN:
            move_y_increase();
            break;

        case TJC_PAGE_MOVE_Y_UP:
            move_y_decrease();
            break;

        case TJC_PAGE_MOVE_HOME:
            move_home();
            break;

        case TJC_PAGE_MOVE_Z_UP:
            move_z_decrease();
            break;

        case TJC_PAGE_MVOE_Z_DOWN:
            move_z_increase();
            break;

        case TJC_PAGE_MOVE_BTN_STOP:
            // firmware_reset();
            break;

        case TJC_PAGE_MOVE_M84:
            move_motors_off();
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_MOVE_POP_1:
    case TJC_PAGE_MOVE_POP_2:
    case TJC_PAGE_MOVE_POP_3:
        switch (widget_id)
        {
        case TJC_PAGE_MOVE_POP_1_YES:
            move_home();
            page_to(TJC_PAGE_MOVE);
            //4.2.2 CLL 修复显示bug
            set_move_dist(0.1);             // 恢复步长为0.1
            break;

        case TJC_PAGE_MOVE_POP_1_NO:
            page_to(TJC_PAGE_MOVE);
            //4.2.2 CLL 修复显示bug
            set_move_dist(0.1);             // 恢复步长为0.1
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_FILAMENT:
        switch (widget_id)
        {
        case TJC_PAGE_FILAMENT_BTN_FILE:
            //if (detect_disk() == 0) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();

                get_object_status();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            
            break;

        case TJC_PAGE_FILAMENT_BTN_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_FILAMENT_BTN_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            break;
        
        case TJC_PAGE_FILAMENT_MOVE:
            // set_move_dist(0.1);             // 恢复步长为0.1
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_FILAMENT_AUTOLEVEL:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;

        case TJC_PAGE_FILAMENT_NETWORK:
            go_to_network();
            // send_cmd_txt(tty_fd, "t0", get_wlan0_ip().data());
            break;

        case TJC_PAGE_FILAMENT_BTN_10:
            set_print_filament_dist(10);
            break;

        case TJC_PAGE_FILAMENT_BTN_20:
            set_print_filament_dist(20);
            break;

        case TJC_PAGE_FILAMENT_BTN_50:
            set_print_filament_dist(50);
            break;

        case TJC_PAGE_FILAMENT_UNLOAD:
            //2023.5.12 CLL 退料前解锁电机
            move_motors_off();
            //2023.4.26-1 修改退料流程
            if ((printer_extruder_temperature >= (printer_extruder_target - 5))&&(printer_extruder_temperature <= (printer_extruder_target + 5))) {
                page_filament_unload_button = true;
                page_to(TJC_PAGE_FILAMENT_POP_4);
            }else {
                page_to(TJC_PAGE_FILAMENT_POP);
            }
            page_filament_unload_button = true;
            //page_to(TJC_PAGE_FILAMENT_POP_4);
            //page_filament_unload_button = true;
            // filament_unload();
            /*
            if (page_filament_unload_button == true) {
                page_filament_unload_button = false;
            }
            */
            break;

        case TJC_PAGE_FILAMENT_BTN_RETRACT:
            printer_idle_timeout_state = "Printing";
            page_filament_extrude_button = true;
            
            start_retract();
            break;

        case TJC_PAGE_FILAMENT_BTN_EXTRUDE:
            printer_idle_timeout_state = "Printing";
            page_filament_extrude_button = true;
            
            start_extrude();
            break;

        case TJC_PAGE_FILAMENT_BTN_EXTRUDER:
            page_to(TJC_PAGE_KB_FILAMENT_1);
            break;

        case TJC_PAGE_FILAMENT_BTN_HEATER_BED:
            page_to(TJC_PAGE_KB_FILAMENT_1);
            break;

        case TJC_PAGE_FILAMENT_BTN_HOT:
            page_to(TJC_PAGE_KB_FILAMENT_1);
            break;

        case TJC_PAGE_FILAMENT_BTN_FAN_1:
            std::cout << "按下第一个风扇" << std::endl;
            filament_fan0();
            break;

        case TJC_PAGE_FILAMENT_BTN_FAN_2:
            std::cout << "按下第二个风扇" << std::endl;
            filament_fan2();
            // filament_keyboard_enabled = true;
            // page_to(TJC_PAGE_KB_FILAMENT_1);
            break;

        //2.1.2 CLL 新增fan3
        case TJC_PAGE_FILAMENT_BTN_FAN_3:
            std::cout << "按下第三个风扇" << std::endl;
            filament_fan3();
            break;

        case TJC_PAGE_FILAMENT_BTN_FILAMENT_SENSOR:
        // case 0x10:
            set_filament_sensor();
            break;

        case TJC_PAGE_FILAMENT_BTN_STOP:
            firmware_reset();
            break;

        case TJC_PAGE_FILAMENT_EXTRUDER:
            filament_extruder_target();
            break;

        case TJC_PAGE_FILAMENT_HEATER_BED:
            filament_heater_bed_target();
            break;

        case TJC_PAGE_FILAMENT_HOT:
            filament_hot_target();
            break;
        
        }
        break;

    case TJC_PAGE_FILAMENT_POP:
        switch (widget_id)
        {
        case TJC_PAGE_FILAMENT_POP_YES:
            // set_extruder_target(240);
            // 2023.4.22-1 修改退料流程
            /*
            if (previous_page_id == TJC_PAGE_FILAMENT_POP_4) {
                printer_idle_timeout_state = "Printing";
                filament_unload();
                page_to(TJC_PAGE_FILAMENT);
            } else {
                set_extruder_target(280);       // 按照加热要求加热到280度
                page_filament_extrude_button = false;

                page_to(TJC_PAGE_FILAMENT);
            }
            */
            page_filament_extrude_button =false;
            page_filament_unload_button = false;
            page_to(TJC_PAGE_FILAMENT);
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_FILAMENT_POP_2:
        switch (widget_id)
        {
        case TJC_PAGE_FILAMENT_POP_2_YES:
            // filament_unload();
            // page_to(TJC_PAGE_FILAMENT);
            filament_pop_2_yes();
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_FILAMENT_POP_4:
        switch (widget_id)
        {
        case TJC_PAGE_FILAMENT_POP_4_OK:
            // filament_unload();
            // page_to(TJC_PAGE_FILAMENT);
            //2023.4.22-1 修改退料流程
            //filament_pop_2_yes();
            filament_unload();
            //2.1.2 CLL 新增退料界面
            //page_to(TJC_PAGE_FILAMENT);
            page_to(TJC_PAGE_UNLOADING);
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_LEVEL_MODE:  
        switch (widget_id)
        {
        case TJC_PAGE_LEVEL_MODE_FILE:

            /*
            page_to(TJC_PAGE_FILE_LIST_1);
            printer_set_babystep();
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();

            get_object_status();
            */
            
            //if (detect_disk() == 0) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();
                sub_object_status();
                get_object_status();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            
            break;

        case TJC_PAGE_LEVEL_MODE_HOME:
            get_object_status();
            sub_object_status();
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_LEVEL_MODE_SERVICE:
            get_object_status();
            sub_object_status();
            page_to(TJC_PAGE_LANGUAGE);
            break;
            
        case TJC_PAGE_LEVEL_MODE_MOVE:
            get_object_status();
            sub_object_status();
            set_move_dist(0.1);             // 恢复步长为0.1
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_LEVEL_MODE_FILAMENT:
            get_object_status();
            sub_object_status();
            page_to(TJC_PAGE_FILAMENT);
            break;

        case TJC_PAGE_LEVEL_MODE_NETWORK:
            get_object_status();
            sub_object_status();
            go_to_network();
            // send_cmd_txt(tty_fd, "t0", get_wlan0_ip().data());
            break;

        case TJC_PAGE_LEVEL_MODE_AUTO_LEVEL:
            //sub_object_status();
            //get_object_status();
            level_mode = TJC_PAGE_AUTO_LEVEL;
            page_to(TJC_PAGE_LEVEL_NULL_1);
            break;

        case TJC_PAGE_LEVEL_MODE_MANUAL_LEVEL:
            sub_object_status();
            get_object_status();
            level_mode = TJC_PAGE_MANUAL_LEVEL;
            page_to(TJC_PAGE_LEVELING_NULL_2);
            break;
        
        //2023.4.20 修改调平数据显示
        case TJC_PAGE_LEVEL_MODE_ZOFFSET:
            get_object_status();
            page_to(TJC_PAGE_SET_ZOFFSET);
            break;

        // case TJC_PAGE_LEVEL_MODE_SET_ZOFFSET:
            // 先屏蔽掉补偿值设置
            // page_to(TJC_PAGE_SET_ZOFFSET);
            // level_mode = TJC_PAGE_SET_ZOFFSET;
            // page_to(TJC_PAGE_LEVELING_NULL);

        case TJC_PAGE_LEVEL_MODE_CALIBRATION:
            sub_object_status();
            get_object_status();
            /*
            level_mode_printing_extruder_target = 220;
            level_mode_printing_heater_bed_target = 60;
            page_to(TJC_PAGE_SET_TEMP_LEVEL);
            send_cmd_val(tty_fd, "n0", std::to_string(level_mode_printing_extruder_target));
            send_cmd_val(tty_fd, "n1", std::to_string(level_mode_printing_heater_bed_target));
            */
            break;

        case TJC_PAGE_LEVEL_MODE_SYNTONY_MOVE:
            sub_object_status();
            get_object_status();
            go_to_syntony_move();
            // level_mode = TJC_PAGE_SYNTONY_MOVE;
            // page_to(TJC_PAGE_LEVELING_NULL);
            break;

        case TJC_PAGE_LEVEL_MODE_PID:
            sub_object_status();
            get_object_status();
            // go_to_pid_working();
            page_to(TJC_PAGE_WAIT_POP);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_LEVELING_NULL:
        switch (widget_id)
        {
        case TJC_PAGE_LEVELING_NULL_BTN_BACK:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;

        case TJC_PAGE_LEVELING_NULL_BTN_FILE:

            /*
            page_to(TJC_PAGE_FILE_LIST_1);
            printer_set_babystep();
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();

            get_object_status();
            */
            
            //if (detect_disk() == 0) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();

                get_object_status();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            
            break;

        case TJC_PAGE_LEVELING_NULL_BTN_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_LEVELING_NULL_BTN_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_LEVELING_NULL_ENTER:
            // page_to(TJC_PAGE_LEVELING_INIT);
            switch (level_mode)
            {
            case TJC_PAGE_AUTO_LEVEL:
                std::cout << "进入到这里" << std::endl;
                auto_level_button_enabled = true;
                printer_idle_timeout_state = "Printing";
                pre_auto_level_init();
                break;

            case TJC_PAGE_MANUAL_LEVEL:
                printer_idle_timeout_state = "Printing";
                pre_manual_level_init();
                break;

            case TJC_PAGE_SET_ZOFFSET:
                // pre_set_zoffset_init();
                page_to(TJC_PAGE_CHANGE_ZOFFSET);
                // page_to(TJC_PAGE_SET_ZOFFSET);
                break;
            
            default:
                break;
            }
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_LEVELING_NULL_2:
        switch (widget_id)
        {
        case TJC_PAGE_LEVELING_NULL_2_ENTER:
            printer_idle_timeout_state = "Printing";
            pre_manual_level_init();
            break;

        case TJC_PAGE_LEVELING_NULL_2_BACK:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_AUTO_LEVEL:
        switch (widget_id)
        {
        case TJC_PAGE_AUTO_LEVEL_001:
            set_auto_level_dist(0.01);
            break;

        case TJC_PAGE_AUTO_LEVEL_0025:
            // set_auto_level_dist(0.025);
            set_auto_level_dist(0.05);
            break;

        case TJC_PAGE_AUTO_LEVEL_005:
            // set_auto_level_dist(0.05);
            set_auto_level_dist(0.1);
            break;

        case TJC_PAGE_AUTO_LEVEL_1:
            set_auto_level_dist(1);
            break;

        case TJC_PAGE_AUTO_LEVEL_UP:
            start_auto_level_dist(false);
            break;

        case TJC_PAGE_AUTO_LEVEL_DOWN:
            start_auto_level_dist(true);
            break;
        
        case TJC_PAGE_AUTO_LEVEL_ENTER:
            page_to(TJC_PAGE_AUTO_MOVE);
            start_auto_level();
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_AUTO_FINISH:
        switch (widget_id)
        {
        case TJC_PAGE_AUTO_FINISH_YES:
            //4.2.2 CLL 修复自动调平页面卡住
            if (auto_level_button_enabled == true || printer_idle_timeout_state == "Idle") {
                auto_level_button_enabled = false;
                std::cout << "自动调平已完成" << std::endl;
                finish_auto_level();
                // page_to(TJC_PAGE_SAVING);
            }
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_MANUAL_LEVEL:
        switch (widget_id)
        {
        case TJC_PAGE_MANUAL_LEVEL_001:
            set_manual_level_dist(0.01);
            break;
        
        case TJC_PAGE_MANUAL_LEVEL_0025:
            // set_manual_level_dist(0.025);
            set_manual_level_dist(0.05);
            break;

        case TJC_PAGE_MANUAL_LEVEL_005:
            // set_manual_level_dist(0.05);
            set_manual_level_dist(0.1);
            break;

        case TJC_PAGE_MANUAL_LEVEL_1:
            set_manual_level_dist(1);
            break;

        case TJC_PAGE_MANUAL_LEVEL_DOWN:
            start_manual_level_dist(true);
            break;

        case TJC_PAGE_MANUAL_LEVEL_UP:
            start_manual_level_dist(false);
            break;

        case TJC_PAGE_MANUAL_LEVEL_ENTER:
            start_manual_level();
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_MANUAL_FINISH:
        switch (widget_id)
        {
        case TJC_PAGE_MANUAL_FINISH_YES:
            // if (manual_level_button_enabled == true) {
                // manual_level_button_enabled = false;
                std::cout << "手动调平已完成" << std::endl;
                finish_manual_level();
            // }
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_PRINT_F_POP:
        switch (widget_id)
        {
        case TJC_PAGE_PRINT_F_POP_YES:
            //2.1.2 CLL 在打印设置zoffset界面也会有断料提醒弹窗
            if (previous_page_id == TJC_PAGE_PRINTING || previous_page_id == TJC_PAGE_PRINT_ZOFFSET) {
                if (get_filament_detected() == false) {
                    // set_print_pause();
                    page_to(TJC_PAGE_PRINT_FILAMENT);
                } else {
                    page_to(TJC_PAGE_PRINT_FILAMENT);
                }
            } else if (previous_page_id == TJC_PAGE_PREVIEW) {
                if (get_filament_detected() == false) {
                    page_to(TJC_PAGE_FILAMENT);
                }
            } else{ //4.2.2 CLL 修复卡在断料界面
                if (get_filament_detected() == false) {
                    page_to(TJC_PAGE_PRINT_FILAMENT);
                } else {
                    page_to(TJC_PAGE_PRINT_FILAMENT);
                }
            }
            break;
        }
        break;

    case TJC_PAGE_SET_ZOFFSET:
        switch (widget_id)
        {
        case TJC_PAGE_SET_ZOFFSET_BACK:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;

        case TJC_PAGE_SET_ZOFFSET_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_SET_ZOFFSET_FILE:
            //if (detect_disk() == 0) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();
                get_object_status();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            break;

        case TJC_PAGE_SET_ZOFFSET_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_SET_ZOFFSET_B1:
            //move_to_certain_position(0);
            break;

        case TJC_PAGE_SET_ZOFFSET_B2:
            //move_to_certain_position(4);
            break;

        case TJC_PAGE_SET_ZOFFSET_B3:
            //move_to_certain_position(8);
            break;

        case TJC_PAGE_SET_ZOFFSET_B4:
            //move_to_certain_position(12);
            break;

        case TJC_PAGE_SET_ZOFFSET_B5:
            //move_to_certain_position(1);
            break;

        case TJC_PAGE_SET_ZOFFSET_B6:
            //move_to_certain_position(5);
            break;

        case TJC_PAGE_SET_ZOFFSET_B7:
            //move_to_certain_position(9);
            break;

        case TJC_PAGE_SET_ZOFFSET_B8:
            //move_to_certain_position(13);
            break;

        case TJC_PAGE_SET_ZOFFSET_B9:
            //move_to_certain_position(2);
            break;

        case TJC_PAGE_SET_ZOFFSET_B10:
            //move_to_certain_position(6);
            break;

        case TJC_PAGE_SET_ZOFFSET_B11:
            //move_to_certain_position(10);
            break;

        case TJC_PAGE_SET_ZOFFSET_B12:
            //move_to_certain_position(14);
            break;

        case TJC_PAGE_SET_ZOFFSET_B13:
            //move_to_certain_position(3);
            break;

        case TJC_PAGE_SET_ZOFFSET_B14:
            //move_to_certain_position(7);
            break;

        case TJC_PAGE_SET_ZOFFSET_B15:
            //move_to_certain_position(11);
            break;

        case TJC_PAGE_SET_ZOFFSET_B16:
            //move_to_certain_position(15);
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_CHANGE_ZOFFSET:
        switch (widget_id)
        {
        case TJC_PAGE_CHANGE_ZOFFSET_FILE:

            /*
            page_to(TJC_PAGE_FILE_LIST_1);
            printer_set_babystep();
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();

            get_object_status();
            */
            
            //if (detect_disk() == 0) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();

                get_object_status();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            
            break;

        case TJC_PAGE_CHANGE_ZOFFSET_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_CHANGE_ZOFFSET_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_CHANGE_ZOFFSET_BACK:
            page_to(TJC_PAGE_LEVELING_NULL);
            break;

        case TJC_PAGE_CHANGE_ZOFFSET_ENTER:
            pre_set_zoffset_init();
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_WIFI_LIST_2:
        switch (widget_id)
        {
        case TJC_PAGE_WIFI_LIST_2_BTN_FILE:
            //if (0 == detect_disk()) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();

                get_object_status();

                mks_wpa_cli_close_connection();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            break;
        
        case TJC_PAGE_WIFI_LIST_2_BTN_HOME:
            page_to(TJC_PAGE_MAIN);

            mks_wpa_cli_close_connection();
            break;

        case TJC_PAGE_WIFI_LIST_2_BTN_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);

            mks_wpa_cli_close_connection();
            break;

        case TJC_PAGE_WIFI_LIST_2_MOVE:
            page_to(TJC_PAGE_MOVE);

            mks_wpa_cli_close_connection();
            break;

        case TJC_PAGE_WIFI_LIST_2_FILAMENT:
            page_to(TJC_PAGE_FILAMENT);

            mks_wpa_cli_close_connection();
            break;

        case TJC_PAGE_WIFI_LIST_2_AUTO_LEVEL:
            page_to(TJC_PAGE_LEVEL_MODE);

            mks_wpa_cli_close_connection();
            break;

        case TJC_PAGE_WIFI_LIST_2_PREVIOUS:
            // std::cout << "page_wifi_current_pages = " << page_wifi_current_pages << std::endl;
            // std::cout << "page_wifi_ssid_list_pages = " << page_wifi_ssid_list_pages << std::endl;
            if (page_wifi_current_pages > 0) {
                std::cout << "page_wifi_current_pages = " << page_wifi_current_pages << std::endl;
                std::cout << "page_wifi_ssid_list_pages = " << page_wifi_ssid_list_pages << std::endl;
                page_wifi_current_pages -= 1;
                set_page_wifi_ssid_list(page_wifi_current_pages);
                refresh_page_wifi_list();
            }
            break;

        case TJC_PAGE_WIFI_LIST_2_NEXT:
            // std::cout << "page_wifi_current_pages = " << page_wifi_current_pages << std::endl;
            // std::cout << "page_wifi_ssid_list_pages = " << page_wifi_ssid_list_pages << std::endl;
            if (page_wifi_current_pages < page_wifi_ssid_list_pages - 1) {
                std::cout << "page_wifi_current_pages = " << page_wifi_current_pages << std::endl;
                std::cout << "page_wifi_ssid_list_pages = " << page_wifi_ssid_list_pages << std::endl;
                page_wifi_current_pages += 1;
                set_page_wifi_ssid_list(page_wifi_current_pages);
                refresh_page_wifi_list();
            }
            break;

        case TJC_PAGE_WIFI_LIST_2_SSID_1:
            if (page_wifi_list_ssid_button_enabled[0] == true) {
                get_wifi_list_ssid(0);
                printing_wifi_keyboard_enabled = true;
                page_to(TJC_PAGE_WIFI_KEYBOARD);
            } else {

            }
            break;

        case TJC_PAGE_WIFI_LIST_2_SSID_2:
            if (true == page_wifi_list_ssid_button_enabled[1]) {
                get_wifi_list_ssid(1);
                printing_wifi_keyboard_enabled = true;
                page_to(TJC_PAGE_WIFI_KEYBOARD);
            }
            break;

        case TJC_PAGE_WIFI_LIST_2_SSID_3:
            if (true == page_wifi_list_ssid_button_enabled[2]) {
                get_wifi_list_ssid(2);
                printing_wifi_keyboard_enabled = true;
                page_to(TJC_PAGE_WIFI_KEYBOARD);
            }
            break;

        case TJC_PAGE_WIFI_LIST_2_SSID_4:
            if (true == page_wifi_list_ssid_button_enabled[3]) {
                get_wifi_list_ssid(3);
                printing_wifi_keyboard_enabled = true;
                page_to(TJC_PAGE_WIFI_KEYBOARD);
            }
            break;

        case TJC_PAGE_WIFI_LIST_2_SSID_5:
            if (true == page_wifi_list_ssid_button_enabled[4]) {
                get_wifi_list_ssid(4);
                printing_wifi_keyboard_enabled = true;
                page_to(TJC_PAGE_WIFI_KEYBOARD);
            }
            break;

        case TJC_PAGE_WIFI_LIST_2_REFRESH:
            std::cout << "################## 按下刷新按钮" << std::endl;
            scan_ssid_and_show();
            //2.1.2 CLL 修复wifi页面bug
            std::cout << "等待延时测试3s..." << std::endl;
            sleep(3);
            scan_ssid_and_show();
            break;

        case TJC_PAGE_WIFI_LIST_2_EHTNET:
            set_mks_net_eth();
            mks_page_internet_ip = get_eth0_ip();
            page_to(TJC_PAGE_INTERNET);
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_KEYDBA:
        switch (widget_id)
        {
        case TJC_PAGE_KEYDBA_HOME:
            break;

        case TJC_PAGE_KEYDBA_FILE:
            break;

        case TJC_PAGE_KEYDBA_SERVICE:
            break;

        // case TJC_PAGE_KEYDBA_ENTER:
            // break;

        case TJC_PAGE_KEYDBA_BACK:
            printing_wifi_keyboard_enabled = false;
            page_to(TJC_PAGE_WIFI_LIST_2);
            break;

        default:
            break;
        }
        break;

    /*
    case TJC_PAGE_WIFI_LIST_2:
        switch (widget_id)
        {
        case TJC_PAGE_WIFI_LIST_2_BTN_FILE:
            page_to(TJC_PAGE_FILE_LIST_1);
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();

            get_object_status();
            break;
        
        case TJC_PAGE_WIFI_LIST_2_BTN_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_WIFI_LIST_2_BTN_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_WIFI_LIST_2_MOVE:
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_WIFI_LIST_2_FILAMENT:
            page_to(TJC_PAGE_FILAMENT);
            break;

        case TJC_PAGE_WIFI_LIST_2_AUTO_LEVEL:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;

        default:
            break;

        }
        break;
    */

    case TJC_PAGE_INTERNET:
        switch (widget_id)
        {
        case TJC_PAGE_INTERNET_MOVE:
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_INTERNET_FILAMENT:
            page_to(TJC_PAGE_FILAMENT);
            break;

        case TJC_PAGE_INTERNET_AUTO_LEVEL:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;
        
        case TJC_PAGE_INTERNET_BTN_FILE:
            //if (0 == detect_disk()) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();
                get_object_status();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            break;
        
        case TJC_PAGE_INTERNET_BTN_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_INTERNET_BTN_SERVICE:
            page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_INTERNET_WIFI:
            set_mks_net_wifi();
            go_to_network();
            break;
        
        default:
            break;

        }
        break;

    case TJC_PAGE_LANGUAGE:
        switch (widget_id)
        {
        case TJC_PAGE_LANGUAGE_BTN_FILE:

            /*
            page_to(TJC_PAGE_FILE_LIST_1);
            printer_set_babystep();
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();

            get_object_status();
            */

            
            //if (detect_disk() == 0) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();

                get_object_status();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            
            break;
        
        case TJC_PAGE_LANGUAGE_BTN_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_LANGUAGE_BTN_TOOL:
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_LANGUAGE_SERVICE:
            page_to(TJC_PAGE_SERVICE);
            break;

        case TJC_PAGE_LANGUAGE_RESET:
            go_to_reset();
            break;

        case TJC_PAGE_LANGUAGE_ABOUT:
            // page_to(TJC_PAGE_ABOUT);
            go_to_about();
            break;

        }
        break;

    //4.2.1 CLL 优化页面跳转
    case TJC_PAGE_SERVICE:
        switch (widget_id)
        {
        case TJC_PAGE_SERVICE_BTN_FILE:

            /*
            page_to(TJC_PAGE_FILE_LIST_1);
            printer_set_babystep();
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();

            get_object_status();
            */

            
            //if (detect_disk() == 0) {  //2023.5.8 CLL 去除U盘判断条件
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();

                get_object_status();
            }
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            
            break;
        
        case TJC_PAGE_SERVICE_BTN_HOME:
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
                get_object_status();
                page_to(TJC_PAGE_MAIN);
            }
            break;

        case TJC_PAGE_SERVICE_BTN_TOOL:
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
                get_object_status();
                page_to(TJC_PAGE_MOVE);
            }
            break;

        case TJC_PAGE_SERVICE_LANGUAGE:
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
                get_object_status();
                page_to(TJC_PAGE_LANGUAGE);
            }
            break;

        case TJC_PAGE_SERVICE_RESET:
            get_object_status();
            go_to_reset();
            break;

        case TJC_PAGE_SERVICE_ABOUT:
            // page_to(TJC_PAGE_ABOUT);
            get_object_status();
            go_to_about();
            break;

        }
        break;

    //4.2.1 CLL 优化页面跳转
    case TJC_PAGE_RESET:
        switch (widget_id)
        {
        case TJC_PAGE_RESET_BTN_FILE:
            // page_to(TJC_PAGE_FILE_LIST_1);
            // page_files_pages = 0;
            // page_files_current_pages = 0;
            // page_files_folder_layers = 0;
            // page_files_previous_path = "";
            // page_files_root_path = DEFAULT_DIR;
            // page_files_path = "";
            // refresh_page_files(page_files_current_pages);
            // refresh_page_files_list_1();
            break;
        
        case TJC_PAGE_RESET_BTN_HOME:
            // page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_RESET_BTN_TOOL:
            // page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_RESET_LANGUAGE:
            // page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_RESET_SERVICE:
            page_to(TJC_PAGE_SERVICE);
            break;

        case TJC_PAGE_RESET_ABOUT:
            // page_reset_to_about = true;
            //current_page_id = TJC_PAGE_ABOUT;
            go_to_about();
            break;

        case TJC_PAGE_RESET_RESTART_KLIPPER:
            reset_klipper();
            break;

        case TJC_PAGE_RESET_FIRMWARE_RESTART:
            reset_firmware();
            break;

        }
        break;

    //2023.4.25-4 修复重启后温度显示不正确
    case TJC_PAGE_SYS_OK:
        switch (widget_id)
        {
        case TJC_PAGE_SYS_OK_HOME:
            get_object_status();
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_SYS_OK_FILE:
            get_object_status();
            /*
            page_to(TJC_PAGE_FILE_LIST_1);
            printer_set_babystep();
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();

            get_object_status();
            */
            
            //if (detect_disk() == 0) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();

                get_object_status();
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            
            break;

        case TJC_PAGE_SYS_OK_TOOL:
            get_object_status();
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_SYS_OK_LANGUAGE:
            get_object_status();
            page_to(TJC_PAGE_LANGUAGE);
            break;

        case TJC_PAGE_SYS_OK_SERVICE:
            get_object_status();
            page_to(TJC_PAGE_SERVICE);
            break;

        case TJC_PAGE_SYS_OK_ABOUT:
            get_object_status();
            // page_to(TJC_PAGE_ABOUT);
            go_to_about();
            break;
        
        default:
            break;
        }
        break;

    // 2023.5.11 CLL 修复页面跳转bug
    case TJC_PAGE_ABOUT:
        switch (widget_id)
        {
        //2023.4.25-1 隐藏开机引导
        case TJC_PAGE_ABOUT_S_BTN:
                set_mks_oobe_enabled(true);
            break;

        case TJC_PAGE_ABOUT_BTN_FILE:

            /*
            page_to(TJC_PAGE_FILE_LIST_1);
            printer_set_babystep();
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();

            get_object_status();
            */
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
            //if (detect_disk() == 0) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();

                get_object_status();
            }
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            
            break;
        
        case TJC_PAGE_ABOUT_BTN_HOME:
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
                page_to(TJC_PAGE_MAIN);
                get_object_status();
            }
            break;

        case TJC_PAGE_ABOUT_BTN_TOOL:
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
                page_to(TJC_PAGE_MOVE);
                get_object_status();
            }
            break;

        case TJC_PAGE_ABOUT_LANGUAGE:
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
                page_to(TJC_PAGE_LANGUAGE);
                get_object_status();
            }
            break;

        case TJC_PAGE_ABOUT_SERVICE:
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
                page_to(TJC_PAGE_SERVICE);
                get_object_status();
            }
            break;

        case TJC_PAGE_ABOUT_RESET:
            go_to_reset();
            break;

        case TJC_PAGE_ABOUT_UPDATE:
            disable_page_about_successed();
            start_update();
            break;

        case TJC_PAGE_ABOUT_OOBE:
            std::cout << "打开oobe开关" << std::endl;
            if (get_mks_oobe_enabled() == true) {
                set_mks_oobe_enabled(false);
                send_cmd_picc(tty_fd, "b1", "132");
            } else {
                set_mks_oobe_enabled(true);
                send_cmd_picc(tty_fd, "b1", "367");
            }
            break;

        default:
            break;
        }
        break;

    //4.2.1 CLL 优化页面跳转
    case TJC_PAGE_NO_UPDATA:
        switch (widget_id)
        {
        //2023.4.25-1 隐藏开机引导
        case TJC_PAGE_NO_UPDATA_S_BTN:
            set_mks_oobe_enabled(true);
            break;

        case TJC_PAGE_NO_UPDATA_BTN_FILE:

            /*
            page_to(TJC_PAGE_FILE_LIST_1);
            printer_set_babystep();
            page_files_pages = 0;
            page_files_current_pages = 0;
            page_files_folder_layers = 0;
            page_files_previous_path = "";
            page_files_root_path = DEFAULT_DIR;
            page_files_path = "";
            refresh_page_files(page_files_current_pages);
            refresh_page_files_list_1();

            get_object_status();
            */
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
            //if (detect_disk() == 0) {  //2023.5.8 CLL 去除U盘判断条件
                page_to(TJC_PAGE_FILE_LIST_1);
                // printer_set_babystep();
                page_files_pages = 0;
                page_files_current_pages = 0;
                page_files_folder_layers = 0;
                page_files_previous_path = "";
                page_files_root_path = DEFAULT_DIR;
                // page_files_path = "/sda1";
                page_files_path = "";
                refresh_page_files(page_files_current_pages);
                refresh_page_files_list_1();

                get_object_status();
            }
            //} else {
            //    page_to(TJC_PAGE_DISK_DETECT_2);
            //}
            
            break;

        case TJC_PAGE_NO_UPDATA_BTN_HOME:
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
                page_to(TJC_PAGE_MAIN);
                get_object_status();
            }
            break;

        case TJC_PAGE_NO_UPDATA_BTN_TOOL:
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
                page_to(TJC_PAGE_MOVE);
                get_object_status();
            }
            break;

        case TJC_PAGE_NO_UPDATA_LANGUAGE:
            if (printer_webhooks_state != "shutdown" && printer_webhooks_state != "error") {
            page_to(TJC_PAGE_LANGUAGE);
            get_object_status();
            }
            break;

        case TJC_PAGE_NO_UPDATA_SERVICE:
            page_to(TJC_PAGE_SERVICE);
            get_object_status();
            break;

        case TJC_PAGE_NO_UPDATA_RESET:
            go_to_reset();
            get_object_status();
            break;

        case TJC_PAGE_NO_UPDATA_OOBE:
            std::cout << "开机引导页面" << std::endl;
            if (get_mks_oobe_enabled() == true) {
                set_mks_oobe_enabled(false);
                send_cmd_picc(tty_fd, "b1", "132");
            } else {
                set_mks_oobe_enabled(true);
                send_cmd_picc(tty_fd, "b1", "367");
            }
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_PRINT_ZOFFSET:
        switch (widget_id)
        {
        case TJC_PAGE_PRINT_ZOFFSET_001:
            // set_intern_zoffset(0.005);
            set_intern_zoffset(0.01);
            break;

        case TJC_PAGE_PRINT_ZOFFSET_01:
            // set_intern_zoffset(0.025);
            set_intern_zoffset(0.1);
            break;

        case TJC_PAGE_PRINT_ZOFFSET_005:
            // set_intern_zoffset(0.01);
            set_intern_zoffset(0.05);
            break;

        case TJC_PAGE_PRINT_ZOFFSET_1:
            // set_intern_zoffset(0.05);
            set_intern_zoffset(1);
            break;

        case TJC_PAGE_PRINT_ZOFFSET_UP:
            set_zoffset(false);
            break;

        case TJC_PAGE_PRINT_ZOFFSET_DOWN:
            set_zoffset(true);
            break;

        case TJC_PAGE_PRINT_ZOFFSET_PAUSE_RESUME:
            // 只要暂停就跳到换料页面
            //2023.5.11 CLL 修复页面跳转bug
            printer_ready = false;
            set_print_pause();
            //4.2.2 CLL 优化页面跳转
            page_to(TJC_PAGE_PRINT_FILAMENT);
            clear_page_printing_arg();
            /*
            if (get_print_pause_resume() == false) {
                set_print_pause();
                // page_to(TJC_PAGE_PRINT_FILAMENT);
            } else {
                set_print_resume();
            }
            */
            break;

        case TJC_PAGE_PRINT_ZOFFSET_STOP:
            page_to(TJC_PAGE_STOP_PRINT);
            break;

        case TJC_PAGE_PRINT_ZOFFSET_BACK:
            printing_keyboard_enabled = false;
            page_to(TJC_PAGE_PRINTING);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_PRINT_KEYBOARD:
        switch (widget_id)
        {
        case TJC_PAGE_PRINT_KEYBOARD_BACK:
            page_to(TJC_PAGE_PRINTING);
            printing_keyboard_enabled = false;
            break;
        }
        break;

    case TJC_PAGE_PRINT_FINISH:
        switch (widget_id)
        {
        case TJC_PAGE_PRINT_FINISH_YES:
            finish_print();
            // get_total_time();
            page_to(TJC_PAGE_MAIN);
            break;

        // case TJC_PAGE_PRINT_FINISH_NO:
            // break;
        }
        break;

    case TJC_PAGE_STOP_PRINT:
        switch (widget_id)
        {
        case TJC_PAGE_STOP_PRINT_YES:
            // if (start_to_printing == false) {
                // start_to_printing = true;
                page_to(TJC_PAGE_STOPPING);
                cancel_print();
                // get_total_time();
            // }
            break;

        case TJC_PAGE_STOP_PRINT_NO:
            page_to(TJC_PAGE_PRINTING);
            break;
        }
        break;

    case TJC_PAGE_SET_ZOFFSET_2:
        switch (widget_id)
        {
        case TJC_PAGE_SET_ZOFFSET_2_001:
            break;

        case TJC_PAGE_SET_ZOFFSET_2_0025:
            break;

        case TJC_PAGE_SET_ZOFFSET_2_005:
            break;

        case TJC_PAGE_SET_ZOFFSET_2_1:
            break;

        case TJC_PAGE_SET_ZOFFSET_2_DOWN:
            break;

        case TJC_PAGE_SET_ZOFFSET_2_UP:
            break;

        case TJC_PAGE_SET_ZOFFSET_2_YES:
            page_to(TJC_PAGE_LEVELING_INIT);
            fresh_page_set_zoffset_data = false;
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_KEYBDB:
        switch (widget_id)
        {
        case TJC_PAGE_KEYBDB_PAUSE:
            if (get_print_pause_resume() == false) {
                set_print_pause();
                page_to(TJC_PAGE_PRINT_FILAMENT);
                clear_page_printing_arg();
            } else {
                set_print_resume();
            }
            break;

        case TJC_PAGE_KEYBDB_STOP:
            page_to(TJC_PAGE_STOP_PRINT);
            clear_page_printing_arg();
            break;

        case TJC_PAGE_KEYBDB_BACK:
            printing_keyboard_enabled = false;
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_POP_1:
        switch (widget_id)
        {
        case TJC_PAGE_POP_1_YES:
            if (previous_page_id == TJC_PAGE_MOVE) {
                // set_move_dist(0.1);
                page_to(TJC_PAGE_MOVE);
            } else if (previous_page_id == TJC_PAGE_AUTO_LEVEL) {
                std::cout << "按下确定键" << std::endl;
                page_to(TJC_PAGE_MAIN);
            } else if (previous_page_id == TJC_PAGE_PRINTING || current_page_id == TJC_PAGE_PRINT_ZOFFSET || current_page_id == TJC_PAGE_PRINT_FILAMENT) {
                page_to(TJC_PAGE_MAIN);
            } else {
                std::cout << "TJC_PAGE_POP_1_YES" << std::endl;
                page_to(previous_page_id);
            }
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_KB_FILAMENT_1:
    // case TJC_PAGE_KB_FILAMENT_2:
    // case TJC_PAGE_KB_FILAMENT_3:
        switch (widget_id)
        {
        case TJC_PAGE_KB_FILAMENT_1_BACK:
            page_to(TJC_PAGE_FILAMENT);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_DISK_DETECT:
        switch (widget_id)
        {
        case TJC_PAGE_DISK_DETECT_ENTER:
            page_to(previous_page_id);
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_UPDATED:
        switch (widget_id)
        {
        case TJC_PAGE_UPDATED_ENTER:
            if (previous_page_id == TJC_PAGE_LOGO) {
                finish_tjc_update();
                page_to(TJC_PAGE_MAIN);
            }
            break;

        default:
            break;
        }

    case TJC_PAGE_SAVING:
        break;

    case TJC_PAGE_SET_TEMP_LEVEL:
        switch (widget_id)
        {
        case TJC_PAGE_SET_TEMP_LEVEL_HOME:
            break;

        case TJC_PAGE_SET_TEMP_LEVEL_FILE:
            break;

        case TJC_PAGE_SET_TEMP_LEVEL_TOOL:
            break;

        case TJC_PAGE_SET_TEMP_LEVEL_SERVICE:
            break;

        case TJC_PAGE_SET_TEMP_LEVEL_BACK:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;

        case TJC_PAGE_SET_TEMP_LEVEL_EXTRUDER:
            page_to(TJC_PAGE_SET_TEMP_LEVEL_KB);
            break;

        case TJC_PAGE_SET_TEMP_LEVEL_HOT_BED:
            page_to(TJC_PAGE_SET_TEMP_LEVEL_KB);
            break;

        case TJC_PAGE_SET_TEMP_LEVEL_ENTER:
            level_mode_printing_set_target();
            level_mode_printing_is_printing_level = true;
            printer_print_stats_state = "printing";
            level_mode_printing_print_file();
            page_to(TJC_PAGE_LEVEL_PRINT);
            break;

        case TJC_PAGE_LEVEL_PRINT:
            break;

        case TJC_PAGE_LEVEL_STOP:
            break;

        case TJC_PAGE_CHOOSE_MODEL:
            switch (widget_id)
            {
            case TJC_PAGE_CHOOSE_MODEL_01:
                std::cout << "选择了第1个 zoffset" << std::endl;
                break;

            case TJC_PAGE_CHOOSE_MODEL_02:
                std::cout << "选择了第2个 zoffset" << std::endl;
                break;

            case TJC_PAGE_CHOOSE_MODEL_03:
                std::cout << "选择了第3个 zoffset" << std::endl;
                break;

            case TJC_PAGE_CHOOSE_MODEL_04:
                std::cout << "选择了第4个 zoffset" << std::endl;
                break;

            case TJC_PAGE_CHOOSE_MODEL_05:
                std::cout << "选择了第5个 zoffset" << std::endl;
                break;

            case TJC_PAGE_CHOOSE_MODEL_06:
                std::cout << "选择了第6个 zoffset" << std::endl;
                break;

            case TJC_PAGE_CHOOSE_MODEL_07:
                std::cout << "选择了第7个 zoffset" << std::endl;
                break;

            case TJC_PAGE_CHOOSE_MODEL_08:
                std::cout << "选择了第8个 zoffset" << std::endl;
                break;

            case TJC_PAGE_CHOOSE_MODEL_09:
                std::cout << "选择了第9个 zoffset" << std::endl;
                break;

            default:
                break;
            }
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_PID_WORKING:
        break;

    case TJC_PAGE_PID_FINISH:
        break;

    case TJC_PAGE_DISK_DETECT_2:
        switch (widget_id)
        {
        case TJC_PAGE_DISK_DETECT_2_HOME:
            page_to(TJC_PAGE_MAIN);
            break;

        case TJC_PAGE_DISK_DETECT_2_TOOL:
            page_to(TJC_PAGE_MOVE);
            break;

        case TJC_PAGE_DISK_DETECT_2_SERVICE:
            page_to(TJC_PAGE_SERVICE);
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_WIFI_SUCCESS:
        switch (widget_id)
        {
        case TJC_PAGE_WIFI_SUCCESS_YES:
            wifi_save_config();
            // page_to(TJC_PAGE_WIFI_SAVE);
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_WIFI_FAILED:
        switch (widget_id)
        {
        case TJC_PAGE_WIFI_FAILED_YES:
            page_to(TJC_PAGE_WIFI_LIST_2);
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_X_CLEAR_MAX3:
        switch (widget_id)
        {
        case TJC_PAGE_X_CLEAR_MAX3_YES:
            page_to(TJC_PAGE_CLEAR_X_VIDEO);
            break;

        case TJC_PAGE_X_CLEAR_MAX3_NO:
            do_not_x_clear();
            page_to(TJC_PAGE_PREVIEW);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_X_CLEAR_PLUS3:
        switch (widget_id)
        {
        case TJC_PAGE_X_CLEAR_PLUS3_YES:
            page_to(TJC_PAGE_CLEAR_X_VIDEO);
            break;

        case TJC_PAGE_X_CLEAR_PLUS3_NO:
            do_not_x_clear();
            page_to(TJC_PAGE_PREVIEW);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_CLEAR_X_VIDEO:
        switch (widget_id)
        {
        case TJC_PAGE_CLEAR_X_VIDEO_NEXT:
            // 暂时用page_to(main)
            // page_to(TJC_PAGE_MAIN);
            do_x_clear();

            go_to_syntony_move();       // 按照要求跳转完成后跳到共振补偿
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_WIFI_KEYBOARD:
        switch (widget_id)
        {
        case TJC_PAGE_WIFI_KEYBOARD_BACK:
            page_to(TJC_PAGE_WIFI_LIST_2);
            printing_wifi_keyboard_enabled = false;
            break;

        case TJC_PAGE_WIFI_KEYBOARD_ENTER:
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_WAIT_POP:
        switch (widget_id)
        {
        case TJC_PAGE_WAIT_POP_OK:
            page_to(previous_page_id);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_FILAMENT_POP_3:
        switch (widget_id)
        {
        case TJC_PAGE_FILAMENT_POP_3_OK:
            page_to(TJC_PAGE_PRINT_FILAMENT);
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_FILAMENT_POP_5:
        switch (widget_id)
        {
        case TJC_PAGE_FILAMENT_POP_5_OK:
            page_to(TJC_PAGE_FILAMENT);
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_FILAMENT_SHOWDOWN:
        switch (widget_id)
        {
        default:
            break;
        }
        break;

    //2023.5.8 CLL 修改自动调平页面跳转
    case TJC_PAGE_LEVEL_NULL_1:
        switch (widget_id)
        {
        case TJC_PAGE_LEVEL_NULL_1_DOWN:
            set_auto_level_heater_bed_target(false);
            break;
        
        case TJC_PAGE_LEVEL_NULL_1_UP:
            set_auto_level_heater_bed_target(true);
            break;

        case TJC_PAGE_LEVEL_NULL_1_ON_OFF:
            filament_heater_bed_target();
            break;

        case TJC_PAGE_LEVEL_NULL_1_BACK:
            page_to(TJC_PAGE_LEVEL_MODE);
            break;

        case TJC_PAGE_LEVEL_NULL_1_ENTER:
            page_to(TJC_PAGE_LEVEL_NULL_2);
            break;

        default:
            break;
        }
        break;

    //2023.5.8 CLL 修改自动调平页面跳转
    case TJC_PAGE_LEVEL_NULL_2:
        switch(widget_id)
        {
        case TJC_PAGE_LEVEL_NULL_2_BACK:
            page_to(TJC_PAGE_LEVEL_NULL_1);
            break;

        case TJC_PAGE_LEVEL_NULL_2_ENTER:
            std::cout << "进入到这里" << std::endl;
            auto_level_button_enabled = true;
            printer_idle_timeout_state = "Printing";
            pre_auto_level_init();
            break;

        default:
            break;
        }
        break;

    //2023.5.8 CLL 报错弹窗
    case TJC_PAGE_DETECT_ERROR:
        switch(widget_id)
        {
        case TJC_PAGE_DETECT_ERROR_YES:
            page_to(TJC_PAGE_MAIN);
            break;

        default:
            break;
        }
        break;
    
    //2023.5.8 CLL 报错弹窗
    case TJC_PAGE_GCODE_ERROR:
        switch(widget_id)
        {
        case TJC_PAGE_GCODE_ERROR_YES:
            page_to(TJC_PAGE_STOPPING);
            cancel_print();
            break;
        
        default:
            break;
        }
        break;

    //2023.5.11 CLL 修改开机引导自动调平页面跳转
    case TJC_PAGE_OPEN_HEATER_BED:
        switch(widget_id)
        {
        case TJC_PAGE_OPEN_HEATER_BED_DOWN:
            set_auto_level_heater_bed_target(false);
            break;
        
        case TJC_PAGE_OPEN_HEATER_BED_UP:
            set_auto_level_heater_bed_target(true);
            break;

        case TJC_PAGE_OPEN_HEATER_BED_ON_OFF:
            filament_heater_bed_target();
            break;

        case TJC_PAGE_OPEN_HEATER_BED_NEXT:
            page_to(TJC_PAGE_NULL_2);
            break;

        default:
            break;
        }
        break;

    //2023.5.12 CLL 开机引导跳过确认弹窗
    case TJC_PAGE_OPEN_POP_1:
        switch(widget_id)
        {
        case TJC_PAGE_OPEN_POP_1_NO:
            page_to(TJC_PAGE_OPEN_LANGUAGE);
            break;
        
        case TJC_PAGE_OPEN_POP_1_YES:
            page_to(TJC_PAGE_MAIN);
            set_mks_oobe_enabled(false);
            break;
        
        default:
            break;
        }
        break;

    //2.1.2 CLL 打印前判断耗材种类并弹窗
    case TJC_PAGE_PREVIEW_POP_1:
    case TJC_PAGE_PREVIEW_POP_2:
        switch(widget_id)
        {
        case TJC_PAGE_PREVIEW_POP_YES:
            page_to(TJC_PAGE_PRINTING);
            break;
        }

    default:
        break;
    }
}

void tjc_event_setted_handler(int page_id, int widget_id, unsigned char first, unsigned char second) {
    std::cout << "!!!" << page_id << std::endl;
    std::cout << "!!!" << widget_id << std::endl;
    std::cout << "!!!" << first << std::endl;
    std::cout << "!!!" << second << std::endl;
    int number = (int)((second << 8) + first);
    switch (page_id)
    {
    case TJC_PAGE_PRINTING:
        switch (widget_id)
        {
        case TJC_PAGE_PRINTING_SET_EXTRUDER:
            std::cout << "********************" << number << std::endl;
            if (number > 350) {
                number = 350;
            }
            page_to(TJC_PAGE_PRINTING);
            printing_keyboard_enabled = false;
            set_extruder_target(number);
            send_cmd_val(tty_fd, "n0", std::to_string(number));
            // set_mks_extruder_target(number);
            break;

        case TJC_PAGE_PRINTING_SET_HEATER_BED:
            std::cout << "********************" << number << std::endl;
            if (number > 120) {
                number = 120;
            }
            page_to(TJC_PAGE_PRINTING);
            printing_keyboard_enabled = false;
            set_heater_bed_target(number);
            send_cmd_val(tty_fd, "n1", std::to_string(number));
            // set_mks_heater_bed_target(number);
            break;
        
        case TJC_PAGE_PRINTING_SET_HOT:
            std::cout << "********************" << number << std::endl;
            //2023.4.25-2 修改热腔温度上限
            if (number > 65) {
                number = 65;
            } 
            page_to(TJC_PAGE_PRINTING);
            printing_keyboard_enabled = false;
            set_hot_target(number);
            send_cmd_val(tty_fd, "n2", std::to_string(number));
            // set_mks_hot_target(number);
            break;

        case TJC_PAGE_PRINTING_SET_FAN_0:
            std::cout << "********************" << number << std::endl;
            if (number > 100) {
                number = 100;
            }
            page_to(TJC_PAGE_PRINTING);
            printing_keyboard_enabled = false;
            set_fan0(number);
            send_cmd_val(tty_fd, "n3", std::to_string(number));
            break;

        case TJC_PAGE_PRINTING_SET_FAN_1:
            std::cout << "********************" << number << std::endl;
            if (number > 100) {
                number = 100;
            }
            page_to(TJC_PAGE_PRINTING);
            printing_keyboard_enabled = false;
            set_fan2(number);
            send_cmd_val(tty_fd, "n4", std::to_string(number));
            break;

        //2.1.2 CLL 新增fan3
        case TJC_PAGE_PRINTING_SET_FAN_3:
            std::cout << "********************" << number << std::endl;
            if (number > 100) {
                number = 100;
            }
            page_to(TJC_PAGE_PRINTING);
            printing_keyboard_enabled = false;
            set_fan3(number);
            send_cmd_val(tty_fd, "n9", std::to_string(number));
            break;

        case TJC_PAGE_PRINTING_SET_SPEED:
            std::cout << "********************" << number << std::endl;
            //2.1.2 CLL 限制喷头移速和挤出上限为150%
            if (number > 150) {
                number = 150;
            }
            page_to(TJC_PAGE_PRINTING);
            printing_keyboard_enabled = false;
            set_printer_speed(number);
            send_cmd_val(tty_fd, "n5", std::to_string(number));
            break;

        case TJC_PAGE_PRINTING_SET_FLOW:
            std::cout << "********************" << number << std::endl;
            //2.1.2 CLL 限制喷头移速和挤出上限为150%
            if (number > 150) {
                number = 150;
            }
            page_to(TJC_PAGE_PRINTING);
            printing_keyboard_enabled = false;
            set_printer_flow(number);
            send_cmd_val(tty_fd, "n6", std::to_string(number));
            break;
        
        default:
            break;
        }
        break;

    case TJC_PAGE_FILAMENT:
        switch (widget_id)
        {
        case TJC_PAGE_FILAMENT_BTN_EXTRUDER:
        // case 0x0b:
            std::cout << "********************" << number << std::endl;
            // filament_keyboard_enabled = false;
            if (number > 350) {
                number = 350;
            }
            set_extruder_target(number);
            set_mks_extruder_target(number);
            page_to(TJC_PAGE_FILAMENT);
            break;

        case TJC_PAGE_FILAMENT_BTN_HEATER_BED:
        // case 0x0c:
            std::cout << "********************" << number << std::endl;
            // filament_keyboard_enabled = false;
            if (number > 120) {
                number = 120;
            }
            set_heater_bed_target(number);
            set_mks_heater_bed_target(number);
            page_to(TJC_PAGE_FILAMENT);
            break;

        case TJC_PAGE_FILAMENT_BTN_HOT:
        // case 0x0d:
            std::cout << "********************" << number << std::endl;
            // filament_keyboard_enabled = false;
            //2023.4.25-2 修改热腔温度上限
            if (number > 65) {
                number = 65;
            }
            set_hot_target(number);
            set_mks_hot_target(number);
            page_to(TJC_PAGE_FILAMENT);
            break;

        case TJC_PAGE_FILAMENT_BTN_FAN_1:
            /*
            std::cout << "********************" << number << std::endl;
            if (number > 100) {
                number = 100;
            }
            printing_keyboard_enabled = false;
            set_fan0(number);
            page_to(TJC_PAGE_FILAMENT);
            */
            break;

        case TJC_PAGE_FILAMENT_BTN_FAN_2:
            /*
            std::cout << "********************" << number << std::endl;
            if (number > 100) {
                number = 100;
            }
            printing_keyboard_enabled = false;
            set_fan2(number);
            page_to(TJC_PAGE_FILAMENT);
            */
            break;

        case TJC_PAGE_FILAMENT_BTN_FILAMENT_SENSOR:
            // set_filament_sensor();
            break;

        default:
            break;
        }
        break;

    case TJC_PAGE_SET_TEMP_LEVEL:
        switch (widget_id)
        {
        case TJC_PAGE_SET_TEMP_LEVEL_EXTRUDER:
            std::cout << "********************" << number << std::endl;
            level_mode_printing_extruder_target = number;
            page_to(TJC_PAGE_SET_TEMP_LEVEL);
            break;

        case TJC_PAGE_SET_TEMP_LEVEL_HOT_BED:
            std::cout << "********************" << number << std::endl;
            level_mode_printing_heater_bed_target = number;
            page_to(TJC_PAGE_SET_TEMP_LEVEL);
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
}

void tjc_event_keyboard(char *cmd) {
    MKSLOG("收到的键盘返回的值为 %s\n", cmd);
    MKSLOG("cmd 1  %d\n", cmd[1]);
    MKSLOG("cmd 2  %d\n", cmd[2]);
    char *psk = &cmd[3];
    // int page;
    // int widget;
    // int type;
    // page = cmd[1];
    // widget = cmd[2];
    // type = cmd[3];
    switch (cmd[1]) {
    case TJC_PAGE_WIFI_LIST_2:
        switch (cmd[2])
        {
        case TJC_PAGE_WIFI_LIST_2_SSID_1:
            MKSLOG_RED("收到密码, %d, 密码是%s", strlen(psk), psk);
            printing_wifi_keyboard_enabled = false;
            // page_to(TJC_PAGE_WIFI_LIST_2);
            page_to(TJC_PAGE_WIFI_CONNECT);
            print_ssid_psk(psk);
            break;

        case TJC_PAGE_WIFI_LIST_2_SSID_2:
            MKSLOG_RED("收到密码, %d, 密码是%s", strlen(psk), psk);
            printing_wifi_keyboard_enabled = false;
            page_to(TJC_PAGE_WIFI_CONNECT);
            print_ssid_psk(psk);
            break;

        case TJC_PAGE_WIFI_LIST_2_SSID_3:
            MKSLOG_RED("收到密码, %d, 密码是%s", strlen(psk), psk);
            
            printing_wifi_keyboard_enabled = false;
            // page_to(TJC_PAGE_WIFI_LIST_2);
            page_to(TJC_PAGE_WIFI_CONNECT);
            print_ssid_psk(psk);
            break;

        case TJC_PAGE_WIFI_LIST_2_SSID_4:
            MKSLOG_RED("收到密码, %d, 密码是%s", strlen(psk), psk);
            
            printing_wifi_keyboard_enabled = false;
            // page_to(TJC_PAGE_WIFI_LIST_2);
            page_to(TJC_PAGE_WIFI_CONNECT);
            print_ssid_psk(psk);
            break;

        case TJC_PAGE_WIFI_LIST_2_SSID_5:
            MKSLOG_RED("收到密码, %d, 密码是%s", strlen(psk), psk);
            
            printing_wifi_keyboard_enabled = false;
            // page_to(TJC_PAGE_WIFI_LIST_2);
            page_to(TJC_PAGE_WIFI_CONNECT);
            print_ssid_psk(psk);
            break;

        default:
            break;
        }
        break;
    }
}
