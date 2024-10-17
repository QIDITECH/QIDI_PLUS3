#ifndef CURLWRAPPER_H
#define CURLWRAPPER_H


#include <map>
#include <mutex>
#include <memory>
#include <string>
#include <fstream>
#include <iostream>
#include <functional>
#include <curl/curl.h>

#include "../include/mks_log.h"

#define SUCCESS 0
#define CURL_GENERAL_EXCEPTION 1000
#define CURL_TIMEOUT 1001
#define CURL_INITIALIZATION_FAIL 1002
#define CURL_FILE_OPEN_FAIL 1003
#define CURL_RESOURCE_NOT_FOUND 1004
#define CURL_BAD_REQUEST 1005
#define CURL_SERVER_ERROR 1006
#define CURL_SSL_VERIFICATION_FAIL 1008
#define CURL_DNS_RESOLVE_FAIL 1009
#define CURL_DISK_FULL 1010
#define CURL_UNKOWN_ERROR 1011

class CurlWrapper {
public:
    CurlWrapper();
    ~CurlWrapper();

    void setRequestHeaders(const std::map<std::string, std::string>& headers);
    void clearRequestHeaders();

    int downloadToFile(const std::string& url, const std::string& file_path, std::function<void(double)> progress_callback);
    int downloadToFile(const std::string& url, const std::string& file_path);
    int downloadToMemory(const std::string& url, std::string& response, std::function<void(double)> progress_callback);
    int downloadToMemory(const std::string& url, std::string& response);
    int sendGetRequest(const std::string& url, std::string& response);
    int sendGetRequest(const std::string& url);
    int sendPostRequest(const std::string& url, std::string json_str, std::string& response);
    int sendPostRequest(const std::string& url);
    int sendDeleteRequest(const std::string& url, std::string& response);
    int sendDeleteRequest(const std::string& url);

private:
    CURL* curl;
    struct curl_slist* headers_list;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static int ProgressCallback(void* ptr, curl_off_t total_download, curl_off_t downloaded, curl_off_t total_upload, curl_off_t uploaded);

    void setupProgress(CURL* curl, std::function<void(double)> progress_callback);
    int checkCurlCode(CURLcode res);
};

#endif // CURLWRAPPER_H