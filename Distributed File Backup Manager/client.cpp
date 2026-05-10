#include "client.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>

// Global request ID counter shared across all clients
static std::atomic<int> g_request_counter{1};

Client::Client(int id, RequestQueue& queue, MetadataManager& meta)
    : id_(id), queue_(queue), meta_(meta), next_request_id_(0) {}

void Client::run() {
    for (int i = 0; i < NUM_FILES; i++) {
        BackupRequest req;
        req.client_id     = id_;
        req.request_id    = g_request_counter.fetch_add(1);
        req.file_name     = "client" + std::to_string(id_)
                          + "_file" + std::to_string(i + 1) + ".dat";
        req.file_size_mb  = 2 + rand() % 8;   // 2–9 MB
        req.total_chunks  = req.file_size_mb;  // 1 chunk per MB
        req.chunks_done   = 0;
        req.status        = "pending";
        req.incremental   = (i > 0);           // first file = full, rest = incremental check
        req.retry_count   = 0;

        meta_.register_request(req);
        queue_.push(req);

        std::cout << Color::CYAN << "[Client " << id_ << "] Submitted: "
                  << req.file_name << " (" << req.file_size_mb << " MB)"
                  << Color::RESET << "\n";

        // Stagger submissions
        std::this_thread::sleep_for(std::chrono::milliseconds(300 + rand() % 200));
    }

    std::cout << Color::CYAN << "[Client " << id_ << "] All files submitted.\n"
              << Color::RESET;
}