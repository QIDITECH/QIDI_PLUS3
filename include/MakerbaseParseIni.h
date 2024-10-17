#ifndef MAKERBASEPARSEINI_H
#define MAKERBASEPARSEINI_H

#define XINDI_PLUS 1
#define XINDI_MAX 0
#define XINDI_MINI 0

#include <iostream>
#include <string>
#include <map>
#include <cstdio>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "./dictionary.h"
#include "./iniparser.h"

//4.3.7 CLL 修改配置文件保存位置
//#define INIPATH "/root/config.mksini"
#define INIPATH "/home/mks/klipper_config/config.mksini"
#define CONFIG_PATH "/home/mks/klipper_config"

#ifdef XINDI_PLUS
#define VERSION_PATH "/root/xindi/version"
#elif XINDI_MAX
#define VERSION_PATH "/root/xindi/version-max"
#elif XINDI_MINI
#define VERSION_PATH "/root/xindi/version-4-3"
#endif

int mksini_load();
int mksini_load_from_path(const char * path);
void mksini_free();
std::string mksini_getstring(std::string section, std::string key, std::string def);
int mksini_getint(std::string section, std::string key, int notfound);
double mksini_getdouble(std::string section, std::string key, double notfound);
bool mksini_getboolean(std::string section, std::string key, int notfound);
std::map<std::string, std::string> mksini_getsection(const std::string &section);
int mksini_set(std::string section, std::string key, std::string value);
void mksini_unset(std::string section, std::string key);
void mksini_save();
int mksini_update(std::string section, std::string key, std::string value);

int mksversion_load();
void mksversion_free();
std::string mksversion_mcu(std::string def);
std::string mksversion_ui(std::string def);
std::string mksversion_soc(std::string def);

int updateini_load();
int progressini_load();

#endif
