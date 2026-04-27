#pragma once
#include <queue>
#include <memory>
#include <boost/asio.hpp>

#include "ConnectionManager.h"

namespace asio = boost::asio;
using boost::asio::awaitable;
using boost::asio::ip::tcp;
using boost::asio::buffer;
using boost::asio::detached;
using boost::asio::co_spawn;
using boost::asio::use_awaitable;
using boost::asio::thread_pool;

class Session : public std::enable_shared_from_this<Session>{
    tcp::socket socket_;
    asio::strand<asio::any_io_executor> strand_;
    asio::steady_timer watchdog_timer_;
    std::queue<std::vector<std::byte>> q_;
    asio::streambuf buffer_;
    bool writing_ = false;
    std::atomic_bool closed_ = false;
    std::atomic<bool> ping_in_flight_{false};
    std::chrono::steady_clock::time_point last_rx_{};
    std::chrono::steady_clock::time_point last_tx_{};
    static constexpr auto ping_interval_ = std::chrono::seconds(15);
    static constexpr auto timeout_ = std::chrono::seconds(45);

    std::weak_ptr<ConnectionManager> manager_;
    uint64_t session_id_{};

public:
    unsigned id{};
    explicit Session(tcp::socket socket);
    void start();
    void send(std::vector<std::byte> data);
    awaitable<void> watch_dog();
    void set_manager(const std::shared_ptr<ConnectionManager> &manager);
    void set_session_id(uint64_t session_id);

private:
    void set_false_ping_in_flight();
    void send_ping();
    awaitable<void> read();
    awaitable<void> write();
    void close();
};
