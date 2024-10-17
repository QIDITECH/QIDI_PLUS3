#ifndef SERVER_CONTROL_H
#define SERVER_CONTROL_H

#include <atomic>
#include <memory>
#include <thread>
#include <boost/asio.hpp>

class ServerControl {
public:
    static ServerControl& getInstance();

    void start_http_server();
    void stop_http_server();
    void start_udp_server();
    void stop_udp_server();

private:
    ServerControl();
    ~ServerControl();
    ServerControl(const ServerControl&) = delete;
    ServerControl& operator=(const ServerControl&) = delete;

    void signal_handler(int signal);

    class HttpServer;
    std::shared_ptr<HttpServer> http_server_;
    std::thread http_server_thread_;
    std::thread udp_server_thread_;
    std::atomic<bool> http_server_running_;
    std::atomic<bool> udp_server_running_;
    boost::asio::io_context http_io_context_;
    boost::asio::io_context udp_io_context_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> udp_work_guard_;
};

#endif // SERVER_CONTROL_H
