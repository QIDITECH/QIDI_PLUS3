#include "../include/MakerbaseParseIni.h"

dictionary *mksini = NULL;

dictionary *printer_cfg = NULL;

dictionary *mksversion = NULL;

bool file_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

void ensure_directory_exists(const std::string &path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
        mkdir(path.c_str(), 0755);
    }
}

bool is_file_locked(const char *path) {
    int fd = open(path, O_WRONLY | O_CREAT, 0666);
    if (fd == -1) {
        return true;
    }
    close(fd);
    return false;
}

int mksini_load() 
{
    if (!file_exists(INIPATH)) {
        std::cout << "Config file does not exist. Creating a new one." << std::endl;
        FILE *file = fopen(INIPATH, "w");
        if (file == NULL) {
            std::cerr << "Failed to create the config file!" << std::endl;
            return -1;
        }
        fclose(file);
    }

    mksini = iniparser_load(INIPATH);

    if (mksini == NULL) {
        std::cout << "Ini parse failure!" << std::endl;
        return -1;
    }

    return 0;
}

int mksini_load_from_path(const char *path) {
    if (!file_exists(path)) {
        std::cout << "Config file does not exist. Creating a new one." << std::endl;
        FILE *file = fopen(path, "w");
        if (file == NULL) {
            std::cerr << "Failed to create the config file!" << std::endl;
            return -1;
        }
        fclose(file);
    }

    mksini = iniparser_load(path);

    if (mksini == NULL) {
        std::cout << "Ini parse failure!" << std::endl;
        return -1;
    }

    return 0;
}

void mksini_free() {
    iniparser_freedict(mksini);
}

std::string mksini_getstring(std::string section, std::string key, std::string def) {
    std::string sk = section + ":" + key;
    const char *value = iniparser_getstring(mksini, sk.c_str(), def.c_str());
    return (std::string)value;
}

int mksini_getint(std::string section, std::string key, int notfound) {
    std::string sk = section + ":" + key;
    int value = iniparser_getint(mksini, sk.c_str(), notfound);
    return value;
}

double mksini_getdouble(std::string section, std::string key, double notfound) {
    std::string sk = section + ":" + key;
    double value = iniparser_getdouble(mksini, sk.c_str(), notfound);
    return value;
}

bool mksini_getboolean(std::string section, std::string key, int notfound) {
    std::string sk = section + ":" + key;
    int value = iniparser_getboolean(mksini, sk.c_str(), notfound);
    return (value == 0) ? false : true;
}

// 用于读取指定节的所有字段并存储在map中
std::map<std::string, std::string> mksini_getsection(const std::string &section) {
    std::map<std::string, std::string> section_map;

    int keys = iniparser_getsecnkeys(mksini, section.c_str());
    if (keys <= 0) {
        std::cout << "Section " << section << " not found or empty!" << std::endl;
        return section_map;
    }

    const char **key_list = new const char *[keys];
    iniparser_getseckeys(mksini, section.c_str(), key_list);
    for (int i = 0; i < keys; ++i) {
        std::string key(key_list[i] + section.length() + 1); // 移除section前缀
        std::string value = mksini_getstring(section, key, "");
        if (!value.empty()) {
            section_map[key] = value;
        }
    }
    delete[] key_list;

    return section_map;
}

int mksini_set(std::string section, std::string key, std::string value) {
    int ret = 0;

	// set section
	ret = iniparser_set(mksini, section.c_str(), NULL);
	if (ret < 0) {
		fprintf(stderr, "cannot set section %s in: %s\n", section, INIPATH);
        return -1;
	}

    std::string sk = section + ":" + key;

	// set key/value pair 
	ret = iniparser_set(mksini, sk.c_str(), value.c_str());
	if (ret < 0) {
		fprintf(stderr, "cannot set key/value %s in: %s\n", sk, INIPATH);
        return -1;
	}

    return ret;
}

void mksini_unset(std::string section, std::string key) {
    std::string sk = section + ":" + key;
    iniparser_unset(mksini, sk.c_str());
}

// 保存到配置文件
void mksini_save() {
    ensure_directory_exists(CONFIG_PATH);

    int retries = 5;
    while (retries--) {
        if (!is_file_locked(INIPATH)) {
            FILE *ini = fopen(INIPATH, "w");
            if (ini == NULL) {
                std::cerr << "open mksini failed" << std::endl;
                return;
            }
            iniparser_dump_ini(mksini, ini);
            fclose(ini);
            return;
        }
        sleep(1);
    }
    std::cerr << "save mksini failed after multiple attempts" << std::endl;
}

// 单次完整调用的set 包含载入保存和释放
int mksini_update(std::string section, std::string key, std::string value)
{
    mksini_load();
    int result = mksini_set(section, key, value);
    mksini_save();
    mksini_free();
    return result;
}

int mksversion_load() {
    mksversion = iniparser_load(VERSION_PATH);

    if (mksversion == NULL) {
        std::cout << "Mks version failure!" << std::endl;
        return -1;
    }

    return 0;
}

void mksversion_free() {
    iniparser_freedict(mksversion);
}

// CLL 用于获取在线更新信息
int updateini_load() {
    return mksini_load_from_path("/root/makerbase-client/update_info.ini");
}

// CLL 用于获取在线更新进度
int progressini_load() {
    mksini = iniparser_load("/root/auto_update/update_progress.ini");
    
    if (mksini == NULL) {
        std::cout << "Ini parse failure" << std::endl;
        return -1;
    }

    return 0;
}

std::string mksversion_mcu(std::string def) {
    std::string version = "version:mcu";
    const char *value = iniparser_getstring(mksversion, version.c_str(), def.c_str());
    return (std::string)value;
}

std::string mksversion_ui(std::string def) {
    std::string version = "version:ui";
    const char *value = iniparser_getstring(mksversion, version.c_str(), def.c_str());
    return (std::string)value;
}

std::string mksversion_soc(std::string def) {
    std::string version = "version:soc";
    const char *value = iniparser_getstring(mksversion, version.c_str(), def.c_str());
    return (std::string)value;
}
