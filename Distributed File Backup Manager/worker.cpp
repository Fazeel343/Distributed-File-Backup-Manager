#include "worker.h"
#include <iostream>
#include <stdexcept>

Worker::Worker(int id,
               RequestQueue&      queue,
               MetadataManager&   meta,
               StorageController& storage,
               RecoveryModule&    recovery)
    : id_(id), queue_(queue), meta_(meta),
      storage_(storage), recovery_(recovery) {}

void Worker::run() {
    while (true) {
        BackupRequest req;
        try {
            req = queue_.pop();   // blocks until work or shutdown
        } catch (const std::runtime_error&) {
            break;   // queue shut down and empty — exit
        }

        std::cout << Color::YELLOW
                  << "[Worker " << id_ << "] Processing: " << req.file_name
                  << Color::RESET << "\n";

        // ── Incremental backup check ──────────────────────────
        if (req.incremental && !meta_.is_modified(req)) {
            std::cout << Color::GREEN
                      << "[Worker " << id_ << "] SKIP (unchanged): "
                      << req.file_name << Color::RESET << "\n";
            meta_.update_status(req.request_id, "skipped", 0);
            continue;
        }

        meta_.update_status(req.request_id, "in_progress", 0);
        bool all_ok = true;

        // ── Chunk-by-chunk write ──────────────────────────────
        for (int c = req.chunks_done + 1; c <= req.total_chunks; c++) {
            bool ok = storage_.write_chunk(id_, req.file_name, c);
            if (!ok) {
                std::cout << Color::RED
                          << "[Worker " << id_ << "] CHUNK FAIL: "
                          << req.file_name << " chunk " << c
                          << Color::RESET << "\n";
                meta_.update_status(req.request_id, "failed", c - 1);
                meta_.add_log("Chunk failure: " + req.file_name +
                              " chunk " + std::to_string(c));
                recovery_.schedule_retry(req, queue_);
                all_ok = false;
                break;
            }
            meta_.update_status(req.request_id, "in_progress", c);
        }

        if (all_ok) {
            meta_.update_status(req.request_id, "done", req.total_chunks);
            meta_.record_backup_hash(req);
            std::cout << Color::GREEN
                      << "[Worker " << id_ << "] DONE: " << req.file_name
                      << Color::RESET << "\n";
        }
    }

    std::cout << Color::MAGENTA
              << "[Worker " << id_ << "] Exiting.\n" << Color::RESET;
}
