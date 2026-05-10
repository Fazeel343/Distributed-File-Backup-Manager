#pragma once
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <chrono>
#include "utils.h"

// Manual semaphore (compatible with C++17 and C++20)
class Semaphore {
public:
    explicit Semaphore(int count) : count_(count) {}

    void acquire() {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]{ return count_ > 0; });
        --count_;
    }

    void release() {
        std::lock_guard<std::mutex> lock(mtx_);
        ++count_;
        cv_.notify_one();
    }

private:
    std::mutex              mtx_;
    std::condition_variable cv_;
    int                     count_;
};


class StorageController {
public:
    explicit StorageController(int max_concurrent_writes = MAX_DISK_WRITES);

    // Simulate writing one chunk; returns false if simulated failure
    bool write_chunk(int client_id, const std::string& file_name, int chunk_no);
    int  total_writes() const;

private:
    Semaphore        semaphore_;
    std::atomic<int> total_writes_;
};