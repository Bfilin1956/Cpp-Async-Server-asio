#include "Server.h"
#include "Logger.h"
Server::Server(const asio::any_io_executor& ex)
    :  acceptor_(ex)
{
    acceptor_.open(tcp::v4());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind({tcp::v4(), 9000});
    acceptor_.listen();

    connection_manager_ = std::make_shared<ConnectionManager>();
}

void Server::start()  {
    co_spawn(acceptor_.get_executor(), do_accept(), detached);
}

awaitable<void> Server::do_accept() {
    boost::system::error_code ec;
    while (true){

        auto socket = co_await  acceptor_.async_accept(asio::redirect_error(use_awaitable, ec));

        if (ec == boost::asio::error::address_in_use) {
            Log::print_log("address_in_use", Log::MsgType::Error);
            co_return;
        }
        if (ec) {
            Log::print_log(ec.message(), Log::MsgType::Error);
            continue;
        }

        auto session = std::make_shared<Session>(std::move(socket));
        const auto id = connection_manager_->add(std::move(session));
        session->set_manager(connection_manager_);
        session->set_session_id(id);
        session->start();

        Log::print_log("Client connected");
    }
}
