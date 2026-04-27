#pragma once

#include "ConnectionManager.h"
#include "Session.h"


class Server {
    tcp::acceptor acceptor_;
    std::shared_ptr<ConnectionManager> connection_manager_;
public:
    explicit Server(const asio::any_io_executor& ex);
    void start();
    awaitable<void> do_accept();
};