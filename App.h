#pragma once
#include "Server.h"
class App {
    asio::io_context io_;
    Server server_;
public:
    explicit App();
    void run();
};