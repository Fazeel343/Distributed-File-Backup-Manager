#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include "utils.h"

class RequestQueue {
public:
    void push(const BackupRequest& req);
    BackupRequest pop();           // blocks until item available
    void shutdown();               // signals workers to stop
    int  size();
    bool is_closed() const;

private:
    std::queue<BackupRequest>   queue_;
    mutable std::mutex          mtx_;
    std::condition_variable     cv_;
    bool                        closed_ = false;
};