#include "request_queue.h"
#include <stdexcept>

void RequestQueue::push(const BackupRequest& req) {
    {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push(req);
    }
    cv_.notify_one();   // wake one sleeping worker
}

BackupRequest RequestQueue::pop() {
    std::unique_lock<std::mutex> lock(mtx_);
    // Wait until there is work OR the queue is shut down
    cv_.wait(lock, [this]{ return !queue_.empty() || closed_; });

    if (queue_.empty())
        throw std::runtime_error("Queue closed and empty");

    BackupRequest req = queue_.front();
    queue_.pop();
    return req;
}

void RequestQueue::shutdown() {
    {
        std::lock_guard<std::mutex> lock(mtx_);
        closed_ = true;
    }
    cv_.notify_all();   // wake all workers so they can exit
}

int RequestQueue::size() {
    std::lock_guard<std::mutex> lock(mtx_);
    return static_cast<int>(queue_.size());
}

bool RequestQueue::is_closed() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return closed_;
}