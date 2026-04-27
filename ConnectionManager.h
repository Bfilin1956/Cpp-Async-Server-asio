#pragma once
#include <mutex>

#include "Session.h"

class ConnectionManager {
    struct Bucket_ {
        std::mutex mtx;
        std::unordered_map<uint64_t, std::shared_ptr<Session>> clients_;
    };

    static constexpr size_t MAX_BUCKETS_ = 16;
    std::array<Bucket_, MAX_BUCKETS_> buckets_;

    std::atomic<uint64_t> id_;

    Bucket_& find_bucket(const uint64_t id);

    uint64_t next_id();

    std::vector<uint64_t> free_ids_;
public:
    ConnectionManager();
    std::shared_ptr<Session> get(const uint64_t id);

    uint64_t add(std::shared_ptr<Session> session);

    void remove(const uint64_t id);
};
