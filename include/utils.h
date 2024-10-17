#ifndef UTILS_H
#define UTILS_H

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

// 系统命令
std::string execute_command(const std::string &cmd);
void remove_folder(const fs::path& path);
void remove_file(const fs::path& path);

// Q1用的移除服务
bool clear_deprecated_services();
bool check_service_exists(const std::string& service_name);

// 移除mac地址中的:
std::string sanitize_mac_address(const std::string& mac);

// 生成二维码图片到指定路径
bool generate_qrcode(const std::string& data, const std::string& output_path, int qr_size);

// 验证UTF-8字符串长度
size_t check_utf8_strlen(const std::string& str, size_t max_length);

#endif