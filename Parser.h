#pragma once
#include "Protocol.h"

namespace Parser {
    inline void parser(const Protocol::Header &header,
        std::vector<std::byte> & body,
        std::shared_ptr<Session> session) {
            switch (static_cast<Protocol::MsgType>(header.type)) {

                case Protocol::MsgType::Ping:
                    break;
                case Protocol::MsgType::Pong:
                    break;
                case Protocol::MsgType::Auth:
                    break;
                case Protocol::MsgType::Error:
                    break;
            }
    }
}
