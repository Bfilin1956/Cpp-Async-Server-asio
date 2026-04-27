#include "Session.h"
#include "Parser.h"
#include "Logger.h"
#include "Protocol.h"
Session::Session(tcp::socket socket)
    : socket_(std::move(socket)),
      strand_(asio::make_strand(socket_.get_executor())),
      watchdog_timer_(strand_)
{}

void Session::start() {
    auto self = shared_from_this();

    co_spawn(strand_,[self]()->awaitable<void> {
        co_await self->read();
    }, detached);
    co_spawn(strand_,[self]()->awaitable<void> {
        co_await self->watch_dog();
    }, detached);
}

void Session::send(std::vector<std::byte> data) {
    auto self = shared_from_this();

    asio::post(strand_,
        [self, data = std::move(data)]() mutable {
            self->q_.push(std::move(data));

            if (!self->writing_) {
                self->writing_ = true;

                asio::co_spawn(
                    self->strand_,
                    self->write(),
                    asio::detached
                );
            }
        });
}

awaitable<void> Session::read() {
    boost::system::error_code ec;
        while (true){
            Protocol::Header header;

            co_await asio::async_read(socket_,
                          asio::buffer(&header, sizeof(Protocol::Header)),
                          asio::bind_executor(strand_, asio::redirect_error(asio::use_awaitable, ec)));

            if (ec) {
                close();
                co_return;
            }

            if (!Protocol::valid_header(header)) {
                Log::print_log("Bad header", Log::MsgType::Error);
                close();
                co_return;
            }

            std::vector<std::byte> body;
            body.reserve(header.size);
            co_await asio::async_read(socket_,
                asio::buffer(body.data(), body.size()),
                asio::bind_executor(strand_, asio::redirect_error(asio::use_awaitable, ec)));

            if (ec) {
                close();
                co_return;
            }

            Parser::parser(header,body, shared_from_this());

            last_rx_ = std::chrono::steady_clock::now();
            Log::print_log("Success reading", Log::MsgType::Read);
        }
}

awaitable<void> Session::write() {
    boost::system::error_code ec;
        while (!q_.empty()) {

            co_await asio::async_write(socket_,
                asio::buffer(q_.front().data(),
                q_.front().size()),
                asio::bind_executor(strand_, asio::redirect_error(asio::use_awaitable, ec)));
            if (ec) {
                close();
                co_return;
            }
            q_.pop();
            last_tx_ = std::chrono::steady_clock::now();
            Log::print_log("Success writing", Log::MsgType::Write);
        }

    writing_ = false;
}



void Session::close() {
    auto self = shared_from_this();

    if (closed_.exchange(true))
        return;

    boost::system::error_code ignore;
    socket_.cancel(ignore);
    watchdog_timer_.cancel();
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignore);
    socket_.close(ignore);

    if (const auto manager = manager_.lock()) {
        manager->remove(session_id_);
    }
    Log::print_log("Client disconnected");
}

awaitable<void> Session::watch_dog() {
    while (true) {
        watchdog_timer_.expires_after(ping_interval_);
        boost::system::error_code ec;
        co_await watchdog_timer_.async_wait(asio::redirect_error(use_awaitable, ec));

        if (ec == asio::error::operation_aborted)
            co_return;

        if (closed_) co_return;

        auto now = std::chrono::steady_clock::now();
        auto idle = now - last_rx_;

        if (idle > timeout_) {
            Log::print_log("Client timeout.");
            close();
            co_return;
        }

        if (idle > ping_interval_ && !ping_in_flight_.exchange(true)) {
            send_ping();
        }
    }
}

void Session::send_ping() {
    auto ping_packet = Protocol::make_packet(Protocol::MsgType::Ping);
    send(std::move(ping_packet));
}

void Session::set_false_ping_in_flight() {
    ping_in_flight_ = false;
}

void Session::set_manager(const std::shared_ptr<ConnectionManager> &manager) {
    manager_ = manager;
}

void Session::set_session_id(const uint64_t session_id) {
    session_id_ = session_id;
}