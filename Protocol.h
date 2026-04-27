#pragma once

namespace Protocol {
    constexpr uint32_t name_{0xBEAFBEAF};
    constexpr uint32_t version_{1};
    constexpr uint32_t maxSize_{1024*1024};

    enum class MsgType : uint16_t {
        Ping,
        Pong,
        Auth,
        Error
    };

    #pragma pack(push, 1)
    struct Header {
        uint32_t name;
        uint16_t version;
        uint16_t type;
        uint16_t flags;
        uint32_t size;
    };
    #pragma pack(pop)

    static_assert(sizeof(Header) == 14);

    inline bool valid_header(const Header& header) {
        if (header.name != name_) return false;
        if (header.version != version_) return false;
        if (header.size > maxSize_) return false;
        return true;
    }

    inline std::vector<std::byte> make_packet(const MsgType &type, const std::span<const std::byte> body = {},
        const uint16_t version = 1, const uint16_t flags = 0) {
        const Header header{
            .name = name_,
            .version = version,
            .type = static_cast<uint16_t>(type),
            .flags = flags,
            .size = static_cast<uint32_t>(body.size())
        };

        std::vector<std::byte> out;
        out.reserve(sizeof(header)+ body.size());

        memcpy(out.data(), &header, sizeof(header));
        if (!body.empty()) memcpy(out.data()+sizeof(header), body.data(), body.size());

        return out;
    }
}