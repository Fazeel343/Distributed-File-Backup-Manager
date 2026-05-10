#include "recovery.h"
#include "request_queue.h"
#include <iostream>

void RecoveryModule::schedule_retry(const BackupRequest& req,
                                     RequestQueue& queue) {
    BackupRequest retry = req;
    retry.status       = "pending";
    retry.chunks_done = req.chunks_done;
    retry.retry_count += 1;

    if (retry.retry_count > 3) {
        std::cout << Color::RED
                  << "[Recovery] " << req.file_name
                  << " exceeded max retries. Giving up."
                  << Color::RESET << "\n";
        return;
    }

    // Retry after 500 ms in a detached thread
    std::thread([retry, &queue]() mutable {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        queue.push(retry);
    }).detach();

    std::lock_guard<std::mutex> lock(mtx_);
    retries_++;
}

int RecoveryModule::total_retries() const {
    return retries_;
}
