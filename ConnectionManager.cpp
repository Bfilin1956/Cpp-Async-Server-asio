//
// Created by r13x on 3/19/26.
//

#include "ConnectionManager.h"
#include "Session.h"


ConnectionManager::Bucket_ &ConnectionManager::find_bucket(const uint64_t id) {
    return buckets_[id % MAX_BUCKETS_];
}

uint64_t ConnectionManager::next_id() {
    return id_.fetch_add(1, std::memory_order_relaxed);
}

ConnectionManager::ConnectionManager() {
    free_ids_.reserve(100);
}

std::shared_ptr<Session> ConnectionManager::get(const uint64_t id) {
    auto &[mtx, clients_] = find_bucket(id);
    std::lock_guard<std::mutex> lock(mtx);

    const auto it = clients_.find(id);
    return it != clients_.end() ? it->second : nullptr;
}

uint64_t ConnectionManager::add(std::shared_ptr<Session> session) {
    uint64_t id;
    if (!free_ids_.empty()) {
        id = free_ids_.back();
        free_ids_.pop_back();
    } else id = next_id();
    auto &[mtx, clients_] = find_bucket(id);
    std::lock_guard<std::mutex> lock(mtx);
    clients_.emplace(id, std::move(session));
    return id;
}

void ConnectionManager::remove(const uint64_t id) {
    auto &[mtx, clients_] = find_bucket(id);
    uint64_t is_exist;
    {
        std::lock_guard<std::mutex> lock(mtx);
        is_exist = clients_.erase(id);
    }
    if (is_exist) free_ids_.emplace_back(id);
}
