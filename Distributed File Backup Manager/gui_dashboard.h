#pragma once
#include <atomic>
#include "metadata_manager.h"
#include "storage_controller.h"
#include "recovery.h"
#include "request_queue.h"

class GUIDashboard {
public:
    GUIDashboard(MetadataManager&   meta,
                 StorageController& storage,
                 RecoveryModule&    recovery,
                 RequestQueue&      queue);

    void run();               // thread entry point (refreshes every 800 ms)
    void stop();

private:
    void render() const;
    void print_bar(int done, int total, int width = 20) const;

    MetadataManager&   meta_;
    StorageController& storage_;
    RecoveryModule&    recovery_;
    RequestQueue&      queue_;
    std::atomic<bool>  running_{true};
};