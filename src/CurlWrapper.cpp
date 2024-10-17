#include "../include/CurlWrapper.h"

CurlWrapper::CurlWrapper() : headers_list(nullptr)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) 
    {
        MKSLOG_RED("[CurlWrapper][Error] Failed to initialize CURL");
        throw std::runtime_error("[CurlWrapper][Error] Failed to initialize CURL");
    }
}

CurlWrapper::~CurlWrapper()
{
    if (headers_list) curl_slist_free_all(headers_list);
    if (curl) curl_easy_cleanup(curl);
    curl_global_cleanup();
}

void CurlWrapper::setRequestHeaders(const std::map<std::string, std::string>& headers)
{
    clearRequestHeaders();
    for (const auto& header : headers) 
    {
        std::string header_string = header.first + ": " + header.second;
        headers_list = curl_slist_append(headers_list, header_string.c_str());
    }
    if (headers_list) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_list);
}

void CurlWrapper::clearRequestHeaders()
{
    if (headers_list) 
    {
        curl_slist_free_all(headers_list);
        headers_list = nullptr;
    }
}

size_t CurlWrapper::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    if (userp == nullptr)
        return size * nmemb; // 忽略数据

    std::ofstream* outFile = static_cast<std::ofstream*>(userp);
    outFile->write(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

size_t CurlWrapper::WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    if (userp == nullptr)
        return size * nmemb; // 忽略数据

    std::string* response = static_cast<std::string*>(userp);
    size_t total_size = size * nmemb;
    response->append(static_cast<char*>(contents), total_size);
    return total_size;
}

int CurlWrapper::ProgressCallback(void* ptr, curl_off_t total_download, curl_off_t downloaded, curl_off_t total_upload, curl_off_t uploaded)
{
    std::function<void(double)>* progress_callback = static_cast<std::function<void(double)>*>(ptr);
    if (total_download > 0)
    {
        double progress = static_cast<double>(downloaded) / static_cast<double>(total_download) * 100.0;
        (*progress_callback)(progress);
    }
    return 0;
}

void CurlWrapper::setupProgress(CURL* curl, std::function<void(double)> progress_callback)
{
    if (progress_callback)
    {
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progress_callback);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    }
    else curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
}

int CurlWrapper::checkCurlCode(CURLcode res) 
{
    if (res != CURLE_OK) 
    {
        MKSLOG_RED("[CurlWrapper][Error] curl_easy_perform() failed: %s", curl_easy_strerror(res));
        switch (res)
        {
            case CURLE_OPERATION_TIMEDOUT: return CURL_TIMEOUT;
            case CURLE_PEER_FAILED_VERIFICATION: return CURL_SSL_VERIFICATION_FAIL;
            case CURLE_COULDNT_RESOLVE_HOST: return CURL_DNS_RESOLVE_FAIL;
            case CURLE_WRITE_ERROR: return CURL_DISK_FULL;
            default: return CURL_INITIALIZATION_FAIL;
        }
    }

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code >= 400 && http_code < 500) 
    {
        MKSLOG_YELLOW("[CurlWrapper][Warning] Bad request: %d", http_code);
        if (http_code == 404) return CURL_RESOURCE_NOT_FOUND;
        else return CURL_BAD_REQUEST;
    } 
    else if (http_code >= 500) 
    {
        MKSLOG_YELLOW("[CurlWrapper][Warning] Server error: %d", http_code);
        return CURL_SERVER_ERROR;
    }

    return SUCCESS;
}

int CurlWrapper::downloadToFile(const std::string& url, const std::string& file_path, std::function<void(double)> progress_callback)
{
    MKSLOG("[CurlWrapper] Downloading file from %s to: %s", url, file_path.c_str());

    if (!curl) 
    {
        MKSLOG_RED("[CurlWrapper][Error] CURL not initialized");
        return CURL_INITIALIZATION_FAIL;
    }

    std::string temp_file_path = file_path + ".downloading";
    std::ofstream outFile(temp_file_path, std::ios::binary);
    if (!outFile.is_open()) 
    {
        MKSLOG_RED("[CurlWrapper][Error] Failed to open file: %s", file_path.c_str());
        return CURL_FILE_OPEN_FAIL;
    }

    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outFile);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 10);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30);
    setupProgress(curl, progress_callback);

    CURLcode res = curl_easy_perform(curl);
    int status = checkCurlCode(res);

    outFile.close();

    if (status != SUCCESS) 
    {
        std::remove(temp_file_path.c_str());
        return status;
    }

    std::rename(temp_file_path.c_str(), file_path.c_str());

    return SUCCESS;
}

int CurlWrapper::downloadToFile(const std::string& url, const std::string& file_path)
{
    return downloadToFile(url, file_path, nullptr);
}

int CurlWrapper::downloadToMemory(const std::string& url, std::string& response, std::function<void(double)> progress_callback)
{
    MKSLOG("[CurlWrapper] Downloading content to memory from: %s", url.c_str());

    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 10);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30);
    setupProgress(curl, progress_callback);

    CURLcode res = curl_easy_perform(curl);
    int status = checkCurlCode(res);
    if (status != SUCCESS) return status;

    return SUCCESS;
}

int CurlWrapper::downloadToMemory(const std::string& url, std::string& response)
{
    return downloadToMemory(url, response, nullptr);
}

int CurlWrapper::sendGetRequest(const std::string& url, std::string& response)
{
    MKSLOG("[CurlWrapper] GET request to: %s", url.c_str());

    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);

    CURLcode res = curl_easy_perform(curl);

    return checkCurlCode(res);;
}

int CurlWrapper::sendGetRequest(const std::string& url)
{
    MKSLOG("[CurlWrapper] GET request to: %s", url.c_str());

    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, nullptr);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);

    CURLcode res = curl_easy_perform(curl);

    return checkCurlCode(res);;
}

int CurlWrapper::sendPostRequest(const std::string& url, std::string post_fields, std::string& response)
{
    //B
    std::cout << "[CurlWrapper] POST request to: "<< url << std::endl;

    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_list);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);

    CURLcode res = curl_easy_perform(curl);

    return checkCurlCode(res);
}

int CurlWrapper::sendPostRequest(const std::string& url)
{
    MKSLOG("[CurlWrapper] POST request to: %s", url.c_str());

    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, nullptr);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_list);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);

    CURLcode res = curl_easy_perform(curl);

    return checkCurlCode(res);
}

int CurlWrapper::sendDeleteRequest(const std::string& url, std::string& response)
{
    MKSLOG("[CurlWrapper] DELETE request to: %s", url.c_str());

    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_list);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);

    CURLcode res = curl_easy_perform(curl);

    return checkCurlCode(res);
}

int CurlWrapper::sendDeleteRequest(const std::string& url)
{
    MKSLOG("[CurlWrapper] DELETE request to: %s", url.c_str());

    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, nullptr);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_list);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);

    CURLcode res = curl_easy_perform(curl);

    return checkCurlCode(res);
}