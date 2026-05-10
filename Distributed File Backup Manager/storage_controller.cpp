#include "storage_controller.h"
#include <cstdlib>

StorageController::StorageController(int max_concurrent_writes)
    : semaphore_(max_concurrent_writes), total_writes_(0) {}

bool StorageController::write_chunk(int client_id,
                                     const std::string& file_name,
                                     int chunk_no) {
    semaphore_.acquire();   // wait for a free disk slot

    // Simulate I/O delay (50–150 ms per chunk)
    int delay = 50 + rand() % 100;
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));

    // Simulate random failure
    bool success = (rand() % FAILURE_RATE != 0);
    if (success) ++total_writes_;

    semaphore_.release();   // free the disk slot
    return success;
}

int StorageController::total_writes() const {
    return total_writes_.load();
}