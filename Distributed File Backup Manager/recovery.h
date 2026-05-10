#pragma once
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>
#include "utils.h"

class RequestQueue;   // forward declare

class RecoveryModule {
public:
    // Schedule a failed request for retry after a delay
    void schedule_retry(const BackupRequest& req, RequestQueue& queue);
    int  total_retries() const;

private:
    std::mutex       mtx_;
    int              retries_ = 0;
};