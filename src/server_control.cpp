
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/beast.hpp>
#include <boost/bind/bind.hpp>
#include <nlohmann/json.hpp>

#include "../include/app_client.h"
#include "../include/mks_log.h"
#include "../include/server_control.h"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;
namespace beast = boost::beast;
namespace http = beast::http;
using json = nlohmann::json;
using namespace boost::placeholders;

class ServerControl::HttpServer {
public:
    HttpServer(boost::asio::io_context& ioc, tcp::endpoint endpoint)
        : acceptor_(ioc, endpoint), socket_(ioc) {
        do_accept();
    }

    void stop()
    {
        acceptor_.close();
        socket_.close();
    }

private:
    void do_accept() 
    {
        acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
            if (!ec)
                std::make_shared<Session>(std::move(socket_))->start();
            if (ServerControl::getInstance().http_server_running_)
                do_accept();
        });
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;

    class Session : public std::enable_shared_from_this<Session> {
    public:
        Session(tcp::socket socket)
            : socket_(std::move(socket)) {}

        void start() 
        {
            do_read();
        }

    private:
        void do_read() 
        {
            auto self(shared_from_this());
            http::async_read(socket_, buffer_, request_,
                [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
                    boost::ignore_unused(bytes_transferred);
                    if (!ec)
                        handle_request();
                    else
                        MKSLOG_RED("[server_control][Error] Read error: %s", ec.message());
                });
        }

        void handle_request() 
        {
            if (request_.method() == http::verb::post && request_.target() == "/syncBindStatus") 
            {
                if (request_.body().empty() || request_.find(http::field::content_type)->value() != "application/json") {
                    send_response(http::status::bad_request, "Invalid request format");
                    return;
                }

                try 
                {
                    json request_data = json::parse(request_.body());
                    current_data_ = request_data;
                    print_current_data();

                    send_response(http::status::ok, "Data received");
                }
                catch (json::parse_error& e) 
                {
                    send_response(http::status::bad_request, "JSON parse error");
                }
            }
            else if (request_.method() == http::verb::get && request_.target().starts_with("/syncBindStatus")) 
            {
                auto query = request_.target().to_string();
                auto pos = query.find('?');
                if (pos != std::string::npos) 
                {
                    auto query_string = query.substr(pos + 1);
                    std::map<std::string, std::string> params;
                    parse_query(query_string, params);

                    if (params["action"] == "bind")
                    {
                        if (app_cli.set_bind_status("BOUND", params))
                            send_response(http::status::ok, "Printer successfully bound");
                        else
                            send_response(http::status::bad_request, "Invalid params");
                    }
                    else
                    {
                        send_response(http::status::bad_request, "Invalid action");
                    }
                } 
                else
                {
                    send_response(http::status::bad_request, "No parameters found");
                }
            } 
            else
                send_response(http::status::method_not_allowed, "Invalid method");
        }

        void send_response(http::status status, const std::string& message) 
        {
            auto self(shared_from_this());
            response_.result(status);
            response_.version(request_.version());
            response_.set(http::field::server, "Boost.Beast/248");
            response_.set(http::field::content_type, "text/plain");
            response_.body() = message;
            response_.prepare_payload();

            http::async_write(socket_, response_,
                [this, self](boost::system::error_code ec, std::size_t) {
                    socket_.shutdown(tcp::socket::shutdown_send, ec);
                });
        }

        void print_current_data() 
        {
            std::string data_str = current_data_.dump();
            data_str.erase(data_str.find_last_not_of(" \n\r\t") + 1);
            MKSLOG("[server_control] Current Data: %s", data_str);
        }

        void parse_query(const std::string& query, std::map<std::string, std::string>& params) 
        {
            std::vector<std::string> pairs;
            boost::split(pairs, query, boost::is_any_of("&"));
            for (const auto& pair : pairs) 
            {
                std::vector<std::string> kv;
                boost::split(kv, pair, boost::is_any_of("="));
                if (kv.size() == 2) 
                    params[kv[0]] = kv[1];
            }
        }

        tcp::socket socket_;
        beast::flat_buffer buffer_;
        http::request<http::string_body> request_;
        http::response<http::string_body> response_;
        json current_data_;
    };
};

ServerControl& ServerControl::getInstance()
{
    static ServerControl instance;
    return instance;
}

ServerControl::ServerControl()
    : http_server_running_(false), 
      udp_server_running_(false),
      udp_work_guard_(boost::asio::make_work_guard(udp_io_context_)) {
    signal(SIGINT, [](int signal) { ServerControl::getInstance().signal_handler(signal); });
    signal(SIGTERM, [](int signal) { ServerControl::getInstance().signal_handler(signal); });
}

ServerControl::~ServerControl()
{
    stop_http_server();
    stop_udp_server();
}

void ServerControl::start_http_server()
{
    if (!http_server_running_) 
    {
        http_server_running_ = true;
        tcp::endpoint endpoint(tcp::v4(), 8990);
        http_server_ = std::make_shared<HttpServer>(http_io_context_, endpoint);
        http_server_thread_ = std::thread([this]() { http_io_context_.run(); });
    }
}

void ServerControl::stop_http_server()
{
    if (http_server_running_) 
    {
        http_server_running_ = false;
        if (http_server_)
            http_server_->stop();
        http_io_context_.stop();
        if (http_server_thread_.joinable())
            http_server_thread_.join();
        http_server_.reset();
    }
}

void ServerControl::start_udp_server()
{
    if (!udp_server_running_) 
    {
        udp_server_running_ = true;
        udp_server_thread_ = std::thread([this]() {
            udp::socket socket(udp_io_context_, udp::endpoint(udp::v4(), 8989));

            while (udp_server_running_) 
            {
                char data[1024];
                udp::endpoint sender_endpoint;
                boost::system::error_code error;
                size_t length = socket.receive_from(boost::asio::buffer(data, 1024), sender_endpoint, 0, error);

                if (error && error != boost::asio::error::message_size)
                {
                    if (error == boost::asio::error::operation_aborted)
                        break; // 程序中止
                    MKSLOG_RED("[server_control][Error] Receive error: %s", error.message());
                    continue;
                }

                std::string received_message(data, length);
                MKSLOG("[server_control] Received UDP message: {}", received_message);

                received_message.erase(received_message.find_last_not_of(" \n\r\t") + 1);

                if (received_message == "mkswifi") 
                {
                    MKSLOG("[server_control]entering mkswifi path");
                    std::string response = "mkswifi:QIDI@Q1 Pro,HJNLM000D268615CE3FB,192.168.31.201,aws,856325,1";
                    socket.send_to(boost::asio::buffer(response), sender_endpoint, 0, error);
                    if (error)
                        MKSLOG_RED("[server_control][Error] UDP response error: %s", error.message());
                    else 
                        MKSLOG("[server_control] Sent UDP response: %s", response);
                }
            }
            socket.close();
        });
    }
}

void ServerControl::stop_udp_server()
{
    if (udp_server_running_) 
    {
        udp_server_running_ = false;
        udp_io_context_.stop();

        // 关闭socket文件描述符的读通道，防止因socket.receive_from的阻塞导致关闭缓慢
        udp::socket socket(udp_io_context_);
        socket.open(udp::v4());
        socket.shutdown(udp::socket::shutdown_receive);

        if (udp_server_thread_.joinable())
            udp_server_thread_.join();
    }
}

void ServerControl::signal_handler(int signal) 
{
    stop_http_server();
    stop_udp_server();
}
