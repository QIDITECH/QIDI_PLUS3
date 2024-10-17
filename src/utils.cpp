#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>  // std::stringstream buffer;
#include <stdexcept>
#include <string>
#include <unistd.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <boost/system/error_code.hpp>

// 图片转化库
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"

#include "../include/mks_log.h"
#include "../include/MakerbaseParseIni.h"
#include "../include/qrcodegen.hpp"
#include "../include/utils.h"

namespace fs = boost::filesystem;
using namespace qrcodegen;

// 系统命令执行 可以获取具体输出信息
std::string execute_command(const std::string &cmd) 
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

// 移除文件夹
void remove_folder(const fs::path& path) 
{
    boost::system::error_code ec;
    if (fs::exists(path, ec)) 
    {
        if (fs::remove_all(path, ec)) 
            MKSLOG("Info: %s removed successfully", path.string().c_str());
        else 
            MKSLOG_RED("Error: failed to remove %s", path.string().c_str());
    } 
    else
        MKSLOG("Info: %s does not exist", path.string().c_str());
}

// 移除文件
void remove_file(const fs::path& path) 
{
    boost::system::error_code ec;
    if (fs::exists(path, ec)) 
    {
        if (fs::remove(path, ec)) 
            MKSLOG("Info: %s removed successfully", path.string().c_str());
        else 
            MKSLOG_RED("Error: failed to remove %s", path.string().c_str());
    } 
    else
        MKSLOG("Info: %s does not exist", path.string().c_str());
}

// 判断服务是否存在
bool check_service_exists(const std::string& service_name) 
{
    std::string cmd = "systemctl list-units --full --all | grep -q " + service_name;
    std::string result = execute_command(cmd);
    return !result.empty();
}

// 清除废弃的服务文件
bool clear_deprecated_services()
{
    try 
    {
        remove_folder("/root/auto_update");

        if (check_service_exists("frpc.service")) 
        {
            MKSLOG("Info: frpc.service exists, removing...");

            system("systemctl disable frpc.service");
            remove_folder("/root/frp");
            remove_file("/etc/systemd/system/frpc.service");
            remove_file("/etc/logrotate.d/frp");
            system("systemctl daemon-reload");
        } 
        else
            MKSLOG("Info: %s", "frpc.service does not exist");

    } 
    catch (const std::exception& e)
    {
        MKSLOG_RED("exception: %s", e.what());
        return false;
    }
    return true;
}

// 去除MAC地址中的: 用于frpc.json拼接设备名
std::string sanitize_mac_address(const std::string& mac) 
{
    std::string sanitized_mac = mac;
    sanitized_mac.erase(std::remove(sanitized_mac.begin(), sanitized_mac.end(), ':'), sanitized_mac.end());
    return sanitized_mac;
}

// 生成二维码
bool generate_qrcode(const std::string& data, const std::string& output_path, int qr_size)
{
    try
    {
        QrCode qr = QrCode::encodeText(data.c_str(), QrCode::Ecc::LOW);
        int size = qr.getSize();
        int border = 4; // 边框大小
        // int imgSize = (size + 2 * border) * qrSize / size; // 最终图像大小
        int img_size = (size + 2 * border); // 最终图像大小

        std::vector<uint8_t> img(img_size * img_size, 255); // 创建白色背景图像

        // 填充二维码数据
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                if (qr.getModule(x, y)) {
                    int img_x = border + x;
                    int img_y = border + y;
                    img[img_y * img_size + img_x] = 0; // 设置模块为黑色
                }
            }
        }

        // O(n^3) 时间复杂度的 图像放大 function
        std::vector<uint8_t> final_img(qr_size * qr_size, 255);
        const int scale = qr_size / img_size; // 放大比例

        for (int y = 0; y < img_size; ++y) {
            int final_y = y * scale; // 计算最终Y坐标的起始位置
            for (int x = 0; x < img_size; ++x) {
                uint8_t color = img[y * img_size + x]; // 获取颜色
                int final_x_start = x * scale; // 计算最终X坐标的起始位置
                uint8_t* row_start = &final_img[final_y * qr_size + final_x_start]; // 获取最终图像的行起始指针
                for (int dy = 0; dy < scale; ++dy) {
                    std::fill_n(row_start + dy * qr_size, scale, color); // 填充一行
                }
            }
        }

        // 保存图像为 PNG 文件
        stbi_write_png(output_path.c_str(), img_size, img_size, 1, img.data(), img_size);
    }
    catch (const std::exception& e)
    {
        MKSLOG_RED("[Error] Exception caught: %s", e.what());
        return false;
    }

    return true;
}

// 检查输入的UTF-8字符串是否符合给定的字符限制
size_t check_utf8_strlen(const std::string& str, size_t max_length)
{
    size_t length = 0;
    for (size_t i = 0; i < str.size(); ++i) 
    {
        if ((str[i] & 0xC0) != 0x80)
            ++length;
    }
    return length <= max_length;
}